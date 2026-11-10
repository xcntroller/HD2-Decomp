#ifndef HD2_GAME_COLLISION_BOX_QUERY_H
#define HD2_GAME_COLLISION_BOX_QUERY_H

#include "game/collision_sweep_candidates.h"
#include "game/dynamic_collision_grid.h"
#include <cstring>

namespace hd2::game {

//006F06F0 /007185E0 broad query. This is deliberately distinct from a swept
//sphere or exact box collision.00514D70 follows a positive result with the
//actual BODYCOL point sweeps. All geometry comes from the SAME tree/buckets.
template<class Frame,class Object,class Resources,class Math>
class CollisionBoxQuery {
public:
    using Tree=CollisionTree<Frame>;using Grid=typename Tree::DynamicGrid;
    using Buckets=DynamicCollisionBuckets<Object,Grid>;
    using Vector=typename Math::Vector;using Matrix=typename Math::Matrix;
    CollisionBoxQuery(const Tree* tree,const Buckets* buckets,Resources& resources,Math& math)
        :tree_(tree),buckets_(buckets),resources_(resources),math_(math){}
    template<class Bounds>bool query(const Bounds& bounds,const Matrix& before,const Matrix& after,
            const Vector& displacement,const Vector& old_up,const Vector& up,
            std::uint32_t layers,std::uint32_t flags){
        if(!tree_)return false;
        //The initial displacement/averaged-up cross products are overwritten
        //before any consumer reads them. They have no query/filter effect.
        (void)displacement;(void)old_up;
        State state;state.layers=layers|8u;state.flags=(flags==0xffffffffu?0u:flags)|0x20u;
        std::array<Vector,8> corners{};unsigned n=0;
        for(unsigned x=0;x!=2;++x)for(unsigned y=0;y!=2;++y)for(unsigned z=0;z!=2;++z)
            corners[n++]={{x?bounds.minimum[0]:bounds.maximum[0],y?bounds.minimum[1]:bounds.maximum[1],
                z?bounds.maximum[2]:bounds.minimum[2],0}};
        auto include=[&](const Vector& point){for(unsigned axis=0;axis!=3;++axis){
            state.low[axis]=std::min(state.low[axis],point[axis]);state.high[axis]=std::max(state.high[axis],point[axis]);}};
        state.low=state.high=math_.car_transform(corners[0],before);
        for(unsigned i=1;i!=8;++i)include(math_.car_transform(corners[i],before));
        Vector last{};for(const auto& corner:corners){last=math_.car_transform(corner,after);include(last);}
        Vector centre{};for(unsigned axis=0;axis!=3;++axis)
            centre[axis]=static_cast<float>((static_cast<long double>(bounds.minimum[axis])+bounds.maximum[axis])*.5f);
        state.centre=math_.car_transform(centre,after);
        Vector delta{},expanded{};
        for(unsigned axis=0;axis!=3;++axis){
            delta[axis]=state.centre[axis]-last[axis];
            expanded[axis]=static_cast<float>(static_cast<long double>(up[axis])*20.f+state.centre[axis])-last[axis];
        }
        //Native stores the SQUARED corner distance at query+68 and adds that
        //value directly to descriptor radii in006F0050/006F02C0. Do not sqrt it.
        state.radius=static_cast<float>((static_cast<long double>(delta[1])*delta[1]+
            static_cast<long double>(delta[2])*delta[2])+static_cast<long double>(delta[0])*delta[0]);
        state.plane_radius=static_cast<float>((static_cast<long double>(expanded[1])*expanded[1]+
            static_cast<long double>(expanded[2])*expanded[2])+static_cast<long double>(expanded[0])*expanded[0]);
        if(layers!=0xffffffffu&&buckets_&&dynamic(state))return true;
        if(flags==0xffffffffu)return false;
        const auto x=collision_sweep_cell_range(state.low[0],state.high[0],tree_->bounds_and_steps[0],tree_->bounds_and_steps[4],tree_->count_x);
        const auto z=collision_sweep_cell_range(state.low[2],state.high[2],tree_->bounds_and_steps[1],tree_->bounds_and_steps[5],tree_->count_z);
        const auto y=collision_sweep_cell_range(state.low[1],state.high[1],tree_->fields20[1],tree_->fields20[3],tree_->count_y);
        bool hit=false;
        for(auto iz=z.begin;iz<z.end;++iz)for(auto iy=y.begin;iy<y.end;++iy)for(auto ix=x.begin;ix<x.end;++ix){
            const auto index=tree_->cell_index(std::size_t(ix),std::size_t(iy),std::size_t(iz));
            //006F0E51 tests only cell+08, with a strict comparison.
            if(state.low[1]<tree_->cell_fields8.at(index)[0])hit=cell(tree_->cells.at(index),state)||hit;
        }
        return hit;
    }
private:
    struct State {Vector low{},high{},centre{};float radius=0,plane_radius=0;std::uint32_t layers=0,flags=0;};
    static float square(const Vector& value){return static_cast<float>((static_cast<long double>(value[0])*value[0]+
        static_cast<long double>(value[1])*value[1])+static_cast<long double>(value[2])*value[2]);}
    static bool sphere(const Vector& position,float radius,const State& state,bool horizontal=false,bool strict=false){
        Vector delta{};for(unsigned axis=0;axis!=3;++axis)delta[axis]=(horizontal&&axis==1)?0:position[axis]-state.centre[axis];
        const float combined=radius+state.radius;
        return strict?square(delta)<combined*combined:square(delta)<=combined*combined;
    }
    bool dynamic(const State& state)const{ //006F0050: layer mask ONLY, not flags.
        const auto& grid=tree_->dynamic;
        const auto x=collision_sweep_cell_range(state.low[0],state.high[0],grid.origin[0],grid.step[0],grid.count_x);
        const auto z=collision_sweep_cell_range(state.low[2],state.high[2],grid.origin[1],grid.step[1],grid.count_z);
        for(auto iz=z.begin;iz<z.end;++iz)for(auto ix=x.begin;ix<x.end;++ix){
            const auto* bucket=buckets_->bucket(std::uint32_t(ix),std::uint32_t(iz));if(!bucket)continue;
            for(const auto* object:bucket->objects){
                if(!object)throw std::runtime_error("Null dynamic box-query descriptor");
                if(object->mask&state.layers)continue;
                switch(object->flags&0xffu){
                case 0x40:case 0x42:case 0x44:if(sphere(object->position,object->radius,state))return true;break;
                case 0x41:{bool overlap=true;for(unsigned axis=0;axis!=3;++axis){
                    overlap=overlap&&object->minimum[axis]<=state.high[axis]&&state.low[axis]<=object->maximum[axis];
                    }
                    if(overlap)return true;
                    break;}
                case 0x49:return true;
                default:break; //Original switch has no other consumers.
                }
            }
        }
        return false;
    }
    static float real(const CollisionShape<Frame>& shape,unsigned offset){
        const auto bits=shape.fields8.at((offset-8)/4);float result;std::memcpy(&result,&bits,4);return result;
    }
    static Vector point(const CollisionShape<Frame>& shape,unsigned offset){return {{real(shape,offset),real(shape,offset+4),real(shape,offset+8),0}};}
    bool shape(const CollisionShape<Frame>& value,const State& state){ //006F02C0, genuine absent cases return false.
        switch(value.flags&0xffu){
        case 0x80:case 0x81:{const auto low=point(value,8),high=point(value,0x14);
            for(unsigned axis=0;axis!=3;++axis)if(!(low[axis]<state.high[axis]&&state.low[axis]<high[axis]))return false;
            return true;}
        case 0x82:return sphere(point(value,0x10),real(value,8),state,false,true);
        case 0x83:{Matrix inverse{};for(unsigned i=0;i!=16;++i)inverse[i]=real(value,0x60+i*4);
            const auto local=math_.car_transform(state.centre,inverse);const auto low=point(value,8),high=point(value,0x14);
            return low[0]<local[0]+state.radius&&local[0]-state.radius<high[0]&&
                low[2]<local[2]+state.radius&&local[2]-state.radius<high[2]&&
                0<local[1]+state.radius&&local[1]-state.radius<0;}
        case 0x84:return sphere({{real(value,8),state.centre[1],real(value,0xc),0}},real(value,0x10),state,true,true);
        default:return false;
        }
    }
    bool cell(const std::vector<CollisionCellReference>& entries,const State& state){ //006F04F0.
        for(const auto& entry:entries){
            if(entry.kind){const auto& value=tree_->shapes.at(entry.kind-0x80).at(entry.index);
                if(!(value.flags&state.flags)&&shape(value,state))return true;
                continue;}
            const auto& triangle=tree_->triangles.at(entry.index);if(triangle.flags&state.flags)continue;
            if(triangle.flags&0xa0u)throw std::runtime_error("Box query triangle has an unresolved shape/rejected tag");
            const auto vertices=collision_triangle_vertices(*tree_,triangle,resources_);
            if(!(state.low[0]<=vertices[2][0]))continue;
            if(state.high[0]<vertices[0][0])return false; //Native sorted early exit.
            bool overlap=true;for(unsigned axis=1;axis!=3;++axis){
                const float low=std::min({vertices[0][axis],vertices[1][axis],vertices[2][axis]});
                const float high=std::max({vertices[0][axis],vertices[1][axis],vertices[2][axis]});
                overlap=overlap&&low<=state.high[axis]&&state.low[axis]<=high;
            }
            if(!overlap)continue;
            const auto& normal=triangle.plane;
            const float distance=static_cast<float>(std::abs((static_cast<long double>(state.centre[0])-vertices[0][0])*normal[0]+
                (static_cast<long double>(state.centre[1])-vertices[0][1])*normal[1]+
                (static_cast<long double>(state.centre[2])-vertices[0][2])*normal[2]));
            if(distance*distance<=state.plane_radius)return true;
        }
        return false;
    }
    const Tree* tree_;const Buckets* buckets_;Resources& resources_;Math& math_;
};
} //namespace hd2::game
#endif
