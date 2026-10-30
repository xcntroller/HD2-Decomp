#ifndef HD2_FRONTEND_WINDOW_POSITION_H
#define HD2_FRONTEND_WINDOW_POSITION_H

#include <array>
#include <cstdint>

namespace hd2::frontend {

// Initial bytes model the original stack contents when an ignored short read or
// a failed GetWindowRect leaves output untouched. Files use two little-endian
// four-byte coordinates on the original i386 client.
using WindowCoordinate = std::array<std::uint8_t, 4>;
struct WindowPosition { WindowCoordinate left, top; };

inline std::int32_t window_coordinate(const WindowCoordinate& bytes) {
    const std::uint32_t value = std::uint32_t(bytes[0]) | (std::uint32_t(bytes[1]) << 8) |
        (std::uint32_t(bytes[2]) << 16) | (std::uint32_t(bytes[3]) << 24);
    // Avoid implementation-defined unsigned-to-signed conversion.
    return value <= 0x7fffffffU ? static_cast<std::int32_t>(value) :
        -1 - static_cast<std::int32_t>(~value);
}

// Files expose open/read/close; platform performs fresh graphics/window lookup
// for each operation. Return values after open are deliberately ignored.
template<class Files, class Platform>
bool restore_window_position(Files& files, Platform& platform, WindowPosition initial) {
    const auto handle = files.open("window.cfg", 0);
    if (handle & 0x80000000U) return false;
    files.read(handle, initial.left.data(), 4);
    files.read(handle, initial.top.data(), 4);
    files.close(handle);
    platform.move_window(window_coordinate(initial.left), window_coordinate(initial.top));
    platform.focus_window();
    return true;
}

template<class Files, class Platform>
bool save_window_position(Files& files, Platform& platform, WindowPosition initial) {
    const auto handle = files.open_write("window.cfg", 0x10000);
    if (handle & 0x80000000U) return false;
    platform.window_position(initial);
    files.write(handle, initial.left.data(), 4);
    files.write(handle, initial.top.data(), 4);
    files.close(handle);
    return true;
}

}
#endif
