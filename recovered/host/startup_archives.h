#ifndef HD2_HOST_STARTUP_ARCHIVES_H
#define HD2_HOST_STARTUP_ARCHIVES_H

#include "io/startup_archives.h"
#include <windows.h>
#include <cstring>

namespace hd2::host {

// Borrowed original rw_data module, shared with DtaTextFiles and engine imports.
// Archive indexes, file precedence, codecs and original global registry remain
// delegated DLL behavior. Compiled/linked only until full startup is integrated.
class StartupArchiveBackend {
public:
    explicit StartupArchiveBackend(HMODULE module) : module_(module) {
        static_assert(sizeof(void*) == 4, "Original archive ABI requires i386");
        const auto entry = module ? GetProcAddress(module, "_dtaCreate@4") : nullptr;
        if (!entry || reinterpret_cast<std::uintptr_t>(entry) !=
                reinterpret_cast<std::uintptr_t>(module) + 0x1424)
            throw std::runtime_error("Unsupported dtaCreate export");
        static_assert(sizeof(factory_) == sizeof(entry));
        std::memcpy(&factory_, &entry, sizeof(entry));
    }
    void* create_archive(const char* name) const { return factory_(name); }
    void set_archive_key(void* archive, std::uint64_t key) const {
        using SetKey = void (__thiscall *)(void*, std::uint32_t, std::uint32_t);
        method<SetKey>(archive, 0x0c, 0x1073)(archive, static_cast<std::uint32_t>(key),
                                             static_cast<std::uint32_t>(key >> 32));
    }
    std::uint64_t archive_key_reply(void* archive) const {
        using Reply = std::uint64_t (__thiscall *)(void*);
        return method<Reply>(archive, 0x10, 0x1384)(archive);
    }
    void release_archive(void* archive) const {
        using Release = void (__thiscall *)(void*);
        method<Release>(archive, 0x08, 0x1181)(archive);
    }
    void missing_archive(const std::string& name) const {
        // Original uses dbgPrintf; this is an explicit native diagnostic sink.
        const auto message = "# ERROR DATAFILE : datafile '" + name + "' missing";
        OutputDebugStringA(message.c_str());
    }

private:
    template<class Function>
    Function method(void* object, std::size_t slot, std::uintptr_t rva) const {
        if (!object) throw std::runtime_error("Null startup archive");
        const auto* methods = *static_cast<const std::uintptr_t* const*>(object);
        if (!methods || methods[slot / 4] != reinterpret_cast<std::uintptr_t>(module_) + rva)
            throw std::runtime_error("Unsupported archive object method");
        return reinterpret_cast<Function>(methods[slot / 4]);
    }
    using Factory = void* (WINAPI *)(const char*);
    HMODULE module_;
    Factory factory_ = nullptr;
};

}
#endif
