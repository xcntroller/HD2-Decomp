#ifndef HD2_GAME_COLLISION_RAY_H
#define HD2_GAME_COLLISION_RAY_H

#include "game/collision_sweep.h"
#include <cstring>

namespace hd2::game {

//006F7C90/006F5890 segment ray, distinct from the sphere sweep. Identities
//borrow the SAME tree records/descriptors; endpoint parameter starts at1.
struct CollisionRayState {
    using Vector=CollisionSweepState::Vector;
    Vector start{},displacement{},normal{};
    std::array<float,3> minimum{},maximum{};
    float parameter=1,square=0,length=0;
    std::uint32_t excluded_flags=0,excluded_layers=0;
    std::optional<CollisionCellReference> hit_static;
    const void* hit_dynamic=nullptr;
};
namespace collision_ray_math {
using Vector=CollisionRayState::Vector;
inline float stored(long double value){return static_cast<float>(value);}
inline long double square(const Vector& v) {
    return (static_cast<long double>(v[0])*v[0]+static_cast<long double>(v[1])*v[1])+static_cast<long double>(v[2])*v[2];
}
inline void bounds(CollisionRayState& s) {
    for(unsigned a=0;a!=3;++a) {
        const float end=stored(static_cast<long double>(s.displacement[a])*s.parameter+s.start[a]);
        s.minimum[a]=std::min(s.start[a],end);s.maximum[a]=std::max(s.start[a],end);
    }
}
//006F4310. X/Z use signed epsilon intervals; Y has its own parallel test.
//An inside start can produce parameter0/Y normal. Equal entry times select Y,
//then Z, matching the native comparisons rather than a generic slab routine.
inline bool box(const CollisionPoint& low,const CollisionPoint& high,CollisionRayState& s) {
    constexpr float epsilon=9.99999993922529e-09f;
    std::array<float,3> enter{},exit{};
    for(unsigned a:{0u,2u,1u}) {
        const auto d=s.displacement[a];const bool parallel=a==1?std::abs(d)<=epsilon:(-epsilon<=d&&d<=epsilon);
        if(parallel) {
            if(s.start[a]<low[a]||high[a]<s.start[a])return false;
            enter[a]=0;exit[a]=std::numeric_limits<float>::max();
        }else {
            const auto inverse=1/static_cast<long double>(d);
            enter[a]=stored(((d>0?low[a]:high[a])-static_cast<long double>(s.start[a]))*inverse);
            if(s.parameter<enter[a])return false;
            exit[a]=stored(((d>0?high[a]:low[a])-static_cast<long double>(s.start[a]))*inverse);
        }
    }
    if(exit[0]<enter[2]||exit[2]<enter[0])return false;
    unsigned axis=1;
    if(enter[0]<=enter[2]) {
        if(enter[1]<enter[2]){if(exit[1]<enter[2]||enter[2]<0)return false;axis=2;}
    }else if(enter[1]<enter[0]){if(exit[1]<enter[0]||enter[0]<0)return false;axis=0;}
    if(axis==1&&(exit[0]<enter[1]||exit[2]<enter[1]||enter[1]<0))return false;
    s.parameter=enter[axis];s.normal={{0,0,0,0}};s.normal[axis]=s.displacement[axis]<=0?-1.f:1.f;return true;
}
inline bool sphere(const Vector& center,float radius,CollisionRayState& s) { //006F4D00(82),006F2C50(42/44).
    if(s.square==0)return false;
    std::array<long double,3> delta{};for(unsigned a=0;a!=3;++a)delta[a]=static_cast<long double>(center[a])-s.start[a];
    const auto projection=(delta[0]*s.displacement[0]+delta[1]*s.displacement[1])+delta[2]*s.displacement[2];
    const auto discriminant=projection*projection-((delta[0]*delta[0]+delta[1]*delta[1])+delta[2]*delta[2]-static_cast<long double>(radius)*radius)*s.square;
    if(discriminant<0)return false;
    const float t=stored((projection-std::sqrt(discriminant))/s.square);
    if(t<0||!(t<s.parameter))return false;
    s.parameter=t;for(unsigned a=0;a!=3;++a)s.normal[a]=stored(delta[a]-static_cast<long double>(t)*s.displacement[a]);return true;
}
} //namespace collision_ray_math

//006F53F0's sorted-cell triangle consumer. Low-byte classification1..7 is
//produced by006EE380. Backfaces use positive dot; bit40000 enables both sides.
template<class Frame,class Resources,class Shapes>
bool filter_collision_ray_cell(const CollisionTree<Frame>& tree,const std::vector<CollisionCellReference>& cell,
                               CollisionRayState& s,Resources& resources,Shapes& shapes) {
    using namespace collision_ray_math;
    bool hit=false;
    for(const auto& id:cell) {
        if(id.kind) {
            const auto& shape=tree.shapes.at(id.kind-0x80).at(id.index);
            if(!(shape.flags&s.excluded_flags))hit=shapes.ray_shape(id,shape,s)||hit;
            continue;
        }
        const auto& t=tree.triangles.at(id.index);if(t.flags&s.excluded_flags)continue;
        if(t.flags&0xa0)throw std::runtime_error("Ray triangle has invalid primitive flags");
        const auto v=collision_triangle_vertices(tree,t,resources);
        if(v[2][0]<s.minimum[0])continue;
        if(s.maximum[0]<v[0][0])return hit;
        const auto low=std::min({v[0][2],v[1][2],v[2][2]}),high=std::max({v[0][2],v[1][2],v[2][2]});
        if(s.maximum[2]<=low||high<s.minimum[2])continue;
        const auto& p=t.plane;
        const auto dot=(static_cast<long double>(s.displacement[0])*p[0]+static_cast<long double>(s.displacement[1])*p[1])+static_cast<long double>(s.displacement[2])*p[2];
        constexpr float epsilon=9.99999993922529e-09f;
        if((t.flags&0x40000u)?std::abs(dot)<epsilon:dot<epsilon)continue;
        const float parameter=stored(-(((static_cast<long double>(p[0])*s.start[0]+static_cast<long double>(p[1])*s.start[1])+static_cast<long double>(p[2])*s.start[2])+p[3])/dot);
        if(parameter<0||s.parameter<=parameter)continue;
        std::array<long double,3> w{},a{},b{};
        for(unsigned axis=0;axis!=3;++axis) {
            w[axis]=(static_cast<long double>(parameter)*s.displacement[axis]+s.start[axis])-v[0][axis];
            a[axis]=static_cast<long double>(v[1][axis])-v[0][axis];b[axis]=static_cast<long double>(v[2][axis])-v[0][axis];
        }
        float u=0;long double q=0;
        switch(t.flags&0xffu) {
        case 1:u=stored(w[1]/b[1]);q=(w[2]-b[2]*u)/a[2];break;
        case 2:u=stored((a[1]*w[2]-a[2]*w[1])/(a[1]*b[2]-b[1]*a[2]));q=(w[1]-b[1]*u)/a[1];break;
        case 3:u=stored(w[0]/b[0]);q=(w[2]-b[2]*u)/a[2];break;
        case 4:u=stored(w[2]/b[2]);q=(w[0]-b[0]*u)/a[0];break;
        case 5:u=stored((a[0]*w[2]-a[2]*w[0])/(b[2]*a[0]-a[2]*b[0]));q=(w[0]-b[0]*u)/a[0];break;
        case 6:u=stored(w[0]/b[0]);q=(w[1]-b[1]*u)/a[1];break;
        case 7:u=stored((a[0]*w[1]-a[1]*w[0])/(b[1]*a[0]-a[1]*b[0]));q=(w[0]-b[0]*u)/a[0];break;
        default:continue;
        }
        if(!(0<=u&&u<=1&&0<=q&&u+q<=1))continue;
        s.parameter=parameter;s.normal={{p[0],p[1],p[2],0}};s.hit_static=id;s.hit_dynamic=nullptr;hit=true;
    }
    return hit;
}

//Required captured80/81/83 static receivers of006F4D00. Wider shape families
//remain explicit failures until their original producers/receivers are present.
template<class Frame,class Math>class StaticCollisionRayNarrowPhase {
public:
    explicit StaticCollisionRayNarrowPhase(Math& math):math_(math){}
    bool ray_shape(const CollisionCellReference& id,const CollisionShape<Frame>& shape,CollisionRayState& s) {
        using namespace collision_ray_math;
        const auto kind=shape.flags&0xffu;
        if(kind==0x81) {
            if(!box(point(shape,8),point(shape,0x14),s))return false;
        }else if(kind==0x80||kind==0x83) {
            const bool is_box=kind==0x80;
            const auto inverse=matrix(shape,is_box?0x80:0x60),world=matrix(shape,is_box?0x40:0x20);
            auto local=s;local.start=math_.car_transform(s.start,inverse);local.displacement=math_.collision_sweep_rotate(s.displacement,inverse);
            if(is_box) {if(!box(point(shape,0x20),point(shape,0x30),local))return false;}
            else {
                if(std::abs(local.displacement[1])<=9.99999993922529e-09f)return false;
                const float t=stored(-static_cast<long double>(local.start[1])/local.displacement[1]);
                if(s.parameter<=t||t<=0)return false;
                const float x=stored(static_cast<long double>(local.displacement[0])*t+local.start[0]);
                const float z=stored(static_cast<long double>(local.displacement[2])*t+local.start[2]);
                if(x<real(shape,8)||real(shape,0x14)<=x||z<real(shape,0x10)||real(shape,0x1c)<=z)return false;
                local.parameter=t;local.normal={{0,local.displacement[1]>0?1.f:-1.f,0,0}};
            }
            s.parameter=local.parameter;s.normal=math_.collision_sweep_rotate(local.normal,world);
        }else if(kind==0x82) {
            const auto center=point(shape,0x10);
            if(!sphere({{center[0],center[1],center[2],0}},real(shape,8),s))return false;
        }else throw std::runtime_error("Required static ray primitive is not reconstructed");
        s.hit_static=id;s.hit_dynamic=nullptr;return true;
    }
private:
    static float real(const CollisionShape<Frame>& s,unsigned offset){const auto bits=s.fields8.at((offset-8)/4);float v;std::memcpy(&v,&bits,4);return v;}
    static CollisionPoint point(const CollisionShape<Frame>& s,unsigned o){return {{real(s,o),real(s,o+4),real(s,o+8)}};}
    static typename Math::Matrix matrix(const CollisionShape<Frame>& s,unsigned o){typename Math::Matrix m{};for(unsigned n=0;n!=16;++n)m[n]=real(s,o+n*4);return m;}
    Math& math_;
};

//006F2C50: original Z/X buckets and line/sphere broad phase, before static
//traversal. Repeated descriptors retain original order; no invented dedup set.
template<class Object,class Math>class DynamicCollisionRayNarrowPhase {
public:
    explicit DynamicCollisionRayNarrowPhase(Math& math):math_(math){}
    bool ray(const Object& object,CollisionRayState& s) {
        using namespace collision_ray_math;
        Vector delta{};for(unsigned a=0;a!=3;++a)delta[a]=stored(static_cast<long double>(s.start[a])-object.position[a]);
        const auto line_square=(static_cast<long double>(s.displacement[2])*s.displacement[2]+static_cast<long double>(s.displacement[1])*s.displacement[1])+static_cast<long double>(s.displacement[0])*s.displacement[0];
        long double line_distance=-10000000272564224.f; //00810BA8.
        if(line_square>=9.99999993922529e-09f) {
            const float projection=stored(((static_cast<long double>(delta[0])*s.displacement[0]+static_cast<long double>(delta[1])*s.displacement[1])+static_cast<long double>(delta[2])*s.displacement[2])/line_square);
            Vector perpendicular{};for(unsigned a=0;a!=3;++a)perpendicular[a]=stored(static_cast<long double>(delta[a])-stored(static_cast<long double>(projection)*s.displacement[a]));
            line_distance=std::sqrt(square(perpendicular));
        }
        if(!(line_distance<=object.radius))return false;
        const auto projection=-((static_cast<long double>(delta[0])*s.displacement[0]+static_cast<long double>(delta[1])*s.displacement[1])+static_cast<long double>(delta[2])*s.displacement[2]);
        const auto reach=projection<=0?static_cast<long double>(object.radius):static_cast<long double>(s.length)+object.radius;
        if(!(square(delta)<=reach*reach))return false;
        return primitive(object,s);
    }
    //Shared actual primitive receivers.006F3250 has its own broad phase;
    //callers must not silently substitute006F2C50's reach test above.
    bool primitive(const Object& object,CollisionRayState& s) {
        using namespace collision_ray_math;
        switch(object.flags&0xffu) {
        case 0x40: {
            if(!object.matrix||!object.inverse)throw std::runtime_error("Dynamic ray box has no transforms");
            auto local=s;local.start=math_.car_transform(s.start,*object.inverse);local.displacement=math_.collision_sweep_rotate(s.displacement,*object.inverse);
            if(!box(point(object.minimum),point(object.maximum),local))return false;
            s.parameter=local.parameter;s.normal=math_.collision_sweep_rotate(local.normal,*object.matrix);break;
        }
        case 0x41:if(!box(point(object.minimum),point(object.maximum),s))return false;break;
        case 0x42:case 0x44:if(!sphere(object.position,object.radius,s))return false;break;
        default:throw std::runtime_error("Required dynamic ray primitive is not reconstructed");
        }
        s.hit_dynamic=&object;s.hit_static.reset();return true;
    }
private:
    static CollisionPoint point(const CollisionRayState::Vector& v){return {{v[0],v[1],v[2]}};}
    Math& math_;
};

//006F7C90's directional dominant-axis grid traversal;006F5890's near-vertical
//branch instead visits the bounding cells Z/Y/X. Y cell limits use the ORIGINAL
//full segment, even after a dynamic hit shortens the static bounds.
template<class Frame,class Resources,class Shapes>
void visit_collision_ray_cells(const CollisionTree<Frame>& tree,CollisionRayState& s,
                               float original_low_y,float original_high_y,Resources& resources,Shapes& shapes,bool vertical) {
    if(tree.cell_fields8.size()!=tree.cells.size())throw std::runtime_error("Ray cell Y limits are missing");
    const auto x=collision_sweep_cell_range(s.minimum[0],s.maximum[0],tree.bounds_and_steps[0],tree.bounds_and_steps[4],tree.count_x);
    const auto z=collision_sweep_cell_range(s.minimum[2],s.maximum[2],tree.bounds_and_steps[1],tree.bounds_and_steps[5],tree.count_z);
    const auto y=collision_sweep_cell_range(s.minimum[1],s.maximum[1],tree.fields20[1],tree.fields20[3],tree.count_y);
    auto visit=[&](std::int32_t ix,std::int32_t iy,std::int32_t iz) {
        const auto index=tree.cell_index(std::size_t(ix),std::size_t(iy),std::size_t(iz));const auto limits=tree.cell_fields8.at(index);
        if(original_low_y<=limits[0]&&limits[1]<=original_high_y)filter_collision_ray_cell(tree,tree.cells.at(index),s,resources,shapes);
    };
    if(vertical) {
        for(auto iz=z.begin;iz<z.end;++iz)for(auto iy=y.begin;iy<y.end;++iy)for(auto ix=x.begin;ix<x.end;++ix)visit(ix,iy,iz);
        return;
    }
    const bool major_z=std::abs(s.displacement[0])<std::abs(s.displacement[2]);const unsigned major=major_z?2u:0u,minor=major_z?0u:2u;
    const auto primary=major_z?z:x,secondary=major_z?x:z;const auto& edges=major_z?tree.edges_z:tree.edges_x;
    const auto origin=tree.bounds_and_steps[major_z?0:1],step=tree.bounds_and_steps[major_z?4:5];
    const bool negative=s.displacement[major]<0,reverse_minor=major_z&&s.displacement[0]<0&&s.displacement[2]<0;
    const auto increment=negative?-1:1;
    for(auto p=negative?primary.end-1:primary.begin;negative?p>=primary.begin:p<primary.end;p+=increment) {
        auto project=[&](float edge) {
            return ((static_cast<long double>(edge)-s.start[major])/s.displacement[major]*s.displacement[minor]+s.start[minor]-origin)/step;
        };
        const auto a=project(edges.at(std::size_t(p))),b=project(edges.at(std::size_t(p+1)));
        auto truncate=[](long double v){if(!std::isfinite(v)||v<std::numeric_limits<std::int32_t>::min()||v>std::numeric_limits<std::int32_t>::max())throw std::runtime_error("Ray grid coordinate exceeds native range");return static_cast<std::int32_t>(v);};
        const auto low=std::max(secondary.begin,truncate(std::min(a,b))),high=std::min(secondary.end,truncate(std::max(a,b)+1));
        for(auto q=reverse_minor?high-1:low;reverse_minor?q>=low:q<high;q+=reverse_minor?-1:1)
            for(auto iy=y.begin;iy<y.end;++iy)visit(major_z?q:p,iy,major_z?p:q);
        const auto endpoint=static_cast<long double>(s.parameter)*s.displacement[major]+s.start[major];
        if(negative?edges.at(std::size_t(p))<=endpoint:endpoint<=edges.at(std::size_t(p+1)))break;
    }
}

template<class Frame,class Object,class Resources,class Math>class CollisionRay {
public:
    using Tree=CollisionTree<Frame>;using Grid=typename Tree::DynamicGrid;
    using Buckets=DynamicCollisionBuckets<Object,Grid>;using Vector=CollisionRayState::Vector;
    using Hit=typename CollisionSweep<Frame,Object,Resources,Math>::Hit;
    CollisionRay(const Tree* tree,const Buckets* buckets,Resources& resources,Math& math)
        :tree_(tree),buckets_(buckets),resources_(resources),static_(math),dynamic_(math){}
    std::optional<Hit> ray(const Vector& start,const Vector& displacement,std::uint32_t excluded_layers=0,std::uint32_t excluded_flags=0,bool normal=true) {
        if(!tree_)return std::nullopt;
        using namespace collision_ray_math;
        CollisionRayState s;s.start=start;s.displacement=displacement;s.excluded_layers=excluded_layers|8u;
        s.excluded_flags=(excluded_flags==0xffffffffu?0:excluded_flags)|0x20u;
        const bool vertical=std::abs(static_cast<long double>(displacement[2]))+std::abs(static_cast<long double>(displacement[0]))<9.99999993922529e-09f;
        //006F5890 sums X/Y/Z;006F7C90 sums Z/X/Y. Keep their native FLOAT square.
        s.square=stored(vertical?square(displacement):(static_cast<long double>(displacement[2])*displacement[2]+static_cast<long double>(displacement[0])*displacement[0])+static_cast<long double>(displacement[1])*displacement[1]);
        if(!std::isfinite(s.square))throw std::runtime_error("Non-finite collision ray displacement");
        s.length=stored(std::sqrt(static_cast<long double>(s.square)));bounds(s);
        const auto low_y=s.minimum[1],high_y=s.maximum[1];
        if(excluded_layers!=0xffffffffu&&buckets_) {
            const auto& grid=tree_->dynamic;
            const auto x=collision_sweep_cell_range(s.minimum[0],s.maximum[0],grid.origin[0],grid.step[0],grid.count_x);
            const auto z=collision_sweep_cell_range(s.minimum[2],s.maximum[2],grid.origin[1],grid.step[1],grid.count_z);
            bool hit=false;
            for(auto iz=z.begin;iz<z.end;++iz)for(auto ix=x.begin;ix<x.end;++ix)if(const auto* bucket=buckets_->bucket(std::uint32_t(ix),std::uint32_t(iz)))
                for(const auto* object:bucket->objects) {
                    if(!object)throw std::runtime_error("Null ray descriptor");
                    if(!(s.excluded_layers&object->mask)&&!(s.excluded_flags&object->flags))hit=dynamic_.ray(*object,s)||hit;
                }
            if(hit&&!vertical)bounds(s); //Vertical wrapper retains its original bounds after dynamic queries.
        }
        if(excluded_flags!=0xffffffffu)visit_collision_ray_cells(*tree_,s,low_y,high_y,resources_,static_,vertical);
        if(!s.hit_static&&!s.hit_dynamic)return std::nullopt;
        Hit result;result.static_identity=s.hit_static;result.dynamic_identity=static_cast<const Object*>(s.hit_dynamic);
        Vector travel{};for(unsigned a=0;a!=3;++a){travel[a]=stored(static_cast<long double>(s.parameter)*displacement[a]);result.position[a]=stored(static_cast<long double>(s.parameter)*displacement[a]+start[a]);}
        result.distance_squared=stored(square(travel));
        if(normal) {
            for(unsigned a=0;a!=3;++a)result.normal[a]=-s.normal[a];
            result.normal=actor_activation_direction(result.normal);
        }
        return result;
    }
private:
    const Tree* tree_;const Buckets* buckets_;Resources& resources_;
    StaticCollisionRayNarrowPhase<Frame,Math> static_;DynamicCollisionRayNarrowPhase<Object,Math> dynamic_;
};
} //namespace hd2::game
#endif
