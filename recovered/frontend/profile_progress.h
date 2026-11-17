#ifndef HD2_FRONTEND_PROFILE_PROGRESS_H
#define HD2_FRONTEND_PROFILE_PROGRESS_H
#include <cstdint>
#include <vector>
namespace hd2::frontend {
//Application profile94/B8:006B5D20 initializes suppression0 and both arrays.
//Typed storage, never native pointer bytes in ProfilePayload numeric words.
struct ProfileProgress {
    bool append_suppressed=false;
    std::vector<std::uint32_t> stored,pending;
    void clear(){stored.clear();pending.clear();} //006A77E0 retains allocation AND suppression.
    void append(std::uint32_t value){if(!append_suppressed)pending.push_back(value);} //006A7820; preserve duplicates/order.
};
} //namespace hd2::frontend
#endif
