#ifndef HD2_GAME_MISSION_SCENE_READER_H
#define HD2_GAME_MISSION_SCENE_READER_H

#include "game/serialized_bytes.h"
#include <array>
#include <map>
#include <optional>
#include <utility>

namespace hd2::game {

using MissionVector = std::array<float, 3>;
using MissionQuaternion = std::array<float, 4>;
template<std::size_t N> inline std::array<float, N> mission_reals(SerializedBytes& bytes) {
    std::array<float, N> values{};
    for (auto& value : values) value = bytes.real();
    return values;
}

// Persistent fields written by006CC760. A later actors/sounds file has no0001
// header and uses the version established by Scene2; do not reset per file.
struct MissionSceneFields {
    std::uint16_t version = 0, revision = 0;
    float field_of_view = 1.39626348f, near_clip = .1f, far_clip = 500.f;
    std::uint32_t field_d8 = 0, field_c8 = 0xffffffffu, field_cc = 0xffffffffu;
};

//006CC760/006CD940's runtime-selected Tutorial/ALPS1 dependency set. The
// backend owns real frames/actors and all engine/resource side effects. Every
// reached operation is mandatory; there is no default-success service. Existing
// scene frames must be registered before reading the first document. Unknown
// chunks follow the original skip branch; known unimplemented required kinds
// are explicitly rejected. Broader historical/projection/volume branches remain
// outside this reader and do not earn whole-body completion credit.
template<class Backend>
class MissionSceneReader {
public:
    using Frame = typename Backend::Frame;
    using Actor = typename Backend::Actor;
    using Names = std::map<std::string, Frame>;
    MissionSceneReader(Backend& backend, Names& names, MissionSceneFields& fields)
        : backend_(backend), names_(names), fields_(fields) {}

    void read(SerializedBytes bytes, unsigned stage) {
        document_begin_ = bytes.take(0);
        document_size_ = bytes.remaining();
        auto root = next_mission_chunk(bytes);
        if (!root || root->id != 0x4c53) throw std::runtime_error("Invalid mission scene root");
        read_root(root->bytes, stage);
        //Original callers leave the first root and close the file; trailing
        //bytes outside it do not enter root dispatch.
    }
private:
    Frame lookup(const std::string& name) { return names_[name]; } //Map inserts null, as original.
    void progress(unsigned stage, SerializedBytes& cursor) {
        const auto offset = static_cast<std::size_t>(cursor.take(0) - document_begin_);
        backend_.scene_progress(stage, document_size_ ? float(offset) / float(document_size_) : 1.f);
    }
    void read_root(SerializedBytes& bytes, unsigned stage) {
        while (auto chunk = next_mission_chunk(bytes)) {
            auto& data = chunk->bytes;
            progress(stage, data);
            switch (chunk->id) {
            case 1:
                fields_.version = data.half(); fields_.revision = data.half();
                if (fields_.version != 7) throw std::runtime_error("Unsupported mission scene version");
                break;
            case 0x3010: fields_.field_of_view = data.real(); break;
            case 0x3011: fields_.near_clip = .1f; fields_.far_clip = data.real(); break;
            case 0x3012: backend_.orthographic_camera(); break;
            case 0x3200: backend_.scene_background(mission_reals<3>(data)); break;
            case 0x3211: {
                auto range = mission_reals<2>(data);
                if (range[0] <= .1f) range[0] = .1f;
                backend_.scene_range(range);
                break;
            }
            case 0x4000:
                while (data.remaining()) {
                    progress(stage, data);
                    auto object = next_mission_chunk(data);
                    if (object->id == 0x4010) read_object(object->bytes);
                }
                break;
            case 0xae00: backend_.read_lightmap_references(data, names_); break;
            case 0xae20: read_actors(data); break;
            case 0xae40: backend_.read_scene_tables(data); break;
            case 0xb004: backend_.scene_driver_flag(data.byte()); break;
            case 0xb600: backend_.scene_graphics_word(data.word()); break;
            case 0xb650: fields_.field_d8 = data.word(); break;
            case 0xb700: backend_.scene_texture_parameters(data); break;
            case 0xb810: backend_.read_scene_lightmap_aliases(data); break;
            case 0xb820:
                while (auto entry = next_mission_chunk(data)) {
                    if (entry->id != 0xb820) break; //Original stops at first other subchunk.
                    auto frame = backend_.find_scene_frame(entry->bytes.text(), 0xffffu);
                    if (frame) backend_.register_scene_frame(frame);
                }
                break;
            case 0xb840: fields_.field_c8 = data.word(); break;
            case 0xb880: fields_.field_cc = data.word(); break;
            default: break; //006CC760's actual unknown-chunk skip.
            }
        }
    }
    void read_actors(SerializedBytes& bytes) {
        while (auto entry = next_mission_chunk(bytes)) {
            if (entry->id != 0xae21) continue;
            Frame frame{};
            Actor actor{};
            while (auto chunk = next_mission_chunk(entry->bytes)) {
                auto& data = chunk->bytes;
                switch (chunk->id) {
                case 0xae23: {
                    const auto name = data.text();
                    frame = lookup(name);
                    if (!frame) frame = backend_.find_scene_frame(name, 0xcffffu);
                    if (!frame) frame = backend_.resolve_scene_path(name);
                    break;
                }
                case 0xae22:
                    actor = backend_.create_actor(data.word(), frame);
                    if (!actor || !backend_.initialize_actor(actor, frame))
                        throw std::runtime_error("Required mission actor initialization failed");
                    break;
                case 0xae24:
                    if (actor && data.remaining()) backend_.read_actor_properties(actor, data);
                    break;
                default: break;
                }
            }
        }
    }
    void read_object(SerializedBytes& bytes) {
        Frame frame{}, parent{};
        std::string name;
        std::uint32_t kind = 0, subtype = 0, flags = 0;
        std::optional<MissionVector> position, world_position, scale;
        std::optional<MissionQuaternion> rotation;
        while (auto chunk = next_mission_chunk(bytes)) {
            auto& data = chunk->bytes;
            switch (chunk->id) {
            case 0x4011: kind = data.word(); break;
            case 0x4012: subtype = data.word(); break;
            case 0x10:
                name = data.text();
                if (!kind) { frame = lookup(name); break; }
                if (lookup(name)) break; //Original declines duplicate explicit creation.
                switch (kind) {
                case 1: case 2: case 3: case 4: case 6: case 8: case 9: case 12: case 14: break;
                default: throw std::runtime_error("Unsupported required mission frame kind");
                }
                frame = backend_.create_scene_object(kind, subtype, name);
                if (!frame) throw std::runtime_error("Required mission frame creation failed");
                flags |= 0x20000;
                names_[backend_.frame_name(frame)] = frame;
                break;
            case 0x20: position = mission_reals<3>(data); flags |= 1; break;
            case 0x22: rotation = mission_reals<4>(data); flags |= 2; break;
            case 0x2c: world_position = mission_reals<3>(data); break;
            case 0x2d: scale = mission_reals<3>(data); flags |= 4; break;
            case 0x2012:
                if (frame && backend_.frame_kind(frame) == 9) {
                    const auto resource = data.text();
                    if (resource.empty()) {
                        backend_.remove_model(frame);
                        names_[name] = Frame{};
                        frame = Frame{};
                    } else backend_.load_scene_model(frame, resource);
                }
                break;
            case 0x4020:
                if (frame) {
                    auto entry = next_mission_chunk(data);
                    if (entry && entry->id == 0x10) {
                        parent = lookup(entry->bytes.text());
                        if (parent) flags |= 0x10;
                        else if (world_position) position = world_position;
                    }
                }
                break;
            case 0x4033: flags |= 0x10000; break;
            case 0x4036:
                if (frame && backend_.frame_kind(frame) == 3)
                    while (auto entry = next_mission_chunk(data)) {
                        if (entry->id == 0x3010) backend_.camera_fov(frame, entry->bytes.real());
                        else if (entry->id == 0x3011) backend_.camera_range(frame, .1f, entry->bytes.real());
                    }
                break;
            case 0x4040:
                if (frame && backend_.frame_kind(frame) == 2) read_light(frame, data);
                break;
            case 0x4050:
                if (frame && backend_.frame_kind(frame) == 6)
                    backend_.dummy_bounds(frame, mission_reals<6>(data));
                break;
            case 0x4060:
                if (frame && backend_.frame_kind(frame) == 4) read_sound(frame, data);
                break;
            case 0x4090:
                if (frame && backend_.frame_kind(frame) == 1) {
                    const auto requested = data.word();
                    if (backend_.visual_type(frame) != requested) {
                        frame = backend_.replace_scene_visual(frame, requested);
                        if (!frame) throw std::runtime_error("Required visual replacement failed");
                        names_[name] = frame;
                    }
                }
                break;
            case 0x40a0:
                if (frame && fields_.version > 2 && backend_.visual_type(frame) == 0x50414d4cu)
                    backend_.read_scene_lightmap(frame, data);
                break;
            case 0xb110:
                if (frame) while (auto entry = next_mission_chunk(data))
                    if (entry->id >= 0xb112 && entry->id <= 0xb117)
                        backend_.visual_environment(frame, entry->id, entry->bytes.word());
                break;
            case 0xb150:
                if (frame && backend_.frame_kind(frame) == 14) read_geometry(frame, data);
                break;
            //Other known resource branches need their actual consumers before
            //they can be accepted. These do not occur in the captured baseline.
            case 0x4080:
                if (frame && backend_.frame_kind(frame) == 12)
                    throw std::runtime_error("Unsupported required mission occluder payload");
                break;
            case 0x40f0:
                if (frame && backend_.visual_type(frame) == 0x4a4f5250u)
                    throw std::runtime_error("Unsupported required mission projector payload");
                break;
            case 0xb401:
                if (frame && backend_.frame_kind(frame) == 5)
                    throw std::runtime_error("Unsupported required mission sector payload");
                break;
            default: break; //006CD940's actual unknown-chunk skip.
            }
        }
        if (!frame) return;
        //006D1873..006D1987, verified against assembly (decompiler aliases the
        //flags word with a string pointer). Apply parent BEFORE rotation/scale.
        if (flags & 1) backend_.object_position(frame, *position);
        backend_.update_scene_object(frame);
        if ((flags & 0x20000) && !(flags & 0x10)) backend_.auto_parent_object(frame);
        if (flags & 0x10) {
            if (parent) backend_.link_scene_object(frame, parent);
            else if (!backend_.object_parent(frame)) backend_.auto_parent_object(frame);
            backend_.update_scene_object(frame);
        }
        if (flags & 2) backend_.object_rotation(frame, *rotation);
        if (flags & 4) backend_.object_scale(frame, *scale);
        if (flags & 0x10000) backend_.object_visible(frame, false);
        backend_.update_scene_object(frame);
        if (backend_.frame_kind(frame) == 9 && backend_.model_animation(frame))
            backend_.register_animated_model(frame);
    }
    void read_light(Frame frame, SerializedBytes& bytes) {
        backend_.clear_light_sectors(frame);
        while (auto chunk = next_mission_chunk(bytes)) {
            auto& data = chunk->bytes;
            switch (chunk->id) {
            case 0x4041: backend_.light_type(frame, data.word()); break;
            case 0x26: backend_.light_colours(frame, mission_reals<9>(data)); break;
            case 0x4042: backend_.light_power(frame, mission_reals<3>(data)); break;
            case 0x4043: backend_.light_cone(frame, mission_reals<2>(data)); break;
            case 0x4044: backend_.light_ranges(frame, mission_reals<6>(data)); break;
            case 0x4045: backend_.light_flags(frame, data.word()); break;
            case 0x4046: {
                const auto sector = lookup(data.text());
                if (sector && backend_.frame_kind(sector) == 5) backend_.add_light_sector(frame, sector);
                break;
            }
            default: break;
            }
        }
        backend_.update_scene_object(frame);
    }
    void read_sound(Frame frame, SerializedBytes& bytes) {
        backend_.begin_scene_sound(frame);
        bool loop = false;
        while (auto chunk = next_mission_chunk(bytes)) {
            auto& data = chunk->bytes;
            switch (chunk->id) {
            case 0x10: backend_.load_scene_sound(frame, data.text()); break;
            case 0x4061: case 0x4062: case 0x4063: case 0xb800:
                backend_.sound_scalar(frame, chunk->id, data.word()); break;
            case 0x4064: backend_.sound_cone(frame, mission_reals<2>(data)); break;
            case 0x4065: throw std::runtime_error("Unsupported legacy mission sound ranges");
            case 0x4066: backend_.sound_enable(frame); break;
            case 0x4067: loop = true; break;
            case 0x4068: backend_.sound_ranges(frame, mission_reals<4>(data)); break;
            case 0xb200: case 0xb300: {
                const auto target = lookup(data.text());
                const auto required_kind = chunk->id == 0xb200 ? 5u : 14u;
                if (target && backend_.frame_kind(target) == required_kind)
                    backend_.sound_target(frame, target, required_kind);
                break;
            }
            default: break;
            }
        }
        backend_.finish_scene_sound(frame, loop);
    }
    void read_geometry(Frame frame, SerializedBytes& bytes) {
        std::vector<MissionVector> vertices;
        while (auto chunk = next_mission_chunk(bytes)) {
            if (chunk->id == 0xb170) {
                const auto count = chunk->bytes.word();
                if (count > chunk->bytes.remaining() / 12) throw std::runtime_error("Truncated mission area vertices");
                vertices.clear(); vertices.reserve(count);
                for (std::uint32_t i = 0; i != count; ++i) vertices.push_back(mission_reals<3>(chunk->bytes));
            } else if (chunk->id == 0xb180) {
                const auto count = chunk->bytes.word();
                if (count > chunk->bytes.remaining() / 6) throw std::runtime_error("Truncated mission area indices");
                std::vector<std::array<std::uint16_t, 3>> triangles(count);
                for (auto& triangle : triangles) for (auto& index : triangle) {
                    index = chunk->bytes.half();
                    if (index >= vertices.size()) throw std::runtime_error("Invalid mission area vertex index");
                }
                backend_.area_geometry(frame, vertices, triangles);
            }
        }
    }
    Backend& backend_;
    Names& names_;
    MissionSceneFields& fields_;
    const unsigned char* document_begin_ = nullptr;
    std::size_t document_size_ = 0;
};

} // namespace hd2::game
#endif
