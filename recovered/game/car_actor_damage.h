#ifndef HD2_GAME_CAR_ACTOR_DAMAGE_H
#define HD2_GAME_CAR_ACTOR_DAMAGE_H
#include "game/car_damage_packets.h"
namespace hd2::game {
//004DDE20, RET4. Friendship uses SOURCE as the observer. Native callbacks and
//late mode/health/side observations must not be merged into cached decisions.
template<class Runtime,class Car,class Actor>bool car_damage_source_allowed(Runtime& r,Car& car,Actor source){
    const auto count=car.occupants.size();
    if((r.car_damage_mode()&7u)==7u){
        for(auto n=count;n;){const auto occupant=r.car_damage_occupant(&car,--n);
            if(occupant&&r.car_damage_hostile(source,occupant)&&r.car_damage_health(occupant)>0)return true;}
    }else for(auto n=count;n;){const auto occupant=r.car_damage_occupant(&car,--n);
        if(occupant&&r.car_damage_health(occupant)>0&&r.car_damage_player(occupant)&&!r.car_damage_suppressed(occupant)){
            const auto side=r.car_damage_side(occupant);if(side!=r.car_damage_side(source))return true;
        }
    }
    return false;
}
//004D3B70. For a parent overlap, wheel-local X is discarded; radial Y/Z is
//tested. For direct frame hits, the actual current wheel40 distance is used.
template<class Frames,class Wheel>unsigned car_wheel_damage_region(Frames& frames,bool field5e0,
        const Wheel& wheel,const CarContactVector& point,bool parent_overlap){
    if(field5e0)return 2;
    long double square;
    if(parent_overlap){
        const auto world=frames.actor_world_matrix(wheel.frame);
        const auto local=frames.car_transform(point,frames.car_inverse_matrix(world));
        square=static_cast<long double>(local[2])*local[2]+static_cast<long double>(local[1])*local[1];
    }else{
        const auto& centre=wheel.vectors.at(0x40);
        const auto x=static_cast<long double>(centre[0])-point[0],y=static_cast<long double>(centre[1])-point[1],z=static_cast<long double>(centre[2])-point[2];
        square=(z*z+y*y)+x*x;
    }
    const float stored=static_cast<float>(square);std::int32_t bits;std::memcpy(&bits,&stored,4);
    const auto distance=bits<0x322bcc77?0.L:std::abs(static_cast<long double>(stored)-1.L)<9.99999993922529e-9f?1.L:std::sqrt(static_cast<long double>(stored));
    return !(distance>static_cast<long double>(wheel.scalars.get(0x1d8))*.699999988079071f)?2u:1u;
}
//004DBA10, D4/RET24. The final native hit word is unused. Caller must borrow
//the SAME initialized body/wheels/dirty and actual24E0 ancillary owner.
template<class Frames,class Runtime,class Car,class StateFactory,class Actor>void apply_car_actor_damage(
        Frames& frames,Runtime& r,Car& car,StateFactory&& get_state,std::uint32_t kind,float amount,
        const CarContactVector& point,const CarContactVector& direction,Actor source){
    if(source&&r.car_damage_role(source)==2&&!car_damage_source_allowed(r,car,source))return;
    if(car.flag254a&&kind==2)return;
    car.source24b0=source?source->base().id10():-1;
    auto scaled=direction;for(unsigned a=0;a!=3;++a)scaled[a]*=10.f;
    const auto hit=frames.car_damage_pick(point,scaled,car.base().frame(),0);
    auto& state=get_state();
    if(hit){
        auto& parts=car.geometry->deformables().parts();const auto& applied=car.geometry->physics().geometry().parts;
        for(auto n=parts.size();n;){const auto index=--n;auto& part=parts[index];
            if((part.flags&1u)||part.frame!=hit)continue;
            const auto absorption=applied.at(index).scalars.get(0x28); //SAME applied native part28 resistance.
            part.health=static_cast<float>(static_cast<long double>(part.health)-
                (static_cast<long double>(amount)-static_cast<long double>(absorption)*.009999999776482582f*amount)*.20000000298023224f);
            state.dirty|=2u;part.flags|=2u;if(part.health<0)part.health=0;
            CarDamageEvent event;event.index=static_cast<std::uint32_t>(index);event.health=car_damage_integer(part.health);
            if(part.type==3){event.kind=0;event.first=direction;}
            else{
                event.kind=1;auto force=direction;
                long double x,y;
                if(part.type==2||part.type==6){
                    const auto world=frames.actor_world_matrix(part.frame);
                    force[0]=world[12]-state.body.pose().position[0];force[1]=world[13]-state.body.pose().position[1];force[2]=world[14]-state.body.pose().position[2];
                    force[2]*=.20000000298023224f;x=static_cast<long double>(force[0])*.20000000298023224f;y=1.5f;
                }else{x=force[0];y=static_cast<long double>(force[1])+.10000000149011612f;}
                const auto scale=kind==1?static_cast<long double>(amount)*.009999999776482582f:static_cast<long double>(amount);
                if(kind==1){const float stored=static_cast<float>(scale);
                    force[0]=static_cast<float>(scale*x);force[1]=static_cast<float>(static_cast<long double>(stored)*y);force[2]*=stored;
                }else{force[0]=static_cast<float>(x*scale);force[1]=static_cast<float>(y*scale);force[2]=static_cast<float>(static_cast<long double>(force[2])*scale);}
                const auto world=frames.actor_world_matrix(part.frame);
                std::array<long double,3> offset{};for(unsigned a=0;a!=3;++a)offset[a]=static_cast<long double>(world[12+a])-state.body.pose().centre[a];
                const auto& angular=state.body.motion().angular_velocity;event.first=state.body.motion().velocity;
                for(unsigned a=0;a!=3;++a){const auto next=(a+1)%3,last=(a+2)%3;
                    const auto cross=static_cast<long double>(angular[next])*offset[last]-offset[next]*angular[last];
                    if(a<2)event.first[a]-=static_cast<float>(cross);else event.first[a]=static_cast<float>(static_cast<long double>(event.first[a])-cross);
                }
                event.second=force;
            }
            r.car_damage_event(&car,event);break; //Wire only; no local presentation apply.
        }
        const auto wheel_count=state.wheels.size();
        for(auto n=wheel_count;n;){const auto index=--n;auto& wheel=state.wheels[index];if(wheel.flags&0x40000000u)continue;
            if(wheel.frame!=hit){const auto parent=frames.object_parent(hit);if(wheel.parent!=parent&&wheel.frame!=parent)continue;}
            if((car_wheel_damage_region(frames,car.geometry->physics().scalars().get(0x550)!=0,wheel,point,wheel.frame!=hit)&1u)&&!(wheel.flags&0x80000000u)){
                CarDamageEvent event;event.kind=3;event.index=static_cast<std::uint32_t>(index);r.car_damage_event(&car,event);
            }
        }
    }
    if(!car.ancillary)throw std::logic_error("Car damage requires its actual ancillary owner");
    car.ancillary->damage(frames,car.base().frame(),r,static_cast<Actor>(&car),kind,amount,point,direction,source);
    state.solver.mass_changed=true; //Actor2350/core22C0, before genuine empty00611080(0).
}
//004DB360 actor1C. Decode the native event completely BEFORE the physical
//mutations. Authority still consumes presentation payload, including wheel2.
template<class Frames,class Runtime,class Car,class StateFactory>void receive_car_damage_packet(
        Frames& frames,Runtime& r,Car& car,StateFactory&& get_state,SessionPacketStream& packet){
    const auto event=read_car_damage_event(packet,packet.read(4));
    if(event.kind>5)return; //Genuine native default, before any state access.
    if(event.kind==5){car.ancillary->present_explosion(frames,car.base().frame(),r,event.index);return;}
    if(event.kind==4){car.ancillary->present_destruction(frames,car.base().frame(),r);r.car_present_destroyed(car);return;}
    auto& state=get_state();
    if(event.kind==0||event.kind==1){
        state.dirty|=2u;auto& part=car.geometry->deformables().parts().at(event.index);part.flags|=2u;part.health=static_cast<float>(event.health);
        if(event.kind==0){ //00500DA0: positive material exchange still requires its actual receiver.
            const auto threshold=car.geometry->physics().geometry().parts.at(event.index).scalars.get(0x20);
            if(part.health<threshold&&!(part.flags&8u))throw std::runtime_error("Car glass requires actual00500DA0 material/texture exchange");
            if(part.health<=0){part.flags|=1u;car.callbacks.value().hide69c(event.index);}
        }else if(part.health<=0)(void)car.callbacks.value().part694(event.index,event.first,&event.second);
    }else if(event.kind==2){
        if(!(r.car_damage_mode()&1u)&&state.wheels.at(event.index).scalars.get(0x230)>0){
            state.dirty|=2u;auto& wheel=state.wheels.at(event.index);wheel.flags|=0x10000000u;wheel.scalars.set(0x230,static_cast<float>(event.health));
            if(wheel.scalars.get(0x230)<=0&&car.geometry->physics().scalars().get(0x550)==0)
                (void)car.callbacks.value().wheel698(event.index,event.first,nullptr);
        }
    }else if(event.kind==3){state.wheels.at(event.index).flags|=0x80000000u;r.car_puncture_sound(car,event.index);}
    //00611080(0) is genuine empty; no2350 change is made here.
}

}
#endif
