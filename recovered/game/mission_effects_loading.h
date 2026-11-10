#ifndef HD2_GAME_MISSION_EFFECTS_LOADING_H
#define HD2_GAME_MISSION_EFFECTS_LOADING_H

#include "game/effect_definitions.h"
#include "frontend/pre_mission_data.h"
#include <cstdio>

namespace hd2::game {

template<class Files,class Frames>class ParticleModuleLoading;
//Particle-manager definition cache and0089A8F4 borrowed scene have process
//lifetime.00445980 clears definitions on initialized module17 finish; closing
//the mission-file facade alone does not. No placed instance producer exists.
template<class Frame>struct MissionEffectsProcess {
    EffectDefinitions definitions;Frame scene{};
    Frame particle_scene()const{return particle_scene_;} //Native module17+100C, distinct from0089A8F4.
    bool particles_initialized()const{return particles_initialized_;}
    std::int32_t particle_identity()const{return particle_identity_;}
private:
    template<class Files,class Frames>friend class ParticleModuleLoading;
    //Only cold owners are exposed: active bucket/1014/1034 producers still
    //reject before creating instances. None can reach a successful empty cleanup.
    Frame particle_scene_{};
    std::int32_t particle_identity_=-1; //00443D30; process1044 is initially BSS zero.
    bool particles_initialized_=false;
    const void* particle_world_=nullptr; //Source lifetime guard, not native readiness.
};

//00445D90 over actual selected files,00444140's nonempty definition cache and
//004464A0 version201 reader. The captured missions have empty201 roots. A valid
//populated instance still requires00445AF0's real particle/resource lifetime.
template<class Files,class Frames>class MissionEffectsLoading {
public:
    using Frame=typename Frames::Frame;using Process=MissionEffectsProcess<Frame>;
    MissionEffectsLoading(Files& files,Frames& frames,Process& process):files_(files),frames_(frames),process_(process){}
    ~MissionEffectsLoading(){close();}
    MissionEffectsLoading(const MissionEffectsLoading&)=delete;
    MissionEffectsLoading& operator=(const MissionEffectsLoading&)=delete;
    Frames& frames(){return frames_;}Process& process(){return process_;}
    void publish_scene(){ //006CA780 after collision tree, BEFORE00445D90.
        const auto scene=frames_.world_scene_frame();
        if(!scene||(process_.scene&&process_.scene!=scene))throw std::logic_error("Effects require the SAME containing world scene");
        process_.scene=published_=scene;
    }
    void load(const std::string& resource){
        if(!published_||process_.scene!=published_||frames_.world_scene_frame()!=published_)
            throw std::logic_error("Mission effects require their actual published world scene");
        ensure_definitions();
        const auto path="missions\\"+resource+"\\effects.bin";
        const auto bytes=frontend::pre_mission_detail::read_file(files_,path.c_str());
        if(!bytes){std::fprintf(stderr,"Effects.bin : could not open '%s'\n",path.c_str());return;} //Original diagnostic/return, not required placed-instance success.
        read_mission_effects(SerializedBytes(*bytes),*this);
    }
    void ensure_definitions(){ //00444140, also called by00443F30 before its initialized latch.
        if(process_.definitions.values().empty()){
            const auto definitions=frontend::pre_mission_detail::read_file(files_,"tables\\effects.def");
            if(definitions)(void)process_.definitions.load(SerializedBytes(*definitions));
            else std::fputs("Could not load data from 'tables\\effects.def'\n",stderr);
        }
    }
    Frame find_effect_frame(const std::string& name){return frames_.find_scene_frame(name,0xffff);} //004464A0 scene7C, not a separate model cache.
    void create_placed_effect(std::uint32_t identity,const std::array<std::uint32_t,16>&,Frame){
        const auto& definitions=process_.definitions.values();
        if(identity>=definitions.size()||!definitions[identity].valid())return; //00445AF0 genuine invalid-definition gate.
        throw std::runtime_error("Placed effect requires actual00445AF0 particle/resource instance producer");
    }
    void close()noexcept{if(published_&&process_.scene==published_)process_.scene=Frame{};published_=Frame{};}
private:
    Files& files_;Frames& frames_;Process& process_;Frame published_{};
};
} //namespace hd2::game
#endif
