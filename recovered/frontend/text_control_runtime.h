#ifndef HD2_FRONTEND_TEXT_CONTROL_RUNTIME_H
#define HD2_FRONTEND_TEXT_CONTROL_RUNTIME_H

#include "frontend/text_control_event.h"
#include "frontend/text_object.h"

namespace hd2::frontend {

// Compose attached text state/layout operations with the emitting control's
// shared state runtime. Backend provides text storage and visual bindings.
template<class Text, class Base>
class TextControlRuntime {
public:
    TextControlRuntime(Text& text, Base& base) : text_(text), base_(base) {}
    bool dispatch(ControlEvent& event) { return dispatch_text_control_event(*this, event); }
    bool dispatch_base_event(ControlEvent& event) { return base_.dispatch(event); }
    void text_set_payload(const SharedText& text, std::uint32_t style, std::uint32_t index) {
        text_.object().set(text, style, static_cast<std::int32_t>(index));
    }
    void text_set_localized(std::uint32_t id, std::uint32_t style, std::uint32_t index) {
        text_.set_localized(id, style, static_cast<std::int32_t>(index));
    }
    void text_append_payload(const SharedText& text, std::uint32_t style) { text_.object().append(text, style); }
    void text_append_localized(std::uint32_t id, std::uint32_t style) { text_.append_localized(id, style); }
    void text_remove_record(std::uint32_t index) { text_.object().remove(static_cast<std::int32_t>(index)); }
    void text_clear() { text_.object().clear(); }
    void set_frame_direction(float x, float y, float z) { text_.set_frame_direction(x, y, z); }
    void layout_text_on_frame() { text_.layout_text_on_frame(); }
    void set_text_word_94(std::uint32_t value) { text_.object().word_94 = value; }
    std::uint32_t text_record_count() const { return static_cast<std::uint32_t>(text_.object().records().size()); }
    void set_text_inset(std::uint8_t value) { text_.object().inset = value; }
private:
    Text& text_;
    Base& base_;
};

}

#endif
