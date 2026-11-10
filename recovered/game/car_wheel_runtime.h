#ifndef HD2_GAME_CAR_WHEEL_RUNTIME_H
#define HD2_GAME_CAR_WHEEL_RUNTIME_H

#include "game/car_wheel_geometry.h"
#include "game/car_tyres.h"
#include <map>

namespace hd2::game {

//The inlined quaternion product in004EA780. Native temporaries spill each
//scaled vector, cross product and vector addition separately to FLOAT.
inline std::array<float,4> car_quaternion_product(const std::array<float,4>& a,const std::array<float,4>& b){
    std::array<float,4> result{};std::array<float,3> cross{},first{},second{};
    result[3]=static_cast<float>(static_cast<long double>(a[3])*b[3]-((static_cast<long double>(b[0])*a[0]+static_cast<long double>(a[1])*b[1])+static_cast<long double>(a[2])*b[2]));
    for(unsigned axis=0;axis!=3;++axis){
        const auto next=(axis+1)%3,last=(axis+2)%3;
        cross[axis]=static_cast<float>(static_cast<long double>(a[next])*b[last]-static_cast<long double>(a[last])*b[next]);
        first[axis]=a[axis]*b[3];second[axis]=b[axis]*a[3];
        const float sum=first[axis]+second[axis];result[axis]=sum+cross[axis];
    }
    return result;
}

//004EA780's wheel initialization, using actual constructed geometry and the
//SAME applied per-wheel parameters/tyre table. These are simulation values,
//not native object bytes. Reads of unproduced scalar/vector slots throw.
template<class Frames>struct CarWheelRuntime {
    using Frame=typename Frames::Frame;using Vector=typename Frames::Vector;
    Frame frame{},parent{};
    const void* query_identity=nullptr;const void* previous_query_identity=nullptr;
    std::uint32_t previous_query_flags=0;
    CarPhysicsScalars scalars;std::map<unsigned,Vector> vectors;
    std::array<float,4> rotation{},alignment{};
    std::uint32_t flags=0;
    std::uint32_t skid_cursor=0; //218 is an integer pool index, not a physics coefficient.
    std::int32_t material_id=-1,material_type=-1,material_surface=0,effect=-1;
    std::uint32_t surface_id=0;
    void initialize(Frames& frames,Frame model,const typename CarWheelGeometry<Frames>::Wheel& geometry,
            const CarWheelPhysics& physics,const CarTyres& tyres,const Vector& up,unsigned index){
        if(!model||!geometry.frame||!geometry.parent)throw std::runtime_error("Car wheel runtime requires actual model/wheel/parent");
        frame=geometry.frame;parent=geometry.parent;scalars=physics.scalars;flags=physics.flags&0x2effff07u;
        scalars.set(0x1d8,geometry.radius);scalars.set(0x1dc,geometry.contact_radius);scalars.set(0x214,geometry.width);
        scalars.set(0x1e4,geometry.diagonal);scalars.set(0x1e8,geometry.contact_diagonal);scalars.set(0x1ec,geometry.angle);scalars.set(0x15c,geometry.local_x);
        material_id=physics.material_id;material_type=physics.material_index;material_surface=0;surface_id=0;
        query_identity=previous_query_identity=nullptr;previous_query_flags=0;effect=-1;skid_cursor=0;
        scalars.set(0x1e0,0); //004E2E30's authored wheel construction, before the runtime query additions.
        vectors.clear();vectors[0x20]=geometry.local_position;vectors[0x30]=geometry.position;
        const auto world=frames.actor_world_matrix(frame);vectors[0x40]={{world[12],world[13],world[14],world[15]}};
        Vector anchor{},previous{};
        for(unsigned axis=0;axis!=3;++axis){
            anchor[axis]=static_cast<float>(static_cast<long double>(vectors.at(0x40)[axis])-static_cast<long double>(geometry.diagonal)*up[axis]);
            previous[axis]=anchor[axis]-up[axis];
        }
        vectors[0x50]=anchor;vectors[0x60]=previous;
        for(unsigned offset:{0xa0u,0xb0u,0xc0u,0xd0u})vectors[offset]=Vector{};
        vectors[0x130]={{0,1,0,0}};
        for(unsigned offset:{0x16cu,0x1f0u,0x25cu,0x260u,0x264u,0x168u,0x1a8u,0x13cu})scalars.set(offset,0);
        scalars.set(0x26c,1000.f);
        frames.object_visible(frame,true);
        if(material_type>0){
            const auto& surfaces=tyres.types().at(static_cast<std::size_t>(material_type)).surfaces;
            material_surface=-1;
            for(auto n=surfaces.size();n;)if(surfaces[--n].id==0){material_surface=static_cast<std::int32_t>(n);break;}
        }
        frames.car_loading_forward_basis(frame);frames.update_scene_object(frame);
        float angle=physics.scalars.get(0x150)+physics.scalars.get(0x158);
        if(!(index&1u))angle=-angle;
        alignment=frames.car_axis_quaternion({{0,0,1,0}},angle);
        if(frames.object_parent(parent)==model)frames.car_loading_forward_basis(parent);
        frames.update_scene_object(parent);
        const auto first=frames.car_axis_quaternion({{1,0,0,0}},physics.scalars.get(0x154));
        float last=physics.scalars.get(0x158);if(index&1u)last=-last;
        rotation=car_quaternion_product(first,frames.car_axis_quaternion({{0,0,1,0}},last));
    }
    void reset_loading_health(){scalars.set(0x22c,0);scalars.set(0x230,scalars.get(0x234));} //004FEEA0's wheel pass.
};
} //namespace hd2::game
#endif
