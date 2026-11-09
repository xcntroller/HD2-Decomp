#ifndef HD2_GAME_COLLISION_SWEEP_TRIANGLE_H
#define HD2_GAME_COLLISION_SWEEP_TRIANGLE_H

#include "game/collision_sweep_candidates.h"

namespace hd2::game {
namespace collision_sweep_math {
using Vector=CollisionSweepState::Vector;
inline Vector subtract(const Vector& a,const Vector& b) {return {{a[0]-b[0],a[1]-b[1],a[2]-b[2],0}};}
inline Vector add(const Vector& a,const Vector& b) {return {{a[0]+b[0],a[1]+b[1],a[2]+b[2],0}};}
inline Vector scale(const Vector& a,float s) {return {{static_cast<float>(static_cast<long double>(a[0])*s),static_cast<float>(static_cast<long double>(a[1])*s),static_cast<float>(static_cast<long double>(a[2])*s),0}};}
inline Vector cross(const Vector& a,const Vector& b) {
    return {{static_cast<float>(static_cast<long double>(a[1])*b[2]-static_cast<long double>(a[2])*b[1]),
        static_cast<float>(static_cast<long double>(a[2])*b[0]-static_cast<long double>(a[0])*b[2]),
        static_cast<float>(static_cast<long double>(a[0])*b[1]-static_cast<long double>(a[1])*b[0]),0}};
}
inline float dot(const Vector& a,const Vector& b) {return static_cast<float>((static_cast<long double>(a[0])*b[0]+static_cast<long double>(a[1])*b[1])+static_cast<long double>(a[2])*b[2]);}
}

//007041D0. Swept sphere against one triangle edge, including native vertex
//fallback marks and the negative-distance overlap result(best=-1). Caller owns
//the primitive identity and tries its original ordered edges/vertices.
inline bool collision_sweep_triangle_edge(const CollisionSweepState::Vector& a,
    const CollisionSweepState::Vector& b,CollisionSweepState& state,
    std::uint32_t& vertex,std::uint32_t first_vertex,std::uint32_t last_vertex) {
    using namespace collision_sweep_math;
    const auto w=subtract(a,state.start),edge=subtract(b,a),perpendicular=cross(edge,state.direction);
    const auto cross_square=dot(perpendicular,perpendicular);
    if(!(1.e-8<std::abs(static_cast<long double>(cross_square))))return false;
    const auto triple=dot(edge,cross(state.direction,w));
    const auto edge_square=dot(edge,edge);
    const float discriminant=static_cast<float>((static_cast<long double>(cross_square)*state.radius*state.radius-static_cast<long double>(triple)*triple)*edge_square);
    if(discriminant<0)return false;
    const auto numerator=dot(perpendicular,cross(edge,w));
    const auto root=std::sqrt(static_cast<long double>(discriminant));
    if(numerator<-root)return false;
    const float distance=static_cast<float>((static_cast<long double>(numerator)-root)/cross_square);
    if(distance<-state.radius)return false;
    const auto along=dot(edge,w);
    const float position=static_cast<float>((static_cast<long double>(dot(edge,state.direction))*distance-along)/edge_square);
    if(position<0){vertex=first_vertex;return false;}
    if(position>1){vertex=last_vertex;return false;}
    const auto closest=add(a,scale(edge,position));
    const auto centre=add(state.start,scale(state.direction,distance));
    const auto normal=subtract(closest,centre);
    if(distance>=0) {
        const float square=distance*distance;
        if(!(square<state.best_distance_squared))return false;
        state.best_distance_squared=square;state.hit_position=centre;state.hit_normal=normal;return true;
    }
    if(along>0){vertex=first_vertex;return false;}
    if(dot(subtract(b,state.start),edge)<0){vertex=last_vertex;return false;}
    if(-dot(normal,state.direction)<=distance) {
        state.best_distance_squared=-1;state.hit_position=centre;state.hit_normal=normal;return true;
    }
    return false;
}

//00704D60, with007041D0's ordered edge and vertex fallbacks. Normals retain
//the original direction;00715510 negates/normalizes them at its output boundary.
template<class Frame>bool collision_sweep_triangle(const CollisionCellReference& identity,
    const CollisionTriangle<Frame>& triangle,const std::array<CollisionPoint,3>& points,
    CollisionSweepState& state) {
    using namespace collision_sweep_math;
    std::array<Vector,3> vertices{};
    for(unsigned i=0;i!=3;++i)for(unsigned axis=0;axis!=3;++axis)vertices[i][axis]=points[i][axis];
    Vector normal{{triangle.plane[0],triangle.plane[1],triangle.plane[2],0}};
    std::uint32_t mask=(triangle.flags&0x100)?7:0;
    const auto facing=dot(normal,state.direction);
    if(!(triangle.flags&0x40000)&&facing<0)return false;
    if((triangle.flags&0x40000)&&facing<0){normal=scale(normal,-1);mask^=7;}
    const auto offset_plane=-dot(normal,subtract(vertices[0],scale(normal,state.radius)));
    auto distance=dot(Vector{{triangle.plane[0],triangle.plane[1],triangle.plane[2],0}},state.start)+triangle.plane[3];
    if(facing<0)distance=-distance;
    if(distance>0)return false;
    auto publish=[&](const Vector& position,const Vector& hit_normal,float square) {
        state.hit_position=position;state.hit_normal=hit_normal;state.best_distance_squared=square;state.hit_static=identity;state.hit_dynamic=nullptr;return true;
    };
    if(-state.radius<distance) {
        const auto moment=cross(normal,state.start);
        std::array<float,3> sides{};
        for(unsigned edge=0;edge!=3;++edge) {
            const auto& a=vertices[edge];const auto& b=vertices[(edge+1)%3];
            sides[edge]=dot(subtract(a,b),moment)+dot(cross(a,b),normal);
        }
        if((mask==0&&sides[0]<0&&sides[1]<0&&sides[2]<0)||
           (mask==7&&sides[0]>=0&&sides[1]>=0&&sides[2]>=0))
            return publish(add(state.start,scale(normal,-distance-state.radius)),normal,0);
    }
    const auto moment=cross(state.displacement,add(state.start,scale(normal,state.radius)));
    for(unsigned edge=0;edge!=3;++edge) {
        const auto& a=vertices[edge];const auto& b=vertices[(edge+1)%3];
        if(dot(subtract(a,b),moment)+dot(cross(a,b),state.displacement)>0)mask^=1u<<edge;
    }
    const auto denominator=dot(normal,state.displacement);
    if(std::abs(static_cast<long double>(denominator))<1.e-8)return false;
    const float parameter=static_cast<float>(-(static_cast<long double>(dot(normal,state.start))+offset_plane)/denominator);
    const auto centre=add(state.start,scale(state.displacement,parameter));
    if(!mask) {
        if(parameter<0)return false;
        const auto travel=subtract(centre,state.start);const auto square=dot(travel,travel);
        return square<state.best_distance_squared?publish(centre,normal,square):false;
    }
    std::uint32_t vertex=0;
    if((mask&4)&&collision_sweep_triangle_edge(vertices[2],vertices[0],state,vertex,1,2)){state.hit_static=identity;state.hit_dynamic=nullptr;return true;}
    if((mask&2)&&collision_sweep_triangle_edge(vertices[1],vertices[2],state,vertex,4,1)){state.hit_static=identity;state.hit_dynamic=nullptr;return true;}
    if((mask&1)&&collision_sweep_triangle_edge(vertices[0],vertices[1],state,vertex,2,4)){state.hit_static=identity;state.hit_dynamic=nullptr;return true;}
    const auto index=vertex==1?2:vertex==2?0:vertex==4?1:-1;
    if(index<0)return false;
    const auto delta=subtract(vertices[std::size_t(index)],state.start);
    const auto projection=dot(delta,state.direction);
    const auto discriminant=static_cast<long double>(state.radius)*state.radius-
        (static_cast<long double>(dot(delta,delta))-static_cast<long double>(projection)*projection);
    if(discriminant<0)return false;
    const auto extended=static_cast<long double>(projection)-std::sqrt(discriminant);
    const float travel=static_cast<float>(extended);
    const float square=static_cast<float>((index==2?static_cast<long double>(travel):extended)*travel);
    if(!(square<state.best_distance_squared))return false;
    const auto position=add(state.start,scale(state.direction,travel));
    return publish(position,subtract(vertices[std::size_t(index)],position),square);
}
} //namespace hd2::game
#endif
