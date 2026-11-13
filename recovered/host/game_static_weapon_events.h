#ifndef HD2_HOST_GAME_STATIC_WEAPON_EVENTS_H
#define HD2_HOST_GAME_STATIC_WEAPON_EVENTS_H
#include "host/game_actor_services.h"
#include "game/static_weapon_loading_lifetime.h"
#include "game/mounted_weapon_control.h"
#include <map>
namespace hd2::host {
//004C1990(frame,event,text,time): original frameEC+1C borrows its actual control.
//Retire hooks/backlinks before control deletion. Callback failure escapes only
//through rethrow_failure outside the engine; no exception crosses stdcall ABI.
class GameStaticWeaponEvents {
public:
    using Frame=GameActorServices::Frame;using Control=game::MountedWeaponControlLifetime;
    explicit GameStaticWeaponEvents(GameActorServices& frames):frames_(frames){}
    ~GameStaticWeaponEvents(){clear();}
    void install(Frame frame,Control& control) {
        if(!frame)throw std::runtime_error("StaticWeapon marker requires its actual barrel frame");
        const auto found=bindings_.find(frame);
        if(found!=bindings_.end()&&(found->second.owner!=this||found->second.control!=&control))throw std::runtime_error("StaticWeapon marker frame has another control owner");
        if(found==bindings_.end())bindings_.emplace(frame,Binding{this,&control});
        auto& data=frames_.actor_frame_data(frame);
        if(data.words[7]&&data.words[7]!=reinterpret_cast<std::uintptr_t>(&control))throw std::runtime_error("StaticWeapon frameEC control backlink is already occupied");
        data.words[7]=reinterpret_cast<std::uintptr_t>(&control);frames_.human_animation_callback(frame,&invoke,0x1000);
    }
    void remove(Frame frame,Control& control) {
        auto found=bindings_.find(frame);if(found==bindings_.end())return;
        if(found->second.owner!=this||found->second.control!=&control)throw std::runtime_error("StaticWeapon marker owner mismatch");
        bindings_.erase(found);frontend::CleanupErrors errors;
        errors.attempt([&]{if(const auto* data=frames_.find_actor_frame_data(frame))if(data->words[7]==reinterpret_cast<std::uintptr_t>(&control))frames_.actor_frame_data(frame).words[7]=0;});
        errors.attempt([&]{frames_.human_animation_callback(frame,nullptr,0);});frames_.remove_unused_actor_frame_data(frame);errors.rethrow();
    }
    void clear()noexcept {
        for(auto found=bindings_.begin();found!=bindings_.end();) {
            if(found->second.owner!=this){++found;continue;}
            const auto frame=found->first;auto* control=found->second.control;++found;
            try{remove(frame,*control);}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
        }
    }
    void rethrow_failure(){if(auto failure=std::exchange(failure_,{}))std::rethrow_exception(failure);}
private:
    struct Binding{GameStaticWeaponEvents* owner;Control* control;};inline static std::map<Frame,Binding> bindings_;
    static std::int32_t WINAPI invoke(Frame frame,std::uint32_t,const char* text,std::int32_t)noexcept {
        const auto found=bindings_.find(frame);if(found==bindings_.end()||!text||*text!='S')return 0;
        auto& binding=found->second;try{binding.control->fire_marker(frame);}catch(...){if(!binding.owner->failure_)binding.owner->failure_=std::current_exception();}return 0;
    }
    GameActorServices& frames_;std::exception_ptr failure_;
};
}
#endif
