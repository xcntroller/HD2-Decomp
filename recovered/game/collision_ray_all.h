#ifndef HD2_GAME_COLLISION_RAY_ALL_H
#define HD2_GAME_COLLISION_RAY_ALL_H
#include "game/collision_ray.h"
#include "game/human_ai_cover.h"
namespace hd2::game {
//006FF140/00700BC0/00701350. Every result borrows the SAME loaded tree or
//descriptor. A fresh closest-ray query per hit is not the native all-hit query.
template<class Frame,class Object,class Resources,class Math>class CollisionRayAll {
public:
    using Tree=CollisionTree<Frame>;using Buckets=DynamicCollisionBuckets<Object,typename Tree::DynamicGrid>;
    using Vector=CollisionRayState::Vector;
    struct Hit {
        Vector position{},normal{};float parameter=0;
        std::optional<CollisionCellReference> static_identity;
        const Object* dynamic_identity=nullptr;
    };
    CollisionRayAll(const Tree& tree,const Buckets& buckets,Resources& resources,Math& math)
        :tree_(tree),buckets_(buckets),resources_(resources),math_(math),dynamic_(math),shapes_(math){
        if(&buckets.grid_identity()!=&tree.dynamic)throw std::logic_error("All-hit ray requires SAME loaded grid/buckets");
    }
    std::vector<Hit> ray(const Vector& start,const Vector& displacement,std::uint32_t excluded_layers,std::uint32_t excluded_flags){
        CollisionRayState state;state.start=start;state.displacement=displacement;state.excluded_layers=excluded_layers|8u;state.excluded_flags=excluded_flags|0x20u;
        for(unsigned axis=0;axis!=3;++axis)if(!std::isfinite(start[axis])||!std::isfinite(displacement[axis]))throw std::runtime_error("All-hit ray reached invalid nonfinite native coordinates");
        collision_ray_math::bounds(state);std::vector<Hit> output;
        if(excluded_layers!=0xffffffffu){
            state.square=stored((static_cast<long double>(displacement[2])*displacement[2]+static_cast<long double>(displacement[1])*displacement[1])+static_cast<long double>(displacement[0])*displacement[0]);
            state.length=stored(std::sqrt(static_cast<long double>(state.square)));
            const auto& grid=tree_.dynamic;
            const auto x=collision_sweep_cell_range(state.minimum[0],state.maximum[0],grid.origin[0],grid.step[0],grid.count_x);
            const auto z=collision_sweep_cell_range(state.minimum[2],state.maximum[2],grid.origin[1],grid.step[1],grid.count_z);
            for(auto iz=z.begin;iz<z.end;++iz)for(auto ix=x.begin;ix<x.end;++ix)if(const auto* bucket=buckets_.bucket(std::uint32_t(ix),std::uint32_t(iz)))for(const auto* object:bucket->objects){
                if(!object)throw std::runtime_error("All-hit ray reached native NULL descriptor");
                if(object->mask&state.excluded_layers)continue; //Native dynamic pass does NOT test excluded_flags.
                const auto kind=object->flags&0xffu;if(kind!=0x40&&kind!=0x41&&kind!=0x42&&kind!=0x44)continue; //Genuine empty switch43/default.
                if(!dynamic_bounds(*object,state))continue;
                auto candidate=state;
                if(dynamic_.primitive(*object,candidate))insert(output,Hit{{},candidate.normal,candidate.parameter,std::nullopt,object});
            }
        }
        if(excluded_flags!=0xffffffffu){
            state.square=stored((static_cast<long double>(displacement[0])*displacement[0]+static_cast<long double>(displacement[1])*displacement[1])+static_cast<long double>(displacement[2])*displacement[2]);
            //The native dominant-axis projection has no vertical alternate.
            //Both horizontal components zero produce an empty secondary span
            //after FTOL; dynamic results above still survive.
            if(displacement[0]!=0||displacement[2]!=0)visit_collision_ray_grid(tree_,state,state.minimum[1],state.maximum[1],false,[&](const auto& cell){
                for(const auto& identity:cell){
                    if(identity.kind){const auto& shape=tree_.shapes.at(identity.kind-0x80).at(identity.index);if(!(shape.flags&state.excluded_flags))shape_hit(identity,shape,state,output);}
                    else triangle_hit(identity,state,output);
                }
            });
        }
        for(auto& hit:output){
            for(unsigned axis=0;axis!=3;++axis)hit.position[axis]=stored(static_cast<long double>(hit.parameter)*displacement[axis]+start[axis]);
            const auto square=(static_cast<long double>(hit.normal[2])*hit.normal[2]+static_cast<long double>(hit.normal[1])*hit.normal[1])+static_cast<long double>(hit.normal[0])*hit.normal[0];
            normalize_human_ai_shot(hit.normal,square); //No global normal flip; dynamic/static policies differ.
        }
        return output;
    }
private:
    static float stored(long double value){return static_cast<float>(value);}
    static bool same(const Hit& a,const Hit& b){
        if(a.dynamic_identity||b.dynamic_identity)return a.dynamic_identity==b.dynamic_identity;
        return a.static_identity&&b.static_identity&&a.static_identity->kind==b.static_identity->kind&&a.static_identity->index==b.static_identity->index;
    }
    static void insert(std::vector<Hit>& output,Hit hit){ //Native scans through equals, checks identity BEFORE first greater parameter.
        auto where=output.begin();for(;where!=output.end();++where){if(same(*where,hit))return;if(hit.parameter<where->parameter)break;}
        output.insert(where,std::move(hit));
    }
    static float real(const CollisionShape<Frame>& shape,unsigned offset){const auto bits=shape.fields8.at((offset-8)/4);float value;std::memcpy(&value,&bits,4);return value;}
    static typename Math::Matrix matrix(const CollisionShape<Frame>& shape,unsigned offset){typename Math::Matrix value{};for(unsigned n=0;n!=16;++n)value[n]=real(shape,offset+n*4);return value;}
    static bool dynamic_bounds(const Object& object,const CollisionRayState& state){
        Vector delta{},perpendicular{};for(unsigned a=0;a!=3;++a)delta[a]=stored(static_cast<long double>(state.start[a])-object.position[a]);
        const auto square=(static_cast<long double>(state.displacement[0])*state.displacement[0]+static_cast<long double>(state.displacement[1])*state.displacement[1])+static_cast<long double>(state.displacement[2])*state.displacement[2];
        long double distance=-10000000272564224.f;
        if(square>=9.99999993922529e-09f){
            const auto projection=stored(((static_cast<long double>(delta[0])*state.displacement[0]+static_cast<long double>(delta[1])*state.displacement[1])+static_cast<long double>(delta[2])*state.displacement[2])/square);
            for(unsigned a=0;a!=3;++a)perpendicular[a]=stored(static_cast<long double>(delta[a])-stored(static_cast<long double>(state.displacement[a])*projection));
            distance=human_ai_cover::distance_xyz(perpendicular,Vector{});
        }
        if(!(distance<=object.radius))return false;
        for(unsigned a=0;a!=3;++a)if(!(state.minimum[a]<=static_cast<long double>(object.radius)+object.position[a]&&static_cast<long double>(object.position[a])-object.radius<=state.maximum[a]))return false;
        return true;
    }
    void triangle_hit(const CollisionCellReference& id,const CollisionRayState& state,std::vector<Hit>& output){
        const auto& triangle=tree_.triangles.at(id.index);if(triangle.flags&state.excluded_flags)return;
        if(triangle.flags&0xa0)throw std::runtime_error("All-hit triangle identity lost its actual primitive family");
        const auto vertices=collision_triangle_vertices(tree_,triangle,resources_);
        if(vertices[2][0]<state.minimum[0]||state.maximum[0]<vertices[0][0])return; //No sorted-X early return in00700BC0.
        const auto low=std::min({vertices[0][2],vertices[1][2],vertices[2][2]}),high=std::max({vertices[0][2],vertices[1][2],vertices[2][2]});
        if(state.maximum[2]<low||high<state.minimum[2])return;
        const auto& plane=triangle.plane;
        const float dot=stored((static_cast<long double>(state.displacement[2])*plane[2]+static_cast<long double>(state.displacement[1])*plane[1])+static_cast<long double>(state.displacement[0])*plane[0]);
        if((triangle.flags&0x40000u)?!(std::abs(dot)>=9.99999993922529e-09):!(dot>=9.99999993922529e-09f))return;
        const float parameter=stored(-(((static_cast<long double>(state.start[0])*plane[0]+static_cast<long double>(state.start[2])*plane[2])+static_cast<long double>(state.start[1])*plane[1])+plane[3])/dot);
        if(parameter<0||1.f<parameter)return; //Endpoint1 is admitted.
        std::array<long double,3> point{},a{},b{};
        for(unsigned axis=0;axis!=3;++axis){point[axis]=(static_cast<long double>(parameter)*state.displacement[axis]+state.start[axis])-vertices[0][axis];a[axis]=static_cast<long double>(vertices[1][axis])-vertices[0][axis];b[axis]=static_cast<long double>(vertices[2][axis])-vertices[0][axis];}
        float u=0;long double v=0;
        switch(triangle.flags&0xffu){
        case 1:u=stored(point[1]/b[1]);v=(point[2]-b[2]*u)/a[2];break;
        case 2:u=stored((a[1]*point[2]-a[2]*point[1])/(a[1]*b[2]-b[1]*a[2]));v=(point[1]-b[1]*u)/a[1];break;
        case 3:u=stored(point[0]/b[0]);v=(point[2]-b[2]*u)/a[2];break;
        case 4:u=stored(point[2]/b[2]);v=(point[0]-b[0]*u)/a[0];break;
        case 5:u=stored((a[0]*point[2]-a[2]*point[0])/(b[2]*a[0]-a[2]*b[0]));v=(point[0]-b[0]*u)/a[0];break;
        case 6:u=stored(point[0]/b[0]);v=(point[1]-b[1]*u)/a[1];break;
        case 7:u=stored((a[0]*point[1]-a[1]*point[0])/(b[1]*a[0]-a[1]*b[0]));v=(point[0]-b[0]*u)/a[0];break;
        default:return;
        }
        if(!(0<=u&&u<=1&&0<=v&&u+v<=1))return;
        insert(output,Hit{{},{{-plane[0],-plane[1],-plane[2],0}},parameter,id,nullptr});
    }
    void shape_hit(const CollisionCellReference& id,const CollisionShape<Frame>& shape,const CollisionRayState& state,std::vector<Hit>& output){
        const auto kind=shape.flags&0xffu;auto candidate=state;
        if(kind==0x80||kind==0x81){
            candidate.parameter=std::nextafter(1.f,2.f); //These all-hit shapes admit exact endpoint1.
            if(!shapes_.ray_shape(id,shape,candidate)||candidate.parameter>1)return;
            for(unsigned a=0;a!=3;++a)candidate.normal[a]=-candidate.normal[a];
        }else if(kind==0x82){
            Vector delta{};for(unsigned a=0;a!=3;++a)delta[a]=stored(static_cast<long double>(real(shape,0x10+a*4))-state.start[a]);
            const auto projection=(static_cast<long double>(delta[2])*state.displacement[2]+static_cast<long double>(delta[1])*state.displacement[1])+static_cast<long double>(delta[0])*state.displacement[0];
            const auto radius=static_cast<long double>(real(shape,8));
            const auto square=(static_cast<long double>(delta[0])*delta[0]+static_cast<long double>(delta[2])*delta[2])+static_cast<long double>(delta[1])*delta[1];
            const auto discriminant=projection*projection-(square-radius*radius)*state.square;if(!(discriminant>=0))return;
            candidate.parameter=stored((projection-std::sqrt(discriminant))/state.square);if(!(candidate.parameter>=0&&candidate.parameter<=1))return;
            for(unsigned a=0;a!=3;++a){const float travel=stored(static_cast<long double>(candidate.parameter)*state.displacement[a]);const float position=stored(static_cast<long double>(travel)+state.start[a]);candidate.normal[a]=stored(static_cast<long double>(position)-real(shape,0x10+a*4));}
        }else if(kind==0x83){
            const auto inverse=matrix(shape,0x60),world=matrix(shape,0x20);
            const auto origin=math_.car_transform(state.start,inverse),motion=math_.collision_sweep_rotate(state.displacement,inverse);
            if(!(std::abs(static_cast<long double>(motion[1]))>9.99999993922529e-09))return;
            candidate.parameter=stored(-static_cast<long double>(origin[1])/motion[1]);if(!(candidate.parameter>0&&candidate.parameter<=1))return;
            const auto x=stored(static_cast<long double>(motion[0])*candidate.parameter+origin[0]),z=stored(static_cast<long double>(motion[2])*candidate.parameter+origin[2]);
            if(!(real(shape,8)<=x&&x<=real(shape,0x14)&&real(shape,0x10)<=z&&z<=real(shape,0x1c)))return;
            candidate.normal=math_.collision_sweep_rotate(Vector{{0,motion[1]<=0?1.f:-1.f,0,0}},world);
        }else if(kind==0x84){
            const float x=stored(static_cast<long double>(real(shape,8))-state.start[0]),z=stored(static_cast<long double>(real(shape,0xc))-state.start[2]);
            const float square=stored(static_cast<long double>(state.displacement[2])*state.displacement[2]+static_cast<long double>(state.displacement[0])*state.displacement[0]);
            const auto length=human_ai_cover::length(square);if(!(std::abs(length)>=9.99999993922529e-09))return;
            const auto inverse=1.L/length;
            const auto projection=(static_cast<long double>(state.displacement[0])*inverse*x+inverse*0.f*0.f)+static_cast<long double>(z)*state.displacement[2]*inverse;
            const auto radius=static_cast<long double>(real(shape,0x10));
            const auto discriminant=radius*radius-((static_cast<long double>(x)*x+static_cast<long double>(z)*z)-projection*projection);if(!(discriminant>=0))return;
            candidate.parameter=stored((projection-std::sqrt(discriminant))*inverse);if(!(candidate.parameter>=0&&candidate.parameter<=1))return;
            candidate.normal={{stored(static_cast<long double>(candidate.parameter)*state.displacement[0]+state.start[0]-real(shape,8)),0,stored(static_cast<long double>(candidate.parameter)*state.displacement[2]+state.start[2]-real(shape,0xc)),0}};
        }else return; //00701350 has no positive85/86/87 branch.
        insert(output,Hit{{},candidate.normal,candidate.parameter,id,nullptr});
    }
    const Tree& tree_;const Buckets& buckets_;Resources& resources_;Math& math_;
    DynamicCollisionRayNarrowPhase<Object,Math> dynamic_;StaticCollisionRayNarrowPhase<Frame,Math> shapes_;
};
} //namespace hd2::game
#endif
