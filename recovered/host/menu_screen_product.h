#ifndef HD2_HOST_MENU_SCREEN_PRODUCT_H
#define HD2_HOST_MENU_SCREEN_PRODUCT_H

#include "host/menu_screen_events.h"

#include <deque>
#include <optional>

namespace hd2::host {

// A registered screen's native lifetime: its copied definition and children
// remain valid while focus, frame and event endpoints borrow them. The scene,
// text store and event queue are session services and must outlive this object.
class MenuScreenProduct {
public:
    // Allocation and definition binding are separate original factory stages.
    MenuScreenProduct(const MenuScene& scene, HMODULE module, bool confirmation,
                      std::vector<frontend::MenuControlRole> available,
                      std::function<MenuTextObject*(std::uint32_t)> text_lookup,
                      frontend::ControlEventDestination& queue, EngineFrame* model_root = nullptr, MenuWidgetContext* widgets = nullptr,
                      std::function<std::unique_ptr<MenuControl>(const frontend::MenuControlDefinition&,EngineFrame*,MenuTextObject&)> extra = {})
        : scene_(scene), module_(module), confirmation_(confirmation), available_(std::move(available)),
          text_lookup_(std::move(text_lookup)), queue_(queue), model_root_(model_root), widgets_(widgets), extra_(std::move(extra)) {}

    MenuScreenProduct(const MenuScreenProduct&) = delete;
    MenuScreenProduct& operator=(const MenuScreenProduct&) = delete;

    bool bind_definition(const frontend::MenuScreenDefinition& source) {
        if (definition_) throw std::logic_error("Screen definition bound twice");
        definition_.emplace(copy_definition(source, strings_));
        controls_ = make_menu_controls(scene_, module_, *definition_, confirmation_, available_, text_lookup_, model_root_, widgets_, extra_);
        focus_.emplace(*definition_, queue_);
        controls_->bind_screen(*focus_);
        frame_.emplace(scene_, module_, confirmation_, *focus_, model_root_);
        events_.emplace(*definition_, *frame_, queue_);
        bind_screen_messages(*events_, *definition_, *controls_);
        return true;
    }
    void initialize_records(std::vector<frontend::ControlEvent>& records) {
        events().initialize_records(records);
    }
    const frontend::MenuScreenDefinition& definition() const { return definition_.value(); }
    EngineFrame* model_frame() const { return model_root_; }
    MenuScreenControls& controls() { return *controls_; }
    MenuFocus& focus() { return focus_.value(); }
    MenuScreenEvents& events() { return events_.value(); }

private:
    static const char* retain(const char* value, std::deque<std::string>& strings) {
        if (!value) return nullptr;
        strings.emplace_back(value);
        return strings.back().c_str();
    }
    static void retain_control(frontend::MenuControlDefinition& control,
                               std::deque<std::string>& strings) {
        control.frame = retain(control.frame, strings);
        control.normal = retain(control.normal, strings);
        control.active = retain(control.active, strings);
        for (auto& child : control.children) retain_control(child, strings);
    }
    static frontend::MenuScreenDefinition copy_definition(
            const frontend::MenuScreenDefinition& source, std::deque<std::string>& strings) {
        auto result = source;
        result.label = retain(result.label, strings);
        result.model = retain(result.model, strings);
        for (auto& control : result.controls) retain_control(control, strings);
        return result;
    }
    const MenuScene& scene_;
    HMODULE module_;
    bool confirmation_;
    std::vector<frontend::MenuControlRole> available_;
    std::function<MenuTextObject*(std::uint32_t)> text_lookup_;
    frontend::ControlEventDestination& queue_;
    EngineFrame* model_root_;
    MenuWidgetContext* widgets_;
    std::function<std::unique_ptr<MenuControl>(const frontend::MenuControlDefinition&,EngineFrame*,MenuTextObject&)> extra_;
    std::deque<std::string> strings_;
    std::optional<frontend::MenuScreenDefinition> definition_;
    std::unique_ptr<MenuScreenControls> controls_;
    std::optional<MenuFocus> focus_;
    std::optional<MenuScreenFrame> frame_;
    std::optional<MenuScreenEvents> events_;
};

}

#endif
