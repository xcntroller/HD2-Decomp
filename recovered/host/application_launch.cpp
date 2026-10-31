#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "frontend/application_controller.h"
#include "frontend/application_states.h"
#include "frontend/application_frame.h"
#include "frontend/frame_clock.h"
#include "host/application_menu.h"
#include "host/engine_video.h"
#include "host/engine_sound.h"
#include "host/opening_audio.h"
#include "host/opening_text.h"
#include "host/frame_clock.h"
#include "host/window_position.h"
#include <optional>
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
        live_ = false;
        graphics_->methods->close(graphics_);
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
    ~WindowHook() { frontend::CleanupErrors errors; errors.attempt([&] { restore(); }); }
    void restore() {
        if (!installed_) return;
        installed_ = false;
        active_ = nullptr;
        if (IsWindow(window_)) {
            SetLastError(0);
            if (!SetWindowLongPtrW(window_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(previous_)) && GetLastError())
                throw std::runtime_error("Cannot restore launch window procedure");
        }
        std::puts("Shutdown: launch window hook restored");
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
    bool installed_ = true;
};

class OpeningBackend {
public:
    OpeningBackend(host::StartupSession& startup, HMODULE module, host::BorrowedGraphicsInput& keyboard)
        : startup_(startup), keyboard_(keyboard), video_(startup.graphics(), module),
          audio_(module), module_(module), subtitles_(*this) {}
    ~OpeningBackend() { release_resources(); }
    void begin_movie(const char* filename) {
        if (resources_live_) throw std::logic_error("Opening resources already owned");
        resources_live_ = true;
        // Underlying Win32 payloads decoded from00A59000; binary integrity VM
        // and original-address checks are outside the native reconstruction.
        IsWindow(reinterpret_cast<HWND>(static_cast<std::uintptr_t>(0x97a5e32)));
        GetFocus();
        audio_.create_voice(); //00688040: separate voice BEFORE IShow, even without cues.
        video_.open(filename, 0x400);
        float target; const std::uint32_t bits = 0x3c343958; std::memcpy(&target, &bits, 4);
        startup_.fade_movie_music(target, 2000);
        if (video_.opened()) {
            using Dimension = std::int32_t (WINAPI *)(host::EngineFrame*);
            const auto width = host::scene_method<Dimension>(startup_.graphics(), module_, 0x18, 0x6de50)(startup_.graphics());
            const auto height = host::scene_method<Dimension>(startup_.graphics(), module_, 0x1c, 0x6de70)(startup_.graphics());
            const auto dimensions = video_.dimensions();
            video_.rectangle(frontend::opening_video_rectangle(width, height, dimensions.width, dimensions.height));
            frontend::LocalizedTextCache local_text;
            const auto loaded = startup_.load_movie_subtitles(filename, local_text);
            const auto enabled = startup_.subtitles_enabled();
            subtitles_.parse(local_text, enabled);
            if (!subtitles_.text().empty()) {
                if (startup_.wide_characters()) throw std::logic_error("Wide opening subtitle renderer is not reconstructed");
                text_ = std::make_unique<host::OpeningText>(
                    reinterpret_cast<host::Graphics*>(startup_.graphics()), module_, startup_.opening_text_styles());
                for (const auto& cue : subtitles_.text()) text_->validate(cue.text);
            }
            std::printf("Opening subtitles: movie=%s loaded=%u enabled=%u text=%zu audio=%zu\n",
                filename, static_cast<unsigned>(loaded), static_cast<unsigned>(enabled),
                subtitles_.text().size(), subtitles_.audio().size());
            video_.volume(startup_.movie_volume());
            video_.play();
            time_marker(); // Original QPC clock sample after successful Play.
        }
        GetTickCount();
    }
    void close_movie(const char* filename) {
        for (int i = 0; i != 3; ++i) video_.clear_present();
        // Ensure engine resources close even if actual profile persistence throws.
        try { startup_.remember_movie(filename); startup_.fade_movie_music(1, 2000); }
        catch (...) { release_resources(); throw; }
        release_resources();
        std::puts("Opening resources released: movie/voice/text records/samples/styles/text owner");
    }
    std::uint32_t next_scan() { return keyboard_.next_scan(); }
    void update_sound(std::uint32_t milliseconds) { startup_.update_sound(milliseconds); }
    std::int32_t movie_status() const { return video_.playback_status(); }
    void stop_movie() { video_.stop(); }
    void clear_present() const { video_.clear_present(); }
    void update_subtitles() { subtitles_.update(); }
    void draw_movie() const { video_.draw_with_text(has_text(), [&] { text_->draw(); }); }

    // Boundary consumed by the portable00689970/00688C10 reconstruction.
    using Sample = host::OpeningAudio::Sample;
    Sample create_sample() const { return audio_.create_sample(); }
    std::int32_t load_sample(const Sample& sample, const char* filename, std::uint32_t flags) const {
        return audio_.load_sample(sample, filename, flags);
    }
    bool has_voice() const { return audio_.has_voice(); }
    bool has_text() const { return text_ != nullptr; }
    void read_movie_time(std::uint32_t& time) {
        video_.elapsed_time(time);
        last_movie_time_ = time;
    }
    void clear_text() {
        text_->clear();
        std::printf("Opening subtitle cleared: time=%u\n", last_movie_time_);
    }
    void set_text(frontend::SharedText text, std::uint8_t style, std::int32_t index) {
        std::printf("Opening subtitle active: time=%u characters=%zu style=%u\n",
            last_movie_time_, text->size(), static_cast<unsigned>(style));
        text_->set(std::move(text), style, index);
    }
    void set_driver_volume() { startup_.set_subtitle_driver_volume(); }
    void bind_sample(const Sample& sample, std::uint32_t flags) { audio_.bind_sample(sample, flags); }
    void voice_volume(float volume) { audio_.voice_volume(volume); }
    void play_voice(std::uint32_t flags, std::uint32_t offset) { audio_.play_voice(flags, offset); }
    void time_marker() const {
        LARGE_INTEGER frequency{}, counter{};
        QueryPerformanceFrequency(&frequency);
        QueryPerformanceCounter(&counter);
    }
private:
    void release_resources() noexcept {
        if (!resources_live_) return;
        video_.close(false);
        audio_.close_voice();
        subtitles_.clear_records(); // Reverse strings, then sample refs AFTER voice closes.
        startup_.opening_text_styles().clear_movie_styles();
        text_.reset();
        resources_live_ = false;
    }
    host::StartupSession& startup_;
    host::BorrowedGraphicsInput& keyboard_;
    host::EngineVideo video_;
    host::OpeningAudio audio_;
    HMODULE module_;
    std::unique_ptr<host::OpeningText> text_;
    frontend::OpeningSubtitles<OpeningBackend> subtitles_;
    std::uint32_t last_movie_time_ = 0;
    bool resources_live_ = false;
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
    void clear_controller_resources() { startup_.clear_controller_resources(); }
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

// Declaration order is also the fallback lifetime: no engine consumer outlives
// its service/device/module. Explicit cleanup below handles all acquired prefixes.
struct LaunchOwners {
    using Controller = frontend::ApplicationController<frontend::ApplicationState, LaunchServices>;
    std::optional<host::StartupSession> startup;
    std::optional<WindowGraphics> graphics;
    std::optional<host::EngineSound> sound;
    std::optional<host::MenuDriver> driver;
    std::optional<host::BorrowedGraphicsInput> keyboard;
    std::optional<LaunchServices> services;
    std::optional<Controller> controller;
    std::optional<WindowHook> hook;
    HMODULE engine, rw_data;
    bool completed = false;

    LaunchOwners(HMODULE module, HMODULE rw) : engine(module), rw_data(rw) {}
    void restore_window_hook() {
        frontend::CleanupErrors errors;
        errors.attempt([&] { if (hook) hook->restore(); });
        hook.reset();
        errors.rethrow();
    }
    void close_application() {
        frontend::CleanupErrors errors;
        errors.attempt([&] { if (controller) controller->shutdown(); });
        controller.reset(); services.reset(); keyboard.reset();
        errors.rethrow();
    }
    void close_consumers() { if (startup) startup->close_consumers(graphics.has_value()); }
    void save_window() {
        // Original early failures skip the normal tail. Only persist a completed
        // run, and seed ignored rectangle failure with real known coordinates.
        if (!completed || !graphics || !IsWindow(graphics->window())) return;
        host::WindowPositionPlatform platform(engine);
        frontend::WindowPosition known{};
        if (!platform.capture_window_position(known)) {
            std::puts("Shutdown: window.cfg skipped; no valid window coordinates");
            return;
        }
        host::WindowPositionFiles files(rw_data);
        const bool opened = frontend::save_window_position(files, platform, known);
        std::printf("Shutdown: window.cfg %s\n", opened ? "write attempted" : "open failed; skipped");
    }
    void close_3d() {
        frontend::CleanupErrors errors;
        //006DC3A0 places final loaded caches after window persistence and
        // before I3D/sound/graphics. Also reclaim partial acquisition on failure.
        errors.attempt([&] { if (startup) startup->clear_final_resources(); });
        driver.reset();
        errors.rethrow();
    }
    void close_sound() { sound.reset(); }
    void close_graphics() { graphics.reset(); }
    void close_archives() {
        if (!startup) return;
        startup->close_archives();
        startup.reset();
        std::puts("Shutdown: archives released; startup owner destroyed");
    }
};

int launch(HMODULE module, HMODULE rw, HMODULE joy, std::uint32_t limit) {
    LaunchOwners owners(module, rw);
    frontend::CleanupErrors errors;
    try {
        auto& startup = owners.startup.emplace(module, rw, joy); // game.cfg before graphics.
        startup.before_devices();
        auto& graphics = owners.graphics.emplace(module);
        owners.sound.emplace(module, graphics.window());
        auto& driver = owners.driver.emplace(module);
        startup.setup_devices();
        auto& hook = owners.hook.emplace(graphics.window());
        SetWindowTextW(graphics.window(), L"HD2 recovered controller - opening / Main / Exit");
        auto& keyboard = owners.keyboard.emplace(startup.graphics(), module);
        auto& services = owners.services.emplace(startup, driver, module, graphics.graphics(), graphics.window(), keyboard);
        auto& controller = owners.controller.emplace(services);
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
        owners.completed = true;
        std::printf("Controller launch completed; frames=%u reason=%s\n", frames,
            hook.closed() || !IsWindow(graphics.window()) ? "window-close" :
            limit && frames >= limit ? "frame-limit" : "state-exit");
    } catch (...) { errors = frontend::CleanupErrors(std::current_exception()); }
    frontend::close_launch_owners(owners, errors);
    std::printf("Controller launch teardown completed; result=%u\n", errors.failed() ? 1u : 0u);
    errors.rethrow();
    return 0;
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
    } catch (...) {
        std::fputs("Controller launch failed: unknown native exception\n", stderr);
        return 1;
    }
}
