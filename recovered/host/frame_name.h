#ifndef HD2_HOST_FRAME_NAME_H
#define HD2_HOST_FRAME_NAME_H

#include <cstddef>
#include <cstring>
#include <stdexcept>

namespace hd2::host {

inline const char* engine_frame_name(const void* frame) {
    if (frame == nullptr) {
        throw std::invalid_argument("Cannot read the name of a null engine frame");
    }
    constexpr std::size_t name_offset = 0xe8;
    const char* name = nullptr;
    std::memcpy(&name, static_cast<const char*>(frame) + name_offset, sizeof(name));
    return name;
}

}

#endif
