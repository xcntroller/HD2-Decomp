#ifndef HD2_GAME_HUMAN_SCRIPT_TIMED_OUTPUT_H
#define HD2_GAME_HUMAN_SCRIPT_TIMED_OUTPUT_H
#include "game/human_ai_target_output.h"
#include "game/human_ai_trigger_output.h"
namespace hd2::game {
//0058F1B0,0058F1F0/0058F260: admission precedes timer/borrower publication.
template<class State,class World>bool admit_human_script_attack(State& state,World& world){
    state.state().set(0x1bd,std::uint8_t{0});
    auto* human=state.human();if(!human)throw std::logic_error("Script attack requires actual Human");
    if(auto* actor=human->field160){
        const auto kind=actor->base().type();
        if((kind!=4u&&kind!=21u)||!world.human_pose_actor_has_control(actor,human->field164))return false;
        state.state().set(0x1bd,std::uint8_t{1});state.state().set(0x1d0,std::uint8_t{0});
    }else if(!world.human_script_has_weapon(*human))return false;
    return true;
}
template<class State,class Actor,class World>void request_human_script_actor_attack(
        State& state,Actor* actor,std::uint32_t duration,bool ill_fire,std::int32_t pc,World& world){
    if(!admit_human_script_attack(state,world))return;
    state.state().set(0x1c0,duration);state.publish_script_attack_actor(actor);
    state.words().set(0xb4,std::uint8_t(ill_fire));state.state().set(0x1dc,pc);
}
template<class State,class Frame,class World>void request_human_script_frame_attack(
        State& state,Frame frame,std::uint32_t duration,bool ill_fire,std::int32_t pc,World& world){
    if(!admit_human_script_attack(state,world))return;
    state.state().set(0x1c0,duration);state.publish_script_attack_actor(nullptr);state.publish_script_attack_frame(frame);
    state.words().set(0xb4,std::uint8_t(ill_fire));state.state().set(0x1dc,pc);
}
//0058E8B0. Target model is selected before the actual attachment A8 query;
//Human stance/velocity are read afterwards. Native return W is unwritten.
template<class State,class World>HumanVector human_script_mounted_target(State& state,bool spread,World& world){
    auto* actor=state.script_attack_actor();
    const auto frame=actor?actor->base().frame():state.script_attack_frame();
    auto* human=state.human();if(!human||!human->field160)throw std::logic_error("Script mounted target requires actual attachment");
    const auto origin=world.human_script_mounted_position(human->field160,human->field164);
    actor=state.script_attack_actor();HumanVector position{};
    auto& frames=world.human_command_frames();
    if(actor&&actor->base().type()==3u){
        auto& target=world.human_ai_target_human(*actor);const auto flags=world.human_script_target_flags(target);
        const auto matrix=frames.actor_world_matrix(flags&6u?target.head_marker:target.base().frame());
        position={{matrix[12],matrix[13],matrix[14],0}};
        if(flags&6u)position[1]=static_cast<float>(static_cast<long double>(position[1])-.30000001192092896f);
        const auto& velocity=target.activation.vector1b0;
        position[0]=static_cast<float>(static_cast<long double>(position[0])+static_cast<long double>(velocity[0])*.125f);
        position[1]=static_cast<float>(static_cast<long double>(position[1])+static_cast<long double>(velocity[1])*.125f);
        const float lead_z=static_cast<float>(static_cast<long double>(velocity[2])*.125f); //58EA19 spill.
        position[2]=static_cast<float>(static_cast<long double>(position[2])+lead_z);
    }else{
        if(!frame)throw std::logic_error("Script mounted target requires actual target frame");
        const auto matrix=frames.actor_world_matrix(frame);
        position={{matrix[12],static_cast<float>(static_cast<long double>(matrix[13])+.75f),matrix[14],0}};
    }
    const auto x=static_cast<long double>(position[0])-origin[0],y=static_cast<long double>(position[1])-origin[1],z=static_cast<long double>(position[2])-origin[2];
    const float square=static_cast<float>((z*z+y*y)+x*x);
    std::uint32_t bits;std::memcpy(&bits,&square,4);
    const auto distance=human_frame_signed_word(bits)<0x322bcc77?0.L:
        std::abs(static_cast<long double>(square)-1)<9.99999993922529e-09f?1.L:std::sqrt(static_cast<long double>(square));
    const float radius=static_cast<float>(distance*.05999999865889549f);
    if(spread){
        constexpr float scale=3.0518509447574615e-05f;
        const auto z_draw=static_cast<long double>(human_frame_signed_word(world.human_frame_random_word()))*scale;
        const float random_z=static_cast<float>((z_draw+z_draw)-1.f);
        const float random_y=static_cast<float>(static_cast<long double>(human_frame_signed_word(world.human_frame_random_word()))*scale-.5f);
        const auto x_draw=static_cast<long double>(human_frame_signed_word(world.human_frame_random_word()))*scale;
        const HumanVector offset{{static_cast<float>(((x_draw+x_draw)-1.f)*radius),static_cast<float>(static_cast<long double>(random_y)*radius),static_cast<float>(static_cast<long double>(random_z)*radius),0}};
        for(unsigned n=0;n!=3;++n)position[n]=static_cast<float>(static_cast<long double>(position[n])+offset[n]);
    }
    HumanVector result{};for(unsigned n=0;n!=3;++n)result[n]=static_cast<float>(static_cast<long double>(position[n])-origin[n]);
    return result;
}
//0058EBD0: original A4/B4/9C selection and two different alignment thresholds.
template<class State,class World>void advance_human_script_mounted_attack(State& state,World& world){
    auto* human=state.human();if(!human||!human->field160)throw std::logic_error("Script mounted attack requires actual attachment");
    auto* actor=human->field160;const auto seat=human->field164;
    auto desired=human_script_mounted_target(state,false,world);
    auto current=world.human_pose_actor_direction(actor,seat);
    const bool precise=world.human_script_mounted_precise(actor,seat);
    normalize_human_ai_shot(current,(static_cast<long double>(current[1])*current[1]+static_cast<long double>(current[2])*current[2])+static_cast<long double>(current[0])*current[0]);
    const auto normalize_desired=[&]{normalize_human_ai_shot(desired,(static_cast<long double>(desired[2])*desired[2]+static_cast<long double>(desired[1])*desired[1])+static_cast<long double>(desired[0])*desired[0]);};
    normalize_desired();state.set_target_direction(desired);
    if(!world.human_script_mounted_ammunition(actor,seat)){
        auto* first=state.first_frame_inputs();if(!first)throw std::logic_error("Script mounted reload requires actual first input");
        first->set(0x34,1.f);return;
    }
    const auto dot=(static_cast<long double>(current[2])*desired[2]+static_cast<long double>(current[1])*desired[1])+static_cast<long double>(desired[0])*current[0];
    if(!(dot>(precise?.9998000264167786f:.9850000143051147f)))return;
    desired=human_script_mounted_target(state,!precise,world);normalize_desired();state.set_target_direction(desired);
    auto* first=state.first_frame_inputs();if(!first)throw std::logic_error("Script mounted trigger requires actual first input");
    first->set(0x18,1.f);
    auto* second=state.second_frame_inputs();if(!second)throw std::logic_error("Script mounted trigger requires actual second input");
    second->set(0x18,1.f);
}
//0058E500, same resources/input/target and actual Script context completion.
template<class State,class World>void advance_human_script_timed_attack(State& state,std::uint32_t elapsed,World& world){
    state.refresh_item();auto& resources=state.resources();auto& words=resources.state();
    if(state.state().template get<std::uint8_t>(0x1bd))advance_human_script_mounted_attack(state,world);
    else if(state.item()&&words.template get<std::int32_t>(0xc8)<1){
        if(!(state.words().template get<std::uint32_t>(0x50)&0x200000u)){
            HumanVector direction{};
            if(auto* actor=state.script_attack_actor()){
                words.set(0xd8,0u);direction=world.human_frame_actor_target_direction(state,*actor);
            }else{
                words.set(0xd8,words.template get<std::uint32_t>(0xd8)|2u);
                direction=world.human_frame_frame_target_direction(state,state.script_attack_frame());
            }
            state.set_target_direction(direction);resources.publish_script_aimed_actor(state.script_attack_actor());
            (void)begin_human_ai_aim(state,world);
        }else{
            advance_human_ai_trigger(state,elapsed,world);
            if(!words.template get<std::uint32_t>(0xc4))stop_human_ai_aim(state,static_cast<std::uint32_t>(human_animation_time_integer(static_cast<long double>(world.human_sound_random())*400.f+300.f)),world);
            if(words.template get<std::uint32_t>(0xd8)&1u)finish_human_ai_trigger(state);
        }
    }
    const auto remaining=state.state().template get<std::uint32_t>(0x1c0)-elapsed;
    state.state().set(0x1c0,remaining);
    if(human_frame_signed_word(remaining)<=0){
        state.state().set(0x1c0,0u);stop_human_ai_aim(state,0u,world);
        state.finish_script_attack_context(state.state().template get<std::int32_t>(0x1dc));
        state.state().set(0x1dc,std::int32_t{-1});
    }
}
}
#endif
