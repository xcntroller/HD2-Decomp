#ifndef HD2_FRONTEND_POINTER_BUTTONS_H
#define HD2_FRONTEND_POINTER_BUTTONS_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#if defined(_M_IX86)
#define HD2_BUTTON_STDCALL __stdcall
#define HD2_BUTTON_THISCALL __thiscall
#define HD2_BUTTON_BODY __declspec(noinline)
#elif defined(__i386__)
#define HD2_BUTTON_STDCALL __attribute__((stdcall))
#define HD2_BUTTON_THISCALL __attribute__((thiscall))
#define HD2_BUTTON_BODY __attribute__((noinline))
#else
#define HD2_BUTTON_STDCALL
#define HD2_BUTTON_THISCALL
#define HD2_BUTTON_BODY
#endif

namespace hd2::frontend {

//0066A050: stdcall(selector, state), AL result, callee removes eight bytes.
// Keep the five i386 bodies separate for static comparison and ABI inspection.
HD2_BUTTON_BODY inline bool HD2_BUTTON_STDCALL
pointer_buttons_selected(std::uint32_t selector, std::uint32_t buttons) {
    return selector == 0 ? buttons != 0 : (buttons & selector) != 0;
}

// Only the prefix read by these five bodies is owned here. This is not a
// reconstruction of the input constructor or the remaining input fields.
struct PointerButtonFields {
    std::array<std::uint32_t, 11> prefix{};
    std::uint32_t previous = 0; // Original input +2c.
    std::uint32_t current = 0;  // Original input +30.
};
static_assert(offsetof(PointerButtonFields, previous) == 0x2c);
static_assert(offsetof(PointerButtonFields, current) == 0x30);
static_assert(sizeof(PointerButtonFields) == 0x34);

class PointerButtons {
public:
    void update(std::uint32_t buttons) {
        fields_.previous = fields_.current;
        fields_.current = buttons;
    }

    void clear() { fields_.previous = fields_.current = 0; }
    std::uint32_t previous() const { return fields_.previous; }
    std::uint32_t current() const { return fields_.current; }

    //0066A080/0066A0A0: ECX=this, selector on stack, AL result, RET4.
    HD2_BUTTON_BODY bool HD2_BUTTON_THISCALL previously_selected(std::uint32_t selector) const {
        return pointer_buttons_selected(selector, fields_.previous);
    }
    HD2_BUTTON_BODY bool HD2_BUTTON_THISCALL currently_selected(std::uint32_t selector) const {
        return pointer_buttons_selected(selector, fields_.current);
    }

    // Client 0066a0c0 and 0066a0f0.
    HD2_BUTTON_BODY bool HD2_BUTTON_THISCALL pressed(std::uint32_t selector) const {
        return currently_selected(selector) && !previously_selected(selector);
    }
    HD2_BUTTON_BODY bool HD2_BUTTON_THISCALL released(std::uint32_t selector) const {
        return !currently_selected(selector) && previously_selected(selector);
    }

private:
    PointerButtonFields fields_;
};
static_assert(std::is_standard_layout_v<PointerButtons>);
static_assert(sizeof(PointerButtons) == sizeof(PointerButtonFields));

}

#undef HD2_BUTTON_STDCALL
#undef HD2_BUTTON_THISCALL
#undef HD2_BUTTON_BODY

#endif
