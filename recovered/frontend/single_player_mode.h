#ifndef HD2_FRONTEND_SINGLE_PLAYER_MODE_H
#define HD2_FRONTEND_SINGLE_PLAYER_MODE_H

#include "frontend/control_events.h"

namespace hd2::frontend {
struct SinglePlayerMode {
    std::uint32_t root = 0, mode = 1;
    bool standalone = false;
    std::uint32_t shared_mode = 0;
};

// 0063B590's shown observer and0063B740's mode/focus observer. The owning menu
//  sends the emitted33/74 events to its real preparation/mission-list service.
//  This helper neither creates a descriptor nor invents a successful receiver.
template <class Queue> void show_single_player_modes(Queue &queue) {
    queue.enqueue({0x0c700000, 0x01000002, 0, 0, 0, {}});
    queue.enqueue({0x0c800000, 0x01000002, 0, 0, 0, {}});
}

template <class Queue>
bool dispatch_single_player_mode(Queue &queue, SinglePlayerMode &mode, std::uint32_t identifier,
                                 std::uint32_t tag) {
    queue.enqueue({0x0c300000, 0x02000018, 0, 0, 0, {}});
    identifier &= 0xfffffff0u;
    if (tag == 0x04003000) {
        if (identifier == 0x0cc00000) {
            queue.enqueue({0x0c200000, 0x0200001a, 0, 0, 0, {}});
            return true;
        }
        if (identifier < 0x0c400000 || identifier > 0x0c800000 || (identifier & 0x000fffffu))
            return false;
        const auto selection = (identifier - 0x0c400000) >> 20;
        queue.enqueue({0x0c200000, 0x02000019, selection, 0, 0, {}});
        queue.enqueue({0x0c300000, 0x01000017, 2, 0x8a2, 0, {}});
        queue.enqueue({0x0c300000, 0x01000017, 2, 2002, 0, {}});
        queue.enqueue({0x0c300000, 0x01000017, 3, 0x8a3 + selection, 0, {}});
        return true;
    }
    if (tag != 0x04001000)
        return false;
    queue.enqueue({0x0c200000, 0x0200001a, 0, 0, 0, {}});
    mode.shared_mode = 0;
    switch (identifier) {
    case 0x0c400000:
        mode.root = 0;
        mode.standalone = false;
        mode.mode = 1;
        break;
    case 0x0c500000:
        mode.root = 0;
        mode.standalone = false;
        mode.mode = 2;
        break;
    case 0x0c600000:
        mode.root = 0;
        mode.standalone = false;
        mode.mode = 3;
        break;
    case 0x0c700000:
        mode.standalone = true;
        mode.mode = 0;
        break;
    case 0x0c800000:
        mode.standalone = true;
        mode.mode = 3;
        break;
    // Original hidden declarations; availability remains in the definition.
    case 0x0c900000:
        mode.root = 1;
        mode.standalone = false;
        mode.mode = 1;
        break;
    case 0x0ca00000:
        mode.root = 1;
        mode.standalone = false;
        mode.mode = 2;
        break;
    case 0x0cb00000:
        mode.root = 1;
        mode.standalone = false;
        mode.mode = 3;
        break;
    case 0x0cc00000:
        queue.enqueue({0, 0x02000003, 0, 0, 0, {}});
        return true;
    default:
        return false;
    }
    queue.enqueue({0, mode.standalone ? 0x06001074u : 0x06001033u, 0, 0, 0, {}});
    return true;
}
} // namespace hd2::frontend
#endif
