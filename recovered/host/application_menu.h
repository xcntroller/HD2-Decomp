#ifndef HD2_HOST_APPLICATION_MENU_H
#define HD2_HOST_APPLICATION_MENU_H

#include "host/graphics_interface.h"
#include "host/menu_input_session.h"
#include "host/menu_screen_factory.h"
#include "host/menu_text.h"
#include "host/startup_session.h"
#include "host/borrowed_input.h"
#include "frontend/keyboard_preprocessing.h"
#include <ctime>

namespace hd2::host {

// Concrete bounded Main/Exit frontend runtime. Every tick/render is called by
// the application state/controller; there is no inner movie/menu host loop.
// Original eager builders for the other twelve screens are not substituted.
class ApplicationMenu {
public:
    ApplicationMenu(StartupSession& startup, MenuDriver& driver, HMODULE module,
                    Graphics* graphics, HWND window, BorrowedGraphicsInput& keyboard)
        : startup_(startup), driver_(driver), module_(module), graphics_(graphics),
          window_(window), keyboard_(keyboard), actions_(*this) {}
    void initialize(std::function<void()> quit) {
        if (scene_) throw std::logic_error("Frontend initialized twice");
        if (!frontend::startup_case_equal(startup_.language(), "english"))
            throw std::logic_error("The current compiled menu font renderer supports English/800x600");
        quit_ = std::move(quit);
        scene_ = std::make_unique<MenuScene>(driver_, module_, true, true);
        scene_->initialize_confirmation_text_frame(module_);
        text_ = std::make_unique<MenuText<Graphics>>(graphics_, module_, *scene_, true, false, &startup_.text());
        cursor_ = std::make_unique<MenuCursor<Graphics>>(graphics_, module_);
        events_.emplace(actions_, false);
        MenuScreenFactory<Actions> factory(*scene_, module_,
            [&](std::uint32_t id) { return text_->object_for(id); }, *events_, products_, actions_);
        frontend::construct_registered_screens(factory);
        for (std::size_t i = 0; i != products_.size(); ++i) products_.screen(i).controls().bind_pick_targets();
        keyboard_context_.emplace(*this);
        const auto moved = [this](float x, float y) { scene_->update_input_cursor(x, y, 800, 600); };
        const auto click = [this] { scene_->request_click_sound(); };
        main_input_.emplace(input_, products_.screen(0).focus(),
            [this](const CursorPoint& point) { return pick(false, point); }, moved, click);
        confirm_input_.emplace(input_, products_.screen(1).focus(),
            [this](const CursorPoint& point) { return pick(true, point); }, moved, click);
        if (startup_.sound_available()) scene_->load_click_sound(module_);
        startup_.begin_menu_music();
        date_tick(true);
        std::puts("Controller menu initialized: type=2; actual localized captions; Main/Exit owners installed");
    }
    void shutdown() {
        if (!scene_) return;
        main_input_.reset(); confirm_input_.reset(); keyboard_context_.reset();
        if (events_) events_->close(); // Stack borrowers before registered products.
        products_.clear();
        events_.reset();
        actions_.clear();
        input_.clear(); point_.reset();
        text_.reset(); cursor_.reset(); scene_.reset();
        SetCursor(LoadCursorW(nullptr, IDC_ARROW));
        std::puts("Controller menu shutdown: screen/input/text/scene owners released");
    }
    void input(const std::uint32_t* first, const std::uint32_t* second) {
        frontend::preprocess_frontend_keyboard(*keyboard_context_, keyboard_, first, second);
    }
    void update(std::uint32_t milliseconds) {
        // Actual0066BEA0 normal mode10/gate7 executes BOTH pairs.
        date_tick(false); startup_.update_sound(milliseconds);
        date_tick(false); startup_.update_sound(milliseconds);
        events_->pump();
        if (exit_requested_) return;
        const auto* current = events_->screens().current();
        if (!current) { input_.clear(); return; }
        const bool focused = GetAncestor(GetForegroundWindow(), GA_ROOT) == window_ && !IsIconic(window_);
        if (!focused) { input_.clear(); point_.reset(); return; }
        if (!point_) point_ = cursor_->position(window_);
        else {
            point_->x = std::clamp(point_->x + static_cast<float>(startup_.mouse_x()), 0.0f, 800.0f);
            point_->y = std::clamp(point_->y + static_cast<float>(startup_.mouse_y()), 0.0f, 600.0f);
        }
        if (!point_) { input_.clear(); return; }
        const auto target = pick(confirming_, *point_);
        auto& active = confirming_ ? *confirm_input_ : *main_input_;
        active.tick(*point_, target, startup_.mouse_buttons(), milliseconds, startup_.mouse_wheel());
        events_->pump(); // Click messages follow the recovered existing native queue.
    }
    void render() {
        if (confirming_) scene_->set_confirmation_visible(module_, false);
        if (scene_->render() < 0) throw std::runtime_error("Controller menu render failed");
        auto& main = products_.screen(0);
        const auto exit_target = main.controls().target_for_role(frontend::MenuControlRole::exit);
        const auto exit_focused = main.focus().control_for_target(exit_target) == main.focus().focused_control();
        text_->draw(!confirming_ && exit_focused);
        if (confirming_) {
            MenuScene::ModalPass modal(*scene_, module_);
            if (scene_->render() < 0) throw std::runtime_error("Controller confirmation render failed");
            text_->draw(false, true, products_.screen(1).focus().focused_control());
            if (!modal.restore()) throw std::runtime_error("Controller modal render state restore failed");
        }
        if (point_ && GetAncestor(GetForegroundWindow(), GA_ROOT) == window_ && !IsIconic(window_)) {
            cursor_->draw_at(*point_);
            SetCursor(nullptr);
        }
        if (!rendered_) {
            std::puts("Controller menu rendered: actual scene/text/cursor submitted");
            rendered_ = true;
        }
    }
private:
    class Actions final : public frontend::MenuEventActions {
    public:
        explicit Actions(ApplicationMenu& owner) : owner_(owner) {}
        void bind_screen(std::uint32_t family, MenuScreenEvents& screen) {
            if (family >= screens_.size() || screens_[family]) throw std::logic_error("Duplicate menu registration");
            screens_[family] = &screen;
        }
        bool screen_available(std::uint32_t family) const override { return family < screens_.size() && screens_[family]; }
        void activate_screen(const frontend::ScreenRegistration& screen, std::uint32_t focus) override {
            runtime(screen.identifier >> 26).activate(focus);
            owner_.confirming_ = screen.identifier == 0x10000000;
            std::puts(owner_.confirming_ ? "Controller confirmation opened" : "Controller main menu active");
        }
        void deactivate_screen(const frontend::ScreenRegistration& screen) override {
            runtime(screen.identifier >> 26).deactivate();
            if (screen.identifier == 0x10000000) owner_.confirming_ = false;
        }
        bool dispatch_screen_event(std::uint32_t family, frontend::ControlEvent& event) override {
            return runtime(family).dispatch(event);
        }
        void quit() override {
            owner_.exit_requested_ = true;
            owner_.quit_(); // State status4, not an outer-loop running Boolean.
            std::puts("Controller menu requested status=4");
        }
        void set_application_input_enabled(bool value) override { owner_.startup_.application_input_enabled(value); }
        void clear() { screens_.fill(nullptr); }
    private:
        MenuScreenEvents& runtime(std::uint32_t family) const {
            if (!screen_available(family)) throw std::logic_error("Unavailable compiled menu screen");
            return *screens_[family];
        }
        ApplicationMenu& owner_;
        std::array<MenuScreenEvents*, 64> screens_{};
    };
    class KeyboardContext : public MenuKeyboardInput {
    public:
        explicit KeyboardContext(ApplicationMenu& owner)
            : MenuKeyboardInput(owner.input_, owner.products_.screen(0).focus(), owner.products_.screen(1).focus(),
                                *owner.events_, owner.confirming_), owner_(owner) {}
        bool input_blocked() const { return owner_.events_->common_state().motion_only; }
        // Final table00815E30+44 →0067EA80 is an actual RET8 body.
        void preprocess_buffers(const std::uint32_t*, const std::uint32_t*) {}
        bool screens_available() const { return owner_.events_->screens().current() != nullptr; }
        bool wide_character_mode() const { return owner_.startup_.wide_characters(); }
        bool character_current_target(std::uint32_t) const {
            if (owner_.events_->common_state().capture_owner)
                throw std::logic_error("Captured character-target backend is not implemented");
            return false; // Actual absent +10 target for the implemented Main/Exit menu.
        }
        void trace_wide_character(std::uint16_t value) const {
            char message[64]; std::snprintf(message, sizeof(message), "KEY VALUE = %u\n", value);
            OutputDebugStringA(message);
        }
        void enqueue_debug_event() { owner_.events_->enqueue({0, 0x0200007d, 0, 0, 0, {}}); }
    private:
        ApplicationMenu& owner_;
    };
    std::uint32_t pick(bool confirmation, const CursorPoint& point) {
        auto& focus = products_.screen(confirmation ? 1 : 0).focus();
        const auto picked = confirmation ? scene_->pick_confirmation(module_, point.x, point.y, 800, 600)
                                         : scene_->pick_main_menu(module_, point.x, point.y, 800, 600);
        return confirmation ? focus.target_for_control(std::max(0, focus.interactive_control_for_target(picked)))
                            : focus.interactive_target(picked);
    }
    void date_tick(bool initial) {
        const auto now = std::time(nullptr);
        const auto* local = std::localtime(&now);
        if (!local) throw std::runtime_error("Menu local date/time unavailable");
        const bool changed = local->tm_hour != cached_hour_ || local->tm_min != cached_minute_;
        if (changed) {
            char value[96];
            std::snprintf(value, sizeof(value), "%d. %02d. %d  %02d:%02d", local->tm_mday,
                          local->tm_mon + 1, local->tm_year + 1900, local->tm_hour, local->tm_min);
            cached_date_ = value; cached_hour_ = local->tm_hour; cached_minute_ = local->tm_min;
        }
        if (initial || changed) {
            frontend::ControlEvent event{0x04300000, 0x01000014, 12, 0, 0, {}};
            event.text = std::make_shared<const std::string>(cached_date_);
            events_->enqueue(event);
        }
    }
    StartupSession& startup_;
    MenuDriver& driver_;
    HMODULE module_;
    Graphics* graphics_;
    HWND window_;
    BorrowedGraphicsInput& keyboard_;
    std::function<void()> quit_;
    std::unique_ptr<MenuScene> scene_;
    std::unique_ptr<MenuText<Graphics>> text_;
    std::unique_ptr<MenuCursor<Graphics>> cursor_;
    MenuInputSession input_;
    Actions actions_;
    frontend::OwnedScreens<MenuScreenProduct> products_;
    std::optional<frontend::MenuEventQueue> events_;
    std::optional<KeyboardContext> keyboard_context_;
    std::optional<MenuFrameInput<MenuFocus>> main_input_, confirm_input_;
    std::optional<CursorPoint> point_;
    bool confirming_ = false, exit_requested_ = false, rendered_ = false;
    int cached_hour_ = 0, cached_minute_ = 0;
    std::string cached_date_;
};

} // namespace hd2::host
#endif
