#ifndef HD2_FRONTEND_POINTER_INHIBITION_H
#define HD2_FRONTEND_POINTER_INHIBITION_H
#include "frontend/original_abi.h"
#include <array>
#include <cstdint>
#include <cstddef>
#include <type_traits>
namespace hd2::frontend {
struct PointerInhibitionFields {
    std::array<std::uint8_t,0x48> prefix{};
    std::uint8_t raw=0;
};
static_assert(offsetof(PointerInhibitionFields,raw)==0x48);
class PointerInhibition {
public:
    //0066A860: store the byte verbatim, including noncanonical true values.
    HD2_CLIENT_BODY void HD2_CLIENT_THISCALL set(std::uint8_t value) { fields_.raw=value; }
    std::uint8_t raw() const { return fields_.raw; }
    operator bool() const { return raw()!=0; }
    PointerInhibition& operator=(bool value) { set(static_cast<std::uint8_t>(value)); return *this; }
private:
    PointerInhibitionFields fields_;
};
static_assert(std::is_standard_layout_v<PointerInhibition>);
}
#endif
