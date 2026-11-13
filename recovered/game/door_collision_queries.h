#ifndef HD2_GAME_DOOR_COLLISION_QUERIES_H
#define HD2_GAME_DOOR_COLLISION_QUERIES_H

#include "game/collision_ray.h"

namespace hd2::game {

//006F78F0/006F3250's dynamic-only segment query, used by Door edge probes.
//Unlike006F7C90's camera ray there is no static tree traversal, vertical
//dispatch or length+radius reach test. Keep original Z/X/bucket order and the
//full initial XZ bounds even when a nearer hit shortens the best parameter.
//Expose the winning descriptor for0054F910 and the existing Door boolean query;
//raw output-vector/W ABI is not reconstructed.
template<class Frame,class Object,class Math>class DoorCollisionQueries {
public:
    using Tree=CollisionTree<Frame>;using Vector=CollisionRayState::Vector;
    using Buckets=DynamicCollisionBuckets<Object,typename Tree::DynamicGrid>;
    DoorCollisionQueries(const Tree& tree,const Buckets& buckets,Math& math):tree_(tree),buckets_(buckets),narrow_(math){}
    bool blocked(const Vector& start,const Vector& displacement,std::uint32_t layers=2,std::uint32_t flags=0) {
        return nearest(start,displacement,layers,flags)!=nullptr;
    }
    const Object* nearest(const Vector& start,const Vector& displacement,std::uint32_t layers=2,std::uint32_t flags=0) {
        using namespace collision_ray_math;
        CollisionRayState state;state.start=start;state.displacement=displacement;
        state.excluded_layers=layers|8u;state.excluded_flags=flags|0x20u;
        state.square=stored((static_cast<long double>(displacement[2])*displacement[2]+static_cast<long double>(displacement[0])*displacement[0])+static_cast<long double>(displacement[1])*displacement[1]);
        if(!std::isfinite(state.square))throw std::runtime_error("Non-finite Door ray displacement");
        state.length=stored(std::sqrt(static_cast<long double>(state.square)));bounds(state);
        const auto& grid=tree_.dynamic;
        const auto x=cell_range(state.minimum[0],state.maximum[0],grid.origin[0],grid.step[0],grid.count_x);
        const auto z=cell_range(state.minimum[2],state.maximum[2],grid.origin[1],grid.step[1],grid.count_z);
        bool hit=false;
        for(auto iz=z.begin;iz<z.end;++iz)for(auto ix=x.begin;ix<x.end;++ix)
            if(const auto* bucket=buckets_.bucket(static_cast<std::uint32_t>(ix),static_cast<std::uint32_t>(iz)))for(const auto* object:bucket->objects) {
                if(!object)throw std::runtime_error("Door ray encountered a null dynamic descriptor");
                if((state.excluded_layers&object->mask)||(state.excluded_flags&object->flags))continue;
                Vector delta{};for(unsigned a=0;a!=3;++a)delta[a]=stored(static_cast<long double>(start[a])-object->position[a]);
                const auto square=(static_cast<long double>(displacement[0])*displacement[0]+static_cast<long double>(displacement[1])*displacement[1])+static_cast<long double>(displacement[2])*displacement[2];
                const float denominator=stored(square);
                long double distance=-10000000272564224.f; //00810BA8, zero-length line branch.
                if(square>=9.99999993922529e-09f) {
                    const auto projection=((static_cast<long double>(delta[0])*displacement[0]+static_cast<long double>(delta[2])*displacement[2])+static_cast<long double>(delta[1])*displacement[1])/denominator;
                    Vector perpendicular{};for(unsigned a=0;a!=3;++a)perpendicular[a]=stored(static_cast<long double>(delta[a])-stored(static_cast<long double>(projection)*displacement[a]));
                    const float length_square=stored((static_cast<long double>(perpendicular[0])*perpendicular[0]+static_cast<long double>(perpendicular[1])*perpendicular[1])+static_cast<long double>(perpendicular[2])*perpendicular[2]);
                    distance=length_square<9.99999993922529e-09f?0.L:std::abs(static_cast<long double>(length_square)-1.f)<9.99999993922529e-09f?1.L:std::sqrt(static_cast<long double>(length_square));
                }
                if(!(distance<=object->radius))continue;
                if(!(state.minimum[0]<=static_cast<long double>(object->position[0])+object->radius&&static_cast<long double>(object->position[0])-object->radius<=state.maximum[0]&&
                     state.minimum[2]<=static_cast<long double>(object->position[2])+object->radius&&static_cast<long double>(object->position[2])-object->radius<=state.maximum[2]))continue;
                //Original unknown primitive tags skip. Known49/4A require their
                //actual additional record layout/receiver rather than a miss.
                const auto kind=object->flags&0xffu;
                if(kind==0x49||kind==0x4a)throw std::runtime_error("Door ray requires its actual49/4A primitive receiver");
                if(kind!=0x40&&kind!=0x41&&kind!=0x42&&kind!=0x44)continue;
                if(kind==0x42||kind==0x44)hit=sphere(*object,state)||hit;
                else hit=narrow_.primitive(*object,state)||hit;
            }
        return hit?static_cast<const Object*>(state.hit_dynamic):nullptr;
    }
private:
    static CollisionSweepCellRange cell_range(float low,float high,float origin,float step,std::uint32_t count) {
        if(!std::isfinite(step)||step<=0||count>std::uint32_t(std::numeric_limits<std::int32_t>::max()))throw std::runtime_error("Invalid Door dynamic grid");
        const auto reciprocal=1.L/step;
        const auto truncate=[](long double value){
            if(!std::isfinite(value)||value<std::numeric_limits<std::int32_t>::min()||value>std::numeric_limits<std::int32_t>::max())throw std::runtime_error("Door ray cell exceeds native integer bounds");
            return static_cast<std::int32_t>(value);
        };
        //006F3250 expands one cell on BOTH sides; the high end is exclusive.
        return {std::max(0,truncate((static_cast<long double>(low)-origin)*reciprocal-1.f)),
                std::min(static_cast<std::int32_t>(count),truncate((static_cast<long double>(high)-origin)*reciprocal+2.f))};
    }
    static bool sphere(const Object& object,CollisionRayState& state) {
        std::array<long double,3> delta{};for(unsigned a=0;a!=3;++a)delta[a]=static_cast<long double>(object.position[a])-state.start[a];
        const float radius=static_cast<float>(static_cast<long double>(object.radius)*object.radius);
        const auto square=(delta[2]*delta[2]+delta[1]*delta[1])+delta[0]*delta[0];const float stored_square=static_cast<float>(square);
        if(square<radius){state.parameter=0;state.hit_dynamic=&object;return true;} //Includes an inside zero-length segment.
        if(state.square==0)return false;
        const auto projection=(delta[2]*state.displacement[2]+delta[1]*state.displacement[1])+delta[0]*state.displacement[0];
        const auto discriminant=projection*projection-(static_cast<long double>(stored_square)-radius)*state.square;
        if(discriminant<0)return false;
        const auto parameter=(projection-std::sqrt(discriminant))/state.square;
        if(parameter<0||!(parameter<state.parameter))return false;
        state.parameter=static_cast<float>(parameter);state.hit_dynamic=&object;return true;
    }
    const Tree& tree_;const Buckets& buckets_;DynamicCollisionRayNarrowPhase<Object,Math> narrow_;
};
} //namespace hd2::game
#endif
