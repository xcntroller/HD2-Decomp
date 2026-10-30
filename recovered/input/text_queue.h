#ifndef HD2_INPUT_TEXT_QUEUE_H
#define HD2_INPUT_TEXT_QUEUE_H

#include <array>
#include <cstdint>

namespace hd2::input {

// Original 100d1e48: element zero is the last returned value, not a queued
// character. WM_CHAR appends to the first empty slot in elements 1..31.
struct TextQueue {
    std::array<std::uint32_t, 32> words{};
};

// WM_CHAR branch at 1006a507. Bit 31 of lParam suppresses insertion. A full
// queue silently drops the value. Preserve DWORD storage, including zero.
inline void append_text_character(TextQueue& queue, std::uint32_t value,
                                  std::uint32_t message_flags) {
    if (message_flags & 0x80000000u) return;
    for (std::size_t index = 1; index < queue.words.size(); ++index) {
        if (queue.words[index] == 0) {
            queue.words[index] = value;
            return;
        }
    }
}

// IGraph +a8 / 1006f2d0 shifts BEFORE returning element zero. This is separate
// from +a4 / 1006f470's scan-translated character and does not clear that value.
inline std::uint32_t take_text_character(TextQueue& queue) {
    for (std::size_t index = 0; index + 1 < queue.words.size(); ++index)
        queue.words[index] = queue.words[index + 1];
    queue.words.back() = 0;
    return queue.words.front();
}

// IGraph +ac / 1006f300 clears all 32 DWORDs, including the last result.
inline void clear_text_characters(TextQueue& queue) { queue.words.fill(0); }

}
#endif
