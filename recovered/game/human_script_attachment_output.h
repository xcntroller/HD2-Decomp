#ifndef HD2_GAME_HUMAN_SCRIPT_ATTACHMENT_OUTPUT_H
#define HD2_GAME_HUMAN_SCRIPT_ATTACHMENT_OUTPUT_H
#include "game/human_ai_shot_direction.h"
namespace hd2::game {
inline bool human_script_attached_kind(std::uint32_t kind){return kind==4u||kind==20u||kind==21u||kind==34u;}
//005904C0. Raw frame loans survive the next lazy matrix update, as native.
template<class State,class Frame,class Frames,class World>
void turn_human_script_at_frame(State& state,Frame frame,Frames& frames,World& world){
    const auto human=[&]{auto* p=state.human();if(!p)throw std::logic_error("Script TurnAt requires actual Human");return p;};
    if(frame){
        const auto* origin=frames.human_target_world_matrix(human()->base().frame());
        const auto* target=frames.human_target_world_matrix(frame);
        HumanVector direction{};
        for(unsigned n=0;n!=3;++n)direction[n]=static_cast<float>(static_cast<long double>(target[12+n])-origin[12+n]);
        normalize_human_ai_shot(direction,(static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[1])*direction[1])+static_cast<long double>(direction[2])*direction[2]);
        state.set_current_direction(direction);state.set_target_direction(direction);
    }
    if(auto* actor=human()->field160)if(human_script_attached_kind(actor->base().type())&&world.human_pose_actor_has_control(actor,human()->field164)){
        state.state().set(0x1d0,std::uint8_t{1});state.publish_script_turn_frame(frame);return;
    }
    state.state().set(0x1d0,std::uint8_t{0}); //A rejected gate preserves old1CC.
}
//0058DB20, reached in presentation and authority at their original call sites.
template<class State,class Frames,class World>
void advance_human_script_attachment(State& state,Frames& frames,World& world){
    const auto human=[&]{auto* p=state.human();if(!p)throw std::logic_error("Script attachment requires actual Human");return p;};
    auto* actor=human()->field160;
    if(!actor||!human_script_attached_kind(actor->base().type())){state.state().set(0x1d0,std::uint8_t{0});return;}
    HumanVector direction{};
    if(state.script_turn_frame()){
        const auto* origin=frames.human_target_world_matrix(human()->base().frame());
        const auto* target=frames.human_target_world_matrix(state.script_turn_frame());
        for(unsigned n=0;n!=3;++n)direction[n]=static_cast<float>(static_cast<long double>(target[12+n])-origin[12+n]);
        const auto current=world.human_pose_actor_direction(human()->field160,human()->field164);
        const auto dot=(static_cast<long double>(current[2])*direction[2]+static_cast<long double>(current[1])*direction[1])+static_cast<long double>(current[0])*direction[0];
        //Native tests the UNNORMALIZED displacement first, and clears1D0.
        if(!(dot<.9850000143051147f)){state.state().set(0x1d0,std::uint8_t{0});return;}
    }else{
        auto matrix=frames.actor_world_matrix(actor->base().frame());
        direction={{matrix[8],matrix[9],matrix[10],matrix[11]}};
        normalize_human_ai_shot(direction,(static_cast<long double>(direction[1])*direction[1]+static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[0])*direction[0]);
        auto current=world.human_pose_actor_direction(human()->field160,human()->field164);
        normalize_human_ai_shot(direction,(static_cast<long double>(direction[2])*direction[2]+static_cast<long double>(direction[1])*direction[1])+static_cast<long double>(direction[0])*direction[0]);
        normalize_human_ai_shot(current,(static_cast<long double>(current[1])*current[1]+static_cast<long double>(current[2])*current[2])+static_cast<long double>(current[0])*current[0]);
        const auto dot=(static_cast<long double>(current[2])*direction[2]+static_cast<long double>(current[1])*direction[1])+static_cast<long double>(current[0])*direction[0];
        if(!(dot<.9999899864196777f))return; //This aligned branch RETAINS1D0.
    }
    normalize_human_ai_shot(direction,(static_cast<long double>(direction[2])*direction[2]+static_cast<long double>(direction[1])*direction[1])+static_cast<long double>(direction[0])*direction[0]);
    state.set_current_direction(direction);state.set_target_direction(direction);
}
}
#endif
