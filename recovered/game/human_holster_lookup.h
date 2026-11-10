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
} //namespace hd2::game
#endif
