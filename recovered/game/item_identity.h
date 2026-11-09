#ifndef HD2_GAME_ITEM_IDENTITY_H
#define HD2_GAME_ITEM_IDENTITY_H

#include <cstdint>
#include <cstring>
#include <deque>

namespace hd2::game {

struct PendingItemIdentity {std::uint32_t timestamp;std::int32_t identity;};

//007DD5D0. The actual Items owner supplies its SAME current7E0 and pending80C
//deque; this body invents no producer, retirement policy or independent counter.
//Clock::microseconds is the actual low32 performance-counter conversion. The
//host's existing FrameClockPlatform supplies it without another timer owner.
template<class Clock>
std::int32_t allocate_item_identity(std::int32_t& current,
                                   std::deque<PendingItemIdentity>& pending,Clock& clock) {
    if(!pending.empty()) {
        const auto now=clock.microseconds();
        //007DD64D..52: CMP2710/JBE is UNSIGNED after wrapped32-bit subtraction.
        if(static_cast<std::uint32_t>(now-pending.front().timestamp)>10000u) {
            const auto identity=pending.front().identity;pending.pop_front();return identity;
        }
    }
    const auto next=static_cast<std::uint32_t>(current)+1u;
    std::memcpy(&current,&next,sizeof(current));return current; //Original INC wraps even at INT_MAX.
}

} //namespace hd2::game
#endif
