#ifndef HD2_GAME_COLLISION_WATER_QUERIES_H
#define HD2_GAME_COLLISION_WATER_QUERIES_H

#include "game/collision_ray.h"
#include "game/collision_vertical_limits.h"

namespace hd2::game {

//006FDA00 and the no-output006FCDA0 call reached by004D2C50. These borrow
//the SAME loaded tree/resources/math. Only triangles and83 are water hits;
//006FD7E0 genuinely returns false for every other shape family.
template<class Frame,class Resources,class Math>class CollisionWaterQueries {
public:
    using Tree=CollisionTree<Frame>;using Vector=CollisionRayState::Vector;using Matrix=std::array<float,16>;
    CollisionWaterQueries(const Tree& tree,Resources& resources,Math& math):tree_(tree),resources_(resources),math_(math){}
    auto bounds(){return collision_vertical_limits(&tree_,resources_,math_);} //SAME already-recovered006FDA00.
    bool segment(const Vector& start,const Vector& displacement,std::uint32_t excluded_flags=0){ //006FCDA0; no dynamic query.
        CollisionRayState state;state.start=start;state.displacement=displacement;state.excluded_flags=excluded_flags|0x20u;
        for(unsigned axis=0;axis!=3;++axis){
            const float end=start[axis]+displacement[axis];
            if(!std::isfinite(start[axis])||!std::isfinite(end))throw std::runtime_error("Non-finite water segment");
            state.minimum[axis]=std::min(start[axis],end);state.maximum[axis]=std::max(start[axis],end);
        }
        if(tree_.cell_fields8.size()!=tree_.cells.size())throw std::runtime_error("Water query cell Y limits are missing");
        const auto x=collision_sweep_cell_range(state.minimum[0],state.maximum[0],tree_.bounds_and_steps[0],tree_.bounds_and_steps[4],tree_.count_x);
        const auto z=collision_sweep_cell_range(state.minimum[2],state.maximum[2],tree_.bounds_and_steps[1],tree_.bounds_and_steps[5],tree_.count_z);
        const auto y=collision_sweep_cell_range(state.minimum[1],state.maximum[1],tree_.fields20[1],tree_.fields20[3],tree_.count_y);
        for(auto iz=z.begin;iz<z.end;++iz)for(auto iy=y.begin;iy<y.end;++iy)for(auto ix=x.begin;ix<x.end;++ix){
            const auto index=tree_.cell_index(ix,iy,iz);const auto limits=tree_.cell_fields8.at(index);
            if(state.minimum[1]<=limits[0]&&limits[1]<=state.maximum[1])cell(tree_.cells.at(index),state);
        }
        return state.hit_static.has_value();
    }
private:
    static float real(const CollisionShape<Frame>& shape,unsigned offset){const auto bits=shape.fields8.at((offset-8)/4);float value;std::memcpy(&value,&bits,4);return value;}
    static Matrix matrix(const CollisionShape<Frame>& shape,unsigned offset){Matrix result{};for(unsigned n=0;n!=16;++n)result[n]=real(shape,offset+n*4);return result;}
    void shape(const CollisionCellReference& id,const CollisionShape<Frame>& shape,CollisionRayState& state){ //006FD7E0.
        if(id.kind!=0x83)return;
        const auto inverse=matrix(shape,0x60);const auto local=math_.car_transform(state.start,inverse);
        const auto direction=math_.collision_sweep_rotate(state.displacement,inverse);
        if(!(std::abs(static_cast<long double>(direction[1]))>9.99999993922529e-09))return;
        const float parameter=static_cast<float>(-static_cast<long double>(local[1])/direction[1]);
        if(!(0<parameter&&parameter<state.parameter))return;
        const float x=static_cast<float>(static_cast<long double>(direction[0])*parameter+local[0]);
        const float z=static_cast<float>(static_cast<long double>(direction[2])*parameter+local[2]);
        if(!(real(shape,8)<=x&&x<=real(shape,0x14)&&real(shape,0x10)<=z&&z<=real(shape,0x1c)))return;
        state.parameter=parameter;state.hit_static=id;
    }
    void cell(const std::vector<CollisionCellReference>& entries,CollisionRayState& state){ //006FD340.
        for(const auto& id:entries){
            if(id.kind){const auto& primitive=tree_.shapes.at(id.kind-0x80).at(id.index);
                if((primitive.flags&0x10000u)&&!(primitive.flags&state.excluded_flags))shape(id,primitive,state);
                continue;
            }
            const auto& triangle=tree_.triangles.at(id.index);const auto flags=triangle.flags;
            if(!(flags&0x10000u)||(flags&state.excluded_flags))continue;
            if(flags&0xa0u)continue; //006FD7E0 sees a non83 first byte and returns false.
            const auto vertices=collision_triangle_vertices(tree_,triangle,resources_);
            if(vertices[2][0]<state.minimum[0])continue;
            if(state.maximum[0]<vertices[0][0])return; //Actual sorted-X early exit.
            const auto low=std::min({vertices[0][2],vertices[1][2],vertices[2][2]}),high=std::max({vertices[0][2],vertices[1][2],vertices[2][2]});
            if(state.maximum[2]<=low||high<state.minimum[2])continue;
            const auto& plane=triangle.plane;
            const float dot=static_cast<float>((static_cast<long double>(state.displacement[2])*plane[2]+static_cast<long double>(state.displacement[1])*plane[1])+static_cast<long double>(state.displacement[0])*plane[0]);
            if((flags&0x40000u)?std::abs(static_cast<long double>(dot))<9.99999993922529e-09:dot<9.99999993922529e-08f)continue;
            const auto extended=-(((static_cast<long double>(plane[2])*state.start[2]+static_cast<long double>(plane[1])*state.start[1])+static_cast<long double>(plane[0])*state.start[0])+plane[3])/dot;
            const float parameter=static_cast<float>(extended);
            if(extended<0||state.parameter<=parameter)continue;
            std::array<float,3> w{},a{},b{};
            for(unsigned axis=0;axis!=3;++axis){
                auto product=static_cast<long double>(parameter)*state.displacement[axis];
                if(axis==2)product=static_cast<float>(product);
                auto position=product+state.start[axis];if(axis==0)position=static_cast<float>(position);
                w[axis]=static_cast<float>(position-vertices[0][axis]);a[axis]=vertices[1][axis]-vertices[0][axis];b[axis]=vertices[2][axis]-vertices[0][axis];
            }
            const auto f=[](long double value){return static_cast<float>(value);};float u=0;long double q=0;
            switch(flags&0xffu){
            case 1:u=f(static_cast<long double>(w[1])/b[1]);q=(static_cast<long double>(w[2])-static_cast<long double>(b[2])*u)/a[2];break;
            case 2:u=f((static_cast<long double>(a[1])*w[2]-static_cast<long double>(a[2])*w[1])/(static_cast<long double>(a[1])*b[2]-static_cast<long double>(b[1])*a[2]));q=(static_cast<long double>(w[1])-static_cast<long double>(b[1])*u)/a[1];break;
            case 3:u=f(static_cast<long double>(w[0])/b[0]);q=(static_cast<long double>(w[2])-static_cast<long double>(b[2])*u)/a[2];break;
            case 4:u=f(static_cast<long double>(w[2])/b[2]);q=(static_cast<long double>(w[0])-static_cast<long double>(b[0])*u)/a[0];break;
            case 5:u=f((static_cast<long double>(a[0])*w[2]-static_cast<long double>(a[2])*w[0])/(static_cast<long double>(b[2])*a[0]-static_cast<long double>(a[2])*b[0]));q=(static_cast<long double>(w[0])-static_cast<long double>(b[0])*u)/a[0];break;
            case 6:u=f(static_cast<long double>(w[0])/b[0]);q=(static_cast<long double>(w[1])-static_cast<long double>(b[1])*u)/a[1];break;
            case 7:u=f((static_cast<long double>(a[0])*w[1]-static_cast<long double>(a[1])*w[0])/(static_cast<long double>(b[1])*a[0]-static_cast<long double>(a[1])*b[0]));q=(static_cast<long double>(w[0])-static_cast<long double>(b[0])*u)/a[0];break;
            default:continue;
            }
            if(!(0<=u&&u<=1&&0<=q&&u+q<=1))continue;
            state.parameter=parameter;state.hit_static=id;
        }
    }
    const Tree& tree_;Resources& resources_;Math& math_;
};

} //namespace hd2::game
#endif
