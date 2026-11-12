#ifndef HD2_GAME_CAR_COLLISION_IMPULSE_H
#define HD2_GAME_CAR_COLLISION_IMPULSE_H
#include "game/car_damage_packets.h"

namespace hd2::game {
//004D3D10, RET8. Source identity uses00475DD0's physical Scene2 lookup,
//distinct from Human request9's00475D50 command identity. Reuse actual2584.
template<class Runtime,class Car>void receive_car_collision_impulse(Runtime& runtime,Car& car,
        std::uint32_t source_identity,const CarContactVector& point){
    auto* driver=runtime.car_damage_occupant(&car,0);bool local=false;
    if(driver){
        driver=runtime.car_damage_occupant(&car,0);
        if(!driver)throw std::logic_error("Car collision driver disappeared before native identity read");
        const auto identity=runtime.car_collision_identity(driver);const auto local_identity=runtime.car_collision_local_identity();
        local=identity==local_identity;
    }
    if(!local){
        if(!(runtime.car_damage_mode()&1u))return;
        if(runtime.car_damage_occupant(&car,0)){
            if(!(runtime.car_damage_mode()&1u))return;
            if(!runtime.car_damage_occupant(&car,0))return;
            const auto now=runtime.car_collision_now();auto& timestamp=runtime.car_collision_clock(car);
            if(car_contact_signed_word(now-timestamp)<=1000)return;
            timestamp=runtime.car_collision_now(); //Before writer, including a failed writer's retained prefix.
            CarDamageEvent event;event.kind=11;event.index=source_identity;event.first=point;
            runtime.car_damage_event(&car,event);return;
        }
    }
    auto* other=runtime.car_collision_source(source_identity);if(!other)return;
    const float own_mass=runtime.car_collision_mass(&car),other_mass=runtime.car_collision_mass(other);
    float ratio=1;
    if(!(other_mass>own_mass)){
        const auto numerator=runtime.car_collision_mass(other);const auto denominator=runtime.car_collision_mass(&car);
        ratio=static_cast<float>(static_cast<long double>(numerator)/denominator);
    }
    //00611080(0) and0069BFE0 are genuine empty receivers. Native reloads
    //mass after them; X/Z spill before the final.3 multiplication, Y does not.
    const float mass=runtime.car_collision_mass(&car);const auto velocity=runtime.car_collision_velocity(other);
    const float x=static_cast<float>((static_cast<long double>(ratio)*velocity[0])*mass);
    const float z=static_cast<float>(static_cast<long double>(ratio)*velocity[2]);
    const float z_mass=static_cast<float>(static_cast<long double>(z)*mass);
    CarContactVector impulse{{static_cast<float>(static_cast<long double>(x)*.30000001192092896f),
        static_cast<float>(((static_cast<long double>(ratio)*velocity[1])*mass)*.30000001192092896f),
        static_cast<float>(static_cast<long double>(z_mass)*.30000001192092896f),0}};
    runtime.car_presentation_impulse(car,impulse,point); //Actual004F2790(mask0,scale1), not a velocity overwrite.
}
//004D3EF0's unowned path; false continues its existing owned Human/Car
//contact bodies. The same native2584 clock governs client request and server relay.
template<class Runtime,class Car,class Source>bool route_car_collision_hit(Runtime& runtime,Car& car,
        Source source,const CarContactVector& point){
    if(runtime.car_presentation_authority(car))return false;
    if(!(runtime.car_damage_mode()&1u)){
        const auto now=runtime.car_collision_now();auto& timestamp=runtime.car_collision_clock(car);
        if(car_contact_signed_word(now-timestamp)>1000){
            timestamp=runtime.car_collision_now();CarDamageEvent event;event.kind=11;event.first=point;
            event.index=static_cast<std::uint32_t>(runtime.car_collision_identity(source));
            runtime.car_contact_event(&car,event,true);return true;
        }
    }
    const auto identity=runtime.car_collision_identity(source);
    runtime.car_receive_collision_impulse(car,static_cast<std::uint32_t>(identity),point);return true;
}
} //namespace hd2::game
#endif
