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
    static const TextStyle body{2, .04f, 0xff000000, 1, 2};
    static const TextStyle version{3, .02f, 0xff000000, 1, 3};
    switch (style) {
    case 10: return &title;
    case 11: return &small;
    case 12: case 13: return &body;
    case 40: return &version;
    default: return nullptr;
    }
}

}

#endif
