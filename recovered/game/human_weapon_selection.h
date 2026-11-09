#ifndef HD2_GAME_HUMAN_WEAPON_SELECTION_H
#define HD2_GAME_HUMAN_WEAPON_SELECTION_H

#include "game/human_inventory_lookup.h"
#include "game/human_inventory_counts.h"
#include "frontend/pre_mission_data.h"
#include <array>
#include <cmath>
#include <vector>

namespace hd2::game {

//0052F750's priority policy after resolving ACTUAL CD0 sector/target/distance.
//Caller must supply those real values; this record is not a world/sector service.
struct HumanWeaponSelectionContext {
    bool root_sector=false,has_target=false,human_target=false,special_target=false,alternate=false;
    long double squared_distance=0; //0052F750 compares x87 directly;00530B30 stores FLOAT first.
};
inline void human_weapon_priorities(const HumanWeaponSelectionContext& context,std::vector<std::uint32_t>& output) {
    output.clear();std::uint32_t remaining=0xfff;
    const auto append=[&](std::uint32_t mask){if(remaining&mask){output.push_back(mask);remaining&=~mask;}};
    if(context.has_target&&context.special_target)append(0x10);
    if(context.alternate){append(0x100);append(0x80);append(0x200);}
    //Unrolled native vector insertions, including bits400/800 never emitted.
    constexpr std::array<std::uint32_t,10> indoor{{0x100,0x200,0x80,2,4,1,0x40,8,0x10,0x20}};
    constexpr std::array<std::uint32_t,10> idle{{0x40,8,0x80,0x100,0x200,4,2,1,0x10,0x20}};
    constexpr std::array<std::uint32_t,10> distant_human{{0x40,8,4,0x80,0x100,2,0x10,0x200,1,0x20}};
    constexpr std::array<std::uint32_t,10> distant_other{{8,4,0x10,0x40,0x80,2,0x100,0x200,1,0x20}};
    constexpr std::array<std::uint32_t,10> close_human{{0x200,0x100,0x80,2,1,4,0x40,8,0x10,0x20}};
    constexpr std::array<std::uint32_t,10> close_other{{2,4,8,0x100,0x80,0x40,0x200,1,0x10,0x20}};
    constexpr std::array<std::uint32_t,10> middle_human{{0x100,0x200,0x80,2,4,0x40,8,0x10,1,0x20}};
    constexpr std::array<std::uint32_t,10> middle_other{{8,2,0x100,4,0x80,0x40,0x10,0x20,0x200,1}};
    const auto& order=!context.root_sector?indoor:!context.has_target?idle:context.squared_distance>1600?
        (context.human_target?distant_human:distant_other):context.squared_distance<=400?
        (context.human_target?close_human:close_other):(context.human_target?middle_human:middle_other);
    for(const auto mask:order)append(mask);
}
inline bool human_special_hand_definition(const frontend::PreMissionItem& definition) {
    const auto id=definition.identifier;return id==0x16||id==7||id==0x104;
}
inline bool human_weapon_secondary_variant(const frontend::PreMissionItem& definition) {
    if(definition.kind()!=4)return false;
    const auto& payload=definition.payloads[1];if(!payload.type)return false;
    if(payload.type==10&&payload.bytes.empty())return false; //Genuine zero-byte type9, not type4.
    if(payload.bytes.size()<12)throw std::runtime_error("Weapon secondary payload is truncated");
    if(frontend::pre_mission_detail::Bytes::word(payload.bytes.data()+4)!=4)return false;
    return frontend::pre_mission_detail::Bytes::integer(frontend::pre_mission_detail::Bytes::word(payload.bytes.data()+8))<4;
}

//005304B0: current hand3, shoulder2, then eligible holsters1/pack6, stacks
//ascending and each entry reversed. Return matching module ENTRY, not a copy.
template<class Human,class Items>
auto find_human_weapon_candidate(Human& human,Items& items,std::uint32_t kind,bool variant,bool special,std::int32_t& slot) {
    using Entry=std::decay_t<decltype(human.base().storage()->holder(0)->entry())>;slot=7;
    auto* storage=human.base().storage();
    if(!storage||storage->single())throw std::runtime_error("Weapon candidate lookup requires actual Human storage");
    const auto accepted=[&](Entry entry){
        auto& item=items.runtime_item(entry);const auto& definition=item.definition();
        return definition.kind()==kind&&human_weapon_secondary_variant(definition)==variant&&
            human_special_hand_definition(definition)==special&&
            (item.amount()!=0||count_human_ammunition(human,items,definition.ammunition(),true)!=0);
    };
    for(unsigned index:{3u,2u}) {
        auto* holder=storage->holder(index);if(!holder)throw std::runtime_error("Weapon candidate lookup lost required holder");
        if(!holder->empty())if(const auto entry=holder->entry();accepted(entry)){slot=static_cast<std::int32_t>(index);return entry;}
    }
    if(kind==3||kind==4||kind==5||kind==6||kind==12)return Entry{};
    for(unsigned index:{1u,6u})if(auto* stacks=storage->slots(index))
        for(std::size_t n=0;n<stacks->occupied_stacks();++n) {
            const auto& entries=stacks->slots().at(n).entries;
            for(auto at=entries.rbegin();at!=entries.rend();++at)if(accepted(*at)){slot=static_cast<std::int32_t>(index);return *at;}
        }
    return Entry{};
}

//00530930 ordered mask→kind/variant/special dispatch. Empty list leaves slot
//untouched, as native. Mask4 tries kind12 before kind4.
template<class Human,class Items>
auto select_human_weapon(Human& human,Items& items,const std::vector<std::uint32_t>& priorities,std::int32_t& slot) {
    using Entry=std::decay_t<decltype(human.base().storage()->holder(0)->entry())>;
    for(auto mask:priorities) {
        unsigned kind=0;bool variant=false,special=false;
        switch(mask) {
        case 1:kind=2;break;case 2:kind=3;break;
        case 4:if(auto entry=find_human_weapon_candidate(human,items,12,false,false,slot))return entry;kind=4;break;
        case 8:kind=5;break;case 0x10:kind=6;break;case 0x20:kind=9;break;
        case 0x40:kind=4;variant=true;break;case 0x80:kind=4;special=true;break;
        case 0x100:kind=3;special=true;break;case 0x200:kind=2;special=true;break;
        default:continue;
        }
        if(auto entry=find_human_weapon_candidate(human,items,kind,variant,special,slot))return entry;
    }
    return Entry{};
}

//00530B30: current item's amount/ammunition check precedes heavy-target/range
//policy. Exact squared constants1600/100/2500/225, unordered < branches retained.
template<class Human,class Items,class Entry,class Context>
bool human_weapon_usable_with_context(Human& human,Items& items,Entry entry,bool alternate,Context&& resolve_context) {
    if(!entry)return false;
    auto& item=items.runtime_item(entry);const auto& definition=item.definition();
    const bool special=human_special_hand_definition(definition);
    if(alternate&&!special)return false;
    if(item.amount()==0&&!count_human_ammunition(human,items,definition.ammunition(),true))return false;
    const auto command=definition.equipment_command();const auto context=resolve_context();
    if(context.has_target&&context.special_target)return command==5;
    const auto square=context.has_target?static_cast<float>(context.squared_distance):0.f;
    switch(command) {
    case 1:return square<=1600.f;
    case 2:if(!context.has_target&&!special)return false;return !(square>=100.f);
    case 3:case 4:return true;
    case 5:if(!context.has_target)return true;if(square<=100.f)return false;return !(square>=2500.f);
    case 6:if(square<=225.f)return false;return !(square>=1600.f);
    default:return false;
    }
}
template<class Human,class Items,class Entry>
bool human_weapon_usable(Human& human,Items& items,Entry entry,const HumanWeaponSelectionContext& context) {
    return human_weapon_usable_with_context(human,items,entry,context.alternate,[&]{return context;});
}
} //namespace hd2::game
#endif
