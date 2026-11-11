#ifndef HD2_GAME_HUMAN_MOTION_VELOCITY_H
#define HD2_GAME_HUMAN_MOTION_VELOCITY_H

#include "game/human_motion_tables.h"
#include "game/human_ai_shot_direction.h"

namespace hd2::game {

inline long double human_motion_square(const HumanVector& value){
    return (static_cast<long double>(value[2])*value[2]+static_cast<long double>(value[0])*value[0])+
        static_cast<long double>(value[1])*value[1];
}
inline float human_motion_length(const HumanVector& value){
    const float square=static_cast<float>(human_motion_square(value));
    if(square<1.e-8f)return 0;
    return std::abs(static_cast<long double>(square)-1)<1.e-8f?1.f:static_cast<float>(std::sqrt(static_cast<long double>(square)));
}
inline long double human_motion_planar_angle(const HumanVector& first,const HumanVector& second){ //007885B0, XZ caller projection.
    auto angle=std::atan2(static_cast<long double>(second[2]),static_cast<long double>(second[0]))-
        std::atan2(static_cast<long double>(first[2]),static_cast<long double>(first[0]));
    if(angle< -3.1415927410125732f)angle+=6.2831854820251465f;
    if(angle>3.1415927410125732f)angle-=6.2831854820251465f;
    return angle;
}

//0076B670 selects animation-driven velocity and playback rates through the
//actual AC0, DD0, E0 and track scalar owners.
template<class Control,class Look,class Human,class Services,class Frames>
void prepare_human_motion_velocity(Control& control,Look& look,Human& human,
        Services& services,Frames& frames,bool refresh){
    auto& state=control.physical().control_frame_words();
    auto& movement=control.movement().frame_words();auto& tracks=control.tracks();
    const auto current=state.template get<std::int32_t>(0x1d8);
    const auto detail=state.template get<std::int32_t>(0x1dc),variant=state.template get<std::int32_t>(0x1e0);
    const auto queued=state.template get<std::int32_t>(0x1fc);
    const auto selected=queued>=8&&queued<=14?queued:current; //00782FA0 copies the selected descriptor only.
    auto input=look.state().template get<HumanVector>(0x10);
    HumanVector forced{};
    float scale=1.f,speed=0;
    if((control.mode_flags()&7u)==7u&&!control.human_is_player()&&human.properties.field19c==1)scale=1.1f;
    tracks.motion_animation_scale(scale);
    unsigned direction_kind=0;
    bool reset=false;
    const auto rate=[&](float value){tracks.motion_animation_rate(0,value);};
    const auto publish=[&](const HumanVector& direction,float amount,float maximum,bool preserve){
        auto output=movement.template get<HumanVector>(0xb0);
        if(preserve)tracks.refresh_motion_velocity(direction,amount,maximum,output);
        else tracks.motion_velocity(direction,amount,maximum,output);
        control.movement().control_frame_track_velocity(output);
    };
    const auto rotate=[&](HumanVector value,float angle){ //00788490, actual engine quaternion/matrix/vector receivers.
        return frames.car_transform(value,frames.human_axis_rotation({{0,1,0,0}},angle));
    };
    const auto table=[&](std::uint32_t address){return human_motion_table_value(address,static_cast<std::int64_t>(detail)*6+variant);};
    switch(selected){
    case 18:case 23:case 24:
        if(movement.template get<std::uint8_t>(0x104)){
            forced=human.base().world_direction();
            for(unsigned axis=0;axis!=3;++axis)forced[axis]=-forced[axis];
        }
        if(selected==18){if(!detail)speed=human_motion_table_value(0x816b00u,variant);}
        else if(selected==23){if(detail==1)speed=human_motion_table_value(0x816b60u,variant);}
        else speed=table(0x816ab8u);
        break;
    case 30:case 31:case 32:case 33:{
        if(selected==31||selected==33)rate(1.3f);
        const auto& action=control.action().state();
        const auto command=action.template get<std::int32_t>(0x10);
        if(action.template get<std::int32_t>(0x20)==-10000||command<190){reset=true;break;}
        const bool backward=command==191||command==199;
        if(selected==31||selected==33)speed=human_motion_table_value(backward?0x8168f0u:0x8168a8u,variant);
        else speed=human_motion_table_value(backward?0x8168c0u:0x816878u,variant+(selected==32?0:6));
        break;
    }
    case 90:case 92:{
        forced=state.template get<HumanVector>(0x940);
        if(control.human_is_player()){
            const float horizontal=human_motion_length({{forced[0],0,forced[2],0}});
            const auto direction=human.base().world_direction();
            forced[0]=direction[0]*horizontal;forced[2]=direction[2]*horizontal;
            normalize_human_ai_shot(forced,human_motion_square(forced));
        }
        const bool backward=movement.template get<std::uint8_t>(0x12c)!=0;
        if(backward)for(unsigned axis=0;axis!=3;++axis)forced[axis]=-forced[axis];
        const auto row=selected==90?11:12;
        if(look.state().template get<std::uint32_t>(0x50)==2){
            const auto amount=backward?.25f:1.f;
            speed=amount*human_motion_table_value(0x816880u,row*18);rate(amount*1.5f);
        }else{
            const auto amount=backward?.4f:1.f;
            speed=amount*human_motion_table_value(0x81687cu,row*18);rate(amount);
        }
        break;
    }
    case 106:case 107:case 108:{
        if(tracks.words().template get<std::int32_t>(0x14)>0){
            publish({{0,0,0,0}},0,0,false);return; //Native106's unused zero-speed direction is unwritten stack data.
        }
        if(control.physical().control_action_actor()){
            const auto& ladder=services.human_ladder_geometry(control.physical().control_action_actor());
            for(unsigned axis=0;axis!=3;++axis)
                forced[axis]=selected==106?ladder.up_position()[axis]-ladder.down_position()[axis]:
                    ladder.down_position()[axis]-ladder.up_position()[axis];
        }
        const auto mode=look.state().template get<std::uint32_t>(0x50);
        if(selected==106&&(mode==2||mode==5)){speed=human_motion_table_value(0x816c24u,0)*1.25f;rate(1.75f);}
        else if(selected==108){speed=human_motion_table_value(0x816c28u,0);rate(1.f);}
        else{speed=human_motion_table_value(0x816c24u,0);rate(1.4f);}
        break;
    }
    case 190:case 198:speed=table(0x816878u);break;
    case 191:case 199:direction_kind=1;speed=table(0x8168c0u);break;
    case 192:case 193:case 194:case 195:case 196:case 197:
    case 200:case 201:case 202:case 203:case 204:case 205:{
        const bool left=selected==192||selected==194||selected==196||selected==200||selected==202||selected==204;
        direction_kind=left?2u:3u;
        auto angle=human_motion_square(input)>=1.e-16f?human_motion_planar_angle(input,human.base().world_direction()):0.L;
        if(!left){
            angle=static_cast<float>(angle);
            if(angle< -3.f)angle=-angle;
        }
        if(left?angle>.1f:angle<-.1f)
            look.position_command(rotate(human.base().world_direction(),left?1.5707963705062866f:-1.5707963705062866f));
        std::uint32_t address=0x816950u;
        switch(selected){
        case 192:case 200:address=0x816908u;break;
        case 194:case 202:address=0x816998u;break;
        case 195:case 203:address=0x8169e0u;break;
        case 196:case 204:address=0x816a28u;break;
        case 197:case 205:address=0x816a70u;break;
        default:break;
        }
        speed=table(address);input=look.state().template get<HumanVector>(0x10);
        break;
    }
    default:
        if(selected>=8&&selected<=14)rate(1.3f);
        reset=true;break;
    }
    if(reset)publish({{0,0,0,0}},0,0,refresh);
    speed=static_cast<float>(static_cast<long double>(services.human_control_animation_scale())*scale*speed);
    const auto transition=state.template get<std::int32_t>(0xa30);
    if(transition&&transition!=-10000){
        auto value=speed*.5f;if(value<.1f)value=.1f;
        services.human_presentation_equipment(human).state().words().set(0xd0,value);
    }
    if(reset)return;
    const bool climbing=(current>=106&&current<=108)||current==8||current==14;
    const bool special=climbing||current==90||current==92;
    const auto input_square=human_motion_square(input);
    if(input_square>=1.e-16f&&!special){
        const float horizontal=human_motion_length({{input[0],0,input[2],0}});
        HumanVector direction{};
        if(horizontal>=1.e-8f){
            const auto inverse=1.L/horizontal;
            direction[0]=static_cast<float>(input[0]*inverse);direction[2]=static_cast<float>(input[2]*inverse);
        }
        publish(direction,speed,horizontal,refresh);return;
    }
    HumanVector direction{};
    if((queued<41||queued>65)||(refresh&&control.human_is_player())||special){
        if(human_motion_square(forced)>=1.e-16f){direction=forced;if(!special)direction[1]=0;}
        else{direction=human.base().world_direction();if(!climbing)direction[1]=0;}
        const auto square=(static_cast<long double>(direction[0])*direction[0]+
            static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[1])*direction[1];
        normalize_human_ai_shot(direction,square);
        if(direction_kind==1)for(unsigned axis=0;axis!=3;++axis)direction[axis]=-direction[axis];
        else if(input_square<1.e-16f&&(direction_kind==2||direction_kind==3))
            direction=rotate(direction,direction_kind==2?1.5707963705062866f:-1.5707963705062866f);
    }
    publish(direction,speed,0,refresh);
}

} //namespace hd2::game
#endif
