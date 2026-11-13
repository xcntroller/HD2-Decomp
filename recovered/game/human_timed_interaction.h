#ifndef HD2_GAME_HUMAN_TIMED_INTERACTION_H
#define HD2_GAME_HUMAN_TIMED_INTERACTION_H
#include "game/human_ai_shot_direction.h"
#include "game/human_loading_stats.h"
namespace hd2::game {
//00751480's timed common-command starts. This publishes the SAME DD0+A4C
//twelve-byte records consumed by0075F230; it does not advance a containing frame.
template<class Control,class Look,class Human,class Actor,class Services,class Frames,class Publish>
bool begin_human_timed_interaction(Control& control,Look& look,Human& human,Actor* target,
        std::uint32_t flags,Services& services,Frames& frames,Publish& publish){
    const auto validate=services.human_common_receiver_guard(human);
    const auto action=flags&0x3ffu;
    auto& physical=control.physical();auto& words=physical.control_frame_words();auto& queue=physical.control_action_queue();
    auto& request=look.frame_words();constexpr std::uint32_t locks=0xb4b;
    const auto posture=[&]{return request.template get<std::uint32_t>(0x4c);};
    const auto delay=[&](float value){return human_loading_stat_word(static_cast<long double>(value)/services.human_control_animation_scale());};
    const auto append=[&](std::uint32_t time,std::uint32_t command,std::uint32_t mode){queue.push_back({{time,command,mode}});};
    const auto lock=[&]{request.set(0xd4,request.template get<std::uint32_t>(0xd4)|locks);};
    const auto hide=[&]{services.human_presentation_equipment(human).set_hidden(true);validate();};
    const auto stop=[&]{HumanVector output=control.movement().frame_words().template get<HumanVector>(0xb0);
        control.tracks().motion_velocity(HumanVector{},0,0,output);control.movement().control_frame_track_velocity(output);
        for(auto offset:{0x4d0u,0x4d4u,0x4d8u})words.set(offset,0.f);
    };
    const auto capacity=action==0xe||action==0x10||action==0x24?11u:
        action==0x14||action==0x16||action==0x18||action==0x20?15u:25u;
    switch(action){case 0xe:case 0x10:case 0x12:case 0x14:case 0x16:case 0x18:case 0x1a:case 0x1c:case 0x1e:case 0x20:case 0x24:break;
    default:throw std::logic_error("Human timed interaction requires its actual selected action");}
    if(queue.size()>capacity)return false; //Original rejection precedes all effects.
    if(!(services.mode_flags()&1u)&&!control.human_is_player()&&!words.template get<std::uint32_t>(0xa7c))queue.clear(); //00757510/007571B0 actual erase(begin,end).
    const auto normalized=[&](bool reverse,unsigned order){
        if(!target)throw std::runtime_error("Timed Human action dereferences missing native target");
        const auto source=human.base().world_position(),destination=target->base().world_position();HumanVector vector{};
        for(unsigned n=0;n!=3;++n)vector[n]=reverse?source[n]-destination[n]:destination[n]-source[n];
        const auto square=[&](unsigned n){return static_cast<long double>(vector[n])*vector[n];};
        const auto length=order==1?(square(2)+square(0))+square(1):order==2?(square(0)+square(1))+square(2):(square(1)+square(2))+square(0);
        const float stored=static_cast<float>(length);constexpr float epsilon=9.99999993922529e-9f;
        if(!std::isfinite(stored))throw std::runtime_error("Non-finite native timed-action direction");
        const auto difference=action==0xe||action==0x10?std::abs(length-1.L):static_cast<long double>(std::abs(static_cast<float>(length-1.L)));
        if(!(difference<epsilon)){
            if(!(stored<epsilon)){const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));for(unsigned n=0;n!=3;++n)vector[n]=static_cast<float>(vector[n]*inverse);}
            else{const auto axis=vector[0]!=0?0u:vector[2]!=0?2u:1u;vector[axis]=vector[axis]<0?-1.f:1.f;}
        }
        return vector;
    };
    //Queued pick/place/lockpick use no actor pose. Posture1 is a short path.
    if(action==0x12||action==0x14||action==0x16){
        const bool special=request.template get<std::uint32_t>(0xb8)==5;
        if(action==0x12||!special)hide();
        lock();const auto command=action==0x12?6u:8u;
        if(action!=0x12&&special){append(10,command,2);return true;}
        if(posture()==1){append(10,command,2);append(delay(500),12,1);return true;}
        append(delay(2500),1,0);append(delay(150),command,2);
        if(posture()==0||posture()==2)append(delay(2500),posture(),0);
        append(delay(500),12,0);return true;
    }
    if(action==0xe||action==0x10){
        hide();stop();const auto direction=normalized(false,0);look.current_direction(direction);
        if(action==0xe){
            auto position=target->base().world_position();
            for(unsigned n=0;n!=3;++n){const auto offset=static_cast<float>(static_cast<long double>(direction[n])*.9f);position[n]=static_cast<float>(static_cast<long double>(position[n])-offset);}
            control.tracks().motion_position_hold(position,2,2000);
            if(posture()==1){append(10,5,2);append(delay(500),12,1);}
            else{append(delay(2500),1,0);append(delay(150),5,2);
                if(posture()==0||posture()==2){append(delay(2500),posture(),0);}
                append(delay(500),12,0);}
        }else{
            append(10,1,2);if(posture()==0||posture()==2)append(delay(2500),posture(),0);
            if(human_frame_signed_word(words.template get<std::uint32_t>(0xa30))<=0&&words.template get<std::uint32_t>(0x1d8)!=0x5f)append(delay(500),12,0);
        }
        look.frame_movement_animation(45,publish);validate();look.position_command(HumanVector{});lock();return true;
    }
    if(action==0x24){
        hide();stop();if(words.template get<std::uint32_t>(0xa34)==0||words.template get<std::uint32_t>(0xa34)==1){
            if(posture()==1){append(10,18,2);append(delay(500),12,1);}
            else{append(delay(2500),1,0);append(delay(150),18,2);
                if(posture()==0||posture()==2){append(delay(2500),posture(),0);}
                append(delay(500),12,0);}
        }
        look.frame_movement_animation(45,publish);validate();look.position_command(HumanVector{});lock();return true;
    }
    if(action==0x1c){
        hide();const auto item=services.human_common_ground_entry(human,true);
        if(item){const auto model=services.human_common_item_model(item);validate();services.human_presentation_equipment(human).state().select(model,6,0xffffffffu,-5,-1); }
        lock();if(posture()==1){append(10,11,2);append(delay(500),12,1);return true;}
        append(delay(2500),1,0);append(delay(150),11,2);
        if(posture()==0||posture()==2)append(delay(2500),posture(),0);
        append(delay(500),12,0);return true;
    }
    const bool reverse=action==0x1a||action==0x20;
    const auto direction=normalized(reverse,action==0x1a?1u:action==0x18?0u:2u);
    if(action==0x18||action==0x1e){
        auto position=target->base().world_position();const auto distance=action==0x18?.9f:.5f;
        for(unsigned n=0;n!=3;++n){const auto offset=static_cast<float>(static_cast<long double>(direction[n])*distance);position[n]=static_cast<float>(static_cast<long double>(position[n])-offset);}
        control.tracks().motion_position_hold(position,2,2000);
    }
    look.current_direction(direction);
    if(action==0x1a||action==0x20)words.set(0x1c8,.45f);
    if(action==0x1a){
        if(!(services.mode_flags()&1u)){
            look.force_frame_commands(true);look.current_direction(direction);look.force_frame_commands(false);
            words.set(0xa34,flags);physical.enable(true);
        }
        hide();words.set(0x20c,0x5cu);words.set(0x218,0x4cu);look.action_command(0x4c,true);lock();
        if(posture()!=0)append(delay(2500),0,0);
        append(posture()==0?10u:delay(150),10,2);return true;
    }
    if(action==0x20){
        if(services.mode_flags()&1u){
            const auto bone=physical.bones().at(4);if(!bone)throw std::runtime_error("Human action20 requires actual bone840");
            const auto matrix=frames.actor_world_matrix(bone);const HumanVector point{{matrix[12],matrix[13],matrix[14],matrix[15]}};
            services.apply_human_damage(human,11u,1000.f,point,human.base().world_direction(),target,0u);validate();
        }
        if(posture()!=0)append(delay(2500),0,0);
        append(posture()==0?10u:delay(150),17,2);return true;
    }
    hide();lock();const auto command=action==0x18?9u:16u;
    if(posture()==0){append(10,command,2);append(delay(500),12,1);return true;}
    append(delay(2500),0,0);append(delay(150),command,2);
    if(posture()==1||posture()==2)append(delay(2500),posture(),0);
    append(delay(500),12,0);return true;
}
}
#endif
