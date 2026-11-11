#ifndef HD2_GAME_HUMAN_LATERAL_STRIDE_H
#define HD2_GAME_HUMAN_LATERAL_STRIDE_H

#include "game/human_ai_shot_direction.h"
#include "game/human_animation_tracks.h"
#include "game/human_state_frame_timers.h"

namespace hd2::game {

//0074A290. Queued1FC descriptor is compared with the active1D8 descriptor
//before touching actual posture colliders, the native840 bone or exclusions.
template<class Control,class Look,class Human,class Frames,class Sweep>
void refine_human_lateral_stride(Control& control,const Look& look,Human& human,Frames& frames,Sweep& sweep){
    auto& physical=control.physical();auto& state=physical.control_frame_words();
    const auto animation=state.template get<std::int32_t>(0x1fc);
    if(animation<162||animation>165)return;
    if(state.template get<std::uint32_t>(0x1fc)==state.template get<std::uint32_t>(0x1d8)&&
       state.template get<std::uint32_t>(0x200)==state.template get<std::uint32_t>(0x1dc)&&
       state.template get<std::uint32_t>(0x204)==state.template get<std::uint32_t>(0x1e0))return;
    physical.rebind_frame_posture(look.state().template get<std::uint32_t>(0x4c));
    const auto bone=physical.bones().at(4); //Native810+4*0C=840, actual bone/flag records.
    if(!bone)throw std::runtime_error("Human lateral stride requires actual840 bone");
    const auto matrix=frames.actor_world_matrix(bone);
    HumanVector origin{{matrix[12],static_cast<float>(static_cast<long double>(matrix[13])+.05f),matrix[14],0}};
    const auto& forward=human.base().world_direction();
    HumanVector displacement{{forward[0],-.2f,forward[2],forward[3]}};
    const auto square=static_cast<long double>(forward[2])*forward[2]+static_cast<long double>(forward[0])*forward[0]+.0400000028312206268310546875f;
    normalize_human_ai_shot(displacement,square);
    for(unsigned axis=0;axis!=3;++axis)displacement[axis]=static_cast<float>(static_cast<long double>(displacement[axis])*1.4f);
    const float angle=animation==162||animation==164?1.57079637050628662109375f:-1.57079637050628662109375f;
    displacement=frames.car_transform(displacement,frames.human_axis_rotation({{0,1,0,0}},angle)); //00788490.
    const auto posture=state.template get<std::int32_t>(0x200);
    if(posture>=0&&posture<=2){
        constexpr std::array<float,3> scales{{.5f,.45f,.3f}};
        for(unsigned axis=0;axis!=3;++axis)displacement[axis]=static_cast<float>(static_cast<long double>(displacement[axis])*scales[posture]);
    }
    const auto hit=[&]{
        physical.exclude_query(false);
        using Physical=std::remove_reference_t<decltype(physical)>;
        struct Restore {Physical& physical;~Restore(){physical.restore_query(false);}} restore{physical};
        return sweep.sweep(origin,displacement,.2f,0xc8002u,0x230000u);
    }();
    if(!hit)return;
    const auto length=[](float squared){
        std::uint32_t bits;std::memcpy(&bits,&squared,4);
        if(human_frame_signed_word(bits)<0x322bcc77)return 0.L;
        return std::abs(static_cast<long double>(squared)-1)<1.e-8f?1.L:std::sqrt(static_cast<long double>(squared));
    };
    const float span=static_cast<float>(length(static_cast<float>((static_cast<long double>(displacement[2])*displacement[2]+
        static_cast<long double>(displacement[0])*displacement[0])+static_cast<long double>(displacement[1])*displacement[1])));
    const auto x=static_cast<long double>(origin[0])-hit->position[0];
    const auto y=static_cast<long double>(origin[1])-hit->position[1];
    const auto z=static_cast<long double>(origin[2])-hit->position[2];
    const auto travel=length(static_cast<float>((z*z+y*y)+x*x));
    if(span!=0)control.action().lateral_stride_limit(static_cast<float>(travel/(static_cast<long double>(span)+span)));
    else {state.set(0x1fc,206u);normalize_human_primary_animation(state,0x1fc);}
}

} //namespace hd2::game
#endif
