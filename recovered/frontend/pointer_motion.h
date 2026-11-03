#ifndef HD2_FRONTEND_POINTER_MOTION_H
#define HD2_FRONTEND_POINTER_MOTION_H

namespace hd2::frontend {

namespace detail {
// The original ordered FCOMP sets invalid for either kind of NaN and respects
// the caller's x87 control word. Keep its comparison/status sequence, including
// the balanced x87 stack, rather than substituting an SSE comparison.
inline bool pointer_delta_nonzero(const float& delta) {
#if (defined(__i386__) || defined(__x86_64__)) && (defined(__GNUC__) || defined(__clang__))
    static const float zero = 0;
    bool result;
    __asm__ volatile("flds %1\n\tfcomps %2\n\tfnstsw %%ax\n\t"
                     "testb $0x44, %%ah\n\tsetp %%al"
                     : "=a"(result) : "m"(delta), "m"(zero) : "cc", "st");
    return result;
#else
    return delta != 0.0f;
#endif
}
}

// Client 00669ff0.
inline bool pointer_moved(float horizontal, float vertical) {
    return detail::pointer_delta_nonzero(horizontal) || detail::pointer_delta_nonzero(vertical);
}

}

#endif
