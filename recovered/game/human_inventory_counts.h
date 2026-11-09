#ifndef HD2_GAME_HUMAN_INVENTORY_COUNTS_H
#define HD2_GAME_HUMAN_INVENTORY_COUNTS_H
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <stdexcept>
namespace hd2::game {
//005A6AA0 counts the FIRST matching stack, not all matching entries globally.
//Single holders0/2/3/4/5 short-circuit to1 before holsters1 then optional pack6.
template<class Human,class Items>std::int32_t count_human_definition(Human& human,Items& items,std::int32_t definition){
    auto* storage=human.base().storage();
    if(!storage||storage->single())throw std::runtime_error("Human item count requires actual Human storage");
    const auto matches=[&](auto entry){return static_cast<std::int32_t>(items.runtime_item(entry).definition().identifier)==definition;};
    for(unsigned index:{0u,2u,3u,4u,5u}){
        auto* holder=storage->holder(index);if(!holder)throw std::runtime_error("Human item count is missing a required holder");
        if(!holder->empty()&&matches(holder->entry()))return 1;
    }
    for(unsigned index:{1u,6u})if(auto* slots=storage->slots(index))
        for(std::size_t n=0;n<slots->occupied_stacks();++n){const auto& stack=slots->slots().at(n);
            for(auto at=stack.entries.rbegin();at!=stack.entries.rend();++at)if(matches(*at))return static_cast<std::int32_t>(stack.entries.size());
        }
    return 0;
}
//005A6C50: reverse optional pack, overwriting result on EACH match, then first
//reverse matching holster stack adds its count. Does not sum duplicate stacks.
template<class Human,class Items>std::int32_t count_human_ammunition(Human& human,Items& items,std::int32_t definition,bool include_pack){
    auto* storage=human.base().storage();
    if(!storage||storage->single())throw std::runtime_error("Human ammunition count requires actual Human storage");
    const auto matches=[&](auto entry){return static_cast<std::int32_t>(items.runtime_item(entry).definition().identifier)==definition;};
    std::uint32_t count=0;
    if(include_pack)if(auto* pack=storage->slots(6))for(auto n=pack->occupied_stacks();n;){const auto& stack=pack->slots().at(--n);
        if(stack.entries.empty())throw std::runtime_error("Occupied Human pack stack is empty");
        if(matches(stack.entries.front()))count=static_cast<std::uint32_t>(stack.entries.size());
    }
    auto* holsters=storage->slots(1);if(!holsters)throw std::runtime_error("Human ammunition count is missing holsters");
    for(auto n=holsters->occupied_stacks();n;){const auto& stack=holsters->slots().at(--n);
        if(stack.entries.empty())throw std::runtime_error("Occupied Human holster stack is empty");
        if(matches(stack.entries.front())){count+=static_cast<std::uint32_t>(stack.entries.size());break;}
    }
    std::int32_t result;std::memcpy(&result,&count,4);return result;
}
} //namespace hd2::game
#endif
