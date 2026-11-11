#ifndef HD2_GAME_INTEGRITY_THREADS_H
#define HD2_GAME_INTEGRITY_THREADS_H

#include "game/integrity_observation_actions.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>

namespace hd2::game {

//009968AB..00996909: first empty/matching process thread slot and saved caller.
//009977FE/00997837 consume the SAME slot's override/caller after the actual API.
//Native IDs persist, including after a thread exits; device/mission retries do
//not reset them. This is continuation storage, not an observation table or RNG.
class IntegrityThreads {
    static constexpr std::size_t capacity=64;
    enum class Decision {pending,rejected,action,fallback,failed};
    struct Slot {
        std::uint32_t thread=0;
        std::uintptr_t caller=0;
        IntegrityReturnOverride returned;
        Decision decision=Decision::pending;
        bool active=false;
    };
    struct State {
        std::mutex mutex;
        std::array<Slot,capacity> slots{};
        bool retired=false;
    };
public:
    struct Completion {std::uint32_t value;std::uintptr_t caller;};
    class Call {
    public:
        Call()=default;
        Call(const Call&)=delete;
        Call& operator=(const Call&)=delete;
        Call(Call&& other)noexcept:state_(std::move(other.state_)),slot_(other.slot_),origin_(other.origin_){}
        Call& operator=(Call&& other)noexcept {
            if(this!=&other){release();state_=std::move(other.state_);slot_=other.slot_;origin_=other.origin_;}
            return *this;
        }
        ~Call(){release();}
        bool apply(std::uint8_t action,bool validated,bool blocked,std::uint32_t matched,
            std::uint32_t* destination) {
            const auto state=require_state();
            std::lock_guard<std::mutex> lock(state->mutex);
            auto& slot=require_slot(*state);
            if(slot.decision!=Decision::pending)
                throw std::logic_error("Integrity call decision already applied");
            slot.decision=Decision::failed;
            const auto accepted=apply_integrity_observation_action(action,validated,blocked,matched,destination,slot.returned);
            slot.decision=accepted?Decision::action:Decision::rejected;
            return accepted;
        }
        void fallback(std::uint8_t action) {
            const auto state=require_state();
            std::lock_guard<std::mutex> lock(state->mutex);
            auto& slot=require_slot(*state);
            if(slot.decision!=Decision::pending&&slot.decision!=Decision::rejected)
                throw std::logic_error("Integrity call fallback already resolved");
            slot.decision=Decision::failed;
            fallback_integrity_observation_action(action);
            slot.decision=Decision::fallback;
        }
        Completion complete(std::uint32_t actual_api_result) {
            const auto state=require_state(); //Keep state alive until lock is released.
            std::lock_guard<std::mutex> lock(state->mutex);
            auto& slot=require_slot(*state);
            if(slot.decision==Decision::rejected)
                throw std::logic_error("Rejected integrity call requires its actual fallback");
            if(slot.decision==Decision::failed)
                throw std::logic_error("Failed integrity receiver requires call retirement");
            const Completion result{restore_integrity_api_return(actual_api_result,slot.returned),slot.caller};
            clear(slot);
            state_.reset();
            return result;
        }
        //Source exception cleanup: retain native ID, retire only this call's
        //borrowed caller/override. No synthetic completion/publication occurs.
        void release()noexcept {
            const auto state=std::move(state_);
            if(!state)return;
            std::lock_guard<std::mutex> lock(state->mutex);
            clear(state->slots[slot_]);
        }
    private:
        friend class IntegrityThreads;
        Call(std::shared_ptr<State> state,std::size_t slot)noexcept
            :state_(std::move(state)),slot_(slot),origin_(std::this_thread::get_id()){}
        std::shared_ptr<State> require_state()const {
            if(!state_)throw std::logic_error("Integrity continuation already retired");
            return state_;
        }
        Slot& require_slot(State& state)const {
            if(origin_!=std::this_thread::get_id())
                throw std::logic_error("Integrity continuation returned on a different thread");
            if(state.retired||!state.slots[slot_].active)
                throw std::logic_error("Integrity continuation owner retired");
            return state.slots[slot_];
        }
        static void clear(Slot& slot)noexcept {
            slot.active=false;slot.caller=0;slot.returned={};slot.decision=Decision::pending;
        }
        std::shared_ptr<State> state_;
        std::size_t slot_=0;
        std::thread::id origin_;
    };

    IntegrityThreads():state_(std::make_shared<State>()){}
    IntegrityThreads(const IntegrityThreads&)=delete;
    IntegrityThreads& operator=(const IntegrityThreads&)=delete;
    ~IntegrityThreads(){retire();}

    Call acquire(std::uint32_t thread,std::uintptr_t caller) {
        if(!thread||!caller)throw std::invalid_argument("Integrity continuation requires actual thread/caller identity");
        std::lock_guard<std::mutex> lock(state_->mutex);
        if(state_->retired)throw std::logic_error("Integrity process continuation owner retired");
        for(std::size_t n=0;n<capacity;++n){
            auto& slot=state_->slots[n];
            if(slot.thread&&slot.thread!=thread)continue;
            //Native nested calls overwrite their saved caller. Source rejects
            //that unsafe re-entry before changing the outer call's state.
            if(slot.active)throw std::logic_error("Nested integrity continuation on the same process thread");
            slot.thread=thread;slot.caller=caller;slot.returned={};slot.active=true;
            return Call(state_,n);
        }
        //Original falls through to slot64 (outside all three arrays).
        throw std::runtime_error("Integrity process thread table exhausted");
    }
    void retire()noexcept {
        //A shared state prevents dangling loans if process replacement occurs
        //inside an API callback. Retired loans reject further actions/returns.
        std::lock_guard<std::mutex> lock(state_->mutex);
        state_->retired=true;
        for(auto& slot:state_->slots)Call::clear(slot);
    }
    std::size_t active_calls()const {
        std::lock_guard<std::mutex> lock(state_->mutex);
        std::size_t result=0;for(const auto& slot:state_->slots)result+=slot.active?1u:0u;
        return result;
    }
private:
    std::shared_ptr<State> state_;
};

} //namespace hd2::game
#endif
