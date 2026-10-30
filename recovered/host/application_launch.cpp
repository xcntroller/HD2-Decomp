#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "frontend/application_controller.h"
#include "frontend/application_states.h"
#include "frontend/application_frame.h"
#include "frontend/frame_clock.h"
#include "host/application_menu.h"
#include "host/engine_video.h"
#include "host/engine_sound.h"
#include "host/frame_clock.h"
#include <objbase.h>
#include <io.h>

namespace {
using namespace hd2;

struct Module {
    HMODULE value;
    explicit Module(const wchar_t* path) : value(LoadLibraryExW(path, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH)) {
        if (!value) throw std::runtime_error("Cannot load application launch dependency");
    }
    ~Module() { FreeLibrary(value); }
    Module(const Module&) = delete;
    Module& operator=(const Module&) = delete;
};

class WindowGraphics {
public:
    explicit WindowGraphics(HMODULE module) : module_(module) {
        const auto entry = GetProcAddress(module, "GetIGraph");
        if (reinterpret_cast<std::uintptr_t>(entry) != reinterpret_cast<std::uintptr_t>(module) + 0x68f90)
            throw std::runtime_error("Unsupported launch GetIGraph export");
        using Getter = host::Graphics* (__cdecl *)();
        Getter getter; std::memcpy(&getter, &entry, sizeof(entry));
        graphics_ = getter();
        verify();
        const auto settings = host::windowed_settings(
            static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(GetModuleHandleW(nullptr))));
        const auto result = graphics_->methods->initialize(graphics_, &settings);
        if (result < 0 || !graphics_->methods->initialized(graphics_) || !IsWindow(window())) {
            graphics_->methods->close(graphics_);
            throw std::runtime_error("Controller graphics initialization failed");
        }
        live_ = true;
    }
    ~WindowGraphics() { close(); }
    void close() {
        if (!live_) return;
        graphics_->methods->close(graphics_); live_ = false;
        std::puts("Controller graphics closed");
    }
    host::Graphics* graphics() const { return graphics_; }
    HWND window() const { return graphics_->methods->window(graphics_); }
private:
    void verify() {
        auto* frame = reinterpret_cast<host::EngineFrame*>(graphics_);
        using Unary = void (WINAPI *)(host::EngineFrame*);
        // Verify the entire interface used by the shared menu/text/cursor code.
        for (const auto& entry : std::initializer_list<std::pair<std::size_t, std::uintptr_t>>{
                {0,0x6c2d0}, {8,0x6c2c0}, {0x0c,0x6da20}, {0x10,0x6da40}, {0x14,0x6a470},
                {0x18,0x6de50}, {0x1c,0x6de70}, {0x3c,0x69f70}, {0x44,0x6a1f0},
                {0x48,0x6a240}, {0x4c,0x6a250}, {0x64,0x6a6c0}, {0x68,0x6b380},
                {0x6c,0x6b3b0}, {0x70,0x6b3e0}, {0x7c,0x6c270}, {0x80,0x6c2b0}, {0x88,0x6b5a0}})
            host::scene_method<Unary>(frame, module_, entry.first, entry.second);
    }
    HMODULE module_;
    host::Graphics* graphics_ = nullptr;
    bool live_ = false;
};

class WindowHook {
public:
    explicit WindowHook(HWND window) : window_(window) {
        if (active_) throw std::logic_error("Launch window hook already installed");
        SetLastError(0);
        previous_ = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(window_, GWLP_WNDPROC,
                                        reinterpret_cast<LONG_PTR>(&procedure)));
        if (!previous_) throw std::runtime_error("Cannot install launch window procedure");
        active_ = this;
    }
    ~WindowHook() {
        if (IsWindow(window_)) SetWindowLongPtrW(window_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(previous_));
        active_ = nullptr;
    }
    bool closed() const { return closed_; }
    void menu_cursor(bool value) { menu_cursor_ = value; }
private:
    static LRESULT CALLBACK procedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
        if (!active_) return DefWindowProcW(window, message, wparam, lparam);
        if (message == WM_CLOSE) { active_->closed_ = true; return 0; }
        if (message == WM_SETCURSOR && active_->menu_cursor_ && LOWORD(lparam) == HTCLIENT) {
            SetCursor(nullptr); return TRUE;
        }
        return CallWindowProcW(active_->previous_, window, message, wparam, lparam);
    }
    inline static WindowHook* active_ = nullptr;
    HWND window_;
    WNDPROC previous_ = nullptr;
    bool closed_ = false, menu_cursor_ = false;
};

class OpeningBackend {
public:
    OpeningBackend(host::StartupSession& startup, HMODULE module, host::BorrowedGraphicsInput& keyboard)
        : startup_(startup), keyboard_(keyboard), video_(startup.graphics(), module), module_(module) {}
    void begin_movie(const char* filename) {
        // Underlying Win32 payloads decoded from00A59000; binary integrity VM
        // and original-address checks are outside the native reconstruction.
        IsWindow(reinterpret_cast<HWND>(static_cast<std::uintptr_t>(0x97a5e32)));
        GetFocus();
        video_.open(filename, 0x400);
        float target; const std::uint32_t bits = 0x3c343958; std::memcpy(&target, &bits, 4);
        startup_.fade_movie_music(target, 2000);
        if (video_.opened()) {
            using Dimension = std::int32_t (WINAPI *)(host::EngineFrame*);
            const auto width = host::scene_method<Dimension>(startup_.graphics(), module_, 0x18, 0x6de50)(startup_.graphics());
            const auto height = host::scene_method<Dimension>(startup_.graphics(), module_, 0x1c, 0x6de70)(startup_.graphics());
            const auto dimensions = video_.dimensions();
            video_.rectangle(frontend::opening_video_rectangle(width, height, dimensions.width, dimensions.height));
            video_.volume(startup_.movie_volume());
            video_.play();
        }
        GetTickCount();
    }
    void close_movie(const char* filename) {
        for (int i = 0; i != 3; ++i) video_.clear_present();
        // Ensure engine resources close even if actual profile persistence throws.
        try { startup_.remember_movie(filename); startup_.fade_movie_music(1, 2000); }
        catch (...) { video_.close(false); throw; }
        video_.close(false);
    }
    std::uint32_t next_scan() { return keyboard_.next_scan(); }
    void update_sound(std::uint32_t milliseconds) { startup_.update_sound(milliseconds); }
    std::int32_t movie_status() const { return video_.playback_status(); }
    void stop_movie() { video_.stop(); }
    void clear_present() const { video_.clear_present(); }
    void draw_movie() const { video_.draw(false); }
private:
    host::StartupSession& startup_;
    host::BorrowedGraphicsInput& keyboard_;
    host::EngineVideo video_;
    HMODULE module_;
};

class LaunchServices {
public:
    LaunchServices(host::StartupSession& startup, host::MenuDriver& driver, HMODULE module,
                   host::Graphics* graphics, HWND window, host::BorrowedGraphicsInput& keyboard)
        : startup_(startup), driver_(driver), module_(module), graphics_(graphics), window_(window), keyboard_(keyboard) {}
    std::uint32_t application_flags() const { return startup_.application_flags(); }
    bool application_input_enabled() const { return startup_.application_input_enabled(); }
    void initialize_mouse(std::uint32_t flags) { startup_.initialize_mouse(flags); }
    bool prepare_application() { return startup_.prepare_application(); }
    void reset_profile() { startup_.reset_profile(); }
    void delay_elapsed() { startup_.delay_elapsed(); }
    std::uint32_t random_word() { return startup_.random_word(); }
    template<class Controller> bool alternate_start(Controller& controller) {
        const auto* command = GetCommandLineA();
        if (!command) throw std::runtime_error("Application command line unavailable");
        if (std::strstr(command, " -cmd ") || std::strstr(command, " -c"))
            throw std::logic_error("Alternate console startup is not reconstructed");
        if (controller.active()) throw std::logic_error("Normal alternate-start check needs an empty owner");
        return false;
    }
    std::unique_ptr<frontend::ApplicationState> create_video_state() {
        return std::make_unique<frontend::OpeningApplicationState<OpeningBackend>>(
            std::make_unique<OpeningBackend>(startup_, module_, keyboard_));
    }
    std::unique_ptr<frontend::ApplicationState> create_frontend_state() {
        return std::make_unique<frontend::FrontendApplicationState<host::ApplicationMenu>>(
            std::make_unique<host::ApplicationMenu>(startup_, driver_, module_, graphics_, window_, keyboard_));
    }
    std::int32_t query_input(const std::uint32_t*& output, std::uint8_t mode) { return startup_.query_input(output, mode); }
    void close_mouse() { startup_.close_mouse(); }
    void reset_control() { startup_.reset_control(); std::puts("Controller cold control reset completed"); }
    void finish_frame(std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t) {
        throw std::logic_error("Network frame finalizer is outside standalone launch mode");
    }
    void finish_fixed_frame(std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t) {
        throw std::logic_error("Network fixed-frame finalizer is outside standalone launch mode");
    }
    void sample_frame_input() { startup_.sample_frame_input(); }
    void tick_driver(std::uint32_t milliseconds) {
        using Tick = void (WINAPI *)(host::EngineFrame*, std::uint32_t);
        host::scene_method<Tick>(driver_.frame(), module_, 0x0c, 0x152c0)(driver_.frame(), milliseconds);
    }
    void control_phase(std::uint32_t milliseconds, bool fixed) { startup_.control_phase(milliseconds, fixed); }
    void control_maintenance(std::uint32_t milliseconds) { startup_.control_maintenance(milliseconds); }
    void present() { graphics_->methods->present(graphics_); }
private:
    host::StartupSession& startup_;
    host::MenuDriver& driver_;
    HMODULE module_;
    host::Graphics* graphics_;
    HWND window_;
    host::BorrowedGraphicsInput& keyboard_;
};

int launch(HMODULE module, HMODULE rw, HMODULE joy, std::uint32_t limit) {
    host::StartupSession startup(module, rw, joy); // game.cfg before graphics.
    startup.before_devices();
    WindowGraphics graphics(module);
    int result = 0;
    try {
        host::EngineSound sound(module, graphics.window());
        host::MenuDriver driver(module);
        try {
            startup.setup_devices();
            WindowHook hook(graphics.window());
            SetWindowTextW(graphics.window(), L"HD2 recovered controller - opening / Main / Exit");
            {
                host::BorrowedGraphicsInput keyboard(startup.graphics(), module);
                LaunchServices services(startup, driver, module, graphics.graphics(), graphics.window(), keyboard);
                frontend::ApplicationController<frontend::ApplicationState, LaunchServices> controller(services);
                if (!controller.start(false)) throw std::runtime_error("Controller application preparation failed");
                host::FrameClockPlatform clock_platform;
                frontend::FrameClock frame_clock(clock_platform);
                std::uint32_t frames = 0, previous_type = 0, previous_sequence = 0;
                const auto trace = [&] {
                    if (!controller.active()) return;
                    const auto type = controller.active()->type();
                    const auto sequence = type == 10 ? controller.active()->video_sequence() : 0;
                    if (type != previous_type || sequence != previous_sequence) {
                        std::printf("Controller active: type=%u sequence=%u status=%u phase=%u\n", type, sequence,
                                    controller.active()->status(), controller.phase());
                        previous_type = type; previous_sequence = sequence;
                    }
                    hook.menu_cursor(type == 2);
                };
                trace();
                while (!hook.closed() && IsWindow(graphics.window()) && (!limit || frames < limit)) {
                    graphics.graphics()->methods->pump(graphics.graphics());
                    if (hook.closed()) break;
                    keyboard.poll_keyboard();
                    // Default original scan mode0 does not pop this observation.
                    // Screenshot export is deliberately not enabled in this menu target.
                    const auto milliseconds = frame_clock.next(10, 100);
                    if (!frontend::advance_application_frame(controller, services, milliseconds)) break;
                    ++frames;
                    trace();
                }
                controller.shutdown();
                std::printf("Controller launch completed; frames=%u\n", frames);
            }
            startup.close_consumers();
        } catch (...) { startup.close_consumers(); throw; }
    } catch (...) { graphics.close(); startup.close_archives(); throw; }
    graphics.close(); startup.close_archives();
    std::printf("Controller launch teardown completed; result=%d\n", result);
    return result;
}
} // namespace

int wmain(int argc, wchar_t** argv) {
    if (argc != 4 || std::wstring_view(argv[2]) != L"--frames") {
        std::fputs("Use tools/run_application_launch.py for the isolated rebuilt launch.\n", stderr);
        return 2;
    }
    try {
        const auto limit = hd2::host::parse_frame_limit(argv[3]);
        wchar_t filename[32768];
        const auto length = GetModuleFileNameW(nullptr, filename, 32768);
        if (!length || length >= 32768) throw std::runtime_error("Launch executable path unavailable");
        const std::wstring executable(filename, length);
        const auto slash = executable.find_last_of(L"\\/");
        if (slash == std::wstring::npos || !SetCurrentDirectoryW(executable.substr(0, slash).c_str()) ||
                GetFileAttributesW(L".hd2-launch-workspace") == INVALID_FILE_ATTRIBUTES)
            throw std::runtime_error("Controller launch requires the isolated staged workspace");
        if (!std::freopen("host-runtime.log", "w", stdout) || !std::freopen("host-runtime.log", "a", stderr) ||
                _dup2(_fileno(stdout), _fileno(stderr)) != 0) return 1;
        std::setvbuf(stdout, nullptr, _IONBF, 0); std::setvbuf(stderr, nullptr, _IONBF, 0);
        std::puts("Recovered application controller launch; bounded Main/Exit, ordinary opening movies");
        Module engine(argv[1]);
        const auto rw = GetModuleHandleW(L"rw_data.dll");
        if (!rw) throw std::runtime_error("LS3DF did not load rw_data.dll");
        const std::wstring engine_path(argv[1]);
        Module joy((engine_path.substr(0, engine_path.find_last_of(L"\\/")) + L"\\IJoy.dll").c_str());
        if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED))) throw std::runtime_error("Launch COM initialization failed");
        struct Com { ~Com() { CoUninitialize(); } } com;
        return launch(engine.value, rw, joy.value, limit);
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Controller launch failed: %s\n", error.what());
        return 1;
    }
}
