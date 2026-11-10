#ifndef HD2_GAME_HUMAN_AI_FRAME_QUERY_H
#define HD2_GAME_HUMAN_AI_FRAME_QUERY_H

#include "game/human_ai_navigation_frame.h"

namespace hd2::game {
template<class AI,class Frames>
bool human_ai_near_query_point(AI& ai,Frames& frames,const std::array<float,3>& point,float radius){ //0057FE40.
    auto* human=ai.human();
    if(!human||!human->base().frame())throw std::logic_error("AI navigation query requires the actual Human model");
    const auto world=frames.actor_world_matrix(human->base().frame());
    const auto x=static_cast<long double>(world[12])-point[0];
    const auto y=static_cast<long double>(world[13])-point[1];
    const auto z=static_cast<long double>(world[14])-point[2];
    return static_cast<long double>(radius)*radius>(z*z+x*x)+y*y;
}

//0057A420 before an active path exists. A missing referenced table point and
//an already reached destination share the original completion publication.
template<class AI,class Frames>
bool human_ai_needs_path(AI& ai,Frames& frames){
    auto& resources=ai.resources();
    auto& query=resources.query().state;
    const auto flags=query.template get<std::uint32_t>(0x50);
    if(flags&4u)return true;
    if(flags&0x4000u)return false;
    if(flags&0x10000000u){ //0057A420, update the tail of the existing path when it still matches.
        const auto& jobs=resources.navigation_jobs();
        if(jobs.empty())return true;
        auto& tail=jobs.back()->words;
        const auto index=query.template get<std::uint16_t>(0x20);
        const auto position=query.template get<HumanVector>(0x10);
        if(flags&2u){
            if(tail.template get<std::uint32_t>(0x20)!=index)return true;
            tail.set(0,position);
            query.set(0x50,flags&0xffffdffcu);
            return false;
        }
        if(index==0xffffu){
            const auto node=tail.template get<HumanVector>(0x10);
            const auto radius=static_cast<long double>(tail.template get<float>(0x28));
            const auto x=static_cast<long double>(position[0])-node[0];
            const auto y=static_cast<long double>(position[1])-node[1];
            const auto z=static_cast<long double>(position[2])-node[2];
            if(!(radius*radius>(x*x+z*z)+y*y))return true; //0057FEB0.
            tail.set(0,position);
        }else{
            if(tail.template get<std::uint32_t>(0x20)!=index)return true;
            auto* navigation=ai.navigation();
            if(!navigation)throw std::logic_error("AI active point query requires the actual navigation table");
            if(!navigation->record(index))return true;
            tail.set(0,navigation->position(index)); //The original stack W is unspecified.
        }
        query.set(0x50,flags&0xffffdffeu);
        return false;
    }
    std::array<float,3> destination;
    bool present=true;
    const auto index=query.template get<std::uint16_t>(0x20);
    if(!(flags&2u)&&index!=0xffffu){
        auto* navigation=ai.navigation();
        if(!navigation)throw std::logic_error("AI point query requires the actual navigation table");
        present=navigation->record(index)!=nullptr;
        if(present)destination=navigation->position(index);
    }else destination=query.template get<std::array<float,3>>(0x10);
    if(present&&!human_ai_near_query_point(ai,frames,destination,.5f))return true;
    query.set(0x50,(flags&0xffffdffcu)|0x80000000u);
    return false;
}

//0057A750. A direct in-range request completes through the model and table
//before steering. The graph binds the same AI, frame and process job owners.
template<class AI,class Frames,class Graph>
std::uint32_t advance_human_ai_frame_query(AI& ai,Frames& frames,Graph& graph,std::uint32_t elapsed){
    auto& resources=ai.resources();
    auto& receiver=resources.query();
    auto& query=receiver.state;
    const auto flags=[&]{return query.template get<std::uint32_t>(0x50);};
    const auto result=[&](std::uint32_t value){query.set(0x4c,value);return value;};
    const auto finish=[&](std::uint32_t value){human_ai_navigation::finish_path(ai,value);};
    const auto tick_path=[&]{ //0057F670 only advances a nonnegative query.
        if(!(flags()&0x80000000u))graph.advance_ai_navigation_path(ai,elapsed); //0057F670 positive worker.
    };
    constexpr std::uint32_t active_path=0x10000000u;
    const auto job_point=[&](const auto& job){
        auto* navigation=ai.navigation();
        if(!navigation)throw std::logic_error("AI navigation job requires the actual navigation table");
        return navigation->record(static_cast<std::uint16_t>(job.words.template get<std::uint32_t>(0x20)));
    };
    for(auto offset:{0x34u,0x24u}){
        const auto left=query.template get<std::uint32_t>(offset)-elapsed;
        query.set(offset,human_frame_signed_word(left)<0?0u:left);
    }
    result(1);
    if(ai.words().template get<std::uint32_t>(0x48)!=5||(flags()&active_path)){ //0057A610.
        const auto* point=ai.point();
        const bool reroute=point&&(point->flags&0x2000u)&&
            (!(flags()&active_path)||(flags()&0x8000u)||query.template get<std::uint8_t>(0x58));
        if(reroute)graph.reroute_ai_navigation_point(ai); //0057B270.
        else{
            auto& jobs=resources.navigation_jobs();
            if((flags()&0x4000u)&&(flags()&active_path)&&!jobs.empty()){
                const auto* tail=job_point(*jobs.back());
                if(tail&&(tail->flags&0x2000u))graph.reroute_ai_navigation_point(ai);
            }
            if(flags()&0x8000u){
                bool released=false;
                if((flags()&active_path)&&!jobs.empty()){
                    const auto* head=job_point(*jobs.front());
                    if(head&&!(head->flags&0x2000u)){
                        query.set(0x50,flags()&~0x8000u);
                        released=true;
                    }
                }
                if(!released&&!query.template get<std::uint32_t>(0x34)){
                    finish(0xfffffffcu);
                    return query.template get<std::uint32_t>(0x4c);
                }
            }
        }
    }
    if(flags()&0x4000u){
        tick_path();
        if(flags()&active_path)return result(0xfffffffeu);
        query.set(0x50,flags()&~0x4000u);
    }
    if(flags()&0x8000u)return result(0xfffffffdu);
    if(flags()&0x2000u)finish(4);
    if(query.template get<std::uint8_t>(0x58))return result(2);
    if((flags()&1u)&&human_ai_needs_path(ai,frames)){
        if(flags()&active_path)finish(4);
        if(!(flags()&active_path)&&!graph.create_ai_navigation_path(ai)) //0057CA70.
            return query.template get<std::uint32_t>(0x4c);
    }
    if(!(flags()&active_path))advance_human_ai_scene_steering(ai);
    else{
        tick_path();
        if(flags()&0x80000000u)return query.template get<std::uint32_t>(0x4c);
    }
    return result(1);
}
} //namespace hd2::game
#endif
