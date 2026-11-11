#ifndef HD2_GAME_HUMAN_EQUIPMENT_SERVICES_H
#define HD2_GAME_HUMAN_EQUIPMENT_SERVICES_H

#include "game/human_inventory_lookup.h"
#include "frontend/spatial_sound.h"
#include <optional>
#include <cstring>

namespace hd2::game {

template<class Frame>struct HumanEquipmentVisibilityReceiver {
    virtual ~HumanEquipmentVisibilityReceiver()=default;
    virtual void refresh(Frame)=0;
};

//Real DD0/E0/inventory consumers shared by230 activation/geometry/visibility.
//The containing World supplies actual player/view state, game clock and SOUND.
//Bind the actual230 refresh receiver before CD0 activation; no null success path.
template<class Human,class Control,class Look,class Items,class World>
class HumanEquipmentServices {
public:
    using Frame=typename Control::Frame;
    HumanEquipmentServices(Human& human,Control& control,Look& look,Items& items,World& world)
        :human_(human),control_(control),look_(look),items_(items),world_(world) {}
    void bind_visibility(HumanEquipmentVisibilityReceiver<Frame>& receiver) {
        if(visibility_)throw std::logic_error("Human230 visibility receiver is already bound");
        visibility_=&receiver;
    }
    void unbind_visibility(HumanEquipmentVisibilityReceiver<Frame>& receiver) noexcept {
        if(visibility_==&receiver)visibility_=nullptr;
    }
    bool visibility_bound()const noexcept{return visibility_!=nullptr;}
    std::uint32_t mode_flags()const{return world_.mode_flags();}
    float equipment_opacity()const{return control_.opacity().opacity();}
    int equipment_mode()const{return signed_word(control_.animation_state_word(0xa44));}
    int equipment_animation_identity()const{return signed_word(control_.animation_state_word(0x1d8));}
    bool equipment_visibility_override(){return world_.human_is_view_actor(human_)&&world_.human_view_offset_enabled();}
    auto equipment_item(std::int32_t identity){return find_human_inventory_item(human_,items_,identity);}
    std::optional<float> equipment_identity_scalar(std::int32_t identity) {
        if(auto* item=equipment_item(identity))return item->scalar();
        return std::nullopt;
    }
    void equipment_face_flag(bool enabled){control_.geometry().equipment_face_flag(enabled);}
    void equipment_primary_face(std::uint32_t command){control_.face().equipment_primary_command(command);}
    void equipment_secondary_face(std::uint32_t command){control_.face().equipment_secondary_command(command);}
    void set_equipment_weapon(Frame model,std::int32_t kind){control_.geometry().set_equipment_weapon(model,kind);}
    bool equipment_weapon_stowed()const{return control_.geometry().equipped_weapon().stowed();}
    void stow_equipment_weapon(bool enabled){control_.geometry().stow_equipment_weapon(enabled);}
    void pose_equipment_weapon(unsigned command){control_.geometry().equipment_weapon_pose(command);}
    std::int32_t equipment_look_command()const{return look_.state().template get<std::int32_t>(0x5c);}
    void blend_equipment_weapon(std::int32_t duration){control_.geometry().blend_equipment_weapon(duration);}
    void equipment_transfer_mode(std::uint32_t command,std::uint32_t mode,bool moved){control_.equipment_transfer_mode(command,mode,moved);}
    void force_equipment_command(std::int32_t command){look_.force_equipment_command(command);}
    void refresh_inventory_holster_animation() { //00791EB0:350.f/1.4f x87→truncate EAX=250.
        if(control_.animation_state_word(0x1e8)==0x20u)control_.tracks().fade_pair(1,250);
    }
    void refresh_equipment_visibility() {
        if(!visibility_)throw std::runtime_error("Human230 visibility receiver is not bound");
        visibility_->refresh(human_.base().frame());
    }
    void reset_equipment_command(){control_.reset_equipment_command();}
    void set_equipment_command(std::int32_t command,bool changed){look_.equipment_command(command,changed);}
    std::uint32_t game_word28()const{return world_.game_word28();}
    std::int32_t equipment_life_value()const{return signed_word(human_.properties.stats_ec[0]);} //Human+EC, signed compare.
    void equipment_sound(std::uint32_t id) {
        frontend::SpatialSoundRequest request;request.group=3;request.entry=signed_word(id);request.source=&human_;
        auto* storage=human_.base().storage();
        if(!storage||storage->single()||!storage->holder(3))throw std::runtime_error("Equipment sound has no actual hand holder");
        //005A2AD0 resolves holder3's item's lazy model, NOT the Human model.
        if(const auto entry=storage->holder(3)->entry())request.relative=items_.runtime_item(entry).model();
        (void)world_.emit_human_equipment_sound(request);
    }
private:
    static std::int32_t signed_word(std::uint32_t bits){std::int32_t value;std::memcpy(&value,&bits,4);return value;}
    Human& human_;Control& control_;Look& look_;Items& items_;World& world_;
    HumanEquipmentVisibilityReceiver<Frame>* visibility_=nullptr;
};
} //namespace hd2::game
#endif
