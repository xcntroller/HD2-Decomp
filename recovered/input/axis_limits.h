#ifndef HD2_INPUT_AXIS_LIMITS_H
#define HD2_INPUT_AXIS_LIMITS_H
#include "frontend/original_abi.h"
#include <array>
#include <cstddef>
#include <cstdint>
namespace hd2::input {
struct BindingAxisFields {
    std::array<std::uint8_t,0xa8> prefix{};
    std::array<float,3> values{};
};
static_assert(offsetof(BindingAxisFields,values)==0xa8);
class BindingAxisLimits {
public:
    //006C6C90: signed integer load and float store obey the caller's x87 CW.
    HD2_CLIENT_BODY bool HD2_CLIENT_THISCALL set(std::int32_t x,std::int32_t y,std::int32_t z) {
#if (defined(__i386__) || defined(__x86_64__)) && (defined(__GNUC__) || defined(__clang__))
        __asm__ volatile("fildl %3\n\tfstps %0\n\tfildl %4\n\tfstps %1\n\tfildl %5\n\tfstps %2"
            : "=m"(fields_.values[0]),"=m"(fields_.values[1]),"=m"(fields_.values[2])
            : "m"(x),"m"(y),"m"(z) : "st");
#else
        fields_.values={{static_cast<float>(x),static_cast<float>(y),static_cast<float>(z)}};
#endif
        return true;
    }
    float& operator[](std::size_t i) { return fields_.values[i]; }
    const float& operator[](std::size_t i) const { return fields_.values[i]; }
private:
    BindingAxisFields fields_;
};
}
#endif
