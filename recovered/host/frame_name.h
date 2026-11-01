#ifndef HD2_HOST_FRAME_NAME_H
#define HD2_HOST_FRAME_NAME_H

#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <string_view>

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

// 1001DF00 defaults to case-insensitive comparison via 10098740.
inline bool menu_frame_name_matches(std::string_view frame, std::string_view pattern) {
    const bool prefix = !pattern.empty() && pattern.back() == '*';
    if (prefix) pattern.remove_suffix(1);
    if (frame.size() < pattern.size() || (!prefix && frame.size() != pattern.size()))
        return false;
    const auto fold = [](unsigned char value) {
        return value >= 'A' && value <= 'Z' ? value + ('a' - 'A') : value;
    };
    for (std::size_t i = 0; i < pattern.size(); ++i)
        if (fold(static_cast<unsigned char>(frame[i])) !=
            fold(static_cast<unsigned char>(pattern[i])))
            return false;
    return true;
}

}

#endif
