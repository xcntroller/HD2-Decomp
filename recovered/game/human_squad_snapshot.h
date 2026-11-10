#ifndef HD2_GAME_HUMAN_SQUAD_SNAPSHOT_H
#define HD2_GAME_HUMAN_SQUAD_SNAPSHOT_H
#include "frontend/pre_mission_inventory.h"
namespace hd2::game {

//006A2A90 merges later matching records into the FIRST, retaining its position
//and native signed-WORD wrapping count. Does not sort or consolidate fixed slots.
inline void consolidate_prepared_soldier(frontend::PreMissionSoldier& soldier){
    for(auto* list:{&soldier.holsters,&soldier.backpack})for(std::size_t first=0;first<list->size();++first)
        for(auto later=list->size();later>first+1;){--later;
            if(list->at(first).item!=list->at(later).item)continue;
            const auto count=static_cast<std::uint16_t>(std::uint16_t(list->at(first).count)+std::uint16_t(list->at(later).count));
            std::memcpy(&list->at(first).count,&count,2);list->erase(list->begin()+static_cast<std::ptrdiff_t>(later));
        }
}

//006A2E50/006A2DC0, reached by00698F80 after snapshot/consolidation. Only this
//caller is bound here; preparation UI, stock/refund and team reconciliation are
//unchanged. Same HumanStore supplies assign006A2340 and selected catalogue items.
template<class Humans>
void refresh_prepared_soldier_totals(frontend::PreMissionSoldier& soldier,const frontend::PreMissionItemCatalogue& catalogue,Humans& humans){
    soldier.capacity=0;soldier.load=soldier.pack_capacity=soldier.pack_load=soldier.holster_load=0;
    soldier.pack_volume=soldier.holster_volume=soldier.pack_available_volume=0;
    if(soldier.human<0)return;
    const auto* profile=humans.record(soldier.human);
    if(!profile)throw std::runtime_error("Live squad extraction requires SAME actual HumanStore record");
    soldier.assign(soldier.human,*profile);
    const auto entry_total=[&](const frontend::PreMissionEquipmentEntry& entry){
        std::pair<float,std::uint32_t> result{};const auto* item=catalogue.item(entry.item);if(!item)return result;
        result.first=static_cast<float>(static_cast<long double>(item->weight)*entry.count);
        std::int32_t category;std::memcpy(&category,&item->category,4);
        //Signed DWORD→x87, zero selects1, otherwise double; __ftol keeps EAX.
        const auto volume=(category?static_cast<long double>(category)*2:1.L)*entry.count;
        if(!std::isfinite(volume)||volume<static_cast<long double>(std::numeric_limits<std::int64_t>::min())||
           volume>=-static_cast<long double>(std::numeric_limits<std::int64_t>::min()))
            throw std::runtime_error("Live equipment volume exceeds native conversion");
        result.second=static_cast<std::uint32_t>(static_cast<std::int64_t>(volume));return result;
    };
    const auto add=[](float a,float b){return static_cast<float>(static_cast<long double>(a)+b);};
    for(unsigned index:{0u,1u,3u,4u,2u})soldier.load=add(entry_total(soldier.equipment[index]).first,soldier.load);
    for(auto* list:{&soldier.holsters,&soldier.backpack})for(const auto& entry:*list){
        const auto [weight,volume]=entry_total(entry);auto& load=list==&soldier.holsters?soldier.holster_load:soldier.pack_load;
        auto& total=list==&soldier.holsters?soldier.holster_volume:soldier.pack_volume;
        const auto bits=static_cast<std::uint32_t>(total)+volume;std::memcpy(&total,&bits,4);
        load=add(weight,load);soldier.load=add(weight,soldier.load);
    }
    if(const auto* pack=catalogue.item(soldier.equipment[3].item);pack&&pack->payloads[0].type&&pack->payloads[0].bytes.size()>=8&&
       frontend::pre_mission_detail::Bytes::word(pack->payloads[0].bytes.data()+4)==7){
        const auto& payload=pack->payloads[0].bytes;
        if(payload.size()<20)throw std::runtime_error("Actual carried pack has no capacity payload");
        soldier.pack_capacity=frontend::pre_mission_detail::Bytes::real(frontend::pre_mission_detail::Bytes::word(payload.data()+12));
        const auto bits=frontend::pre_mission_detail::Bytes::word(payload.data()+16);float volume;std::memcpy(&volume,&bits,4);
        if(!std::isfinite(volume)||volume<static_cast<long double>(std::numeric_limits<std::int64_t>::min())||
           volume>=-static_cast<long double>(std::numeric_limits<std::int64_t>::min()))throw std::runtime_error("Actual pack volume exceeds native conversion");
        const auto converted=static_cast<std::uint32_t>(static_cast<std::int64_t>(volume));std::memcpy(&soldier.pack_available_volume,&converted,4);
    }
}

//005A8B50's ordinary offline inventory extraction. Uses actual holders and
//forward entries of occupied stacks. Positions stay untouched offline. Empty
//definition38 is omitted ONLY from holders2/3; native amount virtual0C is used.
template<class Human,class Items>
void snapshot_human_equipment(Human& human,frontend::PreMissionSoldier& soldier,Items& items,std::uint32_t mode){
    if((mode&7u)!=7u)throw std::runtime_error("Human snapshot requires actual nonoffline item-position consumers");
    auto* storage=human.base().storage();if(!storage||storage->single())throw std::runtime_error("Human snapshot has no actual Human storage");
    constexpr std::array<unsigned,5> holders{{0,4,5,2,3}},indices{{0,1,3,4,2}};
    for(unsigned n=0;n<holders.size();++n){
        auto* holder=storage->holder(holders[n]);if(!holder)throw std::logic_error("Human snapshot lost a required holder");
        if(const auto entry=holder->entry()){
            auto& item=items.runtime_item(entry);const auto definition=item.definition().identifier;
            if(n>=3&&definition==38&&item.amount()==0.f)continue; //0080F51C verified FLOAT0.
            const auto bits=static_cast<std::uint16_t>(definition);auto& value=soldier.equipment[indices[n]];
            std::memcpy(&value.item,&bits,2);value.count=1;
        }
    }
    for(unsigned index:{1u,6u})if(auto* stacks=storage->slots(index))
        for(std::size_t n=0;n<stacks->occupied_stacks();++n)
            for(const auto entry:stacks->slots().at(n).entries){
                const auto bits=static_cast<std::uint16_t>(items.runtime_item(entry).definition().identifier);
                frontend::PreMissionEquipmentEntry value;std::memcpy(&value.item,&bits,2);value.count=1;
                (index==1?soldier.holsters:soldier.backpack).push_back(value);
            }
}

//005A9040/005A8820 ordinary offline writeback. Caller clears equipment first.
//Native specialization40 and speech variant48 remain DISTINCT; voice is not
//invented or copied into40. Retain exact nine HumanC0 words, not profile stats.
template<class Human,class Items>
void snapshot_human_soldier(Human& human,frontend::PreMissionSoldier& soldier,Items& items,std::uint32_t mode){
    if((mode&7u)!=7u)throw std::runtime_error("Human soldier snapshot requires actual nonoffline state virtual64");
    soldier.word0c=human.properties.stats_ec[0];const auto kind=human.properties.field19c;
    soldier.specialization=kind==2?0:kind==1||kind==4?1:2;
    std::memcpy(&soldier.human,&human.properties.field170,4);
    snapshot_human_equipment(human,soldier,items,mode);soldier.state10=human.fieldc0;
}

//00698F80's leading offline extraction. Physical reserved lookup includes
//native pending-retirement Humans. Stop at first missing/nonkind1 Human; live
//record search is REVERSE and uses170 identity, never request squad metadata.
template<class Scene,class Items,class Humans>
void snapshot_live_squad(Scene& scene,std::vector<frontend::PreMissionSoldier>& squad,Items& items,
        const frontend::PreMissionItemCatalogue& catalogue,Humans& humans,std::uint32_t mode){
    if((mode&7u)!=7u)throw std::runtime_error("Live squad finish requires actual nonoffline squad-clear receiver");
    for(std::int32_t identity=0;;++identity){
        auto* human=scene.scene_human_identity(identity);if(!human||human->properties.field19c!=1)break;
        auto found=squad.rend();for(auto at=squad.rbegin();at!=squad.rend();++at)
            if(static_cast<std::uint32_t>(at->human)==human->properties.field170){found=at;break;}
        if(found!=squad.rend()){
            found->clear_equipment();snapshot_human_soldier(*human,*found,items,mode);
            consolidate_prepared_soldier(*found);refresh_prepared_soldier_totals(*found,catalogue,humans);
        }
        if(identity==std::numeric_limits<std::int32_t>::max())break;
    }
}
} //namespace hd2::game
#endif
