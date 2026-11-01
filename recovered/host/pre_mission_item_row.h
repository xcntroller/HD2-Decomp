#ifndef HD2_HOST_PRE_MISSION_ITEM_ROW_H
#define HD2_HOST_PRE_MISSION_ITEM_ROW_H
#include "host/pre_mission_controls.h"
#include <map>

namespace hd2::host {

// 007DF650's lazy menu-icon resource owner. The catalogue supplies its second
// resource name, suffixed with.bmp. Icons are real IGraph textures, shared by
// store and equipped-item visuals and released before the engine module.
class PreMissionIcons {
  public:
    explicit PreMissionIcons(HMODULE module) : module_(module) {}
    ~PreMissionIcons() {
        for (auto &entry : icons_)
            if (entry.second)
                scene_method<MenuDriver::Release>(entry.second, module_, 0, 0x73760)(entry.second);
    }
    EngineFrame *get(const frontend::PreMissionItem &item) {
        const auto found = icons_.find(item.identifier);
        if (found != icons_.end())
            return found->second;
        if (item.resources[1].empty())
            return nullptr;
        using Get = EngineFrame *(__cdecl *)();
        const auto address = GetProcAddress(module_, "GetIGraph");
        if (reinterpret_cast<std::uintptr_t>(address) != reinterpret_cast<std::uintptr_t>(module_) + 0x68f90)
            throw std::runtime_error("Unsupported item icon graphics export");
        Get get = nullptr;
        std::memcpy(&get, &address, sizeof(get));
        auto *graphics = get();
        using Load =
            std::int32_t(WINAPI *)(EngineFrame *, const char *, const char *, unsigned, EngineFrame **);
        EngineFrame *texture = nullptr;
        const auto path = item.resources[1] + ".bmp";
        const auto result = scene_method<Load>(graphics, module_, 0x3c, 0x69f70)(graphics, path.c_str(),
                                                                                 nullptr, 0, &texture);
        if (result < 0)
            texture = nullptr;
        icons_.emplace(item.identifier, texture);
        return texture;
    }

  private:
    HMODULE module_;
    std::map<unsigned, EngineFrame *> icons_;
};

// 00652740/00652AA0's visual-store item row. Its two image endpoints retain
// independent item IDs (weapon and associated ammunition); quantities are not
// deduced from the row index or from the catalogue's ammo capacity.
class PreMissionItemRow final : public MenuControl {
  public:
    PreMissionItemRow(const MenuScene &scene, HMODULE module,
                      const frontend::MenuControlDefinition &definition, EngineFrame *frame,
                      MenuTextObject &text, MenuWidgetContext &context,
                      std::function<MenuTextObject *(unsigned)> lookup)
        : MenuControl(scene, module, false, definition, frame, text, true, &context, false), scene_(scene),
          module_(module), definition_(definition), frame_(frame), context_(context),
          lookup_(std::move(lookup)) {
        text.attach_control(properties_, true);
    }
    bool initialize_visuals() override {
        for (unsigned i = 0; i < 2; ++i) {
            const auto &d = definition_.children.at(i);
            auto *frame = scene_.control_frame(d.frame, module_, false, frame_);
            images_[i] = std::make_unique<PreMissionImage>(scene_, module_, d, frame, *lookup_(d.identifier),
                                                           context_);
            if (!images_[i]->initialize_visuals())
                return false;
            images_[i]->initialize_events(d.initialization);
        }
        auto &caption = *lookup_(definition_.children.at(2).identifier);
        caption.bind_frame(scene_.control_frame(definition_.children.at(2).frame, module_, false, frame_));
        caption.initialize_static_frame();
        caption.visibility_query([this] { return visible(); });
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
        if (event.target & 3u) {
            const auto part = event.target & 3u;
            if (part < 3)
                return images_[part - 1]->receive_event(event, upstream);
            return lookup_(definition_.identifier | 3)->dispatch_control_event(event, upstream);
        }
        frontend::EmittingControlDestination<PreMissionItemRow> destination{*this, &upstream};
        frontend::ControlStateRuntime<PreMissionItemRow> state{*this, properties_, bindings_, destination};
        return state.dispatch(event);
    }
    bool dispatch_control_event(frontend::ControlEvent &event, frontend::ControlEventDestination &upstream) {
        return receive_event(event, upstream);
    }
    bool eligible() const override { return properties_.eligible(); }
    bool disabled() const override { return properties_.disabled != 0; }
    bool visible() const override { return properties_.visible != 0; }
    bool preserves_part_targets() const override { return true; }
    void bind_pick_target(unsigned) override {}
    bool key(unsigned) override { return false; }
    int enter_focus(unsigned target) override {
        focus_ = target & 3;
        return image().enter_focus(target);
    }
    int leave_focus(unsigned target) override { return image().leave_focus(target); }
    int pointer_focus(unsigned target) override {
        focus_ = target & 3;
        return image().pointer_focus(target);
    }
    int pointer_target(frontend::ScreenPointerCallback callback, unsigned target,
                       frontend::ControlEventDestination &events) override {
        focus_ = target & 3;
        return image().pointer_target(callback, target, events);
    }
    int activate(frontend::ControlEventDestination &events) override { return image().activate(events); }
    int deactivate(frontend::ControlEventDestination &events) override { return image().deactivate(events); }
    int click(frontend::ControlEventDestination &events) override { return image().click(events); }
    bool has_text() const { return false; }
    void set_text_disabled(std::uint8_t) {}
    void set_frame_visible(std::uint8_t value) { scene_.set_screen_visible(module_, frame_, value != 0); }
    void update_frame() { scene_.update_screen_frame(module_, frame_); }
    std::size_t child_count() const { return 2; }
    void dispatch_child(std::size_t i, frontend::ControlEvent &event) {
        images_.at(i)->receive_event(event, context_.queue);
    }

  private:
    PreMissionImage &image() { return *images_[focus_ == 2 ? 1 : 0]; }
    const MenuScene &scene_;
    HMODULE module_;
    const frontend::MenuControlDefinition &definition_;
    EngineFrame *frame_;
    MenuWidgetContext &context_;
    std::function<MenuTextObject *(unsigned)> lookup_;
    std::array<std::unique_ptr<PreMissionImage>, 2> images_;
    frontend::ControlProperties properties_;
    frontend::ControlEventBindings bindings_;
    unsigned focus_ = 1;
};
} // namespace hd2::host
#endif
