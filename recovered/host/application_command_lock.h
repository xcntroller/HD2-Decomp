#ifndef HD2_HOST_APPLICATION_COMMAND_LOCK_H
#define HD2_HOST_APPLICATION_COMMAND_LOCK_H
#include <windows.h>
namespace hd2::host {
//Native application+2C critical section. Distinct from control418/430 and
//peer-ownerB4. The containing process lifetime outlives command producers.
class ApplicationCommandLock {
public:
    ApplicationCommandLock(){InitializeCriticalSection(&lock_);}
    ~ApplicationCommandLock(){DeleteCriticalSection(&lock_);}
    ApplicationCommandLock(const ApplicationCommandLock&)=delete;
    ApplicationCommandLock& operator=(const ApplicationCommandLock&)=delete;
    void enter(){EnterCriticalSection(&lock_);}
    void leave()noexcept{LeaveCriticalSection(&lock_);}
private:
    CRITICAL_SECTION lock_{};
};
}
#endif
