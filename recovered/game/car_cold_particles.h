#ifndef HD2_GAME_CAR_COLD_PARTICLES_H
#define HD2_GAME_CAR_COLD_PARTICLES_H

#include "game/car_health_reset.h"
#include "game/car_body_contacts.h"
#include "game/car_particles.h"

namespace hd2::game {

struct CarColdParticleSlots {
    std::int32_t damage=-1;std::array<std::int32_t,3> impacts{{-1,-1,-1}};
    std::vector<std::int32_t> exhausts;
    explicit CarColdParticleSlots(std::size_t exhaust_count):exhausts(exhaust_count,-1){} //004EA780.
};

//Cold branches of00506F00/00508350/00508EE0/00509540. These retire actual
//shared pool slots by004ECFC0's pending byte. Positive emitter production is
//explicitly excluded here; no active emitter is silently substituted/omitted.
template<class Frames>bool update_car_cold_damage_particle(const CarPhysics& physics,
        const CarHealthState& health,CarColdParticleSlots& slots,std::vector<CarParticleState<typename Frames::Frame>>& pool,
        std::uint32_t features,bool live,float speed){
    if((features&0x200u)&&live&&health.body<physics.scalars().get(0x624)&&speed<physics.scalars().get(0x62c))
        throw std::runtime_error("Car damaged-body emitter requires its actual active particle update");
    if(slots.damage>=0){retire_car_particle(pool,slots.damage);slots.damage=-1;}
    return true; //Actual00506F00 cold return, used by finalizer sleeping gates.
}
template<class Frames>void update_car_cold_wheel_particles(const CarMaterialRegistry& materials,
        std::vector<CarWheelRuntime<Frames>>& wheels,std::vector<CarParticleState<typename Frames::Frame>>& pool,
        std::uint32_t features,bool live,float speed,unsigned game_kind){
    if(game_kind==8)return;
    for(auto n=wheels.size();n;){auto& wheel=wheels[--n];
        if((features&0x200u)&&live){
            if(wheel.flags&0x400u)continue;
            if(!(wheel.flags&0x40000000u)&&(wheel.flags&8u)&&(wheel.flags&0x800u)){
                const auto& velocity=wheel.vectors.at(0xa0);
                const auto square=(static_cast<long double>(velocity[2])*velocity[2]+static_cast<long double>(velocity[1])*velocity[1])+static_cast<long double>(velocity[0])*velocity[0];
                const auto& material=materials.responses().at(materials.indices().at(wheel.surface_id));
                const bool slipping=(wheel.flags&0x80u)||((wheel.flags&0x1000020u)&&4.f<square);
                const auto effect=slipping?material.words20[1]:(3.f<speed?material.words20[0]:0u);
                if(car_contact_signed_word(effect)>0)throw std::runtime_error("Car moving-wheel emitter requires its actual active particle update");
            }
        }
        if(wheel.effect>=0){retire_car_particle(pool,wheel.effect);wheel.effect=-1;}
    }
}
template<class Frames>void update_car_cold_exhaust_particles(const CarPhysics& physics,const CarDrivetrainState& drive,
        CarColdParticleSlots& slots,std::vector<CarParticleState<typename Frames::Frame>>& pool,
        std::uint32_t features,bool live,float speed){
    if((features&0x200u)&&live&&drive.running&&speed<physics.scalars().get(0x758)&&physics.field_754()>0)
        throw std::runtime_error("Car running exhaust requires its actual active particle update");
    for(auto n=slots.exhausts.size();n;){auto& slot=slots.exhausts[--n];if(slot>=0){retire_car_particle(pool,slot);slot=-1;}}
}
template<class Frames>void update_car_cold_impact_particles(const CarMaterialRegistry& materials,
        const std::vector<CarBodyContactRecord>& records,CarColdParticleSlots& slots,
        std::vector<CarParticleState<typename Frames::Frame>>& pool,std::uint32_t features,bool live){
    for(auto n=slots.impacts.size();n;){const auto index=--n;auto& slot=slots.impacts[index];
        if((features&0x200u)&&live&&index<records.size()){
            const auto effect=car_contact_signed_word(materials.responses().at(materials.indices().at(records[index].surface)).words20[2]);
            if(effect)throw std::runtime_error("Car impact emitter requires its actual active particle update"); //Native abs(signed ID).
        }
        if(slot>=0){retire_car_particle(pool,slot);slot=-1;}
    }
}
} //namespace hd2::game
#endif
