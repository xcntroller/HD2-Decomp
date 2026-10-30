#ifndef HD2_FRONTEND_KEYBOARD_PREPROCESSING_H
#define HD2_FRONTEND_KEYBOARD_PREPROCESSING_H

#include "frontend/frontend_key.h"
#include <array>
#include <cstring>
#include <stdexcept>

namespace hd2::frontend {

// 0066b380. convert_ansi_z supplies MultiByteToWideChar(CP_ACP, 0, bytes,
// -1, output, capacity); its result is intentionally ignored. The return value
// counts source bytes, including the terminator, rather than converted units.
template<class Input>
bool read_frontend_characters(Input& input, char16_t* output, int capacity) {
    if (!output || capacity <= 0)
        throw std::invalid_argument("Character conversion requires output storage");
    output[0] = 0;
    std::array<char, 1023> bytes;
    std::size_t count = 0;
    do {
        bytes[count++] = static_cast<char>(static_cast<std::uint8_t>(input.take_text_character()));
        if (bytes[count - 1] == 0) {
            input.convert_ansi_z(bytes.data(), output, capacity);
            return count > 1;
        }
    } while (count < bytes.size());
    // Original passes an unterminated stack buffer to conversion in this case.
    throw std::length_error("Unterminated front-end character input");
}

// 0066b3e0, including its existing recovered screen/Escape tail. Input buffers
// are borrowed DWORD storage, as in 006c72b0; a non-null second buffer must
// contain at least 47 words. Its float at +b8 is read only after all callbacks.
// Services resolve mutable targets/state afresh; character_current_target
// returns false when self+10 is null. trace_wide_character supplies the original
// KEY_VALUE diagnostic. No concrete application/text-target backend is implied.
template<class Frontend, class Input>
bool preprocess_frontend_keyboard(Frontend& frontend, Input& input,
                                  const std::uint32_t* first,
                                  const std::uint32_t* second) {
    if (frontend.input_blocked()) return true; // self+2d
    frontend.preprocess_buffers(first, second); // virtual +44
    if (!frontend.screens_available()) return true; // 008ae4e0
    const auto scan = input.next_scan(); // IGraph +a0, before character callbacks
    if (frontend.wide_character_mode()) { // 008ae92c
        std::array<char16_t, 1024> characters{};
        bool consumed = false;
        if (read_frontend_characters(input, characters.data(), 1023)) {
            for (const auto character : characters) {
                if (character == 0) break;
                frontend.trace_wide_character(static_cast<std::uint16_t>(character));
                // Do not short-circuit after consumption: later callbacks can
                // see a different target and must still receive their units.
                if (frontend.character_current_target(static_cast<std::uint16_t>(character)))
                    consumed = true;
            }
        }
        if (consumed) return true;
    } else {
        const auto character = static_cast<std::uint8_t>(input.take_text_character());
        if (character && frontend.character_current_target(character)) return true;
    }

    auto key = static_cast<std::uint8_t>(scan);
    if (key == 0x4e) {
        // Original order is Alt, Shift, Control, each queried only as needed.
        if (input.alt_down() && input.shift_down() && input.control_down()) {
            frontend.enqueue_debug_event(); // target 0, code 0200007d, argument 0
            return true;
        }
    } else if (key == 0) {
        if (input.control_down()) key = input.key_down(0x1d) ? 0x1d : 0x9d;
        else if (input.alt_down()) key = input.key_down(0x38) ? 0x38 : 0xb8;
        else if (input.shift_down()) key = input.key_down(0x2a) ? 0x2a : 0x36;
        else {
            if (!second) return true;
            float escape;
            static_assert(sizeof(escape) == sizeof(std::uint32_t));
            std::memcpy(&escape, second + 0xb8 / 4, sizeof(escape));
            if (escape == 0.0f) return true; // NaN, like original x87, falls through.
            key = 1;
        }
    }
    // Original writes only the low byte of an uninitialized local DWORD before
    // passing it to +20. Native code zero-extends the meaningful scan byte.
    return dispatch_frontend_key(frontend, key);
}

}
#endif
