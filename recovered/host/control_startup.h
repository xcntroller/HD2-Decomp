#ifndef HD2_HOST_CONTROL_STARTUP_H
#define HD2_HOST_CONTROL_STARTUP_H

#include "frontend/control_startup.h"
#include <windows.h>

namespace hd2::host {

// Native storage for the two actual Win32 locks. This is NOT the remaining
// original control owner/device/interface shutdown. It only owns these locks;
// callers must destroy dependent services before this storage. No DLL runs here.
class ControlStartupServices {
public:
    ControlStartupServices() = default;
    ~ControlStartupServices() {
        for (unsigned i = 0; i != 2; ++i)
            if (initialized_[i]) DeleteCriticalSection(&locks_[i]);
    }
    ControlStartupServices(const ControlStartupServices&) = delete;
    ControlStartupServices& operator=(const ControlStartupServices&) = delete;
    void initialize_control_critical_section(unsigned index) {
        if (index >= 2 || initialized_[index])
            throw std::logic_error("Invalid original control critical-section initialization");
        InitializeCriticalSection(&locks_[index]);
        initialized_[index] = true;
    }
private:
    static_assert(sizeof(void*) == 4 && sizeof(CRITICAL_SECTION) == 24,
                  "Original control lock ABI requires i386");
    CRITICAL_SECTION locks_[2]{};
    bool initialized_[2]{};
};

}
#endif
