#include "networking/directplay_server_owner.h"
#include <mutex>

namespace hd2::networking {

DirectPlayServerOwner::CriticalSection::CriticalSection() { InitializeCriticalSection(&value_); }
DirectPlayServerOwner::CriticalSection::~CriticalSection() { DeleteCriticalSection(&value_); }
void DirectPlayServerOwner::CriticalSection::lock() { EnterCriticalSection(&value_); }
void DirectPlayServerOwner::CriticalSection::unlock() noexcept { LeaveCriticalSection(&value_); }

DirectPlayServerOwner::DirectPlayServerOwner() {
    players_.reserve(32);
    for (unsigned index = 0; index < 32; ++index) players_.emplace_back();
}
DirectPlayServerOwner::~DirectPlayServerOwner() { stop(); }

DirectPlayServerTransport::Submission DirectPlayServerOwner::start(
        const DirectPlayServerTransport::Request& request,
        DirectPlayServerTransport::Handler handler,void* context) {
    //00779450 calls007882B0 before new Host: the initial32 slots become empty,
    // retaining capacity. A live old endpoint retires before its records clear.
    stop();
    return transport_.start(request,handler,context);
}
DirectPlayServerTransport::Cleanup DirectPlayServerOwner::stop() {
    const auto result = transport_.stop();
    std::lock_guard<CriticalSection> lock(players_lock_);
    players_.clear();
    return result;
}
std::vector<ServerPlayerSlot> DirectPlayServerOwner::players_snapshot() const {
    std::lock_guard<CriticalSection> lock(players_lock_);
    return players_;
}

} // namespace hd2::networking
