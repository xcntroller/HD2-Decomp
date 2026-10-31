#ifndef HD2_FRONTEND_RUNTIME_CONFIRMATION_H
#define HD2_FRONTEND_RUNTIME_CONFIRMATION_H

#include "frontend/control_definition_ids.h"
#include "frontend/menu_definition.h"

namespace hd2::frontend {

// 00668790 appends these four children before ordinary screen +8c binding.
// Callback/context are native typed replacements for observer 006693a0.
// The visual-pair names reuse the already verified confirm-model bridge.
inline void append_runtime_confirmation_controls(MenuScreenDefinition& definition,
                                                 void* context, ObserverCallback callback) {
    const auto next_id = [&] {
        return screen_child_identifier(definition.identifier,
            static_cast<std::uint32_t>(definition.controls.size()));
    };
    definition.controls.push_back(text_definition(next_id(), "text", {caption_definition(24, 2004)}));
    definition.controls.push_back(button_definition(next_id(), "byes*", "normal02", "actived03",
        MenuControlRole::yes, {caption_definition(13, 2001),
            observer_control_event(0, 0x04001000, context, callback)}));
    definition.controls.push_back(button_definition(next_id(), "bno*", "normal01", "actived01",
        MenuControlRole::no, {caption_definition(13, 2000),
            observer_control_event(0, 0x04001000, context, callback)}));
    definition.controls.push_back(button_definition(next_id(), "bok", "normal03", "actived04",
        MenuControlRole::ok, {caption_definition(13, 2013), {0, 0x01000004, 0, 0, 0, {}},
            observer_control_event(0, 0x04001000, context, callback)}));
}

// Slot-zero subclass state, original +84/+88/+8c. Services provides the base
// screen key method and identifier lookup for a native observer sender. It and
// the queue outlive this stable object, which outlives registered observers.
// Reply words are EVENT CODES, not pointers or callbacks. No new desktop screen
// is enabled by this protocol; the engine model/product binding is separate.
template<class Services>
class RuntimeConfirmation {
public:
    static constexpr std::uint32_t inactive = 0xffffffffu;
    RuntimeConfirmation(Services& services, ControlEventDestination& queue)
        : services_(services), queue_(queue) {}
    RuntimeConfirmation(const RuntimeConfirmation&) = delete;
    RuntimeConfirmation& operator=(const RuntimeConfirmation&) = delete;

    std::uint32_t mode() const { return mode_; }
    std::uint32_t affirmative_code() const { return affirmative_; }
    std::uint32_t negative_code() const { return negative_; }

    // 006688a0 / 00668cd0. Reopening an outstanding request does nothing.
    // Unknown mode words retain previous button visibility, but still replace
    // the caption and request state. Mode three selects text style 25, else 24.
    void open_text(std::uint32_t mode, const std::shared_ptr<const std::string>& text,
                   std::uint32_t affirmative, std::uint32_t negative) {
        if (mode_ != inactive) return;
        configure_buttons(mode);
        ControlEvent caption{0x00100000, 0x01000014, mode == 3 ? 25u : 24u, 0, 0, {}};
        caption.text = text;
        queue_.enqueue(caption);
        finish_open(mode, affirmative, negative);
    }
    void open_localized(std::uint32_t mode, std::uint32_t label,
                        std::uint32_t affirmative, std::uint32_t negative) {
        if (mode_ != inactive) return;
        configure_buttons(mode);
        queue_.enqueue({0x00100000, 0x01000015, mode == 3 ? 25u : 24u, label, 0, {}});
        finish_open(mode, affirmative, negative);
    }

    // 00669090: mark inactive BEFORE queuing removal, with no reply and no
    // clearing of the two stored codes. This also runs when already inactive.
    void close() {
        mode_ = inactive;
        queue_.enqueue({0, 0x02000097, 0, 0, 0, {}});
    }
    // 006691a0 clears only reply words, without changing request/stack state.
    void clear_reply_codes() { affirmative_ = 0; negative_ = 0; }

    // 006691b0. Inactive and mode zero swallow ALL keys. Escape uses the low
    // byte only; other active keys pass the complete word to screen 00668670.
    bool key(std::uint32_t word) {
        if (mode_ == 0 || mode_ == inactive) return true;
        if (static_cast<std::uint8_t>(word) != 1) return services_.screen_key(word);
        queue_.enqueue({0, 0x02000097, 0, 0, 0, {}});
        const auto reply = negative_;
        mode_ = inactive;
        enqueue_reply(reply);
        clear_reply_codes();
        return true;
    }

    // 006693a0. Sender presence and inactive state are checked before reading
    // its identifier. No visibility, eligibility or tag check is added here.
    static bool observer(void* receiver, void*, void* sender, std::uint32_t, std::uint32_t) {
        if (!sender) return false;
        auto& self = *static_cast<RuntimeConfirmation*>(receiver);
        if (self.mode_ == inactive) return true;
        return self.button_activated(self.services_.sender_identifier(sender));
    }
    bool button_activated(std::uint32_t identifier) {
        if (mode_ == inactive) return true;
        queue_.enqueue({0, 0x02000097, 0, 0, 0, {}});
        mode_ = inactive;
        const auto ordinal = (identifier >> 20) & 0x3fu;
        enqueue_reply(ordinal == 2 || ordinal == 4 ? affirmative_ : ordinal == 3 ? negative_ : 0);
        clear_reply_codes();
        return true;
    }
    MenuScreenDefinition definition() {
        MenuScreenDefinition result{"runtime-confirmation", 0, "confirm", {}};
        append_runtime_confirmation_controls(result, this, &observer);
        return result;
    }

private:
    void configure_buttons(std::uint32_t mode) {
        if (mode > 3) return;
        const bool yes_no = mode == 1 || mode == 3;
        queue_.enqueue({0x00200000, yes_no ? 0x01000003u : 0x01000004u, 0, 0, 0, {}});
        queue_.enqueue({0x00300000, yes_no ? 0x01000003u : 0x01000004u, 0, 0, 0, {}});
        queue_.enqueue({0x00400000, mode == 2 ? 0x01000003u : 0x01000004u, 0, 0, 0, {}});
    }
    void finish_open(std::uint32_t mode, std::uint32_t affirmative, std::uint32_t negative) {
        affirmative_ = affirmative;
        negative_ = negative;
        mode_ = mode;
        queue_.enqueue({0, 0x02000096, 0, 0, 0, {}});
    }
    void enqueue_reply(std::uint32_t code) {
        if (code) queue_.enqueue({0, code, 0, 0, 0, {}});
    }
    Services& services_;
    ControlEventDestination& queue_;
    std::uint32_t mode_ = inactive;
    std::uint32_t affirmative_ = 0;
    std::uint32_t negative_ = 0;
};

//0066D780/0066D7A0/0066D7C0: frontend +100 is nullable. Preserve the
// no-op only for an absent dialog; a present service uses the actual protocol.
template<class Dialog>
void request_runtime_text(Dialog* dialog, std::uint32_t mode, std::shared_ptr<const std::string> text,
                          std::uint32_t affirmative, std::uint32_t negative) {
    if (dialog) dialog->open_text(mode, text, affirmative, negative);
}
template<class Dialog>
void request_runtime_localized(Dialog* dialog, std::uint32_t mode, std::uint32_t label,
                               std::uint32_t affirmative, std::uint32_t negative) {
    if (dialog) dialog->open_localized(mode, label, affirmative, negative);
}
template<class Dialog>
void close_runtime_dialog(Dialog* dialog) { if (dialog) dialog->close(); }

}

#endif
