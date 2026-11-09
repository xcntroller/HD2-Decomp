#ifndef HD2_GAME_HUMAN_LOADING_INVENTORY_RECEIVER_H
#define HD2_GAME_HUMAN_LOADING_INVENTORY_RECEIVER_H
#include <cstdint>
namespace hd2::game {
//Borrowed actual loading command owner on the SAME Human. Bind before B0
//initialization; retire B0 before removing this endpoint and its230 dependency.
class HumanLoadingInventoryReceiver {
public:
    virtual ~HumanLoadingInventoryReceiver()=default;
    virtual bool equip_shoulder()=0;
    virtual bool equip_holster(std::int32_t)=0;
};
}
#endif
