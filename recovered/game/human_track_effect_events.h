#ifndef HD2_GAME_HUMAN_TRACK_EFFECT_EVENTS_H
#define HD2_GAME_HUMAN_TRACK_EFFECT_EVENTS_H
#include "game/human_ai_shot_direction.h"
#include "frontend/spatial_sound.h"
namespace hd2::game {
template<class Control,class Services,class Frames>
void apply_human_track_particle(Control& control,Services& services,Frames& frames){ //00758D20 event18.
    const auto bone=control.physical().bones().at(4); //Native840, original810/stride0C.
    if(!bone)throw std::runtime_error("Human particle track requires its actual bone4");
    const auto matrix=frames.actor_world_matrix(bone);
    HumanVector direction{{matrix[8],matrix[9],matrix[10],matrix[11]}};
    const auto square=(static_cast<long double>(direction[1])*direction[1]+
        static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[0])*direction[0];
    normalize_human_ai_shot(direction,square);
    HumanVector position{{matrix[12],matrix[13],matrix[14],matrix[15]}};
    constexpr float distance=.15f,height=.04f; //Original80FF04/810734, exact FLOAT bytes.
    const float y=direction[1]*distance,z=direction[2]*distance;
    position[0]=static_cast<float>(static_cast<long double>(direction[0])*distance+position[0]);
    position[1]=static_cast<float>((static_cast<long double>(position[1])+height)+y);
    position[2]=z+position[2];
    (void)services.emit_human_particle(33,position,direction,typename Frames::Frame{},true,false);
}
//Return false ONLY for the native whole-dispatch return. Sound failure remains
//an exception; original request and original bone/actor identities are retained.
template<class Control,class Human,class Services>
bool apply_human_track_ladder_sound(Control& control,Human& human,Services& services){ //00758D20 event19.
    const auto actor=control.physical().control_action_actor();
    if(!actor)return false;
    const auto group=services.human_track_ladder_sound_group(actor);
    if(group==-1)return false;
    frontend::SpatialSoundRequest request;request.position={{0,-.4f,0}};
    request.flags=1;request.relative=control.physical().bones().at(16);request.source=&human;
    request.entry=0;request.group=group;request.voice_group=3;
    (void)services.emit_human_equipment_sound(request);return true;
}
template<class Control,class Human,class Services>
void apply_human_track_presentation_sound(Control& control,Human& human,Services& services){ //00758D20 event24.
    if(!(services.mode_flags()&2u))return;
    if(services.human_view_offset_enabled()&&services.current_human_frame_player()==&human)return;
    frontend::SpatialSoundRequest request;request.position={{0,.1f,0}};
    request.flags=1;request.relative=control.physical().bones().at(4);request.source=&human;
    request.entry=1;request.group=46;
    (void)services.emit_human_equipment_sound(request);
}
}
#endif
