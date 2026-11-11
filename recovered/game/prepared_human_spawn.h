#ifndef HD2_GAME_PREPARED_HUMAN_SPAWN_H
#define HD2_GAME_PREPARED_HUMAN_SPAWN_H

#include "frontend/pre_mission_inventory.h"
#include "game/human_loading_stats.h"
#include "game/human_inventory_lookup.h"
#include "game/actor_activation_math.h"
#include "game/human_joint_math.h"
#include <cstdio>

namespace hd2::game {
inline std::int16_t prepared_identity_word(std::int32_t identity){
    const auto word=static_cast<std::uint16_t>(identity);std::int16_t value;std::memcpy(&value,&word,2);return value;
}

//00612A60/0048A780 use native clothing resource34. Keep the final dot while
//discarding its suffix; no extension is added when there was no dot. NA01 is
//the actual008397A4 fallback, not a successful substitute for a failed load.
inline std::string prepared_human_model_key(const frontend::PreMissionItemCatalogue& catalogue,
                                            std::int32_t clothing) {
    if(const auto* item=catalogue.item(clothing);item&&item->kind()==15) {
        auto key=item->resources[2];
        if(const auto dot=key.rfind('.');dot!=std::string::npos)key.erase(dot+1);
        if(key.size()>=256)throw std::length_error("Prepared Human model exceeds original stack buffer");
        return key;
    }
    return "NA01";
}

//0059EB60: populate the SAME Human storage from the live soldier, not a second
//opaque inventory.007D7810 owns instance allocation/identity/transfer/registration;
//clothing005A6930 must perform its real model replacement before controls bind.
template<class Human,class Items,class Clothing,class Mode>
void populate_prepared_human_storage_live(Human& human,frontend::PreMissionSoldier& soldier,
                                     Items& items,Clothing& clothing,Mode&& mode) {
    auto* storage=human.base().storage();
    if(!storage||storage->single())throw std::runtime_error("Prepared Human has no actual storage");
    storage->reset_spawn_entries();human.base().field7c(0); //007DDEF0 BEFORE any creation.
    constexpr std::array<unsigned,5> indices{{0,1,3,4,2}}; //Native4C/52/58/5E/64 order, not UI mask order.
    constexpr std::array<std::int32_t,5> slots{{0,4,5,2,3}};
    for(unsigned n=0;n!=indices.size();++n) {
        const auto value=soldier.equipment[indices[n]];if(value.item<0)continue;
        const auto entry=items.create_prepared_human_item(human,value.item,slots[n],value.position);
        //Read actual record/mode AFTER creation, before clothing. Native writes
        //only missing WORD identities; it does not replace an authored identity.
        if((mode()&7u)!=7u&&soldier.equipment[indices[n]].position==-1&&entry)
            soldier.equipment[indices[n]].position=prepared_identity_word(items.runtime_item(entry).identity);
        if(n==1&&(mode()&2u)&&entry)clothing.apply_prepared_human_clothing(human,items.runtime_item(entry).identity); //005A6930.
    }
    //006A2C90 split counts before this routine. Original creates ONE per record
    //even when a caller left a larger count; it does not expand them here.
    for(auto* list:{&soldier.holsters,&soldier.backpack}) {
        const auto slot=list==&soldier.holsters?1:6;
        for(std::size_t n=0;n<list->size();++n) {
            const auto value=list->at(n);
            const auto entry=items.create_prepared_human_item(human,value.item,slot,value.position);
            if((mode()&7u)!=7u&&list->at(n).position==-1&&entry)
                list->at(n).position=prepared_identity_word(items.runtime_item(entry).identity);
        }
    }
}
template<class Human,class Items,class Clothing>
void populate_prepared_human_storage(Human& human,frontend::PreMissionSoldier& soldier,
        Items& items,Clothing& clothing,std::uint32_t mode){
    populate_prepared_human_storage_live(human,soldier,items,clothing,[&]{return mode;});
}

//0059EDC0 offline property/profile/speech/equipment binding. Explicit stats copy
//all12 words; the profile path converts only nine and preserves the other three.
//Eye texture application follows storage/model replacement, not profile lookup.
template<class Human,class Store,class Speech,class Random,class Inventory,class Effects,class Peer>
void configure_prepared_human_with_peer(Human& human,frontend::PreMissionSoldier& soldier,
                             std::uint32_t kind,std::uint32_t personality,const std::array<std::uint32_t,12>* stats,
                             Store& store,Speech& speech,Random& random,Inventory& inventory,Effects& effects,Peer&& peer) {
    human.properties.field19c=kind;human.properties.field1a0=personality;
    human.properties.field170=static_cast<std::uint32_t>(soldier.human);
    if(soldier.human>=0)human.profile_record=store.record(soldier.human);
    peer(human,soldier); //0059EDC0 non7 B060/side/game78, BEFORE speech/stats/storage.
    if(kind==1) {
        if(human.speech_definition){speech.release(human.speech_definition);human.speech_definition=nullptr;}
        auto variant=soldier.speech_variant;
        if(variant<1||variant>4)variant=static_cast<std::int32_t>(random()%4u)+1; //Actual nonnegative C rand().
        human.speech_definition=speech.acquire(variant); //NULL result preserved; no fallback0 in this body.
    }
    if(stats)human.properties.stats_ec=*stats;
    else if(const auto* profile=human.profile_record)for(unsigned n=0;n!=9;++n) {
        float number;std::memcpy(&number,&profile->stats[n+1],4);
        human.properties.stats_ec[n]=human_loading_stat_word(number);
    }
    human.properties.stats_11c=human.properties.stats_ec;
    inventory.populate(human,soldier);
    if(human.profile_record){human.fieldbc=human.profile_record->eye;effects.human_eye_texture(human.base().frame(),human.fieldbc);}
    human.activation.flag178=true;
}
template<class Human,class Store,class Speech,class Random,class Inventory,class Effects>
void configure_prepared_human(Human& human,frontend::PreMissionSoldier& soldier,
        std::uint32_t kind,std::uint32_t personality,const std::array<std::uint32_t,12>* stats,
        Store& store,Speech& speech,Random& random,Inventory& inventory,Effects& effects){
    configure_prepared_human_with_peer(human,soldier,kind,personality,stats,store,speech,random,inventory,effects,[](auto&,auto&){});
}

//00612A60 ordinary offline creation. Models uses the existing category1 cache
//load_into on a real kind9 frame; Scene is the actual Scene2 identity registry.
//Setup must construct the containing Human/inline/state owners and configure
//them through the preceding source. Failure leaves registered actors available
//to the containing world's ordered cleanup; no privately owned duplicate actor.
template<class Frames,class Models,class Scene,class Clothing,class Finish>
auto create_prepared_human_body(Frames& frames,Models& models,Scene& scene,
                          const frontend::PreMissionItemCatalogue& catalogue,
                          const HumanVector& position,const HumanVector& direction,
                          std::int32_t identity,Clothing&& clothing,Finish&& finish) {
    auto model=frames.create_prepared_human_model();
    using HumanPointer=decltype(scene.create_prepared_human(model.get(),identity));
    if(!model)return HumanPointer{};
    const auto key=prepared_human_model_key(catalogue,clothing());
    if(models.load_model(model.get(),key,0)!=0)return HumanPointer{};
    frames.link_prepared_human_root(model.get()); //Actual scene1F8, link flag0.
    frames.object_position(model.get(),{{position[0],position[1],position[2]}});
    frames.prepared_human_direction(model.get(),direction);
    char name[64];std::snprintf(name,sizeof(name),"_PLAYER_%2d",identity);
    frames.game_camera_name(model.get(),name);frames.update_scene_object(model.get());
    auto* human=scene.create_prepared_human(model.get(),identity);
    model.reset(); //00612A60 releases factory reference BEFORE0059EDC0 setup.
    if(human)finish(*human);
    return human;
}
template<class Frames,class Models,class Scene,class Setup>
auto create_prepared_human(Frames& frames,Models& models,Scene& scene,Setup& setup,
        const frontend::PreMissionItemCatalogue& catalogue,const HumanVector& position,const HumanVector& direction,
        std::int32_t identity,frontend::PreMissionSoldier& soldier,std::int32_t clothing,bool alternate){
    return create_prepared_human_body(frames,models,scene,catalogue,position,direction,identity,[&]{return clothing;},
        [&](auto& human){setup.configure(human,soldier,alternate?4u:1u,1u);});
}
//Full00612A60 source factory boundary: distinct ECX peer / EDX Scene identity,
//actual optional stats and non7 clothing/setup/publication over SAME owners.
template<class Frames,class Models,class Scene,class Setup>
auto create_session_human(Frames& frames,Models& models,Scene& scene,Setup& setup,
        const frontend::PreMissionItemCatalogue& catalogue,const HumanVector& position,const HumanVector& direction,
        std::uint32_t peer,std::int32_t identity,frontend::PreMissionSoldier& soldier,
        const std::array<std::uint32_t,12>* stats,std::int32_t clothing,bool alternate){
    return create_prepared_human_body(frames,models,scene,catalogue,position,direction,identity,
        [&]{return (setup.mode_flags()&7u)==7u?clothing:soldier.equipment[1].item;},
        [&](auto& human){setup.configure_session(human,soldier,alternate?4u:1u,1u,stats,peer);setup.publish_peer_human(peer,human);});
}

//00611C90 offline authority spawn, including its actual initial identity and
//default clothing producers. Runtime/nonoffline positioning/packet branches are
//separate; Packet must supply the real local record/serialization receivers.
template<class Frames,class Scene,class Catalogue,class Create,class Packet,class Events>
void spawn_prepared_human(Frames& frames,Scene& scene,const Catalogue& catalogue,
                         Create& create,Packet& packet,Events& events,
                         typename Scene::Spawn& spawn,frontend::PreMissionSoldier& soldier,
                         std::uint32_t mode) {
    if(!(mode&1u))return;
    if((mode&7u)!=7u)throw std::runtime_error("Prepared spawn requires actual nonoffline positioning/transport");
    spawn.flag9c=false;const auto identity=scene.first_free_player_identity(); //May be-1; native appends then.
    const auto model=spawn.base().frame();
    const auto p=frames.actor_world_matrix(model),d=frames.actor_world_matrix(model);
    const HumanVector position{{p[12],p[13],p[14],p[15]}};
    //00611C90 stores(Y²+Z²)+X² but keeps the x87 value for the unit comparison.
    auto direction=HumanVector{{d[8],d[9],d[10],d[11]}};
    const auto extended_square=(static_cast<long double>(direction[1])*direction[1]+
        static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[0])*direction[0];
    const float square=static_cast<float>(extended_square);
    if(!std::isfinite(square))throw std::runtime_error("Non-finite prepared spawn direction");
    if(std::abs(extended_square-1)>=9.99999993922529e-09f) {
        if(square>=9.99999993922529e-09f) {
            const float scale=static_cast<float>(1/std::sqrt(static_cast<long double>(square)));
            for(unsigned axis=0;axis!=3;++axis)direction[axis]=static_cast<float>(static_cast<long double>(direction[axis])*scale);
        }else {const auto axis=direction[0]!=0?0u:direction[2]!=0?2u:1u;direction[axis]=direction[axis]<0?-1.f:1.f;}
    }
    if(soldier.equipment[1].item==-1) {
        const auto clothing=static_cast<std::int16_t>(spawn.properties.fielda4&0xffffu);
        (void)frontend::transfer_pre_mission_equipment(soldier,catalogue,2,{-1,clothing,1});
    }
    soldier.split_spawn_stacks();
    auto* human=create(position,direction,identity,soldier,spawn.properties.fielda4,spawn.properties.fielda0!=0);
    spawn.spawned_human=human; //NativeA8 typed borrower, even for a failed model/factory.
    if(!human)return;
    packet.bind_spawned_player(0,0,""); //0072E940 original caller ID0, not the allocated reserved identity.
    human->activate(true);if(mode&2u)human->activate(false);
    packet.publish_prepared_spawn(spawn,*human,soldier,position,direction); //006123A0 or00475530/local packet consumer.
    events.spawn_model_created(spawn.base().frame()); //Original frameEC+4 guard→0041D960(0).
}
} //namespace hd2::game
#endif
