#ifndef HD2_FRONTEND_CONTROL_STATE_H
#define HD2_FRONTEND_CONTROL_STATE_H

#include "frontend/control_events.h"

namespace hd2::frontend {

// Client 0064d9f0, control +08. Preserve byte/bit semantics: noncanonical
// nonzero visible/focusable bytes are not equivalent to boolean true here.
inline std::uint8_t control_eligibility(std::uint8_t disabled,
                                       std::uint8_t visible,
                                       std::uint8_t focusable) {
    return static_cast<std::uint8_t>(focusable & visible & (disabled == 0 ? 1 : 0));
}

// State subset of base constructor 0064dcc0. This is portable storage, not
// the original object layout; derived controls may override these defaults.
struct ControlProperties {
    std::uint8_t disabled = 0;   // Original +64.
    std::uint8_t visible = 1;    // Original +65.
    std::uint8_t focusable = 1;  // Original +66.
    std::uint32_t word_58 = 0;
    std::uint32_t word_5c = 0xffffffffu;

    bool eligible() const {
        return control_eligibility(disabled, visible, focusable) != 0;
    }
};

// Client 0065c760 / 0065c790. Store state before engine side effects; emit only
// after visibility and update. Repeated show/hide still repeat those effects.
template<class Control>
void set_control_visibility(Control& control, std::uint8_t visible) {
    control.set_visible_state(visible);
    control.set_frame_visible(visible);
    control.update_frame();
    control.emit_event(visible ? 0x0400d000u : 0x0400c000u, 0);
}

// Client 00661cc0. The text binding is resolved after storing the control byte.
template<class Control>
void set_text_control_disabled(Control& control, std::uint8_t disabled) {
    control.set_disabled_state(disabled);
    if (control.has_text()) control.set_text_disabled(disabled);
}

// Client 0065cd40, base control message handler. Portable callbacks replace
// storage/virtual calls. Derived handlers must still process their own messages.
// Delegated helpers: register_observer = 0065cac0, unregister_observer = 0065c9a0,
// emit_event = 0065cf50. Observer/emission bodies are in control_events.cpp;
// the original object ABI is not reproduced here.
// All untagged messages, including unknown codes, are consumed by the original.
template<class Control>
bool dispatch_control_state_event(Control& control, ControlEvent& event) {
    if ((event.code & 0x040ff000u) != 0) {
        control.register_tagged_event(event); // Copies event, retaining payload.
        return true;
    }

    switch (event.code & 0x03000fffu) {
    case 0x01000001:
        if (!control.disabled()) control.set_disabled(1); // Virtual +7c.
        break;
    case 0x01000002:
        if (control.disabled()) control.set_disabled(0);
        break;
    case 0x01000003:
        control.show(); // Virtual +10, not merely a visibility byte store.
        break;
    case 0x01000004:
        control.hide(); // Virtual +14.
        break;
    case 0x01000005:
        control.set_focusable(0); // Direct byte +66 store.
        break;
    case 0x01000006:
        control.set_focusable(1);
        break;
    case 0x01000007:
        control.register_observer(event);
        break;
    case 0x01000008:
        control.unregister_observer(event);
        break;
    case 0x01000009:
        control.set_word_5c(event.argument);
        break;
    case 0x0100000a:
        event.argument = control.word_5c();
        break;
    case 0x0100000b:
        control.set_word_58(event.argument);
        break;
    case 0x0100000c:
        event.argument = control.word_58();
        break;
    case 0x010000a1:
        if ((event.argument & 0x04000000u) != 0) {
            control.emit_event(event.argument, 0);
        }
        break;
    case 0x0200002c:
        // Original rereads the child vector and size after each callback.
        // Each child receives fresh target/code/argument/payload, not the
        // parent's record or the previous child's mutations. Native spare
        // words are zeroed; the original leaves those words unspecified.
        for (std::size_t i = 0; i < control.child_count(); ++i) {
            ControlEvent child_event{0, 0x0200002c, 0, 0, 0, {}};
            control.dispatch_child(i, child_event); // Child virtual +74.
        }
        break;
    default:
        break;
    }
    return true;
}

}

#endif
