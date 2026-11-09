#ifndef HD2_NETWORKING_DIRECTPLAY_SERVER_OWNER_H
#define HD2_NETWORKING_DIRECTPLAY_SERVER_OWNER_H

#include "networking/directplay_server.h"
#include "networking/server_player_slot.h"
#include <vector>

namespace hd2::networking {

// Cold ownership subset of00787EA0/00788210: two real critical sections,
// initial32 slot records and the real server transport. It does not initialize
// auxiliaries, serialization, workers, admission or the game's callback owner.
// Lifecycle stays on the constructing thread. Handler/context survive stop().
class DirectPlayServerOwner {
    class CriticalSection {
    public:
        CriticalSection();
        ~CriticalSection();
        void lock();
        void unlock() noexcept;
        CriticalSection(const CriticalSection&) = delete;
        CriticalSection& operator=(const CriticalSection&) = delete;
    private:
        CRITICAL_SECTION value_{};
    };
public:
    DirectPlayServerOwner();
    ~DirectPlayServerOwner();
    DirectPlayServerOwner(const DirectPlayServerOwner&) = delete;
    DirectPlayServerOwner& operator=(const DirectPlayServerOwner&) = delete;
    DirectPlayServerTransport::Submission start(const DirectPlayServerTransport::Request& request,
        DirectPlayServerTransport::Handler handler,void* context);
    DirectPlayServerTransport::Cleanup stop();
    std::vector<ServerPlayerSlot> players_snapshot() const;
    unsigned mode() const noexcept { return transport_.mode(); }
    bool has_endpoint() const noexcept { return transport_.has_endpoint(); }
private:
    // Declaration order keeps locks/records alive through endpoint destruction,
    // including callbacks delivered by Cancel/Close/Release.
    mutable CriticalSection players_lock_;
    CriticalSection queues_lock_;
    std::vector<ServerPlayerSlot> players_;
    DirectPlayServerTransport transport_;
};

} // namespace hd2::networking
#endif
