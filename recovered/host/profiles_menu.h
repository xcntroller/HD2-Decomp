#ifndef HD2_HOST_PROFILES_MENU_H
#define HD2_HOST_PROFILES_MENU_H
#include "frontend/profile_management.h"
#include "host/options_menu.h"
#include <charconv>
namespace hd2::host {

// 0065AD80/0065AE90 visual editor, using the recovered BF/C0 character owner.
class ProfileEdit final : public MenuControl {
    struct Capture {
        ProfileEdit &owner;
        frontend::TextObject &text() { return owner.text_.object(); }
        bool has_text() const { return true; }
        bool wide_character_mode() const { return false; }
        bool is_digit(std::int32_t c) { return std::isdigit(static_cast<unsigned char>(c)) != 0; }
        void emit_event(unsigned tag, unsigned value) { owner.emit(tag, value); }
        void deactivate(unsigned) { owner.stop(); }
    };

  public:
    ProfileEdit(const MenuScene &scene, HMODULE module, const frontend::MenuControlDefinition &definition,
                EngineFrame *frame, MenuTextObject &text, MenuWidgetContext &context)
        : MenuControl(scene, module, false, definition, frame, text, true, &context), scene_(scene),
          module_(module), definition_(definition), frame_(frame), text_(text), context_(context),
          capture_backend_{*this} {}
    ~ProfileEdit() override {
        auto &common = context_.queue.common_state();
        if (capture_ && common.capture_owner == capture_) {
            capture_->release(0);
            common.capture_owner.reset();
            common.motion_only = false;
        }
    }
    bool initialize_visuals() override {
        pair_ = std::make_unique<ButtonVisualPair>(scene_, module_, definition_.frame, "normal*", "actived*",
                                                   false, &text_, true, frame_);
        pair_->bind_pick_target(definition_.identifier);
        pair_->initialize_events({});
        text_.object().inset = 10;
        text_.editable_viewport = true;
        text_.visibility_query([this] { return properties_.visible != 0; });
        capture_ = std::make_shared<frontend::EditCharacterTarget<Capture>>(definition_.identifier,
                                                                            capture_backend_, context_.queue);
        capture_->state.record_index = 0;
        return true;
    }
    void initialize_events(const std::vector<frontend::ControlEvent> &events) override {
        auto copy = events;
        frontend::initialize_control_events(*this, copy);
    }
    bool dispatch_initialization_event(frontend::ControlEvent &e) { return receive_event(e, context_.queue); }
    bool receive_event(frontend::ControlEvent &e, frontend::ControlEventDestination &upstream) override {
        if (!(e.code & 0x040ff000))
            switch (e.code & 0x03000fff) {
            case 0x0100006e:
                capture_->state.capacity = e.argument;
                return true;
            case 0x0100006f:
                capture_->state.numeric_only = e.argument != 0;
                return true;
            case 0x010000a0:
                minimum_ = static_cast<std::int32_t>(e.argument);
                maximum_ = static_cast<std::int32_t>(e.word_0c);
                if (minimum_ > maximum_)
                    throw std::runtime_error("Invalid numeric edit range");
                bounded_ = true;
                return true;
            case 0x0100009b:
                capture_->state.excluded_characters = e.text;
                return true;
            case 0x02000016:
                activate(upstream);
                return true;
            case 0x0200002f:
                e.text = text_.object().records().empty() ? frontend::SharedText{}
                                                          : text_.object().records().front().text;
                return true;
            default:
                break;
            }
        frontend::EmittingControlDestination<ProfileEdit> destination{*this, &upstream};
        frontend::ControlStateRuntime<ProfileEdit> state{*this, properties_, bindings_, destination};
        frontend::TextControlRuntime<MenuTextObject, decltype(state)> runtime{text_, state};
        return runtime.dispatch(e);
    }
    bool dispatch_control_event(frontend::ControlEvent &e, frontend::ControlEventDestination &d) {
        return receive_event(e, d);
    }
    bool eligible() const override { return properties_.eligible(); }
    bool disabled() const override { return properties_.disabled != 0; }
    bool visible() const override { return properties_.visible != 0; }
    bool key(unsigned) override { return capture_->state.active != 0; }
    void additional_hover(std::function<bool()> query) { additional_hover_=std::move(query); }
    void update(unsigned ms) override { capture_->update(ms); paint_hover(); }
    bool preserves_part_targets() const override { return true; }
    void bind_pick_target(unsigned target) override { pair_->bind_pick_target(target); }
    int enter_focus(unsigned) override { return disabled() ? 0 : paint_hover(); }
    int leave_focus(unsigned) override {
        hovered_=false; pair_->apply(false);
        //0065B090/00662770 allow pointer focus to leave independently of BF/C0
        //keyboard capture. Returning zero here freezes every other hover.
        return 1;
    }
    int pointer_focus(unsigned) override { paint_hover(); return pair_->pointer_move(); }
    int pointer_target(frontend::ScreenPointerCallback c, unsigned,
                       frontend::ControlEventDestination &d) override {
        return pair_->pointer_event(c, d);
    }
    int activate(frontend::ControlEventDestination &) override {
        if (disabled() || capture_->state.active)
            return 0;
        capture_->request_activation();
        return 1;
    }
    int deactivate(frontend::ControlEventDestination &) override { return stop(); }
    int click(frontend::ControlEventDestination &d) override {
        return capture_->state.active ? stop() : activate(d);
    }
    bool has_text() const { return true; }
    void set_text_disabled(std::uint8_t b) { text_.object().disabled = b; }
    void set_frame_visible(std::uint8_t b) { scene_.set_screen_enabled(module_, frame_, b != 0); }
    void update_frame() { scene_.update_screen_frame(module_, frame_); }
    std::size_t child_count() const { return 0; }
    void dispatch_child(std::size_t, frontend::ControlEvent &) {
        throw std::logic_error("Visual edit has no child receiver");
    }
    std::string value() const {
        const auto &r = text_.object().records();
        return r.empty() || !r.front().text ? std::string{} : *r.front().text;
    }

  private:
    int paint_hover() {
        const auto& cursor=context_.input.pointer.cursor();
        const bool next=visible() && !disabled() &&
            (text_.rectangle(false).contains({cursor.x(),cursor.y()}) || (additional_hover_ && additional_hover_()));
        if (next==hovered_) return 1;
        hovered_=next; return pair_->apply(next);
    }
    bool hovered_=false;
    std::function<bool()> additional_hover_;
    int stop() {
        if (!capture_->request_deactivation()) return 0;
        //0065B2C0 clamps only after successful capture release. Empty/invalid
        // numeric text has the original sscanf destination's initial value0.
        if (bounded_) {
            const auto text = value();
            std::int32_t number = 0;
            const auto parsed = std::from_chars(text.data(), text.data() + text.size(), number);
            if (parsed.ec == std::errc::result_out_of_range)
                number = !text.empty() && text[0] == '-' ? minimum_ : maximum_;
            const auto clamped = std::clamp(number, minimum_, maximum_);
            if (clamped != number)
                text_.object().set(std::make_shared<const std::string>(std::to_string(clamped)), 13, 0);
        }
        return 1;
    }
    void emit(unsigned tag, unsigned value) {
        frontend::dispatch_tagged_control_events(bindings_, this, tag, value, context_.queue);
    }
    const MenuScene &scene_;
    HMODULE module_;
    const frontend::MenuControlDefinition &definition_;
    EngineFrame *frame_;
    MenuTextObject &text_;
    MenuWidgetContext &context_;
    frontend::ControlProperties properties_;
    frontend::ControlEventBindings bindings_;
    Capture capture_backend_;
    std::shared_ptr<frontend::EditCharacterTarget<Capture>> capture_;
    std::unique_ptr<ButtonVisualPair> pair_;
    std::int32_t minimum_ = 0, maximum_ = 0;
    bool bounded_ = false;
};

// 00657AB0's internal text|8 endpoint. Row storage remains actual TextObject
// records; scroll|4 is the existing original slider implementation.
class ProfileRecords final : public MenuControl {
  public:
    ProfileRecords(const MenuScene &scene, HMODULE module, const frontend::MenuControlDefinition &definition,
                   EngineFrame *frame, MenuTextObject &text, MenuWidgetContext &context,
                   EngineFrame *layout_root)
        : MenuControl(scene, module, false, definition, frame, text, true, &context), scene_(scene),
          module_(module), definition_(definition), frame_(frame), text_(text), context_(context),
          layout_root_(layout_root) {}
    bool initialize_visuals() override {
        text_.initialize_static_frame();
        text_.record_list = true;
        text_.object().inset = 10;
        text_.attach_control(properties_, true);
        const auto id = definition_.identifier;
        std::memcpy(reinterpret_cast<char *>(frame_) + 0xec, &id, 4);
        selector_ = scene_.bind_record_selector(module_, frame_, layout_root_, id);
        return true;
    }
    void initialize_events(const std::vector<frontend::ControlEvent> &events) override {
        auto copy = events;
        frontend::initialize_control_events(*this, copy);
    }
    bool dispatch_initialization_event(frontend::ControlEvent &e) { return receive_event(e, context_.queue); }
    bool receive_event(frontend::ControlEvent &e, frontend::ControlEventDestination &upstream) override {
        if (!(e.code & 0x040ff000))
            switch (e.code & 0x03000fff) {
            case 0x0100000b:
                slider_ = e.argument;
                return true;
            case 0x01000016:
                text_.object().append(e.text, e.argument);
                refresh();
                return true;
            case 0x02000018:
                text_.object().clear();
                text_.selected_record = -1;
                text_.hovered_record = -1;
                text_.record_offset = 0;
                refresh();
                return true;
            case 0x0200002c:
                refresh();
                return true;
            case 0x0200002b:
                text_.record_offset = std::min<std::size_t>(e.argument, maximum());
                update_selector();
                return true;
            case 0x0200002f:
                e.argument = static_cast<unsigned>(text_.selected_record);
                if (valid(text_.selected_record))
                    e.text = text_.object().records()[text_.selected_record].text;
                return true;
            case 0x02000076:
                e.argument = 0xffffffff;
                if (e.text)
                    for (std::size_t i = 0; i < text_.object().records().size(); ++i) {
                        const auto &record = text_.object().records()[i];
                        if (record.text && frontend::startup_case_equal(*record.text, *e.text)) {
                            text_.selected_record = static_cast<int>(i);
                            ensure(i);
                            e.argument = static_cast<unsigned>(i);
                            break;
                        }
                    }
                refresh();
                return true;
            default:
                break;
            }
        frontend::EmittingControlDestination<ProfileRecords> destination{*this, &upstream};
        frontend::ControlStateRuntime<ProfileRecords> state{*this, properties_, bindings_, destination};
        return state.dispatch(e);
    }
    bool dispatch_control_event(frontend::ControlEvent &e, frontend::ControlEventDestination &d) {
        return receive_event(e, d);
    }
    bool eligible() const override { return properties_.eligible(); }
    bool disabled() const override { return properties_.disabled != 0; }
    bool visible() const override { return properties_.visible != 0; }
    bool preserves_part_targets() const override { return true; }
    void bind_pick_target(unsigned) override {}
    int enter_focus(unsigned) override {
        if (!valid(text_.hovered_record))
            text_.hovered_record = text_.selected_record;
        return 1;
    }
    int leave_focus(unsigned) override {
        text_.hovered_record = -1;
        return 1;
    }
    int pointer_focus(unsigned) override {
        hover();
        return 1;
    }
    int pointer_target(frontend::ScreenPointerCallback callback, unsigned,
                       frontend::ControlEventDestination &) override {
        hover();
        if (disabled() || !valid(text_.hovered_record))
            return 1;
        //0065C120 selects the pressed row without notifying the profile owner.
        //0064DBE0 then emits double-click BEFORE the ordinary click (0064DBC0).
        //00623AE0 compares that row with the previously chosen profile; emitting
        //selection here would make two quick clicks on different rows return.
        if (callback == frontend::ScreenPointerCallback::press) {
            text_.selected_record = text_.hovered_record;
            update_selector();
        }
        if (callback == frontend::ScreenPointerCallback::double_click) {
            emit(0x04013000);
            return 7;
        }
        return 1;
    }
    int activate(frontend::ControlEventDestination &) override { return select_hovered(); }
    int deactivate(frontend::ControlEventDestination &) override { return 1; }
    int click(frontend::ControlEventDestination &) override {
        hover();
        return select_hovered();
    }
    int select_hovered() {
        if (disabled() || !valid(text_.hovered_record))
            return 0;
        text_.selected_record = text_.hovered_record;
        std::printf("Profile list selection: row=%d name=%s\n", text_.selected_record, selected().c_str());
        update_selector();
        emit(0x04001000);
        return 1;
    }
    bool key(unsigned word) override {
        const unsigned key = word & 255;
        int next = valid(text_.hovered_record) ? text_.hovered_record : text_.selected_record;
        if (key == 200)
            --next;
        else if (key == 208)
            ++next;
        else if (key == 199)
            next = 0;
        else if (key == 207)
            next = static_cast<int>(text_.object().records().size()) - 1;
        else if (key == 201)
            next -= static_cast<int>(rows());
        else if (key == 209)
            next += static_cast<int>(rows());
        else
            return false;
        if (text_.object().records().empty())
            return true;
        next = std::clamp(next, 0, static_cast<int>(text_.object().records().size()) - 1);
        text_.hovered_record = next;
        ensure(next);
        refresh();
        return true;
    }
    int wheel_target(std::int32_t delta, frontend::ControlEventDestination &) override {
        const auto steps = frontend::slider_wheel_steps(delta);
        for (unsigned i = 0; i < steps.count; ++i)
            context_.queue.dispatch_immediate({slider_, steps.code, 0, 0, 0, {}});
        return 1;
    }
    bool has_text() const { return true; }
    void set_text_disabled(std::uint8_t b) { text_.object().disabled = b; }
    void set_frame_visible(std::uint8_t b) {
        scene_.set_screen_enabled(module_, frame_, b != 0);
        update_selector();
    }
    void list_visible(bool value) {
        list_visible_ = value;
        update_selector();
    }
    void update_frame() { scene_.update_screen_frame(module_, frame_); }
    std::size_t child_count() const { return 0; }
    void dispatch_child(std::size_t, frontend::ControlEvent &) {
        throw std::logic_error("Record endpoint has no child receiver");
    }
    std::string selected() const {
        if (!valid(text_.selected_record))
            return {};
        const auto &r = text_.object().records()[text_.selected_record];
        return r.text ? *r.text : std::string{};
    }

  private:
    bool valid(int i) const {
        return i >= 0 && static_cast<std::size_t>(i) < text_.object().records().size();
    }
    std::size_t rows() {
        const auto r = text_.rectangle(true);
        return std::max<std::size_t>(
            1, static_cast<std::size_t>(std::max(0.f, std::floor((r.bottom - r.top) / 24.f))));
    }
    std::size_t maximum() {
        return text_.object().records().size() > rows() ? text_.object().records().size() - rows() : 0;
    }
    void ensure(std::size_t i) {
        if (i < text_.record_offset)
            text_.record_offset = i;
        else if (i >= text_.record_offset + rows())
            text_.record_offset = i - rows() + 1;
    }
    void refresh() {
        text_.record_offset = std::min(text_.record_offset, maximum());
        update_selector();
        if (!slider_)
            return;
        context_.queue.enqueue({slider_, 0x01000020, static_cast<unsigned>(maximum() + 1), 0, 0, {}});
        context_.queue.enqueue({slider_, 0x0200002b, static_cast<unsigned>(text_.record_offset), 0, 0, {}});
    }
    void update_selector() {
        std::optional<std::array<float, 2>> center;
        if (list_visible_ && visible() && valid(text_.selected_record) &&
            static_cast<std::size_t>(text_.selected_record) >= text_.record_offset &&
            static_cast<std::size_t>(text_.selected_record) - text_.record_offset < rows()) {
            const auto r = text_.rectangle(true);
            const float y = r.top + (text_.selected_record - text_.record_offset) * 24.f + 12.f;
            center = std::array<float, 2>{(r.left + r.right) / 1600.f, -y / 800.f};
        }
        scene_.position_record_selector(module_, selector_, center);
    }
    void hover() {
        const auto r = text_.rectangle(true);
        const auto &p = context_.input.pointer.cursor();
        if (p.x() < r.left || p.x() >= r.right || p.y() < r.top || p.y() >= r.bottom) {
            text_.hovered_record = -1;
            return;
        }
        const int i = static_cast<int>(text_.record_offset + (p.y() - r.top) / 24.f);
        text_.hovered_record = valid(i) ? i : -1;
    }
    void emit(unsigned tag) {
        frontend::dispatch_tagged_control_events(bindings_, this, tag, 0, context_.queue);
    }
    const MenuScene &scene_;
    HMODULE module_;
    const frontend::MenuControlDefinition &definition_;
    EngineFrame *frame_;
    MenuTextObject &text_;
    MenuWidgetContext &context_;
    frontend::ControlProperties properties_;
    frontend::ControlEventBindings bindings_;
    unsigned slider_ = 0;
    EngineFrame *layout_root_, *selector_ = nullptr;
    bool list_visible_ = true;
};

// 006580B0 resolves internal low-bit parts, unlike the general screen mask.
class ProfileList final : public MenuControl {
  public:
    ProfileList(const MenuScene &scene, HMODULE module, const frontend::MenuControlDefinition &d,
                EngineFrame *frame, MenuTextObject &text, MenuWidgetContext &context,
                std::function<MenuTextObject *(unsigned)> lookup, EngineFrame *layout_root)
        : MenuControl(scene, module, false, d, frame, text, true, &context), scene_(scene), module_(module),
          definition_(d), frame_(frame), text_(text), context_(context), lookup_(std::move(lookup)),
          layout_root_(layout_root) {}
    bool initialize_visuals() override {
        const unsigned zero = 0;
        std::memcpy(reinterpret_cast<char *>(frame_) + 0xec, &zero, 4);
        for (const auto &d : definition_.children) {
            auto *f = scene_.control_frame(d.frame, module_, false, frame_);
            std::unique_ptr<MenuControl> part;
            if (d.kind == frontend::MenuControlKind::records)
                part = std::make_unique<ProfileRecords>(scene_, module_, d, f, *lookup_(d.identifier),
                                                        context_, layout_root_);
            else
                part = std::make_unique<MenuControl>(scene_, module_, false, d, f, *lookup_(d.identifier),
                                                     true, &context_);
            if (!part->initialize_visuals())
                return false;
            part->set_parent_disabled_query([this] { return disabled(); });
            part->initialize_events(d.initialization);
            parts_.push_back(std::move(part));
        }
        text_.visibility_query([this] { return visible(); });
        return parts_.size() == 2;
    }
    ProfileRecords *records() { return static_cast<ProfileRecords *>(parts_.at(0).get()); }
    void initialize_events(const std::vector<frontend::ControlEvent> &events) override {
        auto copy = events;
        frontend::initialize_control_events(*this, copy);
    }
    bool dispatch_initialization_event(frontend::ControlEvent &e) { return receive_event(e, context_.queue); }
    bool receive_event(frontend::ControlEvent &e, frontend::ControlEventDestination &d) override {
        if (e.target & 15)
            return part(e.target).receive_event(e, d);
        if (!(e.code & 0x040ff000))
            switch (e.code & 0x03000fff) {
            case 0x01000005:
            case 0x01000007:
            case 0x0100000b:
            case 0x01000016:
            case 0x01000017:
            case 0x01000018:
            case 0x0100006a:
            case 0x02000015:
            case 0x02000017:
            case 0x02000018:
            case 0x0200001b:
            case 0x0200001c:
            case 0x0200002b:
            case 0x0200002c:
            case 0x0200002f:
            case 0x02000073:
            case 0x02000076:
                return parts_[0]->receive_event(e, d);
            default:
                break;
            }
        frontend::EmittingControlDestination<ProfileList> destination{*this, &d};
        frontend::ControlStateRuntime<ProfileList> state{*this, properties_, bindings_, destination};
        return state.dispatch(e);
    }
    bool dispatch_control_event(frontend::ControlEvent &e, frontend::ControlEventDestination &d) {
        return receive_event(e, d);
    }
    bool eligible() const override { return properties_.eligible(); }
    bool disabled() const override { return properties_.disabled != 0; }
    bool visible() const override { return properties_.visible != 0; }
    bool preserves_part_targets() const override { return true; }
    void bind_pick_target(unsigned) override {}
    bool key(unsigned word) override { return !disabled() && parts_[0]->key(word); }
    int enter_focus(unsigned target) override {
        focus_ = target;
        return disabled() ? 0 : part(target).enter_focus(target);
    }
    int leave_focus(unsigned target) override { return part(focus_).leave_focus(target); }
    int pointer_focus(unsigned target) override {
        focus_ = target;
        return disabled() ? 0 : part(target).pointer_focus(target);
    }
    int pointer_target(frontend::ScreenPointerCallback c, unsigned target,
                       frontend::ControlEventDestination &d) override {
        focus_ = target;
        return disabled() ? 0 : part(target).pointer_target(c, target, d);
    }
    int activate(frontend::ControlEventDestination &d) override {
        return disabled() ? 0 : part(focus_).activate(d);
    }
    int deactivate(frontend::ControlEventDestination &d) override { return part(focus_).deactivate(d); }
    int click(frontend::ControlEventDestination &d) override {
        return disabled() ? 0 : part(focus_).click(d);
    }
    int wheel_target(std::int32_t delta, frontend::ControlEventDestination &d) override {
        return disabled() ? 0 : parts_[0]->wheel_target(delta, d);
    }
    void update(unsigned ms) override { part(focus_).update(ms); }
    bool has_text() const { return true; }
    void set_text_disabled(std::uint8_t b) { records()->set_text_disabled(b); }
    void set_frame_visible(std::uint8_t b) {
        scene_.set_screen_enabled(module_, frame_, b != 0);
        records()->list_visible(b != 0);
    }
    void update_frame() { scene_.update_screen_frame(module_, frame_); }
    std::size_t child_count() const { return parts_.size(); }
    void dispatch_child(std::size_t i, frontend::ControlEvent &e) {
        parts_.at(i)->receive_event(e, context_.queue);
    }

  private:
    MenuControl &part(unsigned target) { return *parts_.at((target & 4) ? 1 : 0); }
    const MenuScene &scene_;
    HMODULE module_;
    const frontend::MenuControlDefinition &definition_;
    EngineFrame *frame_;
    MenuTextObject &text_;
    MenuWidgetContext &context_;
    std::function<MenuTextObject *(unsigned)> lookup_;
    std::vector<std::unique_ptr<MenuControl>> parts_;
    EngineFrame *layout_root_;
    unsigned focus_ = 8;
    frontend::ControlProperties properties_;
    frontend::ControlEventBindings bindings_;
};

class ProfilesMenu {
  public:
    ProfilesMenu(StartupSession &startup, MenuScene &scene, HMODULE module, frontend::MenuEventQueue &queue)
        : startup_(startup), scene_(scene), module_(module), queue_(queue),
          model_(scene.load_screen_model(module, "change_profile")), flow_(*this) {}
    EngineFrame *model() const { return model_.get(); }
    void dialog_request(std::function<void(unsigned, unsigned, unsigned, unsigned)> request) {
        prompt_ = std::move(request);
    }
    void changed(std::function<void()> callback) { changed_ = std::move(callback); }
    frontend::MenuScreenDefinition definition();
    std::unique_ptr<MenuControl> make_control(const frontend::MenuControlDefinition &d, EngineFrame *f,
                                              MenuTextObject &text, MenuWidgetContext &context,
                                              std::function<MenuTextObject *(unsigned)> lookup) {
        using K = frontend::MenuControlKind;
        if (d.kind == K::profile_list) {
            auto result =
                std::make_unique<ProfileList>(scene_, module_, d, f, text, context, lookup, model_.get());
            list_ = result.get();
            return result;
        }
        if (d.kind == K::edit) {
            auto result = std::make_unique<ProfileEdit>(scene_, module_, d, f, text, context);
            edit_ = result.get();
            return result;
        }
        if (d.kind == K::records) {
            auto result =
                std::make_unique<ProfileRecords>(scene_, module_, d, f, text, context, model_.get());
            records_ = result.get();
            return result;
        }
        return std::make_unique<MenuControl>(scene_, module_, false, d, f, text, true, &context);
    }
    void shown() { flow_.shown(); }
    void back() { select_profile(flow_.selected()); }
    bool dispatch(frontend::ControlEvent &e) { return flow_.dispatch(e); }
    const std::string &profile_name() { return startup_.profile_name(); }
    void select_profile(const std::string &value) {
        startup_.select_managed_profile(value);
        if (changed_)
            changed_();
    }
    void save_profile() { startup_.save_managed_profile(); }
    unsigned difficulty() { return startup_.profile_difficulty(); }
    void difficulty(unsigned value) { startup_.profile_difficulty(value); }
    int create_profile(const std::string &value) { return startup_.create_managed_profile(value); }
    int rename_profile(const std::string &from, const std::string &to) {
        return startup_.rename_managed_profile(from, to);
    }
    bool delete_profile(const std::string &value) { return startup_.delete_managed_profile(value); }
    void enqueue(const frontend::ControlEvent &e) { queue_.enqueue(e); }
    void prompt(unsigned mode, unsigned label, unsigned yes, unsigned no) { prompt_(mode, label, yes, no); }
    void refresh_profiles() {
        names_ = startup_.profile_names();
        queue_.enqueue({0x08200008, 0x02000018, 0, 0, 0, {}});
        for (const auto &name : names_) {
            frontend::ControlEvent e{0x08200008, 0x01000016, 19, 0, 0, {}};
            e.text = std::make_shared<const std::string>(name);
            queue_.enqueue(e);
        }
    }
    std::string first_profile() const { return names_.empty() ? std::string{} : names_.front(); }
    void select_list_name(const std::string &value) {
        frontend::ControlEvent e{0x08200008, 0x02000076, 0, 0, 0, {}};
        e.text = std::make_shared<const std::string>(value);
        queue_.enqueue(e);
    }

  private:
    static bool action(void *owner, void *, void *sender, unsigned tag, unsigned) {
        auto &self = *static_cast<ProfilesMenu *>(owner);
        if (!sender)
            return false;
        if (tag == 0x04016000) {
            self.flow_.submit(self.edit_->value());
            return true;
        }
        if (tag == 0x04017000) {
            self.flow_.cancel();
            return true;
        }
        const auto id = static_cast<ButtonVisualPair *>(sender)->identifier();
        if (id == 0x08700000)
            self.flow_.begin(false);
        else if (id == 0x08800000)
            self.flow_.begin(true);
        else if (id == 0x08900000)
            self.flow_.request_delete();
        else if (id >= 0x08300000 && id <= 0x08600000)
            self.flow_.difficulty((id >> 20) - 0x82);
        return true;
    }
    static bool selection(void *owner, void *, void *, unsigned tag, unsigned) {
        auto &self = *static_cast<ProfilesMenu *>(owner);
        if (tag == 0x04001000)
            self.flow_.choose(self.list_->records()->selected());
        else if (tag == 0x04013000 && !self.list_->records()->selected().empty() &&
                 self.list_->records()->selected() == self.flow_.selected())
            self.queue_.enqueue({0, 0x02000003, 0, 0, 0, {}});
        return true;
    }
    StartupSession &startup_;
    MenuScene &scene_;
    HMODULE module_;
    frontend::MenuEventQueue &queue_;
    MenuDriver::Owner model_;
    frontend::ProfileManagement<ProfilesMenu> flow_;
    std::vector<std::string> names_;
    ProfileEdit *edit_ = nullptr;
    ProfileRecords *records_ = nullptr;
    ProfileList *list_ = nullptr;
    std::function<void(unsigned, unsigned, unsigned, unsigned)> prompt_;
    std::function<void()> changed_;
};
inline frontend::MenuScreenDefinition ProfilesMenu::definition() {
    using namespace frontend;
    MenuScreenDefinition result{"profiles", 0x08000000, "change_profile", {}};
    result.controls.push_back({0x08100000,
                               "bcurrent profile",
                               MenuControlKind::edit,
                               "normal*",
                               "actived*",
                               MenuControlRole::none,
                               {}});
    result.controls.back().initialization.push_back({0, 0x01000015, 0x0000000d, 0x000007d2, 0, {}});
    {
        ControlEvent e{0, 0x0100009b, 0, 0, 0, {}};
        e.text = std::make_shared<const std::string>("./\\*?|<>:\"");
        result.controls.back().initialization.push_back(e);
    }
    result.controls.back().initialization.push_back({0, 0x0100006e, 0x00000011, 0x0, 0, {}});
    result.controls.back().initialization.push_back(observer_control_event(0, 0x04016000, this, &action));
    result.controls.back().initialization.push_back(observer_control_event(0, 0x04017000, this, &action));
    result.controls.push_back({0x08200000,
                               "LB_Profile*",
                               MenuControlKind::profile_list,
                               nullptr,
                               nullptr,
                               MenuControlRole::none,
                               {}});
    result.controls.back().initialization.push_back(observer_control_event(0, 0x04001000, this, &selection));
    result.controls.back().initialization.push_back(observer_control_event(0, 0x04013000, this, &selection));
    result.controls.push_back({0x08300000,
                               "beasy-*",
                               MenuControlKind::button,
                               "normal*",
                               "actived*",
                               MenuControlRole::profiles,
                               {}});
    result.controls.back().initialization.push_back({0, 0x01000015, 0x0000000d, 0x0000086d, 0, {}});
    result.controls.back().initialization.push_back(observer_control_event(0, 0x04001000, this, &action));
    result.controls.push_back({0x08400000,
                               "bnormal-*",
                               MenuControlKind::button,
                               "normal*",
                               "actived*",
                               MenuControlRole::profiles,
                               {}});
    result.controls.back().initialization.push_back({0, 0x01000015, 0x0000000d, 0x0000086e, 0, {}});
    result.controls.back().initialization.push_back(observer_control_event(0, 0x04001000, this, &action));
    result.controls.push_back({0x08500000,
                               "bhard-*",
                               MenuControlKind::button,
                               "normal*",
                               "actived*",
                               MenuControlRole::profiles,
                               {}});
    result.controls.back().initialization.push_back({0, 0x01000015, 0x0000000d, 0x0000086f, 0, {}});
    result.controls.back().initialization.push_back(observer_control_event(0, 0x04001000, this, &action));
    result.controls.push_back({0x08600000,
                               "bvery hard-*",
                               MenuControlKind::button,
                               "normal*",
                               "actived*",
                               MenuControlRole::profiles,
                               {}});
    result.controls.back().initialization.push_back({0, 0x01000015, 0x0000000d, 0x00000870, 0, {}});
    result.controls.back().initialization.push_back(observer_control_event(0, 0x04001000, this, &action));
    result.controls.push_back({0x08700000,
                               "bcreate profile",
                               MenuControlKind::button,
                               "normal*",
                               "actived*",
                               MenuControlRole::profiles,
                               {}});
    result.controls.back().initialization.push_back({0, 0x01000015, 0x0000000d, 0x00000868, 0, {}});
    result.controls.back().initialization.push_back(observer_control_event(0, 0x04001000, this, &action));
    result.controls.push_back({0x08800000,
                               "brename profile",
                               MenuControlKind::button,
                               "normal*",
                               "actived*",
                               MenuControlRole::profiles,
                               {}});
    result.controls.back().initialization.push_back({0, 0x01000015, 0x0000000d, 0x0000086a, 0, {}});
    result.controls.back().initialization.push_back(observer_control_event(0, 0x04001000, this, &action));
    result.controls.push_back({0x08900000,
                               "bdelete profile",
                               MenuControlKind::button,
                               "normal*",
                               "actived*",
                               MenuControlRole::profiles,
                               {}});
    result.controls.back().initialization.push_back({0, 0x01000015, 0x0000000d, 0x0000086b, 0, {}});
    result.controls.back().initialization.push_back(observer_control_event(0, 0x04001000, this, &action));
    result.controls.push_back(
        {0x08a00000, "bback", MenuControlKind::button, "normal*", "actived*", MenuControlRole::profiles, {}});
    result.controls.back().initialization.push_back({0, 0x01000015, 0x0000000d, 0x0000086c, 0, {}});
    result.controls.back().initialization.push_back({0, 0x06001003, 0x00000000, 0x0, 0, {}});
    result.controls.push_back(
        {0x08b00000, "text_h&dversion", MenuControlKind::text, nullptr, nullptr, MenuControlRole::none, {}});
    result.controls.back().initialization.push_back({0, 0x01000015, 0x00000028, 0x000007d6, 0, {}});
    result.controls.push_back({0x08c00000,
                               "text_current profile and save",
                               MenuControlKind::text,
                               nullptr,
                               nullptr,
                               MenuControlRole::none,
                               {}});
    result.controls.back().initialization.push_back({0, 0x01000015, 0x0000000b, 0x0000088f, 0, {}});
    result.controls.push_back({0x08d00000,
                               "text_date and time",
                               MenuControlKind::text,
                               nullptr,
                               nullptr,
                               MenuControlRole::none,
                               {}});
    result.controls.push_back({0x08e00000,
                               "text_list of profiles",
                               MenuControlKind::text,
                               nullptr,
                               nullptr,
                               MenuControlRole::none,
                               {}});
    result.controls.back().initialization.push_back({0, 0x01000015, 0x0000000b, 0x00000890, 0, {}});
    result.controls.push_back(
        {0x08f00000, "text01", MenuControlKind::text, nullptr, nullptr, MenuControlRole::none, {}});
    result.controls.push_back(
        {0x09000000, "text_screen", MenuControlKind::text, nullptr, nullptr, MenuControlRole::none, {}});
    result.controls.back().initialization.push_back({0, 0x01000015, 0x0000000a, 0x00000867, 0, {}});
    result.controls.push_back(
        {0x09100000, "text_difficulty", MenuControlKind::text, nullptr, nullptr, MenuControlRole::none, {}});
    result.controls.back().initialization.push_back({0, 0x01000015, 0x0000000b, 0x00000891, 0, {}});
    auto &list = result.controls.at(1);
    MenuControlDefinition records{
        0x08200008,         "text*", MenuControlKind::records, nullptr, nullptr, MenuControlRole::none,
        list.initialization};
    records.initialization.push_back({0, 0x0100000b, 0x08200004, 0, 0, {}});
    list.initialization.clear();
    MenuControlDefinition slider{0x08200004,
                                 "scroll*",
                                 MenuControlKind::slider,
                                 nullptr,
                                 nullptr,
                                 MenuControlRole::none,
                                 {{0, 0x0100000b, 0x08200008, 0, 0, {}}, {0, 0x01000005, 0, 0, 0, {}}}};
    list.children = {std::move(records), std::move(slider)};
    return result;
}
} // namespace hd2::host
#endif
