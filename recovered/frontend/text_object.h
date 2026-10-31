#ifndef HD2_FRONTEND_TEXT_OBJECT_H
#define HD2_FRONTEND_TEXT_OBJECT_H

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <utility>

namespace hd2::frontend {

using SharedText = std::shared_ptr<const std::string>;
struct TextRecord { SharedText text; std::uint8_t style; };

// Portable record/state subset of the client text object. Original records
// are packed pointer+style (five bytes); this is deliberately native storage.
class TextObject {
public:
    using Localizer = std::function<SharedText(std::uint32_t)>;
    using VersionFormatter = std::function<SharedText(const SharedText&)>;
    std::uint8_t disabled = 0;
    std::uint8_t inset = 0;
    std::uint32_t word_94 = 0;
    bool dirty = false;
    const std::vector<TextRecord>& records() const { return records_; }

    // 00643900. Selection is separate from a valid stored record index:
    // selecting an invalid signed-short index clears selection but stores it.
    bool select_record(std::int16_t index) {
        if (index >= 0 && static_cast<std::size_t>(index) < records_.size()) {
            if (selection_active) return false;
            selected_index_ = index;
            selection_ticks = 0;
            selection_active = true;
        } else {
            selected_index_ = index;
            selection_active = false;
        }
        dirty = true;
        return true;
    }
    // 00644080 / 00644000 use the stored index, not selection_active.
    SharedText selected_text() const {
        return valid_selected_index() ? records_[static_cast<std::size_t>(selected_index_)].text : SharedText{};
    }
    bool replace_selected_text(SharedText text) {
        if (!valid_selected_index()) return false;
        records_[static_cast<std::size_t>(selected_index_)].text = std::move(text);
        dirty = true;
        return true;
    }
    // 00643e00 / 00643bf0. Hover restoration delegates the actual row-geometry
    // visibility adjustment (00643b20), then rereads the hovered index.
    bool clear_hover_selection() {
        if (selection_active) return false;
        hovered_record = -1;
        return true;
    }
    template<class EnsureVisible>
    std::int16_t restore_hover_selection(EnsureVisible ensure_visible) {
        if (selection_active) return -1;
        if (hovered_record != -1) ensure_visible(static_cast<std::int16_t>(hovered_record));
        selected_index_ = hovered_record;
        return static_cast<std::int16_t>(hovered_record);
    }
    // 00643c30. Stored record focus changes only outside active selection.
    template<class EnsureVisible>
    std::int16_t select_visible_record(std::int16_t index, EnsureVisible ensure_visible) {
        if (selection_active || index < 0 || static_cast<std::size_t>(index) >= records_.size()) return -1;
        ensure_visible(index);
        selected_index_ = index;
        return index;
    }
    // 00643f60. Keep original wrapping DWORD time arithmetic and strict bounds.
    void update_caret(std::uint32_t milliseconds) {
        caret_visible = false;
        if (!selection_active) return;
        selection_ticks += milliseconds;
        const auto signed_ticks = selection_ticks <= 0x7fffffffu ? static_cast<std::int64_t>(selection_ticks)
            : static_cast<std::int64_t>(selection_ticks) - 0x100000000LL;
        if (signed_ticks < 500) caret_visible = true;
        if (signed_ticks > 1000) selection_ticks = 0;
    }
    std::int32_t hovered_record = -1;
    bool caret_visible = false;
    bool selection_active = false;
    std::uint32_t selection_ticks = 0;

    // 006440f0: replace a valid signed index, otherwise append, never clear.
    bool set(SharedText text, std::uint32_t style, std::int32_t index) {
        TextRecord record{std::move(text), static_cast<std::uint8_t>(style)};
        if (index >= 0 && static_cast<std::size_t>(index) < records_.size()) {
            records_[static_cast<std::size_t>(index)] = std::move(record);
        } else {
            records_.push_back(std::move(record));
        }
        dirty = true;
        return true;
    }
    // 00644590; 00644510 adds the same record via its string wrapper.
    bool append(SharedText text, std::uint32_t style) { return set(std::move(text), style, -1); }

    // 00644200's ordinary localization path. Style WORD 40 uses the original
    // version formatter, not merely the low style byte; absent formatter is an
    // explicit unsupported boundary, checked before modifying text state.
    bool set_localized(std::uint32_t id, std::uint32_t style, std::int32_t index,
                       const Localizer& localize, const VersionFormatter& version = {}) {
        if (style == 40 && !version) return false;
        auto text = localize(id);
        if (style == 40) text = version(text);
        return set(std::move(text), style, index);
    }
    // 00644450: append localized text; unlike 00644200 it does not special-case 40.
    bool append_localized(std::uint32_t id, std::uint32_t style, const Localizer& localize) {
        return append(localize(id), style);
    }
    // 00644890 marks dirty even on invalid input. A successful erase at index
    // zero returns true: original reuses its argument stack slot for the record
    // pointer before its final non-null test.
    bool remove(std::int32_t index) {
        dirty = true;
        if (index < 0 || static_cast<std::size_t>(index) >= records_.size()) return false;
        records_.erase(records_.begin() + index);
        return true;
    }
    // 00644960: clear all records, not a refresh/relocalization operation.
    void clear() { records_.clear(); dirty = true; }
private:
    bool valid_selected_index() const {
        return selected_index_ >= 0 && static_cast<std::size_t>(selected_index_) < records_.size();
    }
    std::int32_t selected_index_ = -1;
    std::vector<TextRecord> records_;
};

struct TextStyle {
    std::uint32_t font;
    float height;
    std::uint32_t color;
    std::uint32_t horizontal;
    std::uint32_t vertical;
};

// Bounded 00641e30 style catalogue used by the live English menu. Remaining
// styles and general wrapping/layout remain outside this reconstruction.
inline const TextStyle* menu_text_style(std::uint8_t style) {
    static const TextStyle title{2, .05f, 0xff000000, 3, 3};
    static const TextStyle small{3, .02f, 0xff000000, 1, 2};
    static const TextStyle list{2, .04f, 0xff000000, 1, 1};
    static const TextStyle body{2, .04f, 0xff000000, 1, 2};
    static const TextStyle selector{2, .04f, 0xa0000000, 2, 2}; //00641E30 entry20.
    // 00641e30 entries 24/25: big font, black, left/vertically centered;
    // heights 0x3d23d70a / 0x3cf5c28f. Paragraph wrapping stays separate.
    static const TextStyle compact_prompt{2, .03f, 0xff000000, 1, 2};
    static const TextStyle version{3, .02f, 0xff000000, 1, 3};
    switch (style) {
    case 19: return &list;
    case 10: return &title;
    case 11: return &small;
    case 12: case 13: case 24: return &body;
    case 20: return &selector;
    case 25: return &compact_prompt;
    case 40: return &version;
    default: return nullptr;
    }
}

}

#endif
