#ifndef HD2_HOST_WINDOW_POSITION_H
#define HD2_HOST_WINDOW_POSITION_H

#include "frontend/window_position.h"
#include "host/startup_localization.h"
#include "host/menu_scene.h"

namespace hd2::host {

// Shares the same borrowed rw_data module and archive context as text loading.
class WindowPositionFiles {
public:
    explicit WindowPositionFiles(HMODULE module) : reads_(module),
        open_write_(resolve<OpenWrite>(module, "_dtaOpenWrite@8")),
        write_(resolve<Write>(module, "_dtaWrite@12")) {}
    std::uint32_t open(const char* name, std::uint32_t flags) { return reads_.open(name, flags); }
    std::uint32_t read(std::uint32_t h, void* p, std::uint32_t n) { return reads_.read(h, p, n); }
    void close(std::uint32_t h) { reads_.close(h); }
    std::uint32_t open_write(const char* name, std::uint32_t flags) { return open_write_(name, flags); }
    std::uint32_t write(std::uint32_t h, const void* p, std::uint32_t n) { return write_(h, p, n); }
private:
    template<class T> static T resolve(HMODULE module, const char* name) {
        const auto p = module ? GetProcAddress(module, name) : nullptr;
        if (!p) throw std::runtime_error("Missing original rw_data window-position export");
        return reinterpret_cast<T>(p);
    }
    using OpenWrite = std::uint32_t (WINAPI *)(const char*, std::uint32_t);
    using Write = std::uint32_t (WINAPI *)(std::uint32_t, const void*, std::uint32_t);
    DtaTextFiles reads_;
    OpenWrite open_write_;
    Write write_;
};

class WindowPositionPlatform {
public:
    explicit WindowPositionPlatform(HMODULE engine) : engine_(engine) {
        static_assert(sizeof(void*) == 4, "Original graphics ABI requires i386");
        const auto p = engine ? GetProcAddress(engine, "GetIGraph") : nullptr;
        if (!p || reinterpret_cast<std::uintptr_t>(p) != reinterpret_cast<std::uintptr_t>(engine) + 0x68f90)
            throw std::runtime_error("Unsupported window GetIGraph export");
        std::memcpy(&get_graphics_, &p, sizeof(p));
    }
    void move_window(std::int32_t left, std::int32_t top) const {
        SetWindowPos(window(), nullptr, left, top, 0, 0, 0x15);
    }
    void focus_window() const { SetFocus(window()); }
    void window_position(frontend::WindowPosition& position) const {
        RECT rect{};
        // Preserve the caller's initial bytes if the Win32 call fails.
        if (GetWindowRect(window(), &rect)) {
            encode(position.left, static_cast<std::uint32_t>(rect.left));
            encode(position.top, static_cast<std::uint32_t>(rect.top));
        }
    }
private:
    static void encode(frontend::WindowCoordinate& out, std::uint32_t value) {
        for (unsigned i = 0; i != 4; ++i) out[i] = static_cast<std::uint8_t>(value >> (8 * i));
    }
    HWND window() const {
        using Window = HWND (WINAPI *)(EngineFrame*);
        auto* graphics = get_graphics_();
        return scene_method<Window>(graphics, engine_, 0x48, 0x6a240)(graphics);
    }
    using GetGraphics = EngineFrame* (__cdecl *)();
    HMODULE engine_;
    GetGraphics get_graphics_ = nullptr;
};
}
#endif
