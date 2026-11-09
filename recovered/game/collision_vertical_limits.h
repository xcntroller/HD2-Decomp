#ifndef HD2_GAME_COLLISION_VERTICAL_LIMITS_H
#define HD2_GAME_COLLISION_VERTICAL_LIMITS_H

#include "game/collision_tree.h"
#include <cstring>

namespace hd2::game {
struct CollisionVerticalLimits {
    float minimum=std::numeric_limits<float>::max();
    float maximum=-std::numeric_limits<float>::max();
};

//006FDA00, consumed by00763780. Inspect the SAME loaded static collision tree:
//10000 triangles and family83's transformed X/Z rectangles. The empty/no-tree
//sentinels are original results, not a substituted ground plane.
template<class Frame,class Resources,class Math>
CollisionVerticalLimits collision_vertical_limits(const CollisionTree<Frame>* tree,Resources& resources,Math& math) {
    CollisionVerticalLimits result;
    if(!tree)return result;
    for(const auto& triangle:tree->triangles)if(triangle.flags&0x10000) {
        const auto vertices=collision_triangle_vertices(*tree,triangle,resources);
        for(const auto& vertex:vertices) {
            const auto y=vertex[1];
            if(!(result.minimum<y))result.minimum=y;
            if(!(y<result.maximum))result.maximum=y;
        }
    }
    for(const auto& rectangle:tree->shapes[3])if(rectangle.flags&0x10000) {
        if(rectangle.fields8.size()<22)throw std::runtime_error("Collision vertical-limit rectangle is truncated");
        auto real=[&](unsigned offset) {float value;const auto bits=rectangle.fields8.at((offset-8)/4);std::memcpy(&value,&bits,4);return value;};
        typename Math::Matrix matrix{};
        for(unsigned index=0;index!=16;++index)matrix[index]=real(0x20+index*4);
        const std::array<std::array<float,4>,4> corners{{
            {{real(8),0,real(0x10),0}},{{real(8),0,real(0x1c),0}},
            {{real(0x14),0,real(0x10),0}},{{real(0x14),0,real(0x1c),0}}
        }};
        for(const auto& corner:corners) {
            const auto y=math.car_transform(corner,matrix)[1];
            if(y<=result.minimum)result.minimum=y;
            if(result.maximum<=y)result.maximum=y;
        }
    }
    return result;
}
} //namespace hd2::game
#endif
