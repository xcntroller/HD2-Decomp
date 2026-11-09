#ifndef HD2_GAME_STATIC_COLLISION_SWEEP_H
#define HD2_GAME_STATIC_COLLISION_SWEEP_H

#include "game/collision_sweep_box.h"
#include "game/collision_sweep_rectangle.h"
#include "game/actor_activation_math.h"
#include <cstring>

namespace hd2::game {

//007151D0's concrete triangle and required captured shape consumers from
//007101B0. Captured Tutorial/ALPS1 contain80/81/83; absent primitive families
//explicitly reject until recovered. Dynamic descriptors have their own receiver.
template<class Frame,class Math>
class StaticCollisionSweepNarrowPhase {
public:
    explicit StaticCollisionSweepNarrowPhase(Math& math):math_(math) {}
    bool sweep_triangle(const CollisionCellReference& identity,const CollisionTriangle<Frame>& triangle,
        const std::array<CollisionPoint,3>& vertices,CollisionSweepState& state) {
        return collision_sweep_triangle(identity,triangle,vertices,state);
    }
    bool sweep_shape(const CollisionCellReference& identity,const CollisionShape<Frame>& shape,CollisionSweepState& state) {
        switch(shape.flags&0xff) {
        case 0x81:
            if(collision_sweep_box(point(shape,8),point(shape,0x14),state)){state.hit_static=identity;state.hit_dynamic=nullptr;return true;}
            return false;
        case 0x80:case 0x83: {
            const bool box=(shape.flags&0xff)==0x80;
            const auto inverse=matrix(shape,box?0x80:0x60),world=matrix(shape,box?0x40:0x20);
            auto local=state;
            local.start=math_.car_transform(state.start,inverse);
            local.displacement=math_.collision_sweep_rotate(state.displacement,inverse);
            local.direction=normalize(local.displacement,box);
            const auto low=point(shape,box?0x20:8),high=point(shape,box?0x30:0x14);
            const bool hit=box?collision_sweep_box(low,high,local):collision_sweep_rectangle(low,high,local);
            if(!hit)return false;
            const auto position=math_.car_transform(local.hit_position,world);
            const auto normal=math_.collision_sweep_rotate(local.hit_normal,world);
            state.best_distance_squared=local.best_distance_squared;state.hit_position=position;state.hit_normal=normal;state.hit_static=identity;state.hit_dynamic=nullptr;return true;
        }
        default:throw std::runtime_error("Required static sweep primitive is not reconstructed");
        }
    }
private:
    static float real(const CollisionShape<Frame>& shape,unsigned offset) {
        if(offset<8||(offset&3))throw std::logic_error("Invalid static collision scalar offset");
        const auto word=shape.fields8.at((offset-8)/4);float value;std::memcpy(&value,&word,4);return value;
    }
    static CollisionPoint point(const CollisionShape<Frame>& shape,unsigned offset) {return {{real(shape,offset),real(shape,offset+4),real(shape,offset+8)}};}
    static typename Math::Matrix matrix(const CollisionShape<Frame>& shape,unsigned offset) {
        typename Math::Matrix result{};for(unsigned i=0;i!=16;++i)result[i]=real(shape,offset+i*4);return result;
    }
    static CollisionSweepState::Vector normalize(CollisionSweepState::Vector value,bool box) {
        const auto square=box?(static_cast<long double>(value[2])*value[2]+static_cast<long double>(value[1])*value[1])+static_cast<long double>(value[0])*value[0]:
            (static_cast<long double>(value[1])*value[1]+static_cast<long double>(value[0])*value[0])+static_cast<long double>(value[2])*value[2];
        constexpr float epsilon=9.99999993922529e-09f;
        if(!std::isfinite(square))throw std::runtime_error("Non-finite static sweep direction");
        if(std::abs(square-1)>=epsilon) {
            if(square>=epsilon) {const auto scale=1/std::sqrt(square);for(unsigned axis=0;axis!=3;++axis)value[axis]=static_cast<float>(value[axis]*scale);}
            else {const auto axis=value[0]!=0?0u:value[2]!=0?2u:1u;value[axis]=value[axis]<0?-1.f:1.f;}
        }
        return value;
    }
    Math& math_;
};
} //namespace hd2::game
#endif
