#ifndef HD2_GAME_CAR_COLD_WHEEL_CONTACTS_H
#define HD2_GAME_CAR_COLD_WHEEL_CONTACTS_H

#include "game/car_body_integration.h"
#include "game/car_suspension_impact.h"

namespace hd2::game {

struct CarWheelQueryState {
    float projected_radius=0; //8DC: also written by the low-speed plane pass.
    bool moving_primitive=false; //2286, set by the actual previous primitive's10000 flag.
};

//00512390, the loading/ordinary first-frame F6C&800 path. Uses real query
//identities and the same wheel/tyre/body owners. Alternate timed query caches
//(!800) are explicitly outside this path. Notification methods are mandatory
//receivers, called only by their native feature/audibility gates.
template<class Frames,class Queries,class Receivers>
typename Frames::Vector car_cold_wheel_contacts(Frames& frames,typename Frames::Frame model,
        CarBodyIntegration<Frames>& body,std::vector<CarWheelRuntime<Frames>>& wheels,
        const CarTyres& tyres,Queries& queries,Receivers& receivers,CarWheelQueryState& query_state,
        float elapsed,const typename Frames::Vector& down,bool impacts,std::uint32_t features,
        std::uint32_t body_flags,float speed,bool audible,std::uint32_t layers,std::uint32_t flags){
    using Vector=typename Frames::Vector;using Wheel=CarWheelRuntime<Frames>;
    if(!(features&0x800u))throw std::runtime_error("Car timed wheel-query cache requires its actual controller");
    if(!(elapsed>0)||!std::isfinite(elapsed))throw std::runtime_error("Car wheel contact interval must be positive and finite");
    const auto& pose=body.pose();const auto& up=pose.up;
    const auto dot=[](const Vector& a,const Vector& b){return (static_cast<long double>(a[0])*b[0]+static_cast<long double>(a[1])*b[1])+static_cast<long double>(a[2])*b[2];};
    const auto cross=[](const Vector& a,const Vector& b){Vector result{};for(unsigned axis=0;axis!=3;++axis){const auto n=(axis+1)%3,l=(axis+2)%3;result[axis]=static_cast<float>(static_cast<long double>(a[n])*b[l]-static_cast<long double>(a[l])*b[n]);}return result;};
    const auto normalize=[&](Vector value){
        const float square=static_cast<float>(dot(value,value));constexpr float epsilon=9.99999993922529e-09f;
        if(!std::isfinite(square))throw std::runtime_error("Non-finite Car wheel direction");
        if(std::abs(static_cast<long double>(square)-1)>=epsilon){
            if(square>=epsilon){const auto amount=1/std::sqrt(static_cast<long double>(square));for(unsigned axis=0;axis!=3;++axis)value[axis]=static_cast<float>(value[axis]*amount);}
            else{const auto axis=value[0]!=0?0u:value[2]!=0?2u:1u;value[axis]=value[axis]<0?-1.f:1.f;}
        }return value;
    };
    const auto length=[&](const Vector& value){
        const float square=static_cast<float>(dot(value,value));std::int32_t bits;std::memcpy(&bits,&square,4);
        if(bits<0x322bcc77)return 0.f;
        if(std::abs(square-1.f)<9.99999993922529e-09f)return 1.f;
        return static_cast<float>(std::sqrt(static_cast<long double>(square)));
    };
    const auto radius=[](const Wheel& wheel){return wheel.scalars.get((wheel.flags&0x80000000u)?0x1e8:0x1e4);};
    const auto origin=[&](Wheel& wheel){
        Vector result;
        if(frames.object_parent(wheel.parent)==model)result=frames.car_transform(frames.car_position(wheel.frame),frames.car_local_matrix(wheel.parent));
        else{const auto world=frames.actor_world_matrix(wheel.frame);result=frames.car_transform(Vector{{world[12],world[13],world[14],world[15]}},pose.inverse);}
        result[1]=wheel.vectors.at(0x30)[1]+.20000000298023224f;
        return frames.car_transform(result,pose.world);
    };
    const auto displacement=[&](const Wheel& wheel){
        const auto& s=wheel.scalars;const float distance=radius(wheel)+s.get(0x264)+s.get(0x1e0)+s.get(0x164)+1.f;
        Vector result{};for(unsigned axis=0;axis!=3;++axis)result[axis]=distance*down[axis];return result;
    };
    const auto projection=[&](Wheel& wheel,const Vector& normal){
        const auto world=frames.actor_world_matrix(wheel.frame);
        const float angle=frames.car_vector_angle(Vector{{world[0],world[1],world[2],world[3]}},normal);
        const auto adjustment=static_cast<long double>(wheel.scalars.get(0x1ec))*(angle>1.5707963705062866f?-1:1);
        return static_cast<float>(std::cos(static_cast<long double>(angle)+adjustment-1.5707963705062866f)*radius(wheel));
    };
    const auto move_centre=[&](Wheel& wheel,const Vector& point,float projected,bool reuse){
        const auto& s=wheel.scalars;const float amount=projected+s.get(0x264)+s.get(0x1e0);
        auto centre=point;for(unsigned axis=0;axis!=3;++axis){
            const auto scaled=static_cast<long double>(amount)*up[axis];
            centre[axis]=static_cast<float>((axis?static_cast<long double>(static_cast<float>(scaled)):scaled)+point[axis]);
            const float difference=centre[axis]-wheel.vectors.at(0x40)[axis];
            wheel.vectors.at(0xc0)[axis]+=difference;
            if(reuse)wheel.vectors.at(0xd0)[axis]+=difference;
        }wheel.vectors[0x40]=centre;if(!reuse)wheel.vectors[0xd0]=Vector{};
    };
    for(auto n=wheels.size();n;){auto& wheel=wheels[--n];frames.update_scene_object(wheel.parent);frames.update_scene_object(wheel.frame);wheel.vectors[0x60]=wheel.vectors.at(0x50);wheel.query_identity=nullptr;}
    bool reuse=!wheels.empty()&&!(body_flags&0x2000000u)&&speed<1.562600016593933f;
    for(auto n=wheels.size();reuse&&n;){const auto& wheel=wheels[--n];if(!(wheel.flags&0x40000400u))reuse=dot(wheel.vectors.at(0xa0),wheel.vectors.at(0xa0))<=1.5625f&&dot(wheel.vectors.at(0xd0),wheel.vectors.at(0xd0))<=.0024999999441206455f;}
    if(reuse)for(auto n=wheels.size();n;){
        auto& wheel=wheels[--n];if((wheel.flags&0x40000408u)!=8)continue;
        const auto& normal=wheel.vectors.at(0x80);const float constant=static_cast<float>(-dot(normal,wheel.vectors.at(0x50)));
        const auto start=origin(wheel),segment=displacement(wheel);const float denominator=static_cast<float>(dot(segment,normal));
        if(std::abs(denominator)<9.99999993922529e-09f)continue;
        const float parameter=static_cast<float>(-(dot(normal,start)+constant)/denominator);
        Vector point{},travel{};for(unsigned axis=0;axis!=3;++axis){const float scaled=parameter*segment[axis];point[axis]=start[axis]+scaled;travel[axis]=point[axis]-start[axis];}
        if(dot(travel,travel)>dot(segment,segment))continue; //Native has no parameter>=0 gate.
        query_state.projected_radius=projection(wheel,normal);const auto& s=wheel.scalars;
        if(length(travel)>query_state.projected_radius+s.get(0x264)+s.get(0x1e0)+s.get(0x164)+.20000000298023224f)continue;
        wheel.flags|=0x20000000u;wheel.vectors[0x50]=point;move_centre(wheel,point,query_state.projected_radius,true);
        wheel.vectors[0x130]=normal;wheel.scalars.set(0x13c,constant);wheel.query_identity=wheel.previous_query_identity;
    }
    for(auto n=wheels.size();n;){
        const auto index=--n;auto& wheel=wheels[index];
        if((wheel.flags&0x40000400u)||((wheel.flags&0x20000000u)&&!(body_flags&0x2000000u)))continue;
        const auto start=origin(wheel),segment=displacement(wheel);auto hit=queries.ray(start,segment,layers,flags);
        float projected=0;bool accepted=false;
        if(hit){
            wheel.vectors[0x130]=hit->normal;wheel.scalars.set(0x13c,static_cast<float>(-dot(hit->position,hit->normal)));wheel.query_identity=hit->identity;
            projected=projection(wheel,hit->normal);Vector travel{};for(unsigned axis=0;axis!=3;++axis)travel[axis]=hit->position[axis]-start[axis];
            const auto& s=wheel.scalars;if(projected+s.get(0x264)+s.get(0x1e0)+s.get(0x164)+.20000000298023224f<length(travel))hit.reset();
        }else wheel.query_identity=nullptr;
        wheel.previous_query_identity=hit?hit->identity:nullptr;wheel.previous_query_flags=hit?hit->flags:0;
        //Native stores previous200 before invoking actual surface6AC, which may reject.
        auto* filtered=hit?receivers.surface(*hit,static_cast<unsigned>(index)):nullptr;
        if(filtered&&wheel.material_id>=0){
            wheel.vectors[0x80]=filtered->normal;if(!(wheel.flags&8u))wheel.vectors[0x90]=filtered->normal;wheel.flags|=8u;
            const auto surface=filtered->flags>>24;
            if(wheel.surface_id!=surface){
                if(wheel.material_type>0){const auto& surfaces=tyres.types().at(static_cast<std::size_t>(wheel.material_type)).surfaces;wheel.material_surface=-1;for(auto i=surfaces.size();i;)if(surfaces[--i].id==surface){wheel.material_surface=static_cast<std::int32_t>(i);break;}}
                wheel.scalars.set(0x26c,wheel.scalars.get(0x260)+wheel.scalars.get(0x25c)+1.f);
            }wheel.surface_id=surface;wheel.vectors[0x50]=filtered->position;
            accepted=!(wheel.material_type>0&&wheel.material_surface<0);
            if(!accepted)for(unsigned axis=0;axis!=3;++axis)wheel.vectors[0x50][axis]=(start[axis]+segment[axis])+up[axis];
        }else{
            projected=radius(wheel);for(unsigned axis=0;axis!=3;++axis)wheel.vectors[0x50][axis]=static_cast<float>((static_cast<long double>(start[axis])+segment[axis]+up[axis])-static_cast<long double>(up[axis])*.20000000298023224f);
        }
        if(!accepted)wheel.flags&=~8u;
        move_centre(wheel,wheel.vectors.at(0x50),projected,false);
    }
    Vector direction_sum{},linear_change{},angular_change{};unsigned impact_count=0;float sound_volume=0;
    for(auto n=wheels.size();n;){
        const auto index=--n;auto& wheel=wheels[index];auto& s=wheel.scalars;
        if(!(wheel.flags&0x40000400u)){
            auto local=frames.car_transform(wheel.vectors.at(0x40),pose.inverse);const auto& authored=wheel.vectors.at(0x30);local[1]=std::min(local[1],authored[1]);
            Vector position=authored;position[1]=local[1];
            if(frames.object_parent(wheel.parent)!=model){const auto matrix=frames.car_local_matrix(wheel.parent);const float offset=local[1]-authored[1];for(unsigned axis=0;axis!=3;++axis)position[axis]=static_cast<float>(static_cast<long double>(matrix[4+axis])*offset+wheel.vectors.at(0x20)[axis]);}
            frames.car_suspension_position(wheel.parent,position);
            float extension=std::min(authored[1]-local[1],s.get(0x164));float rate=(s.get(0x16c)-extension)/elapsed;
            const bool slope=(wheel.flags&8u)&&dot(wheel.vectors.at(0x80),up)<.5f;
            if((extension<=.004999999888241291f||slope)&&impacts){
                if(!slope)extension=0;
                if(!(wheel.flags&0x40u)){
                    wheel.flags|=0x40u;
                    const auto impact=car_suspension_impact(frames,body.contact_state(),up,wheel.vectors.at(0x50),wheel.vectors.at(0x80),wheel.vectors.at(0x90));
                    if(impact){
                        ++impact_count;for(unsigned axis=0;axis!=3;++axis){linear_change[axis]+=impact->linear_change[axis];angular_change[axis]+=impact->angular_change[axis];}
                        if(features&0x8000u)receivers.deform(static_cast<unsigned>(index),wheel.vectors.at(0x40),up,.20000000298023224f,impact->magnitude);
                        if(features&0x4000u){receivers.damage(impact->magnitude);receivers.passengers(*impact,wheel.vectors.at(0x40));}
                        rate=0;
                        if((features&0x20u)&&audible&&impact->normal_speed>1){const float volume=impact->normal_speed>7?1.f:(impact->normal_speed-1.f)*.1666666716337204f;sound_volume=std::max(sound_volume,volume);}
                    }
                }
            }
            //Native copies even an uninitialized noncontact normal. Keep it absent
            //until actually produced; new contact initializes both80 and90 above.
            if(const auto normal=wheel.vectors.find(0x80);normal!=wheel.vectors.end())wheel.vectors[0x90]=normal->second;
            if(sound_volume>.20000000298023224f)receivers.sound(sound_volume);
            const float spring=(s.get(0x164)-extension)*s.get(0x160);
            float load=spring;if(rate<0)load=static_cast<float>(static_cast<long double>(rate)*s.get(0x174)+spring);else if(!(wheel.flags&0x40u))load=static_cast<float>(static_cast<long double>(rate)*s.get(0x170)+spring);
            s.set(0x168,load);s.set(0x16c,extension);if(load<0)wheel.flags&=~8u;wheel.flags&=~0x40u;
            Vector direction{};
            const auto parent_world=frames.actor_world_matrix(wheel.parent);const auto forward=normalize(Vector{{parent_world[8],parent_world[9],parent_world[10],parent_world[11]}});
            if(!(wheel.flags&8u)){
                Vector offset{};for(unsigned axis=0;axis!=3;++axis)offset[axis]=wheel.vectors.at(0x40)[axis]-pose.centre[axis];
                const auto rotation=cross(body.motion().angular_velocity,offset);Vector velocity{};for(unsigned axis=0;axis!=3;++axis)velocity[axis]=body.motion().velocity[axis]-rotation[axis];wheel.vectors[0xa0]=velocity;direction=forward;
            }else{
                if(wheel.previous_query_identity&&(wheel.previous_query_flags&0x10000u))query_state.moving_primitive=true;
                Vector velocity{};const float inverse=1.f/elapsed;for(unsigned axis=0;axis!=3;++axis)velocity[axis]=(wheel.vectors.at(0x50)[axis]-wheel.vectors.at(0x60)[axis])*inverse;wheel.vectors[0xa0]=velocity;
                direction=cross(cross(up,forward),wheel.vectors.at(0x80));
            }
            if(s.get(0x22c)>0){const auto rotation=frames.car_axis_quaternion(up,static_cast<float>(std::cos(static_cast<long double>(s.get(0x1f0)))*s.get(0x22c)));frames.car_transform_in_place(direction,frames.car_quaternion_matrix(rotation));}
            wheel.vectors[0xb0]=normalize(direction);
            if(dot(wheel.vectors.at(0xa0),wheel.vectors.at(0xb0))>=0)wheel.flags|=0x10000u;else wheel.flags&=~0x10000u;
            if(wheel.flags&8u)for(unsigned axis=0;axis!=3;++axis)direction_sum[axis]+=wheel.vectors.at(0xb0)[axis];
        }wheel.flags&=~0x20000000u;
    }
    if(impact_count){const float inverse=1.f/static_cast<float>(impact_count);for(unsigned axis=0;axis!=3;++axis){body.motion().velocity[axis]=static_cast<float>(static_cast<long double>(inverse)*linear_change[axis]+body.motion().velocity[axis]);body.motion().angular_velocity[axis]=static_cast<float>(static_cast<long double>(inverse)*angular_change[axis]+body.motion().angular_velocity[axis]);}}
    return direction_sum;
}
} //namespace hd2::game
#endif
