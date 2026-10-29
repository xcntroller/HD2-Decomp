#ifndef HD2_FRONTEND_TEXT_CONTROL_EVENT_H
#define HD2_FRONTEND_TEXT_CONTROL_EVENT_H

#include "frontend/control_events.h"

namespace hd2::frontend {

// Client 00661940. Sequencing only: callbacks delegate original 00644xxx text
// helpers, layout and storage. Most text cases assume attached text exists;
// do not add a universal null-text early return before base-event dispatch.
template<class Control>
bool dispatch_text_control_event(Control& control, ControlEvent& event) {
    if ((event.code & 0x040ff000u) != 0) {
        control.dispatch_base_event(event);
        return true;
    }
    switch (event.code & 0x03000fffu) {
    case 0x01000014:
        control.text_set_payload(event.payload, event.argument, 0); // 006443c0.
        break;
    case 0x01000015:
        control.text_set_localized(event.word_0c, event.argument, 0); // 00644200.
        break;
    case 0x01000016:
        control.text_append_payload(event.payload, event.argument); // 00644510.
        break;
    case 0x01000017:
        control.text_append_localized(event.word_0c, event.argument); // 00644450.
        break;
    case 0x01000018:
        control.text_set_payload(event.payload, event.argument, event.word_0c);
        break;
    case 0x01000068:
        control.text_set_value(event.argument); // 00644890; semantics remain delegated.
        break;
    case 0x01000077:
    case 0x02000018:
        control.text_refresh(); // 00644960.
        break;
    case 0x01000099:
        control.set_frame_direction(0.0f, 0.0f, 1.0f); // Null up-vector.
        control.layout_text_on_frame();
        break;
    case 0x010000a3:
        control.set_text_word_94(event.argument);
        break;
    case 0x0200002c:
        control.layout_text_on_frame();
        break;
    case 0x02000030:
        event.argument = control.text_record_count(); // Text vector +34/+38, stride five.
        break;
    case 0x02000073:
        control.set_text_inset(static_cast<std::uint8_t>(event.argument));
        control.layout_text_on_frame();
        break;
    default:
        control.dispatch_base_event(event);
        break;
    }
    return true; // Original ignores helper/parent return values.
}

// Client 006551d0. These two button overrides bypass the parent text/base path.
template<class Control>
bool dispatch_button_control_event(Control& control, ControlEvent& event) {
    if ((event.code & 0x040ff000u) != 0) {
        control.dispatch_text_event(event);
        return true;
    }
    switch (event.code & 0x03000fffu) {
    case 0x0200002c:
        if (control.has_text()) control.layout_text_on_normal();
        break;
    case 0x02000076:
        if (control.has_text()) control.set_text_word_94(event.argument ? 0xffffffffu : 0u);
        break;
    default:
        control.dispatch_text_event(event);
        break;
    }
    return true;
}

}

#endif
