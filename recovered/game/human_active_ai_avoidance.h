#ifndef HD2_GAME_HUMAN_ACTIVE_AI_AVOIDANCE_H
#define HD2_GAME_HUMAN_ACTIVE_AI_AVOIDANCE_H
#include "game/mission_navigation.h"
#include "game/human_joint_owners.h"
#include <cmath>
namespace hd2::game {
namespace human_active_avoidance {
//0052D720 retains normalized X in x87 through radius multiplication; Y/Z
//spill to FLOAT. The squared length has its own native spill/compare points.
inline void normalize(long double& x,float& y,float& z,long double square){
    constexpr float epsilon=9.99999993922529e-09f;
    if(std::abs(square-1.f)<epsilon)return;
    const float stored=static_cast<float>(square);
    if(!(stored<epsilon)){
        const auto scale=1.L/std::sqrt(static_cast<long double>(stored));
        x*=scale;y=static_cast<float>(static_cast<long double>(y)*scale);z=static_cast<float>(static_cast<long double>(z)*scale);
    }else if(x!=0)x=x<0?-1.f:1.f;
    else if(z!=0)z=z<0?-1.f:1.f;
    else y=y<0?-1.f:1.f;
}
inline void normalize_xz(long double& x,float& y,long double& z,long double square){
    constexpr float epsilon=9.99999993922529e-09f;if(std::abs(square-1.f)<epsilon)return;
    const float stored=static_cast<float>(square);
    if(!(stored<epsilon)){const auto scale=1.L/std::sqrt(static_cast<long double>(stored));x*=scale;y=static_cast<float>(static_cast<long double>(y)*scale);z*=scale;}
    else if(x!=0){x=x<0?-1.f:1.f;z=static_cast<float>(z);} //Native tiny-X path reloads stored Z.
    else if(z!=0)z=z<0?-1.f:1.f;
    else y=1;
}
inline void normalize_z(float& x,float& y,long double& z,long double square){
    constexpr float epsilon=9.99999993922529e-09f;if(std::abs(square-1.f)<epsilon)return;
    const float stored=static_cast<float>(square);
    if(!(stored<epsilon)){const auto scale=1.L/std::sqrt(static_cast<long double>(stored));x=static_cast<float>(static_cast<long double>(x)*scale);y=static_cast<float>(static_cast<long double>(y)*scale);z*=scale;}
    else if(x!=0)x=x<0?-1.f:1.f;
    else if(z!=0)z=z<0?-1.f:1.f;
    else y=y<0?-1.f:1.f;
}
}
//0052D720 uses actual GAME+54 navigation, not a second table or private route.
inline void clamp_active_avoidance_position(MissionNavigation& navigation,HumanVector& proposed,bool enabled,const HumanVector& original){
    const auto lookup=[&](const HumanVector& p){return navigation.nearest_volume({{p[0],p[1],p[2]}},5,-1);};
    const auto destination=lookup(proposed);if(destination==0xffff||!enabled)return; //FIRST query even when disabled.
    const auto origin=original;const auto source=lookup(origin);if(source==0xffff)return;
    if(source!=destination){
        std::vector<std::uint16_t> route;
        if(navigation.find_route(destination,source,0u,route)&&!(navigation.route_length(route)>4.f))return;
        const auto* point=navigation.record(source);if(!point)throw std::logic_error("Avoidance clamp lost original navigation point");
        const auto centre=navigation.position(source);const auto radius=static_cast<unsigned char>(point->prefix[0x13])*.0625f;
        proposed[1]=0;
        auto x=static_cast<long double>(proposed[0])-origin[0];float y=0,z=static_cast<float>(static_cast<long double>(proposed[2])-origin[2]);
        const float stored_x=static_cast<float>(x);
        const auto square=static_cast<long double>(stored_x)*stored_x+static_cast<long double>(z)*z;
        human_active_avoidance::normalize(x,y,z,square);
        const float offset_x=static_cast<float>(x*radius),offset_y=static_cast<float>(static_cast<long double>(y)*radius);
        proposed={{static_cast<float>(static_cast<long double>(offset_x)+centre[0]),static_cast<float>(static_cast<long double>(offset_y)+centre[1]),static_cast<float>(static_cast<long double>(z)*radius+centre[2]),origin[3]}};
        proposed[1]=centre[1];return;
    }
    const auto* point=navigation.record(destination);if(!point)throw std::logic_error("Avoidance clamp lost matching navigation point");
    const auto centre=navigation.position(destination);proposed[1]=0;
    const auto radius=static_cast<unsigned char>(point->prefix[0x13])*.0625f;
    auto x=static_cast<long double>(static_cast<float>(static_cast<long double>(proposed[0])-centre[0]));float y=0,z=static_cast<float>(static_cast<long double>(proposed[2])-centre[2]);
    const auto square=(static_cast<long double>(y)*y+static_cast<long double>(z)*z)+x*x;
    if(static_cast<long double>(radius)*radius<square){
        human_active_avoidance::normalize(x,y,z,square);
        const float offset_x=static_cast<float>(x*radius),offset_y=static_cast<float>(static_cast<long double>(y)*radius),offset_z=static_cast<float>(static_cast<long double>(z)*radius);
        proposed[0]=static_cast<float>(static_cast<long double>(centre[0])+offset_x);proposed[1]=offset_y;proposed[2]=static_cast<float>(static_cast<long double>(centre[2])+offset_z);
        //Native same-point clipping temporary W is unwritten. Retain the
        //typed proposal lane without claiming a native W value.
    }
    proposed[1]=centre[1];
}
//0052DC10. Native cross/normalization phases retain DIFFERENT x87 components.
//Only after those calculations do40000/E0-floor gates suppress navigation.
template<class AI,class World>void avoid_active_position(AI& ai,const HumanVector& position,const HumanVector& input,bool blend,bool check_angle,World& world){
    if(ai.resources().query().state.template get<std::uint32_t>(0x50)&0x10000000u)return;
    auto* human=ai.human();if(!human)throw std::logic_error("Active avoidance lost Human44");
    const auto validate=world.human_common_receiver_guard(*human);const auto cached=human->base().world_position();
    auto ax=static_cast<long double>(cached[0])-position[0],az=static_cast<long double>(cached[2])-position[2];float ay=0;
    human_active_avoidance::normalize_xz(ax,ay,az,az*az+ax*ax);
    auto direction=input;direction[1]=0;
    const auto direction_square=static_cast<long double>(direction[2])*direction[2]+static_cast<long double>(direction[0])*direction[0];
    auto dx=static_cast<long double>(direction[0]);float dy=0,dz=direction[2];
    human_active_avoidance::normalize(dx,dy,dz,direction_square);direction[0]=static_cast<float>(dx);direction[1]=dy;direction[2]=dz;
    const auto dot=(static_cast<long double>(direction[2])*az+static_cast<long double>(direction[0])*ax)+static_cast<long double>(ay)*direction[1];
    if(check_angle&&dot<.85f)return;
    float cx=static_cast<float>(static_cast<long double>(direction[1])*az-static_cast<long double>(direction[2])*ay);
    float cy=static_cast<float>(static_cast<long double>(direction[2])*ax-az*direction[0]);
    auto cz=static_cast<long double>(direction[0])*ay-ax*direction[1];
    human_active_avoidance::normalize_z(cx,cy,cz,(cz*cz+static_cast<long double>(cy)*cy)+static_cast<long double>(cx)*cx);
    auto x=static_cast<long double>(cy)*direction[2]-cz*direction[1];
    float y=static_cast<float>(static_cast<long double>(static_cast<float>(cz))*direction[0]-static_cast<long double>(direction[2])*cx);
    const auto extended_z=static_cast<long double>(cx)*direction[1]-static_cast<long double>(cy)*direction[0];float z=static_cast<float>(extended_z);
    human_active_avoidance::normalize(x,y,z,(extended_z*z+static_cast<long double>(y)*y)+x*x);
    if(blend){x+=static_cast<long double>(direction[0])*.15f;y=static_cast<float>(static_cast<long double>(direction[1])*.15f+y);z=static_cast<float>(static_cast<long double>(direction[2])*.15f+z);}
    human_active_avoidance::normalize(x,y,z,(static_cast<long double>(z)*z+static_cast<long double>(y)*y)+x*x);
    const auto original=human->base().world_position(); //Fresh cached position AFTER the native arithmetic.
    HumanVector proposed{{static_cast<float>(static_cast<long double>(original[0])+x),static_cast<float>(static_cast<long double>(original[1])+y),static_cast<float>(static_cast<long double>(original[2])+z),input[3]}};
    if(ai.words().template get<std::uint32_t>(0x50)&0x40000u)return;
    if(world.active_follow_floor(*human))return;
    world.active_avoidance_clamp(proposed,true,original);validate();ai.resources().query().request_position(proposed);
    ai.state().set(0x18c,std::uint8_t{1}); //BEFORE leader/formation queries and slot/route effects.
    if(human->formation_leader){
        auto* leader=human->formation_leader;auto* formation=leader->formation.get();if(!formation)return;
        const auto members_live=world.active_follow_guard(*human,*leader,formation);
        if(!formation->has_slots())throw std::runtime_error("Active avoidance requires actual formation member slots");
        formation->request_member_position(human,proposed);members_live();
        std::int32_t index=0;if(formation->contains(human,&index)){formation->update_member_route(static_cast<unsigned>(index));members_live();}
    }
}
//0052E310, selected ordinary on-foot avoidance. The two selected-player
//lookups are distinct; native attachment gates are genuine early returns.
template<class AI,class World>void avoid_active_selected_player(AI& ai,World& world){
    auto& state=ai.state();if(state.template get<std::uint32_t>(0x174)&0x20u)return;
    if(ai.resources().query().state.template get<std::uint32_t>(0x50)&0x10000000u)return;
    if(!world.active_avoidance_player())return;
    auto* human=ai.human();if(!human)throw std::logic_error("Active player avoidance lost Human44");
    if(human->field160)return;
    auto* player=world.active_avoidance_player();if(!player)throw std::runtime_error("Active avoidance repeated selected-player query lost Human");
    if(player->field160&&player->field160->base().type()==4)return;
    const auto a=human->base().world_position(),b=player->base().world_position();
    const auto x=static_cast<long double>(a[0])-b[0],y=static_cast<long double>(a[1])-b[1],z=static_cast<long double>(a[2])-b[2];
    const auto posture=ai.words().template get<std::uint32_t>(0x50)&7u;
    if(!(((z*z+y*y)+x*x)<(posture==1?2.25f:posture==2?1.f:.36f)))return;
    auto direction=player->activation.vector1b0;
    const auto length=[&]{return (static_cast<long double>(direction[1])*direction[1]+static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[0])*direction[0];};
    if(length()==0)direction=player->base().world_direction();
    auto dx=static_cast<long double>(direction[0]);float dy=direction[1],dz=direction[2];
    human_active_avoidance::normalize(dx,dy,dz,length());direction[0]=static_cast<float>(dx);direction[1]=dy;direction[2]=dz;
    ai.avoid_frame_position(player->base().world_position(),direction,true,false);
}
//00526550's on-foot phase through0052D460, after prefix events/mode expiry.
//Occupied continuation is owned separately by H8.4 and never enters this API.
template<class AI,class Frames,class Inline,class World,class Runtime>void advance_active_onfoot_navigation(AI& ai,std::uint32_t elapsed,bool following,Frames& frames,Inline& control,World& world,Runtime& runtime){
    auto* human=ai.human();if(!human||human->field160)throw std::logic_error("Active on-foot navigation requires its actual unattached Human");
    const auto validate=world.human_common_receiver_guard(*human);
    ai.avoid_selected_player();validate();ai.refresh_item();ai.resources().reconcile_direction_flags();
    const auto result=ai.advance_frame_query(control,world,elapsed,runtime);validate();auto& state=ai.state();
    switch(result){
    case 4:
        if(!state.template get<std::uint8_t>(0x202)){if(!state.template get<std::uint8_t>(0x206))state.set(0x200,std::uint8_t{1});}
        else if(state.template get<std::uint8_t>(0x189))state.set(0x1a4,0u);
        if(state.template get<std::uint8_t>(0x18c))state.set(0x18c,std::uint8_t{0});
        break;
    case 0xfffffff6u:
        if(!state.template get<std::uint8_t>(0x202)&&(state.template get<std::uint32_t>(0x220)==25||state.template get<std::uint32_t>(0x220)==26))
            throw std::runtime_error("Active query -10 requires actual00532190 specialized task25/26 replanning before00534340");
        break;
    case 0xfffffffau:ai.reject_pending_task();validate();break;
    case 0xfffffffbu:case 0xfffffffcu:
        if(!state.template get<std::uint8_t>(0x202)&&!state.template get<std::uint8_t>(0x200)){
            ai.require_live_task_target();
            if(!ai.task_target)ai.resources().query().request_position(state.template get<HumanVector>(0x1e0));
            else{(void)frames.actor_world_matrix(ai.task_target->base().frame());validate();} //Native lazy update, no new target-position request.
        }
        break;
    default:break;
    }
    ai.select_frame_navigation_gait(following);
}
} //namespace hd2::game
#endif
