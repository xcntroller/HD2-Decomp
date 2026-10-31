#ifndef HD2_HOST_BORROWED_INPUT_H
#define HD2_HOST_BORROWED_INPUT_H

#include "host/menu_scene.h"

namespace hd2::host {

// Borrow devices already initialized by006DB620/006C0820. Poll exactly once in
// the outer frame. State scan/character queries share that snapshot and never
// initialize/close a second keyboard or maintain a divergent native key state.
class BorrowedGraphicsInput {
public:
    BorrowedGraphicsInput(EngineFrame* graphics, HMODULE module)
        : graphics_(graphics), module_(module) {}
    void poll_keyboard() const { scalar<std::int32_t>(0x9c, 0x6f190); }
    std::uint32_t next_scan() const { return scalar<std::uint32_t>(0xa0, 0x6f320); }
    std::uint32_t take_text_character() const { return scalar<std::uint32_t>(0xa8, 0x6f2d0); }
    bool shift_down() const { return scalar<std::uint8_t>(0xbc, 0x6f550) != 0; }
    bool control_down() const { return scalar<std::uint8_t>(0xc0, 0x6f570) != 0; }
    bool alt_down() const { return scalar<std::uint8_t>(0xc4, 0x6f590) != 0; }
    void set_pointer_position(std::int32_t x, std::int32_t y) const {
        using Function = void (WINAPI *)(EngineFrame*, std::int32_t, std::int32_t);
        scene_method<Function>(graphics_, module_, 0xf8, 0x6fa40)(graphics_, x, y);
    }
    std::int32_t pointer_x() const { return scalar<std::int32_t>(0xfc, 0x6fb10); }
    std::int32_t pointer_y() const { return scalar<std::int32_t>(0x100, 0x6fb20); }
    bool key_down(std::uint32_t scan) const {
        using Function = std::uint8_t (WINAPI *)(EngineFrame*, std::uint32_t);
        return scene_method<Function>(graphics_, module_, 0xb0, 0x6f490)(graphics_, scan) != 0;
    }
    void convert_ansi_z(const char* bytes, char16_t* output, int count) const {
        static_assert(sizeof(wchar_t) == sizeof(char16_t));
        MultiByteToWideChar(CP_ACP, 0, bytes, -1, reinterpret_cast<wchar_t*>(output), count);
    }
private:
    template<class Result> Result scalar(std::size_t slot, std::uintptr_t rva) const {
        using Function = Result (WINAPI *)(EngineFrame*);
        return scene_method<Function>(graphics_, module_, slot, rva)(graphics_);
    }
    EngineFrame* graphics_;
    HMODULE module_;
};

} // namespace hd2::host
#endif
