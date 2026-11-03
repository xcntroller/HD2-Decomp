#ifndef HD2_FRONTEND_LINE_SCAN_H
#define HD2_FRONTEND_LINE_SCAN_H
#include "frontend/original_abi.h"
#include <cstdint>
namespace hd2::frontend {
//006E5300: ECX=begin, EDX=end, DWORD byte count; NUL is not a terminator.
HD2_CLIENT_BODY inline std::uint32_t HD2_CLIENT_FASTCALL
scan_localized_line(const char* begin,const char* end) {
    auto* cursor=begin;
    while (reinterpret_cast<std::uintptr_t>(cursor)<reinterpret_cast<std::uintptr_t>(end) &&
            *cursor!='\n' && *cursor!='\r') ++cursor;
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(cursor)-
                                      reinterpret_cast<std::uintptr_t>(begin));
}
}
#endif
