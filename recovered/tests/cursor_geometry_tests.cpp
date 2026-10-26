#include "host/cursor_geometry.h"

#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>

int main() {
    using namespace hd2::host;
    const auto center = cursor_position(400, 300, 800, 600, 800, 600);
    assert(center && center->x == 400 && center->y == 300);
    const auto scaled = cursor_position(800, 600, 1600, 1200, 800, 600);
    assert(scaled && scaled->x == 400 && scaled->y == 300);
    for (const auto point : {std::array<int, 2>{-1, 0}, {0, -1}, {800, 0}, {0, 600}}) {
        assert(!cursor_position(point[0], point[1], 800, 600, 800, 600));
    }
    const auto quad = cursor_quad(*center, 800, 600);
    assert(quad[0].x == 400 && quad[0].y == 300);
    assert(quad[1].x == 423.4375f && quad[1].y == 300);
    assert(quad[2].x == 423.4375f && quad[2].y == 323.4375f);
    assert(quad[3].x == 400 && quad[3].y == 323.4375f);
    const auto reference = cursor_quad({0, 0}, 1024, 768);
    assert(reference[2].x == 30 && reference[2].y == 30);
    const auto hd = cursor_quad({10, 20}, 1920, 1080);
    assert(hd[2].x == 66.25f && hd[2].y == 62.1875f);
    const float uv[][2] = {{0, 0}, {0, 1}, {1, 1}, {1, 0}};
    for (std::size_t i = 0; i < quad.size(); ++i) {
        assert(quad[i].u == uv[i][0] && quad[i].v == uv[i][1]);
        assert(quad[i].diffuse == 0xffffffff && quad[i].specular == 0);
        std::uint32_t z = 0, rw = 0;
        std::memcpy(&z, &quad[i].z, sizeof(z));
        std::memcpy(&rw, &quad[i].reciprocal_w, sizeof(rw));
        assert(z == 0x3ce8bf06 && rw == 0x420cc9df);
    }
    for (int dimension = 0; dimension < 4; ++dimension) {
        for (int invalid : {0, -1}) {
            int sizes[] = {800, 600, 800, 600};
            sizes[dimension] = invalid;
            bool rejected = false;
            try {
                cursor_position(0, 0, sizes[0], sizes[1], sizes[2], sizes[3]);
            } catch (const std::invalid_argument&) {
                rejected = true;
            }
            assert(rejected);
        }
    }
    for (const auto size : {std::array<int, 2>{0, 600}, {-1, 600}, {800, 0}, {800, -1}}) {
        bool rejected = false;
        try {
            cursor_quad({0, 0}, size[0], size[1]);
        } catch (const std::invalid_argument&) {
            rejected = true;
        }
        assert(rejected);
    }
    std::cout << "Cursor geometry, original UV/depth bytes, scaling and bounds passed\n";
}
