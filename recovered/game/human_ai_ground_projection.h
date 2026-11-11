#ifndef HD2_GAME_HUMAN_AI_GROUND_PROJECTION_H
#define HD2_GAME_HUMAN_AI_GROUND_PROJECTION_H

#include "game/human_ai_frame_query.h"
#include "game/human_scene_movement_geometry.h"

namespace hd2::game {

//0057FBC0, actual AI B0+58. The A4 fallback changes Y while returning AL0.
//Only the active-job interpolation path returns AL1 to the caller's Y limiter.
template<class AI,class Frames>
bool project_human_ai_ground_position(AI& ai,Frames& frames,HumanVector& position){
    auto& resources=ai.resources();
    if(resources.query().state.template get<std::uint32_t>(0x50)&0x10000000u){
        const auto* current=resources.current_navigation_job();
        const auto& jobs=resources.navigation_jobs();
        if(current&&!jobs.empty()){
            if(!jobs.front())throw std::logic_error("AI ground projection reached the native missing-job dereference");
            const auto origin=current->words.template get<HumanVector>(0);
            const auto next=jobs.front()->words.template get<HumanVector>(0);
            const auto x=static_cast<long double>(next[0])-origin[0];
            const auto z=static_cast<long double>(next[2])-origin[2];
            auto fraction=((static_cast<long double>(position[2])-origin[2])*z+
                (static_cast<long double>(position[0])-origin[0])*x)/(z*z+x*x);
            if(fraction<0)fraction=0;
            else if(fraction>1)fraction=1; //Unordered native fractions remain NaN.
            position[1]=static_cast<float>((static_cast<long double>(next[1])-origin[1])*fraction+origin[1]);
            return true;
        }
    }
    const auto* point=ai.point();if(!point)return false;
    const auto origin=human_scene_movement::record_position(*point);
    const std::array<float,3> centre{{origin[0],origin[1],origin[2]}};
    if(human_ai_near_query_point(ai,frames,centre,human_scene_movement::record_radius(*point))){
        const auto* current_point=ai.point(); //Native rereads A4 after the proximity callback.
        if(!current_point)throw std::logic_error("AI ground projection lost its actual A4 point during the model query");
        position[1]=human_scene_movement::record_position(*current_point)[1];
    }
    return false;
}

} //namespace hd2::game
#endif
