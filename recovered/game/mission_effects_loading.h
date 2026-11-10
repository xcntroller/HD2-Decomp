#ifndef HD2_GAME_MISSION_EFFECTS_LOADING_H
#define HD2_GAME_MISSION_EFFECTS_LOADING_H

#include "game/effect_definitions.h"
#include "frontend/pre_mission_data.h"
#include <cstdio>

namespace hd2::game {

//Particle-manager definition cache and0089A8F4 borrowed scene have process
//lifetime. No placed instance, texture or animation producer is invented here.
template<class Frame>struct MissionEffectsProcess {
    EffectDefinitions definitions;Frame scene{};
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
    void publish_scene(){ //006CA780 after collision tree, BEFORE00445D90.
        const auto scene=frames_.world_scene_frame();
        if(!scene||(process_.scene&&process_.scene!=scene))throw std::logic_error("Effects require the SAME containing world scene");
        process_.scene=published_=scene;
    }
    void load(const std::string& resource){
        if(!published_||process_.scene!=published_||frames_.world_scene_frame()!=published_)
            throw std::logic_error("Mission effects require their actual published world scene");
        if(process_.definitions.values().empty()){
            const auto definitions=frontend::pre_mission_detail::read_file(files_,"tables\\effects.def");
            if(definitions)(void)process_.definitions.load(SerializedBytes(*definitions));
            else std::fputs("Could not load data from 'tables\\effects.def'\n",stderr);
        }
        const auto path="missions\\"+resource+"\\effects.bin";
        const auto bytes=frontend::pre_mission_detail::read_file(files_,path.c_str());
        if(!bytes){std::fprintf(stderr,"Effects.bin : could not open '%s'\n",path.c_str());return;} //Original diagnostic/return, not required placed-instance success.
        read_mission_effects(SerializedBytes(*bytes),*this);
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
