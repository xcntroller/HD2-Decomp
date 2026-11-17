#ifndef HD2_GAME_HUMAN_LOADING_INVENTORY_H
#define HD2_GAME_HUMAN_LOADING_INVENTORY_H

#include "game/human_inventory_lookup.h"
#include "game/human_loading_inventory_receiver.h"
#include "game/human_weapon_swap.h"
#include "game/human_weapon_context.h"
#include "frontend/pre_mission_data.h"
#include <cstring>

namespace hd2::game {

//007D8B40's actual loading weapon move: SAME Human, source1/2→EMPTY hand3,
//index0/context0/notify1/override0/retain0/dryRun0. It is distinct from the
//007D81A0 no-source transfer. General exchanges/drops are not supplied here.
//Receivers are actual230, Players/HUD/FPV and B0 owners, not success callbacks.
template<class Frames,class Human,class Items,class Equipment,class Receivers,class Entry>
std::int32_t move_human_loading_weapon(Frames& frames,Human& human,Items& items,Equipment& equipment,
        Receivers& receivers,Entry entry,std::int32_t source_slot,std::uint32_t mode,std::uint32_t game28) {
    if(!entry)return -1;
    if(source_slot!=1&&source_slot!=2)throw std::logic_error("Loading weapon move requires actual source1/2");
    auto& item=items.runtime_item(entry);const auto model=item.model(); //Before destination validation.
    auto* storage=human.base().storage();
    if(!storage||storage->single())throw std::runtime_error("Loading weapon move has no actual Human storage");
    auto* hand=storage->holder(3);auto* shoulder=source_slot==2?storage->holder(2):nullptr;
    auto* holsters=source_slot==1?storage->slots(1):nullptr;
    if(!hand||(!shoulder&&!holsters))throw std::runtime_error("Loading weapon move lost a required storage owner");
    if(!hand->empty())throw std::logic_error("Loading weapon move requires the actual empty-hand caller gate");
    if(static_cast<std::uint32_t>(item.family())!=0)return -8;
    if(hand->first_scalar()<static_cast<long double>(item.scalar()))return -5;
    if(hand->second_scalar()<item.volume())return -6;
    if(frontend::pre_mission_detail::Bytes::integer(hand->flags())<item.storage_size())return -7;
    const bool enabled=items.equipment_notifications_enabled();
    if(enabled&&(mode&1u)) {
        if(hand->cooling_down(game28))return -10;
        if(shoulder&&shoulder->cooling_down(game28))return -9; //Stack-array+48 is genuine false.
    }
    Entry displaced{};const auto result=hand->replace(entry,displaced);if(result<1)return result;
    //Native performs BOTH stores even on a same-actor move. Preserve rounding.
    float weight=frontend::pre_mission_detail::Bytes::real(human.base().field7c());
    const auto store_weight=[&]{std::uint32_t bits;std::memcpy(&bits,&weight,4);human.base().field7c(bits);};
    weight=static_cast<float>(static_cast<long double>(item.scalar())+weight);store_weight();
    weight=static_cast<float>(static_cast<long double>(weight)-item.scalar());store_weight();
    item.holder_actor=&human.base();item.holder_slot=3;
    if(shoulder)(void)shoulder->detach(entry);else (void)holsters->detach(entry);
    if(item.flag1c&&model){item.flag1c=false;frames.object_visible(model,false);}
    if(enabled) {
        equipment.removed(entry,source_slot,&equipment,3);
        equipment.moved(entry,3,&equipment,source_slot,false);
        if((mode&2u)&&receivers.is_current_player(human))receivers.hud_inventory_changed(); //0049FBF0.
        human.activation.flag178=true;
        hand->field18(game28+0x398u);receivers.refresh_item(); //005A6A90→00584830.
        if((mode&2u)&&receivers.is_current_player(human)&&!(equipment.transfer_state().words().template get<std::uint32_t>(0x50)&1u))
            receivers.first_person_request(mode,8,0); //00492930.
    }
    item.field14=game28+0x398u;return result;
}

//005A67C0/005A6830 loading callers. Commands is the actual B0+74 dispatcher;
//it must not silently make player/remote queued commands immediate.
template<class Human,class Items,class Commands>
bool equip_human_loading_shoulder(Human& human,Items&,Commands& commands) { //005A67C0.
    auto* storage=human.base().storage();
    if(!storage||storage->single()||!storage->holder(3)||!storage->holder(2))
        throw std::runtime_error("Human loading hand selection requires actual holders2/3");
    if(!storage->holder(3)->empty()||storage->holder(2)->empty())return false;
    const auto result=commands.move(storage->holder(2)->entry(),human,2,3,0);commands.refresh_item();return result;
}
template<class Human,class Items,class Commands>
bool equip_human_loading_holster(Human& human,Items& items,Commands& commands,std::int32_t kind,std::uint32_t game28) { //005A6830.
    using Entry=std::decay_t<decltype(human.base().storage()->holder(3)->entry())>;
    auto* storage=human.base().storage();
    if(!storage||storage->single()||!storage->holder(3)||!storage->slots(1))
        throw std::runtime_error("Human loading hand selection requires actual hand/holsters");
    if(!storage->holder(3)->empty())return false;
    if(kind!=2&&kind!=9&&kind!=10&&kind!=11)return false;
    Entry selected{};auto& holsters=*storage->slots(1);
    for(auto n=holsters.occupied_stacks();n;) {
        const auto& stack=holsters.slots().at(--n);if(stack.entries.empty())throw std::runtime_error("Human holsters lost an occupied stack");
        const auto entry=stack.entries.front();auto& item=items.runtime_item(entry);
        //Original resets its kind argument and ALWAYS selects kind2. Continue
        //reverse scan after a match, choosing the lowest matching stack index.
        if(item.field14<=game28&&item.definition().kind()==2)selected=entry;
    }
    if(!selected)return false;
    const auto result=commands.move(selected,human,1,3,0);commands.refresh_item();return result;
}

//Shared request policies keep Human callers on the actual general receiver
//while preserving Player/Remote queue ownership. These do not own inventory.
template<class Human,class Items,class States,class World,class Entry>
bool request_human_loading_inventory_move(Human& human,Items& items,States& states,World& world,Entry entry,Human& source,
        std::int32_t source_slot,std::int32_t destination_slot,std::int32_t index){
    auto* state=states.get();if(!state)throw std::runtime_error("Inventory command requires actual published Human B0");
    const auto kind=state->words().template get<std::uint32_t>(0x48),mode=world.mode_flags();
    if(kind==1||kind==7){
        if((mode&7u)!=7u)throw std::runtime_error("Queued inventory publication requires actual nonoffline transport");
        if(!entry)throw std::runtime_error("Human inventory request is missing its actual item entry");
        return state->queue_inventory_move(items.runtime_item(entry).identity,&source==&human?-1:source.base().id10(),source_slot,destination_slot,index);
    }
    if(kind!=3&&kind!=4&&kind!=5&&kind!=6&&kind!=8)throw std::runtime_error("Unrecovered inventory command state");
    if(!entry)throw std::runtime_error("Human inventory command is missing its actual item entry");
    //Actual common00587BF0 delegates to the completed005A2D50 receiver.
    //It owns canonical source lookup, general transfer and post-effects;
    //non7 forwarding is required only AFTER a successful transfer.
    return world.request_human_inventory_move(human,items.runtime_item(entry).identity,
        &source==&human?-1:source.base().id10(),source_slot,destination_slot,index);
}
template<class Human,class Items,class States,class World,class Entry>
bool request_human_loading_inventory_swap(Human& human,Items& items,States& states,World& world,Entry current,std::int32_t current_slot,
        Human& source,Entry selected,std::int32_t selected_slot){
    if(!world.human_inventory_swap_eligible(human,source,selected_slot))return false; //005A2BB0 BEFORE B0/item identity reads.
    auto* state=states.get();if(!state)throw std::runtime_error("Weapon swap requires actual Human B0");
    const auto kind=state->words().template get<std::uint32_t>(0x48),mode=world.mode_flags();
    if(kind==1||kind==7){
        if((mode&7u)!=7u)throw std::runtime_error("Queued weapon swap requires actual nonoffline transport");
        if(!current||!selected)throw std::runtime_error("Weapon swap request is missing actual entries");
        return state->queue_inventory_swap(items.runtime_item(current).identity,current_slot,&source==&human?-1:source.base().id10(),items.runtime_item(selected).identity,selected_slot);
    }
    if(kind!=3&&kind!=4&&kind!=5&&kind!=6&&kind!=8)throw std::runtime_error("Unrecovered weapon swap state");
    if(!current||!selected)throw std::runtime_error("Weapon swap command is missing actual entries");
    //Same common00587E00/005A2E90 owner as received commands. The old
    //loading-only weapon helper must not restrict general AI slot/fallback.
    return world.request_human_inventory_swap(human,items.runtime_item(current).identity,current_slot,
        &source==&human?-1:source.base().id10(),items.runtime_item(selected).identity,selected_slot);
}

//005A2B40/005A2BB0 use the SAME general00587BF0/00587E00 command receivers
//as received inventory commands. Player/Remote remain queued; AI delegates to
//actual full transfer/swap owners. A queued result does not complete its effect.
template<class Frames,class Human,class Items,class States,class Inline,class Players,class Presentation,class World>
class HumanLoadingInventoryCommands final : public HumanLoadingInventoryReceiver<Frames> {
public:
    using Actor=SceneActor<Frames>;
    using Entry=std::decay_t<decltype(std::declval<Human&>().base().storage()->holder(3)->entry())>;
    HumanLoadingInventoryCommands(Frames& frames,Human& human,Items& items,States& states,Inline& control,
            Players& players,Presentation& presentation,World& world)
        :frames_(frames),human_(human),items_(items),states_(states),control_(control),players_(players),presentation_(presentation),world_(world) {
        if(human_.loading_inventory)throw std::logic_error("Human already has its actual inventory command owner");
        human_.loading_inventory=this;
    }
    ~HumanLoadingInventoryCommands()override{if(human_.loading_inventory==this)human_.loading_inventory=nullptr;}
    HumanLoadingInventoryCommands(const HumanLoadingInventoryCommands&)=delete;
    HumanLoadingInventoryCommands& operator=(const HumanLoadingInventoryCommands&)=delete;
    bool move(Entry entry,Human& source,std::int32_t source_slot,std::int32_t destination_slot,std::int32_t index) {
        return request_human_loading_inventory_move(human_,items_,states_,world_,entry,source,source_slot,destination_slot,index);
    }
    bool equip_shoulder()override{return equip_human_loading_shoulder(human_,items_,*this);}
    bool equip_holster(std::int32_t kind)override{return equip_human_loading_holster(human_,items_,*this,kind,world_.game_word28());}
    bool swap(Entry current,std::int32_t current_slot,Human& source,Entry selected,std::int32_t selected_slot) { //005A2BB0→00587E00→005A2E90.
        return request_human_loading_inventory_swap(human_,items_,states_,world_,current,current_slot,source,selected,selected_slot);
    }
    void select_weapon(Entry selected,std::int32_t slot)override { //00538770, reread actual hand instead of trusting B0 cache.
        auto* storage=human_.base().storage();if(!storage||!storage->holder(3))throw std::runtime_error("Weapon selection has no actual hand holder");
        const auto current=storage->holder(3)->entry();if(!selected||selected==current)return;
        if(current)(void)swap(current,3,human_,selected,slot);
        else (void)move(selected,human_,slot,3,0);
    }
    bool weapon_usable(Entry entry,Actor* target,bool alternate)override {
        return human_weapon_usable_with_context(human_,items_,entry,alternate,[&]{
            return human_weapon_context(frames_,human_,control_,world_,target,alternate,false);
        });
    }
    Entry preferred_weapon(Actor* target,bool alternate,std::int32_t& slot)override {
        const auto context=human_weapon_context(frames_,human_,control_,world_,target,alternate,true);
        std::vector<std::uint32_t> priorities;human_weapon_priorities(context,priorities);
        return select_human_weapon(human_,items_,priorities,slot);
    }
    bool special_hand_item(Entry entry)override{return entry&&human_special_hand_definition(items_.runtime_item(entry).definition());}
    void refresh_item(){if(auto* state=states_.get())state->refresh_item();} //005A6A90.
    std::int32_t actor_inventory_capacity(Human& human){return world_.actor_inventory_capacity(human.base());}
    bool is_current_player(Human& human){return players_.current_player()==&human;}
    void hud_inventory_changed(){presentation_.hud_inventory_changed();} //Required actual0049FBF0.
    void first_person_request(std::uint32_t mode,std::int32_t animation,std::uint32_t argument){presentation_.first_person_request(mode,animation,argument);}
private:
    Frames& frames_;Human& human_;Items& items_;States& states_;Inline& control_;Players& players_;Presentation& presentation_;World& world_;
};
} //namespace hd2::game
#endif
