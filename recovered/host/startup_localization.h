#ifndef HD2_HOST_STARTUP_LOCALIZATION_H
#define HD2_HOST_STARTUP_LOCALIZATION_H

#include "frontend/startup_language.h"
#include <windows.h>
#include <io.h>
#include <cstdio>
#include <cstring>

namespace hd2::host {

// Concrete operating-system/CRT boundary for recovered startup localization.
// Source-only until original application setup is integrated into the host.
class StartupLocalizationPlatform {
public:
    std::int32_t open_registry(std::uint32_t hive, const char* path,
                                std::uint32_t options, std::uint32_t access,
                                std::uintptr_t& output) {
        HKEY key = nullptr;
        const auto result = RegOpenKeyExA(reinterpret_cast<HKEY>(static_cast<std::uintptr_t>(hive)),
                                          path, options, access, &key);
        if (result == 0) output = reinterpret_cast<std::uintptr_t>(key);
        return result;
    }
    std::int32_t query_registry(std::uintptr_t key, const char* name,
                                 char* output, std::uint32_t& size) {
        DWORD bytes = size;
        const auto result = RegQueryValueExA(reinterpret_cast<HKEY>(key), name, nullptr,
            nullptr, reinterpret_cast<BYTE*>(output), &bytes);
        size = bytes;
        return result;
    }
    void close_registry(std::uintptr_t key) { RegCloseKey(reinterpret_cast<HKEY>(key)); }
    bool probe_text_file(const std::string& path) {
        auto* file = std::fopen(path.c_str(), "r");
        if (!file) return false;
        std::fclose(file);
        return true;
    }
    std::uint32_t module_filename(char* output, std::uint32_t capacity) {
        return GetModuleFileNameA(nullptr, output, capacity);
    }
    std::intptr_t find_first(const std::string& pattern, std::string& output) {
        _finddata_t data{};
        const auto handle = _findfirst(pattern.c_str(), &data);
        if (handle != -1) output = data.name;
        return handle;
    }
    int find_next(std::intptr_t handle, std::string& output) {
        _finddata_t data{};
        const auto result = _findnext(handle, &data);
        if (result != -1) output = data.name;
        return result;
    }
    void find_close(std::intptr_t handle) { _findclose(handle); }
};

// Original rw_data.dll file exports remain delegated, with their verified
// imported stdcall names. The caller supplies a loaded module and retains its
// lifetime/archive setup. This adapter does not create or configure archives.
class DtaTextFiles {
public:
    explicit DtaTextFiles(HMODULE module)
        : open_(resolve<Open>(module, "_dtaOpen@8")),
          seek_(resolve<Seek>(module, "_dtaSeek@12")),
          read_(resolve<Read>(module, "_dtaRead@12")),
          close_(resolve<Close>(module, "_dtaClose@4")) {}
    std::uint32_t open(const char* path, std::uint32_t flags) { return open_(path, flags); }
    std::uint32_t seek(std::uint32_t handle, std::int32_t offset, std::uint32_t origin) {
        return seek_(handle, offset, origin);
    }
    std::uint32_t read(std::uint32_t handle, void* output, std::uint32_t count) {
        return read_(handle, output, count);
    }
    void close(std::uint32_t handle) { close_(handle); }
    void text_diagnostic(const std::string& path, bool unicode) {
        // Diagnostic transport is native; original 006eced0's logger is separate.
        const auto message = "Trying add file <" + path + (unicode ?
            ">; Unicode format detected, not supporting now." : ">; FAILED");
        OutputDebugStringA(message.c_str());
    }
private:
    template<class Function> static Function resolve(HMODULE module, const char* name) {
        if (!module) throw std::invalid_argument("Text loading needs original rw_data module");
        const auto function = GetProcAddress(module, name);
        if (!function) throw std::runtime_error("Missing original rw_data text-file export");
        return reinterpret_cast<Function>(function);
    }
    using Open = std::uint32_t (WINAPI *)(const char*, std::uint32_t);
    using Seek = std::uint32_t (WINAPI *)(std::uint32_t, std::int32_t, std::uint32_t);
    using Read = std::uint32_t (WINAPI *)(std::uint32_t, void*, std::uint32_t);
    using Close = std::uint32_t (WINAPI *)(std::uint32_t);
    Open open_;
    Seek seek_;
    Read read_;
    Close close_;
};

}
#endif
