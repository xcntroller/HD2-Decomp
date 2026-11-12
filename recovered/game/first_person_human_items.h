#ifndef HD2_GAME_FIRST_PERSON_HUMAN_ITEMS_H
#define HD2_GAME_FIRST_PERSON_HUMAN_ITEMS_H
#include <cstddef>
#include "game/human_holster_lookup.h"
#include <stdexcept>
#include <utility>
namespace hd2::game {
//005A6DA0 uses Human74[4];005A2AC0 uses74[3]. Both are actual single
//holders, not equipment230's selected-model cache or Human74[1] slot array.
template<class Items>
class FirstPersonHumanItems {
public:
    using Entry=typename Items::Entry;
    explicit FirstPersonHumanItems(Items& items):items_(items) {}
    template<class Human>Entry clothing(Human& human) {return holder(human,4)->entry();}
    template<class Human>Entry weapon(Human& human) { //005A2B10's repeated native holder reads.
        auto* hand=holder(human,3);
        if(!hand->entry())return Entry{};
        if(items_.runtime_item(hand->entry()).family()!=Items::Native::Family::weapon)return Entry{};
        return hand->entry();
    }
    const auto& definition(Entry entry) {return items_.runtime_item(entry).definition();}
    const auto* definition_by_id(std::int32_t id)const{return items_.catalogue().item(id);}
    template<class Human>Entry kind9_holster(Human& human,std::uint32_t now){return find_player_mode_holster(human,items_,9,now);}
    float amount(Entry entry) {return items_.runtime_item(entry).amount();}
    auto model(Entry entry) {return items_.first_person_model(definition(entry));}
    auto magazine(Entry entry) {return items_.first_person_magazine(definition(entry));}
private:
    template<class Human>static auto holder(Human& human,std::size_t index) {
        auto* storage=human.base().storage();
        if(!storage||storage->single())throw std::runtime_error("First-person item binding requires actual Human storage");
        auto* result=storage->holder(index);
        if(!result)throw std::runtime_error("First-person Human holder is missing");
        return result;
    }
    Items& items_;
};
} //namespace hd2::game
#endif
