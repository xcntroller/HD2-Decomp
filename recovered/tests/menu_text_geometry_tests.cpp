#include "host/menu_text_geometry.h"

#include <cassert>
#include <cstring>
#include <iostream>
#include <limits>

template<class Operation>
void rejects(Operation operation) {
    bool rejected = false;
    try { operation(); } catch (const std::invalid_argument&) { rejected = true; }
    assert(rejected);
}

int main() {
    using namespace hd2::host;
    const std::array<float, 8> bounds{0, -.75f, 0, 0, 1, 0, 0, 0};
    const auto full = menu_rectangle(bounds, 800, 600);
    assert(full.left == 0 && full.right == 800 && full.top == 1 && full.bottom == 599);
    assert(full.contains({0, 1}) && full.contains({799, 598}));
    assert(!full.contains({800, 1}) && !full.contains({0, 599}));
    assert(!full.contains({-1, 1}) && !full.contains({0, 0}));
    const auto doubled = menu_rectangle(bounds, 1600, 1200);
    assert(doubled.right == 1600 && doubled.bottom == 1199);
    rejects([&] { menu_rectangle(bounds, 0, 600); });
    for (float invalid : {std::numeric_limits<float>::quiet_NaN(),
                          std::numeric_limits<float>::infinity()}) {
        auto broken = bounds;
        broken[0] = invalid;
        rejects([&] { menu_rectangle(broken, 800, 600); });
    }
    auto broken = bounds;
    broken[3] = std::numeric_limits<float>::quiet_NaN();
    assert(menu_rectangle(broken, 800, 600).right == 800);
    broken = bounds;
    broken[4] = -1;
    rejects([&] { menu_rectangle(broken, 800, 600); });

    FontMetrics metrics{};
    metrics['A'] = {.1f, .2f, .3f, .4f, .5f};
    metrics['B'] = {.4f, .5f, .6f, .7f, .7f};
    const GlyphMetrics rounding_case{0, 0, 1, 1, 5.0f / 12.0f};
    assert(glyph_advance(rounding_case, 30) == 12);
    const MenuRectangle rectangle{100, 200, 300, 240};
    const auto vertices = menu_label_vertices("A B", metrics, rectangle, .04f, 600,
                                              false, 0xff123456);
    assert(vertices.size() == 12);
    assert(vertices[0].x == 100 && vertices[0].y == 232);
    assert(vertices[1].x == 100 && vertices[1].y == 208);
    assert(vertices[2].x == 112 && vertices[2].y == 208);
    assert(vertices[0].u == .1f && vertices[0].v == .4f);
    assert(vertices[1].u == .1f && vertices[1].v == .2f);
    assert(vertices[2].u == .3f && vertices[2].v == .2f);
    assert(vertices[3].u == .3f && vertices[3].v == .4f);
    assert(vertices[6].x == 122 && vertices[8].x == 139);
    for (const auto& vertex : vertices) {
        assert(vertex.diffuse == 0xff123456 && vertex.specular == 0);
        std::uint32_t z = 0, rhw = 0;
        std::memcpy(&z, &vertex.z, 4);
        std::memcpy(&rhw, &vertex.reciprocal_w, 4);
        assert(z == 0x3ce8bf06 && rhw == 0x420cc9df);
    }
    const auto title = menu_label_vertices("AB", metrics, rectangle, .05f, 600,
                                           true, 0xff000000);
    assert(title[0].x == 264 && title[0].y == 240);
    assert(title[1].y == 210 && title[8].x == 300);
    assert(menu_label_vertices("", metrics, rectangle, .04f, 600, false, 0).empty());
    assert(menu_label_vertices("  ", metrics, rectangle, .04f, 600, false, 0).empty());
    const auto half_height = menu_label_vertices("A", metrics, rectangle, .02f, 625, false, 0);
    assert(half_height[0].y - half_height[1].y == 12);
    rejects([&] { menu_label_vertices("\n", metrics, rectangle, .04f, 600, false, 0); });
    rejects([&] { menu_label_vertices("A", metrics, rectangle, 0, 600, false, 0); });

    ExitButton button;
    assert(!button.release(true));
    button.press(false);
    assert(!button.release(true));
    button.press(true);
    assert(!button.release(false));
    button.press(true);
    button.cancel();
    assert(!button.release(true));
    button.press(true);
    assert(button.release(true));
    assert(!button.release(true));
    assert(menu_label_bindings.size() == 13);
    assert(menu_label_bindings[10].id == 2112);
    assert(std::string_view(menu_label_bindings[10].visual) == "normal07");
    std::cout << "Original menu bounds, alignment, glyph triangles and Exit click tests passed\n";
}
