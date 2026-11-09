#ifndef HD2_GAME_COLLISION_TREE_H
#define HD2_GAME_COLLISION_TREE_H

#include "game/serialized_bytes.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace hd2::game {

using CollisionPoint = std::array<float, 3>;

template<class Frame>
struct CollisionTriangle {
    std::uint32_t flags = 0;
    Frame frame{};
    bool local_vertices = false;
    std::array<std::uint32_t, 3> vertex_indices{};
    std::array<float, 4> plane{};
};

template<class Frame>
struct CollisionShape {
    //The serialized shape's first two DWORDs are flags and frame index. The
    //remaining scalar/matrix fields retain their original byte offsets minus8.
    //No serialized address becomes a native pointer.
    std::uint32_t flags = 0;
    Frame frame{}; //Borrowed from the containing loaded scene.
    std::vector<std::uint32_t> fields8;
};

struct CollisionCellReference {
    std::uint8_t kind = 0; //0 triangle;80..85/87 named shape families.
    std::size_t index = 0;
};

template<class Frame>
struct CollisionTree {
    std::array<float, 6> bounds_and_steps{}; //Header00..17, including stored grid steps.
    std::array<float, 4> fields20{};
    std::uint32_t count_x = 0, count_z = 0, count_y = 0;
    std::vector<float> edges_x, edges_y, edges_z;
    std::array<std::uint32_t, 4> edge_tail{}; //Four DWORDs after the final Z edge.
    std::vector<CollisionTriangle<Frame>> triangles;
    std::vector<CollisionPoint> local_vertices;
    std::array<std::vector<CollisionShape<Frame>>, 8> shapes;
    std::vector<std::vector<CollisionCellReference>> cells;
    //Serialized cell+08/+0C are live query limits, not disposable scratch.
    //00715510 compares both before dispatching007151D0. Preserve file order
    //within each pair; cells and pairs share native Y,Z,X flattened indices.
    std::vector<std::array<float,2>> cell_fields8;
    struct DynamicGrid {
        std::array<float, 2> origin{}, step{};
        std::uint32_t count_x = 0, count_z = 0;
        std::vector<void*> heads; //Native dynamic-bucket pointers, never engine frames.
    } dynamic;

    std::size_t cell_index(std::size_t x, std::size_t y, std::size_t z) const {
        if (x >= count_x || y >= count_y || z >= count_z)
            throw std::out_of_range("Collision cell coordinates");
        return (count_z * y + z) * count_x + x;
    }
};

inline void reject_degenerate_collision_triangle(std::uint32_t& flags,
                                                 const std::array<CollisionPoint, 3>& vertices) {
    auto low = vertices[0], high = low;
    unsigned score = 0;
    for (unsigned i = 0; i != 3; ++i) {
        float distance = 0;
        for (unsigned axis = 0; axis != 3; ++axis) {
            low[axis] = std::min(low[axis], vertices[i][axis]);
            high[axis] = std::max(high[axis], vertices[i][axis]);
            const auto delta = vertices[i][axis] - vertices[(i + 1) % 3][axis];
            distance += delta * delta;
        }
        if (distance < 1.e-8f) score += 2; //0080F520,32-bit float.
    }
    for (unsigned axis = 0; axis != 3; ++axis)
        if (std::fabs(high[axis] - low[axis]) < 1.e-8f) ++score; //0080FAC8 double→float.
    if (score >= 2) flags = 0x20;
}

inline void classify_collision_triangle(std::uint32_t& flags, const std::array<float, 4>& plane,
                                         const std::array<CollisionPoint, 3>& vertices) {
    //006EE380; only low-byte classification is changed. Shape families have
    //bit80 set and are excluded by the caller, as are rejected triangles(bit20).
    const auto a = std::fabs(plane[0]);
    const auto b = std::fabs(plane[1]);
    const auto c = std::fabs(plane[2]);
    unsigned major = a <= b ? (c <= b ? 9 : 10) : (c <= a ? 8 : 10);
    const auto dx = std::fabs(vertices[1][0] - vertices[0][0]);
    const auto dy = std::fabs(vertices[1][1] - vertices[0][1]);
    const auto dz = std::fabs(vertices[1][2] - vertices[0][2]);
    //008162F8/00810148 are doubles -0.0001/+0.0001, cast to float here.
    if (major == 8) major = dy <= -.0001f || .0001f <= dy ? 2 : 1;
    else if (major == 9) major = .0001f <= dx ? (dz <= -.0001f || .0001f <= dz ? 5 : 4) : 3;
    else major = .0001f <= dx ? 7 : 6;
    flags = (flags & 0xffffff00u) | major;
}

//Keep mesh/frame/index references rather than freezing vertex snapshots: later
//collision consumers must observe the same live geometry as the loaded scene.
template<class Frame, class Services>
std::array<CollisionPoint, 3> collision_triangle_vertices(const CollisionTree<Frame>& tree,
                                                        const CollisionTriangle<Frame>& triangle,
                                                        Services& services) {
    std::array<CollisionPoint, 3> result{};
    for (unsigned i = 0; i != 3; ++i) {
        if (triangle.local_vertices) result[i] = tree.local_vertices.at(triangle.vertex_indices[i]);
        else {
            if (!triangle.frame) throw std::runtime_error("Collision triangle has no mesh frame");
            result[i] = services.collision_vertex(triangle.frame, triangle.vertex_indices[i]);
        }
    }
    return result;
}

//006EE570's file loading/fixups, with006EE380 classification and the dynamic
//grid branch used by006CA780. Services are mandatory real scene consumers:
// Frame; collision_frame(name); collision_is_visual(frame); enable_collision(frame);
// collision_mesh_frame(frame) (kind1, not MIRR, with actual mesh);
// collision_vertex(frame,index) (actual mesh vertex position,32-byte stride).
//A missing frame/mesh follows the original rejected-triangle path; an unavailable
//backend must throw, not manufacture a missing frame to achieve load success.
template<class Services>
CollisionTree<typename Services::Frame> read_collision_tree(SerializedBytes file, Services& services,
                                                            bool make_dynamic_grid) {
    using Frame = typename Services::Frame;
    const auto file_size = file.remaining();
    if (file_size > 128u * 1024u * 1024u) throw std::runtime_error("Collision file exceeds size limit");
    const auto* data = file.take(file_size);
    SerializedBytes header(data, file_size);
    if (header.word() != 0x43666947u) throw std::runtime_error("Invalid collision file signature");
    header.word(); //Original loader does not test version or the last two header DWORDs.
    const auto geometry_offset = header.word();
    const auto frame_count = header.word();
    header.word(); header.word();
    if (geometry_offset < 24 || geometry_offset > file_size || frame_count > (geometry_offset - 24u) / 4u)
        throw std::runtime_error("Invalid collision reference table");
    auto offsets = header.section(frame_count * 4u);
    std::vector<std::uint32_t> name_offsets;
    name_offsets.reserve(frame_count);
    for (std::uint32_t i = 0; i != frame_count; ++i) {
        const auto offset = offsets.word();
        if (offset < 24u + frame_count * 4u || offset >= geometry_offset)
            throw std::runtime_error("Collision name offset exceeds name table");
        name_offsets.push_back(offset);
    }
    std::vector<Frame> frames(frame_count);
    for (std::size_t reverse = frame_count; reverse; --reverse) {
        const auto i = reverse - 1;
        SerializedBytes reference(data + name_offsets[i], geometry_offset - name_offsets[i]);
        const auto flags = reference.word();
        auto frame = services.collision_frame(reference.text());
        if (frame && (flags & 1u)) {
            if (!services.collision_is_visual(frame)) frame = Frame{};
            else services.enable_collision(frame);
        }
        frames[i] = frame;
    }
    SerializedBytes body(data + geometry_offset, file_size - geometry_offset);
    auto structure = body.section(0xa4);
    CollisionTree<Frame> result;
    for (auto& value : result.bounds_and_steps) value = structure.real();
    result.count_x = structure.word(); result.count_z = structure.word();
    for (auto& value : result.fields20) value = structure.real();
    result.count_y = structure.word();
    structure.take(0x14); //Serialized process addresses/scratch38..44 are never used.
    const auto triangle_count = structure.word();
    structure.word();
    std::array<std::uint32_t, 9> counts{};
    for (auto& count : counts) { count = structure.word(); structure.word(); }
    //counts:80,81,82,83,84,local vertices,85,86,87.
    if (!result.count_x || !result.count_z || !result.count_y)
        throw std::runtime_error("Empty collision grid dimensions");
    auto read_edges = [&](std::uint32_t count) {
        if (count > body.remaining() / 4u || count == std::numeric_limits<std::uint32_t>::max())
            throw std::runtime_error("Invalid collision edge count");
        std::vector<float> edges;
        edges.reserve(std::size_t(count) + 1);
        for (std::uint64_t i = 0; i != std::uint64_t(count) + 1; ++i) edges.push_back(body.real());
        return edges;
    };
    result.edges_x = read_edges(result.count_x);
    result.edges_y = read_edges(result.count_y);
    result.edges_z = read_edges(result.count_z);
    for (auto& value : result.edge_tail) value = body.word();
    struct VertexIndices { std::array<std::uint32_t, 3> values; };
    if (triangle_count > body.remaining() / 32u) throw std::runtime_error("Invalid collision triangle count");
    std::vector<VertexIndices> indices(triangle_count);
    result.triangles.resize(triangle_count);
    for (std::size_t i = 0; i != triangle_count; ++i) {
        auto& triangle = result.triangles[i];
        triangle.flags = body.word();
        for (auto& index : indices[i].values) index = body.word();
        for (auto& value : triangle.plane) value = body.real();
    }
    auto frame_at = [&](std::uint32_t index) {
        if (index >= frames.size()) throw std::runtime_error("Collision shape frame index exceeds reference table");
        return frames[index];
    };
    struct ShapeLayout { unsigned family, count_index, stride; };
    constexpr ShapeLayout layouts[] = {{1,1,32},{0,0,192},{4,4,32},{3,3,160},
                                       {2,2,32},{5,6,544},{6,7,160},{7,8,144}};
    for (const auto& layout : layouts) {
        const auto count = counts[layout.count_index];
        if (count > body.remaining() / layout.stride) throw std::runtime_error("Invalid collision shape count");
        auto& shapes = result.shapes[layout.family];
        shapes.reserve(count);
        for (std::uint32_t i = 0; i != count; ++i) {
            CollisionShape<Frame> shape;
            shape.flags = body.word(); shape.frame = frame_at(body.word());
            shape.fields8.reserve((layout.stride - 8u) / 4u);
            for (unsigned offset = 8; offset != layout.stride; offset += 4) shape.fields8.push_back(body.word());
            shapes.push_back(std::move(shape));
        }
    }
    if (std::uint64_t(counts[5]) + 1 > body.remaining() / 16u)
        throw std::runtime_error("Invalid collision local vertex count");
    result.local_vertices.reserve(std::size_t(counts[5]) + 1);
    for (std::uint64_t i = 0; i != std::uint64_t(counts[5]) + 1; ++i) {
        CollisionPoint point{};
        for (auto& value : point) value = body.real();
        body.word(); //Fourth vector component/padding.
        result.local_vertices.push_back(point);
    }
    for (std::size_t i = 0; i != result.triangles.size(); ++i) {
        auto& triangle = result.triangles[i];
        const auto& vertex_indices = indices[i].values;
        const auto frame_index = vertex_indices[0] >> 16;
        for (unsigned j = 0; j != 3; ++j) triangle.vertex_indices[j] = vertex_indices[j] & 0xffff;
        if (frame_index == 0x7fff) {
            triangle.local_vertices = true;
            for (unsigned j = 0; j != 3; ++j) {
                const auto vertex = vertex_indices[j] & 0xffff;
                if (vertex >= result.local_vertices.size()) throw std::runtime_error("Collision local vertex index exceeds table");
            }
        } else {
            const auto frame = frame_at(frame_index);
            if (!frame || !services.collision_mesh_frame(frame)) triangle.flags = 0x20;
            else {
                triangle.frame = frame;
                reject_degenerate_collision_triangle(triangle.flags, collision_triangle_vertices(result, triangle, services));
            }
        }
    }
    if (result.count_x > body.remaining() / 16u / result.count_y / result.count_z)
        throw std::runtime_error("Invalid collision grid size");
    const auto cell_count = std::uint64_t(result.count_x) * result.count_y * result.count_z;
    result.cells.resize(static_cast<std::size_t>(cell_count));
    result.cell_fields8.resize(static_cast<std::size_t>(cell_count));
    //File order Z,Y,X differs from the native flattened index Y,Z,X.
    for (std::uint32_t z = 0; z != result.count_z; ++z)
        for (std::uint32_t y = 0; y != result.count_y; ++y)
            for (std::uint32_t x = 0; x != result.count_x; ++x) {
                const auto count = body.word();
                body.word(); //Serialized pointer is replaced with native ownership.
                const auto cell_index=result.cell_index(x,y,z);
                auto& fields8=result.cell_fields8[cell_index];
                fields8[0]=body.real();fields8[1]=body.real();
                if (count > body.remaining() / 4u) throw std::runtime_error("Invalid collision cell entry count");
                auto& cell = result.cells[cell_index];
                cell.reserve(count);
                for (std::uint32_t i = 0; i != count; ++i) {
                    const auto encoded = body.word();
                    const auto kind = static_cast<std::uint8_t>(encoded >> 24);
                    const auto offset = encoded & 0xffffffu;
                    std::size_t stride = 32, available = result.triangles.size();
                    if (kind) {
                        const auto layout = std::find_if(std::begin(layouts), std::end(layouts),
                            [&](const auto& entry) { return kind == 0x80 + entry.family; });
                        //006EE570 leaves unknown tags and86 as serialized words.
                        //Those are not dereferenceable native references: fail
                        //explicitly instead of publishing an unresolved address.
                        if (layout == std::end(layouts) || kind == 0x86)
                            throw std::runtime_error("Unsupported collision cell reference kind");
                        stride = layout->stride; available = result.shapes[layout->family].size();
                    }
                    if (offset % stride || offset / stride >= available)
                        throw std::runtime_error("Collision cell reference exceeds geometry table");
                    cell.push_back({kind, offset / stride});
                }
            }
    body.exact_end();
    if (make_dynamic_grid) {
        auto& grid = result.dynamic;
        const auto& b = result.bounds_and_steps;
        grid.origin = {b[0],b[1]};
        const auto width = static_cast<long double>(b[2]) - b[0];
        const auto depth = static_cast<long double>(b[3]) - b[1];
        const auto cell_size = result.fields20[0];
        auto dimension = [&](long double extent) {
            const auto value = extent / cell_size;
            if (!std::isfinite(value) || value > std::numeric_limits<std::int32_t>::max() ||
                value < std::numeric_limits<std::int32_t>::min())
                throw std::runtime_error("Invalid dynamic collision grid dimension");
            return static_cast<std::uint32_t>(std::max<std::int32_t>(1, static_cast<std::int32_t>(value)));
        };
        grid.count_x = dimension(width); grid.count_z = dimension(depth);
        grid.step = {static_cast<float>(width / grid.count_x),static_cast<float>(depth / grid.count_z)};
        const auto count = std::uint64_t(grid.count_x) * grid.count_z;
        if (count > 128u * 1024u * 1024u / sizeof(void*)) throw std::runtime_error("Dynamic collision grid exceeds size limit");
        grid.heads.resize(static_cast<std::size_t>(count));
    }
    //006EE380 intentionally visits the first Y layer only (X*Z cell slots).
    for (std::size_t i = 0; i != std::size_t(result.count_x) * result.count_z; ++i)
        for (const auto& entry : result.cells[i]) if (!entry.kind) {
            auto& triangle = result.triangles[entry.index];
            if (!(triangle.flags & 0xa0u))
                classify_collision_triangle(triangle.flags, triangle.plane, collision_triangle_vertices(result, triangle, services));
        }
    return result;
}

} // namespace hd2::game
#endif
