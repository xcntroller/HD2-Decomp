#ifndef HD2_FRONTEND_OPTIONS_WIDGET_PARTS_H
#define HD2_FRONTEND_OPTIONS_WIDGET_PARTS_H

#include "frontend/control_runtime.h"
#include "frontend/button_input.h"
#include "frontend/options_widget_base.h"
#include <array>
#include <algorithm>
#include <cmath>
#include <functional>

namespace hd2::frontend {

//00664570/00664980/006649F0. Frame bounds are the frame's +10/+20
// world box (not the visual-only +170 box). Keep the signed vertical extent.
struct WidgetTravel {
    float left, top, right, bottom, width, height;
    bool horizontal;
    static WidgetTravel from(const std::array<float, 4>& knob,
                             const std::array<float, 4>& track,
                             const std::array<float, 3>& offset) {
        const float half_x = (knob[2] - knob[0]) * .5f;
        const float half_y = (knob[1] - knob[3]) * .5f;
        WidgetTravel r{std::clamp(track[0] + half_x, 0.f, 1.f) - offset[0],
                       std::clamp(track[1] - half_y, -.75f, 0.f) - offset[2],
                       std::clamp(track[2] - half_x, 0.f, 1.f) - offset[0],
                       std::clamp(track[3] + half_y, -.75f, 0.f) - offset[2], 0, 0, false};
        r.width = r.right - r.left; r.height = r.bottom - r.top;
        r.horizontal = !(r.width < r.height);
        return r;
    }
    float coordinate(float percent) const {
        const auto v = horizontal ? static_cast<long double>(width) * percent * .01f + left :
                                  bottom - static_cast<long double>(height) * percent * .01f;
        return clamp(static_cast<float>(v));
    }
    float clamp(float value) const {
        // Deliberately sequential, including inverted/zero travel.
        const auto lo = horizontal ? left : top, hi = horizontal ? right : bottom;
        if (value < lo) value = lo;
        if (hi < value) value = hi;
        return value;
    }
    float percentage(float coordinate) const {
        const auto extent = horizontal ? width : height;
        if (extent == 0) throw std::domain_error("Zero widget travel");
        return static_cast<float>((horizontal ? (static_cast<long double>(coordinate) - left) / extent :
                                               (static_cast<long double>(bottom) - coordinate) / extent) * 100.f);
    }
};

enum class WidgetPartKind { arrow, thumb, toggle_half };

// Original internal tables00813F68/00815288/00814078. Frames are borrowed
// from the owning model. Native properties/bindings replace raw class ABI.
template<class Visuals>
class OptionsWidgetPart {
public:
    using Frame = typename Visuals::Frame;
    OptionsWidgetPart(Visuals& visuals, Frame root, std::uint32_t id,
                      WidgetPartKind kind, ControlEventDestination& queue)
        : visuals_(visuals), root_(root), id_(id), kind_(kind), queue_(queue) {}
    bool initialize() {
        visuals_.pick(root_, id_);
        if (kind_ == WidgetPartKind::toggle_half) throw std::logic_error("Toggle half requires four frames");
        if (kind_ == WidgetPartKind::arrow && visuals_.children(root_) != 3) return false;
        frames_[0] = visuals_.find(root_, "normal*", 0x10001);
        frames_[1] = visuals_.find(root_, "actived*", 0x10001);
        frames_[2] = visuals_.find(root_, kind_ == WidgetPartKind::arrow ? "inactived*" : "sliderbase*", 0x10001);
        for (int i = 0; i != 3; ++i) if (!frames_[i]) return false; // Native guard for original arrow dereference.
        if (kind_ == WidgetPartKind::arrow) {
            visuals_.pick(frames_[0], id_); show(0, 1);
            for (int i = 1; i != 3; ++i) { visuals_.align(frames_[i], frames_[0]); visuals_.visible(frames_[i], 0); visuals_.pick(frames_[i], id_); visuals_.update(frames_[i]); }
        } else {
            visuals_.world_position(frames_[0]);
            visuals_.align(frames_[1], frames_[0]); visuals_.visible(frames_[1], 0);
            visuals_.pick(frames_[0], id_); visuals_.pick(frames_[1], id_);
            visuals_.update(frames_[0]); visuals_.update(frames_[1]); show(2, 0);
            travel_ = WidgetTravel::from(visuals_.bounds(frames_[0]), visuals_.bounds(frames_[2]), visuals_.local_position(frames_[0]));
            set_percentage(50);
        }
        return true;
    }
    void initialize_half(const std::array<Frame, 4>& frames) {
        frames_ = frames; root_ = frames_[0];
        visuals_.visible(frames_[0], 1); visuals_.pick(frames_[0], id_); visuals_.update(frames_[0]);
        for (int i : {1, 2}) { visuals_.visible(frames_[i], 0); visuals_.pick(frames_[i], id_); visuals_.align(frames_[i], frames_[0]); visuals_.update(frames_[i]); }
        // Original does not touch the fourth/shared active-empty frame here.
    }
    void bind(const ControlEvent& event) { auto copy = event; dispatch(copy); }
    bool dispatch(ControlEvent& event) {
        EmittingControlDestination<OptionsWidgetPart> destination{*this, &queue_};
        return dispatch_control_event(event, destination);
    }
    bool dispatch_control_event(ControlEvent& event, ControlEventDestination& destination) {
        if (kind_ == WidgetPartKind::thumb && !(event.code & 0x040ff000u)) {
            switch (event.code & 0x03000fffu) {
            case 0x01000021: set_percentage(widget_float_word(event.word_0c)); return true;
            case 0x01000022: event.word_0c = widget_float_bits(percent_); return true;
            }
        }
        class State : public ControlStateRuntime<OptionsWidgetPart> {
        public:
            State(OptionsWidgetPart& p, ControlEventDestination& d) : ControlStateRuntime<OptionsWidgetPart>(p,p.properties_,p.bindings_,d), part(p) {}
            void set_disabled(std::uint8_t v) { part.set_disabled(v); }
        private: OptionsWidgetPart& part;
        } state{*this,destination};
        return dispatch_control_state_event(state,event);
    }
    void set_disabled(std::uint8_t value) {
        if (kind_ == WidgetPartKind::toggle_half) {
            if (value == properties_.disabled) return;
            properties_.disabled = value;
            visuals_.alpha(frames_[0], value ? .4f : 1.f); visuals_.alpha(frames_[1], value ? .4f : 1.f);
        } else if (kind_ == WidgetPartKind::arrow) {
            properties_.disabled = value;
            visuals_.visible(frames_[0], value == 0); visuals_.visible(frames_[1], 0); visuals_.visible(frames_[2], value);
            for (int i = 0; i != 3; ++i) visuals_.update(frames_[i]);
        } else {
            properties_.disabled = value;
            if (value) { visuals_.visible(frames_[0], 0); visuals_.visible(frames_[1], 0); visuals_.update(frames_[0]); visuals_.update(frames_[1]); }
            else show(0, 1); // Enabling does not clear an already active visual.
        }
    }
    int enter() {
        if (kind_ == WidgetPartKind::toggle_half) {
            if (properties_.disabled || focused_) return 0;
            if (empty_) { show(0, 0); active_empty(1); } else { show(1, 0); show(2, 1); }
            focused_ = true; return 1;
        }
        if (!properties_.disabled) {
            visuals_.visible(frames_[1], 1); visuals_.visible(frames_[0], 0);
            if (kind_ == WidgetPartKind::arrow) { visuals_.visible(frames_[2], 0); visuals_.update(frames_[2]); }
            visuals_.update(frames_[1]); visuals_.update(frames_[0]);
            if (kind_ == WidgetPartKind::thumb) emit(0x04003000);
            return 1;
        }
        return kind_ == WidgetPartKind::thumb ? 1 : 0;
    }
    int leave() {
        if (kind_ == WidgetPartKind::toggle_half) {
            if (!focused_) return 0;
            if (empty_) { show(0, 1); show(3, 0); } else { show(1, 1); show(2, 0); }
            focused_ = false; return 1;
        }
        repeat_ = false;
        if (!properties_.disabled || kind_ == WidgetPartKind::thumb) {
            visuals_.visible(frames_[0], 1); visuals_.visible(frames_[1], 0);
            if (kind_ == WidgetPartKind::arrow) visuals_.visible(frames_[2], 0);
            visuals_.update(frames_[0]); visuals_.update(frames_[1]);
            if (kind_ == WidgetPartKind::arrow) visuals_.update(frames_[2]);
            else emit(0x04004000);
        }
        return 1;
    }
    int pointer(ScreenPointerCallback callback) {
        if (kind_ == WidgetPartKind::arrow) {
            if (callback == ScreenPointerCallback::press) { if (!properties_.disabled) { repeat_ = true; deadline_ = 300; emit(0x04006000); } return 1; }
            if (callback == ScreenPointerCallback::release) { repeat_ = false; return 1; }
            if (callback == ScreenPointerCallback::click) { emit(0x04012000); return 1; }
        }
        if (callback == ScreenPointerCallback::click) {
            if (kind_ == WidgetPartKind::toggle_half) { if (!focused_) return 0; if (empty_) { show(3,0); show(2,1); } else { show(2,0); active_empty(1); } return 1; }
            emit(0x04012000); emit(0x04001000); return 7;
        }
        if (kind_ == WidgetPartKind::thumb && callback == ScreenPointerCallback::drag) {
            if (drag()) emit(0x04008000);
            return 1;
        }
        return button_pointer_event(*this, callback);
    }
    void tick() { //006642B0 compares against input idle+40, not frame delta.
        if (!repeat_ || widget_signed_word(deadline_) > widget_signed_word(visuals_.idle())) return;
        if (!visuals_.held()) { repeat_ = false; deadline_ = 300; return; }
        deadline_ = 0xffffffffu; emit(0x04006000);
    }
    void select(bool value) { //00663EE0 stores the INVERSE selected byte.
        const bool empty = !value;
        if (empty_ == empty) return;
        empty_ = empty;
        if (focused_) {
            if (empty_) { visuals_.visible(frames_[3], 1); visuals_.pick(frames_[3], id_); visuals_.align(frames_[3], frames_[0]); visuals_.visible(frames_[2], 0); }
            else { visuals_.visible(frames_[2], 1); visuals_.visible(frames_[3], 0); }
            visuals_.update(frames_[2]); visuals_.update(frames_[3]);
        } else { visuals_.visible(frames_[0], empty_); visuals_.visible(frames_[1], !empty_); visuals_.update(frames_[0]); visuals_.update(frames_[1]); }
    }
    void reparent(Frame parent) { for (auto frame : frames_) visuals_.link(frame, parent, 1); }
    void emit_event(std::uint32_t tag, std::uint32_t) { emit(tag); }
    bool activate_control() { emit(0x04001000); return true; }
    bool has_text() const { return false; }
    void set_text_disabled(std::uint8_t) { throw std::logic_error("Internal part has no attached text"); }
    void set_frame_visible(std::uint8_t v) { visuals_.visible(root_, v); }
    void update_frame() { visuals_.update(root_); }
    std::size_t child_count() const { return 0; }
    void dispatch_child(std::size_t, ControlEvent&) { throw std::out_of_range("Internal part has no UI children"); }
private:
    void emit(std::uint32_t tag) { EmittingControlDestination<OptionsWidgetPart> d{*this,&queue_}; dispatch_tagged_control_events(bindings_,this,tag,0,d); }
    void show(int i, std::uint8_t visible) { visuals_.visible(frames_[i],visible); visuals_.update(frames_[i]); }
    void active_empty(std::uint8_t v) { visuals_.visible(frames_[3],v); visuals_.pick(frames_[3],id_); visuals_.align(frames_[3],frames_[0]); visuals_.update(frames_[3]); }
    void position(float coordinate) { auto p = visuals_.world_position(root_); p[travel_.horizontal ? 0 : 1] = coordinate; visuals_.position(root_,p); visuals_.update(root_); }
    void set_percentage(float percent) {
        if (!std::isfinite(percent)) throw std::domain_error("Nonfinite widget percentage");
        if (percent_ == percent) return;
        percent_ = std::clamp(percent,0.f,100.f); position(travel_.coordinate(percent_));
    }
    bool drag() {
        const auto delta = visuals_.motion();
        const auto axis = travel_.horizontal ? 0u : 1u;
        if (delta[axis] == 0) return false;
        const auto pointer = visuals_.pointer(); const auto offset = visuals_.local_position(frames_[0]);
        const auto coordinate = travel_.clamp(pointer[axis] - offset[travel_.horizontal ? 0 : 2]);
        position(coordinate); const auto next = travel_.percentage(coordinate);
        if (next == percent_) return false;
        percent_ = next; return true;
    }
    Visuals& visuals_; Frame root_; std::uint32_t id_; WidgetPartKind kind_; ControlEventDestination& queue_;
    std::array<Frame,4> frames_{};
    ControlProperties properties_; ControlEventBindings bindings_;
    WidgetTravel travel_{}; float percent_ = 0;
    bool repeat_ = false, focused_ = false, empty_ = true;
    std::uint32_t deadline_ = 0;
};

}
#endif
