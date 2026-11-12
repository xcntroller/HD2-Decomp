#ifndef HD2_GAME_CAR_MESH_FRAGMENTS_H
#define HD2_GAME_CAR_MESH_FRAGMENTS_H
#include "game/physics_object_parameters.h"
#include <array>
#include <cmath>
#include <cstring>

namespace hd2::game {
struct CarFragmentVertex {std::array<float,4> position{},normal{};std::array<float,2> uv{};};
namespace car_fragment_detail {
using Vector=std::array<float,4>;
inline long double length(float square){
    std::int32_t bits;std::memcpy(&bits,&square,4);
    return bits<0x322bcc77?0.L:std::abs(static_cast<long double>(square)-1)<9.99999993922529e-9f?1.L:std::sqrt(static_cast<long double>(square));
}
inline long double distance(const Vector& a,const Vector& b){
    const auto x=static_cast<long double>(a[0])-b[0],y=static_cast<long double>(a[1])-b[1],z=static_cast<long double>(a[2])-b[2];
    return length(static_cast<float>((x*x+y*y)+z*z));
}
inline Vector subtract(const Vector& a,const Vector& b){Vector r{};for(unsigned n=0;n!=3;++n)r[n]=a[n]-b[n];return r;}
inline Vector normalize(Vector value,long double square,long double first){
    constexpr float epsilon=9.99999993922529e-9f;
    if(!std::isfinite(square))throw std::runtime_error("Non-finite fragment direction");
    if(std::abs(square-1)>=epsilon){
        const auto stored=static_cast<float>(square);
        if(stored>=epsilon){const auto scale=1/std::sqrt(static_cast<long double>(stored));first*=scale;for(unsigned a=1;a!=3;++a)value[a]=static_cast<float>(value[a]*scale);}
        else if(first!=0)first=first<0?-1.f:1.f;
        else{const auto a=value[2]!=0?2u:1u;value[a]=value[a]<0?-1.f:1.f;}
    }
    value[0]=static_cast<float>(first);
    return value;
}
}

//00484F60, FASTCALL+RET40. Recurse in native apex/midpoint/edge order, keeping
//positions and UVs together. The builder holds visual/mesh/LOD factory references
//until AFTER real Physics create and optional six ENGINE-random draws/impulse.
template<class Frames,class Physics>void emit_car_triangle_fragments(Frames& frames,Physics& physics,
        const std::array<CarFragmentVertex,3>& input,typename Frames::Frame source,
        typename Frames::Frame material,float maximum_area,unsigned depth=0){
    using namespace car_fragment_detail;
    if(!std::isfinite(maximum_area)||maximum_area<=0||depth>512)throw std::runtime_error("Invalid fragment subdivision boundary");
    const auto ab=static_cast<float>(distance(input[0].position,input[1].position));
    const auto bc=static_cast<float>(distance(input[1].position,input[2].position));
    const auto ca=distance(input[2].position,input[0].position);
    unsigned apex,first,last;
    if(ab>=bc&&ab>=ca){apex=2;first=0;last=1;}
    else if(ab<=bc&&ca<=bc){apex=0;first=1;last=2;}
    else{apex=1;first=0;last=2;}
    const auto& a=input[apex];const auto& b=input[first];const auto& c=input[last];
    auto edge=subtract(b.position,c.position);
    const auto x=static_cast<long double>(b.position[0])-c.position[0];
    edge=normalize(edge,(x*x+static_cast<long double>(edge[1])*edge[1])+static_cast<long double>(edge[2])*edge[2],x);
    const float square=static_cast<float>((static_cast<long double>(edge[0])*edge[0]+static_cast<long double>(edge[2])*edge[2])+static_cast<long double>(edge[1])*edge[1]);
    float height=-1.e16f; //Native DA0E1BCA sentinel when no usable direction.
    if(square>=9.99999993922529e-9f){
        const auto offset=subtract(b.position,a.position);
        const auto dot=(static_cast<long double>(edge[2])*offset[2]+static_cast<long double>(edge[1])*offset[1])+static_cast<long double>(edge[0])*offset[0];
        const float projection=static_cast<float>(dot/square);Vector perpendicular{};
        for(unsigned n=0;n!=3;++n){const float along=edge[n]*projection;perpendicular[n]=offset[n]-along;}
        height=static_cast<float>(length(static_cast<float>((static_cast<long double>(perpendicular[0])*perpendicular[0]+
            static_cast<long double>(perpendicular[1])*perpendicular[1])+static_cast<long double>(perpendicular[2])*perpendicular[2])));
    }
    if(distance(b.position,c.position)*height*.5f>maximum_area){
        CarFragmentVertex middle;
        middle.position[0]=static_cast<float>((static_cast<long double>(c.position[0])+b.position[0])*.5f);
        middle.position[1]=static_cast<float>((static_cast<long double>(c.position[1])+b.position[1])*.5f);
        const float z=c.position[2]+b.position[2];middle.position[2]=z*.5f;
        for(unsigned n=0;n!=2;++n)middle.uv[n]=static_cast<float>((static_cast<long double>(c.uv[n])+b.uv[n])*.5f);
        emit_car_triangle_fragments(frames,physics,{{a,middle,b}},source,material,maximum_area,depth+1);
        emit_car_triangle_fragments(frames,physics,{{a,middle,c}},source,material,maximum_area,depth+1);return;
    }
    auto one=subtract(input[2].position,input[1].position),two=subtract(input[0].position,input[1].position);
    for(unsigned n=0;n!=3;++n){one[n]*=10000.f;two[n]*=10000.f;}
    Vector normal{};long double normal_x=0,normal_z=0;
    for(unsigned n=0;n!=3;++n){const auto next=(n+1)%3,last_axis=(n+2)%3;
        const auto component=static_cast<long double>(one[next])*two[last_axis]-static_cast<long double>(two[next])*one[last_axis];
        if(n==0)normal_x=component;else if(n==2)normal_z=component;normal[n]=static_cast<float>(component);}
    normal=normalize(normal,(normal_z*normal[2]+static_cast<long double>(normal[1])*normal[1])+normal_x*normal_x,normal_x);
    std::array<CarFragmentVertex,6> vertices{{input[0],input[1],input[2],input[2],input[1],input[0]}};
    for(auto& vertex:vertices)vertex.normal=normal; //Both faces retain SAME native normal.
    auto fragment=frames.create_car_fragment(source,vertices,material);
    PhysicsObjectParameters<typename Frames::Frame> parameters;parameters.frame=fragment.visual.get();parameters.type=4;
    parameters.first_list=true;parameters.sight_pass=true; //Native material10 is ZERO, distinct from mesh material.
    if(auto* handle=physics.create(parameters)){
        const float z=static_cast<float>(static_cast<long double>(frames.volume_random_unit())-.5f);
        const float y=static_cast<float>(static_cast<long double>(frames.volume_random_unit())-.5f);
        const float x_random=static_cast<float>(static_cast<long double>(frames.volume_random_unit())-.5f);
        const auto world=frames.actor_world_matrix(fragment.visual.get());
        Vector point{{world[12]+x_random,world[13]+y,world[14]+z,0}},force{};
        for(unsigned n=0;n!=3;++n)force[n]=static_cast<float>((static_cast<long double>(frames.volume_random_unit())-.5f)*10.f);
        physics.impulse(handle,force,point,.10000000149011612f);
    }
    fragment.retire(); //Native LOD, mesh, visual order; do not retire Physics handle.
}

//00485BA0 (absent frozen). Borrow current LOD0/group0 and process faces reverse,
//re-reading each face-index pointer before its recursive triangle worker.
template<class Frames,class Physics>void emit_car_mesh_fragments(Frames& frames,Physics& physics,
        typename Frames::Frame frame,float maximum_area){
    frames.car_fragment_triangles(frame,[&](const std::array<CarFragmentVertex,3>& vertices,auto material){
        emit_car_triangle_fragments(frames,physics,vertices,frame,material,maximum_area);
    });
}
}
#endif
