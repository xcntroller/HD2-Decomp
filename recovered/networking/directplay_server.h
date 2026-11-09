#ifndef HD2_NETWORKING_DIRECTPLAY_SERVER_H
#define HD2_NETWORKING_DIRECTPLAY_SERVER_H

#include <windows.h>
#include <dplay8.h>
#include <atomic>
#include <string>

namespace hd2::networking {

// Native transport subset of00779450/007882B0. Lifecycle calls belong to the
// constructing thread. The required handler/context must outlive stop(), which
// joins callbacks before releasing their state. No default successful handler.
class DirectPlayServerTransport {
public:
    using Handler = HRESULT (WINAPI *)(void*,DWORD,void*);
    struct Request {
        // Caller owns actual configuration/name decoding. Original name helper
        // has a confirmed capacity discrepancy; no guessed transcoding here.
        std::wstring session_name;
        DWORD configured_port = 0;
        bool internet = false, dedicated = true;
    };
    struct Submission {
        HRESULT result;
        const char* operation;
        bool submitted; // Actual Host HRESULT accepted, not ready/game/session delivery.
    };
    struct Cleanup { HRESULT cancel = S_OK, close = S_OK; };

    DirectPlayServerTransport() = default;
    ~DirectPlayServerTransport();
    DirectPlayServerTransport(const DirectPlayServerTransport&) = delete;
    DirectPlayServerTransport& operator=(const DirectPlayServerTransport&) = delete;
    Submission start(const Request& request,Handler handler,void* context);
    Cleanup stop() noexcept;
    unsigned mode() const noexcept { return mode_.load(); } // Original7 idle,1 dedicated,3 listen.
    bool has_endpoint() const noexcept { return server_ != nullptr; }
private:
    static HRESULT WINAPI callback(void* owner,DWORD message,void* payload) noexcept;
    IDirectPlay8Server* server_ = nullptr;
    Handler handler_ = nullptr;
    void* context_ = nullptr;
    bool com_ = false, initialized_ = false;
    std::atomic<unsigned> mode_{7};
    std::wstring session_name_;
};

} // namespace hd2::networking
#endif
