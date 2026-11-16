#ifndef HD2_GAME_HUMAN_ACTIVE_AI_LOOT_H
#define HD2_GAME_HUMAN_ACTIVE_AI_LOOT_H
#include "game/human_inventory_lookup.h"
#include "game/human_inventory_counts.h"
#include "game/human_inventory_transfer.h"
#include "game/human_joint_math.h"
#include <vector>
namespace hd2::game {
//007DAF80: the no-source transfer kernel is probed, never committed. Unlike
//placement007D9B40, failure returns the LAST kernel result, not always -102.
template<class Frames,class Actor,class Items,class World,class Entry>
std::int32_t probe_human_inventory_capacity(Frames& frames,Actor* destination,Items& items,World& world,Entry entry){
    if(!destination)return -100;
    auto* storage=destination->base().storage();if(!storage)throw std::logic_error("Inventory capacity lost actual storage74");
    if(storage->single())return -101;
    if(!entry)throw std::runtime_error("Inventory capacity reached native NULL entry dereference");
    auto& item=items.runtime_item(entry);const auto size=item.storage_size(),payload=item.primary_payload_kind();
    const auto empty=[&](unsigned slot){auto* holder=storage->holder(slot);if(!holder)throw std::logic_error("Inventory capacity lost Human holder");return holder->empty();};
    const auto attempt=[&](std::int32_t slot){Entry displaced{};return transfer_human_inventory(frames,items,world,entry,displaced,static_cast<Actor*>(nullptr),0,destination,slot,-1,static_cast<Actor*>(nullptr),true,false,false,true);};
    if(payload==7)return empty(5)?attempt(5):-102;
    if(payload==8&&empty(0))if(const auto result=attempt(0);result>0)return result;
    if(payload==3||payload==2||payload==0||payload==1){
        if(empty(3))if(const auto result=attempt(3);result>0)return result;
        if(payload==3||(payload==2&&(size==2||size==3)&&empty(2)))if(const auto result=attempt(2);result>0)return result;
    }
    const auto result=attempt(1);return result>0?result:attempt(6);
}
//005A7850/005A7610: snapshot ALL matching entries, receiver order0..6,
//occupied stacks forward, their entries reverse. Transfers start after lookup.
template<class Human,class Items>auto collect_human_inventory_kind(Human& human,Items& items,std::uint32_t kind){
    using Entry=std::decay_t<decltype(human.base().storage()->holder(0)->entry())>;
    std::vector<std::pair<Entry,std::int32_t>> result;
    auto* storage=human.base().storage();if(!storage||storage->single())throw std::logic_error("Inventory collection requires actual Human storage");
    const auto append=[&](Entry entry,unsigned slot){if(entry&&items.runtime_item(entry).definition().kind()==kind)result.emplace_back(entry,static_cast<std::int32_t>(slot));};
    for(unsigned slot=0;slot!=7;++slot){
        if(auto* holder=storage->holder(slot))append(holder->entry(),slot);
        else if(auto* stacks=storage->slots(slot))for(std::size_t n=0;n<stacks->occupied_stacks();++n){
            const auto& entries=stacks->slots().at(n).entries;for(auto at=entries.rbegin();at!=entries.rend();++at)append(*at,slot);
        }
    }
    return result;
}
//00537350: first failed request stops traversal without undoing earlier moves.
template<class Human,class Items,class World>bool active_take_inventory_kind(Human& human,Human& source,Items& items,World& world,std::uint32_t kind){
    const auto entries=collect_human_inventory_kind(source,items,kind);
    for(const auto& selected:entries)if(!world.active_request_inventory_move(human,source,selected.first,selected.second,1))return false;
    return !entries.empty();
}
//00536D30: relookup after EACH request, max3. A false005A2D50 is deliberately
//ignored, so the same untransferred entry can be selected three times.
template<class Human,class Items,class World>bool active_take_ammunition(Human& human,Human& source,Items& items,World& world,std::int32_t definition){
    bool any=false;
    for(unsigned n=0;n!=3;++n){
        std::int32_t slot=7;const auto entry=find_human_inventory_definition(source,items,definition,&slot);
        if(!entry||world.active_inventory_capacity(human,entry)<1)return any;
        (void)world.active_receive_inventory_move(human,source,entry,slot);any=true;
    }
    return any;
}
//00536DC0: live traversal (not a collection). Pack6's inverted payload3
//predicate is original; do not replace it with the holster1 predicate.
template<class Human,class Items,class World>bool active_take_weapon_ammunition(Human& human,Human& source,Items& items,World& world){
    auto* storage=human.base().storage();if(!storage||storage->single())throw std::logic_error("Active ammunition collection requires Human storage");
    bool any=false;const auto take=[&](auto entry,bool pack){
        if(!entry)return;
        auto& item=items.runtime_item(entry);const bool weapon=item.primary_payload_kind()==3;
        if(weapon!=pack){const bool moved=active_take_ammunition(human,source,items,world,item.definition().ammunition());any=any|moved;}
    };
    for(unsigned slot:{3u,2u}){auto* holder=storage->holder(slot);if(!holder)throw std::logic_error("Active ammunition collection lost holder");take(holder->entry(),false);}
    for(unsigned slot:{1u,6u})if(auto* stacks=storage->slots(slot))for(std::size_t n=0;n<stacks->occupied_stacks();++n){
        //Native captures this stack's reverse count, but rereads the actual
        //vector for each entry. Invalidating callbacks are required failures.
        for(auto at=stacks->slots().at(n).entries.size();at;){const auto entry=stacks->slots().at(n).entries.at(--at);take(entry,slot==6);}
    }
    return any;
}
//00479600 geometry. Raw x87 sums (Z²+X²)+Y²; AND4100 accepts unordered.
inline bool active_loose_weapon_in_radius(const HumanVector& origin,const HumanVector& position,float radius){
    const auto x=static_cast<long double>(origin[0])-position[0],y=static_cast<long double>(origin[1])-position[1],z=static_cast<long double>(origin[2])-position[2];
    const float limit=static_cast<float>(static_cast<long double>(radius)*radius);return !(((z*z+x*x)+y*y)>limit);
}
//00536FB0: target hand wins ties, then shoulder, then first nearby loose
//weapon. Counts are target ammunition even for the loose candidate; pack is0.
template<class Human,class Items,class World>bool active_take_better_weapon(Human& human,Human& source,Items& items,World& world){
    using Entry=std::decay_t<decltype(human.base().storage()->holder(0)->entry())>;
    const auto eligible=[&](Entry entry){if(!entry)return false;const auto kind=items.runtime_item(entry).definition().item_class;return kind==1||kind==3||kind==4;};
    auto* loose=world.active_nearby_loose_weapon(source.base().world_position(),3.f);std::int32_t loose_count=0;
    if(loose){if(eligible(loose->entry()))loose_count=count_human_ammunition(source,items,items.runtime_item(loose->entry()).definition().ammunition(),false);else loose=nullptr;}
    const auto validate=world.active_loot_borrowers(human,source,loose);
    const auto pair=world.active_inventory_pair_borrowers(human,source);
    std::array<Entry,2> selected{},own{};std::array<std::int32_t,2> counts{},own_counts{};
    for(unsigned phase=0;phase!=2;++phase){auto& actor=phase?human:source;auto* storage=actor.base().storage();
        if(!storage||storage->single())throw std::logic_error("Active weapon collection lost Human storage");
        for(unsigned n=0;n!=2;++n){auto* holder=storage->holder(3-n);if(!holder)throw std::logic_error("Active weapon collection lost holder");
            const auto entry=holder->entry();if(eligible(entry)){(phase?own:selected)[n]=entry;(phase?own_counts:counts)[n]=count_human_ammunition(actor,items,items.runtime_item(entry).definition().ammunition(),false);}
        }
    }
    unsigned chosen=2;
    if(selected[0]&&counts[0]>=counts[1]&&counts[0]>=loose_count)chosen=0;
    else if(selected[1]&&counts[1]>=counts[0]&&counts[1]>=loose_count)chosen=1;
    else if(!loose||loose_count<counts[0]||loose_count<counts[1])return false;
    if(own_counts[0]>=2||own_counts[1]>=2)return false;
    for(const auto entry:own)if(entry){world.active_drop_loot_weapon(human,entry,human.base().world_direction());validate();}
    const auto definition=items.runtime_item(chosen==2?loose->entry():selected[chosen]).definition().ammunition();
    const bool moved=chosen==2?world.active_receive_loose_weapon(human,*loose):world.active_receive_inventory_move(human,source,selected[chosen],static_cast<std::int32_t>(3-chosen));
    pair(); //The chosen loose actor may now be retired; only Human loans remain.
    (void)active_take_ammunition(human,source,items,world,definition);return moved;
}
//00537300: all four calls execute, including after an earlier success.
template<class Human,class Items,class World>bool active_loot_human(Human& human,Human* source,Items& items,World& world){
    if(!source)return false;
    const bool ammunition=active_take_weapon_ammunition(human,*source,items,world);
    const bool weapon=active_take_better_weapon(human,*source,items,world);
    const bool grenades=active_take_inventory_kind(human,*source,items,world,9);
    const bool supplies=active_take_inventory_kind(human,*source,items,world,13);
    return ammunition|weapon|grenades|supplies;
}
template<class Human>auto active_handoff_entry(Human& human,unsigned slot){
    auto* storage=human.base().storage();auto* holder=storage?storage->holder(slot):nullptr;
    if(!holder)throw std::runtime_error("Active handoff reached native missing Human holder dereference");
    return holder->entry();
}
//00528950/task24: clothing is reread after the swap, head entries after BOTH
//clothing callbacks. Even successful head transfer cannot repair a false
//clothing-swap result. It still runs before that final rejection.
template<class Human,class World>bool active_handoff_inventory(Human& human,Human& source,World& world){
    const auto validate=world.active_inventory_pair_borrowers(human,source);
    const auto selected=active_handoff_entry(source,4),current=active_handoff_entry(human,4);
    const bool clothing=world.active_request_inventory_swap(human,current,4,source,selected,4);
    validate();
    if(clothing){world.active_apply_loot_clothing(human,active_handoff_entry(human,4));validate();world.active_apply_loot_clothing(source,active_handoff_entry(source,4));validate();}
    const auto head=active_handoff_entry(source,0),own_head=active_handoff_entry(human,0);
    if(head){const bool moved=own_head?world.active_request_inventory_swap(human,own_head,0,source,head,0):world.active_request_inventory_move(human,source,head,0,0);validate();if(!moved)return false;}
    return clothing;
}
} //namespace hd2::game
#endif
