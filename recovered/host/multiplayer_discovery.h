#ifndef HD2_HOST_MULTIPLAYER_DISCOVERY_H
#define HD2_HOST_MULTIPLAYER_DISCOVERY_H

#include "frontend/multiplayer_discovery.h"
#include <windows.h>
#include <dplay8.h>
#include <atomic>
#include <mutex>
#include <map>

namespace hd2::host {

// Source-only DirectPlay boundary for00723A30/00723330/00723AB0 and the
// browser-response portion of00722180. It never hosts/connects/enters a game.
// Lifecycle/drain methods belong to the constructing thread; DirectPlay calls
// the response handler on its own threads. Close joins callbacks before release.
// No original client/DLL entry point is invoked.
class MultiplayerLanDiscovery {
public:
    struct Submission {
        HRESULT result;
        const char* operation;
        bool accepted; // Original submission predicate, NOT discovery success.
    };
    struct Snapshot {
        std::vector<frontend::MultiplayerServer> servers;
        struct Restored {
            frontend::MultiplayerServer requested, server;
        };
        std::vector<Restored> restored;
        struct Information {
            frontend::MultiplayerServer requested, server;
            frontend::MultiplayerInformation details;
        };
        std::vector<Information> information;
        std::size_t malformed = 0;
        HRESULT callback_error = S_OK;
    };

    MultiplayerLanDiscovery() = default;
    ~MultiplayerLanDiscovery();
    MultiplayerLanDiscovery(const MultiplayerLanDiscovery&) = delete;
    MultiplayerLanDiscovery& operator=(const MultiplayerLanDiscovery&) = delete;

    // Fresh broadcast: no target, query0, default31567. Targeted restore uses
    // target address/port and query2; detail refresh uses query0. Internet is a
    // separate service and is deliberately absent from this LAN-only interface.
    Submission start(const frontend::MultiplayerServer* target = nullptr,
                     std::uint32_t query = 0, bool utf8 = false);
    //00619B60/00723CC0 submit a stored list on one retained client. Appending
    // must not cancel earlier targets; each callback carries its query tag.
    Submission append(const frontend::MultiplayerServer& target, std::uint32_t query = 2);
    void stop() noexcept;
    Snapshot drain();

private:
    static HRESULT WINAPI callback(void* context, DWORD message, void* data) noexcept;
    void receive(const DPNMSG_ENUM_HOSTS_RESPONSE& response);
    Submission submit(const frontend::MultiplayerServer* target, std::uint32_t query);

    IDirectPlay8Client* client_ = nullptr;
    bool com_ = false, initialized_ = false, utf8_ = false;
    std::atomic<bool> receiving_{false};
    std::mutex mutex_;
    struct Request {
        std::uint32_t query;
        std::optional<frontend::MultiplayerServer> target;
    };
    std::map<std::uintptr_t,Request> requests_;
    std::uintptr_t next_request_ = 1;
    Snapshot pending_;
};

} // namespace hd2::host
#endif
