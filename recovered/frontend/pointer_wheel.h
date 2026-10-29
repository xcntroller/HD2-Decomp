#ifndef HD2_FRONTEND_POINTER_WHEEL_H
#define HD2_FRONTEND_POINTER_WHEEL_H

#include <cmath>
#include <cstdint>
#include <cstring>

namespace hd2::frontend {

// Client 0066a030; 0066a020 reads the stored float delta directly.
inline bool pointer_wheel_moved(float delta) { return delta != 0.0f; }

// 0066b990 calls 007e5e00 (truncate to signed int64), then passes its low DWORD.
// Model the masked-invalid integer-indefinite result without an undefined C++
// float-to-integer cast. Original floating exception flags/traps are excluded.
inline std::int32_t pointer_wheel_argument(float delta) {
    const double truncated = std::trunc(static_cast<double>(delta));
    constexpr double limit = 0x1p63;
    std::uint32_t low = 0; // int64 integer-indefinite is 8000000000000000.
    if (std::isfinite(truncated) && truncated >= -limit && truncated < limit) {
        low = static_cast<std::uint32_t>(static_cast<std::uint64_t>(
            static_cast<std::int64_t>(truncated)));
    }
    std::int32_t result = 0;
    std::memcpy(&result, &low, sizeof(result));
    return result;
}

}

#endif
