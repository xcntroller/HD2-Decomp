#ifndef HD2_FRONTEND_SLIDER_CONTROL_H
#define HD2_FRONTEND_SLIDER_CONTROL_H

#include "frontend/options_widget_base.h"
#include "frontend/pointer_dispatch.h"

#include <cmath>
#include <limits>

namespace hd2::frontend {

struct SliderWheelSteps { std::uint32_t count; std::uint32_t code; };
// 006658c0: round magnitude UP in 120-unit steps; positive wheel decrements.
// Unsigned magnitude preserves the original behavior for INT_MIN as well.
inline SliderWheelSteps slider_wheel_steps(std::int32_t delta) {
    if (!delta) return {0, 0};
    const auto word = static_cast<std::uint32_t>(delta);
    const auto magnitude = delta < 0 ? 0u - word : word;
    return {(magnitude - 1u) / 120u + 1u, delta > 0 ? 0x01000023u : 0x01000024u};
}

// Kind eight range/value/message/input subset. The complete part factory and concrete
// track/input bindings are composed by built_options_widget.h / host backend.
template<class Backend>
class SliderControl : public OptionsWidgetBase<SliderControl<Backend>, Backend> {
    using Base = OptionsWidgetBase<SliderControl<Backend>, Backend>;
public:
    using Base::Base;
    std::uint32_t range() const { return range_; }
    std::uint32_t position() const { return position_; }
    std::uint32_t step() const { return step_; }
    float scale() const { return scale_; }
    int focused_part() const { return focused_; }
    bool repeating() const { return repeat_; }
    std::int32_t repeat_timer() const { return widget_signed_word(repeat_timer_); }

    // 00665010. Signed/unsigned comparisons and asymmetric range limits are
    // intentional. Position==range is accepted by a programmatic set.
    bool dispatch_control_event(ControlEvent& event, ControlEventDestination& destination) {
        if ((event.code & 0x040ff000u) != 0) {
            this->dispatch_base(event, destination);
            return true;
        }
        switch (event.code & 0x03000fffu) {
        case 0x01000020:
            if (widget_signed_word(event.argument) < 2) {
                range_ = 0; position_ = 0; scale_ = 100;
                set_disabled(1);
            } else {
                if (range_ == 0) { range_ = event.argument; set_disabled(0); }
                range_ = event.argument;
                scale_ = 100.0f / static_cast<float>(widget_signed_word(range_ - 1u));
                if (widget_signed_word(position_) >= widget_signed_word(range_)) position_ = range_ - 1u;
            }
            refresh_position();
            break;
        case 0x01000022:
            this->backend_.receive_inner(event); // Direct +6c; query mutates caller.
            break;
        case 0x01000023:
            if (widget_signed_word(position_) > 0) {
                position_ -= step_;
                if (widget_signed_word(position_) < 0) position_ = 0;
                changed_step(destination);
            }
            break;
        case 0x01000024:
            if (widget_signed_word(position_) < widget_signed_word(range_ - 1u)) {
                position_ += step_;
                if (widget_signed_word(position_) > widget_signed_word(range_)) position_ = range_;
                changed_step(destination);
            }
            break;
        case 0x01000025: {
            ControlEvent query{0, 0x01000022, 0, 0, 0, {}};
            this->backend_.receive_inner(query);
            const auto next = quantize(widget_float_word(query.word_0c));
            if (next == position_) return true;
            position_ = next;
            // Thumb movement queues a targeted message; arrow stepping uses
            // the separate immediate linked-control helper instead.
            if (this->properties_.word_58) {
                ControlEvent linked{this->properties_.word_58, 0x0200002b, position_, 0, 0, {}};
                destination.enqueue(linked);
            }
            this->emit(0x04010000, 0, destination);
            break;
        }
        case 0x01000080:
            if (event.argument) step_ = event.argument;
            break;
        case 0x0100008f: event.argument = position_; break;
        case 0x0200002b:
            if (event.argument <= range_) { position_ = event.argument; refresh_position(); }
            break;
        default: this->dispatch_base(event, destination); break;
        }
        return true;
    }
    // 00665760: enabling is refused while the parent is disabled or range is
    // zero. Disabling is unconditional and never clears repeat state here.
    void set_disabled(std::uint8_t value) {
        if (value == 0 && (this->backend_.parent_disabled() || range_ == 0)) return;
        this->set_disabled_state(value);
        this->backend_.set_part_disabled(0, value);
        this->backend_.set_part_disabled(1, this->disabled_byte());
        if (this->backend_.has_inner()) this->backend_.set_part_disabled(2, this->disabled_byte());
        refresh_position();
    }
    // 006657c0: thumb and arrow-only sliders have distinct update paths.
    void refresh_position() {
        if (this->disabled()) return;
        if (this->backend_.has_inner()) {
            const float percent = static_cast<float>(widget_signed_word(position_)) * scale_;
            ControlEvent update{0, 0x01000021, 0, widget_float_bits(percent), 0, {}};
            dispatch_copied_control_event(update, [&](ControlEvent& copy) { return this->backend_.receive_inner(copy); });
        } else {
            if (this->backend_.has_part(0)) this->backend_.set_part_disabled(0, position_ == 0);
            if (this->backend_.has_part(1))
                this->backend_.set_part_disabled(1, widget_signed_word(position_) >= widget_signed_word(range_ - 1u));
        }
    }
    int wheel(std::int32_t delta) {
        const auto steps = slider_wheel_steps(delta);
        for (std::uint32_t i = 0; i < steps.count; ++i) {
            const ControlEvent event{0, steps.code, 0, 0, 0, {}};
            dispatch_copied_control_event(event, [&](ControlEvent& copy) { return this->dispatch(copy); });
        }
        return 1;
    }
    // 00665430/4b0 and 00665500. Entry first offers leave to old focus and
    // maps a veto to seven. This differs from generic group entry.
    int enter_focus(std::uint32_t target) {
        if (this->backend_.valid_part(focused_) && this->backend_.leave_part(focused_, target) == 0) return 7;
        focused_ = this->backend_.part_for_target(target);
        if (!this->backend_.valid_part(focused_)) return 0;
        return this->backend_.enter_part(focused_, target);
    }
    int leave_focus(std::uint32_t target) {
        if (this->backend_.valid_part(focused_)) {
            if (this->backend_.leave_part(focused_, target) == 0) return 0;
            focused_ = -1;
        }
        return 1;
    }
    int pointer_focus(std::uint32_t target) {
        if (!this->backend_.valid_part(this->backend_.part_for_target(target))) return leave_focus(target);
        const int old = focused_;
        if (old == this->backend_.part_for_target(target)) return this->backend_.move_part(old, target);
        if (leave_focus(target) == 0) return 1;
        return enter_focus(target);
    }
    // 006656d0 / 00665720 / 006659c0. Geometry/input sampling stays in the
    // backend; native repeat timer subtraction preserves 32-bit wrap and <0.
    int press(std::uint32_t target) {
        if (!this->disabled()) {
            if (this->backend_.valid_part(focused_))
                return this->backend_.pointer_part(focused_, ScreenPointerCallback::press, target);
            repeat_ = true; repeat_timer_ = 300; step_toward_pointer();
        }
        return 1;
    }
    int drag(std::uint32_t target) {
        if (this->disabled()) return 1;
        if (!this->backend_.valid_part(focused_)) return 0;
        return this->backend_.pointer_part(focused_, ScreenPointerCallback::drag, target);
    }
    void tick(std::uint32_t delta) {
        if (!repeat_) return;
        if (!this->backend_.primary_held()) { repeat_ = false; return; }
        repeat_timer_ -= delta;
        if (widget_signed_word(repeat_timer_) < 0) { step_toward_pointer(); repeat_timer_ = 50; }
    }
    void step_toward_pointer() {
        const auto percent = this->backend_.track_percent();
        if (!percent) return; // Nonvisual track; sampled input/geometry delegated.
        const auto desired = quantize(*percent < 0 ? 0 : *percent);
        if (desired == position_) return;
        if (desired < position_) --position_;
        if (position_ < desired) ++position_;
        refresh_position();
        synchronize_linked();
        this->emit(0x04010000, 0);
    }
private:
    std::uint32_t quantize(float percent) const {
        // Original x87 ratio is stored as double before floor(+0.5), then
        // converted to an integer. Reject nonfinite/out-of-domain native casts.
        const double ratio = static_cast<double>(static_cast<long double>(percent) / scale_ + 0.5L);
        const double rounded = std::floor(ratio);
        if (!std::isfinite(rounded) || rounded < -9223372036854775808.0 || rounded >= 9223372036854775808.0)
            throw std::domain_error("Slider position cannot be converted to an integer");
        auto result = static_cast<std::uint32_t>(static_cast<std::int64_t>(rounded));
        if (result > range_ - 1u) result = range_ - 1u;
        return result;
    }
    void synchronize_linked() {
        const ControlEvent event{0, 0x0200002b, position_, 0, 0, {}};
        dispatch_copied_control_event(event, [&](ControlEvent& copy) {
            return this->backend_.receive_linked(this->properties_.word_58, copy, this->queue_);
        });
    }
    void changed_step(ControlEventDestination& destination) {
        refresh_position();
        synchronize_linked();
        this->emit(0x04010000, 0, destination);
    }
    std::uint32_t range_ = 0, position_ = 0, step_ = 1;
    float scale_ = 0;
    int focused_ = -1;
    bool repeat_ = false;
    std::uint32_t repeat_timer_ = 0;
};

}

#endif
