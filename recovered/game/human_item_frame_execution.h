#ifndef HD2_GAME_HUMAN_ITEM_FRAME_EXECUTION_H
#define HD2_GAME_HUMAN_ITEM_FRAME_EXECUTION_H

#include "game/human_joint_owners.h"
#include <cmath>

namespace hd2::game {
//0058B6C0. Fire is the REQUIRED actual item virtual24/28 receiver, not a pose
//success callback. This helper returns its native result unchanged. An early
//remote kind1 return leaves BOTH output vectors untouched, as in the original.
template<class State,class Item,class Queries,class Fire>
std::uint32_t execute_human_item_frame(State& state,Item* item,bool primary,std::uint32_t extra,
        HumanVector& position,HumanVector& direction,Queries& queries,Fire& fire){
    auto* human=state.human();if(!human)throw std::logic_error("Item execution requires actual bound Human B0+44");
    if((queries.mode_flags()&7u)!=7u&&!queries.human_is_player(*human)&&human->properties.field19c==1)return 0;
    if(!queries.human_is_player(*human)){
        direction=state.words().template get<HumanVector>(0x10); //RAW10, not derived virtual4C AI aim.
        if(item){
            auto frame=item->shot_frame();
            if(!frame)frame=item->model();else frame=item->shot_frame(); //Native preflight then cached getter.
            position=queries.sight_position(frame);
        }else position=queries.sight_position(human->head_marker);
    }else{
        direction=queries.human_projectile_direction();
        if(queries.human_view_offset_enabled())position=queries.human_projectile_position();
        else if(item){
            auto frame=item->shot_frame();
            if(!frame)frame=item->model();else frame=item->shot_frame();
            position=queries.sight_position(frame);
        }else position=queries.sight_position(human->head_marker);
    }
    //00810174=-.9,0080F5C8=.7,0080F5CC=.3, all FLOAT. Native
    //stores the adjusted Y and the squared length before normalization.
    if(queries.human_is_player(*human)&&item&&item->definition().kind()==9&&direction[1]>-.9f&&direction[1]<.7f){
        const auto y=static_cast<long double>(direction[1])+.3f;
        direction[1]=static_cast<float>(y);if(y>.7f)direction[1]=.7f;
        const auto x=static_cast<long double>(direction[0]),dy=static_cast<long double>(direction[1]),z=static_cast<long double>(direction[2]);
        const float length=static_cast<float>((x*x+z*z)+dy*dy);
        constexpr float epsilon=9.99999993922529e-09f;
        if(!(std::abs(static_cast<long double>(length)-1)<epsilon)){
            if(!(length<epsilon)){
                const auto scale=1/std::sqrt(static_cast<long double>(length));
                for(unsigned axis=0;axis!=3;++axis)direction[axis]=static_cast<float>(direction[axis]*scale);
            }else{
                const auto axis=direction[0]!=0?0u:direction[2]!=0?2u:1u;
                direction[axis]=direction[axis]<0?-1.f:1.f;
            }
        }
    }
    if(!item)throw std::runtime_error("Item execution reached native NULL item virtual24/28; no successful fire receiver");
    return fire(*item,*human,position,direction,extra,primary);
}
} //namespace hd2::game
#endif
