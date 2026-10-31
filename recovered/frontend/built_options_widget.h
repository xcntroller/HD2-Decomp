#ifndef HD2_FRONTEND_BUILT_OPTIONS_WIDGET_H
#define HD2_FRONTEND_BUILT_OPTIONS_WIDGET_H

#include "frontend/options_widget_parts.h"
#include "frontend/slider_control.h"
#include "frontend/selector_control.h"
#include "frontend/toggle_control.h"
#include "frontend/options_observers.h"
#include "frontend/menu_definition.h"
#include <optional>

namespace hd2::frontend {

enum class OptionsObserverKind { graphics, slider, toggle, voice };

// Complete native product for kinds8/10/24: original visual constructors,
// owned internal UI objects, actual message/input runtimes and typed observers.
// Visuals owns the engine/text/input service bindings; model/text/queue outlive us.
template<class Visuals>
class BuiltOptionsWidget {
    using Self = BuiltOptionsWidget;
    using Part = OptionsWidgetPart<Visuals>;
public:
    using Frame = typename Visuals::Frame;
    BuiltOptionsWidget(MenuControlKind kind, std::uint32_t id, Visuals& visuals,
                       ControlEventDestination& queue)
        : kind_(kind), id_(id), visuals_(visuals), queue_(queue) {
        switch (kind_) {
        case MenuControlKind::slider: slider_.emplace(id,*this,queue); break;
        case MenuControlKind::selector: selector_.emplace(id,*this,queue); break;
        case MenuControlKind::toggle: toggle_.emplace(id,*this,queue); break;
        default: throw std::invalid_argument("Not an options widget");
        }
    }
    ~BuiltOptionsWidget() {
        lifetime_.reset();
        for (auto& cancel : listeners_) cancel();
        selector_.reset(); // Release selector item storage before internal parts.
        while (!parts_.empty()) parts_.pop_back(); // Original child teardown is reverse insertion order.
    }
    BuiltOptionsWidget(const BuiltOptionsWidget&) = delete;
    BuiltOptionsWidget& operator=(const BuiltOptionsWidget&) = delete;
    bool initialize_visuals() {
        auto root = visuals_.root(); visuals_.pick(root,id_);
        if (toggle_) {
            if (visuals_.children(root) != 7) return false;
            visuals_.layout_text_on_frame();
            const char* names[]{"yes_empty*","yes_full*","yesactived_full*","no_empty*","no_full*","noactived_full*","noactived_empty*"};
            std::array<Frame,7> frames{};
            for (std::size_t i = 0; i < frames.size(); ++i) { frames[i] = visuals_.find(root,names[i],0x10001); if (!frames[i]) return false; }
            for (int i = 0; i != 2; ++i) {
                auto p = std::make_unique<Part>(visuals_,root,id_ | (i+1),WidgetPartKind::toggle_half,queue_);
                p->initialize_half({frames[i*3],frames[i*3+1],frames[i*3+2],frames[6]});
                parts_.push_back(std::move(p));
            }
            parts_[0]->select(true); parts_[1]->select(false);
        } else {
            // Native guards reject the original incomplete/null arrow state.
            for (int i = 0; i != 2; ++i) {
                auto frame = visuals_.find(root,i ? "barrow down*" : "barrow up*",0x10020);
                if (!frame) return false;
                auto p = std::make_unique<Part>(visuals_,frame,id_ | (i+1),WidgetPartKind::arrow,queue_);
                if (!p->initialize()) return false;
                p->bind({id_,slider_ ? (i ? 0x05006024u : 0x05006023u) : (i ? 0x0600601cu : 0x0600601bu),0,0,0,{}});
                parts_.push_back(std::move(p));
                if (slider_) subscribe(id_ | (i+1),[this,i](std::uint32_t) { parts_.at(i)->tick(); });
            }
            if (slider_) {
                if (auto frame = visuals_.find(root,"slider*",0x10020)) {
                    auto p = std::make_unique<Part>(visuals_,frame,id_ | 3,WidgetPartKind::thumb,queue_);
                    if (!p->initialize()) return false;
                    p->bind({id_,0x05008025,0,0,0,{}});
                    p->bind({0,0x01000021,0,widget_float_bits(0),0,{}});
                    parts_.push_back(std::move(p));
                }
                subscribe(id_,[this](std::uint32_t delta) { slider_->tick(delta); });
                slider_->set_disabled(1);
            } else { visuals_.layout_text_on_frame(); selector_->set_disabled(1); }
        }
        return true;
    }
    void initialize(const std::vector<ControlEvent>& records) { visit([&](auto& runtime) { runtime.initialize(records); }); }
    std::uint32_t identifier() const { return id_; }
    bool dispatch(ControlEvent& event) { return visit([&](auto& runtime) { return runtime.dispatch(event); }); }
    bool receive_event(ControlEvent& event, ControlEventDestination& upstream) { return visit([&](auto& runtime) { return runtime.receive_event(event,upstream); }); }
    bool eligible() const { return visit([](auto& runtime) { return runtime.eligible(); }); }
    bool disabled() const { return visit([](auto& runtime) { return runtime.disabled(); }); }
    void update(std::uint32_t delta) { if (slider_) slider_->tick(delta); } // Original focused and listener phases both call +1c.
    const ControlProperties& properties() const { return visit([](auto& r) -> const ControlProperties& { return r.properties(); }); }
    int deactivate() { visit([](auto& r) { r.emit_input(0x04002000,0); }); return 7; }
    int enter_focus(std::uint32_t target) { return visit([&](auto& runtime) { return runtime.enter_focus(target); }); }
    int leave_focus(std::uint32_t target) { return visit([&](auto& runtime) { return runtime.leave_focus(target); }); }
    int pointer_focus(std::uint32_t target) { return visit([&](auto& runtime) { return runtime.pointer_focus(target); }); }
    int activate() { if (toggle_) return toggle_->activate(); if (slider_) { slider_->emit_input(0x04001000,0); return 7; } return 1; }
    int click() {
        if (toggle_) return toggle_->click();
        // Base0064DBC0 emits04012000 then virtual activation; selectors +38 is RET1.
        visit([](auto& runtime) { runtime.emit_input(0x04012000,0); });
        return activate();
    }
    int pointer_event(ScreenPointerCallback callback,std::uint32_t target) {
        if (callback == ScreenPointerCallback::focus) return pointer_focus(target);
        if (callback == ScreenPointerCallback::click) return click();
        if (callback == ScreenPointerCallback::press) {
            if (slider_) return slider_->press(target);
            if (selector_) return selector_->press(target);
        }
        if (callback == ScreenPointerCallback::drag && slider_) return slider_->drag(target);
        return visit([&](auto& runtime) { struct Emitter { decltype(runtime)& r; void emit_event(std::uint32_t tag,std::uint32_t arg) { r.emit_input(tag,arg); } } emitter{runtime}; return button_pointer_event(emitter,callback); });
    }
    int wheel(std::int32_t delta) { if (slider_) return slider_->wheel(delta); return visit([](auto& r) { r.emit_input(0x04011000,0); return 7; }); }
    template<class Settings>
    void connect(Settings& settings, std::uint8_t screen, OptionsObserverKind observer) {
        if ((observer == OptionsObserverKind::slider && !slider_) ||
            (observer == OptionsObserverKind::graphics && !selector_ && !toggle_) ||
            ((observer == OptionsObserverKind::toggle || observer == OptionsObserverKind::voice) && !toggle_))
            throw std::invalid_argument("Options observer/widget mismatch");
        changed_ = [this,&settings,screen,observer](std::uint32_t argument) {
            switch (observer) {
            case OptionsObserverKind::slider: return options_slider_observer(settings,screen,this);
            case OptionsObserverKind::graphics: return options_graphics_observer(settings,screen,this,argument,queue_);
            case OptionsObserverKind::toggle: return options_toggle_observer(settings,screen,this,argument);
            case OptionsObserverKind::voice: return options_voice_observer(settings,screen,argument,queue_);
            }
            return false;
        };
        auto event = observer_control_event(0,0x04010000,this,changed);
        dispatch(event);
    }
    // Backend for the existing three runtimes; all operations reach owned parts
    // or real text/engine/input services, never successful placeholder services.
    bool has_text() const { return !slider_; }
    auto& object() { return visuals_.object(); }
    void set_localized(std::uint32_t id,std::uint32_t style,std::int32_t index) { visuals_.set_localized(id,style,index); }
    void append_localized(std::uint32_t id,std::uint32_t style) { visuals_.append_localized(id,style); }
    SharedText localize(std::uint32_t id) { return visuals_.localize(id); }
    void layout_text_on_frame() { visuals_.layout_text_on_frame(); }
    void set_frame_direction(float x,float y,float z) { visuals_.direction(visuals_.root(),{x,y,z}); }
    void set_frame_visible(std::uint8_t value) { visuals_.visible(visuals_.root(),value); }
    void update_frame() { visuals_.update(visuals_.root()); }
    std::size_t child_count() const { return parts_.size(); }
    bool receive_child(std::size_t index,ControlEvent& event,ControlEventDestination&) { return parts_.at(index)->dispatch(event); }
    bool parent_disabled() const { return visuals_.parent_disabled(); }
    bool has_inner() const { return parts_.size() == 3; }
    bool has_part(int part) const { return valid_part(part); }
    bool valid_part(int part) const { return part >= 0 && static_cast<std::size_t>(part) < parts_.size(); }
    int part_for_target(std::uint32_t target) const { for (std::size_t i = 0; i < parts_.size(); ++i) if ((((id_ | (i+1)) ^ target) & 15) == 0) return static_cast<int>(i); return -1; }
    void set_part_disabled(int part,std::uint8_t value) { parts_.at(part)->set_disabled(value); }
    int enter_part(int part,std::uint32_t) { return parts_.at(part)->enter(); }
    int leave_part(int part,std::uint32_t) { return parts_.at(part)->leave(); }
    int move_part(int part,std::uint32_t) { parts_.at(part)->emit_event(0x04005000,0); return 7; }
    int pointer_part(int part,ScreenPointerCallback callback,std::uint32_t) { return parts_.at(part)->pointer(callback); }
    bool receive_inner(ControlEvent& event) { if (!has_inner()) throw std::logic_error("Slider query requires a thumb"); return parts_[2]->dispatch(event); }
    bool receive_linked(std::uint32_t target,ControlEvent& event,ControlEventDestination&) { if (!target) return false; event.target=target; return visuals_.dispatch_immediate(event); }
    bool primary_held() const { return visuals_.held(); }
    std::optional<float> track_percent() { return visuals_.track_percent(); }
    void text_focus(bool focus) { visuals_.text_focus(focus); }
    void select_part(int part,bool selected) { parts_.at(part)->select(selected); }
    void layout_part(int part,bool attached) { parts_.at(part)->reparent(attached ? visuals_.root() : Frame{}); }
private:
    template<class F> decltype(auto) visit(F&& f) { if (slider_) return f(*slider_); if (selector_) return f(*selector_); return f(*toggle_); }
    template<class F> decltype(auto) visit(F&& f) const { if (slider_) return f(*slider_); if (selector_) return f(*selector_); return f(*toggle_); }
    void subscribe(std::uint32_t token,std::function<void(std::uint32_t)> update) { auto weak = std::weak_ptr<int>(lifetime_);
        listeners_.push_back(visuals_.subscribe(token,[weak,update=std::move(update)](std::uint32_t delta) { if (weak.lock()) update(delta); })); }
    static bool changed(void* receiver,void*,void*,std::uint32_t,std::uint32_t argument) { auto& self=*static_cast<Self*>(receiver); return self.changed_(argument); }
    MenuControlKind kind_; std::uint32_t id_; Visuals& visuals_; ControlEventDestination& queue_;
    std::shared_ptr<int> lifetime_ = std::make_shared<int>(0);
    std::vector<std::unique_ptr<Part>> parts_;
    std::vector<std::function<void()>> listeners_;
    std::optional<SliderControl<Self>> slider_;
    std::optional<SelectorControl<Self>> selector_;
    std::optional<ToggleControl<Self>> toggle_;
    std::function<bool(std::uint32_t)> changed_;
};

}
#endif
