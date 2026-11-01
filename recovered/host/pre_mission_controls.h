#ifndef HD2_HOST_PRE_MISSION_CONTROLS_H
#define HD2_HOST_PRE_MISSION_CONTROLS_H

#include "frontend/pre_mission_controls.h"
#include "host/menu_control.h"
#include "host/options_menu.h"

namespace hd2::host {

// 00662D80/00662DC0/00662B50, the original stat/capacity meter. Scale remains
// on the authored local axis; axis2 is used by the soldier-stat bars. Quaternion
// conversion delegates the exact imported engine operation rather than assuming
// the model's rotation is identity.
class PreMissionMeter final : public MenuControl {
  public:
    PreMissionMeter(const MenuScene &scene, HMODULE module, const frontend::MenuControlDefinition &definition,
                    EngineFrame *frame, MenuTextObject &text, frontend::MenuEventQueue &queue)
        : MenuControl(scene, module, false, definition, frame, text, false), scene_(scene), module_(module),
          frame_(frame), id_(definition.identifier), queue_(queue) {
        properties_.focusable = 0;
    }
    bool initialize_visuals() override {
        std::memcpy(reinterpret_cast<char *>(frame_) + 0xec, &id_, sizeof(id_));
        layout();
        return true;
    }
    void initialize_events(const std::vector<frontend::ControlEvent> &events) override {
        auto copy = events;
        frontend::initialize_control_events(*this, copy);
    }
    bool dispatch_initialization_event(frontend::ControlEvent &event) { return receive_event(event, queue_); }
    bool receive_event(frontend::ControlEvent &event, frontend::ControlEventDestination &upstream) override {
        if (!(event.code & 0x040ff000u)) {
            switch (event.code & 0x03000fffu) {
            case 0x0100008d:
                event.word_0c = frontend::widget_float_bits(minimum_);
                return true;
            case 0x0100008e:
                event.word_0c = frontend::widget_float_bits(maximum_);
                return true;
            case 0x0100008f:
                event.word_0c = frontend::widget_float_bits(value_);
                return true;
            case 0x01000090:
                minimum_ = frontend::widget_float_word(event.word_0c);
                layout();
                return true;
            case 0x01000091:
                maximum_ = frontend::widget_float_word(event.word_0c);
                layout();
                return true;
            case 0x01000092:
                value_ = frontend::widget_float_word(event.word_0c);
                layout();
                return true;
            case 0x01000093:
                axis_ = event.argument;
                layout();
                return true;
            default:
                break;
            }
        }
        frontend::EmittingControlDestination<PreMissionMeter> destination{*this, &upstream};
        frontend::ControlStateRuntime<PreMissionMeter> state{*this, properties_, bindings_, destination};
        return state.dispatch(event);
    }
    bool dispatch_control_event(frontend::ControlEvent &event,
                                frontend::ControlEventDestination &destination) {
        return receive_event(event, destination);
    }
    bool eligible() const override { return properties_.eligible(); }
    bool visible() const override { return properties_.visible != 0; }
    bool disabled() const override { return properties_.disabled != 0; }
    bool key(unsigned) override { return false; }
    void bind_pick_target(unsigned) override {}
    bool has_text() const { return false; }
    void set_text_disabled(std::uint8_t) {}
    void set_frame_visible(std::uint8_t value) { scene_.set_screen_visible(module_, frame_, value != 0); }
    void update_frame() { scene_.update_screen_frame(module_, frame_); }
    std::size_t child_count() const { return 0; }
    void dispatch_child(std::size_t, frontend::ControlEvent &) {
        throw std::logic_error("Meter has no child controls");
    }

  private:
    void layout() {
        if (!std::isfinite(value_) || !std::isfinite(minimum_) || !std::isfinite(maximum_))
            throw std::runtime_error("Invalid meter value");
        const float extent = maximum_ - minimum_;
        const float ratio = std::clamp(value_ / (extent > 0 ? extent : 1.f), 0.f, 1.f);
        scene_.set_screen_visible(module_, frame_, ratio > 0);
        if (ratio > 0 && axis_ < 3) {
            auto *bytes = reinterpret_cast<char *>(frame_);
            std::array<float, 3> scale{{1, 1, 1}};
            scale[axis_] = ratio;
            std::memcpy(bytes + 0xd0, scale.data(), sizeof(scale));
            unsigned flags = 0;
            std::memcpy(&flags, bytes + 0xe0, sizeof(flags));
            if (!(flags & 8u)) {
                const auto entry = GetProcAddress(module_, "?Make@S_quat@@QAGXABUS_matrix@@@Z");
                if (reinterpret_cast<std::uintptr_t>(entry) !=
                    reinterpret_cast<std::uintptr_t>(module_) + 0x2f220)
                    throw std::runtime_error("Unsupported original meter quaternion conversion");
                using Make = void(WINAPI *)(void *, const void *);
                Make make = nullptr;
                std::memcpy(&make, &entry, sizeof(make));
                make(bytes + 0xc0, bytes + 0x80);
            }
            flags = (flags & 0xfffffecbu) | 0x40000008u;
            std::memcpy(bytes + 0xe0, &flags, sizeof(flags));
        }
        scene_.update_screen_frame(module_, frame_);
    }
    const MenuScene &scene_;
    HMODULE module_;
    EngineFrame *frame_;
    unsigned id_;
    frontend::MenuEventQueue &queue_;
    frontend::ControlProperties properties_;
    frontend::ControlEventBindings bindings_;
    float value_ = 0, minimum_ = 0, maximum_ = 1;
    unsigned axis_ = 0;
};

// 00650730/006507E0 image control. Portraits, item icons and mission snapshots
// replace the actual first material bitmap; the native owner retains that
// binding rather than drawing a replacement rectangle over the original model.
class PreMissionImage final : public MenuControl {
  public:
    struct Texture {
        EngineFrame *value;
    }; // Borrowed from the actual resource owner.
    static frontend::ControlEvent texture_event(unsigned target, EngineFrame *value) {
        return {target, 0x01000038, 0, 0, 0, std::make_shared<Texture>(Texture{value})};
    }
    PreMissionImage(const MenuScene &scene, HMODULE module, const frontend::MenuControlDefinition &definition,
                    EngineFrame *frame, MenuTextObject &text, MenuWidgetContext &context)
        : MenuControl(scene, module, false, definition, frame, text, true, &context), scene_(scene),
          module_(module), id_(definition.identifier), frame_(frame), text_(text), context_(context) {}
    bool initialize_visuals() override {
        MenuControl::initialize_visuals();
        text_.attach_control(properties_, true);
        // 00693270/006763B0 and0067C860 isolate each image's material
        // before installing a portrait/icon. Frame copies alone share it.
        using Unary = void(WINAPI *)(EngineFrame *);
        cinematics_engine_method<Unary>(frame_, module_, 0xa0)(frame_);
        EngineFrame *materials = nullptr;
        std::memcpy(&materials, reinterpret_cast<const char *>(frame_) + 0x1e0, sizeof(materials));
        cinematics_engine_method<Unary>(materials, module_, 0x18)(materials);
        using Get = EngineFrame *(WINAPI *)(EngineFrame *, unsigned);
        auto *material = cinematics_engine_method<Get>(materials, module_, 4)(materials, 0);
        if (!material)
            throw std::runtime_error("Original image has no material");
        std::memcpy(&binding_node_, reinterpret_cast<const char *>(material) + 0x5c, sizeof(binding_node_));
        if (!binding_node_)
            throw std::runtime_error("Original image has no bitmap binding");
        bitmap_ = binding();
        if (bitmap_) {
            refresh_bitmap(bitmap_);
            std::memcpy(&original_, reinterpret_cast<const char *>(bitmap_) + 0x0c, sizeof(original_));
        }
        return true;
    }
    void initialize_events(const std::vector<frontend::ControlEvent> &events) override {
        auto copy = events;
        frontend::initialize_control_events(*this, copy);
    }
    bool dispatch_initialization_event(frontend::ControlEvent &event) {
        return receive_event(event, context_.queue);
    }
    bool receive_event(frontend::ControlEvent &event, frontend::ControlEventDestination &upstream) override {
        if (!(event.code & 0x040ff000u)) {
            switch (event.code & 0x03000fffu) {
            case 0x01000038:
                if (!event.payload && event.word_0c)
                    throw std::logic_error("Image texture requires a typed native resource payload");
                texture(event.payload ? std::static_pointer_cast<Texture>(event.payload)->value : nullptr);
                return true;
            case 0x01000039:
                set_bitmap(original_);
                return true;
            case 0x0100009a:
                if (event.text && !event.text->empty())
                    select_bitmap(*event.text);
                return true;
            case 0x0100003a: {
                EngineFrame *loaded = nullptr;
                using Load = std::int32_t(WINAPI *)(EngineFrame *, const char *, const char *, unsigned,
                                                    EngineFrame **);
                const auto status = scene_method<Load>(context_graphics(), module_, 0x3c, 0x69f70)(
                    context_graphics(), event.text ? event.text->c_str() : "", nullptr, 0x400, &loaded);
                set_bitmap(status < 0 ? original_ : loaded);
                if (loaded)
                    scene_method<MenuDriver::Release>(loaded, module_, 0, 0x73760)(loaded);
                return true;
            }
            default:
                break;
            }
        }
        frontend::EmittingControlDestination<PreMissionImage> destination{*this, &upstream};
        frontend::ControlStateRuntime<PreMissionImage> state{*this, properties_, bindings_, destination};
        frontend::TextControlRuntime<MenuTextObject, decltype(state)> runtime{text_, state};
        return runtime.dispatch(event);
    }
    bool dispatch_control_event(frontend::ControlEvent &event,
                                frontend::ControlEventDestination &destination) {
        return receive_event(event, destination);
    }
    void texture(EngineFrame *value) {
        set_bitmap(value);
        frontend::ControlEvent event{0, value ? 0x01000003u : 0x01000004u, 0, 0, 0, {}};
        receive_event(event, context_.queue);
    }
    unsigned identifier() const { return id_; }
    unsigned user_data() const { return properties_.word_5c; }
    bool eligible() const override { return properties_.eligible(); }
    bool visible() const override { return properties_.visible != 0; }
    bool disabled() const override { return properties_.disabled != 0; }
    bool key(unsigned) override { return false; }
    void bind_pick_target(unsigned target) override {
        std::memcpy(reinterpret_cast<char *>(frame_) + 0xec, &target, sizeof(target));
    }
    int enter_focus(unsigned) override {
        emit(0x04003000);
        return 7;
    }
    int leave_focus(unsigned) override {
        emit(0x04004000);
        return 7;
    }
    int pointer_focus(unsigned) override {
        emit(0x04005000);
        return 7;
    }
    int pointer_target(frontend::ScreenPointerCallback callback, unsigned,
                       frontend::ControlEventDestination &) override {
        return frontend::button_pointer_event(*this, callback);
    }
    int activate(frontend::ControlEventDestination &) override {
        emit(0x04001000);
        return 1;
    }
    int deactivate(frontend::ControlEventDestination &) override {
        emit(0x04002000);
        return 1;
    }
    int click(frontend::ControlEventDestination &destination) override {
        emit(0x04012000);
        return activate(destination);
    }
    int wheel_target(std::int32_t, frontend::ControlEventDestination &) override {
        emit(0x04011000);
        return 7;
    }
    void emit_event(unsigned tag, unsigned) { emit(tag); }
    bool has_text() const { return true; }
    void set_text_disabled(std::uint8_t value) { text_.object().disabled = value; }
    void set_frame_visible(std::uint8_t value) { scene_.set_screen_visible(module_, frame_, value != 0); }
    void update_frame() { scene_.update_screen_frame(module_, frame_); }
    std::size_t child_count() const { return 0; }
    void dispatch_child(std::size_t, frontend::ControlEvent &) {
        throw std::logic_error("Image has no child controls");
    }

  private:
    EngineFrame *context_graphics() const {
        using Get = EngineFrame *(__cdecl *)();
        const auto entry = GetProcAddress(module_, "GetIGraph");
        if (reinterpret_cast<std::uintptr_t>(entry) != reinterpret_cast<std::uintptr_t>(module_) + 0x68f90)
            throw std::runtime_error("Unsupported image GetIGraph export");
        Get get = nullptr;
        std::memcpy(&get, &entry, sizeof(get));
        return get();
    }
    EngineFrame *binding() const {
        EngineFrame *result = nullptr;
        if (binding_node_)
            std::memcpy(&result, reinterpret_cast<const char *>(binding_node_) + 4, sizeof(result));
        return result;
    }
    void refresh_bitmap(EngineFrame *bitmap) {
        unsigned flags = 0;
        std::memcpy(&flags, reinterpret_cast<const char *>(bitmap) + 8, sizeof(flags));
        if (flags & 0x04000000u) {
            using Update = void(WINAPI *)(EngineFrame *);
            cinematics_engine_method<Update>(bitmap, module_, 0x44)(bitmap);
        }
    }
    void set_bitmap(EngineFrame *texture) {
        auto *bitmap = binding();
        if (!bitmap)
            return;
        using Set = void(WINAPI *)(EngineFrame *, EngineFrame *);
        cinematics_engine_method<Set>(bitmap, module_, 0x1c)(bitmap, texture);
    }
    void select_bitmap(const std::string &name) {
        EngineFrame *materials = nullptr;
        std::memcpy(&materials, reinterpret_cast<const char *>(frame_) + 0x1e0, sizeof(materials));
        using Get = EngineFrame *(WINAPI *)(EngineFrame *, unsigned);
        auto *material = cinematics_engine_method<Get>(materials, module_, 4)(materials, 0);
        EngineFrame *node = nullptr;
        if (material)
            std::memcpy(&node, reinterpret_cast<const char *>(material) + 0x5c, sizeof(node));
        for (unsigned visited = 0; node; ++visited) {
            if (visited == 4096)
                throw std::runtime_error("Invalid image bitmap chain");
            EngineFrame *bitmap = nullptr, *resource = nullptr;
            std::memcpy(&bitmap, reinterpret_cast<const char *>(node) + 4, sizeof(bitmap));
            if (bitmap) {
                refresh_bitmap(bitmap);
                std::memcpy(&resource, reinterpret_cast<const char *>(bitmap) + 0x0c, sizeof(resource));
                const char *filename = nullptr;
                if (resource)
                    std::memcpy(&filename, reinterpret_cast<const char *>(resource) + 0x18, sizeof(filename));
                if (name == (filename ? filename : "")) {
                    binding_node_ = node;
                    bitmap_ = bitmap;
                    original_ = resource;
                    return;
                }
            }
            EngineFrame *next = nullptr;
            std::memcpy(&next, reinterpret_cast<const char *>(node) + 0x0c, sizeof(next));
            node = next;
        }
    }
    void emit(unsigned tag) {
        frontend::dispatch_tagged_control_events(bindings_, this, tag, 0, context_.queue);
    }
    const MenuScene &scene_;
    HMODULE module_;
    unsigned id_;
    EngineFrame *frame_, *binding_node_ = nullptr, *bitmap_ = nullptr, *original_ = nullptr;
    MenuTextObject &text_;
    MenuWidgetContext &context_;
    frontend::ControlProperties properties_;
    frontend::ControlEventBindings bindings_;
};

// 00650A00/00650A10, actual kind0C frame selector used by screen3. The image
// children are borrowed from the loaded screen model; they are never cloned or
// released by this control. This control is deliberately absent from focus.
class PreMissionSideImages final : public MenuControl {
  public:
    using Image = EngineFrame;
    PreMissionSideImages(const MenuScene &scene, HMODULE module,
                         const frontend::MenuControlDefinition &definition, EngineFrame *frame,
                         MenuTextObject &text, frontend::MenuEventQueue &queue)
        : MenuControl(scene, module, false, definition, frame, text, false), scene_(scene), module_(module),
          identifier_(definition.identifier), frame_(frame), queue_(queue), selector_(*this) {
        properties_.focusable = 0;
    }
    bool initialize_visuals() override {
        std::memcpy(reinterpret_cast<unsigned char *>(frame_) + 0xec, &identifier_, sizeof(identifier_));
        return true;
    }
    void initialize_events(const std::vector<frontend::ControlEvent> &events) override {
        auto copy = events;
        frontend::initialize_control_events(*this, copy);
    }
    bool dispatch_initialization_event(frontend::ControlEvent &event) { return receive_event(event, queue_); }
    bool receive_event(frontend::ControlEvent &event, frontend::ControlEventDestination &upstream) override {
        if (selector_.dispatch(event))
            return true;
        frontend::EmittingControlDestination<PreMissionSideImages> destination{*this, &upstream};
        frontend::ControlStateRuntime<PreMissionSideImages> base{*this, properties_, bindings_, destination};
        return base.dispatch(event);
    }
    bool dispatch_control_event(frontend::ControlEvent &event,
                                frontend::ControlEventDestination &destination) {
        return receive_event(event, destination);
    }
    Image *find_image(const char *name) { return scene_.find_control_frame(frame_, name, module_); }
    void show_image(Image *frame, bool visible) { scene_.set_screen_visible(module_, frame, visible); }
    bool eligible() const override { return properties_.eligible(); }
    bool visible() const override { return properties_.visible != 0; }
    bool disabled() const override { return properties_.disabled != 0; }
    bool key(unsigned) override { return false; }
    void bind_pick_target(unsigned target) override {
        std::memcpy(reinterpret_cast<unsigned char *>(frame_) + 0xec, &target, sizeof(target));
    }
    bool has_text() const { return false; }
    void set_text_disabled(std::uint8_t) {}
    void set_frame_visible(std::uint8_t value) { scene_.set_screen_visible(module_, frame_, value != 0); }
    void update_frame() { scene_.update_screen_frame(module_, frame_); }
    std::size_t child_count() const { return 0; }
    void dispatch_child(std::size_t, frontend::ControlEvent &) {
        throw std::logic_error("Side-image control has no child controls");
    }

  private:
    const MenuScene &scene_;
    HMODULE module_;
    unsigned identifier_;
    EngineFrame *frame_;
    frontend::MenuEventQueue &queue_;
    frontend::ControlProperties properties_;
    frontend::ControlEventBindings bindings_;
    frontend::PreMissionImageSelector<PreMissionSideImages> selector_;
};

// 0064FA80/00650350: hero stat/counter control with two attached text planes
// and optional repeating arrows. Definitions include the value text (id|1)
// as a child so it uses the ordinary menu text renderer. Caption storage uses
// the root id, matching the original internal caption endpoint.
class PreMissionStat final : public MenuControl {
    using Part = frontend::OptionsWidgetPart<MenuWidgetVisuals>;

  public:
    PreMissionStat(const MenuScene &scene, HMODULE module, const frontend::MenuControlDefinition &definition,
                   EngineFrame *frame, MenuTextObject &caption, MenuTextObject &value,
                   MenuWidgetContext &context)
        : MenuControl(scene, module, false, definition, frame, caption, true, &context, false), scene_(scene),
          module_(module), id_(definition.identifier), frame_(frame), caption_(caption), value_(value),
          context_(context), visuals_(module, frame, caption, context) {}
    ~PreMissionStat() override {
        for (auto &cancel : listeners_)
            cancel();
    }
    bool initialize_visuals() override {
        visuals_.pick(frame_, id_);
        auto *caption = visuals_.find(frame_, "skill_text*", 0x10001);
        auto *value = visuals_.find(frame_, "skill_value*", 0x10001);
        if (!caption || !value)
            return false;
        caption_.bind_frame(caption, "skill_text*");
        value_.bind_frame(value);
        caption_.initialize_static_frame();
        value_.initialize_static_frame();
        value_.object().inset = 0; // 0064FBAB sends02000073(0).
        visuals_.pick(caption, id_);
        visuals_.pick(value, id_ | 1);
        for (unsigned i = 0; i < 2; ++i) {
            auto *arrow = visuals_.find(frame_, i ? "barrow down*" : "barrow up*", 0x10020);
            if (!arrow) {
                properties_.focusable = 0;
                continue;
            }
            arrows_[i] = std::make_unique<Part>(visuals_, arrow, id_ | (i + 2),
                                                frontend::WidgetPartKind::arrow, context_.queue);
            if (!arrows_[i]->initialize())
                return false;
            // 0064FC56/0064FD2F. Names are counterintuitive but verified in asm.
            arrows_[i]->bind({id_, i ? 0x05006070u : 0x05006071u, 0, 0, 0, {}});
            listeners_.push_back(
                context_.subscribe(id_ | (i + 2), [this, i](unsigned) { arrows_[i]->tick(); }));
        }
        return true;
    }
    void initialize_events(const std::vector<frontend::ControlEvent> &events) override {
        auto copy = events;
        frontend::initialize_control_events(*this, copy);
    }
    bool dispatch_initialization_event(frontend::ControlEvent &event) {
        return receive_event(event, context_.queue);
    }
    bool receive_event(frontend::ControlEvent &event, frontend::ControlEventDestination &upstream) override {
        if (!(event.code & 0x040ff000u)) {
            switch (event.code & 0x03000fffu) {
            case 0x01000014:
                caption_text_ = event.text;
                return true;
            case 0x01000070:
                event.argument = stat_.increase([this](unsigned code) { return linked(code); });
                if (event.argument)
                    refresh();
                return true;
            case 0x01000071:
                event.argument = stat_.decrease([this](unsigned code) { return linked(code); });
                if (event.argument)
                    refresh();
                return true;
            case 0x01000072:
                stat_.base(frontend::widget_signed_word(event.argument));
                refresh();
                return true;
            case 0x01000073:
                stat_.value(frontend::widget_signed_word(event.argument));
                refresh();
                return true;
            case 0x01000079:
                event.argument = static_cast<unsigned>(stat_.value());
                return true;
            case 0x0100000b:
                break;
            default:
                // 00650350 broadcasts all other untagged messages to its two
                // text controls, including state/visibility/layout requests.
                caption_.dispatch_control_event(event, upstream);
                value_.dispatch_control_event(event, upstream);
                return true;
            }
        }
        frontend::EmittingControlDestination<PreMissionStat> destination{*this, &upstream};
        frontend::ControlStateRuntime<PreMissionStat> base{*this, properties_, bindings_, destination};
        return base.dispatch(event);
    }
    bool dispatch_control_event(frontend::ControlEvent &event,
                                frontend::ControlEventDestination &destination) {
        return receive_event(event, destination);
    }
    bool eligible() const override { return properties_.eligible() && caption_.visible(); }
    bool visible() const override { return properties_.visible != 0; }
    bool disabled() const override { return properties_.disabled != 0; }
    bool key(unsigned word) override {
        // 006502C0: main/keypad minus and plus. The key is consumed even when
        // the base,100 cap or linked point budget prevents a value change.
        const auto key = word & 255u;
        if (key != 12 && key != 74 && key != 13 && key != 78)
            return false;
        const bool changed = (key == 12 || key == 74)
                                 ? stat_.decrease([this](unsigned code) { return linked(code); })
                                 : stat_.increase([this](unsigned code) { return linked(code); });
        if (changed)
            refresh();
        return true;
    }
    bool preserves_part_targets() const override { return true; }
    void bind_pick_target(unsigned) override {}
    int enter_focus(unsigned target) override { return pointer_focus(target); }
    int leave_focus(unsigned) override {
        if (focused_ >= 0)
            arrows_[focused_]->leave();
        focused_ = -1;
        return 1;
    }
    int pointer_focus(unsigned target) override {
        const int next = part(target);
        if (next != focused_) {
            leave_focus(target);
            focused_ = next;
            if (next >= 0)
                return arrows_[next]->enter();
        }
        return 1;
    }
    int pointer_target(frontend::ScreenPointerCallback callback, unsigned target,
                       frontend::ControlEventDestination &) override {
        const int index = part(target);
        return index >= 0 ? arrows_[index]->pointer(callback) : 1;
    }
    int activate(frontend::ControlEventDestination &) override { return 1; }
    int deactivate(frontend::ControlEventDestination &) override { return leave_focus(id_); }
    int click(frontend::ControlEventDestination &) override { return 1; }
    int wheel_target(std::int32_t, frontend::ControlEventDestination &) override { return 1; }
    bool has_text() const { return false; }
    void set_text_disabled(std::uint8_t) {}
    void set_frame_visible(std::uint8_t value) { scene_.set_screen_visible(module_, frame_, value != 0); }
    void update_frame() { scene_.update_screen_frame(module_, frame_); }
    std::size_t child_count() const { return 0; }
    void dispatch_child(std::size_t, frontend::ControlEvent &) {
        throw std::logic_error("Stat has no base children");
    }

  private:
    int part(unsigned target) const {
        const auto low = target & 15;
        return (low == 2 || low == 3) && arrows_[low - 2] ? static_cast<int>(low - 2) : -1;
    }
    bool linked(unsigned code) {
        if (!properties_.word_58)
            return true;
        frontend::ControlEvent request{properties_.word_58, code, 0, 0, 0, {}};
        context_.queue.query_control(request);
        return request.argument != 0;
    }
    void refresh() {
        caption_.object().set(caption_text_, 11, 0);
        value_.object().set(std::make_shared<const std::string>(std::to_string(stat_.value())), 22, 0);
        for (unsigned i = 0; i < 2; ++i)
            if (arrows_[i])
                arrows_[i]->set_disabled(i ? stat_.value() >= 100 : stat_.delta() < 1);
    }
    const MenuScene &scene_;
    HMODULE module_;
    unsigned id_;
    EngineFrame *frame_;
    MenuTextObject &caption_, &value_;
    MenuWidgetContext &context_;
    MenuWidgetVisuals visuals_;
    frontend::ControlProperties properties_;
    frontend::ControlEventBindings bindings_;
    frontend::PreMissionStatValue stat_;
    frontend::SharedText caption_text_;
    std::array<std::unique_ptr<Part>, 2> arrows_;
    std::vector<std::function<void()>> listeners_;
    int focused_ = -1;
};

// 00655820/00655F10's derived lists. Unlike the accepted Options binding page,
// mission/hero lists carry their own page size, row spacing and active-row set.
// Rows retain original owner-scoped IDs; filtering changes display order only.
class PreMissionList final : public OptionsGroup {
  public:
    PreMissionList(const MenuScene &scene, HMODULE module, const frontend::MenuControlDefinition &definition,
                   EngineFrame *frame, MenuTextObject &text, MenuWidgetContext &context,
                   std::function<MenuTextObject *(unsigned)> lookup, Extra extra)
        : OptionsGroup(scene, module, definition, frame, text, context, std::move(lookup), std::move(extra)),
          scene_(scene), module_(module) {}
    bool initialize_visuals() override {
        if (!OptionsGroup::initialize_visuals())
            return false;
        for (std::size_t i = 0; i < tree_->child_count(); ++i) {
            active_.push_back(i);
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
        layout();
    }
    bool receive_event(frontend::ControlEvent &event,
                       frontend::ControlEventDestination &destination) override {
        const unsigned mask = (definition_.identifier & 0xff000u) ? 0xff0u : 0xff000u;
        if (event.target & mask)
            return tree_->receive_event(event, destination);
        if (!(event.code & 0x040ff000u)) {
            switch (event.code & 0x03000fffu) {
            case 0x0100000b:
                slider_ = event.argument;
                return true;
            case 0x01000028:
                if (!event.argument)
                    throw std::runtime_error("List page size must be positive");
                page_ = event.argument;
                layout();
                return true;
            case 0x0100002a:
                spacing_ = frontend::widget_float_word(event.word_0c);
                if (!std::isfinite(spacing_) || spacing_ < 0)
                    throw std::runtime_error("Invalid list spacing");
                layout();
                return true;
            case 0x01000029:
                spacing_x_ = frontend::widget_float_word(event.word_0c);
                if (!std::isfinite(spacing_x_) || spacing_x_ < 0)
                    throw std::runtime_error("Invalid horizontal list spacing");
                layout();
                return true;
            case 0x0100006b:
                if (event.payload) {
                    anchor_ = *std::static_pointer_cast<std::array<float, 3>>(event.payload);
                    for (const auto value : *anchor_)
                        if (!std::isfinite(value))
                            throw std::runtime_error("Invalid list position anchor");
                    layout();
                }
                return true;
            case 0x02000085:
                tree_->leave_focus(0);
                active_.clear();
                offset_ = 0;
                layout();
                return true;
            case 0x02000083: {
                const int index = tree_->control_for_target(event.argument);
                if (index >= 0 && std::find(active_.begin(), active_.end(),
                                            static_cast<std::size_t>(index)) == active_.end())
                    active_.insert(std::lower_bound(active_.begin(), active_.end(),
                                                    static_cast<std::size_t>(index)),
                                   static_cast<std::size_t>(index));
                layout();
                return true;
            }
            case 0x02000084: {
                const int index = tree_->control_for_target(event.argument);
                if (index >= 0) {
                    const auto found =
                        std::find(active_.begin(), active_.end(), static_cast<std::size_t>(index));
                    if (found != active_.end())
                        active_.erase(found);
                }
                layout();
                return true;
            }
            case 0x0200002b: {
                const auto next = std::min<std::size_t>(event.argument, maximum());
                if (next != offset_) {
                    offset_ = next;
                    layout();
                }
                return true;
            }
            case 0x0200002c:
                layout();
                return true;
            case 0x020000aa:
                if (event.argument < active_.size()) {
                    ensure(event.argument);
                    tree_->enter_focus(definition_.children.at(active_[event.argument]).identifier);
                }
                return true;
            default:
                break;
            }
        }
        return tree_->receive_event(event, destination);
    }
    bool key(unsigned word) override {
        if (tree_->key(word))
            return true;
        const unsigned key = word & 255;
        if (active_.empty())
            return false;
        const auto at =
            std::find(active_.begin(), active_.end(), static_cast<std::size_t>(tree_->focused_control()));
        std::int64_t next = at == active_.end() ? 0 : at - active_.begin();
        if (key == 199)
            next = 0;
        else if (key == 207)
            next = active_.size() - 1;
        else if (key == 201)
            next -= page_ > 1 ? page_ - 1 : 1;
        else if (key == 209)
            next += page_ > 1 ? page_ - 1 : 1;
        else if (key == 200 || key == 203)
            --next;
        else if (key == 208 || key == 205)
            ++next;
        else
            return false;
        next = std::clamp<std::int64_t>(next, 0, active_.size() - 1);
        const auto target = definition_.children.at(active_.at(next)).identifier;
        if (tree_->leave_focus(target)) {
            ensure(next);
            tree_->enter_focus(target);
        }
        return key == 200 || key == 203 || key == 205 || key == 208;
    }
    int wheel_target(std::int32_t delta, frontend::ControlEventDestination &) override {
        if (!slider_)
            return 1;
        const auto steps = frontend::slider_wheel_steps(delta);
        for (unsigned i = 0; i < steps.count; ++i)
            context_.queue.dispatch_immediate({slider_, steps.code, 0, 0, 0, {}});
        return 1;
    }

  private:
    std::size_t maximum() const { return active_.size() > page_ ? active_.size() - page_ : 0; }
    void ensure(std::size_t row) {
        if (row < offset_)
            offset_ = row;
        else if (row >= offset_ + page_)
            offset_ = row - page_ + 1;
        layout();
    }
    void layout() {
        if (!tree_ || starts_.empty())
            return;
        offset_ = std::min(offset_, maximum());
        for (std::size_t i = 0; i < tree_->child_count(); ++i) {
            const auto at = std::find(active_.begin(), active_.end(), i);
            const auto row = static_cast<std::size_t>(at - active_.begin());
            const bool visible = at != active_.end() && row >= offset_ && row - offset_ < page_;
            auto &child = tree_->child_at(i);
            auto position = starts_[i];
            const auto origin = anchor_ ? *anchor_ : starts_.front();
            const auto displacement =
                static_cast<float>(static_cast<std::int64_t>(row) - static_cast<std::int64_t>(offset_));
            if (spacing_ > 0) {
                position = origin;
                position[1] -= displacement * spacing_;
            }
            if (spacing_x_ > 0) {
                position = origin;
                position[0] += displacement * spacing_x_;
            }
            scene_.set_frame_local_position(child.frame(), position);
            scene_.update_screen_frame(module_, child.frame());
            frontend::ControlEvent event{0, visible ? 0x01000003u : 0x01000004u, 0, 0, 0, {}};
            child.receive_event(event, context_.queue);
            scene_.set_screen_visible(module_, child.frame(), visible);
            event.code = 0x0200002c;
            child.receive_event(event, context_.queue);
        }
        if (slider_) {
            context_.queue.enqueue({slider_, 0x01000020, static_cast<unsigned>(maximum() + 1), 0, 0, {}});
            context_.queue.enqueue({slider_, 0x0200002b, static_cast<unsigned>(offset_), 0, 0, {}});
        }
    }
    const MenuScene &scene_;
    HMODULE module_;
    std::vector<std::array<float, 3>> starts_;
    std::vector<std::size_t> active_;
    std::size_t page_ = 20, offset_ = 0;
    unsigned slider_ = 0;
    float spacing_ = 0, spacing_x_ = 0;
    std::optional<std::array<float, 3>> anchor_;
};

} // namespace hd2::host
#endif
