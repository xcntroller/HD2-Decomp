#ifndef HD2_GAME_HUMAN_AI_TRAVERSAL_H
#define HD2_GAME_HUMAN_AI_TRAVERSAL_H
#include "game/human_ai_navigation_frame.h"
namespace hd2::game {
//0057CC50, SAME12C query/124 direction/134 mode, real Ladder occupants and F8 jobs.
template<class AI,class Inline,class World,class Runtime>
void advance_human_ai_ladder_traversal(AI& ai,Inline& control,World& world,Runtime& runtime){
    auto& resources=ai.resources();auto& receiver=resources.query();auto& query=receiver.state;
    auto& direction=resources.direction();const auto flags=[&]{return query.template get<std::uint32_t>(0x50);};
    const auto mode=[&]{return resources.flag().state.template get<std::uint32_t>(0);};
    const auto job_height=[&]{
        const auto& jobs=resources.navigation_jobs();if(jobs.empty())throw std::logic_error("AI Ladder traversal reached original missing F8 head");
        const auto matrix=ai.frame_traversal_matrix();return static_cast<long double>(matrix[13])-jobs.front()->words.template get<float>(4);
    };
    auto* animation=control.control();if(!animation)throw std::logic_error("AI Ladder traversal requires actual DD0");
    const auto blocked=[&]{const auto value=animation->animation_state_word(0x1d8);return value==8||value==14||(value>=106&&value<=108);};
    if(!(flags()&0x10u)){
        if((flags()&0x20u)&&!blocked()){
            if(mode()==4&&direction.target_locked()){direction.unlock_target();direction.lock_current();}
            receiver.actor=nullptr;query.set(0x48,std::uint16_t{0xffff});query.set(0x4a,std::uint16_t{0xffff});
            query.set(0x50,flags()&0xffffff0fu);return;
        }
    }else if(!blocked()){
        const auto kind=ai.words().template get<std::uint32_t>(0x48);
        if(kind==4||kind==8){
            auto& ladder=world.human_ladder(receiver.actor);
            if(!ladder.field164.empty()&&(ladder.field164.size()>1||ladder.field164.front()!=ai.human())){
                human_ai_navigation::finish_path(ai,0xfffffff9u);return;
            }
        }
        if(mode()==2){human_ai_navigation::finish_path(ai,0xfffffff9u);return;}
        if(mode()==4&&!direction.target_locked()){direction.unlock_current();direction.lock_target();}
        world.human_navigation_request_ladder(ai,receiver.actor,runtime); //00587540→actual DC subtype1E.
        if((world.mode_flags()&7u)==7u)return;
        if(world.mode_flags()&1u)ai.queue_navigation_ladder_contact(world.human_pose_actor(receiver.actor).base().id10());
        return;
    }else{
        query.set(0x50,(flags()&~0x10u)|0x20u);
        if(job_height()>0)query.set(0x50,flags()|0x80u); //Native first publishes this before resampling the world pose.
    }
    const auto height=job_height();
    query.set(0x50,height<=0?flags()&~0x80u:flags()|0x80u);
    const auto request=query.template get<std::uint32_t>(0x54);
    ai.words().set(0x5c,(request&0x14000u)?0x4000u:0x2000u);
    auto* input=ai.first_frame_inputs();if(!input)throw std::logic_error("AI Ladder traversal requires actual74 input");
    input->set(!(request&0x10u)&&!(flags()&0x80u)?0u:4u,1.f);
}
} //namespace hd2::game
#endif
