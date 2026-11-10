#ifndef HD2_GAME_DYNAMIC_COLLISION_POINT_QUERIES_H
#define HD2_GAME_DYNAMIC_COLLISION_POINT_QUERIES_H
#include "game/dynamic_collision_grid.h"
#include <array>
namespace hd2::game {
//006FCB60/006FB920. Same registered dynamic descriptors and Z/X bucket order.
//These are point/radius queries, distinct from segment/sphere sweeps. Mask tests
//descriptor+8 ONLY; low primitive tag comes from descriptor+0.
template<class Object,class Grid,class Math>class DynamicCollisionPointQueries {
public:
    using Buckets=DynamicCollisionBuckets<Object,Grid>;using Vector=std::array<float,4>;
    DynamicCollisionPointQueries(const Buckets& buckets,Math& math):buckets_(buckets),math_(math){}
    std::vector<const Object*> radius(const Vector& centre,float radius,std::uint32_t mask){ //006FCB60.
        if(!buckets_.bound())return {}; //Native missing3EC returns before clearing caller scratch.
        const auto& grid=buckets_.grid_identity();
        const auto x=radius_range(centre[0],radius,grid.origin[0],grid.step[0],grid.count_x);
        const auto z=radius_range(centre[2],radius,grid.origin[1],grid.step[1],grid.count_z);
        const float square=static_cast<float>(static_cast<long double>(radius)*radius);
        std::vector<const Object*> result;
        for(auto iz=z[0];iz<z[1];++iz)for(auto ix=x[0];ix<x[1];++ix){
            const auto* bucket=buckets_.bucket(ix,iz);if(!bucket)continue;
            for(const auto* object:bucket->objects){
                if(!object)throw std::runtime_error("Null dynamic radius-query descriptor");
                if(object->mask&mask)continue;
                const auto dx=static_cast<long double>(object->position[0])-centre[0];
                const auto dz=static_cast<long double>(object->position[2])-centre[2];
                if(dx*dx+dz*dz<=square&&
                   std::find(result.begin(),result.end(),object)==result.end())result.push_back(object);
            }
        }
        return result;
    }
    const Object* containing(const Vector& point,std::uint32_t mask){ //006FB920; loaded grid REQUIRED.
        const auto& grid=buckets_.grid_identity();
        const auto x=point_range(point[0],grid.origin[0],grid.step[0],grid.count_x);
        const auto z=point_range(point[2],grid.origin[1],grid.step[1],grid.count_z);
        for(auto iz=z[0];iz<z[1];++iz)for(auto ix=x[0];ix<x[1];++ix){
            const auto* bucket=buckets_.bucket(ix,iz);if(!bucket)continue;
            for(const auto* object:bucket->objects){
                if(!object)throw std::runtime_error("Null dynamic point-query descriptor");
                if(object->mask&mask)continue;
                switch(object->flags&0xffu){
                case 0x40:{
                    if(!object->inverse)throw std::runtime_error("Dynamic point-query box has no actual inverse transform");
                    if(in_box(math_.car_transform(point,*object->inverse),*object))return object;
                    break;
                }
                case 0x41:if(in_box(point,*object))return object;break;
                case 0x42:case 0x44:{
                    const auto dx=static_cast<long double>(object->position[0])-point[0];
                    const auto dy=static_cast<long double>(object->position[1])-point[1];
                    const auto dz=static_cast<long double>(object->position[2])-point[2];
                    if((dz*dz+dx*dx)+dy*dy<=
                        static_cast<long double>(object->radius)*object->radius)return object;
                    break;
                }
                default:break; //Native no consumer for other primitive tags.
                }
            }
        }
        return nullptr;
    }
private:
    static bool in_box(const Vector& point,const Object& object){
        for(unsigned a=0;a!=3;++a)if(!(object.minimum[a]<point[a]&&point[a]<=object.maximum[a]))return false;
        return true; //Strict LOWER, inclusive UPPER.
    }
    static std::int32_t truncate(long double value){
        value=std::trunc(value);
        if(!std::isfinite(value)||value<-2147483648.L||value>2147483647.L)throw std::runtime_error("Dynamic point query exceeds native integer bounds");
        return static_cast<std::int32_t>(value);
    }
    static std::array<std::uint32_t,2> clamp(std::int32_t low,std::int32_t high,std::uint32_t count){
        if(count>0x7fffffffu)throw std::runtime_error("Dynamic point-query dimensions exceed native signed range");
        //Native clamps ONLY lower.begin and upper.end. Do not force queries
        //outside the grid onto an edge bucket or manufacture a hit.
        low=std::max(low,0);high=std::min(high,static_cast<std::int32_t>(count));
        if(low>=high)return {0,0};
        return {static_cast<std::uint32_t>(low),static_cast<std::uint32_t>(high)};
    }
    static std::array<std::uint32_t,2> radius_range(float coordinate,float radius,float origin,float step,std::uint32_t count){
        const auto inverse=1.L/step;
        return clamp(truncate((((static_cast<long double>(coordinate)-radius)-step)-origin)*inverse-1.f),
                     truncate((((static_cast<long double>(coordinate)+step)+radius)-origin)*inverse+2.f),count);
    }
    static std::array<std::uint32_t,2> point_range(float coordinate,float origin,float step,std::uint32_t count){
        const auto cell=(static_cast<long double>(coordinate)-origin)/step;
        return clamp(truncate(cell-1.f),truncate(cell+2.f),count);
    }
    const Buckets& buckets_;Math& math_;
};
} //namespace hd2::game
#endif
