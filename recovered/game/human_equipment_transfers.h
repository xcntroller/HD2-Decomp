#ifndef HD2_GAME_HUMAN_EQUIPMENT_TRANSFERS_H
#define HD2_GAME_HUMAN_EQUIPMENT_TRANSFERS_H

#include "game/human_holsters.h"
#include "game/human_equipment_state.h"
#include "game/human_equipment_mount.h"

namespace hd2::game {

//00792590/00792720/00792AB0 inventory notifications and007913D0 events0..3/14.
//This is the SAME230 state/holsters/DD0/E0, not an inventory or animation owner.
//Pending peers must outlive the handoff; the world stops animation before
//retiring Human inline owners. Event14 shares the actual mounted-model release.
template<class Frames,class Items,class Services,class Holsters,class Attachments>
class HumanEquipmentTransfers final : public HumanEquipmentTransferPeer<typename Frames::Frame> {
public:
    using Frame=typename Frames::Frame;using Peer=HumanEquipmentTransferPeer<Frame>;
    using State=HumanEquipmentState<Frame>;
    HumanEquipmentTransfers(Frames& frames,Items& items,Services& services,Holsters& holsters,
            Attachments& attachments,State& state)
        :frames_(frames),items_(items),services_(services),holsters_(holsters),attachments_(attachments),state_(state) {}
    State& transfer_state()override{return state_;}
    std::int32_t transfer_life()const override{return services_.equipment_life_value();}
    void attach_inventory_model(unsigned kind)override{attachments_.attach(kind);}
    template<class Entry>void hide_inventory_holster(Entry entry,std::int32_t slot) { //007D9DE0's explicit pre-notification counts.
        auto& item=items_.runtime_item(entry);
        if(slot==1||(slot==3&&human_item_holster_group(item.definition())==0))holsters_.hide_items(item.definition(),1);
    }
    void refresh_inventory_holster_animation(){services_.refresh_inventory_holster_animation();}
    void remove_inventory_model(unsigned kind)override { //00790520 inventory0..3, held/hand actions4..8 and mounted action9.
        if(kind==4||kind==8){retire_human_equipment_held_action(frames_,state_,services_,kind);return;}
        if(kind==9){retire_human_equipment_mount(frames_,state_,services_);return;}
        if(kind==5){retire_human_equipment_hand_action(frames_,state_,kind);return;}
        if(kind==6||kind==7){
            const auto model=state_.model(1);if(!model)return;
            auto& words=state_.words();const auto mode=words.template get<std::uint32_t>(0xb0);
            if(mode!=kind&&mode!=11){remove_inventory_model(mode);return;}
            retire_human_equipment_hand_action(frames_,state_,kind);return;
        }
        unsigned index;
        switch(kind){case 0:index=3;break;case 1:index=4;break;case 2:index=2;break;case 3:index=0;break;
            default:throw std::runtime_error("Equipment removal requires its actual action-model producer");}
        const auto model=state_.model(index);if(!model)return;
        if(kind==3&&services_.equipment_weapon_stowed())event(1);
        frames_.human_equipment_link(model,Frame{},0);state_.model(index,Frame{});
        if(kind==0){services_.equipment_face_flag(false);services_.refresh_equipment_visibility();}
        else if(kind==1) {
            for(const auto& part:attachments_.parts())if(part)frames_.human_equipment_link(part.get(),Frame{},0);
            services_.refresh_equipment_visibility();
        }else if(kind==3) {
            services_.set_equipment_weapon(Frame{},-1);services_.equipment_primary_face(0);
            services_.equipment_secondary_face(0);services_.refresh_equipment_visibility();
        }
    }
    template<class Entry>void added(Entry entry,std::int32_t slot) { //00792590, no source actor.
        auto& item=items_.runtime_item(entry);
        switch(slot) {
        case 0:select(item,0,-1);attachments_.attach(0);return;
        case 1:show(item);return;
        case 2:select(item,2);attachments_.attach(2);return;
        case 3:
            if(human_item_holster_group(item.definition())==0)show(item);
            select(item,3);services_.equipment_transfer_mode(item.definition().equipment_command(),3,false);
            services_.force_equipment_command(item.definition().equipment_command());event(1);return;
        case 5:select(item,1);attachments_.attach(1);return;
        default:return; //Native slots4/6 have no230 notification.
        }
    }
    template<class Entry>void removed(Entry entry,std::int32_t source_slot,Peer* destination,std::int32_t destination_slot) { //00792AB0.
        auto& item=items_.runtime_item(entry);
        const bool taking=destination_slot==3&&destination&&destination->transfer_life()>0&&!back(*destination);
        switch(source_slot) {
        case 0:if(!taking){remove_inventory_model(0);state_.model(3,Frame{});}return;
        case 1:holsters_.hide_items(item.definition(),1);return;
        case 2:if(!taking){remove_inventory_model(2);state_.model(2,Frame{});}return;
        case 3:
            if(human_item_holster_group(item.definition())==0)holsters_.hide_items(item.definition(),1);
            if(taking){state_.model(0,Frame{});services_.set_equipment_weapon(Frame{},-1);}
            else if(!destination) {
                remove_inventory_model(3);services_.set_equipment_weapon(Frame{},-1);services_.force_equipment_command(100);return;
            }else if(transfer_life()<1){remove_inventory_model(3);return;}
            else {
                state_.departing(destination);
                switch(destination_slot) {
                case 0:state_.words().set(8,std::uint32_t{0});break;
                case 2:state_.words().set(8,std::uint32_t{2});break;
                case 3:state_.words().set(8,std::uint32_t{3});break;
                case 5:state_.words().set(8,std::uint32_t{1});break;
                default:state_.departing(nullptr);break;
                }
            }
            services_.force_equipment_command(0);return;
        case 5:if(!taking){remove_inventory_model(1);state_.model(4,Frame{});}return;
        default:return;
        }
    }
    template<class Entry>void moved(Entry entry,std::int32_t destination_slot,Peer* source,std::int32_t source_slot,bool retain_source) { //00792720.
        auto& item=items_.runtime_item(entry);
        const bool giving=source_slot==3&&source&&source->transfer_life()>0;
        const auto finish_source=[&]{
            if(giving&&back(*source)) {
                auto& state=source->transfer_state();state.words().set(0xd4,std::int32_t{-10000});
                state.model(0,Frame{});state.words().set(0x30,std::int32_t{-1});state.words().set(0x34,std::int32_t{-1});
            }
        };
        switch(destination_slot) {
        case 0:select(item,0,-1);if(!giving||back(*source))attachments_.attach(0);finish_source();return;
        case 1:show(item);break;
        case 2:select(item,2);if(!giving||back(*source))attachments_.attach(2);break;
        case 3: {
            if(human_item_holster_group(item.definition())==0)show(item);
            if(transfer_life()<1){select(item,3);if(!giving)attachments_.attach(3);return;}
            const auto previous=state_.selected_model(3);
            const auto previous_identity=state_.words().template get<std::uint32_t>(0x30);
            const auto previous_definition=state_.words().template get<std::int32_t>(0x34);
            select(item,3);
            if(!giving||back(*source)) {
                if(!back(*this)) {
                    state_.arriving(source);state_.words().set(0x14,std::uint8_t(retain_source));
                    if(retain_source)event(1);
                    switch(source_slot) {
                    case 0:state_.words().set(0x10,std::uint32_t{0});break;
                    case 1:
                        state_.arriving(nullptr);
                        if(previous) {
                            state_.words().set(0xd4,std::int32_t{-10000});
                            state_.select(previous,3,previous_identity,-10000,previous_definition);
                            remove_inventory_model(3);state_.words().set(0xd4,std::int32_t{-10000});select(item,3);
                        }
                        break;
                    case 2:state_.words().set(0x10,std::uint32_t{2});break;
                    case 3:state_.words().set(0x10,std::uint32_t{3});break;
                    case 5:state_.words().set(0x10,std::uint32_t{1});break;
                    default:state_.arriving(nullptr);break;
                    }
                }else {
                    if(!source)throw std::runtime_error("Back-hand transfer is missing its actual source Human");
                    if(!back(*source)&&source_slot==3){source->transfer_state().departing(this);state_.words().set(8,std::uint32_t{3});}
                    else attachments_.attach(3);
                }
            }
            services_.equipment_transfer_mode(0,0,true);services_.force_equipment_command(item.definition().equipment_command());return;
        }
        case 5:select(item,1);if(!giving||back(*source))attachments_.attach(1);break;
        default:return;
        }
        finish_source();
    }
    void event(unsigned command) { //007913D0 inventory events; no invented completed animation tick.
        auto& words=state_.words();
        switch(command) {
        case 0:case 1:
            if(services_.equipment_weapon_stowed()!=(command!=0))return;
            if(!back(*this))services_.stow_equipment_weapon(command==0);
            else words.set(0x50,command==0?(words.template get<std::uint32_t>(0x50)|2u):(words.template get<std::uint32_t>(0x50)&~2u));
            return;
        case 2: {
            Frame moved_model{};if(state_.model(0)&&services_.equipment_weapon_stowed())event(1);
            if(auto* peer=state_.departing()) {
                const auto kind=words.template get<std::uint32_t>(8);moved_model=peer->transfer_state().selected_model(kind);
                peer->attach_inventory_model(kind);state_.departing(nullptr);
            }
            if(state_.model(0)&&moved_model!=state_.model(0))remove_inventory_model(3);
            state_.model(0,Frame{});return;
        }
        case 3:
            if(auto* peer=state_.arriving()) {
                const auto kind=words.template get<std::uint32_t>(0x10);
                if(words.template get<std::uint8_t>(0x14))peer->attach_inventory_model(kind);else peer->remove_inventory_model(kind);
                state_.arriving(nullptr);words.set(0x14,std::uint8_t{0});
            }
            if(state_.model(0))attachments_.attach(back(*this)?10u:3u);
            return;
        case 4:remove_inventory_model(4);return;
        case 5:attachments_.attach(4);return;
        case 12:remove_inventory_model(8);return;
        case 13:attachments_.attach(8);return;
        case 14:remove_inventory_model(9);return;
        case 6:remove_inventory_model(5);return;
        case 7:attachments_.attach(5);return;
        case 8:remove_inventory_model(6);return;
        case 9:attachments_.attach(6);return;
        case 10:remove_inventory_model(7);return;
        case 11:attachments_.attach(7);return;
        case 16:services_.attach_equipment_reload_auxiliary(state_.second_volume());return;
        case 17:services_.attach_equipment_reload_auxiliary(state_.first_volume());return;
        case 18:services_.attach_equipment_reload_auxiliary(Frame{});return;
        default:throw std::runtime_error("Equipment event requires its actual action-model producer");
        }
    }
private:
    static bool back(Peer& peer){return (peer.transfer_state().words().template get<std::uint32_t>(0x50)&1u)!=0;}
    template<class Item>void select(Item& item,unsigned kind) {select(item,kind,static_cast<std::int32_t>(item.definition().identifier));}
    template<class Item>void select(Item& item,unsigned kind,std::int32_t definition) {state_.select(item.model(),kind,static_cast<std::uint32_t>(item.identity),-10000,definition);}
    template<class Item>void show(Item& item){holsters_.show_items(item.definition(),1,services_.equipment_opacity());}
    Frames& frames_;Items& items_;Services& services_;Holsters& holsters_;Attachments& attachments_;State& state_;
};
} //namespace hd2::game
#endif
