#ifndef HD2_GAME_MISSION_EFFECTS_LOADING_H
#define HD2_GAME_MISSION_EFFECTS_LOADING_H

#include "game/effect_definitions.h"
#include "frontend/pre_mission_data.h"
#include "game/mission_particle_instance.h"
#include <functional>
#include <cstdio>

namespace hd2::game {

template<class Files,class Frames>class ParticleModuleLoading;
//SAME process00443D30 definition/instance state and008998C4 point pool. Cached
//instances borrow the current Frames only until module17 finish/close. The
//containing world must retire this module before its scene/services unbind.
template<class Frames>struct MissionEffectsProcess {
    using Frame=typename Frames::Frame;using Instance=MissionParticleInstance<Frames>;
    struct Record {std::unique_ptr<Instance> instance;std::int32_t identity=-1;std::uint32_t definition=0;bool flag=false;};
    EffectDefinitions definitions;Frame scene{};
    Frame particle_scene()const{return particle_scene_;}
    bool particles_initialized()const{return particles_initialized_;}
    std::int32_t particle_identity()const{return particle_identity_;}
    std::int32_t emit(std::uint32_t definition,const HumanMatrix& transform,Frame parent,bool orientation,bool flag){
        if(!particle_world_||!emit_)throw std::logic_error("Particles require the actual live module17 producer");
        return emit_(definition,transform,parent,orientation,flag);
    }
    void place(std::uint32_t definition,const HumanMatrix& transform,Frame parent){
        if(!particle_world_||!place_)throw std::logic_error("Placed particles require actual module17 ownership");
        place_(definition,transform,parent);
    }
    Record* particle_record(std::int32_t identity,std::int32_t& index){ //00445740, signed native binary lookup.
        if(active_.size()>0x7fffffffu)throw std::length_error("Particle identity vector exceeds native signed range");
        std::int32_t low=0,high=static_cast<std::int32_t>(active_.size())-1,span=high+1;
        while(low<=high){
            const auto half=span/2;
            if(!half){if(span&&active_[static_cast<std::size_t>(low)].identity==identity){index=low;return &active_[static_cast<std::size_t>(low)];}return nullptr;}
            const bool odd=(span&1)!=0;const auto middle=low+half-(odd?0:1);
            auto& record=active_[static_cast<std::size_t>(middle)];
            if(record.identity==identity){index=middle;return &record;}
            if(record.identity>identity){high=middle-1;span=odd?half:half-1;}
            else{low=middle+1;span=half;}
        }
        return nullptr; //Native out index remains unchanged on a miss.
    }
    void stop(std::int32_t identity){ //00445800, delete actual instance BEFORE stable vector erase; no reuse pool.
        std::int32_t index=0;auto* record=particle_record(identity,index);if(!record)return;
        if(record->instance){record->instance->close();record->instance.reset();}
        active_.erase(active_.begin()+index);
    }
    std::size_t active_particles()const{return active_.size();}std::size_t placed_particles()const{return placed_.size();}
    const auto& point_pool()const{return points_;}
private:
    template<class Files,class Backend>friend class ParticleModuleLoading;
    //Declare the shared point pool BEFORE every possible instance owner.
    MissionParticlePointPool points_;MissionParticleScratch scratch_;
    std::array<std::vector<std::unique_ptr<Instance>>,256> pools_;
    std::vector<Record> active_,placed_; //Native1014/1034, ordered16-byte records with typed ownership.
    Frame particle_scene_{};
    std::int32_t particle_identity_=-1;
    bool particles_initialized_=false;const void* particle_world_=nullptr;
    std::function<std::int32_t(std::uint32_t,const HumanMatrix&,Frame,bool,bool)> emit_;
    std::function<void(std::uint32_t,const HumanMatrix&,Frame)> place_;
};

//00445D90 over actual selected files,00444140's nonempty definition cache and
//004464A0 version201 reader. The captured missions have empty201 roots. A valid
//populated instance still requires00445AF0's real particle/resource lifetime.
template<class Files,class Frames>class MissionEffectsLoading {
public:
    using Frame=typename Frames::Frame;using Process=MissionEffectsProcess<Frames>;
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
    void create_placed_effect(std::uint32_t identity,const std::array<std::uint32_t,16>& words,Frame parent){
        const auto& definitions=process_.definitions.values();
        if(identity>=definitions.size()||!definitions[identity].valid())return; //00445AF0 genuine invalid-definition gate.
        HumanMatrix transform;std::memcpy(transform.data(),words.data(),sizeof transform);
        process_.place(identity,transform,parent);
    }
    void close()noexcept{if(published_&&process_.scene==published_)process_.scene=Frame{};published_=Frame{};}
private:
    Files& files_;Frames& frames_;Process& process_;Frame published_{};
};
} //namespace hd2::game
#endif
