#ifndef HD2_GAME_HUMAN_MOTION_STATE_H
#define HD2_GAME_HUMAN_MOTION_STATE_H

#include <cstdint>

namespace hd2::game {

//DD0+CB0's scalar tail:007506E0, thunk00750710 and empty close00750720.
//Its four borrowed control/Human links are supplied by the actual DD0 binding;
//these three fields do not substitute for those owners or motion producers.
struct HumanMotionState {
    template<class Random>void initialize(Random& random) {
        field10=0;field14=-10000;field18=static_cast<std::int32_t>(random()%8000)+5000;
    }
    std::int32_t field10=0,field14=0,field18=0;
};
} //namespace hd2::game
#endif
