#ifndef HD2_GAME_PARTICLE_MODULE_LOADING_H
#define HD2_GAME_PARTICLE_MODULE_LOADING_H

#include "frontend/module_registration.h"
#include "game/mission_effects_loading.h"

namespace hd2::game {

//Module17's reached cold lifetime, sharing the actual00444140 definitions with
//the00445D90 file loader. Active256 buckets/1014/1034 have no source producer:
//00445AF0 rejects valid placed instances before allocation. This supplies no
//positive particle, texture, animation, update or render cleanup substitute.
template<class Files,class Frames>class ParticleModuleLoading {
public:
    using Effects=MissionEffectsLoading<Files,Frames>;
    using Process=typename Effects::Process;using Frame=typename Frames::Frame;
    ParticleModuleLoading(frontend::ClientModuleRegistry& registry,frontend::ClientModuleRegistration& registration,Effects& effects)
        :registration_(registry,registration,17,"Particles"),effects_(effects),process_(effects.process()){
        if(process_.particle_world_||process_.particles_initialized_)
            throw std::logic_error("Retire the actual particle module before binding another world");
        process_.particle_world_=this;
    }
    ~ParticleModuleLoading(){close();}
    ParticleModuleLoading(const ParticleModuleLoading&)=delete;
    ParticleModuleLoading& operator=(const ParticleModuleLoading&)=delete;
    auto& registration(){return registration_.get();}
    void update(std::uint32_t){ //004455B0, SAME genuinely cold1014/1034 lists; no active-particle producer/API.
        require_open();
    }
    void finish_loading(bool){ //00444110 ignores authority and always borrows game18.
        require_open();const auto scene=effects_.frames().world_scene_frame();
        if(!scene||(process_.scene&&process_.scene!=scene))
            throw std::logic_error("Particles require the SAME containing world scene");
        process_.particle_scene_=scene;
    }
    void activate(bool){ //00443F30 ignores authority; second pass sees the shared latch.
        require_open();
        if(process_.particles_initialized_)return;
        process_.particle_identity_=-1;
        effects_.ensure_definitions();
        //Native reverse256-bucket retirement is genuinely empty with this
        //unpopulatable cold source owner. Definition failure may still return.
        process_.particles_initialized_=true;
    }
    void finish(bool) { //00443FE0, with genuinely empty1014/buckets/1034.
        require_open();if(!process_.particles_initialized_)return;
        process_.definitions.clear(); //00445980, between bucket and1034 retirement.
        process_.particle_identity_=-1;
        process_.particles_initialized_=false;
    }
    void close()noexcept{
        if(process_.particle_world_!=this)return;
        if(process_.particles_initialized_){process_.definitions.clear();process_.particle_identity_=-1;process_.particles_initialized_=false;}
        //Typed borrower invalidation before scene destruction; native100C is
        //otherwise overwritten at the next00444110, not a retained reference.
        process_.particle_scene_=Frame{};process_.particle_world_=nullptr;
    }
private:
    void require_open()const{if(process_.particle_world_!=this)throw std::logic_error("Particle module world has been retired");}
    frontend::ClientModuleBinding registration_;Effects& effects_;Process& process_;
};
} //namespace hd2::game
#endif
