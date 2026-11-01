#ifndef HD2_HOST_PRE_MISSION_MENU_H
#define HD2_HOST_PRE_MISSION_MENU_H

#include "host/application_menu.h"
#include "host/pre_mission_description.h"
#include "host/pre_mission_equipment_menu.h"
#include "host/pre_mission_hero_menu.h"
#include "host/pre_mission_team_menu.h"

namespace hd2::host {

// Content owners supply actual definitions/control constructors. Every model is
// loaded through MenuScene's real resource service; no registry slot substitutes
// for a missing controller or content owner.
struct PreMissionMenuContent {
    struct Screen {
        frontend::MenuScreenDefinition definition;
        EngineFrame *model;
        bool modal = false;
    };
    std::shared_ptr<void> owner;
    frontend::MenuScreenDefinition primary;
    std::vector<Screen> additional;
    std::function<std::unique_ptr<MenuControl>(const frontend::MenuControlDefinition &, EngineFrame *,
                                               MenuTextObject &)>
        control;
    std::function<bool(frontend::ControlEvent &)> receive;
    std::function<void()> shown;
    std::shared_ptr<PreMissionDescription> description;
    std::function<void(unsigned)> tick = {};
    std::function<std::optional<MenuRectangle>()> text_mask = {};
};

class PreMissionMenu {
  public:
    using Builder = std::function<PreMissionMenuContent(PreMissionMenu &)>;
    PreMissionMenu(StartupSession &startup, MenuDriver &driver, HMODULE module, Graphics *graphics,
                   HWND window, BorrowedGraphicsInput &keyboard, Builder builder)
        : startup_(startup), driver_(driver), module_(module), graphics_(graphics), window_(window),
          keyboard_(keyboard), builder_(std::move(builder)), actions_(*this) {
        if (!builder_)
            throw std::invalid_argument("Setup menu requires its actual content builder");
    }
    ~PreMissionMenu() {
        frontend::CleanupErrors errors;
        errors.attempt([&] { shutdown(); });
    }
    void initialize(const frontend::GameDataNode &descriptor, std::function<void(unsigned)> status) {
        if (scene_ || (descriptor.kind != 6 && descriptor.kind != 7) ||
            startup_.pre_mission().descriptor() != &descriptor)
            throw std::logic_error("Setup menu descriptor/lifetime mismatch");
        const char *model = descriptor.kind == 7                               ? "main_menu"
                            : descriptor.word10 == 0                           ? "heros_screen"
                            : descriptor.word10 == 1 || descriptor.word10 == 2 ? "team members selection"
                            : descriptor.word10 == 3 || descriptor.word10 == 4 ? "items selection"
                                                                               : nullptr;
        if (!model)
            throw std::logic_error("Unsupported pre-mission screen subtype");
        initialize_model(model,std::move(status));
    }
    // The type8 connection owner has its own scene, queue and screen registry.
    // Reuse resource/input ownership without inserting overlapping slots into Main.
    void initialize_connection(std::function<void(unsigned)> status) {
        if (scene_) throw std::logic_error("Connection menu already initialized");
        initialize_model("confirm",std::move(status));
    }
  private:
    void initialize_model(const char* model,std::function<void(unsigned)> status) {
        status_ = std::move(status);
        frontend::load_menu_content(driver_, 10, [&] {
            scene_ = std::make_unique<MenuScene>(driver_, module_, true, true, &startup_.loaded_resources(),
                                                 true, model);
            scene_->initialize_confirmation_text_frame(module_);
            scene_->initialize_confirmation_text_frame(module_, scene_->runtime_confirmation_frame());
            text_ = std::make_unique<MenuText<Graphics>>(graphics_, module_, *scene_, false, true,
                                                         &startup_.text(), false);
            cursor_ = std::make_unique<MenuCursor<Graphics>>(graphics_, module_);
            queue_ = std::make_unique<frontend::MenuEventQueue>(actions_, false);
            settings_.emplace(startup_, [this] { scene_->request_click_sound(); });
            widgets_ = std::make_unique<MenuWidgetContext>(input_, *queue_, *settings_);
            dialog_services_ = std::make_unique<DialogServices>(DialogServices{*this});
            dialog_ =
                std::make_unique<frontend::RuntimeConfirmation<DialogServices>>(*dialog_services_, *queue_);
            dialog_definition_ = dialog_->definition();
            text_->attach_runtime_definition(*scene_, module_, *dialog_definition_);
            RuntimeMenuScreenFactory<Actions> factory(
                *scene_, module_, *dialog_definition_, [this](unsigned id) { return text_->object_for(id); },
                *queue_, runtime_dialog_, actions_);
            if (!frontend::construct_runtime_confirmation(factory))
                throw std::runtime_error("Setup dialog construction failed");
            runtime_dialog_->controls().bind_pick_targets();
            content_ = builder_(*this);
            if (!content_->owner || content_->primary.identifier != 0x04000000 ||
                std::strcmp(content_->primary.model, model))
                throw std::logic_error("Setup content does not match its actual descriptor model");
            if (content_->description) {
                text_->attach_screen_definition(*scene_, module_, content_->description->definition());
                content_->description->bind(*text_->object_for(0xfc100000),
                                            content_->description->definition().controls.size() > 1
                                                ? text_->object_for(0xfc200000)
                                                : nullptr);
            }
            add_screen(content_->primary, scene_->main_model_frame(), false);
            auto confirmation = frontend::confirmation_definition();
            add_screen(confirmation, scene_->confirmation_model_frame(), true);
            for (auto &screen : content_->additional)
                add_screen(screen.definition, screen.model, false);
            queue_->dispatch_immediate({0, 0x02000001, 1, 0, 0, {}});
        });
        keyboard_context_ = std::make_unique<Keyboard>(*this);
        pointer_.emplace(
            input_, queue_->common_state(), [this]() -> MenuFocus & { return current().focus(); },
            [this](MenuFocus &, const CursorPoint &point) { return pick(point); },
            [this](float x, float y) { scene_->update_input_cursor(x, y, 800, 600); },
            [this] { scene_->request_click_sound(); });
        const auto &position = startup_.menu_cursor();
        keyboard_.set_pointer_position(static_cast<int>(position[0]), static_cast<int>(position[1]));
        input_.pointer.cursor().initialize(position[0], position[1], 800, 600);
        input_.previous_point = CursorPoint{position[0], position[1]};
        input_.armed = true;
        scene_->update_input_cursor(position[0], position[1], 800, 600);
        if (startup_.sound_available())
            scene_->load_click_sound(module_);
    }
  public:
    void shutdown() {
        if (!scene_)
            return;
        if (input_.armed)
            startup_.menu_cursor(input_.pointer.cursor().x(), input_.pointer.cursor().y());
        pointer_.reset();
        keyboard_context_.reset();
        frontend::CleanupErrors errors;
        errors.attempt([&] {
            if (queue_)
                queue_->close();
        });
        products_.clear();
        runtime_dialog_.reset();
        dialog_.reset();
        dialog_definition_.reset();
        dialog_services_.reset();
        widgets_.reset();
        settings_.reset();
        queue_.reset();
        actions_.clear();
        input_.clear();
        text_.reset();
        cursor_.reset();
        content_.reset();
        scene_.reset();
        SetCursor(LoadCursorW(nullptr, IDC_ARROW));
        errors.rethrow();
    }
    void input(const unsigned *first, const unsigned *second) {
        frontend::preprocess_frontend_keyboard(*keyboard_context_, keyboard_, first, second);
    }
    void update(unsigned milliseconds) {
        FrameServices services{*this};
        frontend::update_frontend(update_, services, milliseconds);
    }
    void render() {
        auto &screen = current();
        const auto id = screen.definition().identifier;
        const bool additional_modal =
            std::any_of(content_->additional.begin(), content_->additional.end(),
                        [&](const auto &entry) { return entry.modal && entry.definition.identifier == id; });
        const bool modal = id == 0 || id == 0x10000000 || additional_modal;
        auto *overlay = additional_modal ? screen.model_frame()
                        : id == 0        ? scene_->runtime_confirmation_frame()
                                         : scene_->confirmation_model_frame();
        if (modal)
            scene_->set_screen_visible(module_, overlay, false);
        if (scene_->render() < 0)
            throw std::runtime_error("Setup scene render failed");
        auto *background = &screen;
        if (modal) {
            const auto *previous = queue_->screens().previous();
            if (!previous)
                throw std::logic_error("Setup dialog has no retained screen");
            background = &product(previous->identifier);
        }
        const auto mask=content_->text_mask?content_->text_mask():std::optional<MenuRectangle>{};
        if (mask)
            text_->draw_screen_masked(background->definition(), background->focus().focused_control(),*mask);
        else
            text_->draw_screen(background->definition(), background->focus().focused_control());
        if (modal) {
            std::vector<EngineFrame *> backgrounds;
            for (std::size_t i = 0; i < products_.size(); ++i)
                backgrounds.push_back(products_.screen(i).model_frame());
            MenuScene::ModalPass pass(*scene_, module_, overlay, std::move(backgrounds));
            if (scene_->render() < 0)
                throw std::runtime_error("Setup modal render failed");
            text_->draw_screen(screen.definition(), screen.focus().focused_control());
            if (!pass.restore())
                throw std::runtime_error("Setup modal render state restore failed");
        }
        if (content_->description && content_->description->visible()) {
            if (mask) text_->draw_screen_masked(content_->description->definition(), -1,*mask);
            else text_->draw_screen(content_->description->definition(), -1);
        }
        if (GetAncestor(GetForegroundWindow(), GA_ROOT) == window_ && !IsIconic(window_)) {
            cursor_->draw_at({input_.pointer.cursor().x(), input_.pointer.cursor().y()});
            SetCursor(nullptr);
        }
    }
    MenuScene &scene() { return *scene_; }
    frontend::MenuEventQueue &queue() { return *queue_; }
    MenuWidgetContext &widgets() { return *widgets_; }
    MenuTextObject *text(unsigned id) { return text_->object_for(id); }
    StartupSession &startup() { return startup_; }
    MenuDriver &driver() { return driver_; }
    HMODULE module() const { return module_; }
    void status(unsigned value) { status_(value); }
    void advance_setup() {
        auto &flow = startup_.pre_mission();
        if (flow.screen() != frontend::PreMissionScreen::game_unavailable) {
            status(2);
            return;
        }
        const auto &request = flow.game_request();
        if (!request || request->game_entry_available)
            throw std::logic_error("Unavailable game boundary requires a validated owned request");
        std::printf("Pre-mission handoff validated: mission=%s root=%u mode=%u standalone=%u soldiers=%u; "
                    "game entry unavailable\n",
                    request->mission_resource.c_str(), request->root, request->mode,
                    request->standalone ? 1u : 0u, static_cast<unsigned>(request->squad.size()));
        for (const auto &soldier : request->squad) {
            std::printf("Pre-mission squad: human=%d name=%s role=%d active=%u equipment=", soldier.human,
                        soldier.name.c_str(), soldier.specialization, soldier.active ? 1u : 0u);
            for (const auto &entry : soldier.equipment)
                std::printf(" %d:%d", entry.item, entry.count);
            std::printf(" holsters=%u backpack=%u load=%.3f capacity=%d\n",
                        static_cast<unsigned>(soldier.holsters.size()),
                        static_cast<unsigned>(soldier.backpack.size()), soldier.load, soldier.capacity);
        }
        unavailable_pending_ = true;
        // This explicit endpoint is the user-approved reconstruction boundary,
        // not an acknowledgment from original00698510/006989B0 world loading.
        prompt(2,
               std::make_shared<const std::string>(
                   "Mission setup is complete. Starting the game is unavailable in this build."),
               0x020000c7, 0x020000c7);
    }
    void confirm_back() {
        if (current().definition().identifier == 0x10000000) {
            queue_->cancel_confirmation();
            return;
        }
        queue_->enqueue({0, 0x02000002, 4, 0, 0, {}});
        queue_->enqueue({0, 0x22000007, 0, 0, 0, {}});
    }
    void prompt(unsigned mode, unsigned label, unsigned yes, unsigned no) {
        frontend::request_runtime_localized(dialog_.get(), mode, label, yes, no);
    }
    void prompt(unsigned mode, frontend::SharedText text, unsigned yes, unsigned no) {
        frontend::request_runtime_text(dialog_.get(), mode, std::move(text), yes, no);
    }
    static PreMissionMenuContent hero_content(PreMissionMenu &menu) {
        auto hero =
            std::make_shared<PreMissionHeroMenu>(menu.startup(), menu.scene(), menu.driver(), menu.module(),
                                                 menu.scene().main_model_frame(), menu.queue());
        hero->transitions([&menu] { menu.advance_setup(); }, [&menu] { menu.confirm_back(); });
        PreMissionMenuContent content{hero, hero->definition(), {}, {}, {}, [hero] {hero->shown(); }, {}};
        content.control = [hero, &menu](const frontend::MenuControlDefinition &definition, EngineFrame *frame,
                                        MenuTextObject &text) {
            return hero->make_control(definition, frame, text, menu.widgets(),
                                      [&menu](unsigned id) { return menu.text(id); });
        };
        content.receive = [&menu](frontend::ControlEvent &event) {
            if ((event.code & 0x03000fffu) != 0x0200000a)
                return false;
            menu.confirm_back();
            return true;
        };
        return content;
    }
    static PreMissionMenuContent team_content(PreMissionMenu &menu) {
        auto team =
            std::make_shared<PreMissionTeamMenu>(menu.startup(), menu.scene(), menu.driver(), menu.module(),
                                                 menu.scene().main_model_frame(), menu.queue());
        team->transitions([&menu] { menu.advance_setup(); }, [&menu] { menu.confirm_back(); });
        PreMissionMenuContent content{team, team->definition(), {}, {}, {}, [team] {team->shown(); }, {}};
        content.description = std::make_shared<PreMissionDescription>(menu.scene(), menu.module(), true);
        team->description(content.description);
        content.text_mask=[team] { return team->drag_rectangle(); };
        content.control = [team, &menu](const frontend::MenuControlDefinition &definition, EngineFrame *frame,
                                        MenuTextObject &text) {
            return team->make_control(definition, frame, text, menu.widgets(),
                                      [&menu](unsigned id) { return menu.text(id); });
        };
        content.receive = [team](frontend::ControlEvent &event) { return team->receive(event); };
        content.additional.push_back({team->selection().definition(), team->selection().model(), true});
        return content;
    }
    static PreMissionMenuContent unavailable_content(PreMissionMenu &menu) {
        auto shown = std::make_shared<bool>(false);
        frontend::MenuScreenDefinition definition{
            "GAME ENTRY",
            0x04000000,
            "main_menu",
            {frontend::text_definition(0x04100000, "text_screen", {frontend::caption_definition(10, 2101)}),
             frontend::text_definition(0x05400000, "text_h&dversion",
                                       {frontend::caption_definition(40, 0x7d6)})}};
        for (const auto &control : frontend::main_menu_definition().controls)
            if (control.frame && std::strcmp(control.frame, "text_screen") &&
                std::strcmp(control.frame, "text_h&dversion"))
                menu.scene().set_screen_visible(menu.module(),
                                                menu.scene().control_frame(control.frame, menu.module(),
                                                                           false,
                                                                           menu.scene().main_model_frame()),
                                                false);
        PreMissionMenuContent content{shown,
                                      std::move(definition),
                                      {},
                                      {},
                                      {},
                                      [shown, &menu] {
                                          if (!*shown) {
                                              *shown = true;
                                              menu.advance_setup();
                                          }
                                      },
                                      {}};
        return content;
    }
    static PreMissionMenuContent equipment_content(PreMissionMenu &menu) {
        auto equipment = std::make_shared<PreMissionEquipmentMenu>(
            menu.startup(), menu.scene(), menu.driver(), menu.module(), menu.scene().main_model_frame(),
            menu.queue());
        equipment->transitions([&menu] { menu.advance_setup(); }, [&menu] { menu.confirm_back(); },
                               [&menu](unsigned mode, frontend::SharedText text, unsigned yes, unsigned no) {
                                   menu.prompt(mode, std::move(text), yes, no);
                               });
        PreMissionMenuContent content{
            equipment, equipment->definition(), {}, {}, {}, [equipment] {equipment->shown(); }, {}};
        content.description = std::make_shared<PreMissionDescription>(menu.scene(), menu.module());
        equipment->description(content.description);
        content.control = [equipment, &menu](const frontend::MenuControlDefinition &definition,
                                             EngineFrame *frame, MenuTextObject &text) {
            return equipment->make_control(definition, frame, text, menu.widgets(),
                                           [&menu](unsigned id) { return menu.text(id); });
        };
        content.receive = [equipment](frontend::ControlEvent &event) { return equipment->receive(event); };
        content.additional.push_back(
            {equipment->selection().definition(), equipment->selection().model(), true});
        return content;
    }

  private:
    void add_screen(frontend::MenuScreenDefinition &definition, EngineFrame *model, bool confirmation) {
        if (!model)
            throw std::runtime_error("Setup definition has no actual loaded model");
        if (confirmation)
            scene_->position_menu_model(module_, model, .5f, -.375f, -.5f);
        scene_->update_screen_frame(module_, model);
        text_->attach_screen_definition(*scene_, module_, definition);
        auto screen = std::make_unique<MenuScreenProduct>(
            *scene_, module_, confirmation,
            std::vector<frontend::MenuControlRole>{
                frontend::MenuControlRole::single_player, frontend::MenuControlRole::yes,
                frontend::MenuControlRole::no, frontend::MenuControlRole::ok,
                frontend::MenuControlRole::multiplayer},
            [this](unsigned id) { return text_->object_for(id); }, *queue_, model, widgets_.get(),
            confirmation ? decltype(content_->control){} : content_->control);
        if (!screen->bind_definition(definition))
            throw std::runtime_error("Setup screen binding failed");
        screen->initialize_records(definition.initialization);
        screen->controls().bind_pick_targets();
        actions_.bind_screen(definition.identifier >> 26, screen->events());
        products_.append({definition.label, definition.model, definition.identifier}, std::move(screen));
    }
    MenuScreenProduct &product(unsigned id) {
        if (!id)
            return *runtime_dialog_;
        for (std::size_t i = 0; i < products_.size(); ++i)
            if (products_.screen(i).definition().identifier == id)
                return products_.screen(i);
        throw std::logic_error("Setup screen has no bound runtime product");
    }
    MenuScreenProduct &current() {
        const auto *screen = queue_->screens().current();
        if (!screen)
            throw std::logic_error("Setup screen stack is empty");
        return product(screen->identifier);
    }
    unsigned pick(const CursorPoint &point) {
        auto &screen = current();
        const auto target = scene_->pick_model(module_, point.x, point.y, 800, 600, screen.model_frame());
        const auto id = screen.definition().identifier;
        return id == 0 || id == 0x10000000 ? screen.focus().target_for_control(std::max(
                                                 0, screen.focus().interactive_control_for_target(target)))
                                           : screen.focus().interactive_target(target);
    }
    class Actions final : public frontend::MenuEventActions {
      public:
        explicit Actions(PreMissionMenu &menu) : menu_(menu) {}
        void bind_screen(unsigned index, MenuScreenEvents &screen) {
            if (index >= screens_.size() || screens_[index])
                throw std::logic_error("Duplicate setup registration");
            screens_[index] = &screen;
        }
        bool screen_available(unsigned index) const override {
            return index < screens_.size() && screens_[index];
        }
        void activate_screen(const frontend::ScreenRegistration &screen, unsigned focus) override {
            required(screen.identifier >> 26).activate(focus);
            menu_.confirming_ = screen.identifier == 0x10000000;
            if (screen.identifier == 0x04000000 && menu_.content_->shown)
                menu_.content_->shown();
        }
        void deactivate_screen(const frontend::ScreenRegistration &screen) override {
            required(screen.identifier >> 26).deactivate();
            if (screen.identifier == 0x10000000)
                menu_.confirming_ = false;
        }
        bool dispatch_screen_event(unsigned index, frontend::ControlEvent &event) override {
            return required(index).dispatch(event);
        }
        bool dispatch_menu_handler(frontend::MenuApplicationHandler handler,
                                   frontend::ControlEvent &event) override {
            if (handler != frontend::MenuApplicationHandler::mission &&
                handler != frontend::MenuApplicationHandler::connection)
                return false;
            if (menu_.unavailable_pending_ && (event.code & 0x03000fffu) == 0x020000c7) {
                menu_.unavailable_pending_ = false;
                menu_.status(3); // Kind7 has no previous setup entry: controller returns to Main.
                return true;
            }
            return menu_.content_->receive && menu_.content_->receive(event);
        }
        void quit() override { menu_.status(4); }
        void select_application_state(unsigned value) override { menu_.status(value); }
        void set_application_input_enabled(bool value) override {
            menu_.startup_.application_input_enabled(value);
        }
        void set_input_motion_only(bool value) override { menu_.input_.inhibit_click = value; }
        void clear() { screens_.fill(nullptr); }

      private:
        MenuScreenEvents &required(unsigned index) {
            if (!screen_available(index))
                throw std::logic_error("Setup screen is unavailable");
            return *screens_[index];
        }
        PreMissionMenu &menu_;
        std::array<MenuScreenEvents *, 64> screens_{};
    };
    class Keyboard : public MenuKeyboardInput {
      public:
        explicit Keyboard(PreMissionMenu &menu)
            : MenuKeyboardInput(menu.input_, menu.products_.screen(0).focus(),
                                menu.products_.screen(1).focus(), *menu.queue_, menu.confirming_),
              menu_(menu) {}
        bool key_current_screen(unsigned word) {
            return current_screen_identifier() == 0 ? menu_.dialog_->key(word)
                                                    : menu_.current().focus().key(word);
        }
        void end_current_drag(unsigned target) { frontend::screen_drag_end(menu_.current().focus(), target); }
        unsigned current_screen_identifier() const { return menu_.current().definition().identifier; }
        bool input_blocked() const { return menu_.queue_->common_state().motion_only; }
        void preprocess_buffers(const unsigned *, const unsigned *) {} // 0067EA80 RET8.
        bool screens_available() const { return menu_.queue_->screens().current() != nullptr; }
        bool wide_character_mode() const { return menu_.startup_.wide_characters(); }
        bool character_current_target(unsigned value) const {
            return frontend::character_current_target(menu_.queue_->common_state(), value);
        }
        void trace_wide_character(std::uint16_t value) const {
            char text[40];
            std::snprintf(text, sizeof(text), "KEY VALUE = %u\n", value);
            OutputDebugStringA(text);
        }
        void enqueue_debug_event() { menu_.queue_->enqueue({0, 0x0200007d, 0, 0, 0, {}}); }

      private:
        PreMissionMenu &menu_;
    };
    struct DialogServices {
        PreMissionMenu &menu;
        bool screen_key(unsigned word) { return menu.runtime_dialog_->focus().key(word); }
        unsigned sender_identifier(void *sender) const {
            return static_cast<ButtonVisualPair *>(sender)->identifier();
        }
    };
    struct FrameServices {
        PreMissionMenu &menu;
        void update_menu(unsigned milliseconds) {
            if (menu.content_->tick) menu.content_->tick(milliseconds);
            if (menu.content_->description)
                menu.content_->description->update(menu.input_,
                                                   menu.current().definition().identifier == 0x04000000);
        }
        void update_sound(unsigned milliseconds) { menu.startup_.update_sound(milliseconds); }
        void pump_events() { menu.queue_->pump(); }
        bool screens_available() const { return menu.queue_->screens().current() != nullptr; }
        void update_pointer(unsigned milliseconds) {
            menu.pointer_->tick_relative(
                static_cast<float>(menu.startup_.mouse_x()), static_cast<float>(menu.startup_.mouse_y()),
                menu.startup_.mouse_buttons(), milliseconds, menu.startup_.mouse_wheel());
        }
        void update_capture(unsigned milliseconds) {
            menu.queue_->common_state().update_capture(milliseconds);
        }
        void update_current_screen(unsigned milliseconds) { menu.current().events().update(milliseconds); }
        void update_overlay(unsigned milliseconds) {
            menu.queue_->common_state().update_overlay(milliseconds);
        }
        std::size_t listener_count() const { return menu.queue_->common_state().notification_tokens.size(); }
        void update_listener(std::size_t index, unsigned milliseconds) {
            menu.queue_->common_state().update_listener(index, milliseconds);
        }
    };
    StartupSession &startup_;
    MenuDriver &driver_;
    HMODULE module_;
    Graphics *graphics_;
    HWND window_;
    BorrowedGraphicsInput &keyboard_;
    Builder builder_;
    Actions actions_;
    std::function<void(unsigned)> status_;
    std::unique_ptr<MenuScene> scene_;
    std::unique_ptr<MenuText<Graphics>> text_;
    std::unique_ptr<MenuCursor<Graphics>> cursor_;
    std::unique_ptr<frontend::MenuEventQueue> queue_;
    std::optional<frontend::OptionsSettings<StartupSession>> settings_;
    std::unique_ptr<MenuWidgetContext> widgets_;
    std::optional<PreMissionMenuContent> content_;
    frontend::OwnedScreens<MenuScreenProduct> products_;
    std::unique_ptr<MenuScreenProduct> runtime_dialog_;
    std::unique_ptr<DialogServices> dialog_services_;
    std::unique_ptr<frontend::RuntimeConfirmation<DialogServices>> dialog_;
    std::optional<frontend::MenuScreenDefinition> dialog_definition_;
    MenuInputSession input_;
    std::unique_ptr<Keyboard> keyboard_context_;
    std::optional<MenuFrameInput<MenuFocus>> pointer_;
    frontend::FrontendUpdateState update_;
    bool confirming_ = false, unavailable_pending_ = false;
};

} // namespace hd2::host
#endif
