#ifndef HD2_GAME_PARTICLE_MODULE_LOADING_H
#define HD2_GAME_PARTICLE_MODULE_LOADING_H
#include "frontend/module_registration.h"
#include "game/mission_effects_loading.h"

namespace hd2::game {
//Actual module17 and SAME process definition/instance/point owners. Positive
//004451B0/00445AF0, update004455B0 and ordered finish use actual scene/pause/light.
template<class Files,class Frames>class ParticleModuleLoading {
public:
    using Effects=MissionEffectsLoading<Files,Frames>;using Process=typename Effects::Process;
    using Frame=typename Frames::Frame;using Instance=typename Process::Instance;using Record=typename Process::Record;
    ParticleModuleLoading(frontend::ClientModuleRegistry& registry,frontend::ClientModuleRegistration& registration,Effects& effects,
            std::function<bool()> paused={},std::function<float(const HumanVector&)> light={})
        :registration_(registry,registration,17,"Particles"),effects_(effects),process_(effects.process()),paused_(std::move(paused)),light_(std::move(light)){
        if(process_.particle_world_||process_.particles_initialized_)throw std::logic_error("Retire the actual particle module before binding another world");
        process_.emit_=[this](std::uint32_t definition,const HumanMatrix& matrix,Frame parent,bool orientation,bool flag){return emit(definition,matrix,parent,orientation,flag);};
        try{process_.place_=[this](std::uint32_t definition,const HumanMatrix& matrix,Frame parent){place(definition,matrix,parent);};}
        catch(...){process_.emit_={};throw;}
        process_.particle_world_=this;
    }
    ~ParticleModuleLoading(){close();}
    ParticleModuleLoading(const ParticleModuleLoading&)=delete;ParticleModuleLoading& operator=(const ParticleModuleLoading&)=delete;
    auto& registration(){return registration_.get();}
    void finish_loading(bool){ //00444110 ignores authority.
        require_open();const auto scene=effects_.frames().world_scene_frame();
        if(!scene||(process_.scene&&process_.scene!=scene))throw std::logic_error("Particles require the SAME containing world scene");
        process_.particle_scene_=scene;
    }
    void activate(bool){ //00443F30: definitions, reverse256 cached buckets, then latch; placed owners persist.
        require_open();if(process_.particles_initialized_)return;
        process_.particle_identity_=-1;effects_.ensure_definitions();
        frontend::CleanupErrors errors;clear_pools(errors);errors.rethrow();process_.particles_initialized_=true;
    }
    std::int32_t emit(std::uint32_t index,const HumanMatrix& matrix,Frame parent,bool orientation,bool flag){ //004451B0.
        require_open();const auto* definition=valid_definition(index);if(!definition)return -1;
        require_scene();if(index>=process_.pools_.size())throw std::runtime_error("Particle definition exceeds actual256 reuse buckets");
        auto& pool=process_.pools_[index];std::unique_ptr<Instance> instance;
        auto light=[this](const HumanVector& position){return lighting(position);};
        if(!pool.empty()){instance=std::move(pool.back());pool.pop_back();instance->reuse(process_.particle_scene_,matrix,parent,orientation,light);}
        else{instance=std::make_unique<Instance>(effects_.frames(),process_.points_,process_.scratch_);
            instance->initialize(process_.particle_scene_,matrix,parent,*definition,orientation,light);}
        const auto identity=particle_integer(static_cast<std::uint32_t>(process_.particle_identity_)+1u);process_.particle_identity_=identity;
        process_.active_.push_back(Record{std::move(instance),identity,index,flag});return identity;
    }
    void place(std::uint32_t index,const HumanMatrix& matrix,Frame parent){ //00445AF0, separate1034 owner, id-1 and orientation1.
        require_open();const auto* definition=valid_definition(index);if(!definition)return;
        require_scene();auto instance=std::make_unique<Instance>(effects_.frames(),process_.points_,process_.scratch_);
        auto light=[this](const HumanVector& position){return lighting(position);};
        instance->initialize(process_.particle_scene_,matrix,parent,*definition,true,light);
        process_.placed_.push_back(Record{std::move(instance),-1,index,false}); //Native final record byte is unwritten; source guard only.
    }
    void update(std::uint32_t elapsed){ //004455B0: reread game64 for EACH row; type2C/2D still update during pause.
        require_open();if(!process_.particle_scene_)return;
        for(std::size_t n=0;n<process_.placed_.size();++n){auto& record=process_.placed_[n];
            if(paused())record.instance->present();else (void)record.instance->update(elapsed);}
        if(process_.particle_identity_==-1)return;
        for(std::size_t n=0;n<process_.active_.size();){auto& record=process_.active_[n];
            if(paused()&&record.definition!=0x2cu&&record.definition!=0x2du){record.instance->present();++n;continue;}
            if(record.instance->update(elapsed)){++n;continue;}
            record.instance->retire_to_pool();
            process_.pools_.at(record.definition).push_back(std::move(record.instance));
            process_.active_.erase(process_.active_.begin()+static_cast<std::ptrdiff_t>(n)); //Process shifted row next.
        }
    }
    void finish(bool){ //00443FE0: active ascending, buckets reverse, definitions, id-1, placed ascending, latch.
        require_open();if(!process_.particles_initialized_)return;retire_all(true);
    }
    void close()noexcept{
        if(process_.particle_world_!=this)return;
        try{retire_all(process_.particles_initialized_);}catch(...){effects_.frames().record_actor_cleanup_failure(std::current_exception());}
        process_.emit_={};process_.place_={};process_.particle_scene_=Frame{};process_.particle_world_=nullptr;
    }
    void discard_scene_borrower(){require_open();
        if(!process_.active_.empty()||!process_.placed_.empty())throw std::logic_error("Particle scene cannot retire before its actual instances");
        for(const auto& pool:process_.pools_)if(!pool.empty())throw std::logic_error("Particle scene cannot retire before cached visual owners");
        process_.particle_scene_=Frame{};
    }
private:
    const EffectDefinition* valid_definition(std::uint32_t index)const{const auto& definitions=process_.definitions.values();
        return index<definitions.size()&&definitions[index].valid()?&definitions[index]:nullptr;}
    bool paused(){if(!paused_)throw std::logic_error("Positive particle update requires actual game64 pause borrower");return paused_();}
    float lighting(const HumanVector& position){if(!light_)throw std::logic_error("Particle lighting requires actual0069E9D0 World queries");return light_(position);}
    void require_open()const{if(process_.particle_world_!=this)throw std::logic_error("Particle module world has been retired");}
    void require_scene()const{if(!process_.particle_scene_)throw std::logic_error("Particle creation requires actual module17 scene100C");}
    void clear_pools(frontend::CleanupErrors& errors){for(auto n=process_.pools_.size();n;){auto& pool=process_.pools_[--n];
        while(!pool.empty()){auto instance=std::move(pool.back());pool.pop_back();if(instance)errors.attempt([&]{instance->close();});}}}
    void retire_all(bool definitions){
        frontend::CleanupErrors errors;
        for(std::size_t n=0;n<process_.active_.size();++n)if(auto& instance=process_.active_[n].instance){errors.attempt([&]{instance->close();});instance.reset();}
        process_.active_.clear();clear_pools(errors);
        if(definitions){process_.definitions.clear();process_.particle_identity_=-1;}
        for(std::size_t n=0;n<process_.placed_.size();++n)if(auto& instance=process_.placed_[n].instance){errors.attempt([&]{instance->close();});instance.reset();}
        process_.placed_.clear();process_.particles_initialized_=false;errors.rethrow();
    }
    frontend::ClientModuleBinding registration_;Effects& effects_;Process& process_;
    std::function<bool()> paused_;std::function<float(const HumanVector&)> light_;
};
} //namespace hd2::game
#endif
