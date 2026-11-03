#ifndef HD2_GAME_TABLE_STORAGE_H
#define HD2_GAME_TABLE_STORAGE_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <new>

namespace hd2::game {

//00524600/00524610/00524840. Native pointer-width record; on i386 the original
// allocation has a four-byte cookie,36-byte stride and payload pointer at+20.
// A64-bit audit adds cookie alignment/pointer width only; serialized data never
// passes through this native record representation.
struct MissionTableRecord {
    std::array<unsigned char, 0x14> prefix;
    std::uint16_t flags;
    std::uint8_t byte16;
    std::array<unsigned char, 9> reserved17;
    void* payload;
};
static_assert(offsetof(MissionTableRecord, payload) == 0x20);
#if UINTPTR_MAX == 0xffffffffu
static_assert(sizeof(MissionTableRecord) == 0x24);
#endif

class TableStorage {
public:
    static constexpr std::size_t cookie_bytes = alignof(MissionTableRecord);
    static MissionTableRecord* allocate_table_records(std::uint32_t count) {
        if (count > (std::numeric_limits<std::size_t>::max() - cookie_bytes) / sizeof(MissionTableRecord))
            throw std::bad_array_new_length();
        auto* block = static_cast<unsigned char*>(std::malloc(cookie_bytes + count * sizeof(MissionTableRecord)));
        if (!block) throw std::bad_alloc();
        auto* records = reinterpret_cast<MissionTableRecord*>(block + cookie_bytes);
        std::memcpy(reinterpret_cast<unsigned char*>(records) - 4, &count, 4);
        for (std::uint32_t i = 0; i < count; ++i) {
            // Placement default-initialization preserves fields not written by00524600.
            ::new (static_cast<void*>(records + i)) MissionTableRecord;
            records[i].flags = 0;
            records[i].byte16 = 0;
            records[i].payload = nullptr;
        }
        return records;
    }
    static void destroy_table_records(void* pointer) noexcept {
        auto* records = static_cast<MissionTableRecord*>(pointer);
        std::uint32_t count;
        std::memcpy(&count, reinterpret_cast<const unsigned char*>(records) - 4, 4);
        while (count) {
            auto& record = records[--count];
            if (!(record.flags & 0x0400u) && record.payload) {
                std::free(record.payload);
                record.flags &= 0xffc0u;
                record.payload = nullptr;
            }
            record.~MissionTableRecord();
        }
    }
    static void release_table_record_allocation(void* pointer) noexcept {
        std::free(static_cast<unsigned char*>(pointer) - cookie_bytes);
    }
    static void release_table_storage(void* pointer) noexcept { std::free(pointer); }
};

} // namespace hd2::game
#endif
