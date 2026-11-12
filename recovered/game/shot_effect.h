#ifndef HD2_GAME_SHOT_EFFECT_H
#define HD2_GAME_SHOT_EFFECT_H
#include "game/mission_particle_emission.h"
#include "frontend/pre_mission_data.h"
namespace hd2::game {
struct MineEffectDefinition {std::int32_t sound;std::uint32_t particle;};
inline MineEffectDefinition mine_effect_definition(const frontend::PreMissionItem& definition){
    const auto& bytes=definition.payloads[0].bytes;
    //Serialized payload begins at native+04: native14/1C are file10/18.
    if(bytes.size()<0x1c)throw std::runtime_error("Mine effect requires actual native48 item payload");
    using Read=frontend::pre_mission_detail::Bytes;
    return {Read::integer(Read::word(bytes.data()+0x10)),Read::word(bytes.data()+0x18)};
}
//00451A80 ->00451DB0's actual material=-1 branch, used by Mine005FEF50.
//Reuse SAME ShotMgr D0 sound request and module17; other material branches are
//not admitted through this scoped producer and receive no implementation credit.
template<class State,class Sound,class Particles,class Frames>
void emit_shot_effect_without_material(State& state,Sound& sound,Particles& particles,Frames& frames,
        const HumanVector& position,const HumanVector& direction,std::uint32_t particle,std::int32_t entry){
    auto& request=state.hit_sound;request.group=6;state.hit_sound_position=position;
    request.position={{position[0],position[1],position[2]}};request.flags=1;request.entry=entry;
    request.source=nullptr;request.relative=nullptr;(void)sound.emit(request);
    //Preserve all remaining reused D0 fields, including filename/ranges/voice_group.
    (void)emit_mission_particle(particles,frames,particle,position,direction,typename Frames::Frame{},true,false);
}
}
#endif
