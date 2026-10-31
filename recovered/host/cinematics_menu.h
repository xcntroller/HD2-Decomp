#ifndef HD2_HOST_CINEMATICS_MENU_H
#define HD2_HOST_CINEMATICS_MENU_H
#include "frontend/cinematics.h"
#include "frontend/menu_frame_clone.h"
#include "host/engine_video.h"
#include "host/menu_control.h"
#include "host/startup_session.h"
#include <deque>

namespace hd2::host {
// Polymorphic clone/copy slots belong to several original engine classes. The
// surrounding launcher fingerprints the DLL; verify each reached virtual target
// belongs to that executable module instead of guessing a common class RVA.
template <class Function>
Function cinematics_engine_method(EngineFrame *frame, HMODULE module, unsigned slot) {
    if (!frame || !frame->methods)
        throw std::runtime_error("Null Cinematics engine interface");
    const auto address = frame->methods[slot / 4];
    MEMORY_BASIC_INFORMATION region{};
    if (!VirtualQuery(reinterpret_cast<const void *>(address), &region, sizeof(region)) ||
        region.AllocationBase != module ||
        !(region.Protect &
          (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)))
        throw std::runtime_error("Cinematics virtual target is outside the original engine");
    return reinterpret_cast<Function>(address);
}
struct CinematicsCloneBackend {
    using Frame = EngineFrame *;
    using Owner = MenuDriver::Owner;
    MenuDriver &driver;
    HMODULE module;
    Owner create_like(Frame source) {
        std::uint16_t kind = 0;
        std::memcpy(&kind, reinterpret_cast<const char *>(source) + 0xf8, 2);
        if (kind != 1)
            return driver.create(kind);
        using Type = std::uint32_t(WINAPI *)(Frame);
        using Create = Frame(WINAPI *)(Frame, std::uint32_t);
        const auto type = cinematics_engine_method<Type>(source, module, 4)(source);
        auto *frame = cinematics_engine_method<Create>(driver.frame(), module, 0x50)(driver.frame(), type);
        if (!frame)
            throw std::runtime_error("Cinematics visual clone factory failed");
        return Owner(frame, scene_method<MenuDriver::Release>(frame, module, 0, 0x1c370));
    }
    void copy(Frame destination, Frame source) {
        using Copy = void(WINAPI *)(Frame, Frame);
        cinematics_engine_method<Copy>(destination, module, 0x4c)(destination, source);
    }
    void link(Frame frame, Frame parent, std::uint32_t mode) {
        using Link = std::int32_t(WINAPI *)(Frame, Frame, std::uint32_t);
        if (scene_method<Link>(frame, module, 0x38, 0x1d680)(frame, parent, mode) < 0)
            throw std::runtime_error("Cinematics frame link failed");
    }
    unsigned child_count(Frame frame) {
        using Count = unsigned(WINAPI *)(Frame);
        return scene_method<Count>(frame, module, 0x3c, 0x1db80)(frame);
    }
    Frame child(Frame frame, unsigned index) {
        using Child = Frame(WINAPI *)(Frame, unsigned);
        return scene_method<Child>(frame, module, 0x40, 0x1dba0)(frame, index);
    }
};

class CinematicsGrid final : public MenuControl {
  public:
    CinematicsGrid(const MenuScene &scene, HMODULE module, const frontend::MenuControlDefinition &definition,
                   EngineFrame *frame, MenuTextObject &text, MenuWidgetContext &context,
                   std::function<MenuTextObject *(std::uint32_t)> lookup, std::array<float, 3> start)
        : MenuControl(scene, module, false, definition, frame, text, true, &context), scene_(scene),
          module_(module), definition_(definition), frame_(frame), context_(context),
          lookup_(std::move(lookup)), start_(start) {}
    bool initialize_visuals() override {
        if (definition_.children.size() != 10)
            throw std::logic_error("Cinematics requires the original ten row identities");
        for (unsigned i = 0; i < 10; ++i) {
            const auto &definition = definition_.children[i];
            auto *frame = scene_.control_frame(definition.frame, module_, false, frame_);
            auto row = std::make_unique<MenuControl>(scene_, module_, false, definition, frame,
                                                     *lookup_(definition.identifier), true, &context_);
            row->initialize_visuals();
            row->on_row_activation([this, i] {
                if (auto previous = list_.selected(); previous && *previous != i) {
                    rows_.at(*previous)->deselect_row();
                    rows_.at(*previous)->leave_focus(1);
                }
                list_.select(i);
            });
            row->initialize_events(definition.initialization);
            row_frames_.push_back(frame);
            rows_.push_back(std::move(row));
            list_.include(definition.identifier);
        }
        selector_ = scene_.find_control_frame(frame_, "selector*", module_);
        if (selector_)
            throw std::runtime_error("Unexpected selector in the generated Cinematics group");
        return true;
    }
    void initialize_events(const std::vector<frontend::ControlEvent> &events) override {
        for (auto event : events)
            receive_event(event, context_.queue);
    }
    bool receive_event(frontend::ControlEvent &event, frontend::ControlEventDestination &upstream) override {
        for (unsigned i = 0; i < rows_.size(); ++i)
            if ((event.target & 0x000ff000u) == (frontend::CinematicsList::row_identifier(i) & 0x000ff000u))
                return rows_[i]->receive_event(event, upstream);
        const auto code = event.code & 0x03000fffu;
        if ((event.code & 0x040ff000u) == 0) {
            if (code == 0x02000085) {
                if (auto selected = list_.selected())
                    rows_[*selected]->deselect_row();
                list_.clear();
                for (unsigned i = 0; i < rows_.size(); ++i)
                    visible(i, false);
                refresh();
                return true;
            }
            if (code == 0x02000083) {
                list_.include(event.argument);
                refresh();
                return true;
            }
            if (code == 0x0200002c) {
                refresh();
                return true;
            }
            if (code == 0x0100000b) {
                properties_.word_58 = event.argument;
                return true;
            }
            if (code == 0x01000028) {
                if (event.argument != 17)
                    throw std::logic_error("Cinematics page size changed");
                return true;
            }
            if (code == 0x0100002a) {
                if (event.word_0c != 0x3d03126f)
                    throw std::logic_error("Cinematics row spacing changed");
                return true;
            }
            if (code == 0x0200002b) {
                if (event.argument)
                    throw std::out_of_range("Ten Cinematics rows cannot scroll beyond their17-slot page");
                return true;
            }
            if (code == 0x020000aa) {
                if (event.argument < list_.active().size())
                    focus(list_.active()[event.argument]);
                return true;
            }
        }
        return state_event(event, upstream);
    }
    bool dispatch_control_event(frontend::ControlEvent &event, frontend::ControlEventDestination &upstream) {
        return receive_event(event, upstream);
    }
    bool eligible() const override { return properties_.eligible(); }
    bool disabled() const override { return properties_.disabled != 0; }
    bool preserves_part_targets() const override { return true; }
    void bind_pick_target(std::uint32_t value) override {
        if (value != frontend::CinematicsList::identifier)
            throw std::logic_error("Cinematics list ID changed");
        std::memcpy(reinterpret_cast<char *>(frame_) + 0xec, &value, 4);
    }
    void update(std::uint32_t delta) override {
        if (auto current = list_.focused())
            rows_[*current]->update(delta);
    }
    int enter_focus(std::uint32_t target) override {
        auto index = list_.target_index(target);
        if (!index)
            index = list_.focused();
        if (!index && !list_.active().empty())
            index = list_.active().front();
        return index ? focus(*index) : 0;
    }
    int leave_focus(std::uint32_t) override {
        if (auto current = list_.focused())
            rows_[*current]->leave_focus(frontend::CinematicsList::row_identifier(*current));
        list_.leave();
        return 1;
    }
    int pointer_focus(std::uint32_t target) override {
        const auto index = list_.target_index(target);
        if (!index)
            return leave_focus(target);
        if (list_.focused() == index)
            return rows_[*index]->pointer_focus(target);
        if (!leave_focus(target))
            return 1;
        return enter_focus(target);
    }
    int pointer_target(frontend::ScreenPointerCallback callback, std::uint32_t target,
                       frontend::ControlEventDestination &events) override {
        // Shared container callbacks forward to the focused child. Only release
        // requires the freshly picked row to match that child (00656CD0).
        const auto current = list_.focused();
        if (!current)
            return callback == frontend::ScreenPointerCallback::double_click ? 7 : 0;
        if (callback == frontend::ScreenPointerCallback::release && list_.target_index(target) != current)
            return 0;
        return rows_[*current]->pointer_target(callback, target, events);
    }
    int click(frontend::ControlEventDestination &events) override {
        return list_.focused() ? rows_[*list_.focused()]->click(events) : 0;
    }
    int activate(frontend::ControlEventDestination &events) override {
        return list_.focused() ? rows_[*list_.focused()]->activate(events) : 0;
    }
    int deactivate(frontend::ControlEventDestination &events) override {
        return list_.focused() ? rows_[*list_.focused()]->deactivate(events) : 0;
    }
    int wheel_target(std::int32_t delta, frontend::ControlEventDestination &) override {
        // 00655CC0 forwards rounded wheel steps immediately to the actual linked
        //  slider. Its negative original range disables it for the ten-row list.
        const auto count =
            (static_cast<std::uint64_t>(delta < 0 ? -static_cast<std::int64_t>(delta) : delta) + 119) / 120;
        for (std::uint64_t i = 0; i < count; ++i)
            context_.queue.dispatch_immediate(
                {properties_.word_58, delta > 0 ? 0x01000023u : 0x01000024u, 0, 0, 0, {}});
        return 1;
    }
    bool key(std::uint32_t word) override {
        const auto key = word & 0xffu;
        if (key == 200 || key == 203 || key == 208 || key == 205) {
            auto next = list_.adjacent(key == 208 || key == 205);
            return next && focus(*next);
        }
        if (key == 199 || key == 207 || key == 201 || key == 209) {
            if (!list_.active().empty())
                focus((key == 199 || key == 201) ? list_.active().front() : list_.active().back());
            return true;
        }
        return false;
    }
    bool has_text() const { return false; }
    void set_text_disabled(std::uint8_t) { throw std::logic_error("Cinematics group has no text object"); }
    void set_frame_visible(std::uint8_t value) { set_visible(frame_, value); }
    void update_frame() { update(frame_); }
    std::size_t child_count() const { return list_.active().size(); }
    void dispatch_child(std::size_t index, frontend::ControlEvent &event) {
        rows_[list_.active().at(index)]->receive_event(event, context_.queue);
    }

  private:
    bool state_event(frontend::ControlEvent &event, frontend::ControlEventDestination &upstream) {
        frontend::EmittingControlDestination<CinematicsGrid> destination{*this, &upstream};
        frontend::ControlStateRuntime<CinematicsGrid> state{*this, properties_, bindings_, destination};
        return state.dispatch(event);
    }
    void set_visible(EngineFrame *frame, std::uint8_t value) {
        using Set = void(WINAPI *)(EngineFrame *, std::uint8_t);
        scene_method<Set>(frame, module_, 0x30, 0x1cfe0)(frame, value);
    }
    void update(EngineFrame *frame) {
        using Update = void(WINAPI *)(EngineFrame *);
        scene_method<Update>(frame, module_, 0x24, 0x1e610)(frame);
    }
    void position(EngineFrame *frame, const std::array<float, 3> &value) {
        using Set = void(WINAPI *)(EngineFrame *, const float *);
        scene_method<Set>(frame, module_, 8, 0x1c9f0)(frame, value.data());
        update(frame);
    }
    void visible(unsigned index, bool value) {
        frontend::ControlEvent event{0, value ? 0x01000003u : 0x01000004u, 0, 0, 0, {}};
        rows_[index]->receive_event(event, context_.queue);
    }
    int focus(unsigned index) {
        if (!list_.contains(index))
            return 0;
        if (auto previous = list_.focused(); previous && *previous != index)
            rows_[*previous]->leave_focus(frontend::CinematicsList::row_identifier(*previous));
        list_.focus(index);
        return rows_[index]->enter_focus(frontend::CinematicsList::row_identifier(index));
    }
    void refresh() {
        unsigned ordinal = 0;
        for (unsigned i = 0; i < rows_.size(); ++i) {
            const bool active = list_.contains(i);
            visible(i, active);
            if (active) {
                auto value = start_;
                value[1] = static_cast<float>(static_cast<long double>(start_[1]) -
                                              static_cast<long double>(ordinal++) * .032f);
                position(row_frames_[i], value);
                // Moving the frame invalidates the cached world-space caption bounds.
                frontend::ControlEvent layout{0, 0x0200002c, 0, 0, 0, {}};
                rows_[i]->receive_event(layout, context_.queue);
            }
        }

        context_.queue.enqueue(
            {properties_.word_58, 0x01000020, static_cast<std::uint32_t>(list_.range()), 0, 0, {}});
        context_.queue.enqueue({properties_.word_58, 0x0200002b, 0, 0, 0, {}});
    }
    const MenuScene &scene_;
    HMODULE module_;
    const frontend::MenuControlDefinition &definition_;
    EngineFrame *frame_;
    MenuWidgetContext &context_;
    std::function<MenuTextObject *(std::uint32_t)> lookup_;
    std::array<float, 3> start_;
    std::vector<std::unique_ptr<MenuControl>> rows_;
    std::vector<EngineFrame *> row_frames_;
    frontend::CinematicsList list_;
    frontend::ControlProperties properties_;
    frontend::ControlEventBindings bindings_;
    EngineFrame *selector_ = nullptr;
};

class CinematicsMenu {
  public:
    CinematicsMenu(StartupSession &startup, MenuScene &scene, MenuDriver &driver, HMODULE module,
                   Graphics *graphics, frontend::MenuEventQueue &queue)
        : startup_(startup), scene_(scene), driver_(driver), module_(module), graphics_(graphics),
          queue_(queue), model_(scene.load_screen_model(module, "single mission 2")),
          group_(driver.create(6)), preview_(startup.graphics(), module) {
        clone_.emplace(CinematicsCloneBackend{driver_, module_});
    }
    ~CinematicsMenu() { preview_.close(false); }
    EngineFrame *model() const { return model_.get(); }
    const std::string &filename() const { return filename_; }
    void bind(std::function<void(std::uint32_t)> select) { select_state_ = std::move(select); }
    frontend::MenuScreenDefinition definition() {
        if (built_)
            throw std::logic_error("Cinematics definition built twice");
        built_ = true;
        auto *prototype = scene_.control_frame("blong", module_, false, model_.get());
        auto *short_row = scene_.control_frame("bshort", module_, false, model_.get());
        std::memcpy(start_.data(), reinterpret_cast<const char *>(prototype) + 0xb0, 12);
        using Name = void(WINAPI *)(EngineFrame *, const char *);
        cinematics_engine_method<Name>(group_.get(), module_, 0x34)(group_.get(), "GB_Cinematics");
        clone_->link(group_.get(), model_.get(), 0);
        scene_.position_menu_model(module_, group_.get(), 0, 0, 0);
        clone_->link(prototype, group_.get(), 1);
        show(prototype, false);
        show(short_row, false);
        auto *video = scene_.control_frame("video", module_, false, model_.get());
        show(video, false);
        if (auto *text = scene_.find_control_frame(model_.get(), "text00", module_))
            show(text, false);
        // 006141AF: clear the screenshot's shared material, not a replacement picture.
        auto *screenshot = scene_.control_frame("screen_shot", module_, false, model_.get());
        using Unary = void(WINAPI *)(EngineFrame *);
        cinematics_engine_method<Unary>(screenshot, module_, 0xa0)(screenshot);
        EngineFrame *material = nullptr;
        std::memcpy(&material, reinterpret_cast<const char *>(screenshot) + 0x1e0, sizeof(material));
        cinematics_engine_method<Unary>(material, module_, 0x18)(material);
        frontend::MenuScreenDefinition result{"cinematics", 0x1c000000, "single mission 2", {}};
        const auto control = [&](unsigned ordinal, const char *frame, frontend::MenuControlKind kind,
                                 frontend::MenuControlRole role = frontend::MenuControlRole::none) {
            return frontend::MenuControlDefinition{
                0x1c000000 | (ordinal << 20),
                frame,
                kind,
                kind == frontend::MenuControlKind::button ? "normal*" : nullptr,
                kind == frontend::MenuControlKind::button ? "actived*" : nullptr,
                role,
                {}};
        };
        result.controls.push_back(control(1, "text_screen", frontend::MenuControlKind::text));
        result.controls.back().initialization = {frontend::caption_definition(10, 2109)};
        result.controls.push_back(control(2, "screen_shot", frontend::MenuControlKind::image));
        result.controls.push_back(control(3, "video", frontend::MenuControlKind::video));
        result.controls.push_back(
            control(4, "bexit01", frontend::MenuControlKind::button, frontend::MenuControlRole::play));
        result.controls.back().initialization = {
            frontend::caption_definition(13, 2264),
            {0, 0x01000001, 0, 0, 0, {}},
            frontend::observer_control_event(0, 0x04001000, this, &play)};
        result.controls.push_back(
            control(5, "bexit", frontend::MenuControlKind::button, frontend::MenuControlRole::back));
        result.controls.back().initialization = {frontend::caption_definition(13, 2256),
                                                 {0, 0x06001003, 0, 0, 0, {}}};
        result.controls.push_back(control(6, "scroll00", frontend::MenuControlKind::slider));
        result.controls.back().initialization = {{0, 0x0100000b, 0x1c700000, 0, 0, {}}};
        result.controls.push_back(control(7, "GB_Cinematics", frontend::MenuControlKind::movie_list,
                                          frontend::MenuControlRole::movie_list));
        auto &list = result.controls.back();
        list.initialization = {{0, 0x0100000b, 0x1c600000, 0, 0, {}},
                               {0, 0x01000028, 17, 0, 0, {}},
                               {0, 0x0100002a, 0, 0x3d03126f, 0, {}}};
        const auto &movies = startup_.profile_movies();
        if (movies.size() != 10)
            throw std::logic_error("Original profile movie catalog must contain ten entries");
        for (unsigned i = 0; i < movies.size(); ++i) {
            char name[20];
            std::snprintf(name, sizeof(name), "Anim_%02u", i);
            row_names_.emplace_back(name);
            auto frame = frontend::clone_menu_frame(*clone_, prototype, group_.get());
            cinematics_engine_method<Name>(frame.get(), module_, 0x34)(frame.get(),
                                                                       row_names_.back().c_str());
            show(frame.get(), false);
            row_frames_.push_back(std::move(frame));
            list.children.push_back({frontend::CinematicsList::row_identifier(i),
                                     row_names_.back().c_str(),
                                     frontend::MenuControlKind::movie_row,
                                     "normal*",
                                     "actived*",
                                     frontend::MenuControlRole::none,
                                     {frontend::caption_definition(12, movies[i].caption),
                                      {0, 0x01000009, i, 0, 0, {}},
                                      frontend::observer_control_event(0, 0x04001000, this, &select)}});
        }
        result.controls.push_back(control(8, "text_list of profiles", frontend::MenuControlKind::text));
        result.controls.back().initialization = {frontend::caption_definition(11, 159)};
        result.controls.push_back(control(9, "text_h&dversion", frontend::MenuControlKind::text));
        result.controls.back().initialization = {frontend::caption_definition(40, 2006)};
        result.controls.push_back(control(10, "bload lastsave", frontend::MenuControlKind::button));
        result.controls.back().initialization = {{0, 0x01000004, 0, 0, 0, {}}};
        result.controls.push_back(control(11, "bload mission", frontend::MenuControlKind::button));
        result.controls.back().initialization = {frontend::caption_definition(13, 2256),
                                                 {0, 0x01000004, 0, 0, 0, {}}};
        result.initialization = {frontend::observer_control_event(0, 0x0400c000, this, &hidden),
                                 frontend::observer_control_event(0, 0x0400d000, this, &shown)};
        return result;
    }
    std::unique_ptr<MenuControl> make_control(const frontend::MenuControlDefinition &definition,
                                              EngineFrame *frame, MenuTextObject &text,
                                              MenuWidgetContext &widgets,
                                              std::function<MenuTextObject *(std::uint32_t)> lookup) {
        if (definition.kind == frontend::MenuControlKind::movie_list)
            return std::make_unique<CinematicsGrid>(scene_, module_, definition, frame, text, widgets,
                                                    std::move(lookup), start_);
        if (definition.kind == frontend::MenuControlKind::video) {
            preview_.create();
            preview_frame_ = frame;
        }
        return {};
    }
    bool video_event(frontend::ControlEvent &event) {
        if (event.target != 0x1c300000 || (event.code & 0x040ff000u))
            return false;
        switch (event.code & 0x03000fffu) {
        case 0x020000a0:
            preview_.close_stream();
            return true;
        case 0x020000a1:
            preview_.stop();
            return true;
        case 0x020000a2: {
            if (!event.text)
                throw std::logic_error("Cinematics preview requires owned filename text");
            //00667EA0 formats GameMovie\\%s2; PLAY keeps the unsuffixed filename.
            const auto preview_filename = *event.text + "2";
            preview_.open(preview_filename.c_str(), 0);
            // Original00667EA0 also fits after a failed open, consuming undefined
            // dimensions. Keep failed streams closed instead of using that data.
            if (preview_.opened()) {
                fit_preview();
                preview_.volume(0);
                preview_.play();
            }
            return true;
        }
        default:
            return false;
        }
    }
    void render_preview() {
        if (!preview_.opened())
            return;
        preview_.backdrop(0);
        const auto driver_state = driver_.render_state(3);
        driver_.set_render_state(3, 0);
        using Get = std::uint32_t(WINAPI *)(EngineFrame *, std::uint32_t);
        using Set = std::int32_t(WINAPI *)(EngineFrame *, std::uint32_t, std::uint32_t);
        const auto graph_state =
            scene_method<Get>(startup_.graphics(), module_, 0x78, 0x6b220)(startup_.graphics(), 0);
        graphics_->methods->begin_scene(graphics_);
        preview_.draw_in_scene();
        graphics_->methods->end_scene(graphics_);
        scene_method<Set>(startup_.graphics(), module_, 0x74, 0x6abc0)(startup_.graphics(), 0, graph_state);
        driver_.set_render_state(3, driver_state);
        const auto status = preview_.playback_status();
        if (status == 1 || status == -1) {
            preview_.stop();
            fit_preview();
            preview_.play();
        }
    }

  private:
    void show(EngineFrame *frame, bool visible) {
        using Set = void(WINAPI *)(EngineFrame *, std::uint8_t);
        scene_method<Set>(frame, module_, 0x30, 0x1cfe0)(frame, visible);
    }
    void fit_preview() {
        const auto bounds = scene_.label_bounds("video", "video", module_, false, preview_frame_);
        const auto size = preview_.dimensions();
        preview_.rectangle(frontend::cinematics_preview_rectangle(
            bounds, graphics_->methods->width(graphics_), graphics_->methods->height(graphics_), size.width,
            size.height));
    }
    static bool play(void *receiver, void *, void *sender, std::uint32_t, std::uint32_t) {
        if (!sender)
            return false;
        static_cast<CinematicsMenu *>(receiver)->select_state_(12);
        return true;
    }
    static bool select(void *receiver, void *, void *sender, std::uint32_t, std::uint32_t) {
        if (!sender)
            return false;
        auto &self = *static_cast<CinematicsMenu *>(receiver);
        const auto index = static_cast<ButtonVisualPair *>(sender)->user_data();
        const auto &movies = self.startup_.profile_movies();
        if (index >= movies.size())
            throw std::out_of_range("Cinematics movie metadata index");
        self.filename_ = movies[index].filename;
        self.queue_.enqueue({0x1c400000, 0x01000002, 0, 0, 0, {}});
        self.queue_.enqueue({0x1c300000, 0x020000a0, 0, 0, 0, {}});
        frontend::ControlEvent event{0x1c300000, 0x020000a2, 0, 0, 0, {}};
        event.text = std::make_shared<const std::string>(self.filename_);
        self.queue_.enqueue(event);
        return true;
    }
    static bool hidden(void *receiver, void *, void *, std::uint32_t, std::uint32_t) {
        auto &self = *static_cast<CinematicsMenu *>(receiver);
        self.queue_.enqueue({0x1c300000, 0x020000a0, 0, 0, 0, {}});
        self.filename_.clear();
        return true;
    }
    static bool shown(void *receiver, void *, void *, std::uint32_t, std::uint32_t) {
        auto &self = *static_cast<CinematicsMenu *>(receiver);
        self.queue_.enqueue({0x1c700000, 0x02000085, 0, 0, 0, {}});
        const auto &movies = self.startup_.profile_movies();
        for (unsigned i = 0; i < movies.size(); ++i)
            if (movies[i].enabled)
                self.queue_.enqueue(
                    {0x1c700000, 0x02000083, frontend::CinematicsList::row_identifier(i), 0, 0, {}});
        return true;
    }
    StartupSession &startup_;
    MenuScene &scene_;
    MenuDriver &driver_;
    HMODULE module_;
    Graphics *graphics_;
    frontend::MenuEventQueue &queue_;
    MenuDriver::Owner model_, group_;
    std::vector<frontend::MenuFrameClone<CinematicsCloneBackend>> row_frames_;
    std::deque<std::string> row_names_;
    std::optional<CinematicsCloneBackend> clone_;
    EngineVideo preview_;
    EngineFrame *preview_frame_ = nullptr;
    std::array<float, 3> start_{};
    std::string filename_;
    std::function<void(std::uint32_t)> select_state_;
    bool built_ = false;
};
} // namespace hd2::host
#endif
