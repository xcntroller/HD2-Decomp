#ifndef HD2_GAME_HUMAN_VEHICLE_PATH_H
#define HD2_GAME_HUMAN_VEHICLE_PATH_H
#include "game/human_ai_resources.h"
#include "game/mission_navigation.h"
#include "game/rigid_body_motion_math.h"
#include "game/mission_script_player_predicates.h"
namespace hd2::game::human_vehicle {
inline float store(long double v){return static_cast<float>(v);}
inline long double square(const HumanVector& v){return (static_cast<long double>(v[1])*v[1]+static_cast<long double>(v[2])*v[2])+static_cast<long double>(v[0])*v[0];}
inline HumanVector delta(const HumanVector& a,const HumanVector& b){HumanVector v{};for(unsigned n=0;n<3;++n)v[n]=store(static_cast<long double>(a[n])-b[n]);return v;}
inline bool negative(float v){std::uint32_t bits;std::memcpy(&bits,&v,4);return (bits&0x80000000u)!=0;}
inline HumanVector unit(HumanVector v,unsigned order=0){
    const auto x=static_cast<long double>(v[0])*v[0],y=static_cast<long double>(v[1])*v[1],z=static_cast<long double>(v[2])*v[2];
    const auto extended=order==0?(x+y)+z:x+z;const float value=store(extended);
    if(std::abs(extended-1)>=9.99999993922529e-9f){if(value>=9.99999993922529e-9f){const auto inverse=1.L/std::sqrt(static_cast<long double>(value));for(unsigned n=0;n<3;++n)v[n]=store(static_cast<long double>(v[n])*inverse);}
        else if(v[0]!=0)v[0]=v[0]<0?-1.f:1.f;else if(v[2]!=0)v[2]=v[2]<0?-1.f:1.f;else v[1]=v[1]<0?-1.f:1.f;}
    return v;
}
//0055F320: append native30-byte path records to the SAME buffer. Failed route
//search still appends the actual endpoint; it is not a successful empty route.
inline void append_route(HumanAIPathLoading& path,MissionNavigation& navigation,const HumanVector& start,const HumanVector& end,const HumanVector& tail){
    auto& points=path.route();const auto old=points.size();
    const auto append=[&](const HumanVector& p,bool terminal){HumanAIPathLoading::RoutePoint v;v.position=p;v.terminal=terminal;points.push_back(v);};
    if(points.empty())append(start,false);
    else if(square(delta(start,points.back().position))<=9.99999993922529e-9f)points.back().terminal=0;
    else append(start,false);
    const auto from=navigation.nearest_volume({{start[0],start[1],start[2]}},10.f,-1.f);
    const auto to=navigation.nearest_volume({{end[0],end[1],end[2]}},10.f,-1.f);
    std::vector<std::uint16_t> route;
    if(from!=0xffff&&to!=0xffff&&navigation.find_route(from,to,0,route)){
        for(std::size_t i=0;i<route.size();++i){
            const auto p=navigation.position(route[i]);HumanVector centre{{p[0],p[1],p[2],0}},next=end;
            if(i+1<route.size()){const auto q=navigation.position(route[i+1]);next={{q[0],q[1],q[2],0}};}
            const auto* record=navigation.record(route[i]);if(!record)throw std::logic_error("Vehicle route lost actual navigation point");
            const float radius=static_cast<unsigned char>(record->prefix[0x13])*.0625f;
            const auto back=delta(points.back().position,centre),forward=delta(next,centre);auto across=delta(forward,back);across[1]=0;
            const float before=store((static_cast<long double>(across[0])*back[0]+static_cast<long double>(across[2])*back[2])+static_cast<long double>(across[1])*0);
            const float after=store((static_cast<long double>(across[0])*forward[0]+static_cast<long double>(across[2])*forward[2])+static_cast<long double>(across[1])*0);
            if(!negative(before)||negative(after))continue;
            auto normal=unit({{across[2],0,-across[0],0}},1);
            float offset=store((static_cast<long double>(back[0])*normal[0]+static_cast<long double>(back[2])*normal[2])+static_cast<long double>(normal[1])*0);
            if(offset>radius)offset=radius;else if(offset < -radius)offset=-radius;
            HumanVector point{};for(unsigned n=0;n<3;++n){const float step=store(static_cast<long double>(normal[n])*offset);point[n]=store(static_cast<long double>(centre[n])+step);}
            if(square(delta(point,points.back().position))>=9.99999993922529e-9f)append(point,false);
        }
    }
    if(square(delta(end,points.back().position))<=9.99999993922529e-9f)points.back().terminal=1;else append(end,true);
    std::uint32_t tail_bits;std::memcpy(&tail_bits,&tail[0],4);const bool absent=tail_bits==0x7fc00000u||tail_bits==0xffc00000u;
    for(std::size_t i=old?old-1:0;i<points.size();++i){
        if(points.size()<2)throw std::runtime_error("Vehicle route requires native adjacent-point input");
        auto& output=points[i].tangent;
        for(unsigned n=0;n<3;++n){
            const auto here=points[i].position[n];
            if(!i)output[n]=store((static_cast<long double>(points[1].position[n])-here)*.5f);
            else if(i+1==points.size()&&absent)output[n]=store((static_cast<long double>(here)-points[i-1].position[n])*.5f);
            else {const auto previous=static_cast<long double>(here)-points[i-1].position[n];const auto next=static_cast<long double>(i+1==points.size()?tail[n]:points[i+1].position[n])-here;output[n]=store((next+previous)*.5f);}
        }
    }
}
//00579760/00579D50: source path/Car owners are mandatory. Output order differs
//for reverse, including brake versus first-pedal order. No driver-input remap.
template<class State,class World>void steer(State& state,void* car,const HumanVector& position,HumanVector heading,const HumanVector& target,const HumanVector& tangent,bool terminal,const HumanVector& endpoint,bool reverse,World& world){
    const auto speed=world.human_vehicle_speed(car);auto desired=delta(target,position);desired[1]=0;
    const float distance=rigid_motion::length(desired);float blend=store((3.L-distance)*.3333333432674408f);if(negative(blend))blend=0;
    if(distance>9.99999993922529e-9f)desired=rigid_motion::scale(desired,1.L/distance);
    HumanVector direction{};const auto remain=1.L-blend;
    for(unsigned n=0;n<3;++n)direction[n]=store(static_cast<long double>(blend)*tangent[n]+static_cast<long double>(desired[n])*remain);
    direction=unit(direction);heading[1]=0;heading=unit(heading,1);
    const float cosine=store((static_cast<long double>(direction[2])*heading[2]+static_cast<long double>(direction[1])*heading[1])+static_cast<long double>(direction[0])*heading[0]);
    float steering;
    if(!negative(cosine)){
        const auto side=store((static_cast<long double>(direction[1])*0+static_cast<long double>(direction[0])*heading[2])+static_cast<long double>(direction[2])*-heading[0]);
        //007E67D0 is acos of the clamped signed SIDE product (579B6C..BA6).
        const auto angle=std::acos(std::max(-1.L,std::min(1.L,static_cast<long double>(side))));
        steering=store((reverse?angle-1.5707963705062866f:1.5707963705062866f-angle)*.6366197466850281f);
    }else{
        const float side=store((static_cast<long double>(direction[1])*0+static_cast<long double>(direction[0])*heading[2])+static_cast<long double>(direction[2])*-heading[0]);
        steering=negative(side)?(reverse?1.f:-1.f):(reverse?-1.f:1.f);
    }
    auto& words=state.resources().path().state();float maximum;
    if(state.words().template get<std::uint32_t>(0x48)==5u)maximum=std::abs(words.template get<float>(0x8c));
    else{const auto flags=words.template get<std::uint32_t>(0x90);const auto limit=world.human_vehicle_maximum(car);maximum=store(static_cast<long double>(limit)*(flags==0x4000?1.L:flags==0x2000?.75L:(flags==0x1000||flags==0x8000)?.25L:flags==0x10000?1.25L:.1));}
    auto amount=(1.L-std::abs(static_cast<long double>(steering)))*(reverse?.800000011920929:.699999988079071)+(reverse?.20000000298023224:.30000001192092896);
    if(terminal){float factor=store(square(delta(position,endpoint))*.0625f);if(factor>1)factor=1;amount*=factor;}
    const float wanted=store(amount*maximum);world.human_vehicle_governor(car,std::abs(wanted));
    float ratio;
    if(wanted<=speed){if(reverse)world.human_vehicle_brake(car,0.f);else world.human_vehicle_drive(car,0.f);ratio=store((static_cast<long double>(speed)-wanted)/speed);}
    else{if(reverse)world.human_vehicle_brake(car,store((static_cast<long double>(wanted)-speed)/wanted));else world.human_vehicle_drive(car,store((static_cast<long double>(wanted)-speed)/wanted));ratio=0;}
    if(reverse)world.human_vehicle_drive(car,ratio);else world.human_vehicle_brake(car,ratio);
    world.human_vehicle_steering(car,steering);world.human_vehicle_emergency(car,false,false);
}
//0051E1A0→0051C800, actual16-byte case-insensitive point name.
inline std::uint16_t named_point(MissionTableSet& table,const std::string& name){

        if(table.count0c&&!table.data10)throw std::logic_error("Vehicle named point requires actual SECOND table names");
        const auto* records=static_cast<const MissionTableVectorRecord*>(table.data10);
        for(unsigned i=0;i<table.count0c;++i){const auto* bytes=reinterpret_cast<const unsigned char*>(records[i].words.data());bool match=true;
            for(unsigned n=0;n<16;++n){const unsigned a=bytes[n],b=n<name.size()?static_cast<unsigned char>(name[n]):0u;
                const auto fold=[](unsigned c){return c>='A'&&c<='Z'?c+32:c;};
                if(!a||!b){if(a!=b)match=false;break;}if(fold(a)!=fold(b)){match=false;break;}}
            if(match)return records[i].value;
        }
        return 0xffff;

}
//00421C00 op2B: reverse roster, nearest eligible actor and filtered reachable points.
template<class Frame,class World>std::optional<std::uint16_t> chase_point(Frame frame,World& world){
        if(!frame)return std::nullopt;
        auto& roster=world.human_vehicle_roster();using Target=std::remove_pointer_t<decltype(roster.at(0))>;
        Target* nearest=nullptr;float best=std::numeric_limits<float>::max();
        for(auto n=roster.size();n;){auto* target=roster.at(static_cast<std::int32_t>(--n));
            if(!target)throw std::logic_error("Vehicle chase lost actual roster Human");
            if(human_frame_signed_word(target->properties.stats_ec[0])<=0||world.human_selected_suppressed(*target)||!script_human_player_controlled(*target,world.mode_flags())||!target->base().frame())continue;
            const auto a=world.human_command_frames().actor_world_matrix(frame),b=world.human_command_frames().actor_world_matrix(target->base().frame());
            const auto x=static_cast<long double>(b[12])-a[12],y=static_cast<long double>(b[13])-a[13],z=static_cast<long double>(b[14])-a[14];const float distance=static_cast<float>((z*z+y*y)+x*x);
            if(distance<best){best=distance;nearest=target;}
        }
        if(!nearest)return std::nullopt;
        MissionNavigation navigation(world.human_vehicle_second_tables());const auto position=nearest->base().world_position();const auto start=navigation.nearest_volume({{position[0],position[1],position[2]}},20.f,-1.f);
        if(start==0xffff)return std::nullopt;
        std::vector<std::uint16_t> points;if(!navigation.append_reachable(start,20.f,points))return std::nullopt;
        for(auto n=points.size();n;){--n;const auto* point=navigation.record(points[n]);if(point&&(point->flags&0x100u)){points[n]=points.back();points.pop_back();}}
        return points.empty()?std::uint16_t{0xffff}:points[world.human_frame_random_word()%points.size()];

}
//005796D0: clear running2 and restore governor before the Script PC owner.
template<class State,class World>bool finish(State& state,void* car,World& world){
    state.resources().path().clear_flags(2);world.human_vehicle_governor(car,world.human_vehicle_maximum(car));
    if(state.words().template get<std::uint32_t>(0x48)==5u)world.human_vehicle_finish_script(state,state.resources().path().state().template get<std::int32_t>(0x94));
    else if(world.human_vehicle_human_kind(*state.human())==1u)world.human_vehicle_emergency(car,true,true);
    return true;
}
//00578770→00578790: path flags, actual driver gate, stuck recovery, forward
//Hermite samples/reverse endpoint and SAME Car control receivers.
template<class State,class World>bool advance(State& state,std::uint32_t elapsed,World& world){
    auto& path=state.resources().path();auto& words=path.state();auto* car=path.vehicle();
    auto* human=state.human();if(!car||!human||!human->field160)return false;
    auto* attached=human->field160;bool result=false;
    if(path.flags(4)){path.clear_flags(1);path.clear_flags(2);path.clear_flags(4);path.reset();world.human_vehicle_governor(car,world.human_vehicle_maximum(car));world.human_vehicle_emergency(car,true,true);result=finish(state,car,world);}
    if(path.flags(1)){
        path.clear_flags(1);
        if(!human->field160||world.human_pose_actor_type(human->field160)!=4u||human->field164!=0)path.clear_flags(2);
        else{
            path.set_flags(2);path.reset();const auto position=world.human_vehicle_position(attached);
            auto navigation=world.human_vehicle_navigation(path);append_route(path,navigation,position,words.template get<HumanVector>(0x50),words.template get<HumanVector>(0x60));
            world.human_vehicle_governor(car,world.human_vehicle_maximum(car));world.human_vehicle_driver_input(car,0,0);world.human_vehicle_brake(car,0);world.human_vehicle_emergency(car,false,false);
            words.set(0x18,0x4cbebc20u);words.set(0x24,0u);words.set(0x1d,std::uint8_t{0});words.set(0x30,world.human_vehicle_position(car));
        }
    }
    if(!path.flags(2))return result;
    if(!elapsed)throw std::runtime_error("Vehicle running path requires native nonzero elapsed divisor");
    auto& route=path.route();std::uint32_t index=words.template get<std::uint16_t>(0x10);
    const float seconds=store(static_cast<long double>(elapsed)*.0010000000474974513f),factor=store(static_cast<long double>(seconds)*4.f);
    const auto velocity=world.human_vehicle_velocity(car);auto predicted=world.human_vehicle_position(attached);
    for(unsigned n=0;n<3;++n){const float step=store(static_cast<long double>(factor)*velocity[n]);predicted[n]=store(static_cast<long double>(step)+predicted[n]);}
    auto heading=world.human_vehicle_direction(car);const auto old=words.template get<HumanVector>(0x30),current=world.human_vehicle_position(car);const auto moved=delta(current,old);words.set(0x30,world.human_vehicle_position(car));
    const float inverse=store(1.L/(static_cast<long double>(seconds)*seconds));
    const auto motion=static_cast<long double>(inverse)*(static_cast<long double>(moved[0])*moved[0]+static_cast<long double>(moved[2])*moved[2]);
    if(!words.template get<std::uint8_t>(0x1d)){if(motion>.0024999999441206455f)words.set(0x1d,std::uint8_t{1});}
    else if(motion>=.0024999999441206455f){
        if(index+1>=route.size())throw std::runtime_error("Vehicle progress requires actual next route point");
        const float distance=store(square(delta(route[index+1].position,world.human_vehicle_position(attached))));
        if(distance<=words.template get<float>(0x18)){words.set(0x18,distance);words.set(0x24,0u);}
        else{words.set(0x24,words.template get<std::uint32_t>(0x24)+elapsed);if(words.template get<std::int32_t>(0x24)>5000){++index;if(static_cast<std::uint16_t>(index)==route.size()-1)return finish(state,car,world);words.set(0x24,0u);words.set(0x18,0x4cbebc20u);}}
    }else{
        words.set(0x20,words.template get<std::uint32_t>(0x20)+elapsed);
        if(words.template get<std::int32_t>(0x20)>3000){words.set(0x1c,std::uint8_t{!words.template get<std::uint8_t>(0x1c)});words.set(0x20,0u);words.set(0x1d,std::uint8_t{0});}
        words.set(0x24,0u);words.set(0x18,0x4cbebc20u);
    }
    if(!words.template get<std::uint8_t>(0x1c)){
        float best=100000000.f;const auto first=static_cast<std::uint16_t>(index);
        for(std::uint32_t i=first;i<static_cast<std::uint32_t>(first)+3u&&i+1<route.size();++i){
            const auto offset=delta(predicted,route[i].position),segment=delta(route[i+1].position,route[i].position);
            float projection=store(square(segment));const float dot=store((static_cast<long double>(segment[1])*offset[1]+static_cast<long double>(segment[2])*offset[2])+static_cast<long double>(segment[0])*offset[0]);
            float distance;
            if(negative(dot))distance=store((static_cast<long double>(offset[1])*offset[1]+static_cast<long double>(offset[2])*offset[2])+static_cast<long double>(offset[0])*offset[0]);
            else{
                projection=store(static_cast<long double>(dot)/projection);
                if(projection<=1.f){HumanVector residual{};for(unsigned n=0;n<3;++n)residual[n]=store(static_cast<long double>(offset[n])-static_cast<long double>(segment[n])*projection);distance=store((static_cast<long double>(residual[0])*residual[0]+static_cast<long double>(residual[2])*residual[2])+static_cast<long double>(residual[1])*residual[1]);}
                else if(i+2==route.size()){const auto d=delta(route[i+1].position,predicted);distance=store(square(d));}
                else continue;
            }
            if(distance<best){index=i;best=distance;}
        }
        index=static_cast<std::uint16_t>(index);if(index+1>=route.size())throw std::runtime_error("Vehicle interpolation requires actual adjacent route records");
        const auto& a=route[index];const auto& b=route[index+1];const auto segment=delta(b.position,a.position),offset=delta(predicted,a.position);
        const float dot=store((static_cast<long double>(segment[1])*offset[1]+static_cast<long double>(segment[2])*offset[2])+static_cast<long double>(segment[0])*offset[0]);
        float parameter=negative(dot)?.10000000149011612f:store(static_cast<long double>(dot)/square(segment));parameter=store(static_cast<long double>(parameter)*1.2999999523162842f);if(parameter>1)parameter=1;
        if(index==words.template get<std::uint16_t>(0x10)&&parameter<words.template get<float>(0x14))parameter=words.template get<float>(0x14);
        const float squared=store(static_cast<long double>(parameter)*parameter),cubed=store(static_cast<long double>(squared)*parameter);HumanVector target=a.position,tangent{};
        for(unsigned n=0;n<3;++n){
            const auto difference=static_cast<long double>(a.position[n])-b.position[n];
            const float cubic=store((difference+difference+b.tangent[n])+a.tangent[n]);
            const float quadratic=store((static_cast<long double>(a.tangent[n])* -2.f-b.tangent[n])-difference*3.f);
            const float first=store((static_cast<long double>(cubic)*cubed+static_cast<long double>(squared)*quadratic)+static_cast<long double>(parameter)*a.tangent[n]);target[n]=store(static_cast<long double>(first)+a.position[n]);
            const float twice=store(static_cast<long double>(parameter)+parameter),triple_square=store(static_cast<long double>(squared)*3.f);
            tangent[n]=store((static_cast<long double>(cubic)*triple_square+static_cast<long double>(quadratic)*twice)+a.tangent[n]);
        }
        tangent[1]=0;tangent=unit(tangent,1);
        if(index!=words.template get<std::uint16_t>(0x10)){words.set(0x18,0x4cbebc20u);words.set(0x24,0u);}words.set(0x10,static_cast<std::uint16_t>(index));words.set(0x14,parameter);
        const bool reverse=words.template get<float>(0x8c)<0;if(reverse)for(unsigned n=0;n<3;++n)heading[n]=-heading[n];
        steer(state,car,predicted,heading,target,tangent,b.terminal!=0,b.position,reverse,world);
        if(b.terminal){const auto d=delta(predicted,b.position);if(square(d)<4.f)return finish(state,car,world);const float enddot=store((static_cast<long double>(d[1])*segment[1]+static_cast<long double>(d[2])*segment[2])+static_cast<long double>(d[0])*segment[0]);if(!negative(enddot))return finish(state,car,world);}
    }else{
        index=static_cast<std::uint16_t>(index);if(index+1>=route.size())throw std::runtime_error("Vehicle reversal requires actual adjacent route records");
        const auto& a=route[index];const auto& b=route[index+1];const bool reverse=!(words.template get<float>(0x8c)<0);if(reverse)for(unsigned n=0;n<3;++n)heading[n]=-heading[n];
        steer(state,car,predicted,heading,b.position,b.tangent,true,b.position,reverse,world);
        const auto d=delta(predicted,b.position),segment=delta(b.position,a.position);const float dot=store((static_cast<long double>(d[1])*segment[1]+static_cast<long double>(d[2])*segment[2])+static_cast<long double>(d[0])*segment[0]);if(!negative(dot))words.set(0x1c,std::uint8_t{0});
        if(index!=words.template get<std::uint16_t>(0x10)){words.set(0x18,0x4cbebc20u);words.set(0x24,0u);}words.set(0x10,static_cast<std::uint16_t>(index));
    }
    return result;
}
}
#endif
