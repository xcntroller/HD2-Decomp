#ifndef HD2_GAME_HUMAN_LOADING_INVENTORY_H
#define HD2_GAME_HUMAN_LOADING_INVENTORY_H

#include "game/human_inventory_lookup.h"
#include "game/human_loading_inventory_receiver.h"
#include "game/human_weapon_swap.h"
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

//005A2B40→00587BF0→005A2D50 offline loading call composition. Only the actual
//same-Human/empty-hand1/2/6→3 loading boundary is supported. Player1/remote7
//must use their genuine queued native84 commands; reject those before mutation.
template<class Frames,class Human,class Items,class States,class Inline,class Players,class Presentation,class World>
class HumanLoadingInventoryCommands final : public HumanLoadingInventoryReceiver {
public:
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
        auto* state=states_.get();if(!state)throw std::runtime_error("Inventory command requires actual published Human B0");
        const auto kind=state->words().template get<std::uint32_t>(0x48),mode=world_.mode_flags();
        if(kind==1||kind==7)throw std::runtime_error("Player/remote inventory command requires its actual queued packet consumer");
        if(kind!=3&&kind!=4&&kind!=5&&kind!=6&&kind!=8)throw std::runtime_error("Unrecovered inventory command state");
        if((mode&7u)!=7u)throw std::runtime_error("AI inventory command requires its actual outgoing packet owner outside offline7");
        if(&source!=&human_||destination_slot!=3||index!=0||(source_slot!=1&&source_slot!=2&&source_slot!=6))
            throw std::runtime_error("Inventory command is outside the recovered loading weapon move");
        if(!entry)throw std::runtime_error("Human inventory command is missing its actual item entry");
        //Native command carries item identity, then receiver looks it up again
        //on the resolved SOURCE actor. Never trust an entry from another slot.
        const auto found=find_human_inventory_entry(source,items_,items_.runtime_item(entry).identity,source_slot);
        if(!found)return false;
        auto* owner=control_.equipment();if(!owner)throw std::runtime_error("Human inventory command has no actual230 owner");
        std::int32_t result;
        if(source_slot==6) {
            auto* storage=human_.base().storage();
            if(!storage||!storage->holder(3)||!storage->holder(3)->empty())throw std::logic_error("Loading pack move requires actual empty hand");
            Entry displaced{};
            result=transfer_human_weapon(frames_,human_,items_,owner->transfers(),*this,found,displaced,true,6,3,0,true,false,false,false,mode,world_.game_word28());
        }else result=move_human_loading_weapon(frames_,human_,items_,owner->transfers(),*this,found,source_slot,mode,world_.game_word28());
        if(result!=1)return false;
        human_.activation.flag178=true;
        //005A8190(0) is gated by005A0890: offline B0 kind1 only, already
        //excluded here because its move is queued. AI has no player-mode reset.
        return true;
    }
    bool equip_shoulder()override{return equip_human_loading_shoulder(human_,items_,*this);}
    bool equip_holster(std::int32_t kind)override{return equip_human_loading_holster(human_,items_,*this,kind,world_.game_word28());}
    bool swap(Entry current,std::int32_t current_slot,Human& source,Entry selected,std::int32_t selected_slot) { //005A2BB0→00587E00→005A2E90.
        auto* state=states_.get();if(!state)throw std::runtime_error("Weapon swap requires actual Human B0");
        const auto kind=state->words().template get<std::uint32_t>(0x48),mode=world_.mode_flags();
        if((mode&7u)!=7u||kind==1||kind==7)throw std::runtime_error("Weapon swap requires actual queued/nonoffline packet owner");
        if(kind!=3&&kind!=4&&kind!=5&&kind!=6&&kind!=8)throw std::runtime_error("Unrecovered weapon swap state");
        if(&source!=&human_||current_slot!=3||(selected_slot!=1&&selected_slot!=2&&selected_slot!=6))
            throw std::runtime_error("Weapon swap is outside actual loading weapon slots");
        if(!current||!selected)throw std::runtime_error("Weapon swap command is missing actual entries");
        const auto found_current=find_human_inventory_entry(human_,items_,items_.runtime_item(current).identity,current_slot);
        const auto found_selected=find_human_inventory_entry(source,items_,items_.runtime_item(selected).identity,selected_slot);
        if(!found_current||!found_selected)return false;
        auto* owner=control_.equipment();if(!owner)throw std::runtime_error("Weapon swap has no actual230 owner");
        Entry displaced{};std::int32_t displaced_slot=7;
        const auto result=swap_human_loading_weapon(frames_,human_,items_,owner->transfers(),*this,found_selected,selected_slot,displaced,displaced_slot,mode,world_.game_word28());
        if(result!=1)return false;
        human_.activation.flag178=true;return true; //005A8190 mode0 skipped by genuine offline AI gate.
    }
    void select_weapon(Entry selected,std::int32_t slot) { //00538770, reread actual hand instead of trusting B0 cache.
        auto* storage=human_.base().storage();if(!storage||!storage->holder(3))throw std::runtime_error("Weapon selection has no actual hand holder");
        const auto current=storage->holder(3)->entry();if(!selected||selected==current)return;
        if(current)(void)swap(current,3,human_,selected,slot);
        else (void)move(selected,human_,slot,3,0);
    }
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
