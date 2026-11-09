#ifndef HD2_GAME_HUMAN_LOADING_INVENTORY_RECEIVER_H
#define HD2_GAME_HUMAN_LOADING_INVENTORY_RECEIVER_H
#include <cstdint>
namespace hd2::game {
template<class Backend>class SceneActor;
//Borrowed actual loading command owner on the SAME Human. Bind before B0
//initialization; retire B0 before removing this endpoint and its230 dependency.
template<class Backend>class HumanLoadingInventoryReceiver {
public:
    using Actor=SceneActor<Backend>;using Entry=typename Backend::Entry;
    virtual ~HumanLoadingInventoryReceiver()=default;
    virtual bool equip_shoulder()=0;
    virtual bool equip_holster(std::int32_t)=0;
    virtual bool weapon_usable(Entry,Actor*,bool)=0;
    virtual Entry preferred_weapon(Actor*,bool,std::int32_t&)=0;
    virtual void select_weapon(Entry,std::int32_t)=0;
    virtual bool special_hand_item(Entry)=0;
};
}
#endif
