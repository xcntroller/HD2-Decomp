#ifndef HD2_FRONTEND_EDIT_CHARACTER_TARGET_H
#define HD2_FRONTEND_EDIT_CHARACTER_TARGET_H

#include "frontend/common_event_services.h"
#include "frontend/text_object.h"

#include <array>
#include <string_view>

namespace hd2::frontend {

// Editable-control fields from 006577d0. Portable owned strings replace raw
// +70/+74/+78 storage. The visual/text binding and derived callbacks are supplied
// by the backend; this is not a constructed or live-enabled menu widget.
struct EditCharacterState {
    SharedText original_text;
    std::string buffer;
    SharedText excluded_characters;
    std::uint8_t active = 0;
    bool numeric_only = false;
    std::int32_t record_index = -1;
    std::uint32_t capacity = 30;
};

// 0064d5f0, including its pre-Unicode-limit four-byte acceptance and malformed
// two-byte fallback for values >= 200000. No terminator is written. The caller
// must supply capacity strictly GREATER than the encoded byte count.
inline int encode_edit_character(char* output, std::uint32_t value, int capacity) {
    if (value >= 0xd800 && value <= 0xdfff) return -3;
    int count = 2;
    if (value < 0x80) count = 1;
    else if (value > 0x7ff) {
        if (value < 0x10000) count = 3;
        else if (value < 0x200000) count = 4;
        else value = 0xfffd;
    }
    if (capacity <= count) return -4;
    static constexpr std::array<std::uint8_t, 5> prefixes{0, 0, 0xc0, 0xe0, 0xf0};
    for (int index = count - 1; index > 0; --index) {
        output[index] = static_cast<char>((value & 0x3f) | 0x80);
        value >>= 6;
    }
    output[0] = static_cast<char>(prefixes[static_cast<std::size_t>(count)] | value);
    return count;
}

// Original strings are NUL terminated even when native SharedText contains
// more bytes. Acquisition copies at most +84 bytes into +84+1 zeroed storage.
inline std::string_view edit_c_string(const std::string& text) {
    const auto end = text.find('\0');
    return std::string_view(text.data(), end == std::string::npos ? text.size() : end);
}

// 006629c0 / +68. Set active BEFORE any text operation; selection failure
// leaves it set. Invalid configured indices set a null, style-zero record at
// index zero (append only if empty); the configured index is not repaired.
template<class Backend>
int acquire_edit_characters(EditCharacterState& state, Backend& backend) {
    if (state.active) return 7;
    state.active = 1;
    if (state.record_index < 0 ||
        static_cast<std::size_t>(state.record_index) >= backend.text().records().size())
        backend.text().set({}, 0, 0); // 006443c0(null string, style 0, index 0).
    if (!backend.text().select_record(static_cast<std::int16_t>(state.record_index))) return 0;
    state.original_text = backend.text().selected_text();
    const auto source = state.original_text ? edit_c_string(*state.original_text) : std::string_view{};
    state.buffer.assign(source.substr(0, state.capacity));
    return 1;
}

// 00662980 / +64. Notify while still active, only after successful deselection;
// a callback can reenter, and the final store still clears the active byte.
template<class Backend>
bool release_edit_characters(EditCharacterState& state, Backend& backend, std::uint32_t argument) {
    if (!state.active) return false;
    if (backend.text().select_record(-1)) backend.emit_event(0x0400b000, argument);
    state.active = 0;
    return true;
}

// 00662380. Wide mode scans continuation bytes and checks lead range/count,
// accepting overlong and non-Unicode sequences. Reject the original buffer
// underread for all-continuation input. Empty input succeeds without touching text.
template<class Backend>
bool backspace_edit_character(EditCharacterState& state, Backend& backend) {
    if (state.buffer.empty()) return true;
    std::size_t erase_at = state.buffer.size() - 1;
    if (backend.wide_character_mode()) {
        while (static_cast<std::uint8_t>(state.buffer[erase_at]) >= 0x80 &&
               static_cast<std::uint8_t>(state.buffer[erase_at]) < 0xc0) {
            if (erase_at == 0) return false;
            --erase_at;
        }
        const auto lead = static_cast<std::uint8_t>(state.buffer[erase_at]);
        const auto count = state.buffer.size() - erase_at;
        if (!((count == 1 && lead < 0x80) || (count == 2 && lead >= 0xc0 && lead < 0xe0) ||
              (count == 3 && lead >= 0xe0 && lead < 0xf0) || (count == 4 && lead >= 0xf0 && lead <= 0xf7)))
            return false;
    }
    state.buffer.erase(erase_at);
    backend.text().replace_selected_text(std::make_shared<const std::string>(state.buffer));
    return true;
}

// 00662490 / +24. Backend supplies actual derived +3c deactivation, wide-mode
// global and CRT digit classification (including signed-byte argument), not
// successful stand-ins. Tagged observers may change active state mid-callback.
template<class Backend>
bool dispatch_edit_character(EditCharacterState& state, Backend& backend, std::uint32_t value) {
    if (!state.active || value == 0) return false;
    switch (value) {
    case 8:
        backspace_edit_character(state, backend); // Return deliberately ignored.
        return true;
    case 9: return false;
    case 13:
        backend.emit_event(0x04016000, 0);
        backend.deactivate(0); // Unconditional, even if the observer released capture.
        return true;
    case 27:
        backend.emit_event(0x04017000, 0);
        if (!state.active) return false;
        backend.text().replace_selected_text(state.original_text);
        backend.deactivate(0);
        return true;
    default: break;
    }
    if (value >= 0xff10 && value <= 0xff19) value -= 0xfee0;
    if (value >= 0x80 && backend.wide_character_mode()) {
        std::array<char, 6> bytes{};
        const int count = encode_edit_character(bytes.data(), value, 6);
        if (count > 0 && state.buffer.size() + static_cast<std::size_t>(count) < state.capacity) {
            state.buffer.append(bytes.data(), static_cast<std::size_t>(count));
            backend.text().replace_selected_text(std::make_shared<const std::string>(state.buffer));
        }
        return true; // Wide path bypasses byte exclusions and numeric restriction.
    }
    const auto byte = static_cast<std::uint8_t>(value);
    if (state.excluded_characters && edit_c_string(*state.excluded_characters).find(static_cast<char>(byte)) !=
            std::string_view::npos) return true;
    // strchr(exclusions, 0) also finds the string terminator.
    if (state.excluded_characters && byte == 0) return true;
    if (state.numeric_only && !backend.is_digit(static_cast<std::int8_t>(byte)) &&
        (!state.buffer.empty() || byte != '-')) return true;
    if (byte == 0) return false;
    if (state.buffer.size() < state.capacity) {
        state.buffer.push_back(static_cast<char>(byte));
        backend.text().replace_selected_text(std::make_shared<const std::string>(state.buffer));
    }
    return true;
}

// Typed native owner for the recovered bf/c0 protocol and +24/+64/+68 methods.
// Must be shared-owned before requesting capture. Backend/text/queue outlive it;
// no input capture event is installed by the bounded Main/Exit definitions.
template<class Backend>
class EditCharacterTarget final : public FrontendCaptureOwner,
                                  public std::enable_shared_from_this<EditCharacterTarget<Backend>> {
public:
    EditCharacterTarget(Backend& backend, ControlEventDestination& queue) : backend_(backend), queue_(queue) {}
    EditCharacterState state;
    bool character(std::uint32_t value) override { return dispatch_edit_character(state, backend_, value); }
    bool release(std::uint32_t argument) override { return release_edit_characters(state, backend_, argument); }
    void acquire(std::uint32_t) override { acquire_edit_characters(state, backend_); } // Result ignored by0066c950.
    // 00662790 / +38: queue ownership request without changing the active byte.
    bool request_activation() {
        if (state.active) return false;
        queue_capture(0x020000bf);
        return true;
    }
    // 00662880 / +3c: queue release before emitting deactivation; stay active
    // until the common queue later calls +64. Duplicate pending requests remain.
    bool request_deactivation() {
        if (!state.active) return false;
        queue_capture(0x020000c0);
        backend_.emit_event(0x04002000, 0);
        return true;
    }
private:
    void queue_capture(std::uint32_t code) {
        ControlEvent event{0, code, 0, 0, 0, {}};
        event.capture_owner = this->shared_from_this();
        queue_.enqueue(event);
    }
    Backend& backend_;
    ControlEventDestination& queue_;
};

}
#endif
