#ifndef HD2_HOST_GAME_HUMAN_SOUND_LOADING_H
#define HD2_HOST_GAME_HUMAN_SOUND_LOADING_H

#include "host/game_sight_loading.h"
#include "game/human_sound_notifications.h"
#include "frontend/spatial_sound.h"
#include <algorithm>
#include <exception>
#include <utility>
#include <vector>
#include <mmsystem.h>

namespace hd2::host {

//Actual005A5430 receiver uses SAME Scene2 actor-frame registry and Sight
//friendship source. SOUND must retain this borrower only while the world lives.
//Derived callbacks share the SAME Human state factory/inline owner. World
//supplies the actual CRT random stream and typed attachment mapping.
template<class World,class StateCallbacks,class SoundOwner>class GameHumanSoundLoading {
public:
    using Frame=GameSceneActors::Frame;using Actor=GameSceneActors::Instance*;
    using Human=typename GameSightLoading<World>::Human;
    using Event=game::HumanSoundEvent<Frame,Actor>;
    GameHumanSoundLoading(GameSceneActors& frames,GameSightLoading<World>& sight,World& world,StateCallbacks& states,SoundOwner& sound)
        :frames_(frames),sight_(sight),world_(world),states_(states),sound_(sound){
        if(current_)throw std::logic_error("Human sound callback already belongs to an actual world");
        current_=this; //Original callback has no userdata; one process world owns its routing.
    }
    ~GameHumanSoundLoading(){
        for(auto n=registrations_.size();n;)sound_.remove_game_sound_listener(registrations_[--n]);
        if(current_==this)current_=nullptr;
    }
    GameHumanSoundLoading(const GameHumanSoundLoading&)=delete;
    GameHumanSoundLoading& operator=(const GameHumanSoundLoading&)=delete;
    std::int32_t add_listener(Frame frame) {
        registrations_.reserve(registrations_.size()+1);
        const auto slot=sound_.add_game_sound_listener(frame,reinterpret_cast<void*>(&heard),[&](Frame value){
            const auto* data=value?frames_.find_actor_frame_data(value):nullptr;
            return data?reinterpret_cast<Actor>(data->words[0]):nullptr;
        },[](Actor actor){return actor->base().type();});
        if(slot>=0)registrations_.push_back(slot);
        return slot;
    }
    bool remove_listener(std::int32_t slot) {
        const auto removed=sound_.remove_game_sound_listener(slot);
        const auto found=std::find(registrations_.begin(),registrations_.end(),slot);
        if(found!=registrations_.end())registrations_.erase(found);
        return removed;
    }
    void rethrow_callback_failure(){if(auto failure=std::exchange(callback_failure_,{}))std::rethrow_exception(failure);}
    void notify(const Event& event) {
        if(!event.receiver){OutputDebugStringA("EVENT_SOUNT_WITHOUT_HUMAN and FRAME\n");return;}
        const auto* data=frames_.find_actor_frame_data(event.receiver);
        const auto actor=data?reinterpret_cast<Actor>(data->words[0]):nullptr;
        if(!actor) {
            const auto text="EVENT_SOUNT_WITHOUT_HUMAN: "+frames_.frame_name(event.receiver)+"\n";
            OutputDebugStringA(text.c_str());return;
        }
        auto* human=dynamic_cast<Human*>(actor);
        if(!human)throw std::runtime_error("Human sound receiver resolves to a different actual actor family");
        game::notify_human_sound(*human,event,frames_,*this);
    }
    bool human_sound_friendly(Human& human,Actor actor){return sight_.human_sight_friendly(&human,actor);}
    bool human_sound_in_car(Human& human){
        if(!human.field160)return false;
        auto* actor=world_.human_attached_actor(human);
        if(!actor)throw std::runtime_error("Human sound attachment has no actual actor owner");
        return actor->base().type()==4;
    }
    auto human_sound_random(){return world_.human_sound_random();} //SAME original RandFloat stream/precision.
    std::uint32_t human_sound_ticks(){return timeGetTime();}
    std::int32_t human_sound_health_ec(Actor actor) {
        auto* human=dynamic_cast<Human*>(actor);
        if(!human)throw std::runtime_error("Human sound kind9 requires its actual EC health owner");
        const auto bits=human->properties.stats_ec[0];std::int32_t value;std::memcpy(&value,&bits,4);return value;
    }
    void human_sound_state_a8(Human& human,Actor actor){states_.a8(state(human),actor);}
    void human_sound_state_ac(Human& human,Actor actor){states_.ac(state(human),actor);}
    void human_sound_state_b0(Human& human,Actor actor){states_.b0(state(human),actor);}
    template<class Record>void human_sound_state_b4(Human& human,Record& record){states_.b4(state(human),record);}
private:
    static void WINAPI heard(const frontend::HeardSpatialSound* input)noexcept {
        if(!current_||!input)return;
        try {
            const Event event{input->position,static_cast<Frame>(input->listener),input->volume,
                static_cast<std::uint32_t>(input->group),static_cast<Actor>(input->source)};
            current_->notify(event);
        }catch(...){if(!current_->callback_failure_)current_->callback_failure_=std::current_exception();}
    }
    static auto& state(Human& human){
        auto* state=dynamic_cast<typename StateCallbacks::Base*>(human.loading_state);
        if(!state)throw std::runtime_error("Human sound receiver has no actual shared B0 state");
        return *state;
    }
    inline static GameHumanSoundLoading* current_=nullptr;
    GameSceneActors& frames_;GameSightLoading<World>& sight_;World& world_;StateCallbacks& states_;SoundOwner& sound_;
    std::vector<std::int32_t> registrations_;std::exception_ptr callback_failure_;
};
} //namespace hd2::host
#endif
