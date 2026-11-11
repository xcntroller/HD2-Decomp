#ifndef HD2_GAME_HUMAN_EQUIPMENT_TRANSITION_SOUND_H
#define HD2_GAME_HUMAN_EQUIPMENT_TRANSITION_SOUND_H

#include "game/human_holster_lookup.h"
#include "frontend/spatial_sound.h"
#include <utility>

namespace hd2::game {

//0077B870's two SOUND emissions. Definition48 is the primary serialized payload,
//whose +18 receiver is007E0290 in each of the four weapon vtables. The SAME
//runtime item/catalogue and actual view gate supply these reads.005A2B10 makes
//three distinct005A2AC0 hand lookups when the selected item is a weapon.
template<class Human,class Items,class Services>
void emit_human_equipment_transition_sound(Human& human,Items& items,Services& services,bool holster_kind9){
    using Entry=std::decay_t<decltype(human.base().storage()->holder(3)->entry())>;
    using Item=std::remove_reference_t<decltype(items.runtime_item(std::declval<Entry>()))>;
    const auto hand=[&]()->Entry{
        auto* storage=human.base().storage();auto* holder=storage?storage->holder(3):nullptr;
        if(!storage||storage->single()||!holder)throw std::runtime_error("Human transition sound requires its actual hand holder");
        return holder->entry();
    };
    const auto weapon=[&]()->Entry{ //005A2B10, preserve lookup count and native family gate.
        if(!hand())return Entry{};
        const auto entry=hand();
        if(!entry)throw std::runtime_error("Human transition sound lost its actual hand item");
        if(items.runtime_item(entry).family()!=Item::Family::weapon)return Entry{};
        return hand();
    };
    const auto definition=[&](Entry entry){
        if(!entry)throw std::runtime_error("Human transition sound lost its actual selected item");
        return items.runtime_item(entry).definition().identifier;
    };
    frontend::SpatialSoundRequest request;
    request.flags=1;request.source=&human;request.group=2;
    if(holster_kind9){
        const auto entry=find_player_mode_holster(human,items,9,services.game_word28());
        if(services.human_view_offset_enabled()||!entry)return;
        const auto id=definition(entry);
        if(id>=500||!items.catalogue().item(static_cast<std::int32_t>(id)))return;
        request.entry=items.runtime_item(entry).primary_action_sound();
    }else{
        if(services.human_view_offset_enabled()||!weapon())return;
        const auto id=definition(weapon());
        if(id>=500||!items.catalogue().item(static_cast<std::int32_t>(id)))return;
        const auto entry=weapon();const auto current=definition(entry);
        if(current>=500||!items.catalogue().item(static_cast<std::int32_t>(current)))
            throw std::runtime_error("Human transition sound lost its actual catalogue definition");
        request.entry=items.runtime_item(entry).primary_action_sound();
    }
    (void)services.emit_human_equipment_sound(request);
}

} //namespace hd2::game
#endif
