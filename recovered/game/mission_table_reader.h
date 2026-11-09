#ifndef HD2_GAME_MISSION_TABLE_READER_H
#define HD2_GAME_MISSION_TABLE_READER_H

#include "game/mission_tables.h"
#include "game/table_storage.h"
#include "game/serialized_bytes.h"
#include <array>
#include <limits>

namespace hd2::game {

// Native representations of0051C7B0's 18-byte record and the two name-resolved
// records0051C870/0051C910. Frame and actor pointers are borrowed from the live
// scene/actor owners, never manufactured from serialized addresses.
struct MissionTableVectorRecord {
    std::array<std::uint32_t, 4> words;
    std::uint16_t value;
};
struct MissionTableReference {
    void* object;
    std::uint16_t value;
};

//00523C00's block reader +0051DE80's version9 decoder. Resolve provides the
// original driver+88 then scene+7C lookup policy, and actual frame-user-data
// actor resolution. Allocation/free use the SAME backend as MissionTables.
// Native bounds/transactional publication replace original unchecked pointers;
// the parser never treats malformed or unsupported required input as success.
template<class Backend, class Resolve>
std::uint32_t read_mission_table(SerializedBytes& file, MissionTableSet& destination,
                                 Backend& backend, Resolve& resolve) {
    const auto version = file.word();
    const auto header_words = version == 0x7ed00101u ? 5u : version == 0x7ed00102u ? 6u :
        version >= 0x7ed00103u && version <= 0x7ed00109u ? 7u : 0u;
    if (!header_words) throw std::runtime_error("Unknown mission-table version");
    std::array<std::uint32_t, 7> header{};
    header[0] = version;
    for (unsigned i = 1; i != header_words; ++i) header[i] = file.word();
    if (header[1] < header_words * 4u)
        throw std::runtime_error("Mission-table size is smaller than its header");
    auto data = file.section(header[1] - header_words * 4u);
    //The original decoder rejects all versions except9 after clearing its set.
    if (version != 0x7ed00109u)
        throw std::runtime_error("Mission-table decoder requires version9");
    for (const auto i : {2u, 4u, 5u, 6u})
        if (header[i] > 0xffffu) throw std::runtime_error("Mission-table count exceeds WORD storage");
    //Serialized widths give an allocation bound BEFORE allocating any count.
    const auto fixed_bytes = std::uint64_t(header[2]) * 32u + std::uint64_t(header[4]) * 18u +
        std::uint64_t(header[5] + header[6]) * 3u;
    if (fixed_bytes > data.remaining() || header[3] > std::numeric_limits<std::size_t>::max() / 4u)
        throw std::runtime_error("Invalid mission-table allocation counts");
    //The DWORD link capacity also includes optional synthesized sentinel slots.
    if (header[3] > std::uint64_t(header[2]) * 64u)
        throw std::runtime_error("Invalid mission-table link capacity");
    MissionTableSet next;
    struct Cleanup {
        MissionTableSet& set; Backend& backend; bool published = false;
        ~Cleanup() { if (!published) clear_mission_table_set(set, backend); }
    } cleanup{next, backend};
    next.count0 = static_cast<std::uint16_t>(header[2]);
    next.count0c = static_cast<std::uint16_t>(header[4]);
    next.count14 = static_cast<std::uint16_t>(header[5]);
    next.count1c = static_cast<std::uint16_t>(header[6]);
    if (next.count0) {
        next.records4 = backend.allocate_table_records(next.count0);
        //Original query/scratch arrays are allocated here, not serialized.
        next.data24 = backend.allocate_table_storage(std::size_t(next.count0) * 4u);
        next.data28 = backend.allocate_table_storage(std::size_t(next.count0) * 2u);
    }
    next.data8 = backend.allocate_table_storage(std::size_t(header[3]) * 4u);
    next.data10 = backend.allocate_table_storage(std::size_t(next.count0c) * sizeof(MissionTableVectorRecord));
    next.data18 = backend.allocate_table_storage(std::size_t(next.count14) * sizeof(MissionTableReference));
    next.data20 = backend.allocate_table_storage(std::size_t(next.count1c) * sizeof(MissionTableReference));
    auto* links = static_cast<unsigned char*>(next.data8);
    std::size_t used = 0;
    for (unsigned i = 0; i != next.count0; ++i) {
        auto& record = static_cast<MissionTableRecord*>(next.records4)[i];
        const auto* serialized = data.take(32);
        std::memcpy(record.prefix.data(), serialized, 20);
        record.flags = static_cast<std::uint16_t>(serialized[20] | (std::uint16_t(serialized[21]) << 8));
        record.byte16 = serialized[22];
        //00524620 permutes the three serialized bytes17/18/19.
        record.reserved17[0] = serialized[24];
        record.reserved17[1] = serialized[25];
        record.reserved17[2] = serialized[23];
        std::memcpy(record.reserved17.data() + 3, serialized + 26, 6);
        record.flags |= 0x0400u; //Payload borrows the table's shared link array.
        const auto count = record.flags & 0x3fu;
        const auto capacity = count + ((record.flags & 0x40u) ? 1u : 0u);
        if (used + capacity > header[3]) throw std::runtime_error("Mission-table links exceed allocated capacity");
        record.payload = links ? links + used * 4u : nullptr;
        for (unsigned j = 0; j != count; ++j) {
            const auto first = data.half(), second = data.half();
            auto* p = links + (used + j) * 4u;
            p[0] = static_cast<unsigned char>(first);
            p[1] = static_cast<unsigned char>(first >> 8);
            p[2] = static_cast<unsigned char>(second);
            p[3] = static_cast<unsigned char>(second >> 8);
        }
        if (record.flags & 0x40u) {
            auto* p = links + (used + count) * 4u;
            p[0] = p[1] = 0xff; p[2] = p[3] = 0;
        }
        used += capacity;
    }
    for (unsigned i = 0; i != next.count0c; ++i) {
        auto& record = static_cast<MissionTableVectorRecord*>(next.data10)[i];
        record.value = data.half();
        for (auto& word : record.words) word = data.word();
    }
    for (unsigned family = 0; family != 2; ++family) {
        const auto count = family ? next.count1c : next.count14;
        auto* references = static_cast<MissionTableReference*>(family ? next.data20 : next.data18);
        for (unsigned i = 0; i != count; ++i) {
            references[i].value = data.half();
            const auto name = data.text();
            references[i].object = nullptr;
            if (!name.empty()) references[i].object = family ? resolve.table_actor(name) : resolve.table_frame(name);
        }
    }
    data.exact_end(); //00523C00 compares the returned pointer to blob+size.
    clear_mission_table_set(destination, backend);
    //0051DD60 preserves the query vector/capacity across a reload.
    next.begin30 = destination.begin30;
    next.end34 = destination.end34;
    next.limit38 = destination.limit38;
    destination = next;
    cleanup.published = true;
    return version;
}

} // namespace hd2::game
#endif
