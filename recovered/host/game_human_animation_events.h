#ifndef HD2_HOST_GAME_HUMAN_ANIMATION_EVENTS_H
#define HD2_HOST_GAME_HUMAN_ANIMATION_EVENTS_H

#include "host/game_actor_services.h"
#include "game/human_animation_events.h"
#include <map>

namespace hd2::host {

//The engine frame+28 callback has no user-data argument. A registry borrows
//actual marker/track lifetimes without overwriting original frame userdata.
//Close tracks/this bridge BEFORE the models, Actor access, Sound and Frames.
template<class Actors,class Sound>
class GameHumanAnimationEvents {
public:
    using Frame=GameActorServices::Frame;
    GameHumanAnimationEvents(GameActorServices& frames,Actors& actors,Sound& sound)
        :frames_(frames),actors_(actors),events_(frames,actors,sound) {}
    ~GameHumanAnimationEvents() {
        for(auto at=bindings_.begin();at!=bindings_.end();) {
            if(at->second.owner!=this){++at;continue;}
            try{frames_.human_animation_callback(at->first,nullptr,0);}
            catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
            at=bindings_.erase(at);
        }
    }
    GameHumanAnimationEvents(const GameHumanAnimationEvents&)=delete;
    GameHumanAnimationEvents& operator=(const GameHumanAnimationEvents&)=delete;
    template<class Tracks>void install_animation_events(Frame marker,Tracks& tracks) {
        const auto found=bindings_.find(marker);
        if(found!=bindings_.end()&&(found->second.owner!=this||found->second.tracks!=&tracks))
            throw std::runtime_error("Human animation marker callback already belongs to another owner");
        const bool added=found==bindings_.end();
        if(added)bindings_.emplace(marker,Binding{this,&tracks});
        try{frames_.human_animation_callback(marker,&invoke,0x1000);}
        catch(...){if(added)bindings_.erase(marker);throw;}
    }
    template<class Tracks>void remove_animation_events(Frame marker,Tracks& tracks) {
        const auto found=bindings_.find(marker);
        if(found!=bindings_.end()&&(found->second.owner!=this||found->second.tracks!=&tracks))
            throw std::runtime_error("Human animation callback cleanup found a different owner");
        frames_.human_animation_callback(marker,nullptr,0);
        if(found!=bindings_.end())bindings_.erase(found);
    }
    void animation_player_check(Frame model){actors_.animation_player_check(model);}
    float animation_time_scale(){return actors_.animation_time_scale();}
    void rethrow_animation_event_failure() const {if(failure_)std::rethrow_exception(failure_);}
private:
    struct Binding {GameHumanAnimationEvents* owner;const void* tracks;};
    static std::int32_t WINAPI invoke(Frame marker,std::uint32_t event,const char* text,std::int32_t time) noexcept {
        const auto found=bindings_.find(marker);
        if(found==bindings_.end())return 0; //Only possible during failed unregister/retirement; failure is already latched.
        auto& owner=*found->second.owner;
        if(!owner.failure_)try{return owner.events_.invoke(marker,event,text,time);}
        catch(...){owner.failure_=std::current_exception();}
        return 0;
    }
    inline static std::map<Frame,Binding> bindings_;
    GameActorServices& frames_;Actors& actors_;
    game::HumanAnimationEvents<GameActorServices,Actors,Sound> events_;
    std::exception_ptr failure_;
};
} //namespace hd2::host
#endif
