#ifndef HD2_GAME_LIGHTMAP_PAYLOAD_H
#define HD2_GAME_LIGHTMAP_PAYLOAD_H

#include "game/serialized_bytes.h"
#include <limits>

namespace hd2::game {

// Structural preflight for006CD940's40A0 payload delegated to engine10028A60.
// That DLL method reads a raw dta HANDLE, so it cannot enforce the surrounding
// native chunk boundary. Validate every variable-size read before delegation.
// Geometry supplies ACTUAL live mesh metadata, not counts inferred from this
// payload: lod_count(), vertices(lod), faces(lod), groups(lod), group_faces().
// This is a guard around engine delegation, not a recovered/credited DLL body.
template<class Geometry>
void validate_lightmap_payload(SerializedBytes bytes, const Geometry& geometry) {
    auto block = [&](std::uint32_t count, std::size_t stride) {
        if (count > bytes.remaining() / stride) throw std::runtime_error("Truncated mission lightmap array");
        bytes.take(std::size_t(count) * stride);
    };
    auto count = [&](std::size_t stride) {
        const auto value = bytes.word(); block(value, stride); return value;
    };
    const auto mask = bytes.byte();
    for (unsigned layer = 0; layer != 8; ++layer) {
        if (!(mask & (1u << layer))) continue;
        const auto version = bytes.byte(), flags = bytes.byte();
        const auto lods = bytes.word();
        bytes.real(); bytes.real(); //Layer scale/range; interpreted by real engine.
        const auto channel = bytes.byte();
        if ((version != 0x21 && version != 0x22) || lods != geometry.lod_count())
            throw std::runtime_error("Mission lightmap mesh/version mismatch");
        if (channel > 6) throw std::runtime_error("Unsupported mission lightmap channel");
        for (std::uint32_t lod = 0; lod != lods; ++lod) {
            bytes.half(); //Original reads the per-LOD word without using it.
            if (flags & 2) {
                const auto patches = bytes.half(), groups = bytes.half();
                if (groups != geometry.groups(lod)) throw std::runtime_error("Mission lightmap group mismatch");
                for (unsigned patch = 0; patch != patches; ++patch) {
                    const auto solid = bytes.byte();
                    if (solid) bytes.word();
                    const auto images = bytes.word();
                    //Every image consumes at least width/height, including the
                    //solid-colour branch which has no RGB pixel payload.
                    if (images > bytes.remaining() / 8) throw std::runtime_error("Truncated mission lightmap images");
                    for (std::uint32_t image = 0; image != images; ++image) {
                        const auto width = bytes.word(), height = bytes.word();
                        if (!solid) {
                            if (!width || !height || (image == 0 && width == 3 && height == 3))
                                throw std::runtime_error("Invalid mission lightmap image dimensions");
                            if (height > bytes.remaining() / 3 || width > bytes.remaining() / (std::size_t(height) * 3))
                                throw std::runtime_error("Truncated mission lightmap pixels");
                            bytes.take(std::size_t(width) * height * 3);
                        }
                    }
                }
                const auto uv_count = count(8);
                block(uv_count, 4);
                const auto corners = bytes.word();
                if (geometry.faces(lod) > std::numeric_limits<std::uint32_t>::max() / 3 ||
                    corners != geometry.faces(lod) * 3)
                    throw std::runtime_error("Mission lightmap face mismatch");
                //UV indices address the just-read UV table. The original only
                //checks triangle count; the native guard also bounds fixups.
                if (corners > bytes.remaining() / 2) throw std::runtime_error("Truncated mission lightmap UV indices");
                for (std::uint32_t corner = 0; corner != corners; ++corner)
                    if (bytes.half() >= uv_count) throw std::runtime_error("Invalid mission lightmap UV index");
                for (unsigned group = 0; group != groups; ++group) {
                    const auto faces = bytes.word();
                    if (faces != geometry.group_faces(lod, group))
                        throw std::runtime_error("Mission lightmap material face mismatch");
                    if (faces > bytes.remaining() / 2) throw std::runtime_error("Truncated mission lightmap patch indices");
                    for (std::uint32_t face = 0; face != faces; ++face)
                        if (bytes.half() >= patches) throw std::runtime_error("Invalid mission lightmap patch index");
                }
            } else {
                const auto vertices = (flags & 4) ? bytes.word() : geometry.vertices(lod);
                if (vertices != geometry.vertices(lod)) throw std::runtime_error("Mission lightmap vertex mismatch");
                block(vertices, 4);
            }
        }
    }
    bytes.exact_end();
}

} // namespace hd2::game
#endif
