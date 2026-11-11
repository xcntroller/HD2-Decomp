#ifndef HD2_GAME_HUMAN_AI_LIGHT_OBSERVATION_H
#define HD2_GAME_HUMAN_AI_LIGHT_OBSERVATION_H

#include "game/human_ai_frame_query.h"
#include "game/human_scene_movement_geometry.h"
#include "game/mission_navigation.h"

namespace hd2::game {

template<class AI,class Frames>
bool observe_human_ai_light(AI& ai,Frames& frames,float& shadowed,float& plain){ //0057F880, actual B0 virtual5C.
    const auto flags=ai.words().template get<std::uint32_t>(0x50);
    if(flags&0x800u)return false;
    auto* human=ai.human();
    if(!human)throw std::logic_error("AI light observation requires its actual bound Human");
    if(ai.words().template get<std::uint32_t>(0x48)==3&&human->field160&&human->field160->base().type()==4)return false;
    const unsigned mode=flags&1u?0u:(flags&(4u|0x1f000u))?1u:0u;
    const auto nearby_point=[&]() -> const MissionTableRecord* {
        const auto* point=ai.point();
        if(!point)return nullptr;
        const auto position=human_scene_movement::record_position(*point);
        if(!human_ai_near_query_point(ai,frames,{{position[0],position[1],position[2]}},human_scene_movement::record_radius(*point)))return nullptr;
        point=ai.point(); //Native rereads A4 after0057FE40.
        if(!point)throw std::logic_error("AI light observation lost its actual navigation point");
        return point;
    };
    auto& resources=ai.resources();
    if(resources.query().state.template get<std::uint32_t>(0x50)&0x10000000u){
        if(const auto* point=nearby_point())return MissionNavigation::light_values(*point,mode,shadowed,plain);
        const auto* current=resources.current_navigation_job();
        const auto& jobs=resources.navigation_jobs();
        if(current&&current->words.template get<std::uint32_t>(0x20)!=0xffffffffu&&!jobs.empty()){
            if(!jobs.front())throw std::logic_error("AI light observation reached the native missing-job dereference");
            const auto* next=jobs.front().get();
            if(next->words.template get<std::uint32_t>(0x20)!=0xffffffffu){
                const auto start=current->words.template get<HumanVector>(0),end=next->words.template get<HumanVector>(0);
                HumanVector travel{},offset{};
                for(unsigned axis=0;axis!=3;++axis)travel[axis]=end[axis]-start[axis];
                const auto world=frames.actor_world_matrix(human->base().frame());
                for(unsigned axis=0;axis!=3;++axis)offset[axis]=world[12+axis]-start[axis];
                (void)frames.actor_world_matrix(human->base().frame()); //Original second cache query before interpolation.
                const auto dot=[](const HumanVector& first,const HumanVector& second){
                    return (static_cast<long double>(first[2])*second[2]+static_cast<long double>(first[1])*second[1])+
                        static_cast<long double>(first[0])*second[0];
                };
                const auto quotient=dot(offset,travel)/dot(travel,travel);
                const float fraction=static_cast<float>(quotient);
                const auto point_for=[&](const HumanAINavigationJob& job) -> const MissionTableRecord& {
                    auto* navigation=ai.navigation();
                    if(!navigation)throw std::logic_error("AI light observation requires the SAME navigation table");
                    const auto* point=navigation->record(job.words.template get<std::uint16_t>(0x20));
                    if(!point)throw std::logic_error("AI light observation has an unavailable navigation job point");
                    return *point;
                };
                if(quotient<0)return MissionNavigation::light_values(point_for(*current),mode,shadowed,plain);
                if(fraction>1)return MissionNavigation::light_values(point_for(*next),mode,shadowed,plain);
                float current_shadowed,current_plain,next_shadowed,next_plain;
                if(!MissionNavigation::light_values(point_for(*current),mode,current_shadowed,current_plain))return false;
                if(!MissionNavigation::light_values(point_for(*next),mode,next_shadowed,next_plain))return false;
                shadowed=static_cast<float>((static_cast<long double>(next_shadowed)-current_shadowed)*fraction+current_shadowed);
                plain=static_cast<float>((static_cast<long double>(next_plain)-current_plain)*fraction+current_plain);
                return true;
            }
        }
    }
    if(const auto* point=nearby_point())return MissionNavigation::light_values(*point,mode,shadowed,plain);
    return false;
}

} //namespace hd2::game
#endif
