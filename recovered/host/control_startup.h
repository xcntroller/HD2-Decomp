#ifndef HD2_HOST_CONTROL_STARTUP_H
#define HD2_HOST_CONTROL_STARTUP_H

#include "frontend/control_startup.h"
#include <windows.h>

namespace hd2::host {

// Native storage for two startup locks and the peer-owner Win32 lock.
// The remaining original control owner/device/interface shutdown is separate;
// callers must destroy dependent services before this storage. No DLL runs here.
class ControlStartupServices {
public:
    // Peer-owner+ B4 lock is constructed with the cold owner007466E0, before
    // the two startup locks. Native storage does not reproduce its object ABI.
    ControlStartupServices() {
        InitializeCriticalSection(&locks_[2]);
        initialized_[2] = true;
    }
    ~ControlStartupServices() {
        for (unsigned i = 0; i != 3; ++i)
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
    void enter_control_lock(unsigned index) {
        if (index >= 3 || !initialized_[index]) throw std::logic_error("Control lock is not initialized");
        EnterCriticalSection(&locks_[index]);
    }
    void leave_control_lock(unsigned index) noexcept { LeaveCriticalSection(&locks_[index]); }
private:
    static_assert(sizeof(void*) == 4 && sizeof(CRITICAL_SECTION) == 24,
                  "Original control lock ABI requires i386");
    CRITICAL_SECTION locks_[3]{};
    bool initialized_[3]{};
};

}
#endif
