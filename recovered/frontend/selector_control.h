#ifndef HD2_FRONTEND_SELECTOR_CONTROL_H
#define HD2_FRONTEND_SELECTOR_CONTROL_H

#include "frontend/options_widget_base.h"
#include "frontend/pointer_dispatch.h"

#include <limits>

namespace hd2::frontend {

// Kind 10, 0064f550 and its focus/press methods. Native shared strings replace
// intrusive item storage. Frame/arrow construction0064F1C0 is composed by
// built_options_widget.h with actual attached text and owned internal parts.
template<class Backend>
class SelectorControl : public OptionsWidgetBase<SelectorControl<Backend>, Backend> {
    using Base = OptionsWidgetBase<SelectorControl<Backend>, Backend>;
public:
    using Base::Base;
    std::int32_t selected() const { return selected_; }
    std::size_t item_count() const { return items_.size(); }
    int focused_part() const { return focused_; }
    const SharedText& item(std::size_t index) const { return items_.at(index); }

    bool dispatch_control_event(ControlEvent& event, ControlEventDestination& destination) {
        if ((event.code & 0x040ff000u) != 0) {
            this->dispatch_text(event, destination);
            return true;
        }
        switch (event.code & 0x03000fffu) {
        case 0x01000067:
            if (items_.size() >= static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()))
                throw std::length_error("Too many selector items");
            items_.push_back(event.argument ? this->backend_.localize(event.argument) : event.text);
            if (selected_ < 0) { selected_ = 0; display_selected(); }
            if (this->disabled()) set_disabled(0);
            return true;
        case 0x01000068: {
            const auto index = widget_signed_word(event.argument);
            if (index >= 0 && static_cast<std::size_t>(index) < items_.size())
                items_.erase(items_.begin() + index);
            const auto last = static_cast<std::int32_t>(items_.size()) - 1;
            if (selected_ > last) selected_ = last;
            // Original tests the selected INDEX, not the remaining item count.
            // It neither refreshes the displayed caption nor emits a change.
            if (selected_ < 1) set_disabled(1);
            return true;
        }
        case 0x01000069:
        case 0x0100006c:
            event.argument = static_cast<std::uint32_t>(selected_);
            [[fallthrough]];
        case 0x0100006a: {
            const auto index = widget_signed_word(event.argument);
            if (index >= 0 && static_cast<std::size_t>(index) < items_.size()) event.text = items_[index];
            else event.argument = 0xffffffffu; // Invalid query retains its old payload.
            return true;
        }
        case 0x0100006d:
            if (event.argument >= items_.size()) return true;
            selected_ = static_cast<std::int32_t>(event.argument);
            break;
        case 0x0200001b:
            if (this->disabled()) return true;
            // 32-bit wrap before signed comparison, as in the original DEC.
            selected_ = widget_signed_word(static_cast<std::uint32_t>(selected_) - 1u);
            if (selected_ < 0) selected_ = static_cast<std::int32_t>(items_.size()) - 1;
            break;
        case 0x0200001c:
            if (this->disabled()) return true;
            selected_ = widget_signed_word(static_cast<std::uint32_t>(selected_) + 1u);
            if (static_cast<std::uint32_t>(selected_) >= items_.size()) selected_ = 0;
            break;
        case 0x02000065:
            items_.clear(); selected_ = -1; set_disabled(1); return true;
        default:
            this->dispatch_text(event, destination);
            return true;
        }
        display_selected();
        this->emit(0x04010000, static_cast<std::uint32_t>(selected_), destination);
        return true;
    }
    // 0064fa50: unconditional byte/text/arrow stores, including repeated values.
    void set_disabled(std::uint8_t value) {
        this->set_disabled_state(value);
        this->set_text_disabled(value);
        this->backend_.set_part_disabled(0, value);
        this->backend_.set_part_disabled(1, this->disabled_byte());
    }
    int activate() const { return 1; } // 0064fa00: no notification.
    int enter_focus(std::uint32_t target) {
        this->backend_.text_focus(true);
        focused_ = this->backend_.part_for_target(target);
        if (!this->backend_.valid_part(focused_)) return 1;
        return this->backend_.enter_part(focused_, target);
    }
    int leave_focus(std::uint32_t target) {
        this->backend_.text_focus(false);
        if (this->backend_.valid_part(focused_)) {
            if (this->backend_.leave_part(focused_, target) == 0) return 0;
            focused_ = -1;
        }
        return 1;
    }
    int pointer_focus(std::uint32_t target) {
        const int old = focused_;
        if (old == this->backend_.part_for_target(target)) {
            if (this->backend_.valid_part(old)) return this->backend_.move_part(focused_, target);
        } else if (leave_focus(target) != 0) return enter_focus(target);
        return 1;
    }
    int press(std::uint32_t target) {
        if (this->disabled()) return 1;
        if (!this->backend_.valid_part(focused_)) return 0;
        return this->backend_.pointer_part(focused_, ScreenPointerCallback::press, target);
    }
private:
    void display_selected() {
        // Original can dereference invalid storage when an empty selector is
        // manually enabled. Native storage reports that unsupported state.
        if (selected_ < 0 || static_cast<std::size_t>(selected_) >= items_.size())
            throw std::out_of_range("Enabled selector has no selected item");
        this->backend_.object().set(items_[selected_], 20, 0);
    }
    std::vector<SharedText> items_;
    std::int32_t selected_ = -1;
    int focused_ = -1;
};

}

#endif
