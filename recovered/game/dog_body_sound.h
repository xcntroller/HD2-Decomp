#ifndef HD2_GAME_DOG_BODY_SOUND_H
#define HD2_GAME_DOG_BODY_SOUND_H
#include "frontend/spatial_sound.h"
namespace hd2::game {
//005C5BF0 and005C5030's bark branch. Actor position is snapped BEFORE the
//actual Body3EC world-matrix callback; model-relative pointer is reloaded after.
template<class Dog,class Frames,class Sound>
std::int32_t emit_dog_body_voice(Dog& dog,Frames& frames,Sound& sound,std::uint32_t flags,std::int32_t entry){
    const auto position=dog.base().world_position();const auto marker=dog.body.damage_bones[1];
    if(!marker)throw std::runtime_error("Dog voice requires actual Body3EC skeletal frame");
    const auto matrix=frames.actor_world_matrix(marker);frontend::SpatialSoundRequest request;
    for(unsigned n=0;n!=3;++n)request.position[n]=static_cast<float>(static_cast<long double>(matrix[12+n])-position[n]);
    request.flags=flags;request.relative=dog.base().frame();request.source=nullptr;request.group=11;request.entry=entry;
    return sound.emit(request);
}
}
#endif
