#ifndef HD2_GAME_HUMAN_HOLSTER_LOOKUP_H
#define HD2_GAME_HUMAN_HOLSTER_LOOKUP_H
#include <cstdint>
#include <type_traits>
#include <stdexcept>

namespace hd2::game {
//005A6730, shared by player mode and frame feedback: reverse occupied holster
//stacks, FRONT entry only, unsigned cooldown, requested kind. No ammo/variant/
//family/hand/shoulder filtering or successful missing-storage fallback.
template<class Human,class Items>auto find_player_mode_holster(Human& human,Items& items,std::int32_t kind,std::uint32_t now) {
    using Entry=std::decay_t<decltype(human.base().storage()->holder(3)->entry())>;
    auto* storage=human.base().storage();
    if(!storage||storage->single()||!storage->slots(1))throw std::runtime_error("Human lookup requires its actual holsters");
    const auto& stacks=storage->slots(1)->slots();
    for(auto n=storage->slots(1)->occupied_stacks();n;){
        const auto& entries=stacks.at(--n).entries;if(entries.empty())continue;
        const auto entry=entries.front();auto& item=items.runtime_item(entry);
        if(now>=item.field14&&item.definition().kind()==static_cast<std::uint32_t>(kind))return entry;
    }
    return Entry{};
}
//005A7E00. The held kind13 bypasses cooldown; fallback checks the front of
//each occupied holster in reverse, through the existing005A6730 owner.
template<class Human,class Items>auto find_human_medical_item(Human& human,Items& items,std::uint32_t now){
    auto* storage=human.base().storage();
    if(!storage||storage->single()||!storage->holder(5))throw std::runtime_error("Human medical query requires actual holder5");
    const auto held=storage->holder(5)->entry();
    if(held&&items.runtime_item(held).definition().kind()==13)return held;
    return find_player_mode_holster(human,items,13,now);
}
} //namespace hd2::game
#endif
