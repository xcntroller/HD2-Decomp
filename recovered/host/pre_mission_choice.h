#ifndef HD2_HOST_PRE_MISSION_CHOICE_H
#define HD2_HOST_PRE_MISSION_CHOICE_H
#include "host/profiles_menu.h"

namespace hd2::host {

// 00658770/00659160: preparation combo boxes own a header, cbdown button and
// retained listbox. Item text and selected index belong to this control, not to
// the current profile or its Options selector settings.
class PreMissionChoice final : public MenuControl {
  public:
    static void children(frontend::MenuControlDefinition &definition) {
        using namespace frontend;
        const auto id = definition.identifier;
        auto header = text_definition(id | 2, "header*");
        header.normal = "text*";
        auto list = MenuControlDefinition{id | 0x1000, "listbox*", MenuControlKind::profile_list,
                                          nullptr,     nullptr,    MenuControlRole::none,
                                          {}};
        list.children.push_back({id | 0x1008,
                                 "text*",
                                 MenuControlKind::records,
                                 nullptr,
                                 nullptr,
                                 MenuControlRole::none,
                                 {{0, 0x0100000b, id | 0x1004, 0, 0, {}}}});
        list.children.push_back({id | 0x1004,
                                 "scroll*",
                                 MenuControlKind::slider,
                                 nullptr,
                                 nullptr,
                                 MenuControlRole::none,
                                 {{0, 0x0100000b, id | 0x1008, 0, 0, {}}}});
        definition.children = {std::move(header), std::move(list)};
    }
    PreMissionChoice(const MenuScene &scene, HMODULE module,
                     const frontend::MenuControlDefinition &definition, EngineFrame *frame,
                     MenuTextObject &text, MenuWidgetContext &context,
                     std::function<MenuTextObject *(unsigned)> lookup)
        : MenuControl(scene, module, false, definition, frame, text, true, &context, false), scene_(scene),
          module_(module), definition_(definition), frame_(frame), context_(context),
          lookup_(std::move(lookup)) {
        text.attach_control(properties_, true);
    }
    bool initialize_visuals() override {
        const auto id = definition_.identifier;
        header_frame_ = scene_.control_frame("header*", module_, false, frame_);
        auto *arrow = scene_.control_frame("cbdown*", module_, false, frame_);
        list_frame_ = scene_.control_frame("listbox*", module_, false, frame_);
        auto *header = lookup_(id | 2);
        header->bind_frame(header_frame_);
        header->initialize_static_frame();
        header->visibility_query([this] { return visible(); });
        arrow_ = std::make_unique<ButtonVisualPair>(scene_, module_, "cbdown*", "normal*", "actived*", false,
                                                    nullptr, true, arrow);
        arrow_->bind_pick_target(id | 1);
        arrow_->initialize_events({frontend::observer_control_event(0, 0x04001000, this, &toggle)});
        auto *text = lookup_(id | 0x1000);
        list_ = std::make_unique<ProfileList>(scene_, module_, definition_.children.at(1), list_frame_, *text,
                                              context_, lookup_, list_frame_);
        if (!list_->initialize_visuals())
            return false;
        frontend::ControlEvent observer =
            frontend::observer_control_event(id | 0x1008, 0x04001000, this, &chosen);
        list_->receive_event(observer, context_.queue);
        show_list(false);
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
        if (event.target & 0x1000u)
            return list_->receive_event(event, upstream);
        if (!(event.code & 0x040ff000u))
            switch (event.code & 0x03000fffu) {
            case 0x01000016:
            case 0x01000017: {
                auto text = event.code == 0x01000017
                                ? lookup_(definition_.identifier | 2)->localize(event.word_0c)
                                : event.text;
                items_.push_back(text);
                frontend::ControlEvent append{
                    definition_.identifier | 0x1008, 0x01000016, event.argument, 0, 0, {}};
                append.text = text;
                list_->receive_event(append, context_.queue);
                return true;
            }
            case 0x02000018:
                items_.clear();
                selected_ = -1;
                list_->receive_event(event, context_.queue);
                return true;
            case 0x01000081:
                header_style_ = event.argument;
                set_header(lookup_(definition_.identifier | 2)->localize(event.word_0c));
                return true;
            case 0x01000076:
                if (event.argument < items_.size()) {
                    selected_ = event.argument;
                    set_header(items_[selected_]);
                }
                return true;
            case 0x0200002f:
                event.argument = static_cast<unsigned>(selected_);
                event.text = selected_ >= 0 && static_cast<std::size_t>(selected_) < items_.size()
                                 ? items_[selected_]
                                 : frontend::SharedText{};
                return true;
            case 0x0200002d:
                show_list(!expanded_);
                return true;
            case 0x0200002e:
                choose();
                return true;
            default:
                break;
            }
        frontend::EmittingControlDestination<PreMissionChoice> destination{*this, &upstream};
        frontend::ControlStateRuntime<PreMissionChoice> state{*this, properties_, bindings_, destination};
        return state.dispatch(event);
    }
    bool dispatch_control_event(frontend::ControlEvent &event, frontend::ControlEventDestination &upstream) {
        return receive_event(event, upstream);
    }
    bool eligible() const override { return properties_.eligible(); }
    bool visible() const override { return properties_.visible != 0; }
    bool disabled() const override { return properties_.disabled != 0; }
    bool preserves_part_targets() const override { return true; }
    void bind_pick_target(unsigned) override {}
    bool key(unsigned word) override {
        if ((word & 255) == 1 && expanded_) {
            show_list(false);
            return true;
        }
        if (expanded_)
            return list_->key(word);
        if ((word & 255) == 200 || (word & 255) == 208) {
            show_list(true);
            return true;
        }
        return false;
    }
    void update(unsigned ms) override {
        if (expanded_)
            list_->update(ms);
    }
    int enter_focus(unsigned target) override {
        focus_ = target;
        return target & 0x1000 ? list_->enter_focus(target) : arrow_->apply(true);
    }
    int leave_focus(unsigned target) override {
        if (expanded_ && (target & 0x03f00000u) == (definition_.identifier & 0x03f00000u))
            return 1;
        show_list(false);
        return arrow_->apply(false);
    }
    int pointer_focus(unsigned target) override {
        focus_ = target;
        return target & 0x1000 ? list_->pointer_focus(target) : arrow_->pointer_move();
    }
    int pointer_target(frontend::ScreenPointerCallback callback, unsigned target,
                       frontend::ControlEventDestination &events) override {
        focus_ = target;
        return target & 0x1000 ? list_->pointer_target(callback, target, events)
                               : arrow_->pointer_event(callback, events);
    }
    int activate(frontend::ControlEventDestination &) override {
        show_list(!expanded_);
        return 1;
    }
    int deactivate(frontend::ControlEventDestination &) override {
        show_list(false);
        return 1;
    }
    int click(frontend::ControlEventDestination &events) override {
        return focus_ & 0x1000 ? list_->click(events) : activate(events);
    }
    int wheel_target(std::int32_t amount, frontend::ControlEventDestination &events) override {
        return expanded_ ? list_->wheel_target(amount, events) : 1;
    }
    bool has_text() const { return true; }
    void set_text_disabled(std::uint8_t value) {
        lookup_(definition_.identifier | 2)->object().disabled = value;
    }
    void set_frame_visible(std::uint8_t value) { scene_.set_screen_visible(module_, frame_, value != 0); }
    void update_frame() { scene_.update_screen_frame(module_, frame_); }
    std::size_t child_count() const { return 0; }
    void dispatch_child(std::size_t, frontend::ControlEvent &) {
        throw std::logic_error("Combo children have explicit endpoints");
    }

  private:
    void set_header(frontend::SharedText text) {
        auto &object = lookup_(definition_.identifier | 2)->object();
        object.clear();
        object.append(std::move(text), header_style_);
    }
    void show_list(bool value) {
        expanded_ = value;
        if (list_) {
            frontend::ControlEvent event{0, value ? 0x01000003u : 0x01000004u, 0, 0, 0, {}};
            list_->receive_event(event, context_.queue);
        }
        scene_.set_screen_visible(module_, list_frame_, value);
    }
    void choose() {
        selected_ = list_->records()->selected().empty()
                        ? -1
                        : lookup_(definition_.identifier | 0x1008)->selected_record;
        if (selected_ >= 0 && static_cast<std::size_t>(selected_) < items_.size())
            set_header(items_[selected_]);
        show_list(false);
        frontend::dispatch_tagged_control_events(bindings_, this, 0x0400e000, 0, context_.queue);
    }
    static bool toggle(void *owner, void *, void *, unsigned, unsigned) {
        auto &self = *static_cast<PreMissionChoice *>(owner);
        self.show_list(!self.expanded_);
        return true;
    }
    static bool chosen(void *owner, void *, void *, unsigned, unsigned) {
        static_cast<PreMissionChoice *>(owner)->choose();
        return true;
    }
    const MenuScene &scene_;
    HMODULE module_;
    const frontend::MenuControlDefinition &definition_;
    EngineFrame *frame_, *header_frame_ = nullptr, *list_frame_ = nullptr;
    MenuWidgetContext &context_;
    std::function<MenuTextObject *(unsigned)> lookup_;
    std::unique_ptr<ButtonVisualPair> arrow_;
    std::unique_ptr<ProfileList> list_;
    frontend::ControlProperties properties_;
    frontend::ControlEventBindings bindings_;
    std::vector<frontend::SharedText> items_;
    int selected_ = -1;
    unsigned header_style_ = 12, focus_ = 0;
    bool expanded_ = false;
};
} // namespace hd2::host
#endif
