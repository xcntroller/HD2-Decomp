#ifndef HD2_GAME_HUMAN_FRAME_INPUTS_H
#define HD2_GAME_HUMAN_FRAME_INPUTS_H
#include "game/human_joint_owners.h"

namespace hd2::game {
using HumanFrameInputBuffer=HumanJointWords<0x100>;
//Process BSS008AAF48/008AAE48, shared sequentially by AI frame callbacks.
//These are not network input7C or per-Human replacement buffers.
struct HumanAIFrameInputs {
    HumanFrameInputBuffer first,second;
    void clear(){first.bytes.fill(0);second.bytes.fill(0);}
};
} //namespace hd2::game
#endif
