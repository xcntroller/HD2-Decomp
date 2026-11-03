#ifndef HD2_FRONTEND_CREDITS_SCROLL_STATE_H
#define HD2_FRONTEND_CREDITS_SCROLL_STATE_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#if defined(_M_IX86)
#define HD2_SCROLL_CALL __thiscall
#define HD2_SCROLL_BODY __declspec(noinline)
#elif defined(__i386__)
#define HD2_SCROLL_CALL __attribute__((thiscall))
#define HD2_SCROLL_BODY __attribute__((noinline))
#else
#define HD2_SCROLL_CALL
#define HD2_SCROLL_BODY
#endif

namespace hd2::frontend {
struct CreditsScrollFields {
    std::array<std::uint32_t, 30> prefix{};
    std::uint32_t scroll = 0;
};
static_assert(offsetof(CreditsScrollFields, scroll) == 0x78);

//0064B940 owns only this DWORD write and AL=true. The renderer subsequently
//reads its low WORD; neither the text owner nor renderer is credited here.
class CreditsScrollState {
public:
    HD2_SCROLL_BODY bool HD2_SCROLL_CALL store(std::uint32_t value) {
        fields_.scroll = value;
        return true;
    }
    std::uint32_t value() const { return fields_.scroll; }
private:
    CreditsScrollFields fields_;
};
static_assert(std::is_standard_layout_v<CreditsScrollState>);
static_assert(sizeof(CreditsScrollState) == 0x7c);
}

#undef HD2_SCROLL_CALL
#undef HD2_SCROLL_BODY
#endif
