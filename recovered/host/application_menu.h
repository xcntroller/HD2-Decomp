#ifndef HD2_HOST_APPLICATION_MENU_H
#define HD2_HOST_APPLICATION_MENU_H

#include "frontend/keyboard_preprocessing.h"
#include "frontend/menu_content_settings.h"
#include "frontend/runtime_confirmation.h"
#include "host/borrowed_input.h"
#include "host/cinematics_menu.h"
#include "host/options_menu.h"
#include "host/profiles_menu.h"
#include "host/single_player_menu.h"
#include "host/multiplayer_menu.h"
#include "host/graphics_interface.h"
#include "host/menu_input_session.h"
#include "host/menu_screen_factory.h"
#include "host/menu_text.h"
#include "host/startup_session.h"
#include <ctime>

namespace hd2::host {

// Concrete bounded Main/Exit frontend runtime. Every tick/render is called by
// the application state/controller; there is no inner movie/menu host loop.
// Original eager builders for the other twelve screens are not substituted.
class ApplicationMenu {
  public:
    ApplicationMenu(StartupSession &startup, MenuDriver &driver, HMODULE module, Graphics *graphics,
                    HWND window, BorrowedGraphicsInput &keyboard)
        : startup_(startup), driver_(driver), module_(module), graphics_(graphics), window_(window),
          keyboard_(keyboard), actions_(*this) {}
    ~ApplicationMenu() {
        frontend::CleanupErrors errors;
        errors.attempt([&] { shutdown(); });
    }
    void initialize(std::function<void()> quit, std::function<void(std::uint32_t)> select_state) {
        if (scene_)
            throw std::logic_error("Frontend initialized twice");
        if (!frontend::startup_case_equal(startup_.language(), "english"))
            throw std::logic_error("The current compiled menu font renderer supports English/800x600");
        quit_ = std::move(quit);
        select_state_ = std::move(select_state);
        frontend::load_menu_content(driver_, 10, [&] {
            scene_ =
                std::make_unique<MenuScene>(driver_, module_, true, true, &startup_.loaded_resources(), true);
            options_.emplace(startup_, [this] { scene_->request_click_sound(); });
            scene_->initialize_confirmation_text_frame(module_);
            scene_->initialize_confirmation_text_frame(module_, scene_->runtime_confirmation_frame());
            text_ = std::make_unique<MenuText<Graphics>>(graphics_, module_, *scene_, true, true,
                                                         &startup_.text());
            cursor_ = std::make_unique<MenuCursor<Graphics>>(graphics_, module_);
            events_.emplace(actions_, false);
            dialog_services_.emplace(*this);
            dialog_ =
                std::make_unique<frontend::RuntimeConfirmation<DialogServices>>(*dialog_services_, *events_);
            dialog_definition_.emplace(dialog_->definition());
            text_->attach_runtime_definition(*scene_, module_, *dialog_definition_);
            RuntimeMenuScreenFactory<Actions> runtime_factory(
                *scene_, module_, *dialog_definition_,
                [this](std::uint32_t id) { return text_->object_for(id); }, *events_, runtime_product_,
                actions_);
            if (!frontend::construct_runtime_confirmation(runtime_factory))
                throw std::runtime_error("Runtime dialog construction failed");
            runtime_product_->controls().bind_pick_targets();
            widgets_ = std::make_unique<MenuWidgetContext>(input_, *events_, *options_);
            cinematics_ =
                std::make_unique<CinematicsMenu>(startup_, *scene_, driver_, module_, graphics_, *events_);
            cinematics_->bind(select_state_);
            options_menu_=std::make_unique<OptionsMenu>(startup_,*scene_,driver_,module_,*events_);
            options_menu_->dialog_request([this] { request_dialog_localized(2,9520,0x020000c7,0x020000c7); });
            profiles_menu_=std::make_unique<ProfilesMenu>(startup_,*scene_,module_,*events_);
            profiles_menu_->dialog_request([this](unsigned mode,unsigned label,unsigned yes,unsigned no){request_dialog_localized(mode,label,yes,no);});
            profiles_menu_->changed([this]{
                options_menu_->profile_changed();
                refresh_main();
            });
            single_player_=std::make_unique<SinglePlayerMenu>(startup_,*scene_,driver_,module_,*events_);
            single_player_->transitions([this]{select_state_(2);},[this](frontend::SharedText text) {
                request_dialog_text(2,std::move(text),0,0);
            });
            multiplayer_=std::make_unique<MultiplayerMenu>(startup_,*scene_,driver_,module_,*events_);
            multiplayer_->errors([this](frontend::SharedText text){request_dialog_text(2,std::move(text),0,0);});
            MenuScreenFactory<Actions> factory(
                *scene_, module_, [&](std::uint32_t id) { return text_->object_for(id); }, *events_,
                products_, actions_, widgets_.get(), true, cinematics_->model(),
                [this] { return cinematics_->definition(); },
                [this](const frontend::MenuScreenDefinition &definition) {
                    text_->attach_screen_definition(*scene_, module_, definition);
                },
                [this](const frontend::MenuControlDefinition &definition, EngineFrame *frame,
                       MenuTextObject &text) -> std::unique_ptr<MenuControl> {
                    if ((definition.identifier & 0xfc000000u) == 0x08000000)
                        return profiles_menu_->make_control(definition,frame,text,*widgets_,[this](unsigned id){return text_->object_for(id);});
                    if ((definition.identifier & 0xfc000000u) == 0x18000000)
                        return options_menu_->make_control(definition,frame,text,*widgets_,
                            [this](std::uint32_t id) { return text_->object_for(id); });
                    if ((definition.identifier & 0xfc000000u) != 0x1c000000)
                        return {};
                    return cinematics_->make_control(
                        definition, frame, text, *widgets_,
                        [this](std::uint32_t id) { return text_->object_for(id); });
                },options_menu_->model(),[this] { return options_menu_->definition(); },
                profiles_menu_->model(),[this]{return profiles_menu_->definition();},true,true);
            frontend::construct_registered_screens(factory);
            add_single_player_screen(single_player_->modes_definition(),single_player_->mode_model());
            add_single_player_screen(single_player_->missions_definition(),single_player_->mission_model());
            for (unsigned slot=8;slot<=14;++slot)
                add_multiplayer_screen(multiplayer_->definition(slot),multiplayer_->model(slot));
            for (std::size_t i = 0; i != products_.size(); ++i)
                products_.screen(i).controls().bind_pick_targets();
        });
        keyboard_context_.emplace(*this);
        const auto moved = [this](float x, float y) { scene_->update_input_cursor(x, y, 800, 600); };
        const auto click = [this] { scene_->request_click_sound(); };
        pointer_input_.emplace(
            input_, events_->common_state(), [this]() -> MenuFocus & { return current_product().focus(); },
            [this](MenuFocus &focus, const CursorPoint &point) { return pick(focus.identifier(), point); },
            moved, click);
        //0066A6E0 retains the shared cursor across frontend controller owners.
        const auto &position = startup_.menu_cursor();
        keyboard_.set_pointer_position(static_cast<int>(position[0]), static_cast<int>(position[1]));
        point_ = CursorPoint{position[0], position[1]};
        input_.pointer.cursor().initialize(point_->x, point_->y, 800, 600);
        input_.previous_point = point_;
        input_.armed = true;
        moved(point_->x, point_->y);
        if (startup_.sound_available())
            scene_->load_click_sound(module_);
        startup_.begin_menu_music();
        date_tick(true);
        std::puts(
            "Controller menu initialized: type=2; actual localized captions; Main/Exit owners installed");
    }
    std::string selected_video_filename() const {
        return cinematics_ ? cinematics_->filename() : std::string{};
    }
    // Original nullable frontend request endpoints for recovered screen callers.
    // No diagnostic key or synthetic menu action is installed.
    void request_dialog_text(std::uint32_t mode, frontend::SharedText text, std::uint32_t affirmative,
                             std::uint32_t negative) {
        frontend::request_runtime_text(dialog_.get(), mode, std::move(text), affirmative, negative);
    }
    void request_dialog_localized(std::uint32_t mode, std::uint32_t label, std::uint32_t affirmative,
                                  std::uint32_t negative) {
        frontend::request_runtime_localized(dialog_.get(), mode, label, affirmative, negative);
    }
    void close_dialog() { frontend::close_runtime_dialog(dialog_.get()); }
    void shutdown() {
        if (!scene_)
            return;
        if (input_.armed)
            startup_.menu_cursor(input_.pointer.cursor().x(), input_.pointer.cursor().y());
        pointer_input_.reset();
        keyboard_context_.reset();
        frontend::CleanupErrors errors;
        if (multiplayer_) multiplayer_->cancel();
        // A throwing deactivation must not strand products or their engine refs.
        errors.attempt([&] {
            if (events_)
                events_->close();
        });
        products_.clear();
        runtime_product_.reset(); // Destroy borrowed button observer contexts first.
        dialog_definition_.reset();
        dialog_.reset();
        dialog_services_.reset();
        widgets_.reset();
        options_.reset();
        events_.reset();
        actions_.clear();
        input_.clear();
        point_.reset();
        text_.reset();
        cursor_.reset();
        cinematics_.reset();
        options_menu_.reset();
        profiles_menu_.reset();
        single_player_.reset();
        multiplayer_.reset();
        scene_.reset();
        SetCursor(LoadCursorW(nullptr, IDC_ARROW));
        std::puts("Controller menu shutdown: screen/input/text/scene owners released");
        errors.rethrow();
    }
    void input(const std::uint32_t *first, const std::uint32_t *second) {
        frontend::preprocess_frontend_keyboard(*keyboard_context_, keyboard_, first, second);
    }
    void update(std::uint32_t milliseconds) {
        FrameServices services{*this};
        frontend::update_frontend(update_state_, services, milliseconds);
    }

    void render() {
        const auto id = current_product().definition().identifier;
        const bool network_modal=id==0x28000000 || id==0x34000000 || id==0x38000000;
        const bool modal = id == 0 || id == 0x10000000 || network_modal;
        auto *overlay = id == 0 ? scene_->runtime_confirmation_frame() :
            network_modal?current_product().model_frame():scene_->confirmation_model_frame();
        if (modal)
            scene_->set_screen_visible(module_, overlay, false);
        if (scene_->render() < 0)
            throw std::runtime_error("Controller menu render failed");
        MenuScreenProduct *background = &current_product();
        if (id == 0) {
            const auto *previous = events_->screens().previous();
            if (!previous)
                throw std::logic_error("Runtime dialog has no retained screen");
            background = &product_for_identifier(previous->identifier);
        } else if (id == 0x10000000)
            background = &products_.screen(0);
        else if (network_modal) {
            const auto retained=multiplayer_->backdrop(id);
            if (!retained) throw std::logic_error("Multiplayer dialog has no retained screen");
            background=&product_for_identifier(*retained);
        }
        if (const auto retained=multiplayer_->backdrop(background->definition().identifier)) {
            auto& base=product_for_identifier(*retained);
            text_->draw_screen(base.definition(),base.focus().focused_control());
        }
        if (background->definition().identifier == 0x1c000000)
            cinematics_->render_preview();
        auto* dropdown=id==0x24000000?multiplayer_->dropdown():nullptr;
        if (dropdown)
            text_->draw_screen_overlay(background->definition(),background->focus().focused_control(),
                dropdown->overlay_definition(),dropdown->overlay_rectangle());
        else if (id==0x24000000 && multiplayer_->drag_text())
            text_->draw_screen_masked(background->definition(),background->focus().focused_control(),
                multiplayer_->drag_text()->rectangle(false));
        else
            text_->draw_screen(background->definition(), id == 0x10000000 ? 0 : background->focus().focused_control());
        if (modal) {
            std::vector<EngineFrame*> backgrounds;
            backgrounds.reserve(products_.size());
            for(std::size_t i=0;i<products_.size();++i)
                backgrounds.push_back(products_.screen(i).model_frame());
            MenuScene::ModalPass pass(*scene_, module_, overlay, std::move(backgrounds));
            if (scene_->render() < 0)
                throw std::runtime_error("Controller dialog render failed");
            text_->draw_screen(current_product().definition(), current_product().focus().focused_control());
            if (!pass.restore())
                throw std::runtime_error("Controller dialog render state restore failed");
        }
        if (id==0x24000000) if (auto* drag=multiplayer_->drag_text()) text_->draw_transient(*drag);
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
    void add_multiplayer_screen(frontend::MenuScreenDefinition definition,EngineFrame* model) {
        scene_->update_screen_frame(module_,model);
        text_->attach_screen_definition(*scene_,module_,definition);
        auto screen=std::make_unique<MenuScreenProduct>(*scene_,module_,false,
            std::vector<frontend::MenuControlRole>{frontend::MenuControlRole::multiplayer},
            [this](unsigned id){return text_->object_for(id);},*events_,model,widgets_.get(),
            [this](const frontend::MenuControlDefinition& d,EngineFrame* frame,MenuTextObject& text){
                return multiplayer_->make_control(d,frame,text,*widgets_,[this](unsigned id){return text_->object_for(id);});
            });
        if (!screen->bind_definition(definition)) throw std::runtime_error("Multiplayer screen binding failed");
        screen->initialize_records(definition.initialization);
        actions_.bind_screen(definition.identifier>>26,screen->events());
        products_.append({definition.label,definition.model,definition.identifier},std::move(screen));
    }
    void add_single_player_screen(frontend::MenuScreenDefinition definition,EngineFrame *model) {
        scene_->update_screen_frame(module_,model);
        text_->attach_screen_definition(*scene_,module_,definition);
        auto screen=std::make_unique<MenuScreenProduct>(*scene_,module_,false,
            std::vector<frontend::MenuControlRole>{frontend::MenuControlRole::single_player},
            [this](unsigned id){return text_->object_for(id);},*events_,model,widgets_.get(),
            [this](const frontend::MenuControlDefinition &d,EngineFrame *frame,MenuTextObject &text) {
                return single_player_->make_control(d,frame,text,*widgets_,[this](unsigned id){return text_->object_for(id);});
            });
        if(!screen->bind_definition(definition))throw std::runtime_error("Single-player entry screen binding failed");
        screen->initialize_records(definition.initialization);
        actions_.bind_screen(definition.identifier>>26,screen->events());
        products_.append({definition.label,definition.model,definition.identifier},std::move(screen));
    }
    MenuScreenProduct &current_product() {
        const auto *current = events_->screens().current();
        if (!current)
            throw std::logic_error("Frontend callback removed the current screen");
        return product_for_identifier(current->identifier);
    }
    MenuScreenProduct &product_for_identifier(std::uint32_t identifier) {
        if (identifier == 0) {
            if (!runtime_product_)
                throw std::logic_error("Runtime dialog product is unavailable");
            return *runtime_product_;
        }
        for(std::size_t i=0;i<products_.size();++i)
            if(products_.screen(i).definition().identifier==identifier) return products_.screen(i);
        throw std::logic_error("Current screen has no compiled product");
    }
    struct FrameServices {
        ApplicationMenu &owner;
        void update_menu(std::uint32_t ms) { owner.date_tick(false); owner.multiplayer_->tick(ms); }
        void update_sound(std::uint32_t milliseconds) { owner.startup_.update_sound(milliseconds); }
        void pump_events() { owner.events_->pump(); }
        bool screens_available() const { return owner.events_->screens().current() != nullptr; }
        void update_pointer(std::uint32_t milliseconds) {
            owner.pointer_input_->tick_relative(
                static_cast<float>(owner.startup_.mouse_x()), static_cast<float>(owner.startup_.mouse_y()),
                owner.startup_.mouse_buttons(), milliseconds, owner.startup_.mouse_wheel());
            owner.point_ = CursorPoint{owner.input_.pointer.cursor().x(), owner.input_.pointer.cursor().y()};
        }
        void update_capture(std::uint32_t milliseconds) {
            owner.events_->common_state().update_capture(milliseconds);
        }
        void update_current_screen(std::uint32_t milliseconds) {
            owner.current_product().events().update(milliseconds);
        }
        void update_overlay(std::uint32_t milliseconds) {
            owner.events_->common_state().update_overlay(milliseconds);
        }
        std::size_t listener_count() const {
            return owner.events_->common_state().notification_tokens.size();
        }
        void update_listener(std::size_t index, std::uint32_t milliseconds) {
            owner.events_->common_state().update_listener(index, milliseconds);
        }
    };
    class Actions final : public frontend::MenuEventActions {
      public:
        explicit Actions(ApplicationMenu &owner) : owner_(owner) {}
        void bind_screen(std::uint32_t family, MenuScreenEvents &screen) {
            if (family >= screens_.size() || screens_[family])
                throw std::logic_error("Duplicate menu registration");
            screens_[family] = &screen;
        }
        bool screen_available(std::uint32_t family) const override {
            return family < screens_.size() && screens_[family];
        }
        void activate_screen(const frontend::ScreenRegistration &screen, std::uint32_t focus) override {
            runtime(screen.identifier >> 26).activate(focus);
            if(screen.identifier==0x04000000) owner_.refresh_main();
            if(screen.identifier==0x08000000) owner_.profiles_menu_->shown();
            if(screen.identifier==0x18000000) owner_.options_menu_->shown();
            if(screen.identifier>=0x20000000 && screen.identifier<=0x38000000) owner_.multiplayer_->shown(screen.identifier);
            owner_.confirming_ = screen.identifier == 0x10000000;
            std::puts(owner_.confirming_ ? "Controller confirmation opened" : "Controller main menu active");
        }
        void deactivate_screen(const frontend::ScreenRegistration &screen) override {
            runtime(screen.identifier >> 26).deactivate();
            if (screen.identifier == 0x10000000)
                owner_.confirming_ = false;
        }
        void profile_back_cleanup() override {owner_.profiles_menu_->back();}
        bool dispatch_screen_event(std::uint32_t family, frontend::ControlEvent &event) override {
            if (family == 7 && owner_.cinematics_->video_event(event))
                return true;
            return runtime(family).dispatch(event);
        }
        bool dispatch_menu_handler(frontend::MenuApplicationHandler handler,frontend::ControlEvent& event) override {
            if(handler==frontend::MenuApplicationHandler::profiles)return owner_.profiles_menu_->dispatch(event);
            if(handler==frontend::MenuApplicationHandler::mission)return owner_.single_player_->receive(event);
            if(handler==frontend::MenuApplicationHandler::multiplayer || handler==frontend::MenuApplicationHandler::connection)
                return owner_.multiplayer_->receive(event);
            return handler==frontend::MenuApplicationHandler::options && owner_.options_menu_->dispatch(event);
        }
        void quit() override {
            owner_.quit_(); // State status4, not an outer-loop running Boolean.
            std::puts("Controller menu requested status=4");
        }
        void select_application_state(std::uint32_t state) override {
            if (state != 12 && state != 2)
                throw std::logic_error("Unavailable menu application transition");
            owner_.select_state_(state);
            std::printf("Controller menu requested status=%u\n",state);
        }
        void set_application_input_enabled(bool value) override {
            owner_.startup_.application_input_enabled(value);
        }
        void set_input_motion_only(bool value) override { owner_.input_.inhibit_click = value; }
        void clear() { screens_.fill(nullptr); }

      private:
        MenuScreenEvents &runtime(std::uint32_t family) const {
            if (!screen_available(family))
                throw std::logic_error("Unavailable compiled menu screen");
            return *screens_[family];
        }
        ApplicationMenu &owner_;
        std::array<MenuScreenEvents *, 64> screens_{};
    };
    class KeyboardContext : public MenuKeyboardInput {
      public:
        explicit KeyboardContext(ApplicationMenu &owner)
            : MenuKeyboardInput(owner.input_, owner.products_.screen(0).focus(),
                                owner.products_.screen(1).focus(), *owner.events_, owner.confirming_),
              owner_(owner) {}
        bool key_current_screen(std::uint32_t word) {
            return owner_.current_product().definition().identifier == 0
                       ? owner_.dialog_->key(word)
                       : owner_.current_product().focus().key(word);
        }
        void end_current_drag(std::uint32_t target) {
            frontend::screen_drag_end(owner_.current_product().focus(), target);
        }
        std::uint32_t current_screen_identifier() const {
            return owner_.current_product().definition().identifier;
        }
        bool input_blocked() const { return owner_.events_->common_state().motion_only; }
        // Final table00815E30+44 →0067EA80 is an actual RET8 body.
        void preprocess_buffers(const std::uint32_t *, const std::uint32_t *) {}
        bool screens_available() const { return owner_.events_->screens().current() != nullptr; }
        bool wide_character_mode() const { return owner_.startup_.wide_characters(); }
        bool character_current_target(std::uint32_t value) const {
            return frontend::character_current_target(owner_.events_->common_state(), value);
        }
        void trace_wide_character(std::uint16_t value) const {
            char message[64];
            std::snprintf(message, sizeof(message), "KEY VALUE = %u\n", value);
            OutputDebugStringA(message);
        }
        void enqueue_debug_event() { owner_.events_->enqueue({0, 0x0200007d, 0, 0, 0, {}}); }

      private:
        ApplicationMenu &owner_;
    };
    std::uint32_t pick(std::uint32_t identifier, const CursorPoint &point) {
        if (identifier==0x24000000)
            if (auto* dropdown=multiplayer_->dropdown())
                if (dropdown->overlay_rectangle().contains(point))
                    return dropdown->pick_overlay(point.x,point.y);
        const bool confirmation = identifier == 0x10000000;
        auto &focus = current_product().focus();
        const auto picked =
            identifier == 0 ? scene_->pick_model(module_, point.x, point.y, 800, 600,
                                                 scene_->runtime_confirmation_frame())
            : confirmation  ? scene_->pick_confirmation(module_, point.x, point.y, 800, 600)
                : scene_->pick_model(module_,point.x,point.y,800,600,current_product().model_frame());
        return (confirmation || identifier == 0)
                   ? focus.target_for_control(std::max(0, focus.interactive_control_for_target(picked)))
                   : focus.interactive_target(picked);
    }
    void refresh_main() {
        frontend::refresh_main_menu(*events_,startup_.profile_name(),startup_.last_save_caption());
    }
    void date_tick(bool initial) {
        const auto now = std::time(nullptr);
        const auto *local = std::localtime(&now);
        if (!local)
            throw std::runtime_error("Menu local date/time unavailable");
        const bool changed = local->tm_hour != cached_hour_ || local->tm_min != cached_minute_;
        if (changed) {
            char value[96];
            std::snprintf(value, sizeof(value), "%d. %02d. %d  %02d:%02d", local->tm_mday, local->tm_mon + 1,
                          local->tm_year + 1900, local->tm_hour, local->tm_min);
            cached_date_ = value;
            cached_hour_ = local->tm_hour;
            cached_minute_ = local->tm_min;
        }
        if (initial || changed) {
            frontend::ControlEvent event{0x04300000, 0x01000014, 12, 0, 0, {}};
            event.text = std::make_shared<const std::string>(cached_date_);
            events_->enqueue(event);
        }
    }
    struct DialogServices {
        explicit DialogServices(ApplicationMenu &value) : owner(value) {}
        bool screen_key(std::uint32_t word) {
            if (!owner.runtime_product_)
                throw std::logic_error("Runtime dialog focus is unavailable");
            return owner.runtime_product_->focus().key(word);
        }
        std::uint32_t sender_identifier(void *sender) const {
            return static_cast<ButtonVisualPair *>(sender)->identifier();
        }
        ApplicationMenu &owner;
    };
    StartupSession &startup_;
    MenuDriver &driver_;
    HMODULE module_;
    Graphics *graphics_;
    HWND window_;
    BorrowedGraphicsInput &keyboard_;
    std::function<void()> quit_;
    std::function<void(std::uint32_t)> select_state_;
    std::unique_ptr<MenuScene> scene_;
    std::unique_ptr<CinematicsMenu> cinematics_;
    std::unique_ptr<OptionsMenu> options_menu_;
    std::unique_ptr<ProfilesMenu> profiles_menu_;
    std::unique_ptr<SinglePlayerMenu> single_player_;
    std::unique_ptr<MultiplayerMenu> multiplayer_;
    std::unique_ptr<MenuText<Graphics>> text_;
    std::unique_ptr<MenuCursor<Graphics>> cursor_;
    MenuInputSession input_;
    Actions actions_;
    frontend::OwnedScreens<MenuScreenProduct> products_;
    std::optional<frontend::MenuEventQueue> events_;
    std::optional<DialogServices> dialog_services_;
    std::unique_ptr<frontend::RuntimeConfirmation<DialogServices>> dialog_;
    std::optional<frontend::MenuScreenDefinition> dialog_definition_;
    std::unique_ptr<MenuScreenProduct> runtime_product_;
    std::optional<KeyboardContext> keyboard_context_;
    std::optional<frontend::OptionsSettings<StartupSession>> options_;
    std::unique_ptr<MenuWidgetContext> widgets_;
    std::optional<MenuFrameInput<MenuFocus>> pointer_input_;
    frontend::FrontendUpdateState update_state_;
    std::optional<CursorPoint> point_;
    bool confirming_ = false, rendered_ = false;
    int cached_hour_ = 0, cached_minute_ = 0;
    std::string cached_date_;
};

} // namespace hd2::host
#endif
