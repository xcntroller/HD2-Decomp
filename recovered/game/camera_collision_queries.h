#ifndef HD2_GAME_CAMERA_COLLISION_QUERIES_H
#define HD2_GAME_CAMERA_COLLISION_QUERIES_H

#include "game/collision_ray.h"

namespace hd2::game {

//00468130 consumes the actual first two words of the returned primitive.
//Keep identity as a borrowed pointer with host width, never a serialized DWORD.
template<class Frame>struct CameraCollisionHit {
    CollisionSweepState::Vector position{},normal{};
    const void* identity=nullptr;
    std::uint32_t flags=0;
    Frame frame{};
};

template<class Frame,class Object,class Resources,class Math>
class CameraCollisionQueries {
public:
    using Tree=CollisionTree<Frame>;using Grid=typename Tree::DynamicGrid;
    using Buckets=DynamicCollisionBuckets<Object,Grid>;
    using Vector=CollisionSweepState::Vector;using Hit=CameraCollisionHit<Frame>;
    CameraCollisionQueries(const Tree& tree,const Buckets* buckets,Resources& resources,Math& math)
        :tree_(tree),sweep_(&tree,buckets,resources,math),ray_(&tree,buckets,resources,math){}
    std::optional<Hit> sweep(const Vector& start,const Vector& displacement,float radius,
                            std::uint32_t layers,std::uint32_t flags) {
        return convert(sweep_.sweep(start,displacement,radius,layers,flags));
    }
    std::optional<Hit> ray(const Vector& start,const Vector& displacement,
                          std::uint32_t layers,std::uint32_t flags) {
        return convert(ray_.ray(start,displacement,layers,flags));
    }
private:
    using SourceHit=typename CollisionSweep<Frame,Object,Resources,Math>::Hit;
    std::optional<Hit> convert(const std::optional<SourceHit>& source)const {
        if(!source)return std::nullopt;
        Hit result;result.position=source->position;result.normal=source->normal;
        if(const auto* object=source->dynamic_identity) {
            result.identity=object;result.flags=object->flags;result.frame=object->frame;
        }else if(const auto id=source->static_identity) {
            if(id->kind) {
                const auto& shape=tree_.shapes.at(id->kind-0x80).at(id->index);
                result.identity=&shape;result.flags=shape.flags;result.frame=shape.frame;
            }else {
                const auto& triangle=tree_.triangles.at(id->index);
                result.identity=&triangle;result.flags=triangle.flags;
                //Native triangle+04 is vertex0, NOT a frame. bit40 belongs to
                //dynamic descriptors; reject a producer that violates this.
                if(triangle.flags&0x40u)throw std::runtime_error("Camera triangle unexpectedly exposes descriptor-frame flag40");
            }
        }else throw std::logic_error("Camera collision hit lost its actual identity");
        return result;
    }
    const Tree& tree_;CollisionSweep<Frame,Object,Resources,Math> sweep_;
    CollisionRay<Frame,Object,Resources,Math> ray_;
};
} //namespace hd2::game
#endif
