#ifndef HD2_GAME_HUMAN_CAMERA_AMOUNT_H
#define HD2_GAME_HUMAN_CAMERA_AMOUNT_H

#include <cstdint>
#include <cstring>

namespace hd2::game {

//00786650. DD0 supplies its real Human model/A30 and SAME equipment owner.
//GEffects virtual50 is00486D30 (model+54 complement), distinct from54 opacity.
template<class Effects,class Frame,class EquipmentState>
void apply_human_camera_amount(Effects& effects,Frame model,std::uint32_t equipment_mode,
                              EquipmentState& equipment,float amount) {
    (void)effects.model_camera_amount(model,amount);
    if(equipment_mode==1&&equipment.selected_model(9))
        (void)effects.model_camera_amount(equipment.selected_model(9),amount);
}

template<class Human>
float human_camera_amount(const Human& human) { //005A97A0.
    static_assert(sizeof(human.field94)==sizeof(float));
    float amount;std::memcpy(&amount,&human.field94,sizeof amount);return amount;
}

//005A9760. Native94 caches the amount before calling actual A0; equal values
//make no call, even after a failed receiver. A null inline/control models the
//genuine A0-null branch, not a successful replacement for a live control.
template<class Human,class Inline>
void set_human_camera_amount(Human& human,float amount,Inline* control) {
    if(human_camera_amount(human)==amount)return;
    std::memcpy(&human.field94,&amount,sizeof amount);
    if(control&&control->control())control->apply_camera_amount(amount);
}
} //namespace hd2::game
#endif
