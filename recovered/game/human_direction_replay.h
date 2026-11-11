#ifndef HD2_GAME_HUMAN_DIRECTION_REPLAY_H
#define HD2_GAME_HUMAN_DIRECTION_REPLAY_H

#include "game/human_motion_velocity.h"
#include "game/human_animation_tracks.h"
#include "game/human_state_frame_timers.h"

namespace hd2::game {

template<class State>bool human_direction_replay_enabled(const State& state){ //0076CF90, native AL only.
    const auto animation=state.template get<std::int32_t>(0x1d8);
    const auto attachment=state.template get<std::int32_t>(0xa34),action=state.template get<std::int32_t>(0xa44);
    return ((animation<8||animation>14)||state.template get<std::int32_t>(0x998)!=-10000)&&
        ((attachment==0||attachment==1)||action<1||action>2);
}

template<class Control,class Look,class Human,class Services,class Frames>
bool validate_human_direction_turn(Control& control,const Look& look,Human& human,
        Services& services,Frames& frames,float angle){ //00749E60, AA0 links and actual posture collision queries.
    auto& physical=control.physical();auto& state=physical.control_frame_words();
    const bool offline=(control.mode_flags()&7u)==7u;
    const auto posture=look.state().template get<std::uint32_t>(0x4c);
    const auto animation=state.template get<std::int32_t>(0x1d8);
    if(!(offline||control.human_is_player())||(offline&&human.properties.field19c==2)||
            !(look.state().template get<std::uint32_t>(0xb8)==5||(animation>=162&&animation<=165)||(posture!=0&&posture!=1)))return true;
    if(!std::isfinite(angle))throw std::runtime_error("Human posture turn requires its actual finite angle");
    const auto center=human.base().world_position();
    physical.rebind_frame_posture(posture);
    for(int index=0;index<physical.primary_count();++index){
        const auto sphere=physical.posture_spheres().at(static_cast<std::size_t>(index));
        auto origin=sphere;origin[1]+=.2f;
        auto remaining=angle;
        for(;;){
            //0074A21C..0074A276 keeps the nonnegative remainder for a positive
            //request. Exact positive cardinal angles then repeat0/PI/2 forever.
            if(remaining==0&&angle>=0)
                throw std::runtime_error("Human posture turn reached original nonterminating cardinal-angle boundary");
            auto step=remaining;
            if(std::abs(static_cast<long double>(step))>1.5707963705062866)
                step=angle<=0?-1.5707963705062866f:1.5707963705062866f;
            HumanVector delta{{origin[0]-center[0],0,origin[2]-center[2],0}};
            auto rotated=frames.car_transform(delta,frames.human_axis_rotation({{0,1,0,0}},-step)); //00788490.
            rotated[1]=origin[1]-center[1];
            HumanVector destination{},travel{};
            for(unsigned axis=0;axis!=3;++axis){destination[axis]=rotated[axis]+center[axis];travel[axis]=destination[axis]-origin[axis];}
            const auto hit=[&]{
                struct Restore {decltype(physical)& owner;~Restore(){owner.restore_query(false);}} restore{physical};
                physical.exclude_query(false);
                if(index==0||index==physical.primary_count()-1)
                    return services.human_geometry_sweep().sweep(origin,travel,sphere[3],0xc8002u,0x230000u);
                return services.human_geometry_ray(origin,travel,0xc8002u,0x230000u);
            }();
            if(hit){
                origin=hit->position;
                if((static_cast<long double>(hit->normal[2])+hit->normal[0])*0+hit->normal[1]<.55f){
                    const auto queued=state.template get<std::int32_t>(0x1fc);
                    if(queued==43||queued==44){state.set(0x1fc,206u);normalize_human_primary_animation(state,0x1fc);}
                    return false;
                }
            }else origin=destination;
            remaining-=remaining<=0?-1.5707963705062866f:1.5707963705062866f;
            if((remaining>=0&&angle>=0)||(remaining<0&&angle<0))continue;
            break;
        }
    }
    return true;
}

template<class Control,class Look,class Human,class Services,class Frames>
bool advance_human_direction_replay(Control& control,Look& look,Human& human,
        Services& services,Frames& frames,std::int32_t elapsed,bool constrained){ //0076CFE0, native boolean completion.
    auto& state=control.physical().control_frame_words();auto& movement=control.movement().frame_words();
    if(!human_direction_replay_enabled(state))return false;
    const auto target=movement.template get<HumanVector>(0x20);
    const auto current=human.base().world_direction();
    const float horizontal=human_motion_length({{current[0],0,current[2],0}});
    const float threshold=look.state().template get<std::uint32_t>(0x4c)==0?1.2217304706573486f:1.0471975803375244f;
    const auto exact_angle=human_motion_planar_angle(target,current);
    const float angle=static_cast<float>(exact_angle),absolute=static_cast<float>(std::abs(exact_angle));
    const float step_time=static_cast<float>(elapsed);
    if(std::abs(static_cast<long double>(angle))<static_cast<long double>(elapsed)*.00001f)return true;
    float turn=0;
    if(!constrained&&movement.template get<std::int32_t>(0x130)==-10000){
        const auto animation=state.template get<std::int32_t>(0x1d8);
        if(absolute<static_cast<long double>(threshold)*1.6f&&absolute>static_cast<long double>(threshold)*.25f&&
                look.state().template get<std::uint32_t>(0x5c)&&animation>=41&&animation<=65){
            float rate=angle<=0?.0075f:-.0075f;
            if(absolute>2.85f)rate*=14.f;
            else if(absolute>2.55f)rate*=4.f;
            else if(absolute>2.25f)rate*=3.f;
            else if(absolute>1.85f)rate+=rate;
            turn=step_time*rate;
        }else{
            turn=-angle;
            if(state.template get<std::int32_t>(0x950)>0)
                state.set(0x950,state.template get<std::uint32_t>(0x950)+static_cast<std::uint32_t>(elapsed));
        }
    }else{
        auto rate=movement.template get<float>(0x8c);
        if(angle>0)rate=-rate;
        const auto stance=look.state().template get<std::uint32_t>(0x50);
        if(stance==4)rate*=.3f;
        else if(stance!=2&&stance!=5)rate*=.75f;
        turn=step_time*rate;
    }
    const bool complete=std::abs(angle)<=std::abs(turn);
    if(complete){
        if(validate_human_direction_turn(control,look,human,services,frames,angle)||
                ((control.mode_flags()&7u)!=7u&&!control.human_is_player())){
            auto direction=target;direction[1]=0;
            const auto square=static_cast<long double>(direction[2])*direction[2]+static_cast<long double>(direction[0])*direction[0];
            normalize_human_ai_shot(direction,square);
            direction[0]*=horizontal;direction[2]*=horizontal;direction[1]=current[1];
            human.base().world_direction(direction);movement.set(0x50,std::uint8_t{1});look.mark_direction_frame_feedback();return true;
        }
    }else{
        HumanVector flat{{current[0],0,current[2],current[3]}};
        const auto direction=frames.car_transform(flat,frames.human_axis_rotation({{0,1,0,0}},turn));
        if(validate_human_direction_turn(control,look,human,services,frames,-turn)){
            human.base().world_direction({{direction[0],current[1],direction[2],current[3]}});
            movement.set(0x50,std::uint8_t{1});look.mark_direction_frame_feedback();return false;
        }
    }
    const auto model_forward=[&]{
        const auto matrix=frames.actor_world_matrix(human.base().frame());
        HumanVector direction{{matrix[8],matrix[9],matrix[10],matrix[11]}};
        const auto square=(static_cast<long double>(direction[2])*direction[2]+
            static_cast<long double>(direction[1])*direction[1])+static_cast<long double>(direction[0])*direction[0];
        normalize_human_ai_shot(direction,square);return direction;
    };
    look.force_frame_commands(true);
    try{look.current_direction(model_forward());}catch(...){look.force_frame_commands(false);throw;}
    look.force_frame_commands(false);
    state.set(0x240,target);movement.set(0x20,-10000.099609375f);
    human.base().world_direction(model_forward());movement.set(0x130,500u);
    return complete;
}

template<class Control,class Look,class Human,class Publish>
void prepare_human_direction_replay(Control& control,Look& look,Human& human,const HumanVector& requested,
        std::int32_t elapsed,Publish& publish){ //0076DB30, actual presentation input and idle-turn request producer.
    auto& state=control.physical().control_frame_words();auto& movement=control.movement().frame_words();
    HumanVector target{{requested[0],0,requested[2],requested[3]}};
    movement.set(0x20,-10000.099609375f);
    if(static_cast<long double>(target[2])*target[2]+static_cast<long double>(target[0])*target[0]<1.e-16f)return;
    const auto action=state.template get<std::int32_t>(0xa44);
    if(action>0&&action!=4)return;
    const auto angle=human_motion_planar_angle(human.base().world_direction(),target);
    const auto stored=static_cast<float>(angle),absolute=std::abs(stored);
    const auto posture=look.state().template get<std::uint32_t>(0x4c);
    const auto settle=[&]{
        const auto animation=look.state().template get<std::uint32_t>(0x44);
        if(animation==43||animation==44)look.frame_movement_animation(posture==1?55:posture==2?63:45,publish);
    };
    if(absolute<static_cast<long double>(elapsed)*.00001f){
        look.mark_direction_frame_feedback();movement.set(0x20,-10000.099609375f);settle();return;
    }
    float threshold=posture==0?1.2217304706573486f:1.0471975803375244f;
    if(angle<=0&&posture==0)threshold*=.5f;
    const auto base=static_cast<std::int32_t>(static_cast<long double>(threshold)/3.f*1000.f);
    const auto delay=static_cast<std::int32_t>(static_cast<long double>(base)*1.5f);
    const bool small=absolute<static_cast<long double>(elapsed)*.0012f;
    const auto current=state.template get<std::int32_t>(0x1d8);
    const bool idle=current>=41&&current<=65;
    movement.set(0x88,static_cast<std::uint8_t>(!idle&&control.human_is_player()));
    const auto equipment=look.state().template get<std::uint32_t>(0x54);
    const auto secondary=state.template get<std::int32_t>(0x1e8);
    const bool idle_command=posture!=2&&look.state().template get<std::uint32_t>(0x5c)==1&&equipment!=0&&equipment!=6&&
        (secondary<80||secondary>82)&&idle;
    if(!idle_command){state.set(0x950,0u);movement.set(0x20,target);if(!idle)return;}
    else if(absolute<=threshold){
        if(state.template get<std::int32_t>(0x950)<1&&control.human_is_player()){
            movement.set(0x20,-10000.099609375f);settle();return;
        }
        movement.set(0x20,target);
    }else{state.set(0x950,delay);movement.set(0x20,target);}
    if(small)return;
    const auto command=look.state().template get<std::int32_t>(0x44);
    if(command>=41&&command<=65&&current!=43&&current!=44)
        look.frame_movement_animation(stored<=0?43:44,publish);
}

} //namespace hd2::game
#endif
