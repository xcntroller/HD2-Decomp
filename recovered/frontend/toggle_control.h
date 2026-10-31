#ifndef HD2_FRONTEND_TOGGLE_CONTROL_H
#define HD2_FRONTEND_TOGGLE_CONTROL_H

#include "frontend/options_widget_base.h"

namespace hd2::frontend {

// Kind 24 value/message/focus subset. The yes/no visual products and text
// layout are built by the006541E0 native composition in built_options_widget.h.
template<class Backend>
class ToggleControl : public OptionsWidgetBase<ToggleControl<Backend>, Backend> {
    using Base = OptionsWidgetBase<ToggleControl<Backend>, Backend>;
public:
    using Base::Base;
    std::uint8_t value() const { return value_; }
    std::uint32_t focused_target() const { return focused_target_; }

    // 006549e0 / 00654a10. Update both halves before storing the new byte.
    void select(bool selected) {
        if (selected && value_ == 0) {
            this->backend_.select_part(0, true);
            this->backend_.select_part(1, false);
            value_ = 1;
        } else if (!selected && value_ != 0) {
            this->backend_.select_part(0, false);
            this->backend_.select_part(1, true);
            value_ = 0;
        }
    }
    // 006549a0: direct activation has no disabled gate; callers own that gate.
    void toggle(ControlEventDestination& destination) {
        select(value_ == 0);
        this->emit(0x04010000, value_, destination);
    }
    int activate() { EmittingControlDestination<ToggleControl> destination{*this, &this->queue_}; toggle(destination); return 1; }
    int click() { return activate(); } // 00654910/00654920, no 04012000 prefix.

    bool dispatch_control_event(ControlEvent& event, ControlEventDestination& destination) {
        if ((event.code & 0x040ff000u) == 0) {
            switch (event.code & 0x03000fffu) {
            case 0x0100006c: event.argument = value_; return true;
            case 0x0100006d:
                if (static_cast<std::uint8_t>(event.argument) != value_) toggle(destination);
                [[fallthrough]];
            case 0x01000076:
                // Reread after notification; observers can change the widget.
                if (static_cast<std::uint8_t>(event.argument) != value_) select(value_ == 0);
                return true;
            case 0x01000099:
                this->backend_.layout_part(0, false);
                this->backend_.layout_part(1, false);
                this->backend_.set_frame_direction(0, 0, -1);
                this->backend_.layout_part(0, true);
                this->backend_.layout_part(1, true);
                return true;
            }
        }
        this->dispatch_text(event, destination);
        return true;
    }
    // 006541a0: equality suppresses all work; optional internal controls.
    void set_disabled(std::uint8_t value) {
        if (value == this->disabled_byte()) return;
        set_text_control_disabled(*this, value);
        for (int part = 0; part != 2; ++part)
            if (this->backend_.has_part(part)) this->backend_.set_part_disabled(part, value);
    }
    // 00654840 / 006548b0 / 00654930. Preserve full target and low-nibble
    // half selection. Focus helper return values are ignored at this layer.
    int enter_focus(std::uint32_t target) {
        if (this->disabled()) return 0;
        if (this->has_text()) this->backend_.text_focus(true);
        enter_part(target);
        focused_target_ = target;
        this->emit(0x04003000, 0);
        return 1;
    }
    int leave_focus(std::uint32_t target) {
        if (this->has_text()) this->backend_.text_focus(false);
        leave_part(focused_target_, target);
        focused_target_ = 0;
        this->emit(0x04004000, 0);
        return 1;
    }
    int pointer_focus(std::uint32_t target) {
        if (focused_target_ && focused_target_ != target) {
            leave_part(focused_target_, target);
            enter_part(target);
            focused_target_ = target;
        }
        return 1;
    }
private:
    void enter_part(std::uint32_t target) {
        const auto part = target & 15;
        if (part == 1 || part == 2) this->backend_.enter_part(static_cast<int>(part - 1), target);
    }
    void leave_part(std::uint32_t old, std::uint32_t target) {
        const auto part = old & 15;
        if (part == 1 || part == 2) this->backend_.leave_part(static_cast<int>(part - 1), target);
    }
    std::uint8_t value_ = 1; // Factory 0065d150, kind 24 +78.
    std::uint32_t focused_target_ = 0;
};

}

#endif
