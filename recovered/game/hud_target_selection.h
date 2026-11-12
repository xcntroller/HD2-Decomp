#ifndef HD2_GAME_HUD_TARGET_SELECTION_H
#define HD2_GAME_HUD_TARGET_SELECTION_H
#include "game/hud_roll_policy.h"
#include "game/human_hud_loading.h"
#include <array>
#include <cmath>
namespace hd2::game {
inline float hud_target_distance(const std::array<float,4>& point,const std::array<float,4>& head){
    const auto x=static_cast<long double>(point[0])-head[0],y=static_cast<long double>(point[1])-head[1],z=static_cast<long double>(point[2])-head[2];
    const float square=static_cast<float>((z*z+y*y)+x*x);std::int32_t bits;std::memcpy(&bits,&square,4);
    if(bits<0x322bcc77)return 0;if(std::abs(static_cast<long double>(square)-1.f)<9.99999993922529e-09f)return 1;return std::sqrt(square);
}
template<class Pair,class Queries>void advance_hud_target_selection(std::uint32_t& accumulated,std::uint32_t& misses,Pair& pair,std::uint32_t elapsed,Queries& queries){ //0049D6E0.
    using Actor=typename Queries::Actor;queries.selection_visibility();if(queries.selection_command_visible())return;
    accumulated+=elapsed;if(queries.offline()&&human_hud_signed(accumulated)<100)return;accumulated=0;
    auto* human=queries.current_player();if(!human)throw std::logic_error("HUD selection requires actual current Human");
    auto direction=queries.selection_direction();auto origin=queries.selection_origin(*human);
    if(queries.attachment(*human))for(unsigned axis=0;axis!=3;++axis)origin[axis]=static_cast<float>(static_cast<long double>(direction[axis])*1.5f+origin[axis]);
    const auto range=queries.selection_range();for(unsigned axis=0;axis!=3;++axis)direction[axis]=static_cast<float>(static_cast<long double>(direction[axis])*range);
    const auto ray=[&](std::uint32_t flags){queries.exclude_signal_human(*human);decltype(queries.selection_ray(origin,direction,flags)) hit;
        try{hit=queries.selection_ray(origin,direction,flags);}catch(...){queries.restore_signal_human(*human);throw;}queries.restore_signal_human(*human);return hit;};
    auto first=ray(0x210000u);Actor* target=first&&(first->flags&0x40u)?first->actor:nullptr;
    const auto head=queries.selection_head(*human);float distance=0;
    if(!target){if(pair[0]){++misses;if(human_hud_signed(misses)<5)return;}}
    else{distance=hud_target_distance(first->position,head);misses=0;}
    bool alternate=false;Actor* caption_target=nullptr;
    if(first&&(first->flags&0x110000u)){auto second=ray(0x110000u);
        if(second&&!queries.same_selection_hit(*first,*second)&&(second->flags&0x40u)&&(!second->actor||queries.kind(*second->actor)==3)){
            distance=hud_target_distance(second->position,head);if(second->actor){queries.selection_roll_target(static_cast<Actor*>(nullptr),distance);caption_target=second->actor;alternate=true;
                queries.selection_crosshair(caption_target,distance<=2.f);pair[1]=nullptr;}}
    }
    if(!alternate){if(target&&queries.kind(*target)==6&&queries.selection_door_blocked(*target)&&distance>.6499999761581421f)target=nullptr;
        auto* attachment=queries.attachment(*human);if(!attachment||!hud_roll_vehicle_kind(queries.kind(*attachment)))queries.selection_roll_target(target,distance);
        if(hud_roll_ordinary_eligible(queries,target,distance)){queries.selection_crosshair(target,true);pair[1]=target?queries.selection_identity(*target):nullptr;}
        else{queries.selection_crosshair(target,distance<=.20000000298023224f);pair[1]=nullptr;}
    }
    pair[0]=target?queries.selection_identity(*target):nullptr;
    if(auto* attached=queries.attachment(*human))if(hud_roll_vehicle_kind(queries.kind(*attached))){queries.selection_roll_target(attached,1.f);pair[0]=pair[1]=nullptr;}
    if(queries.selection_signals_active())queries.selection_signal_target(pair[0]);
}
} //namespace hd2::game
#endif
