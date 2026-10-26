#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include "host/graphics_settings.h"
#include "host/menu_scene.h"

#include <cstdio>
#include <cstring>
#include <exception>

namespace {

struct Graphics;
using Method = void (WINAPI *)(Graphics*);
using Settings = hd2::host::GraphicsSettings32;

struct GraphicsVtable {
    std::int32_t (WINAPI *initialize)(Graphics*, const Settings*);
    const Settings* (WINAPI *settings)(Graphics*);
    std::int32_t (WINAPI *set_flags)(Graphics*, std::uint32_t);
    std::uint8_t (WINAPI *initialized)(Graphics*);
    Method close;
    Method pump;
    std::int32_t (WINAPI *width)(Graphics*);
    std::int32_t (WINAPI *height)(Graphics*);
    Method reserved_20[10];
    HWND (WINAPI *window)(Graphics*);
    Method reserved_4c[6];
    void (WINAPI *clear)(Graphics*, std::uint32_t, float, std::uint32_t);
    Method begin_scene;
    Method end_scene;
    Method present;
};

struct Graphics {
    const GraphicsVtable* methods;
};

static_assert(sizeof(void*) == 4, "The original engine requires an i386 executable");
static_assert(offsetof(GraphicsVtable, close) == 0x10);
static_assert(offsetof(GraphicsVtable, window) == 0x48);
static_assert(offsetof(GraphicsVtable, clear) == 0x64);
static_assert(offsetof(GraphicsVtable, present) == 0x70);

bool running = true;
WNDPROC previous_window_proc = nullptr;

void windows_error(const char* operation) {
    std::fprintf(stderr, "%s failed: Win32 error %lu\n", operation, GetLastError());
}

LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    if (message == WM_CLOSE) {
        std::puts("WM_CLOSE received");
        running = false;
        return 0;
    }
    return CallWindowProcW(previous_window_proc, window, message, wparam, lparam);
}

template<class Function>
bool expected_method(Function function, HMODULE module, std::uintptr_t rva) {
    return reinterpret_cast<std::uintptr_t>(function) ==
           reinterpret_cast<std::uintptr_t>(module) + rva;
}

bool verify_methods(Graphics* graphics, HMODULE module) {
    const auto* methods = graphics->methods;
    return expected_method(methods->initialize, module, 0x6c2d0) &&
           expected_method(methods->initialized, module, 0x6da20) &&
           expected_method(methods->close, module, 0x6da40) &&
           expected_method(methods->window, module, 0x6a240) &&
           expected_method(methods->clear, module, 0x6a6c0) &&
           expected_method(methods->begin_scene, module, 0x6b380) &&
           expected_method(methods->end_scene, module, 0x6b3b0) &&
           expected_method(methods->present, module, 0x6b3e0);
}

int finish(Graphics* graphics, int result) {
    graphics->methods->close(graphics);
    if (graphics->methods->initialized(graphics) != 0) {
        std::fputs("Engine cleanup did not clear the initialized flag\n", stderr);
        return 1;
    }
    std::puts("Graphics closed");
    return result;
}

int run_window(Graphics* graphics, HMODULE module, std::uint32_t frame_limit, bool menu_model) {
    const auto settings = hd2::host::windowed_settings(
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr))));
    const auto* methods = graphics->methods;
    const auto status = methods->initialize(graphics, &settings);
    if (status < 0) {
        std::fprintf(stderr, "IGraph initialization failed: 0x%08x\n",
                     static_cast<unsigned>(status));
        return finish(graphics, 1);
    }
    HWND window = methods->window(graphics);
    if (methods->initialized(graphics) == 0 || window == nullptr || !IsWindow(window)) {
        std::fputs("Engine initialization returned without a valid initialized window\n", stderr);
        return finish(graphics, 1);
    }
    if (!SetWindowTextW(window, menu_model ?
            L"HD2 reconstruction - original menu model (not interactive)" :
            L"HD2 reconstruction - engine bootstrap (not menu)")) {
        windows_error("SetWindowTextW");
        return finish(graphics, 1);
    }
    SetLastError(0);
    const auto previous = SetWindowLongPtrW(window, GWLP_WNDPROC,
                                            reinterpret_cast<LONG_PTR>(&window_proc));
    if (previous == 0) {
        windows_error("SetWindowLongPtrW");
        return finish(graphics, 1);
    }
    previous_window_proc = reinterpret_cast<WNDPROC>(previous);
    std::puts("Graphics initialized; original LS3DF.dll owns the window and D3D8 device");
    std::fflush(stdout);
    std::uint32_t frames = 0;
    int result = 0;
    try {
        std::unique_ptr<hd2::host::MenuScene> menu;
        if (menu_model) {
            menu = std::make_unique<hd2::host::MenuScene>(module);
        }
        while (running) {
            MSG message{};
            while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE)) {
                if (message.message == WM_QUIT ||
                    (message.message == WM_KEYDOWN && message.wParam == VK_ESCAPE)) {
                    running = false;
                    break;
                }
                TranslateMessage(&message);
                DispatchMessageW(&message);
            }
            if (!running) {
                break;
            }
            if (menu) {
                const auto render_status = menu->render();
                if (render_status < 0) {
                    std::fprintf(stderr, "Menu scene render failed: 0x%08x\n",
                                 static_cast<unsigned>(render_status));
                    result = 1;
                    break;
                }
            } else {
                methods->clear(graphics, 0xff203050, 1.0f, 3);
                methods->begin_scene(graphics);
                methods->end_scene(graphics);
            }
            methods->present(graphics);
            ++frames;
            if (frame_limit != 0 && frames == frame_limit) {
                if (!PostMessageW(window, WM_CLOSE, 0, 0)) {
                    windows_error("PostMessageW(WM_CLOSE)");
                    result = 1;
                    break;
                }
            }
            Sleep(16);
        }
    } catch (const std::runtime_error& error) {
        std::fprintf(stderr, "Menu preview failed: %s\n", error.what());
        result = 1;
    }
    SetLastError(0);
    if (SetWindowLongPtrW(window, GWLP_WNDPROC,
                         reinterpret_cast<LONG_PTR>(previous_window_proc)) == 0) {
        windows_error("Restore window procedure");
        result = 1;
    }
    std::printf("Submitted %u frames\n", static_cast<unsigned>(frames));
    return finish(graphics, result);
}

}

int wmain(int argc, wchar_t** argv) {
    if (argc < 2) {
        std::fputs("Usage: engine_window.exe ABSOLUTE_ENGINE_DLL [--frames COUNT] [--menu-model]\n"
                   "Use the fingerprint-checking Python launcher. Default: 180 frames; "
                   "0 waits for Escape/Close.\n", stderr);
        return 2;
    }
    std::uint32_t frame_limit = 180;
    bool menu_model = false;
    bool frames_seen = false;
    try {
        for (int i = 2; i < argc; ++i) {
            if (std::wstring_view(argv[i]) == L"--menu-model" && !menu_model) {
                menu_model = true;
            } else if (std::wstring_view(argv[i]) == L"--frames" &&
                       !frames_seen && i + 1 < argc) {
                frame_limit = hd2::host::parse_frame_limit(argv[++i]);
                frames_seen = true;
            } else {
                throw std::invalid_argument("Unknown, duplicate, or incomplete host option");
            }
        }
    } catch (const std::invalid_argument& error) {
        std::fprintf(stderr, "%s\n", error.what());
        return 2;
    }
    std::puts("Engine-window host started (i386)");
    std::fflush(stdout);
    HMODULE module = LoadLibraryExW(argv[1], nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (module == nullptr) {
        windows_error("LoadLibraryExW(LS3DF.dll)");
        return 1;
    }
    FARPROC entry = GetProcAddress(module, "GetIGraph");
    int result = 1;
    if (entry == nullptr || !expected_method(entry, module, 0x68f90)) {
        std::fputs("Missing or unsupported GetIGraph export\n", stderr);
    } else {
        using GetGraphics = Graphics* (__cdecl *)();
        GetGraphics get_graphics = nullptr;
        static_assert(sizeof(get_graphics) == sizeof(entry));
        std::memcpy(&get_graphics, &entry, sizeof(entry));
        Graphics* graphics = get_graphics();
        if (graphics == nullptr || !verify_methods(graphics, module)) {
            std::fputs("Missing graphics singleton or unsupported graphics vtable\n", stderr);
        } else {
            result = run_window(graphics, module, frame_limit, menu_model);
        }
    }
    if (!FreeLibrary(module)) {
        windows_error("FreeLibrary(LS3DF.dll)");
        result = 1;
    }
    return result;
}
