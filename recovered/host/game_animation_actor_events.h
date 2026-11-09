#ifndef HD2_HOST_GAME_ANIMATION_ACTOR_EVENTS_H
#define HD2_HOST_GAME_ANIMATION_ACTOR_EVENTS_H
#include "host/game_actor_services.h"
#include "game/animation_actor_events.h"
#include "frontend/cleanup.h"
#include <map>
#include <algorithm>
#include <utility>
namespace hd2::host {
//005B4610 real frame+28 callback. Group bindings own no engine frames; retire
//them before unlink/release of the helper model and before the actual SOUND.
template<class Sound>class GameAnimationActorEvents {
public:
    using Frame=GameActorServices::Frame;
    GameAnimationActorEvents(GameActorServices& frames,Sound& sound):frames_(frames),sound_(sound){}
    ~GameAnimationActorEvents(){clear();}
    GameAnimationActorEvents(const GameAnimationActorEvents&)=delete;GameAnimationActorEvents& operator=(const GameAnimationActorEvents&)=delete;
    void install(Frame frame,Frame group) {
        if(!frame||!group)throw std::runtime_error("Animation event callback requires actual model/frame borrowers");
        const auto at=bindings_.find(frame);
        if(at!=bindings_.end()&&(at->second.owner!=this||at->second.group!=group))throw std::runtime_error("Animation event frame already belongs to another actual owner");
        if(at==bindings_.end())bindings_.emplace(frame,Binding{this,group});
        //Keep the binding on an engine call failure: a partially installed hook
        //must be explicitly removed by this group's actual loading rollback.
        frames_.human_animation_callback(frame,&invoke,0x1000);
    }
    void remove(Frame group) {
        frontend::CleanupErrors errors;
        for(auto at=bindings_.begin();at!=bindings_.end();) {
            if(at->second.owner!=this||at->second.group!=group){++at;continue;}
            const auto frame=at->first;at=bindings_.erase(at);
            errors.attempt([&]{frames_.human_animation_callback(frame,nullptr,0);});
        }
        errors.rethrow();
    }
    void clear()noexcept {
        while(true) {
            const auto at=std::find_if(bindings_.begin(),bindings_.end(),[&](const auto& value){return value.second.owner==this;});
            if(at==bindings_.end())return;
            try{remove(at->second.group);}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
        }
    }
    void rethrow_failure(){if(auto failure=std::exchange(failure_,{}))std::rethrow_exception(failure);}
private:
    struct Binding{GameAnimationActorEvents* owner;Frame group;};
    static std::int32_t WINAPI invoke(Frame frame,std::uint32_t event,const char* text,std::int32_t)noexcept {
        const auto at=bindings_.find(frame);if(at==bindings_.end())return 0;
        auto& owner=*at->second.owner;
        try{return game::animation_actor_event(frame,event,text,owner.sound_);}catch(...){if(!owner.failure_)owner.failure_=std::current_exception();}
        return 0;
    }
    inline static std::map<Frame,Binding> bindings_;
    GameActorServices& frames_;Sound& sound_;std::exception_ptr failure_;
};
} //namespace hd2::host
#endif
