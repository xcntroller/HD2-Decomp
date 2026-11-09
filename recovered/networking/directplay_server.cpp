#include "networking/directplay_server.h"
#include <memory>
#include <new>
#include <stdexcept>
#include <vector>

namespace hd2::networking {
namespace {
// Fingerprinted original00858F80/30/C0/B0/00858EC0/008560E8.
constexpr GUID server_class{0xda825e1b,0x6830,0x43d7,{0x83,0x5d,0x0b,0x5a,0xd8,0x29,0x56,0xa2}};
constexpr GUID server_interface{0x5102dace,0x241b,0x11d3,{0xae,0xa7,0,0x60,0x97,0xb0,0x14,0x11}};
constexpr GUID address_class{0x934a9523,0xa3ca,0x4bc5,{0xad,0xa0,0xd6,0xd9,0x5d,0x97,0x94,0x21}};
constexpr GUID address_interface{0x83783300,0x4063,0x4c8a,{0x9d,0xb3,0x82,0x83,0x0a,0x7f,0xeb,0x31}};
constexpr GUID tcpip{0xebfe7ba0,0x628d,0x11d2,{0xae,0x0f,0,0x60,0x97,0xb0,0x14,0x11}};
constexpr GUID application{0x2ae5b082,0xcfa0,0x11d4,{0x9e,0x1d,0,0x50,0xfc,0x21,0x3d,0xe6}};
struct ReleaseAddress { void operator()(IDirectPlay8Address* value) const noexcept { if (value) value->Release(); } };
}

DirectPlayServerTransport::~DirectPlayServerTransport() { stop(); }

DirectPlayServerTransport::Submission DirectPlayServerTransport::start(
        const Request& request,Handler handler,void* context) {
    stop();
    if (!handler || request.session_name.find(L'\0') != std::wstring::npos)
        return {E_INVALIDARG,"Server::handler/name",false};
    const auto fail = [&](HRESULT result,const char* operation) {
        stop(); return Submission{result,operation,false};
    };
    try { session_name_ = request.session_name; }
    catch (const std::bad_alloc&) { return fail(E_OUTOFMEMORY,"Server::name allocation"); }
    handler_ = handler; context_ = context;
    auto result = CoInitialize(nullptr);
    if (SUCCEEDED(result)) com_ = true;
    // Original bootstrap already initializes MTA; its later CoInitialize(NULL)
    // can return CHANGED_MODE. It acquires no new reference in that case.
    else if (result != RPC_E_CHANGED_MODE) return fail(result,"CoInitialize");
    result = CoCreateInstance(server_class,nullptr,CLSCTX_INPROC_SERVER,server_interface,
        reinterpret_cast<void**>(&server_));
    if (FAILED(result)) return fail(result,"Server::Create");
    if (!server_) return fail(E_UNEXPECTED,"Server::Create null interface");
    result = server_->Initialize(this,callback,0);
    if (FAILED(result)) return fail(result,"Server::Initialize");
    initialized_ = true;

    try {
        DWORD bytes = 0, count = 0;
        //00779619..64: size-query HRESULT is ignored; filled result is checked.
        (void)server_->EnumServiceProviders(&tcpip,nullptr,nullptr,&bytes,&count,0);
        if (bytes < sizeof(DPN_SERVICE_PROVIDER_INFO))
            return fail(E_UNEXPECTED,"Server::EnumServiceProviders size");
        // Value-initialized storage prevents reading original indeterminate bytes.
        std::vector<unsigned char> providers(bytes);
        const auto capacity = bytes;
        result = server_->EnumServiceProviders(&tcpip,nullptr,
            reinterpret_cast<DPN_SERVICE_PROVIDER_INFO*>(providers.data()),&bytes,&count,0);
        if (FAILED(result)) return fail(result,"Server::EnumServiceProviders");
        if (bytes > capacity || bytes < sizeof(DPN_SERVICE_PROVIDER_INFO) || !count ||
            count > bytes / sizeof(DPN_SERVICE_PROVIDER_INFO))
            return fail(E_UNEXPECTED,"Server::EnumServiceProviders bounds");
        // The first enum GUID is copied by original code but overwritten by
        // GetSP; SetSP uses requested TCP/IP, not an inferred replacement provider.
    } catch (const std::bad_alloc&) { return fail(E_OUTOFMEMORY,"Server::provider allocation"); }
      catch (const std::length_error&) { return fail(E_OUTOFMEMORY,"Server::provider length"); }

    IDirectPlay8Address* raw = nullptr;
    result = CoCreateInstance(address_class,nullptr,CLSCTX_ALL,address_interface,reinterpret_cast<void**>(&raw));
    std::unique_ptr<IDirectPlay8Address,ReleaseAddress> address(raw);
    if (FAILED(result) || !address) {
        address.reset(); // Temporary address retires before server/callback context.
        return fail(FAILED(result) ? result : E_UNEXPECTED,"Address::Create");
    }
    result = address->SetSP(&tcpip);
    if (FAILED(result)) { address.reset(); return fail(result,"Address::SetSP"); }
    GUID actual{};
    (void)address->GetSP(&actual);
    DWORD port = request.internet ? request.configured_port : 31567;
    if (port) (void)address->AddComponent(L"port",&port,sizeof(port),DPNA_DATATYPE_DWORD);

    DPN_APPLICATION_DESC description{};
    description.dwSize = sizeof(description);
    description.dwFlags = DPNSESSION_CLIENT_SERVER;
    description.guidApplication = application;
    description.pwszSessionName = session_name_.data();
    DPN_CAPS caps{};
    caps.dwSize = sizeof(caps); caps.dwConnectTimeout = 2000;
    caps.dwConnectRetries = 0xffffffffu; caps.dwTimeoutUntilKeepAlive = 4000;
    (void)server_->SetCaps(&caps,0);
    mode_.store(request.dedicated ? 1u : 3u);
    auto* device = address.get();
    result = server_->Host(&description,&device,1,nullptr,nullptr,nullptr,0);
    address.reset();
    if (FAILED(result)) return fail(result,"Server::Host");
    return {result,"Server::Host",true};
}

DirectPlayServerTransport::Cleanup DirectPlayServerTransport::stop() noexcept {
    Cleanup result;
    // Do not hold a lock: Close may join callbacks, which retain the handler,
    // original mode and owned name through the final Release.
    if (server_) {
        if (initialized_) {
            result.cancel = server_->CancelAsyncOperation(0,DPNCANCEL_ALL_OPERATIONS);
            result.close = server_->Close(0);
        }
        server_->Release(); server_ = nullptr;
    }
    initialized_ = false;
    mode_.store(7);
    handler_ = nullptr; context_ = nullptr; session_name_.clear();
    if (com_) { CoUninitialize(); com_ = false; }
    return result;
}

HRESULT WINAPI DirectPlayServerTransport::callback(void* owner,DWORD message,void* payload) noexcept {
    auto& self = *static_cast<DirectPlayServerTransport*>(owner);
    // All messages, including RETURN_BUFFER during Close, reach the required
    // actual handler. This adapter never fabricates callback/session success.
    try { return self.handler_(self.context_,message,payload); }
    catch (const std::bad_alloc&) { return E_OUTOFMEMORY; }
    catch (...) { return E_FAIL; }
}
} // namespace hd2::networking
