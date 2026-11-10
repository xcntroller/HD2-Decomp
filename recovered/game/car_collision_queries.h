#ifndef HD2_GAME_CAR_COLLISION_QUERIES_H
#define HD2_GAME_CAR_COLLISION_QUERIES_H

#include "game/collision_ray.h"
#include "game/collision_box_query.h"
#include "game/car_contact_callbacks.h"

namespace hd2::game {

//Actual primitive identities for00512390/00514D70. Flags/surface come from
//the SAME tree record or dynamic descriptor; no synthesized material/actor.
template<class Frame,class Actor>struct CarCollisionHit {
    CollisionSweepState::Vector position{},normal{};
    const void* identity=nullptr;std::uint32_t flags=0;
    CarContact<Actor,Frame> contact{};
};

template<class Frame,class Object,class Resources,class Math,class Actor>
class CarCollisionQueries {
public:
    using Tree=CollisionTree<Frame>;using Grid=typename Tree::DynamicGrid;
    using Buckets=DynamicCollisionBuckets<Object,Grid>;using Vector=CollisionSweepState::Vector;
    using Hit=CarCollisionHit<Frame,Actor>;
    CarCollisionQueries(const Tree& tree,const Buckets& buckets,Resources& resources,Math& math)
        :tree_(tree),ray_(&tree,&buckets,resources,math),sweep_(&tree,&buckets,resources,math),box_(&tree,&buckets,resources,math){}
    std::optional<Hit> ray(const Vector& start,const Vector& displacement,std::uint32_t layers,std::uint32_t flags,bool normal=true){
        return convert(ray_.ray(start,displacement,layers,flags,normal)); //007183A0 ->006F5890/006F7C90.
    }
    std::optional<Hit> sweep(const Vector& start,const Vector& displacement,float radius,
            std::uint32_t layers,std::uint32_t flags){return convert(sweep_.sweep(start,displacement,radius,layers,flags));}
    template<class Bounds>bool box(const Bounds& bounds,const typename Math::Matrix& before,const typename Math::Matrix& after,
            const Vector& displacement,const Vector& old_up,const Vector& up,std::uint32_t layers,std::uint32_t flags){
        return box_.query(bounds,before,after,displacement,old_up,up,layers,flags);
    }
private:
    using SourceHit=typename CollisionSweep<Frame,Object,Resources,Math>::Hit;
    std::optional<Hit> convert(const std::optional<SourceHit>& source)const{
        if(!source)return std::nullopt;
        Hit result;result.position=source->position;result.normal=source->normal;
        if(const auto* object=source->dynamic_identity){
            result.identity=object;result.flags=object->flags;
            result.contact.frame=object->frame;result.contact.flags8=object->mask;
            result.contact.actor=static_cast<Actor>(object->actor);
        }else if(const auto identity=source->static_identity){
            if(identity->kind){
                const auto& shape=tree_.shapes.at(identity->kind-0x80).at(identity->index);
                result.identity=&shape;result.flags=shape.flags;result.contact.frame=shape.frame;
            }else{
                const auto& triangle=tree_.triangles.at(identity->index);
                result.identity=&triangle;result.flags=triangle.flags;
                if(result.flags&0x40u)throw std::runtime_error("Car triangle cannot expose a descriptor frame/actor");
            }
        }else throw std::logic_error("Car collision query lost its actual primitive identity");
        result.contact.flags=static_cast<std::uint8_t>(result.flags);
        result.contact.surface=static_cast<std::uint8_t>(result.flags>>24);
        return result;
    }
    const Tree& tree_;CollisionRay<Frame,Object,Resources,Math> ray_;CollisionSweep<Frame,Object,Resources,Math> sweep_;
    CollisionBoxQuery<Frame,Object,Resources,Math> box_;
};
} //namespace hd2::game
#endif
