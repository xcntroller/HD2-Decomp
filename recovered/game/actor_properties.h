#ifndef HD2_GAME_ACTOR_PROPERTIES_H
#define HD2_GAME_ACTOR_PROPERTIES_H

#include "game/serialized_bytes.h"
#include <array>
#include <vector>

namespace hd2::game {

// Property consumers used by the actor loader AE24. These own decoded fields;
// they are not opaque copies of the complete serialized attribute blob. Actual
// actor constructors, retained frames, geometry initialization and post-read
// scene changes must be supplied by the matching actor owners before these
// readers can establish complete actor loading.
struct HumanActorProperties {
    std::uint32_t field19c = 3, field1a0 = 1;
    std::array<std::uint32_t, 12> stats_ec{}, stats_11c{};
    std::uint32_t field170 = 0xffffffffu, field1a8 = 0xffffffffu;
    std::array<std::uint8_t, 3> flags1a4{};
    void load(SerializedBytes& bytes) { //0059E800.
        const auto version = static_cast<std::int8_t>(bytes.byte());
        if (version < 11 || version > 15) throw std::runtime_error("Unsupported Human actor properties version");
        field19c = bytes.word(); field1a0 = bytes.word();
        for (auto& word : stats_ec) word = bytes.word();
        field170 = bytes.word();
        if (version > 11) flags1a4[0] = bytes.byte();
        if (version > 12) flags1a4[1] = bytes.byte();
        if (version > 13) flags1a4[2] = bytes.byte();
        if (version > 14) field1a8 = bytes.word();
        if (!field1a8) field1a8 = 0xffffffffu;
        stats_11c = stats_ec;
    }
};

struct LadderActorProperties {
    std::array<std::uint32_t, 4> settings170{};
    std::uint8_t flag180 = 0;
    void load(SerializedBytes& bytes) { //005EBBD0.
        const auto version = static_cast<std::int8_t>(bytes.byte());
        if (version < 1 || version > 3) throw std::runtime_error("Unsupported Ladder actor properties version");
        for (unsigned i = 0; i != 3; ++i) settings170[i] = bytes.word();
        if (version > 1) settings170[3] = bytes.word();
        if (version > 2) flag180 = bytes.byte();
    }
};

struct SupplyBoxActorProperties {
    std::uint32_t field11c = 0, field120 = 0, field128 = 5, field12c = 20;
    std::uint32_t field134 = 0xffffffffu, field210 = 0;
    std::array<std::vector<std::uint32_t>, 2> items;
    void load(SerializedBytes& bytes) { //005FC200. Version byte is read/ignored.
        bytes.byte();
        field11c = bytes.word(); field120 = bytes.word(); field128 = bytes.word();
        field12c = bytes.word(); field134 = bytes.word();
        for (auto& list : items) for (unsigned i = 0; i != 10; ++i) {
            const auto item = bytes.word();
            if (item != 0xffffffffu) list.push_back(item); //Original appends, does not clear.
        }
        field210 = bytes.word();
    }
};

struct PikeActorProperties {
    std::uint32_t field184 = 0, field18c = 0, field190 = 0;
    std::array<std::uint32_t, 3> vector90{}, fields194{};
    void load(SerializedBytes& bytes) { //005B9C80.
        field184 = bytes.word(); field18c = bytes.word(); field190 = bytes.word();
        for (auto& word : vector90) word = bytes.word();
        for (auto& word : fields194) word = bytes.word();
    }
};

struct DogActorProperties {
    std::array<std::uint32_t, 4> fields4a0{};
    void load(SerializedBytes& bytes) { //005D6630; leading DWORD is read/ignored.
        bytes.word();
        for (auto& word : fields4a0) word = bytes.word();
    }
};

inline void require_actor_chunk_version(SerializedBytes& bytes, std::uint32_t required) {
    auto version = next_mission_chunk(bytes);
    if (!version || version->id != 0 || version->bytes.word() != required)
        throw std::runtime_error("Unsupported actor chunk version");
}

struct SpawnPointActorProperties {
    std::uint32_t field90 = 0, field94 = 0, field98 = 0, fielda0 = 0, fielda4 = 0;
    void load(SerializedBytes& bytes) { //006119B0.
        require_actor_chunk_version(bytes, 3);
        while (auto chunk = next_mission_chunk(bytes)) {
            switch (chunk->id) {
            case 1: field90 = chunk->bytes.word(); break;
            case 2: field94 = chunk->bytes.word(); break;
            case 3: fielda4 = chunk->bytes.word(); break;
            case 4: field98 = chunk->bytes.word(); break;
            case 5: fielda0 = chunk->bytes.word(); break;
            default: throw std::runtime_error("Unknown SpawnPoint actor property");
            }
        }
    }
};

struct FowlGeneratorActorProperties {
    std::uint32_t fielda0 = 0, fielda8 = 0, fieldac = 0, fieldb0 = 0;
    std::string name;
    void load(SerializedBytes& bytes) { //005BB7E0.
        require_actor_chunk_version(bytes, 1);
        while (auto chunk = next_mission_chunk(bytes)) {
            switch (chunk->id) {
            case 1: fielda0 = chunk->bytes.word(); break;
            case 2: fielda8 = chunk->bytes.word(); break;
            case 3: fieldac = chunk->bytes.word(); break;
            case 4: fieldb0 = chunk->bytes.word(); break;
            case 5: name = chunk->bytes.text(); break;
            default: throw std::runtime_error("Unknown FowlGenerator actor property");
            }
        }
    }
};

struct AnimationActorProperties {
    std::string name0, name5;
    float fieldd0 = -1.f;
    std::uint32_t fielda8 = 0, fieldac = 0, fieldb0 = 0;
    std::uint8_t flaga2 = 0, flaga4 = 0;
    void load(SerializedBytes& bytes) { //005B5AF0.
        if (!bytes.remaining()) throw std::runtime_error("Empty Animation actor properties");
        while (auto chunk = next_mission_chunk(bytes)) {
            switch (chunk->id) {
            case 0: name0 = chunk->bytes.text(); break;
            case 1: fieldd0 = chunk->bytes.real(); if (fieldd0 < -1.f) fieldd0 = -1.f; break;
            case 2: flaga2 = chunk->bytes.byte(); break;
            case 3: fielda8 = chunk->bytes.word(); break;
            case 4: fieldac = chunk->bytes.word(); break;
            case 5: name5 = chunk->bytes.text(); break;
            case 6: fieldb0 = chunk->bytes.word(); break;
            case 7: flaga4 = chunk->bytes.byte(); break;
            default: throw std::runtime_error("Unknown Animation actor property");
            }
        }
    }
};

struct ExplosivePlaceActorProperties {
    std::uint32_t field9c = 0, fielda0 = 0, fielda4 = 0;
    std::array<std::uint32_t, 4> fieldsa8{};
    std::uint8_t flag98 = 0, flagb8 = 0, flagb9 = 0;
    void load(SerializedBytes& bytes) { //0060CA30.
        while (auto chunk = next_mission_chunk(bytes)) {
            switch (chunk->id) {
            case 100: field9c = chunk->bytes.word(); flag98 = chunk->bytes.byte(); break;
            case 0x65: fielda0 = chunk->bytes.word(); break;
            case 0x66: fielda4 = chunk->bytes.word(); break;
            case 0x67: for (auto& word : fieldsa8) word = chunk->bytes.word(); break;
            case 0x68: flagb8 = chunk->bytes.byte(); break;
            case 0x69: flagb9 = chunk->bytes.byte(); break;
            default: throw std::runtime_error("Unknown ExplosivePlace actor property");
            }
        }
    }
};

struct SwitchActorProperties {
    std::uint32_t fieldd4 = 0, fieldd8 = 0;
    template<class Apply>
    void load(SerializedBytes& bytes, Apply&& apply_state) { //00613E10/006136F0.
        while (auto chunk = next_mission_chunk(bytes)) {
            if (chunk->id == 0) {
                fieldd4 = chunk->bytes.word();
                apply_state(fieldd4); //Required real frame/animation change, in serialized order.
            } else if (chunk->id == 1) fieldd8 = chunk->bytes.word();
            else throw std::runtime_error("Unknown Switch actor property");
        }
    }
};

struct DoorActorProperties {
    std::array<std::uint32_t, 4> fields220{};
    std::array<std::uint32_t, 3> vector210{}, vector200{};
    std::array<std::uint32_t, 9> fields240{{0xffffffffu,0xffffffffu,0xffffffffu,
        0xffffffffu,0xffffffffu,0xffffffffu,0xffffffffu,0xffffffffu,0xffffffffu}};
    std::uint32_t field230 = 0xffffffffu, field114 = 0;
    std::uint8_t flag234 = 0, flag23c = 0, flaga0 = 1, flag98 = 1, flag99 = 1;
    std::uint8_t flag190 = 0, flag235 = 1, flag118 = 0;
    std::string name120;
    template<class RefreshPortal>
    void load(SerializedBytes& bytes, RefreshPortal&& refresh_portal) { //00601AC0.
        for (auto& word : fields220) word = bytes.word();
        for (auto& word : vector210) word = bytes.word();
        field230 = bytes.word(); flag234 = bytes.byte();
        bytes.byte(); bytes.word(); //Historical byte/DWORD fields are read/ignored.
        flag23c = bytes.byte();
        while (auto chunk = next_mission_chunk(bytes)) {
            switch (chunk->id) {
            case 0x65: for (auto& word : fields240) word = chunk->bytes.word(); break;
            case 0x66: flaga0 = chunk->bytes.byte(); break;
            case 0x67: name120 = chunk->bytes.text(); break;
            case 0x68: flag98 = chunk->bytes.byte(); break;
            case 0x69: flag99 = chunk->bytes.byte(); break;
            case 0x6a: for (auto& word : vector200) word = chunk->bytes.word(); break;
            case 0x6b: flag190 = chunk->bytes.byte(); break;
            case 0x6c: flag235 = chunk->bytes.byte(); break;
            case 0x6d: field114 = chunk->bytes.word(); flag118 = chunk->bytes.byte(); break;
            default: throw std::runtime_error("Unknown Door actor property");
            }
        }
        if (flaga0) refresh_portal(); //00600A50; cannot be an empty success service.
        if (!field230) field230 = 0xffffffffu;
    }
};

} // namespace hd2::game
#endif
