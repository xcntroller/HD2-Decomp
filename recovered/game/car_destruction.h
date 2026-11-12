#ifndef HD2_GAME_CAR_DESTRUCTION_H
#define HD2_GAME_CAR_DESTRUCTION_H
#include "game/car_damage_packets.h"
namespace hd2::game {
//004DBF00/004DC780 inline core light-control setters. Every native callback
//observes the current flags; a callback may affect the following sign gates.
template<class Notify>void clear_destroyed_car_lights(std::uint32_t& flags,Notify&& notify){
    flags&=~8u;flags&=~0x800u;flags&=~0x8000u;flags&=~0x80u;notify();
    if(!(flags&0x80u)){flags&=~0x10u;notify();}
    if(!(flags&0x80u)){flags&=~0x20u;notify();}
    if(!(flags&8u))flags&=~1u;
    if(!(flags&8u))flags&=~2u;
    flags&=~0x10000u;notify();
    if(!(flags&0x800u)){flags&=~0x100u;notify();}
    if(!(flags&0x8000u)){flags&=~0x1000u;notify();}
}
inline CarContactVector normalize_car_destruction_vector(CarContactVector value,long double square){
    constexpr float epsilon=9.99999993922529e-9f;
    if(!std::isfinite(square))throw std::runtime_error("Non-finite Car destruction direction");
    const float stored=static_cast<float>(square);
    if(std::abs(square-1.L)>=epsilon){
        if(stored>=epsilon){const auto scale=1.L/std::sqrt(static_cast<long double>(stored));
            for(unsigned a=0;a!=3;++a)value[a]=static_cast<float>(static_cast<long double>(value[a])*scale);
        }else{const auto axis=value[0]!=0?0u:value[2]!=0?2u:1u;value[axis]=value[axis]<0?-1.f:1.f;}
    }
    return value;
}
//004DC580, direct nearby Human damage (not another BlastWave record). Frame260
//is the actual retained Human head marker. Scene2 slots are reread after ray.
template<class Frames,class Runtime,class Car,class State>void damage_car_nearby_humans(
        Frames& frames,Runtime& r,Car& car,State& state){
    auto centre=state.body.pose().position;centre[1]+=.5f;
    const auto source=r.car_destruction_source(car.source24b0);
    for(auto n=r.car_area_count();n;){const auto index=--n;auto* actor=r.car_area_actor(index);
        if(!actor||r.car_area_type(actor)!=3)continue;
        const auto world=frames.actor_world_matrix(r.car_area_head(actor));
        const auto x=static_cast<float>(static_cast<long double>(world[12])-centre[0]);
        const auto y=static_cast<long double>(world[13])-centre[1],z=static_cast<long double>(world[14])-centre[2];
        if((static_cast<long double>(x)*x+y*y)+z*z>25.f)continue;
        const CarContactVector offset{{x,static_cast<float>(y),static_cast<float>(z),0}};
        if(r.car_area_obstructed(centre,offset))continue;
        const float square=static_cast<float>((static_cast<long double>(offset[2])*offset[2]+
            static_cast<long double>(offset[1])*offset[1])+static_cast<long double>(offset[0])*offset[0]);
        std::int32_t bits;std::memcpy(&bits,&square,4);
        const float distance=bits<0x322bcc77?0.f:std::abs(static_cast<long double>(square)-1.L)<9.99999993922529e-9f?1.f:
            static_cast<float>(std::sqrt(static_cast<long double>(square)));
        actor=r.car_area_actor(index);if(!actor)throw std::runtime_error("Car explosion Human slot retired during ray");
        const auto point=r.car_area_random_point(actor); //Actual005A3CC0 consumes its CRT draw before D4.
        const float amount=static_cast<float>((1.L-static_cast<long double>(distance)*.20000000298023224f)*1000.f);
        r.car_damage_human(actor,2,amount,point,offset,source);
    }
}
//004DBF00 RET4, water argument retained independently from the source identity.
//The actual runtime owns control/Script/Scene2/CRT/engine-random/item receivers.
template<class Frames,class Runtime,class Car,class StateFactory>void destroy_car_actor(
        Frames& frames,Runtime& r,Car& car,StateFactory&& get_state,bool water){
    if(car.flag254a)return;
    car.flag254a=true;
    CarDamageEvent event;event.kind=4;r.car_damage_event(&car,event);
    if((r.car_damage_mode()&7u)!=7u&&!r.car_destruction_remote_ai()&&r.car_destruction_style()!=2)car.restart_clock();
    //00611080(0) is genuinely empty. Control callbacks retain native ordering.
    r.car_destruction_controls(car);
    r.car_destruction_script(car);
    const auto source=r.car_destruction_source(car.source24b0);
    bool experience=false;
    for(auto n=car.occupants.size();n;){auto* victim=r.car_damage_occupant(&car,--n);if(!victim)continue;
        const auto point=car.base().world_position();const CarContactVector up{{0,1,0,0}};
        r.car_damage_human(victim,5,-1.f,point,up,water?car.driver24ac:source);
        if(!water&&source&&r.car_damage_hostile(victim,source))experience=true;
    }
    car.model254c.reset(); //Release BEFORE wheel iteration, including on otherwise empty lists.
    auto& state=get_state();
    for(std::size_t index=0;index<state.wheels.size()&&index<6;++index){
        if(state.wheels[index].flags&0x40000000u)continue;
        if(!(r.car_destruction_random_word()&1u))continue; //CRT draw precedes5E0.
        if(car.geometry->physics().scalars().get(0x550)!=0)continue;
        auto force=state.wheels[index].vectors.at(0x40);
        const auto& centre=state.body.pose().centre;
        force[0]-=centre[0];force[1]-=centre[1];
        const auto z=static_cast<long double>(force[2])-centre[2];force[2]=static_cast<float>(z);
        const auto square=(z*force[2]+static_cast<long double>(force[0])*force[0])+static_cast<long double>(force[1])*force[1];
        force=normalize_car_destruction_vector(force,square);
        const auto gain=((static_cast<long double>(r.car_destruction_random_float())-.5f)+1.f)*10.f;
        force[0]=static_cast<float>(static_cast<long double>(force[0])*gain);
        force[1]=static_cast<float>(std::abs(static_cast<long double>(force[1])*gain));
        force[2]=static_cast<float>(static_cast<long double>(force[2])*gain);
        const auto scaled_square=(static_cast<long double>(force[2])*force[2]+static_cast<long double>(force[0])*force[0])+static_cast<long double>(force[1])*force[1];
        auto velocity=normalize_car_destruction_vector(force,scaled_square);
        const auto speed=static_cast<long double>(r.car_destruction_random_float())+2.f;
        velocity[0]=static_cast<float>(static_cast<long double>(velocity[0])*speed);
        velocity[1]=static_cast<float>(speed); //Native replaces Y, rather than scaling normalized Y.
        velocity[2]=static_cast<float>(static_cast<long double>(velocity[2])*speed);
        (void)car.callbacks.value().wheel698(index,velocity,&force);
        state.solver.mass_changed=true;
    }
    if(experience&&source)r.car_destruction_experience(source,600);
    if(!(r.car_damage_mode()&2u))car.flag2548=true;
    damage_car_nearby_humans(frames,r,car,state);
}

//004DC780, received event4 presentation, after004E2000. Reuse the SAME
//physical lifetime and callbacks; missing positive glass/Item12 still fail at
//their actual call site, after the native prefix has already been published.
template<class Frames,class Runtime,class Car,class StateFactory>void present_destroyed_car(
        Frames& frames,Runtime& r,Car& car,StateFactory&& get_state){
    auto& state=get_state();std::array<bool,6> detached{};
    for(std::size_t n=0;n<state.wheels.size()&&n<detached.size();++n)detached[n]=(state.wheels[n].flags&0x40000000u)!=0;
    r.car_physical_reset(car,state.solver.speed,true);
    for(std::size_t n=0;n<state.wheels.size()&&n<detached.size();++n)if(detached[n]){
        state.wheels[n].flags|=0x40000000u;frames.object_visible(state.wheels[n].frame,false);frames.update_scene_object(state.wheels[n].frame);
    }
    r.car_destruction_controls(car);
    if(r.car_presentation_authority(car)){
        CarContactVector impulse{{0,car.geometry->physics().scalars().get(0x918)*3.f,0,0}};
        r.car_presentation_impulse(car,impulse,state.body.pose().position); //Actor350/core2C0.
    }
    r.car_destroyed_meshes(car);
    auto& parts=car.geometry->deformables().parts();
    for(auto n=parts.size();n;){const auto index=--n;auto& part=parts[index];if(part.flags&1u)continue;
        const auto world=frames.actor_world_matrix(part.frame);auto force=state.body.pose().position;
        force[0]=world[12]-force[0];force[1]=world[13]-force[1];const auto z=static_cast<long double>(world[14])-force[2];force[2]=static_cast<float>(z);
        force=normalize_car_destruction_vector(force,(z*force[2]+static_cast<long double>(force[0])*force[0])+static_cast<long double>(force[1])*force[1]);
        const auto gain=(static_cast<long double>(r.car_destruction_random_float())-.5f)*10.f+20.f;
        force[0]=static_cast<float>(force[0]*gain);force[2]=static_cast<float>(force[2]*gain);force[1]=static_cast<float>(std::abs(force[1]*gain));
        if(force[1]<5.f)force[1]=5.f;
        if(part.type==3){
            part.health=0;auto glass=force;for(unsigned a=0;a!=3;++a)glass[a]*=.10000000149011612f;
            r.car_present_glass(car,index,glass);
        }else if(r.car_destruction_random_word()&1u){
            part.health=0;const CarContactVector velocity{{0,0,0,0}};(void)car.callbacks.value().part694(index,velocity,&force);
        }
    }
    car.model254c.reset();
    for(std::size_t n=0;n<car.occupants.size();++n)if(auto* occupant=r.car_damage_occupant(&car,n))r.car_present_occupant(occupant);
}
}
#endif
