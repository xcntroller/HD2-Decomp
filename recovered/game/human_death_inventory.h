#ifndef HD2_GAME_HUMAN_DEATH_INVENTORY_H
#define HD2_GAME_HUMAN_DEATH_INVENTORY_H
#include "game/human_ai_shot_direction.h"
#include "game/human_state_frame_timers.h"

namespace hd2::game {
//005A2FE0. All entries and containers stay owned by SAME Items/Human storage;
//005A3490 transfers them to actual type11 actors and queues their native IDs.
template<class Human,class Items,class World,class Frames>
void drop_human_death_inventory(Human& human,Items& items,World& world,Frames& frames){
    auto* storage=human.base().storage();if(!storage||storage->single())throw std::logic_error("Death inventory requires actual Human storage");
    auto* hand=storage->holder(3);if(!hand)throw std::logic_error("Death inventory lost actual hand holder");
    const auto held=hand->entry();auto base=world.human_damage_target_direction(human);base[1]+=1.f;
    const auto square=(static_cast<long double>(base[0])*base[0]+static_cast<long double>(base[1])*base[1])+static_cast<long double>(base[2])*base[2];
    normalize_human_ai_shot(base,square);for(unsigned axis=0;axis!=3;++axis)base[axis]+=base[axis];
    const auto velocity=[&]{
        const auto first=frames.volume_random_unit();const float amplitude=first+first;
        const float x=static_cast<float>(static_cast<long double>(frames.volume_random_unit())*.20000000298023224f);
        const float y=static_cast<float>(static_cast<long double>(frames.volume_random_unit())*.20000000298023224f);
        const auto z=static_cast<long double>(frames.volume_random_unit())*.20000000298023224f;
        HumanVector result{};result[0]=static_cast<float>(static_cast<long double>(x)*amplitude+base[0]);
        result[1]=static_cast<float>(static_cast<long double>(y)*amplitude+base[1]);result[2]=static_cast<float>(z*amplitude+base[2]);return result;
    };
    if(held&&items.runtime_item(held).amount()>0){
        auto& item=items.runtime_item(held);item.loaded_ammunition(item.amount());world.drop_human_damage_item(human,held,velocity());
    }
    auto* shoulder=storage->holder(2);if(!shoulder)throw std::logic_error("Death inventory lost actual shoulder holder");
    if(const auto entry=shoulder->entry();entry&&items.runtime_item(entry).definition().kind()==21)world.drop_human_damage_item(human,entry,velocity());
    for(unsigned slot:{6u,1u})if(auto* stacks=storage->slots(slot))for(auto n=stacks->occupied_stacks();n;){
        const auto index=--n;const auto& entries=stacks->slots().at(index).entries;
        if(entries.empty())throw std::logic_error("Death inventory occupied stack is empty");
        const auto kind=items.runtime_item(entries.front()).definition().kind();if(kind!=20&&kind!=21)continue;
        for(auto count=entries.size();count;){const auto entry=stacks->slots().at(index).entries.at(--count);world.drop_human_damage_item(human,entry,velocity());}
    }
}
} //namespace hd2::game
#endif
