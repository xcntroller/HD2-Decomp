#ifndef HD2_GAME_DOG_SOUND_NOTIFICATIONS_H
#define HD2_GAME_DOG_SOUND_NOTIFICATIONS_H
#include "game/dog_controller_process.h"
#include "game/dog_damage.h"
#include "game/human_packet_commands.h"
#include "game/human_sound_notifications.h"
#include <type_traits>
namespace hd2::game {
//005D6000 AFTER actual listenerEC→Dog resolution. SAME564 retained records
//and actual process heard pool; new and repeated events use DIFFERENT clocks.
template<class Dog,class Event,class Frames,class World>void notify_dog_sound(Dog& dog,const Event& event,Frames& frames,World& world){
    const auto flags=dog.require_controller().dog_control_state().flags20;
    if(flags&0x40000000u||dog.require_controller().dog_control_kind()==1)return;
    if(!event.source||event.source->base().type()!=3)return;
    const auto source_kind=world.dog_sound_human_kind(*event.source);if(source_kind==2||source_kind==3)return;
    const auto matrix=frames.actor_world_matrix(dog.base().frame());
    const auto x=static_cast<long double>(event.position[0])-matrix[12],y=static_cast<long double>(event.position[1])-matrix[13],z=static_cast<long double>(event.position[2])-matrix[14];
    const float square=static_cast<float>((y*y+z*z)+x*x);std::int32_t bits;std::memcpy(&bits,&square,4);
    const auto distance=bits<0x322bcc77?0.L:std::abs(static_cast<long double>(square)-1)<9.99999993922529e-09f?1.L:std::sqrt(static_cast<long double>(square));
    if(distance>dog_signed_word(dog.properties.fields4a0[3]))return; //Native unordered continues.
    const float volume=static_cast<float>(static_cast<long double>(event.field14)*3.f);
    float score=0;
    switch(event.kind){
    case 0:{const auto movement=world.dog_sound_human_movement(*event.source);score=movement==0?10.f:movement==2?14.f:movement==5?16.f:12.f;break;}
    case 1:score=3;break;case 2:score=1;break;case 3:score=2;break;
    case 4:if(volume>.949999988079071f)dog.require_controller().dog_control_loud_sound();return;
    case 5:case 7:return;
    case 9:case 10:score=15;break;
    default:break; //6/8/out-of-range produce actual zero-score records.
    }
    for(auto n=dog.heard564.size();n;){
        auto* record=dog.heard564[--n];if(!record)throw std::runtime_error("Dog SOUND reaches native NULL heard record");
        if(!record->source14)throw std::runtime_error("Dog SOUND requires actual heard source producer");
        if(*record->source14!=static_cast<void*>(event.source))continue;
        if(!record->kind10)throw std::runtime_error("Dog SOUND requires actual heard kind producer");
        if(*record->kind10!=event.kind)continue;
        record->position0.assign(event.position);
        record->clock18=world.dog_sound_microseconds();
        if(!record->strength1c)throw std::runtime_error("Dog SOUND requires actual heard strength producer");
        record->strength1c=dog_signed_word(human_packet_ftol(static_cast<long double>(*record->strength1c)+score));
        dog.require_controller().dog_control_heard(*record);return;
    }
    auto& record=world.dog_controller_process().acquire_heard(event.kind,event.position,static_cast<void*>(event.source),world.dog_sound_ticks());
    record.strength1c=dog_signed_word(human_packet_ftol(score));
    dog.heard564.push_back(&record);dog.require_controller().dog_control_heard(record);
}
//005D7760's synthetic nearby-Human kind0 notifications, original signed
//elapsed gate and low32 hearing-radius square. Physical Actor30 is intentional.
template<class Dog,class Frames,class World>void poll_dog_nearby_sounds(Dog& dog,Frames& frames,World& world){
    if(!dog.elapsed55c)throw std::runtime_error("Dog nearby hearing requires actual55C timer producer");
    if(*dog.elapsed55c<=999)return;
    const auto radius=dog.properties.fields4a0[3];const float range=static_cast<float>(dog_signed_word(radius*radius));dog.elapsed55c=0;
    if(dog.friendly54c.empty())for(std::size_t n=0;n!=32;++n){
        if(!world.dog_scene_actor(n))continue;
        if(world.dog_scene_human_kind(n)==1)dog.friendly54c.push_back(world.dog_scene_actor(n));
    }
    using Actor=typename std::decay_t<decltype(dog.friendly54c)>::value_type;
    using Event=HumanSoundEvent<typename Frames::Frame,Actor>;
    for(std::size_t n=0;n<dog.friendly54c.size();++n){
        auto* actor=dog.friendly54c[n];if(!actor)throw std::runtime_error("Dog nearby hearing reaches native NULL friendly");
        if(world.dog_sighted_human_health(*actor)<1)continue;
        const auto own=dog.base().world_position(),other=actor->base().world_position();
        const auto x=static_cast<long double>(other[0])-own[0],y=static_cast<long double>(other[1])-own[1],z=static_cast<long double>(other[2])-own[2];
        const auto square=(z*z+y*y)+x*x;if(!(square<range))continue;
        //Reload the live list entry for the actual event source and position.
        actor=dog.friendly54c[n];if(!actor)throw std::runtime_error("Dog nearby hearing lost actual friendly");
        Event event{actor->base().world_position(),dog.base().frame(),static_cast<float>(1.L-square/range),0,actor};
        notify_dog_sound(dog,event,frames,world);
    }
}
}
#endif
