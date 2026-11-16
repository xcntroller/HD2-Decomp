#ifndef HD2_GAME_HUMAN_WEAPON_RELOAD_H
#define HD2_GAME_HUMAN_WEAPON_RELOAD_H

#include "game/human_state_frame_timers.h"
#include <type_traits>

namespace hd2::game {

//007D4850, explicit source slot (default1, with kind6 slot1 pack redirect).
//The caller supplies the actual Items module, reload sound and holder removal.
template<class Item,class Human,class World>
bool reload_human_weapon(Item& weapon,Human& human,std::int32_t& ammunition,
        bool retain_last,bool unlimited_ammunition,World& world,std::uint32_t source_slot=1u){
    ammunition=-1;
    if(weapon.family()!=Item::Family::weapon)return false;
    const auto finish=[&]{
        const auto end=world.game_word28()+weapon.reload_duration();
        if(human_frame_signed_word(weapon.field14)<human_frame_signed_word(end))weapon.field14=end;
        if(human_frame_signed_word(weapon.field5c)<human_frame_signed_word(end))weapon.field5c=end;
        weapon.field60&=~2u;
        return true;
    };
    const bool forced=(weapon.field60&2u)!=0;
    if(!weapon.allows_partial_reload()&&weapon.loaded_ammunition()!=0)
        return forced?finish():false;
    if(world.game_word28()<weapon.field5c)return false;
    auto* storage=human.base().storage();
    if(!storage||(storage->single()&&source_slot!=0)||source_slot>6)return false;
    const auto slot=weapon.definition().kind()==6&&source_slot==1&&!storage->single()&&storage->slots(6)?6u:source_slot;
    auto* slots=storage->slots(slot);
    if(!slots){
        if(auto* holder=storage->holder(slot)){
            if(holder->first_scalar()<=0)return false;
            //Actual8170D0: holder virtual3C=007D6860 returnsNULL while
            //virtual44=007D6880 returns1. Native reload then dereferencesNULL.
            throw std::runtime_error("Reload source holder reaches native NULL3C stack dereference");
        }
        return false;
    }
    if(slots->first_scalar()<=0)return false;
    using Entry=std::decay_t<decltype(human.base().storage()->holder(3)->entry())>;
    auto& items=world.human_reload_items();
    Entry fullest{},emptiest{};
    std::size_t count=0;
    const auto definition=weapon.definition().ammunition();
    for(std::size_t n=0;n<slots->occupied_stacks()&&definition>0;++n){
        const auto& stack=slots->slots().at(n).entries;
        if(stack.empty())continue;
        const auto& first=items.runtime_item(stack.front());
        if(first.family()!=Item::Family::basic||first.definition().identifier!=static_cast<std::uint32_t>(definition))continue;
        fullest=emptiest=stack.front();
        count=stack.size();
        for(auto entry:stack){
            const auto amount=items.runtime_item(entry).amount();
            if(amount>items.runtime_item(fullest).amount())fullest=entry;
            else if(amount<items.runtime_item(emptiest).amount())emptiest=entry;
        }
        break;
    }
    if(!unlimited_ammunition){
        if(!fullest){weapon.field60&=~2u;return forced;}
        if(count==1&&retain_last)weapon.loaded_ammunition(weapon.capacity());
        else {
            auto& magazine=items.runtime_item(fullest);
            const float total=static_cast<float>(static_cast<long double>(weapon.loaded_ammunition())+magazine.amount());
            const auto capacity=weapon.capacity();
            weapon.loaded_ammunition(total);
            bool consume=true;
            if(total>capacity){
                const auto remaining=total-capacity;
                weapon.loaded_ammunition(capacity);
                auto& other=items.runtime_item(emptiest);
                const auto merged=other.amount()+remaining;
                if(remaining!=0){
                    if(emptiest==fullest||merged>other.capacity()){
                        magazine.magazine_amount(remaining);
                        consume=false;
                    }else other.magazine_amount(merged);
                }
            }
            if(consume){
                ammunition=magazine.identity;
                world.consume_human_ammunition(human,fullest,slot);
            }
        }
    }else weapon.loaded_ammunition(weapon.capacity());
    world.emit_human_weapon_reload(weapon,human);
    return finish();
}

} //namespace hd2::game
#endif
