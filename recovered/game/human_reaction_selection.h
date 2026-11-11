#ifndef HD2_GAME_HUMAN_REACTION_SELECTION_H
#define HD2_GAME_HUMAN_REACTION_SELECTION_H
#include "game/human_reaction_shape.h"
#include "game/human_animation_tracks.h"
#include "game/human_physics_globals.h"

namespace hd2::game {
//0074D510, SAME AA0 links. Look supplies commands; DD0 owns the queued
//descriptor and the process owner supplies both original shared selectors.
template<class Control,class Look,class Human,class Services,class Frames,class Random>
bool select_human_reaction(Control& control,Look& look,Human& human,Services& services,
        Frames& frames,Random& random,HumanPhysicsGlobals& globals){
    using namespace human_retained_clip;
    auto& physical=control.physical();auto& state=physical.control_frame_words();
    const auto word=[&](unsigned offset){return state.template get<std::int32_t>(offset);};
    const auto wanted=[&](unsigned offset){return look.state().template get<std::int32_t>(offset);};
    const auto queue=[&](std::uint32_t id){state.set(0x1fc,id);normalize_human_primary_animation(state,0x1fc);};
    const auto motion=[&](std::uint32_t value){state.set(0x1ac,value);state.set(0x18c,std::uint8_t{1});};
    const auto zero_velocity=[&]{for(auto offset:{0x9d0u,0x9d4u,0x9d8u})state.set(offset,0.f);};
    if(word(0x1ac)!=1){queue(206);return false;}
    const auto flags=static_cast<std::uint32_t>(wanted(0x60));
    auto original=flags&0x3c0u;state.set(0xa10,original);
    if(original&0x300u)state.set(0xa10,random()%2?0x40u:0x80u);
    if(!word(0xa10)){state.set(0xa10,0x80u);original=0x80u;}
    const auto active=word(0x1d8);
    if((active>=8&&active<=14)||(active>=106&&active<=108))state.set(0xa10,0x40u);
    if(word(0x9e0)==3)state.set(0x9e0,wanted(0xb8)==5?2:wanted(0x4c));
    state.set(0x200,word(0x9e0));normalize_human_primary_animation(state,0x1fc);
    physical.prepare_reaction_posture(word(0x9e0)!=0);
    for(auto offset:{0x9b8u,0x9bcu,0x9c0u,0x9c4u})state.set(offset,std::int32_t{-10000});
    const auto by_posture=[&](std::uint32_t standing,std::uint32_t crouched,std::uint32_t prone){
        switch(word(0x9e0)){case 0:queue(standing);break;case 1:queue(crouched);break;case 2:queue(prone);break;default:break;}
    };
    std::uint16_t reason=0;
    if((flags&0x400u)&&wanted(0xb8)==5){queue(0x9a);motion(8);zero_velocity();}
    else if(flags&0x800u){by_posture(0x74,0x75,0x76);motion(2);zero_velocity();}
    else if(flags&0x2000u){
        by_posture(0x98,0x99,0x8d);motion(6);reason=3;zero_velocity();
        const auto origin=human.base().world_position();
        const auto source=look.death_source()?look.death_source()->base().world_position():human.base().world_position();
        const auto direction=normalize(subtract(origin,source));look.current_direction(direction);
        state.set(0x1c8,.449999988079071f);
        control.tracks().motion_position_hold(add(scale(direction,.18000000715255737f),source),4.f,2000);
        look.position_command(Vector{},true);
    }else if(flags&0x1000u){by_posture(0x80,0x8a,0x8d);motion(2);reason=4;zero_velocity();}
    else{
        by_posture(0x80,0x8a,0x8d);
        if(word(0xa44)==4){motion(2);reason=2;}
        else if(active==0x62){queue(0x71);motion(2);reason=3;}
        else{
            bool selected=false;const auto sequence=globals.reaction_sequence;
            for(unsigned attempt=0;attempt!=3&&!selected;++attempt){
                auto direction=human.base().world_direction();const auto posture=word(0x9e0);
                if(posture==2){queue(sequence%5+0x8d);motion(2);selected=true;continue;}
                if(posture!=0&&posture!=1)continue;
                if(posture==0&&(wanted(0xb8)==2||wanted(0xb8)==3||active==0x9f||active==0x9e||active==0x9b||active==0x6f||active==0x22)){
                    motion(7);
                    queue(wanted(0xb8)==2?(sequence&1)+0x92:word(0xa44)<1?0x94:
                        active==0x9b||active==0x6f?(sequence&1)+0x72:sequence%3+0x95);
                    selected=true;continue;
                }
                if(posture==1&&active==0x62){queue(0x71);selected=true;continue;}
                const auto side=word(0xa10);
                if(side==0x40){
                    if(check_human_reaction_shape(control,look,human,services,frames,scale(direction,-1.f),0)){
                        queue(posture?(sequence%3+0x87):((sequence&3)+0x7c));selected=true;
                    }else state.set(0xa10,0x80u);
                }else if(side==0x80){
                    if(check_human_reaction_shape(control,look,human,services,frames,direction,0)){
                        queue(posture?((sequence&3)+0x83):(sequence%5+0x77));selected=true;
                    }else state.set(0xa10,0x40u);
                }else if(side==0x100||side==0x200){
                    const float angle=side==0x100?-1.5707963705062866f:1.5707963705062866f;
                    direction=frames.car_transform(direction,frames.human_axis_rotation(Vector{{0,1,0,0}},angle)); //00788490.
                    if(check_human_reaction_shape(control,look,human,services,frames,direction,side==0x100?2u:1u)){
                        queue(side==0x100?(posture?0x8c:0x82):(posture?0x8b:0x81));selected=true;
                    }else state.set(0xa10,original&0xc0u);
                }
            }
        }
    }
    ++globals.reaction_sequence;
    control.finish_reaction_selection(look,reason);
    return true;
}
} //namespace hd2::game
#endif
