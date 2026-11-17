#ifndef HD2_GAME_HUMAN_ACTIVE_AI_DAMAGE_H
#define HD2_GAME_HUMAN_ACTIVE_AI_DAMAGE_H
#include "game/human_ai_shot_direction.h"
namespace hd2::game {
//0052EAF0 supplies a fixed100 to0052E7C0, irrespective of remaining health.
//This is the actual Active BC receiver; it must not inherit a RET24 dispatch.
template<class AI,class Human,class World,class Frames>void notify_active_human_damage(AI& ai,
        std::uint32_t kind,std::uint32_t hit,std::int32_t health,Human* source,
        const HumanVector& point,const HumanVector& direction,World& world,Frames& frames){
    auto* human=ai.human();if(!human)throw std::logic_error("Active damage lost Human44");
    const auto validate=world.human_common_receiver_guard(*human);auto& state=ai.state();
    if(source){
        const auto other=world.human_common_receiver_guard(*source);
        world.active_shared_damage(ai,kind,hit,health,source,point,direction);validate();other();
        if(source->properties.field19c==1&&kind==1&&world.human_is_player(*source)){
            const auto amount=static_cast<long double>(state.template get<float>(0x1c4))+100.f;
            state.set(0x1c4,static_cast<float>(amount)); //FST, not FSTP: compare EXTENDED sum.
            if(amount>400.f){state.set(0x1c4,0u);world.active_player_damage_reaction(*source);validate();other();}
        }
        if(source->properties.field19c==2){
            const auto& target=ai.resources().target();const auto value=target.state.template get<HumanVector>(0x10);
            const auto square=(static_cast<long double>(value[0])*value[0]+static_cast<long double>(value[1])*value[1])+static_cast<long double>(value[2])*value[2];
            if(!target.actor&&square<0x1.cd2b2ap-54f)state.set(0x198,0u); //0080F948, ordered <.
        }
    }
    if(hit==11){
        auto* storage=human->base().storage();auto* holder=storage?storage->holder(0):nullptr;
        if(!holder)throw std::runtime_error("Active damage reached native NULL head holder dereference");
        if(const auto entry=holder->entry()){
            auto velocity=direction;velocity[1]=static_cast<float>(static_cast<long double>(velocity[1])+1.f);
            const auto square=(static_cast<long double>(velocity[2])*velocity[2]+static_cast<long double>(velocity[1])*velocity[1])+static_cast<long double>(velocity[0])*velocity[0];
            normalize_human_ai_shot(velocity,square);
            const auto speed=static_cast<long double>(frames.volume_random_unit())+2.f;validate();
            for(unsigned axis=0;axis!=3;++axis)velocity[axis]=static_cast<float>(static_cast<long double>(velocity[axis])*speed);
            world.drop_human_damage_item(*human,entry,velocity);validate();
        }
    }else if(hit==3&&world.human_frame_random_word()%20u==0){
        ai.refresh_item();validate();if(const auto entry=ai.item()){
            const auto item_class=world.human_frame_output_item(entry).definition().item_class;
            if(item_class==1||item_class==3||item_class==4){
                //Native computes unused scratch from hit direction, but passes
                //fresh Human005A80E0/virtual4C, with no random engine draw.
                const auto velocity=world.human_damage_target_direction(*human);
                world.drop_human_damage_item(*human,entry,velocity);validate();
                world.queue_active_ai_speech(ai,0x72u,false,500u,false);validate();
            }
        }
    }
}
} //namespace hd2::game
#endif
