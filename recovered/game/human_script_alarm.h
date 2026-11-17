#ifndef HD2_GAME_HUMAN_SCRIPT_ALARM_H
#define HD2_GAME_HUMAN_SCRIPT_ALARM_H
#include "game/human_ai_frame_metrics.h"
#include "game/human_packet_commands.h"
namespace hd2::game {
//0058E650.1B8 suppresses alarm categories: enabling clears bits, disabling
//sets them. Commit the new mask BEFORE the optional actual B0+98 replay.
template<class State,class Random>void change_human_script_alarm_types(
        State& state,std::uint32_t mask,bool enabled,Random&& random){
    const auto old=state.state().template get<std::uint32_t>(0x1b8);
    const bool replay=!(state.words().template get<std::uint32_t>(0x50)&0x80000000u)&&
        enabled&&(old&mask&0x20u);
    state.state().set(0x1b8,enabled?old&~mask:old|mask);
    if(!replay)return;
    auto* human=state.human();if(!human)throw std::logic_error("Alarm type replay requires its actual Human");
    const auto& enemies=human->perception.lists()[0];
    if(enemies.empty())return;
    auto* target=dynamic_cast<std::remove_pointer_t<decltype(human)>*>(enemies.at(random()%enemies.size()));
    if(!target)throw std::runtime_error("Alarm type replay requires its actual perceived Human");
    state.sight_enemy_acquired(*target);
}
//00591550. CRT rand is independent of engine RandFloat. No random draw occurs
//for state20/other states; failed item/target/range admission takes one retry draw.
template<class State,class Words,class Human,class World>void choose_human_script_alarm(
        State& state,Words& words,Human*& target,std::uint32_t& target_id,World& world){
    const auto retry=[&]{return human_packet_ftol(static_cast<long double>(human_frame_signed_word(world.human_frame_random_word()))*
        3.0518509447574615e-05f*3000.f+500.f);};
    const auto action=words.template get<std::uint32_t>(0x19c);
    auto& resources=state.resources();
    if(action==0x40){
        world.script_alarm_clear_item(state);resources.flag().request(1);resources.target().bind_actor(nullptr);
        resources.query().state.set(0x58,std::uint8_t{0});const auto timer=retry();
        target=nullptr;target_id=0;words.set(0x19c,0x10u);words.set(0x1a4,0u);words.set(0x1a0,timer);return;
    }
    if(action!=0x10)return;
    if(!state.item()){words.set(0x1a0,retry());return;}
    target=world.script_alarm_target(state);
    if(!target){words.set(0x1a0,retry());return;}
    target_id=static_cast<std::uint32_t>(target->base().id10());
    auto* human=state.human();if(!human)throw std::logic_error("Script alarm requires actual Human44");
    const float distance=static_cast<float>(world.script_alarm_distance(*human,*target));
    const auto draw=[&]{return static_cast<long double>(human_frame_signed_word(world.human_frame_random_word()))*3.0518509447574615e-05f*10.f;};
    unsigned selected=0,base=0;
    switch(world.active_follow_item_kind(state.item())){
    case 2:if(distance<30.f)selected=draw()<7.f?0x43u:0x41u;base=300;break;
    case 3:if(distance<70.f)selected=draw()<5.f?0x43u:0x41u;base=300;break;
    case 4:if(distance<250.f)selected=draw()<7.f?0x43u:0x42u;base=1000;break;
    case 5:if(distance<300.f)selected=0x42u;base=600;break;
    case 12:if(distance<300.f)selected=draw()<7.f?0x43u:0x42u;base=1000;break;
    default:break;
    }
    if(!selected){words.set(0x1a0,retry());return;}
    world.active_follow_ready_item(state);resources.flag().request(selected==0x42?2u:4u);resources.target().bind_actor(target);
    if(selected!=0x41)resources.query().state.set(0x58,std::uint8_t{1}); //41 leaves the previous byte untouched.
    const auto timer=human_packet_ftol(static_cast<long double>(human_frame_signed_word(world.human_frame_random_word()))*
        3.0518509447574615e-05f*1500.f+base);
    words.set(0x1a4,selected==0x42?2u:4u);words.set(0x19c,0x40u);words.set(0x1a0,timer);
}
}
#endif
