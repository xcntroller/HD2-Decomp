#ifndef HD2_HOST_OPTIONS_MENU_H
#define HD2_HOST_OPTIONS_MENU_H
#include "frontend/edit_character_target.h"
#include "frontend/options_definition.h"
#include "frontend/options_flow.h"
#include "host/cinematics_menu.h"
#include "host/menu_control_tree.h"
#include <cctype>

namespace hd2::host {
// Kind-five adapter composes the already recovered recursive factory/focus
// runtime with the registered screen's polymorphic control collection.
class OptionsGroup : public MenuControl {
  public:
    using Extra =
        std::function<std::unique_ptr<MenuControl>(const frontend::MenuControlDefinition &, EngineFrame *)>;
    OptionsGroup(const MenuScene &scene, HMODULE module, const frontend::MenuControlDefinition &definition,
                 EngineFrame *frame, MenuTextObject &text, MenuWidgetContext &context,
                 std::function<MenuTextObject *(std::uint32_t)> lookup, Extra extra)
        : MenuControl(scene, module, false, definition, frame, text, true, &context), definition_(definition),
          frame_(frame), text_(text), context_(context),
          backend_{scene, module, false, std::move(lookup), &context, std::move(extra)} {}
    bool initialize_visuals() override {
        group_definition_ = definition_;
        group_definition_.kind = frontend::MenuControlKind::container;
        if (definition_.kind != frontend::MenuControlKind::container)
            group_definition_.initialization.clear();
        auto result = MenuControlTree::construct(backend_, group_definition_, frame_, frame_, context_.queue);
        if (!result.handled || !result.control)
            return false;
        tree_ = std::move(result.control);
        text_.visibility_query([this] { return tree_->visible(); });
        return true;
    }
    // The recursive factory initialized root and children in original order.
    void initialize_events(const std::vector<frontend::ControlEvent> &) override {}
    bool receive_event(frontend::ControlEvent &event,
                       frontend::ControlEventDestination &destination) override {
        return tree_->receive_event(event, destination);
    }
    bool eligible() const override { return tree_->eligible(); }
    bool visible() const override { return tree_->visible(); }
    bool disabled() const override { return tree_->disabled(); }
    bool preserves_part_targets() const override { return true; }
    void bind_pick_target(std::uint32_t) override {} // Recursive factory assigned every frame/part.
    bool key(std::uint32_t word) override { return tree_->key(word); }
    void update(std::uint32_t milliseconds) override { tree_->update(milliseconds); }
    int enter_focus(std::uint32_t target) override { return tree_->enter_focus(target); }
    int leave_focus(std::uint32_t target) override { return tree_->leave_focus(target); }
    int pointer_focus(std::uint32_t target) override { return tree_->pointer_focus(target); }
    int pointer_target(frontend::ScreenPointerCallback callback, std::uint32_t target,
                       frontend::ControlEventDestination &) override {
        return tree_->pointer(callback, target);
    }
    int activate(frontend::ControlEventDestination &) override { return tree_->activate(0); }
    int deactivate(frontend::ControlEventDestination &) override { return tree_->deactivate(0); }
    int click(frontend::ControlEventDestination &) override { return tree_->click(0); }
    int wheel_target(std::int32_t delta, frontend::ControlEventDestination &) override {
        return tree_->wheel(0, delta);
    }

  protected:
    const frontend::MenuControlDefinition &definition_;
    EngineFrame *frame_;
    MenuTextObject &text_;
    MenuWidgetContext &context_;
    MenuControlTreeBackend backend_;
    frontend::MenuControlDefinition group_definition_{};
    std::unique_ptr<MenuControlTree> tree_;
};

// 0064D800 binds an editable input child|1 and the command caption child|2.
//  The shared BF/C0 target owns character selection; the row polls the actual
//  input manager with mask9A and only then mutates the profile configuration.
class OptionsBindingRow final : public MenuControl {
    struct CaptureBackend {
        OptionsBindingRow &row;
        frontend::TextObject &text() { return row.value_.object(); }
        bool has_text() const { return true; }
        bool wide_character_mode() const { return false; } // Application font boundary is English.
        bool is_digit(std::int32_t value) { return std::isdigit(value) != 0; }
        void emit_event(std::uint32_t, std::uint32_t) {} // Internal edit has no definition observers.
        void deactivate(std::uint32_t) { row.stop_capture(); }
    };

  public:
    OptionsBindingRow(const MenuScene &scene, HMODULE module,
                      const frontend::MenuControlDefinition &definition, EngineFrame *frame,
                      MenuTextObject &text, MenuWidgetContext &context, StartupSession &startup,
                      MenuTextObject &value, MenuTextObject &label)
        : MenuControl(scene, module, false, definition, frame, text, true, &context), scene_(scene),
          module_(module), definition_(definition), frame_(frame), text_(text), context_(context),
          startup_(startup), value_(value), label_(label), capture_backend_{*this} {
        text_.attach_control(properties_, true);
    }
    ~OptionsBindingRow() override {
        auto &common = context_.queue.common_state();
        if (capture_ && common.capture_owner == capture_) {
            capture_->release(0);
            common.capture_owner.reset();
            common.motion_only = false;
        }
    }
    bool initialize_visuals() override {
        value_.bind_frame(frame_);
        value_.object().inset = 10;
        label_.bind_frame(scene_.control_frame("hktext*", module_, false, frame_));
        label_.initialize_static_frame();
        const auto label_identifier=definition_.identifier|2u;
        auto* label_frame=scene_.control_frame("hktext*",module_,false,frame_);
        std::memcpy(reinterpret_cast<unsigned char*>(label_frame)+0xec,&label_identifier,4);
        pair_ = std::make_unique<ButtonVisualPair>(scene_, module_, definition_.frame, "normal*", "actived*",
                                                   false, &value_, true, frame_);
        pair_->bind_pick_target(definition_.identifier | 1);
        pair_->initialize_events({});
        value_.visibility_query([this] { return properties_.visible != 0; });
        label_.visibility_query([this] { return properties_.visible != 0; });
        capture_ = std::make_shared<frontend::EditCharacterTarget<CaptureBackend>>(
            definition_.identifier | 1, capture_backend_, context_.queue);
        capture_->state.record_index = 0;
        return true;
    }
    void initialize_events(const std::vector<frontend::ControlEvent> &events) override {
        auto records = events;
        frontend::initialize_control_events(*this, records);
    }
    bool dispatch_initialization_event(frontend::ControlEvent &event) {
        return receive_event(event, context_.queue);
    }
    bool receive_event(frontend::ControlEvent &event, frontend::ControlEventDestination &upstream) override {
        if ((event.code & 0x040ff000u) == 0) {
            switch (event.code & 0x03000fffu) {
            case 0x0100003d:
                binding_ = event.argument;
                label_.set_localized(event.word_0c, 12, 0);
                refresh_text();
                return true;
            case 0x0100003c:
            case 0x0100003e:
                event.argument = static_cast<std::uint32_t>(binding_);
                return true;
            case 0x0200002c:
                refresh_text();
                value_.invalidate_bounds();
                label_.invalidate_bounds();
                return true;
            default:
                break;
            }
        }
        frontend::EmittingControlDestination<OptionsBindingRow> destination{*this, &upstream};
        frontend::ControlStateRuntime<OptionsBindingRow> state{*this, properties_, bindings_, destination};
        return state.dispatch(event);
    }
    bool dispatch_control_event(frontend::ControlEvent &event,
                                frontend::ControlEventDestination &destination) {
        return receive_event(event, destination);
    }
    bool eligible() const override { return properties_.eligible(); }
    bool disabled() const override { return properties_.disabled != 0; }
    bool visible() const override { return properties_.visible != 0; }
    bool preserves_part_targets() const override { return true; }
    void bind_pick_target(std::uint32_t) override {}
    bool key(std::uint32_t word) override {
        if (!capture_->state.active)
            return false;
        if ((word & 0xff) == 1)
            stop_capture();
        return true;
    }
    int enter_focus(std::uint32_t target) override {
        last_target_ = target;
        pair_->apply(true);
        label_.highlighted(true);
        return 1;
    }
    int leave_focus(std::uint32_t) override {
        if (capture_->state.active)
            return 0;
        pair_->apply(false);
        label_.highlighted(false);
        return 1;
    }
    int pointer_focus(std::uint32_t target) override {
        last_target_ = target;
        return pair_->pointer_move();
    }
    int pointer_target(frontend::ScreenPointerCallback callback, std::uint32_t target,
                       frontend::ControlEventDestination &events) override {
        last_target_ = target;
        return pair_->pointer_event(callback, events);
    }
    int activate(frontend::ControlEventDestination &) override {
        if (capture_->state.active || disabled())
            return 0;
        warm_ = false;
        capture_->request_activation();
        context_.queue.enqueue({0, 0x020000c2, 1, 0, 0, {}});
        return 1;
    }
    int deactivate(frontend::ControlEventDestination &) override { return stop_capture(); }
    int click(frontend::ControlEventDestination &events) override {
        if (last_target_ != (definition_.identifier | 1))
            return 0;
        return capture_->state.active ? stop_capture() : activate(events);
    }
    int wheel_target(std::int32_t, frontend::ControlEventDestination &) override { return 0; }
    void update(std::uint32_t milliseconds) override {
        if (!capture_->state.active)
            return;
        capture_->update(milliseconds);
        if (!warm_) {
            warm_ = true;
            return;
        }
        const auto candidate = startup_.poll_options_binding();
        if (!candidate)
            return;
        if (candidate->kind == 1 && candidate->code == 1) {
            stop_capture();
            return;
        }
        if (input::edit_binding(startup_.profile_bindings(), binding_, candidate->kind, candidate->code,
                                candidate->direction)) {
            warm_ = false;
            frontend::dispatch_tagged_control_events(bindings_, this, 0x04010000, 0, context_.queue);
            stop_capture();
            context_.input.pointer.suppress_for(200);
        }
    }
    bool has_text() const { return false; }
    void set_text_disabled(std::uint8_t value) {
        value_.object().disabled = value;
        label_.object().disabled = value;
    }
    void set_frame_visible(std::uint8_t value) {
        scene_method<void(WINAPI *)(EngineFrame *, std::uint8_t)>(frame_, module_, 0x30, 0x1cfe0)(frame_,
                                                                                                  value);
    }
    void update_frame() { scene_.update_screen_frame(module_, frame_); }
    std::size_t child_count() const { return 0; }
    void dispatch_child(std::size_t, frontend::ControlEvent &) {
        throw std::logic_error("Binding child broadcast is not a row command");
    }

  private:
    int stop_capture() {
        if (!capture_->request_deactivation())
            return 0;
        context_.queue.enqueue({0, 0x020000c2, 0, 0, 0, {}});
        return 1;
    }
    void refresh_text() {
        value_.object().set(std::make_shared<const std::string>(startup_.binding_name(binding_)), 13, 0);
    }
    const MenuScene &scene_;
    HMODULE module_;
    const frontend::MenuControlDefinition &definition_;
    EngineFrame *frame_;
    MenuTextObject &text_;
    MenuWidgetContext &context_;
    StartupSession &startup_;
    MenuTextObject &value_;
    MenuTextObject &label_;
    frontend::ControlProperties properties_;
    frontend::ControlEventBindings bindings_;
    CaptureBackend capture_backend_;
    std::shared_ptr<frontend::EditCharacterTarget<CaptureBackend>> capture_;
    std::unique_ptr<ButtonVisualPair> pair_;
    std::size_t binding_ = 0;
    std::uint32_t last_target_ = 0;
    bool warm_ = false;
};

// Kind0B uses the recovered container for nested input and adds the original
// nine-row window.00655950 lays rows down local Y;00656700 moves the group
// in world Y when scrolling and refreshes each row's cached text bounds.
class OptionsBindingList final : public OptionsGroup {
  public:
    OptionsBindingList(const MenuScene &scene, HMODULE module,
                       const frontend::MenuControlDefinition &definition, EngineFrame *frame,
                       MenuTextObject &text, MenuWidgetContext &context,
                       std::function<MenuTextObject *(std::uint32_t)> lookup, Extra extra)
        : OptionsGroup(scene, module, definition, frame, text, context, std::move(lookup), std::move(extra)),
          scene_(scene), module_(module) {}
    bool initialize_visuals() override {
        if (!OptionsGroup::initialize_visuals())
            return false;
        scene_.update_screen_frame(module_,frame_);
        origin_=scene_.frame_world_position(module_,frame_);
        for (std::size_t i = 0; i < tree_->child_count(); ++i) {
            std::array<float, 3> position{};
            std::memcpy(position.data(), reinterpret_cast<const char *>(tree_->child_at(i).frame()) + 0xb0,
                        12);
            starts_.push_back(position);
        }
        return true;
    }
    void initialize_events(const std::vector<frontend::ControlEvent> &events) override {
        for (auto event : events)
            receive_event(event, context_.queue);
    }
    bool receive_event(frontend::ControlEvent &event,
                       frontend::ControlEventDestination &destination) override {
        if ((event.target & 0xff0u) != 0)
            return tree_->receive_event(event, destination);
        if ((event.code & 0x040ff000u) == 0) {
            switch (event.code & 0x03000fffu) {
            case 0x0100000b:
                slider_ = event.argument;
                return true;
            case 0x01000028:
                if (event.argument != 9)
                    throw std::logic_error("Options binding page size changed");
                return true;
            case 0x0100002a:
                if (event.word_0c != 0x3d1374bc)
                    throw std::logic_error("Options binding spacing changed");
                return true;
            case 0x0200002b:
                scroll(event.argument);
                return true;
            case 0x0200002c:
                refresh();
                notify_scroll();
                return true;
            case 0x020000aa:
                if (event.argument < tree_->child_count()) {
                    selected_ = static_cast<int>(event.argument);
                    ensure_visible(event.argument);
                    notify_scroll();
                }
                return true;
            default:
                break;
            }
        }
        return tree_->receive_event(event, destination);
    }
    int enter_focus(std::uint32_t target) override { return tree_->enter_focus(target); }
    bool key(std::uint32_t word) override {
        if (tree_->key(word))
            return true; // Capturing row swallows every scan.
        const auto key = word & 0xffu;
        if (key != 199 && key != 200 && key != 201 && key != 203 && key != 205 && key != 207 && key != 208 &&
            key != 209)
            return false;
        const auto count = static_cast<int>(tree_->child_count());
        if (!count)
            return false;
        auto next = tree_->focused_control();
        if (key == 199)
            next = 0;
        else if (key == 207)
            next = count - 1;
        else if (key == 201)
            next = std::max(0, next - 8);
        else if (key == 209)
            next = std::min(count - 1, next + 8);
        else
            next += key == 200 || key == 203 ? -1 : 1;
        if (next < 0 || next >= count)
            return false;
        const auto target = definition_.children[static_cast<std::size_t>(next)].identifier;
        if (!tree_->leave_focus(target))
            return true;
        ensure_visible(static_cast<unsigned>(next));
        tree_->enter_focus(target);
        notify_scroll();
        // 006554B0 returnsfalse for Home/End/Page branches after moving.
        return key == 200 || key == 203 || key == 205 || key == 208;
    }
    int wheel_target(std::int32_t delta, frontend::ControlEventDestination &) override {
        const auto steps =
            (static_cast<std::uint64_t>(delta < 0 ? -static_cast<std::int64_t>(delta) : delta) + 119) / 120;
        for (std::uint64_t i = 0; i < steps; ++i)
            context_.queue.dispatch_immediate({slider_, delta > 0 ? 0x01000023u : 0x01000024u, 0, 0, 0, {}});
        return 1;
    }

  private:
    unsigned maximum() const {
        return tree_->child_count() > 9 ? static_cast<unsigned>(tree_->child_count() - 9) : 0;
    }
    void ensure_visible(unsigned row) {
        if (row < offset_)
            scroll(row);
        else if (row >= offset_ + 9)
            scroll(row - 8);
    }
    void scroll(unsigned value) {
        const auto next = std::min(value, maximum());
        if (next == offset_)
            return;
        offset_ = next;
        refresh();
    }
    void notify_scroll() {
        if (!slider_)
            return;
        // 00655E10 returns count minus page size plus ONE, including negatives.
        const auto range = static_cast<std::uint32_t>(tree_->child_count()) - 8u;
        context_.queue.enqueue({slider_, 0x01000020, range, 0, 0, {}});
        if (range)
            context_.queue.enqueue({slider_, 0x0200002b, offset_, 0, 0, {}});
    }
    void refresh() {
        std::printf("Options binding layout: list=%08x rows=%u offset=%u origin=(%.6f,%.6f,%.6f)\n",
                    static_cast<unsigned>(definition_.identifier),static_cast<unsigned>(tree_->child_count()),
                    offset_,origin_[0],origin_[1],origin_[2]);
        auto position = origin_;
        position[1] = static_cast<float>(static_cast<long double>(origin_[1]) + offset_ * .036f);
        scene_.position_menu_model(module_, frame_, position[0], position[1], position[2]);
        //1001E610 propagates the changed transform to all descendant frames
        // and invalidates their visual bounds before row captions query them.
        scene_.update_screen_frame(module_,frame_);
        for (std::size_t i = 0; i < tree_->child_count(); ++i) {
            auto &row = tree_->child_at(i);
            auto local = starts_.at(i);
            local[1] = static_cast<float>(static_cast<long double>(starts_.front()[1]) - i * .036f);
            scene_.set_frame_local_position(row.frame(),local);
            scene_.update_screen_frame(module_,row.frame());
            frontend::ControlEvent visible{
                0, i >= offset_ && i < offset_ + 9 ? 0x01000003u : 0x01000004u, 0, 0, 0, {}};
            row.receive_event(visible, context_.queue);
            frontend::ControlEvent layout{0, 0x0200002c, 0, 0, 0, {}};
            row.receive_event(layout, context_.queue);
            if(i<2 || i+1==tree_->child_count()) {
                //Keep the engine's actual transform/cache state in desktop logs.
                //Native fake frames cannot establish these values for LS3DF.
                std::array<float,3> actual{};
                std::uint32_t flags=0;
                std::memcpy(actual.data(),reinterpret_cast<const char*>(row.frame())+0xb0,12);
                std::memcpy(&flags,reinterpret_cast<const char*>(row.frame())+0xe0,4);
                const auto world=scene_.frame_world_position(module_,row.frame());
                const auto bounds=scene_.label_bounds(definition_.children.at(i).frame,"normal*",
                                                       module_,false,row.frame());
                const auto caption=scene_.label_bounds("hktext*","hktext*",module_,false,row.frame());
                std::printf("Options binding row: id=%08x frame=%p flags=%08x requestedY=%.6f "
                            "local=(%.6f,%.6f,%.6f) world=(%.6f,%.6f,%.6f) "
                            "inputY=(%.6f,%.6f) captionY=(%.6f,%.6f)\n",
                            static_cast<unsigned>(row.identifier()),static_cast<void*>(row.frame()),
                            static_cast<unsigned>(flags),local[1],actual[0],actual[1],actual[2],
                            world[0],world[1],world[2],bounds[1],bounds[5],caption[1],caption[5]);
            }
        }
    }
    const MenuScene &scene_;
    HMODULE module_;
    std::array<float, 3> origin_{};
    std::vector<std::array<float, 3>> starts_;
    std::uint32_t slider_ = 0;
    unsigned offset_ = 0;
    int selected_ = -1;
};

class OptionsMenu {
  public:
    OptionsMenu(StartupSession &startup, MenuScene &scene, MenuDriver &driver, HMODULE module,
                frontend::MenuEventQueue &queue)
        : startup_(startup), scene_(scene), driver_(driver), module_(module), queue_(queue),
          model_(scene.load_screen_model(module, "options")), clone_{driver, module}, flow_(*this) {}
    EngineFrame *model() const { return model_.get(); }
    void dialog_request(std::function<void()> request) { dialog_ = std::move(request); }
    frontend::MenuScreenDefinition definition() {
        if (built_)
            throw std::logic_error("Options model augmented twice");
        built_ = true;
        using Name = void(WINAPI *)(EngineFrame *, const char *);
        auto rename = [&](EngineFrame *frame, const char *name) {
            checked_engine_method<Name>(frame, module_, 0x34)(frame, name);
        };
        auto dummy = [&](EngineFrame *parent, const char *name, std::uint32_t link_mode = 0) {
            auto owner = driver_.create(6);
            auto *frame = owner.get();
            rename(frame, name);
            clone_.link(frame, parent, link_mode);
            scene_.position_menu_model(module_, frame, 0, 0, 0);
            groups_.push_back(std::move(owner));
            return frame;
        };
        auto result = frontend::options_definition({0x04010000, this, &category_changed},
                                                   {0x04001000, this, &defaults},startup_.mission_active());
        auto *tabs = dummy(model(), "GROptions");
        for (const auto &button : result.controls.at(4).children)
            clone_.link(scene_.control_frame(button.frame, module_, false, model()), tabs, 1);
        const char *names[] = {"nextdetail",        "shadows",    "mirrors",        "morphing",
                               "dynamic lightning", "light glow", "use high models"};
        for (unsigned i = 0; i < 7; ++i)
            rename(scene_.control_frame(names[i], module_, false, model()),
                   result.controls.at(5).children.at(i + 5).frame);
        // 0061CF60 aligns the duplicate sound ruler with the original ruler.
        auto *original = scene_.control_frame("text_min norm max", module_, false, model());
        auto *duplicate = scene_.control_frame("text_min norm max02", module_, false, model());
        scene_.update_screen_frame(module_, original);
        std::array<float, 3> world{};
        std::memcpy(world.data(), reinterpret_cast<const char *>(original) + 0x70, 12);
        scene_.position_menu_model(module_, duplicate, world[0], world[1], world[2]);
        auto *prototype = scene_.control_frame("hkinput", module_, false, model());
        auto *caption = scene_.control_frame("hktext", module_, false, model());
        clone_.link(caption, prototype, 1);
        auto *page = scene_.control_frame("keysscreen01", module_, false, model());
        auto &keys = result.controls.at(8);
        const char *groups[] = {"GR_Movements", "GR_Weapons", "GR_Other", "GR_Selections", "GR_Tactical"};
        for (unsigned group = 0; group < 5; ++group) {
            //The loaded keysscreen01 has a rotated basis: inherited local Y
            //points along world Z (confirmed in the desktop transform log).
            //Preserve the new dummy's world axes while linking, so00655950's
            //local-Y row spacing and00656700's world-Y scrolling agree.
            auto *frame = dummy(page, groups[group], 1);
            clone_.link(prototype, frame, 1);
            auto list = frontend::MenuControlDefinition{0x18900000u | ((group + 4) << 12),
                                                        groups[group],
                                                        frontend::MenuControlKind::binding_list,
                                                        nullptr,
                                                        nullptr,
                                                        frontend::MenuControlRole::none,
                                                        {{0, 0x01000028, 9, 0, 0, {}},
                                                         {0, 0x0100000b, 0x18903000, 0, 0, {}},
                                                         {0, 0x0100002a, 0, 0x3d1374bc, 0, {}}}};
            if (group)
                list.initialization.push_back({0, 0x01000004, 0, 0, 0, {}});
            const auto &bindings = startup_.profile_bindings().settings;
            for (std::size_t i = 0; i < bindings.size(); ++i)
                if (bindings[i].group == group) {
                    char name[32];
                    std::snprintf(name, sizeof(name), "Control_%2u", static_cast<unsigned>(i));
                    strings_.emplace_back(name);
                    auto row = frontend::clone_menu_frame(clone_, prototype, frame);
                    rename(row.get(), strings_.back().c_str());
                    const auto id =
                        list.identifier | ((static_cast<unsigned>(list.children.size()) + 1) << 4);
                    frontend::MenuControlDefinition definition{
                        id,
                        strings_.back().c_str(),
                        frontend::MenuControlKind::binding_row,
                        nullptr,
                        nullptr,
                        frontend::MenuControlRole::none,
                        {{0, 0x0100003d, static_cast<std::uint32_t>(i), bindings[i].caption, 0, {}},
                         frontend::observer_control_event(0, 0x04010000, this, &bindings_changed)}};
                    definition.children = {frontend::text_definition(id | 1, strings_.back().c_str()),
                                           frontend::text_definition(id | 2, "hktext*")};
                    definition.children.front().normal = "normal*";
                    list.children.push_back(std::move(definition));
                    rows_.push_back(std::move(row));
                }
            keys.children.push_back(std::move(list));
        }
        scene_.set_screen_visible(module_, prototype, false);
        for (unsigned i = 0; i < 2; ++i) {
            auto button = frontend::button_definition(
                0x18909000u + (i << 12), i ? "bback01" : "bdefault*", "normal*", "actived*",
                frontend::MenuControlRole::none,
                {frontend::caption_definition(13, i ? 0x99b : 0x999),
                 frontend::observer_control_event(0, 0x04001000, this, &defaults)});
            keys.children.push_back(std::move(button));
        }
        return result;
    }
    std::unique_ptr<MenuControl> make_control(const frontend::MenuControlDefinition &definition,
                                              EngineFrame *frame, MenuTextObject &text,
                                              MenuWidgetContext &context,
                                              std::function<MenuTextObject *(std::uint32_t)> lookup) {
        using K = frontend::MenuControlKind;
        if ((definition.identifier & 0xfc000000u) != 0x18000000)
            return {};
        auto extra = [this, &context, lookup](const frontend::MenuControlDefinition &child,
                                              EngineFrame *child_frame) {
            return make_control(child, child_frame, *lookup(child.identifier), context, lookup);
        };
        if (definition.kind == K::container)
            return std::make_unique<OptionsGroup>(scene_, module_, definition, frame, text, context, lookup,
                                                  extra);
        if (definition.kind == K::binding_list)
            return std::make_unique<OptionsBindingList>(scene_, module_, definition, frame, text, context,
                                                        lookup, extra);
        if (definition.kind == K::binding_row)
            return std::make_unique<OptionsBindingRow>(scene_, module_, definition, frame, text, context,
                                                       startup_, *lookup(definition.identifier | 1),
                                                       *lookup(definition.identifier | 2));
        return std::make_unique<MenuControl>(scene_, module_, false, definition, frame, text, true, &context);
    }
    bool dispatch(frontend::ControlEvent &event) { return flow_.dispatch(event); }
    bool options_current() const {
        const auto *current = queue_.screens().current();
        return current && current->identifier == 0x18000000;
    }
    auto &profile_settings() { return startup_.profile_settings(); }
    auto &profile_bindings() { return startup_.profile_bindings(); }
    void save_options() { startup_.save_options(); }
    void apply_graphics_options() { startup_.apply_graphics_options(); }
    void apply_bindings() { startup_.apply_bindings(); }
    bool gamma_available() const { return startup_.gamma_available(); }
    void enqueue(const frontend::ControlEvent &event) { queue_.enqueue(event); }
    void request_unassigned_dialog() {
        if (!dialog_)
            throw std::logic_error("Options dialog receiver missing");
        dialog_();
    }
    void refresh_binding_lists() {
        for (unsigned i = 0; i < 5; ++i)
            if (i != category_)
                queue_.enqueue({0x18900000u | ((i + 4) << 12), 0x0200002c, 0, 0, 0, {}});
        queue_.enqueue({0x18900000u | ((category_ + 4) << 12), 0x0200002c, 0, 0, 0, {}});
    }
    void shown() {
        frontend::ControlEvent text{0x18300000, 0x01000014, 12, 0, 0, {}};
        const auto &name = startup_.profile_name();
        if (name.empty()) {
            text.code = 0x01000015;
            text.word_0c = 2020;
        } else
            text.text = std::make_shared<const std::string>(name);
        queue_.enqueue(text);
    }

  private:
    static bool category_changed(void *owner, void *, void *, std::uint32_t, std::uint32_t argument) {
        auto &self = *static_cast<OptionsMenu *>(owner);
        if (argument > 4)
            return true;
        self.category_ = argument;
        for (unsigned group = 0; group < 5; ++group) {
            const auto id = 0x18900000u | ((group + 4) << 12);
            self.queue_.enqueue({id, group == argument ? 0x01000003u : 0x01000004u, 0, 0, 0, {}});
            if (group == argument) {
                self.queue_.enqueue({0x18903000, 0x0100000b, id, 0, 0, {}});
                self.queue_.enqueue({id, 0x0200002c, 0, 0, 0, {}});
            }
        }
        return true;
    }
    static bool defaults(void *owner, void *, void *sender, std::uint32_t, std::uint32_t) {
        if (!sender)
            return false;
        auto &self = *static_cast<OptionsMenu *>(owner);
        self.startup_.reset_bindings(static_cast<ButtonVisualPair *>(sender)->identifier() == 0x1890a000 ? 1
                                                                                                         : 0);
        self.refresh_binding_lists();
        return true;
    }
    static bool bindings_changed(void *owner, void *, void *, std::uint32_t, std::uint32_t) {
        static_cast<OptionsMenu *>(owner)->refresh_binding_lists();
        return true;
    }
    StartupSession &startup_;
    MenuScene &scene_;
    MenuDriver &driver_;
    HMODULE module_;
    frontend::MenuEventQueue &queue_;
    MenuDriver::Owner model_;
    CinematicsCloneBackend clone_;
    std::vector<MenuDriver::Owner> groups_;
    std::vector<frontend::MenuFrameClone<CinematicsCloneBackend>> rows_;
    std::deque<std::string> strings_;
    frontend::OptionsFlow<OptionsMenu> flow_;
    std::function<void()> dialog_;
    unsigned category_ = 0;
    bool built_ = false;
};
} // namespace hd2::host
#endif
