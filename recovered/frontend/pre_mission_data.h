#ifndef HD2_FRONTEND_PRE_MISSION_DATA_H
#define HD2_FRONTEND_PRE_MISSION_DATA_H

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace hd2::frontend {

// Native values, not original packed object layouts. File offsets below are
// offsets in original serialized records. No engine item/world instance is
// constructed by these data owners.
namespace pre_mission_detail {
class Bytes {
  public:
    Bytes(const unsigned char *data, std::size_t size) : data_(data), size_(size) {}
    explicit Bytes(const std::vector<unsigned char> &data) : Bytes(data.data(), data.size()) {}
    std::size_t remaining() const { return size_ - offset_; }
    const unsigned char *take(std::size_t size) {
        if (size > remaining())
            throw std::runtime_error("Truncated pre-mission data");
        const auto *result = data_ + offset_;
        offset_ += size;
        return result;
    }
    std::uint32_t word() {
        const auto *p = take(4);
        return std::uint32_t{p[0]} | std::uint32_t{p[1]} << 8 | std::uint32_t{p[2]} << 16 |
               std::uint32_t{p[3]} << 24;
    }
    static std::uint32_t word(const unsigned char *p) {
        Bytes input(p, 4);
        return input.word();
    }
    static std::int32_t integer(std::uint32_t value) {
        std::int32_t result;
        std::memcpy(&result, &value, 4);
        return result;
    }
    static float real(std::uint32_t value) {
        float result;
        std::memcpy(&result, &value, 4);
        return result;
    }
    static std::string text(const unsigned char *p, std::size_t size) {
        const auto *end = std::find(p, p + size, 0);
        if (end == p + size)
            throw std::runtime_error("Unterminated pre-mission resource name");
        return std::string(reinterpret_cast<const char *>(p), reinterpret_cast<const char *>(end));
    }
    std::uint32_t count(std::size_t record_size) {
        const auto result = word();
        if (result > remaining() / record_size)
            throw std::runtime_error("Invalid pre-mission record count");
        return result;
    }

  private:
    const unsigned char *data_;
    std::size_t size_, offset_ = 0;
};

// Catalogue007DC500 reads at most252000 bytes. Other original data readers use
// incremental DTA reads. Native buffers have explicit bounds; short/corrupt
// records fail instead of reading uninitialized storage. Files must be the real
// archive/loose resolver and must outlive each call.
template <class Files>
std::optional<std::vector<unsigned char>> read_file(Files &files, const char *path,
                                                    std::uint32_t limit = 0x7fffffffu) {
    const auto handle = files.open(path, 0);
    if (handle == 0xffffffffu)
        return std::nullopt;
    struct Close {
        Files &files;
        std::uint32_t handle;
        ~Close() { files.close(handle); }
    } close{files, handle};
    const auto size = files.seek(handle, 0, 2);
    if (size == 0xffffffffu)
        throw std::runtime_error("Invalid pre-mission file size");
    if (files.seek(handle, 0, 0) == 0xffffffffu)
        throw std::runtime_error("Cannot seek pre-mission data");
    const auto count = std::min(size, limit);
    std::vector<unsigned char> bytes(count);
    if (count && files.read(handle, bytes.data(), count) != count)
        throw std::runtime_error("Cannot read pre-mission data");
    return bytes;
}
} // namespace pre_mission_detail

struct PreMissionItemPayload {
    std::uint32_t type = 0;
    std::vector<unsigned char> bytes;
};

struct PreMissionItem {
    std::uint32_t identifier = 0, item_class = 0;
    std::array<std::string, 3> resources;
    std::uint32_t field0c = 0, caption = 0, category = 0;
    float weight = 0;
    std::array<std::uint32_t, 4> fields38{};
    std::array<PreMissionItemPayload, 2> payloads;
    std::array<std::uint32_t, 5> derived{};

    // Original item virtual+10:007E0430/007E0530/007E0690.
    std::uint32_t kind() const { return item_class == 0 ? 23 : derived[1]; }
    // +18: only class1 has an associated ammunition item (007E0510).
    std::int32_t ammunition() const {
        return item_class == 1 ? pre_mission_detail::Bytes::integer(derived[3]) : -1;
    }
    bool restricted_holder() const { return fields38[3] == 1; } // Original+44.

    // 007DF950: slot compatibility is based on actual class/kind/category.
    std::uint32_t permitted_slots() const {
        const auto type = kind();
        const bool basic = item_class == 0;
        if (type == 16)
            return 1;
        if (type == 15)
            return 2;
        if (category == 5)
            return 8;
        std::uint32_t result = 0;
        if ((category == 2 || category == 3) && !basic &&
            (type == 3 || type == 4 || type == 5 || type == 6 || type == 12))
            result = 0x10;
        if (category != 4 && !basic && (type < 10 || type == 12))
            result |= 4;
        if (category == 0 || category == 1)
            result |= 0x60;
        if (category == 2) {
            if (basic || type == 11 || type == 10)
                result |= 0x40;
            if (type == 13)
                result |= 0x60;
        }
        return result;
    }
};

class PreMissionItemCatalogue {
  public:
    static constexpr std::size_t capacity = 500;
    const PreMissionItem *item(std::int32_t id) const {
        return id >= 0 && static_cast<std::size_t>(id) < capacity && items_[id] ? &*items_[id] : nullptr;
    }
    bool loaded() const { return loaded_; }
    std::size_t size() const {
        return std::count_if(items_.begin(), items_.end(),
                             [](const auto &value) { return value.has_value(); });
    }
    template <class Files> bool load(Files &files) {
        if (loaded_)
            return true; // 007DC4E0's lazy initialization.
        const auto bytes = pre_mission_detail::read_file(files, "Tables\\items.sav", 0x3d860);
        if (!bytes)
            return false;
        decode(*bytes);
        return true;
    }
    void decode(const std::vector<unsigned char> &bytes) {
        using B = pre_mission_detail::Bytes;
        std::array<std::optional<PreMissionItem>, capacity> result;
        B input(bytes);
        for (std::uint32_t id = 0; id != capacity; ++id) {
            if (!input.word())
                continue;
            const auto item_class = input.word();
            // Original consumes only the two words for unknown active classes.
            // Shipped baseline contains development-fill records of this form.
            if (item_class > 2)
                continue;
            const auto *record = input.take(500);
            PreMissionItem item;
            item.identifier = id;
            item.item_class = item_class;
            for (std::size_t i = 0; i != 3; ++i)
                item.resources[i] = B::text(record + i * 20, 20);
            item.field0c = B::word(record + 0x3c);
            item.caption = B::word(record + 0x64);
            item.weight = B::real(B::word(record + 0x68));
            item.category = B::word(record + 0x6c);
            for (std::size_t i = 0; i != 4; ++i)
                item.fields38[i] = B::word(record + 0x70 + i * 4);
            // 007DFAB0 includes the two tags and reserved trailing common bytes.
            //  Nested serializers007DEED0..007DF330 supply these exact sizes;
            //  type10 has a zero-byte payload (007DF330/007DF340).
            constexpr std::array<std::size_t, 11> sizes{
                {0, 0x24, 0x28, 0x88, 0x80, 0x10, 0x0c, 0x0c, 0x1c, 0x10, 0}};
            B payload(record + 0x80, 500 - 0x80);
            std::size_t common_size = 0xa8;
            for (auto &nested : item.payloads) {
                nested.type = payload.word();
                if (nested.type >= sizes.size())
                    throw std::runtime_error("Unknown item payload type");
                const auto size = sizes[nested.type];
                const auto *data = payload.take(size);
                nested.bytes.assign(data, data + size);
                common_size += size;
            }
            const std::size_t fields = item_class == 0 ? 1 : item_class == 1 ? 4 : 5;
            if (common_size + fields * 4 > 500)
                throw std::runtime_error("Invalid item derived record size");
            for (std::size_t i = 0; i != fields; ++i)
                item.derived[i] = B::word(record + common_size + i * 4);
            result[id] = std::move(item);
        }
        items_ = std::move(result);
        loaded_ = true;
    }

  private:
    std::array<std::optional<PreMissionItem>, capacity> items_;
    bool loaded_ = false;
};

struct PreMissionPreset {
    std::string name;
    std::array<std::int32_t, 5> equipment{{-1, -1, -1, -1, -1}};
    std::vector<std::array<std::int32_t, 2>> holsters, backpack;
    std::uint32_t field1ac = 0, field1b4 = 0;
    std::int32_t field1b0 = -1;
};

// 0067D280 defaults /0067D660 serialization. Profile overlay selection and
//  allocation remain caller policy; these records never write original files.
inline std::vector<PreMissionPreset> decode_pre_mission_presets(const std::vector<unsigned char> &bytes) {
    using B = pre_mission_detail::Bytes;
    B input(bytes);
    const auto version = input.word();
    const std::size_t record_size = version == 0 ? 0x1b0 : version == 1 ? 0x1b4 : 0x1b8;
    const auto count = input.count(record_size);
    std::vector<PreMissionPreset> result;
    result.reserve(count);
    for (std::uint32_t i = 0; i != count; ++i) {
        const auto *record = input.take(record_size);
        PreMissionPreset value;
        value.name = B::text(record, 32);
        for (std::size_t slot = 0; slot != 5; ++slot)
            value.equipment[slot] = B::integer(B::word(record + 0x20 + slot * 4));
        const auto read_entries = [&](std::size_t offset, std::uint32_t maximum,
                                      std::vector<std::array<std::int32_t, 2>> &output) {
            const auto entries = B::word(record + offset);
            if (entries > maximum)
                throw std::runtime_error("Invalid preset equipment count");
            for (std::uint32_t j = 0; j != entries; ++j)
                output.push_back({{B::integer(B::word(record + offset + 4 + j * 8)),
                                   B::integer(B::word(record + offset + 8 + j * 8))}});
        };
        read_entries(0x34, 14, value.holsters);
        read_entries(0xa8, 32, value.backpack);
        value.field1ac = B::word(record + 0x1ac);
        if (version != 0)
            value.field1b0 = B::integer(B::word(record + 0x1b0));
        if (version != 0 && version != 1)
            value.field1b4 = B::word(record + 0x1b4);
        result.push_back(std::move(value));
    }
    return result;
}

struct PreMissionEquipmentPolicy {
    std::uint32_t mode = 4;
    std::array<std::array<std::int32_t, 3>, 10> assignments;
    std::vector<std::array<std::int32_t, 2>> required;
    PreMissionEquipmentPolicy() { clear(); }
    void clear() {
        for (auto &entry : assignments)
            entry.fill(-1);
        required.clear();
    }
};

// 0067E350, as an update of the existing policy slots. An inactive serialized
//  slot clears its triplets/requirements but preserves its mode when allocated.
inline void decode_pre_mission_policies(const std::vector<unsigned char> &bytes,
                                        std::vector<std::optional<PreMissionEquipmentPolicy>> &slots) {
    using B = pre_mission_detail::Bytes;
    B input(bytes);
    const auto version = input.word();
    input.word(); // Original header word; not the slot count.
    input.take(version == 1 ? 60 : 200);
    const auto count = input.count(4);
    if (count > 120)
        throw std::runtime_error("Too many mission equipment policies");
    if (slots.size() < count)
        slots.resize(count);
    for (std::uint32_t i = 0; i != count; ++i) {
        if (!input.word()) {
            if (slots[i])
                slots[i]->clear();
            continue;
        }
        if (!slots[i])
            slots[i].emplace();
        auto &value = *slots[i];
        value.mode = input.word();
        const auto triplets = input.count(12);
        if (triplets > value.assignments.size())
            throw std::runtime_error("Too many mission equipment assignments");
        for (std::uint32_t j = 0; j != triplets; ++j)
            for (auto &word : value.assignments[j])
                word = B::integer(input.word());
        const auto required = input.count(8);
        for (std::uint32_t j = 0; j != required; ++j)
            value.required.push_back({{B::integer(input.word()), B::integer(input.word())}});
    }
}

} // namespace hd2::frontend
#endif
