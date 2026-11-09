#ifndef HD2_GAME_HUMAN_INVENTORY_LOOKUP_H
#define HD2_GAME_HUMAN_INVENTORY_LOOKUP_H

#include <type_traits>
#include <utility>
#include <cstdint>
#include <stdexcept>
#include <initializer_list>

namespace hd2::game {

//005A6FF0 searches THIS Human's storage, not the global Items registry. Holder
//order3/0/2/4/5, then stack arrays1/6 ascending with entries reversed. Runtime
//instances are resolved through their SAME owning item module before ID reads.
template<class Human,class Items>
auto find_human_inventory_item(Human& human,Items& items,std::int32_t identity) {
    using Entry=std::decay_t<decltype(human.base().storage()->holder(0)->entry())>;
    using Item=std::remove_reference_t<decltype(items.runtime_item(std::declval<Entry>()))>;
    auto* storage=human.base().storage();
    if(!storage||storage->single())throw std::runtime_error("Human inventory lookup has no actual Human storage");
    for(unsigned index:{3u,0u,2u,4u,5u}) {
        auto* holder=storage->holder(index);
        if(!holder)throw std::runtime_error("Human inventory lookup has no required holder");
        if(const auto entry=holder->entry()) {auto& item=items.runtime_item(entry);if(item.identity==identity)return &item;}
    }
    for(unsigned index:{1u,6u})if(auto* stacks=storage->slots(index))
        for(const auto& slot:stacks->slots())for(auto at=slot.entries.rbegin();at!=slot.entries.rend();++at) {
            auto& item=items.runtime_item(*at);if(item.identity==identity)return &item;
        }
    return static_cast<Item*>(nullptr);
}

//005A6DB0: command lookup is RESTRICTED to the requested storage slot. This
//must not use005A6FF0's whole-inventory search when executing B0+74/+78 commands.
template<class Human,class Items>
auto find_human_inventory_entry(Human& human,Items& items,std::int32_t identity,std::int32_t slot) {
    using Entry=std::decay_t<decltype(human.base().storage()->holder(0)->entry())>;
    auto* storage=human.base().storage();
    if(!storage||storage->single())throw std::runtime_error("Human command lookup requires actual Human storage");
    if(slot==0||slot==2||slot==3||slot==4||slot==5) {
        auto* holder=storage->holder(static_cast<std::size_t>(slot));
        if(!holder)throw std::runtime_error("Human command lookup is missing a required holder");
        if(const auto entry=holder->entry())if(items.runtime_item(entry).identity==identity)return entry;
    }else if(slot==1||slot==6) {
        auto* stacks=storage->slots(static_cast<std::size_t>(slot));
        if(!stacks){if(slot==6)return Entry{};throw std::runtime_error("Human command lookup is missing holsters");}
        for(std::size_t n=0;n<stacks->occupied_stacks();++n) {
            const auto& entries=stacks->slots().at(n).entries;
            for(auto at=entries.rbegin();at!=entries.rend();++at)if(items.runtime_item(*at).identity==identity)return *at;
        }
    }
    return Entry{};
}
} //namespace hd2::game
#endif
