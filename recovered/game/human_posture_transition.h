#ifndef HD2_GAME_HUMAN_POSTURE_TRANSITION_H
#define HD2_GAME_HUMAN_POSTURE_TRANSITION_H

#include "game/human_animation_tracks.h"
#include "game/human_state_frame_timers.h"
#include <cmath>

namespace hd2::game {

//0076AF70. Room queries can update E0 or queue an action before returning.
//Read the requested posture again after the actual query. Physical99C..9B4
//and geometry62D0/65F0/65F4 belong to their existing shared owners.
template<class Control,class Look,class Human,class Services,class Publish>
void apply_human_posture_transition(Control& control,Look& look,Human& human,
        Services& services,Publish& publish){
    auto& state=control.physical().control_frame_words();
    auto& request=look.frame_words();
    const auto current=[&](unsigned offset){return state.template get<std::int32_t>(offset);};
    const auto posture=[&]{return request.template get<std::uint32_t>(0x4c);};
    const auto duration=[&](float value){
        const auto result=static_cast<long double>(value)/services.human_control_animation_scale();
        if(!std::isfinite(result)||result< -0x1p63L||result>=0x1p63L)return std::int32_t{0}; //Native indefinite LOW DWORD.
        return human_frame_signed_word(static_cast<std::uint32_t>(static_cast<std::int64_t>(result)));
    };
    const auto defer=[&]{request.set(0xc8,request.template get<std::uint32_t>(0xc8)|8u);};
    const auto restore=[&]{look.frame_posture(state.template get<std::uint32_t>(0x7bc),publish,false);defer();};
    const auto primary=[&](std::uint32_t animation){state.set(0x1fc,animation);normalize_human_primary_animation(state,0x1fc);};
    const auto geometry=[&](bool equipment){
        control.geometry().set_posture_transition(posture());
        if(equipment)services.human_presentation_equipment(human).state().words().set(0xcc,std::uint32_t{posture()==2});
    };
    const auto transition=[&](std::int32_t delay,std::uint32_t old){
        state.set(0x99c,1u);state.set(0x9a0,delay);state.set(0x9a4,old);state.set(0x9a8,3u);
    };
    const auto active=current(0x1d8);
    if(active>=8&&active<=14){
        //0075C7E0 is always true for this enclosing native ID range. Its
        //other-ID fallback cannot execute without a change to that pure predicate.
        if(posture()!=0)return;
        primary(45);
        state.set(0x200,state.template get<std::uint32_t>(0x7bc));normalize_human_primary_animation(state,0x1fc);
        look.frame_movement_animation(45,publish,false);
        defer();look.schedule_frame_posture(duration(200.f),posture(),publish);
        return;
    }
    const bool moving=active>=190&&current(0x1e0)!=0;
    switch(current(0x7bc)){
    case 0:
        if(posture()==1){
            (void)control.posture_room_available(look,3u); //0074EF60 result deliberately ignored in this branch.
            geometry(false);transition(duration(400.f),0);
            if(!moving)primary(26);
        }else if(posture()==2){
            if(!control.posture_room_available(look,0u)){restore();return;}
            geometry(true);transition(duration(220.f),0);primary(32);
        }
        break;
    case 1:
        if(posture()==0){
            if(!control.posture_room_available(look,2u)){restore();return;}
            geometry(true);if(!moving)primary(27);
        }else if(posture()==2){
            if(!control.posture_room_available(look,1u)){restore();return;}
            geometry(true);transition(duration(900.f),1);primary(30);
        }
        break;
    case 2:
        if(posture()==0){
            if(!control.posture_room_available(look,4u)){restore();return;}
            geometry(true);
            state.set(0x99c,1u);state.set(0x9a4,2u);state.set(0x9a8,3u);state.set(0x9b4,430u);
            state.set(0x9a0,moving?500u:1000u);primary(33);
        }else if(posture()==1){
            if(!control.posture_room_available(look,5u)){restore();return;}
            geometry(true);state.set(0x9b4,duration(1100.f));transition(duration(1000.f),2);primary(31);
        }
        break;
    default:break;
    }
}

} //namespace hd2::game
#endif
