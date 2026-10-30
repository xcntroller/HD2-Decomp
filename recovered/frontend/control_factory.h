#ifndef HD2_FRONTEND_CONTROL_FACTORY_H
#define HD2_FRONTEND_CONTROL_FACTORY_H

#include "frontend/control_initialization.h"

#include <cstdint>
#include <new>

namespace hd2::frontend {

// Original 0065d150 has every kind from 2 through 0x28. Native constructors
// remain supplied backends; knowing a kind does not reconstruct its widget.
inline bool original_control_kind(std::uint32_t kind) { return kind >= 2 && kind <= 0x28; }

template<class Factory>
bool construct_defined_control(Factory& factory) {
    if (!factory.find_frame(0x10021)) {
        factory.report_missing_frame();
        return false;
    }
    const auto kind = factory.control_kind();
    if (!original_control_kind(kind)) {
        factory.report_unknown_kind(kind);
        return false;
    }
    // Original allocation-failure paths can dereference null. Native storage
    // reports allocation failure explicitly rather than reproducing that fault.
    if (!factory.allocate_control(kind)) throw std::bad_alloc();
    return initialize_created_control(factory);
}

}

#endif
