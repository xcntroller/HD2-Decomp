#ifndef HD2_GAME_CAR_FRAME_EVENTS_H
#define HD2_GAME_CAR_FRAME_EVENTS_H

#include "game/car_damage_packets.h"
#include "game/car_targeted_placement.h"

namespace hd2::game {

//004DE1F0, RET8. Decode/mutate one entry at a time. The apply byte is captured
//after the first nonterminal identity; no storage access in the consume-only path.
template<class Runtime,class Car>void receive_car_inventory(Runtime& runtime,Car& car,
        SessionPacketStream& packet,bool apply){
    auto identity=packet.read(16);
    if(identity==0xffffu)return;
    do {
        const auto definition=packet.read(8);
        if(apply){
            const auto entry=runtime.car_inventory_item(definition,identity);
            //Native rereads74/first storage AFTER factory construction. Failed
            //insertion neither deletes nor unregisters the module-owned entry.
            auto* storage=car.base().storage();
            if(!storage||!storage->slots(0))throw std::logic_error("Car inventory packet requires actual first storage");
            (void)storage->slots(0)->insert(entry);
        }
        identity=packet.read(16);
    }while(identity!=0xffffu);
}

//004D2790's prefix. Expiry itself has already refreshed2470 with its second
//clock observation;2478 is cleared only AFTER publication and any local reset.
template<class Car,class Mode,class Publish,class Reset>void update_car_periodic_event(Car& car,
        Mode&& mode,Publish&& publish,Reset&& reset){
    if(!car.clock_expired())return;
    CarDamageEvent event;event.kind=6;publish(event);
    if(!(mode()&2u))reset(); //Native rereads mode AFTER publication.
    car.fields2474[1]=0;
}

//004DEC10: signed wrapping subtraction; publication of -1/source/2544 precedes
//the tail-call destruction. A failure cannot re-arm the countdown on retry.
template<class Car,class Destroy>void update_car_targeted_damage(Car& car,
        std::uint32_t elapsed,Destroy&& destroy){
    if(!car.damage2550)throw std::logic_error("Car pending damage requires actual activation");
    if(*car.damage2550==-1)return;
    *car.damage2550=car_contact_signed_word(static_cast<std::uint32_t>(*car.damage2550)-elapsed);
    if(*car.damage2550>0)return;
    car.damage2550=-1;
    if(!car.source2554)throw std::logic_error("Car pending damage has no actual source producer");
    car.source24b0=*car.source2554;car.flag2544=true;destroy();
}

//004DEAD0, RET16. Item and Human are borrowed actual owners. The presentation
//prefix (mode2) completes before15000/source publication; item retirement follows.
template<class Car,class Entry,class Human,class Placement,class Runtime>bool begin_car_targeted_damage_lazy(
        Car& car,Entry item,Human* human,Placement&& placement,Runtime& runtime){
    if(!item||!human)return false;
    if(!car.damage2550)throw std::logic_error("Car pending damage requires actual activation");
    if(*car.damage2550!=-1)return false;
    if(runtime.car_damage_mode()&2u){const auto value=placement();runtime.car_targeted_presentation(car,item,*human,value.position,value.direction);}
    car.damage2550=15000;car.source2554=human->base().id10();
    if(!(runtime.car_damage_mode()&2u))runtime.car_targeted_consume(*human,item);
    return true;
}
template<class Car,class Entry,class Human,class Runtime>bool begin_car_targeted_damage(
        Car& car,Entry item,Human* human,const CarContactVector& first,
        const CarContactVector& second,Runtime& runtime){
    return begin_car_targeted_damage_lazy(car,item,human,[&]{return CarTargetedPlacement{first,second};},runtime);
}

//004DEB50: save BOTH transforms before template lookup/factory. Publish254C
//before copy, retire the inventory entry before link/pose; copy status is unused.
template<class Frames,class Car,class Entry,class Human,class Runtime>void present_car_targeted_damage(
        Frames& frames,Car& car,Entry item,Human& human,const CarContactVector& first,
        const CarContactVector& second,Runtime& runtime){
    car.target2560=first;car.target2570=second;
    const auto source=runtime.car_targeted_model(item);
    auto frame=frames.create_car_targeted_frame(source);car.model254c.emplace(std::move(frame));
    if(!car.model254c->get())throw std::runtime_error("Car targeted visual factory returned null after254C publication");
    frames.copy_car_targeted_frame(car.model254c->get(),source);
    runtime.car_targeted_consume(human,item);
    frames.car_reset_link(car.model254c->get(),car.base().frame());
    frames.car_targeted_pose(car.model254c->get(),first,second);
}

} //namespace hd2::game
#endif
