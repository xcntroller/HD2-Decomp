#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "host/startup_session.h"
#include "host/graphics_settings.h"
#include "host/engine_sound.h"
#include <objbase.h>
#include <io.h>

namespace {
template<class Function>
Function exported(HMODULE module, const char* name, std::uintptr_t rva) {
    const auto entry = GetProcAddress(module, name);
    if (reinterpret_cast<std::uintptr_t>(entry) != reinterpret_cast<std::uintptr_t>(module) + rva)
        throw std::runtime_error("Unsupported startup engine export");
    Function function;
    std::memcpy(&function, &entry, sizeof(entry));
    return function;
}
struct Module {
    HMODULE value;
    explicit Module(const wchar_t* path) : value(LoadLibraryExW(path, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH)) {
        if (!value) throw std::runtime_error("Cannot load startup dependency");
    }
    ~Module() { FreeLibrary(value); }
    Module(const Module&) = delete;
    Module& operator=(const Module&) = delete;
};
struct Graphics {
    using Close = void (WINAPI *)(hd2::host::EngineFrame*);
    hd2::host::EngineFrame* frame;
    HMODULE module;
    Close close_ = nullptr;
    bool initialized = false;
    explicit Graphics(HMODULE engine) : module(engine) {
        frame = exported<hd2::host::EngineFrame* (__cdecl *)()>(engine, "GetIGraph", 0x68f90)();
        close_ = hd2::host::scene_method<Close>(frame, module, 0x10, 0x6da40);
        using Initialize = std::int32_t (WINAPI *)(hd2::host::EngineFrame*, const hd2::host::GraphicsSettings32*);
        const auto settings = hd2::host::windowed_settings(
            static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr))));
        const auto status = hd2::host::scene_method<Initialize>(frame, module, 0, 0x6c2d0)(frame, &settings);
        if (status < 0) { close_(frame); throw std::runtime_error("Startup probe graphics initialization failed"); }
        using Initialized = std::uint8_t (WINAPI *)(hd2::host::EngineFrame*);
        if (!hd2::host::scene_method<Initialized>(frame, module, 0x0c, 0x6da20)(frame) || !IsWindow(window())) {
            close_(frame);
            throw std::runtime_error("Startup probe has no initialized graphics window");
        }
        initialized = true;
    }
    ~Graphics() { close(); }
    void close() {
        if (!initialized) return;
        close_(frame);
        initialized = false;
    }
    HWND window() const {
        using Window = HWND (WINAPI *)(hd2::host::EngineFrame*);
        return hd2::host::scene_method<Window>(frame, module, 0x48, 0x6a240)(frame);
    }
    void pump() const {
        using Pump = void (WINAPI *)(hd2::host::EngineFrame*);
        hd2::host::scene_method<Pump>(frame, module, 0x14, 0x6a470)(frame);
    }
};
}

int wmain(int argc, wchar_t** argv) {
    if (argc != 4 || std::wstring_view(argv[2]) != L"--frames") {
        std::fputs("Use tools/run_startup_probe.py to stage an isolated startup run.\n", stderr);
        return 2;
    }
    try {
        const auto frames = hd2::host::parse_frame_limit(argv[3]);
        // Executable-directory config/profile consumers must resolve inside the
        // isolated working directory. The launcher stages this executable there.
        wchar_t filename[32768];
        const auto length = GetModuleFileNameW(nullptr, filename, 32768);
        if (!length || length >= 32768) throw std::runtime_error("Startup executable path unavailable");
        const std::wstring executable(filename, length);
        const auto slash = executable.find_last_of(L"\\/");
        if (slash == std::wstring::npos || !SetCurrentDirectoryW(executable.substr(0, slash).c_str()))
            throw std::runtime_error("Startup working directory unavailable");
        if (GetFileAttributesW(L".hd2-startup-workspace") == INVALID_FILE_ATTRIBUTES)
            throw std::runtime_error("Startup probe requires an isolated staged workspace");
        if (std::freopen("host-runtime.log", "w", stdout) == nullptr) return 1;
        if (std::freopen("host-runtime.log", "a", stderr) == nullptr ||
            _dup2(_fileno(stdout), _fileno(stderr)) != 0) return 1;
        std::setvbuf(stdout, nullptr, _IONBF, 0);
        std::setvbuf(stderr, nullptr, _IONBF, 0);
        Module engine(argv[1]);
        const auto rw = GetModuleHandleW(L"rw_data.dll");
        if (!rw) throw std::runtime_error("LS3DF did not load its actual rw_data module");
        const std::wstring engine_path(argv[1]);
        Module joy((engine_path.substr(0, engine_path.find_last_of(L"\\/")) + L"\\IJoy.dll").c_str());
        const auto com = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (FAILED(com)) throw std::runtime_error("Startup COM initialization failed");
        struct Com { ~Com() { CoUninitialize(); } } com_owner;
        std::puts("Recovered startup integration probe; not the original controller/menu launch");
        hd2::host::StartupSession session(engine.value, rw, joy.value);
        session.before_devices();
        Graphics graphics(engine.value);
        int result = 1;
        try {
            hd2::host::EngineSound sound(engine.value, graphics.window());
            hd2::host::MenuDriver driver(engine.value);
            try {
                if (session.after_devices()) {
                    SetWindowTextW(graphics.window(), L"HD2 startup integration completed - close to finish");
                    std::puts("Startup integration completed; beginning bounded window pump");
                    for (std::uint32_t i = 0; (!frames || i < frames) && IsWindow(graphics.window()); ++i) {
                        graphics.pump();
                        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) break;
                        Sleep(10);
                    }
                    result = 0;
                } else std::fputs("Startup preparation failed; profile selection was not run\n", stderr);
            } catch (...) { session.close_consumers(); throw; }
            session.close_consumers(); // Portraits/input before3D/sound close.
        } catch (...) {
            graphics.close();
            session.close_archives();
            throw;
        }
        graphics.close();
        session.close_archives(); // Original archives close after device phases.
        std::printf("Startup probe teardown completed; result=%d\n", result);
        return result;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Startup probe failed: %s\n", error.what());
        return 1;
    }
}
