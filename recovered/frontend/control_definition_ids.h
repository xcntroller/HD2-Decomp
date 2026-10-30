#ifndef HD2_FRONTEND_CONTROL_DEFINITION_IDS_H
#define HD2_FRONTEND_CONTROL_DEFINITION_IDS_H

#include <cstdint>

namespace hd2::frontend {

// 00615360: one-based child ordinal in bits 20..25, OR parent unchanged.
// Preserve original wrap/OR behavior; this helper does not invent a limit.
inline std::uint32_t screen_child_identifier(std::uint32_t parent, std::uint32_t count) {
    return parent | (((count + 1u) & 0x3fu) << 20);
}

// 0061da10: nested children occupy bits 12..19, then bits 4..11 if the
// parent already has bits 12..19. IDs are not sequential integer increments.
inline std::uint32_t nested_child_identifier(std::uint32_t parent, std::uint32_t count) {
    return parent | (((count + 1u) & 0xffu) << ((parent & 0x000ff000u) ? 4 : 12));
}

}

#endif
