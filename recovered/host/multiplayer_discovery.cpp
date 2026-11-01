#include "host/multiplayer_discovery.h"
#include <cstdio>
#include <cwchar>
#include <memory>
#include <new>

namespace hd2::host {
namespace {
// Same values as original008190B8/0068/E8/D8/00818FE8. Local constants avoid
// requiring any DirectPlay runtime import library or global GUID definitions.
constexpr GUID client_class{0x743f1dc6,0x5aba,0x429f,{0x8b,0xdf,0xc5,0x4d,0x03,0x25,0x3d,0xc2}};
constexpr GUID client_interface{0x5102dacd,0x241b,0x11d3,{0xae,0xa7,0,0x60,0x97,0xb0,0x14,0x11}};
constexpr GUID address_class{0x934a9523,0xa3ca,0x4bc5,{0xad,0xa0,0xd6,0xd9,0x5d,0x97,0x94,0x21}};
constexpr GUID address_interface{0x83783300,0x4063,0x4c8a,{0x9d,0xb3,0x82,0x83,0x0a,0x7f,0xeb,0x31}};
constexpr GUID tcpip{0xebfe7ba0,0x628d,0x11d2,{0xae,0x0f,0,0x60,0x97,0xb0,0x14,0x11}};
constexpr GUID application{0x2ae5b082,0xcfa0,0x11d4,{0x9e,0x1d,0,0x50,0xfc,0x21,0x3d,0xe6}};

struct ReleaseAddress {
    void operator()(IDirectPlay8Address* address) const { if (address) address->Release(); }
};
using Address = std::unique_ptr<IDirectPlay8Address, ReleaseAddress>;

HRESULT create_address(Address& result) {
    IDirectPlay8Address* raw = nullptr;
    const auto created = CoCreateInstance(address_class, nullptr, CLSCTX_ALL,
        address_interface, reinterpret_cast<void**>(&raw));
    result.reset(raw);
    return FAILED(created) ? created : raw->SetSP(&tcpip);
}

// Original00724DC0 requests numeric sender hostname, then packs dotted bytes
// little-endian into the network-order IPv4 word. Reject malformed values.
std::uint32_t sender_ip(IDirectPlay8Address* address) {
    if (!address) throw std::runtime_error("Missing DirectPlay response address");
    std::array<wchar_t, 50> host{};
    DWORD length = static_cast<DWORD>(sizeof(host)), type = 0;
    if (FAILED(address->GetComponentByName(L"hostname", host.data(), &length, &type)) ||
            type != DPNA_DATATYPE_STRING || length > sizeof(host) || length < sizeof(wchar_t) ||
            length % sizeof(wchar_t) || host[length / sizeof(wchar_t) - 1] != 0)
        throw std::runtime_error("Invalid DirectPlay response hostname");
    std::uint32_t value = 0;
    std::size_t position = 0;
    for (unsigned part = 0; part < 4; ++part) {
        unsigned octet = 0, digits = 0;
        while (position < host.size() && host[position] >= L'0' && host[position] <= L'9') {
            if (++digits > 3) throw std::runtime_error("Invalid DirectPlay IPv4 octet");
            octet = octet * 10 + static_cast<unsigned>(host[position++] - L'0');
        }
        if (!digits || octet > 255 || position == host.size() || host[position] != (part == 3 ? 0 : L'.'))
            throw std::runtime_error("Invalid DirectPlay IPv4 address");
        ++position; value |= octet << (part * 8);
    }
    return value;
}

} // namespace

MultiplayerLanDiscovery::~MultiplayerLanDiscovery() { stop(); }

MultiplayerLanDiscovery::Submission MultiplayerLanDiscovery::start(
        const frontend::MultiplayerServer* target, std::uint32_t query, bool utf8) {
    stop();
    { std::lock_guard<std::mutex> lock(mutex_); pending_ = {}; }
    utf8_ = utf8;
    const auto fail = [&](HRESULT result, const char* operation) {
        stop(); return Submission{result, operation, false};
    };
    auto result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if (FAILED(result)) return fail(result, "CoInitializeEx");
    com_ = true;
    result = CoCreateInstance(client_class, nullptr, CLSCTX_INPROC_SERVER,
        client_interface, reinterpret_cast<void**>(&client_));
    if (FAILED(result)) return fail(result, "Client::Create");
    result = client_->Initialize(this, callback, 0);
    if (FAILED(result)) return fail(result, "Client::Initialize");
    initialized_ = true;
    receiving_.store(true);
    Submission submitted{E_OUTOFMEMORY,"Client::EnumHosts allocation",false};
    try { submitted=submit(target,query); }
    catch (const std::bad_alloc&) { stop(); return submitted; }
    if (!submitted.accepted) stop();
    return submitted;
}

MultiplayerLanDiscovery::Submission MultiplayerLanDiscovery::append(
        const frontend::MultiplayerServer& target,std::uint32_t query) {
    if (!client_ || !initialized_) return {E_UNEXPECTED,"Client::EnumHosts without owner",false};
    try { return submit(&target,query); }
    catch (const std::bad_alloc&) { return {E_OUTOFMEMORY,"Client::EnumHosts allocation",false}; }
}

MultiplayerLanDiscovery::Submission MultiplayerLanDiscovery::submit(
        const frontend::MultiplayerServer* target,std::uint32_t query) {
    if (query > 2 || (!target && query != 0)) return {E_INVALIDARG,"Client::EnumHosts query",false};
    Address host, device;
    const auto fail = [](HRESULT result,const char* operation) { return Submission{result,operation,false}; };
    auto result = create_address(host);
    if (FAILED(result)) return fail(result, "Host address::Create/SetSP");
    result = create_address(device);
    if (FAILED(result)) return fail(result, "Device address::Create/SetSP");
    if (target) {
        wchar_t ip[16]{};
        const auto value = target->ipv4;
        const auto count = std::swprintf(ip, 16, L"%u.%u.%u.%u",
            value & 255u, (value >> 8) & 255u, (value >> 16) & 255u, value >> 24);
        if (count < 0 || count >= 16) return fail(E_INVALIDARG, "Host address::Format");
        result = host->AddComponent(L"hostname", ip,
            static_cast<DWORD>((count + 1) * sizeof(wchar_t)), DPNA_DATATYPE_STRING);
        if (FAILED(result)) return fail(result, "Host address::Hostname");
    }
    DWORD port = target ? target->port : 31567;
    result = host->AddComponent(L"port", &port, sizeof(port), DPNA_DATATYPE_DWORD);
    if (FAILED(result)) return fail(result, "Host address::Port");
    DPN_APPLICATION_DESC description{};
    description.dwSize = sizeof(description); description.guidApplication = application;
    DPNHANDLE handle = 0;
    if (next_request_==std::numeric_limits<std::uintptr_t>::max())
        return fail(E_OUTOFMEMORY,"Client::EnumHosts context exhausted");
    const auto token=next_request_++;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        requests_.emplace(token,Request{query,target?std::optional<frontend::MultiplayerServer>(*target):std::nullopt});
    }
    result = client_->EnumHosts(&description, host.get(), device.get(), &query, sizeof(query),
        target ? 1 : 0xffffffffu, 0, target ? 1000 : 0xffffffffu,
        reinterpret_cast<void*>(token), &handle, 0);
    const auto accepted = result == S_OK || result == DPNERR_ADDRESSING ||
        result == DPNERR_INVALIDDEVICEADDRESS || result == DPNSUCCESS_PENDING;
    if (!accepted) return fail(result, "Client::EnumHosts");
    return {result, "Client::EnumHosts", true};
}

void MultiplayerLanDiscovery::stop() noexcept {
    receiving_.store(false);
    // Never hold mutex while Close waits for callbacks that may acquire it.
    if (client_) {
        if (initialized_) {
            client_->CancelAsyncOperation(0, DPNCANCEL_ALL_OPERATIONS);
            client_->Close(0);
        }
        client_->Release(); client_ = nullptr;
    }
    initialized_ = false;
    if (com_) { CoUninitialize(); com_ = false; }
    std::lock_guard<std::mutex> lock(mutex_);
    pending_ = {}; // Discard-before-restart prevents stale rows from old requests.
    requests_.clear(); // Close has joined callbacks before request identities die.
}

MultiplayerLanDiscovery::Snapshot MultiplayerLanDiscovery::drain() {
    std::lock_guard<std::mutex> lock(mutex_);
    Snapshot result; std::swap(result, pending_); return result;
}

HRESULT WINAPI MultiplayerLanDiscovery::callback(void* context, DWORD message, void* data) noexcept {
    auto& self = *static_cast<MultiplayerLanDiscovery*>(context);
    if (!self.receiving_.load() || message != DPN_MSGID_ENUM_HOSTS_RESPONSE) return S_OK;
    try {
        if (!data) throw std::runtime_error("Null DirectPlay enumeration response");
        const auto& response = *static_cast<const DPNMSG_ENUM_HOSTS_RESPONSE*>(data);
        if (response.dwSize < sizeof(response)) throw std::runtime_error("Short DirectPlay enumeration response");
        self.receive(response);
        return S_OK;
    } catch (const std::bad_alloc&) {
        std::lock_guard<std::mutex> lock(self.mutex_);
        self.pending_.callback_error = E_OUTOFMEMORY;
        return E_OUTOFMEMORY;
    } catch (...) {
        std::lock_guard<std::mutex> lock(self.mutex_);
        ++self.pending_.malformed;
        return S_OK; // Invalid peer input is discarded; no fabricated row.
    }
}

void MultiplayerLanDiscovery::receive(const DPNMSG_ENUM_HOSTS_RESPONSE& response) {
    if (!response.pApplicationDescription || !response.pApplicationDescription->pwszSessionName)
        throw std::runtime_error("Missing DirectPlay session description");
    std::array<char, 51> name{};
    // Original00724760/00724800 cap at49 output bytes plus NUL. Conversion
    // failure leaves the zero-initialized name instead of uninitialized memory.
    WideCharToMultiByte(utf8_ ? CP_UTF8 : CP_ACP, 0,
        response.pApplicationDescription->pwszSessionName, -1, name.data(), 49, nullptr, nullptr);
    name[49] = 0;
    Request request{};
    {
        std::lock_guard<std::mutex> lock(mutex_);
        const auto found=requests_.find(reinterpret_cast<std::uintptr_t>(response.pvUserContext));
        if (found==requests_.end()) throw std::runtime_error("Invalid DirectPlay enumeration context");
        request=found->second;
    }
    std::optional<frontend::MultiplayerInformation> information;
    std::uint32_t kind = 3;
    auto server = frontend::decode_multiplayer_discovery(
        static_cast<const unsigned char*>(response.pvResponseData), response.dwResponseDataSize,
        frontend::multiplayer_text(name), sender_ip(response.pAddressSender), response.dwRoundTripLatencyMS,
        &information, &kind);
    if (kind != request.query) throw std::runtime_error("Mismatched DirectPlay enumeration response");
    if (!server || !receiving_.load()) return;
    std::lock_guard<std::mutex> lock(mutex_);
    if (!receiving_.load()) return;
    if (information && request.target) pending_.information.push_back({*request.target,*server,std::move(*information)});
    else if (kind == 2 && request.target) pending_.restored.push_back({*request.target,*server});
    else pending_.servers.push_back(*server);
}

} // namespace hd2::host
