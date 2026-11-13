#ifndef HD2_GAME_RUNTIME_ACTOR_REGISTRY_H
#define HD2_GAME_RUNTIME_ACTOR_REGISTRY_H

#include "frontend/cleanup.h"
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace hd2::game {

//00479450's separate module74 word; the caller borrows its actual storage.
inline std::int32_t consume_runtime_detached_identity(std::uint32_t& counter){
    const auto old=counter++;const auto value=old+1000000u;std::int32_t result;std::memcpy(&result,&value,4);return result;
}

//004770B0/00477190/004772F0/00477330/00477430. This is the runtime
//registry, not Scene2's reserved identity array. Backend supplies real matching
//factories, activation/deactivation, presentation-reference removal and frame
//userdata endpoints. Update visitors are required callers, never empty defaults.
template<class Backend>
class RuntimeActorRegistry {
public:
    using Frame=typename Backend::Frame;
    using ActorOwner=typename Backend::ActorOwner;
    using Actor=typename ActorOwner::element_type;
    explicit RuntimeActorRegistry(Backend& backend) : backend_(backend) {}
    ~RuntimeActorRegistry() {clear();}
    RuntimeActorRegistry(const RuntimeActorRegistry&)=delete;
    RuntimeActorRegistry& operator=(const RuntimeActorRegistry&)=delete;
    Actor* create(std::uint32_t type,Frame frame,std::int32_t identity=-1) {
        if(clearing_)throw std::logic_error("Runtime actor creation during cleanup");
        auto owner=backend_.make_runtime_actor(type);
        if(!owner)return nullptr;
        if(!owner->initialize(frame))throw std::runtime_error("Required runtime actor initialization failed");
        auto* actor=owner.get();
        actor->base().registered(true);
        owners_.reserve(owners_.size()+1);
        backend_.bind_runtime_actor_frame(actor->base().frame(),actor);
        if(identity<0) {
            identity=next_identity_;
            const auto next=static_cast<std::uint32_t>(next_identity_)+1u;
            std::memcpy(&next_identity_,&next,4);
        }
        actor->base().id10(identity); //00610230; explicit identities do not advance70.
        owners_.push_back(Record{std::move(owner)});
        try {
            if(backend_.mode_flags()&1) {
                find(actor)->authority_started=true;
                backend_.activate_runtime_actor(*actor,true);
            }
            if(!find(actor))throw std::runtime_error("Runtime actor removed during construction");
            if(backend_.mode_flags()&2) {
                find(actor)->presentation_started=true;
                backend_.activate_runtime_actor(*actor,false);
            }
            if(!find(actor))throw std::runtime_error("Runtime actor removed during construction");
        } catch(...) {destroy(actor,true);throw;}
        return actor;
    }
    //00477660's counter reset is separate from queued construction. Do not reset
    //this counter on every create or use item identities as runtime actor IDs.
    void begin_activation(bool authority) {
        if(authority||!(backend_.mode_flags()&1))next_identity_=0;
    }
    Actor* identity(std::int32_t value) const { //004772F0 searches backwards, including duplicate IDs.
        for(auto i=owners_.size();i!=0;)if(auto* actor=owners_[--i].owner.get();actor->base().id10()==value)return actor;
        return nullptr;
    }
    template<class Base>Actor* actor_for_base(const Base& base) const {
        for(const auto& record:owners_)if(&record.owner->base()==&base)return record.owner.get();
        return nullptr;
    }
    void active(Actor* actor,bool value) { //00477330.
        if(!actor||!find(actor))throw std::logic_error("Runtime active change requires its owned actor");
        const auto index=actor->base().id14();
        if((value&&index>=0)||(!value&&index<0))return;
        if(visiting_) {toggles_.push_back(actor);return;} //No desired-state byte: native queue toggles at drain.
        if(value) {
            if(active_.size()>=std::size_t(std::numeric_limits<std::int32_t>::max()))
                throw std::runtime_error("Runtime active count exceeds bounds");
            active_.push_back(actor);actor->base().id14(static_cast<std::int32_t>(active_.size()-1));
        } else remove_active(*actor);
    }
    void remove(Actor* actor) noexcept { //00477190; defer while +6C is set.
        if(!actor||!find(actor))return;
        if(visiting_) {
            try {removals_.push_back(actor);}catch(...) {backend_.record_actor_cleanup_failure(std::current_exception());}
            return;
        }
        destroy(actor,false);
    }
    template<class Visit>void visit_active(Visit&& visit) { //004774C0/+18 or00477590/+14.
        if(visiting_||clearing_)throw std::logic_error("Nested runtime actor traversal");
        visiting_=true;
        frontend::CleanupErrors errors;
        errors.attempt([&] {
            //The native count is captured once; changes queue until the end.
            for(auto i=active_.size();i!=0;)visit(*active_[--i]);
        });
        visiting_=false;
        for(auto i=toggles_.size();i!=0;) {
            auto* actor=toggles_[--i];
            if(find(actor))errors.attempt([&]{active(actor,actor->base().id14()==-1);});
        }
        toggles_.clear();
        for(auto i=removals_.size();i!=0;)remove(removals_[--i]);
        removals_.clear();errors.rethrow();
    }
    void clear() noexcept { //00477430: reverse actual ownership order.
        if(visiting_) {backend_.record_actor_cleanup_failure(std::make_exception_ptr(
            std::logic_error("Runtime actor cleanup during traversal")));return;}
        if(clearing_)return;
        for(const auto& record:owners_)if(record.removing) {
            backend_.record_actor_cleanup_failure(std::make_exception_ptr(
                std::logic_error("Runtime actor cleanup during removal")));return;
        }
        clearing_=true;
        while(!owners_.empty())destroy(owners_.back().owner.get(),false);
        active_.clear();toggles_.clear();removals_.clear();clearing_=false;
    }
    const std::vector<Actor*>& active_actors() const {return active_;}
    template<class Predicate>void remove_owned_forward(Predicate&& matches){ //00479700; native increment still skips a swapped-in owner after removal.
        for(std::size_t index=0;index<owners_.size();++index){auto* actor=owners_[index].owner.get();if(matches(*actor))remove(actor);}
    }
    std::size_t size() const {return owners_.size();}
    bool owns(const Actor* actor)const{for(const auto& record:owners_)if(record.owner.get()==actor)return true;return false;}
private:
    struct Record {ActorOwner owner;bool authority_started=false,presentation_started=false,removing=false;};
    Record* find(Actor* actor) {
        for(auto& record:owners_)if(record.owner.get()==actor)return &record;
        return nullptr;
    }
    void remove_active(Actor& actor) noexcept {
        const auto index=actor.base().id14();
        if(index<0)return;
        active_[static_cast<std::size_t>(index)]=active_.back();
        active_[static_cast<std::size_t>(index)]->base().id14(index);
        active_.pop_back();actor.base().id14(-1);
    }
    void destroy(Actor* actor,bool rollback) noexcept {
        auto* record=find(actor);if(!record||record->removing)return;
        record->removing=true;
        const bool authority=record->authority_started,presentation=record->presentation_started;
        remove_active(*actor);
        static_assert(noexcept(std::declval<Backend&>().unbind_runtime_actor_frame(std::declval<Frame>())),
                      "Runtime frame userdata cleanup must not throw");
        backend_.unbind_runtime_actor_frame(actor->base().frame());
        frontend::CleanupErrors errors;
        errors.attempt([&] {if(rollback?presentation:bool(backend_.mode_flags()&2))backend_.deactivate_runtime_actor(*actor,false);});
        errors.attempt([&] {if(rollback?authority:bool(backend_.mode_flags()&1))backend_.deactivate_runtime_actor(*actor,true);});
        remove_active(*actor); //A failing/reentrant receiver must not leave a freed active pointer.
        //Consume this owner before calling its destructor, which may remove others.
        ActorOwner owner;
        for(auto i=owners_.size();i!=0;)if(owners_[--i].owner.get()==actor) {
            owner=std::move(owners_[i].owner);
            if(i+1!=owners_.size())owners_[i]=std::move(owners_.back());
            owners_.pop_back();break;
        }
        //00477190 removes from the ownership list before0049EF70. Each
        //callback can change mode; re-read it for that final presentation consumer.
        errors.attempt([&] {if(backend_.mode_flags()&2)backend_.remove_actor_presentation_references(*actor);});
        owner.reset();
        try {errors.rethrow();}catch(...) {backend_.record_actor_cleanup_failure(std::current_exception());}
    }
    Backend& backend_;
    std::vector<Record> owners_;
    std::vector<Actor*> active_,toggles_,removals_;
    std::int32_t next_identity_=0;
    bool visiting_=false,clearing_=false;
};

} //namespace hd2::game
#endif
