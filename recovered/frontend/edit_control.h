#ifndef HD2_FRONTEND_EDIT_CONTROL_H
#define HD2_FRONTEND_EDIT_CONTROL_H

#include "frontend/edit_character_target.h"
#include "frontend/options_widget_base.h"

namespace hd2::frontend {

enum class EditControlKind { base, plain, visual, records };

// Source input/state composition for the four tables sharing00662490. This
// builds no frames/text geometry. Backend provides original text/layout,
// visual-pair operations, row visibility/count/offset and numeric CRT parsing.
// It and the queue outlive this shared-owned runtime and queued capture events.
template<class Backend>
class EditControl : public OptionsWidgetBase<EditControl<Backend>, Backend>,
                    public FrontendCaptureOwner,
                    public std::enable_shared_from_this<EditControl<Backend>> {
    using Base = OptionsWidgetBase<EditControl<Backend>, Backend>;
public:
    EditControl(std::uint32_t identifier, EditControlKind kind, Backend& backend, ControlEventDestination& queue)
        : Base(identifier, backend, queue), kind_(kind), bridge_{*this} {
        // Plain/visual +2c sets record zero after attached-text binding.
        if ((kind == EditControlKind::plain || kind == EditControlKind::visual) && backend.has_text()) state.record_index = 0;
    }
    EditCharacterState state;
    bool character(std::uint32_t value) override { return dispatch_edit_character(state, bridge_, value); }
    void acquire(std::uint32_t) override { acquire_edit_characters(state, bridge_); }
    bool release(std::uint32_t argument) override {
        if (!release_edit_characters(state, bridge_, argument)) return false;
        if (kind_ == EditControlKind::visual) clamp_number(); // 0065b2c0.
        return true;
    }
    bool request_activation() {
        if (state.active) return false;
        queue_capture(0x020000bf);
        return true;
    }
    bool request_deactivation() {
        if (!state.active) return false;
        queue_capture(0x020000c0);
        Base::emit(0x04002000, 0);
        return true;
    }
    void set_disabled(std::uint8_t value) { set_text_control_disabled(*this, value); }
    // 006621e0: caret ticks only while edit capture and attached text exist.
    void update(std::uint32_t milliseconds) {
        if (state.active && Base::backend_.has_text()) Base::backend_.object().update_caret(milliseconds);
    }
    // 0064da60 / 0065acb0 / 0065b030 / 0065c370.
    int enter_focus(std::uint32_t) {
        if (kind_ == EditControlKind::records) return 1;
        if (kind_ != EditControlKind::base && Base::disabled()) return 0;
        if (kind_ == EditControlKind::visual) {
            Base::backend_.set_active_visible(true); Base::backend_.set_normal_visible(false);
            Base::backend_.update_active(); Base::backend_.update_normal();
        }
        Base::emit(0x04003000, 0);
        return kind_ == EditControlKind::base ? 7 : 1;
    }
    // 00662770 / 0065acd0 / 0065b090 / 0065c380.
    int leave_focus(std::uint32_t) {
        if (kind_ == EditControlKind::plain && !Base::backend_.object().clear_hover_selection()) return 0;
        if (kind_ == EditControlKind::records) Base::backend_.object().clear_hover_selection();
        if (kind_ == EditControlKind::plain) Base::emit(0x04004000, 0);
        Base::emit(0x04004000, 0);
        if (kind_ == EditControlKind::visual) {
            Base::backend_.set_normal_visible(true); Base::backend_.set_active_visible(false);
            Base::backend_.update_normal(); Base::backend_.update_active();
            Base::emit(0x04004000, 0);
        }
        return 7;
    }
    // 00662790 / 0065ad00 / 0065b0f0 / 0065bf20.
    int activate(std::uint32_t) {
        if (kind_ == EditControlKind::plain) return 7;
        if (kind_ == EditControlKind::visual && Base::disabled()) return 0;
        if (kind_ == EditControlKind::records) {
            if (state.active) return 1;
            selected_record_ = Base::backend_.object().restore_hover_selection(ensure_visible());
            Base::queue_.enqueue({Base::properties_.word_58, 0x01000020,
                static_cast<std::uint32_t>(static_cast<std::int32_t>(Base::backend_.visible_row_count())), 0, 0, {}});
            Base::queue_.enqueue({Base::properties_.word_58, 0x0200002b, Base::backend_.scroll_offset(), 0, 0, {}});
            Base::backend_.refresh_rows();
            Base::emit(0x04001000, 0);
            return 1;
        }
        return request_activation() ? 1 : 0;
    }
    // 00662880 / 0065ad50 / 0065b150.
    int deactivate(std::uint32_t) {
        if (!request_deactivation()) return 0;
        if (kind_ == EditControlKind::plain || kind_ == EditControlKind::visual) {
            Base::backend_.object().clear_hover_selection();
            Base::backend_.object().restore_hover_selection(ensure_visible());
            if (kind_ == EditControlKind::visual) clamp_number();
        }
        return 1;
    }
    // 0065ad10 / 0065b110: toggle only for this control's exact pointer target.
    int click(std::uint32_t target) {
        if (Base::disabled() || target != Base::identifier()) return 0;
        return state.active ? deactivate(target) : activate(target);
    }
    int selected_record() const { return selected_record_; }
    void set_selected_record(int value) { selected_record_ = value; }

    bool dispatch_control_event(ControlEvent& event, ControlEventDestination& destination) {
        if ((event.code & 0x040ff000u) != 0) return Base::dispatch_text(event, destination);
        const auto code = event.code & 0x03000fffu;
        if (code == 0x02000016 && kind_ == EditControlKind::records) {
            // Focus-entry branch of0065b430; row/data messages remain list-widget work.
            state.record_index = selected_record_;
            Base::backend_.object().select_visible_record(static_cast<std::int16_t>(selected_record_), ensure_visible());
            Base::backend_.refresh_rows();
            request_activation();
            return true;
        }
        if (kind_ == EditControlKind::records) {
            // These are the record widget's own row/data branches in0065b430,
            // not base edit messages. They require its actual list backend.
            switch (code) {
            case 0x0100000b: Base::properties_.word_58 = event.argument; return true;
            case 0x0100006a: case 0x01000016: case 0x01000017:
            case 0x02000015: case 0x02000017: case 0x02000018:
            case 0x0200001b: case 0x0200001c: case 0x0200002b:
            case 0x0200002c: case 0x0200002f: case 0x02000030: case 0x02000076:
                return Base::backend_.dispatch_record_event(event, destination);
            default: break;
            }
        }
        if ((kind_ == EditControlKind::plain || kind_ == EditControlKind::visual) &&
            (code == 0x0200002c || code == 0x02000073)) {
            if (Base::backend_.has_text()) {
                if (code == 0x02000073) Base::backend_.object().inset = static_cast<std::uint8_t>(event.argument);
                if (kind_ == EditControlKind::visual) Base::backend_.layout_text_on_normal();
                else Base::backend_.layout_text_on_frame();
            }
            return true;
        }
        if (kind_ == EditControlKind::visual) {
            if (code == 0x010000a0) {
                minimum_ = widget_signed_word(event.argument); maximum_ = widget_signed_word(event.word_0c);
                ranged_ = minimum_ != maximum_;
                return true;
            }
            if (code == 0x01000078) {
                event.code = 0x01000014;
                if (state.numeric_only) event.text = std::make_shared<const std::string>(std::to_string(widget_signed_word(event.word_0c)));
                return dispatch_edit_event(event, destination);
            }
            if (code == 0x01000079 && state.numeric_only) {
                event.code = 0x0200002f;
                dispatch_edit_event(event, destination);
                event.code = 0x01000079;
                event.word_0c = static_cast<std::uint32_t>(Base::backend_.parse_integer(event.text));
                return true;
            }
        }
        // 0065ac30 bypasses the plain +38 no-op for programmatic focus entry.
        if (code == 0x02000016 && kind_ == EditControlKind::plain) { request_activation(); return true; }
        return dispatch_edit_event(event, destination);
    }
private:
    bool dispatch_edit_event(ControlEvent& event, ControlEventDestination& destination) {
        // 00662200: base edit field messages, exact mutable query sequencing.
        switch (event.code & 0x03000fffu) {
        case 0x0100006e:
            if (event.argument && state.capacity != event.argument) {
                state.capacity = event.argument;
                state.buffer.clear(); // Native initialized replacement for original uninitialized allocation.
            }
            return true;
        case 0x0100006f: state.numeric_only = event.argument != 0; return true;
        case 0x01000074: return true;
        case 0x0100009b:
            if (event.word_0c && !event.text) throw std::invalid_argument("Edit exclusions require typed native text");
            state.excluded_characters = event.text;
            return true;
        case 0x02000016: activate(0); return true;
        case 0x0200002f:
            Base::backend_.object().select_record(0);
            event.text = Base::backend_.object().selected_text();
            Base::backend_.object().select_record(-1);
            return true;
        case 0x020000ab: if (state.active) deactivate(0); return true;
        default: return Base::dispatch_text(event, destination);
        }
    }
    auto ensure_visible() { return [this](std::int16_t index) { Base::backend_.ensure_record_visible(index); }; }
    void clamp_number() {
        if (!ranged_) return;
        ControlEvent query{0, 0x0200002f, 0, 0, 0, {}};
        EmittingControlDestination<EditControl> destination{*this, &this->queue_};
        dispatch_edit_event(query, destination);
        const auto value = Base::backend_.parse_integer(query.text);
        const auto bound = value < minimum_ ? minimum_ : maximum_;
        if (value < minimum_ || value > maximum_) {
            ControlEvent replacement{0, 0x01000014, 13, 0, 0, {}};
            replacement.text = std::make_shared<const std::string>(std::to_string(bound));
            dispatch_edit_event(replacement, destination);
        }
    }
    void queue_capture(std::uint32_t code) {
        ControlEvent event{0, code, 0, 0, 0, {}};
        event.capture_owner = this->shared_from_this();
        Base::queue_.enqueue(event);
    }
    struct Bridge {
        EditControl& owner;
        TextObject& text() { return owner.backend_.object(); }
        bool wide_character_mode() const { return owner.backend_.wide_character_mode(); }
        bool is_digit(std::int32_t value) { return owner.backend_.is_digit(value); }
        void emit_event(std::uint32_t tag, std::uint32_t argument) { owner.emit(tag, argument); }
        void deactivate(std::uint32_t value) { owner.deactivate(value); }
    };
    EditControlKind kind_;
    Bridge bridge_;
    int selected_record_ = -1;
    bool ranged_ = false;
    std::int32_t minimum_ = 0, maximum_ = 0;
};

}
#endif
