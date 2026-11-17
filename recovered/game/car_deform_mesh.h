#ifndef HD2_GAME_CAR_DEFORM_MESH_H
#define HD2_GAME_CAR_DEFORM_MESH_H

#include "game/car_physics.h"
#include <cstring>
#include <exception>

namespace hd2::game {
//004FF8B0/004FE910 lock the source FIRST, then the destination, even
//when both are the same LOD. Only XYZ in each32-byte vertex is changed.
template<class Frames,class Visit>void visit_car_deform_vertices(Frames& frames,
        typename Frames::Frame source,typename Frames::Frame destination,Visit&& visit){
    bool source_locked=false,destination_locked=false;
    try{
        auto* original=frames.car_lock_deform_vertices(source);source_locked=true;
        auto* current=frames.car_lock_deform_vertices(destination);destination_locked=true;
        const auto count=frames.car_deform_vertex_count(destination);
        const auto read=[&](unsigned char* data,typename Frames::Frame lod,std::size_t index){
            if(!data||index>=frames.car_deform_vertex_count(lod))throw std::runtime_error("Car deformation vertex has no actual source");
            CarContactVector value{};std::memcpy(value.data(),data+index*32,12);return value;
        };
        visit(count,[&](std::size_t index){return read(original,source,index);},
            [&](std::size_t index){return read(current,destination,index);},
            [&](std::size_t index,const CarContactVector& value){
                if(!current||index>=count)throw std::out_of_range("Car deformation destination vertex");
                std::memcpy(current+index*32,value.data(),12);
            });
        source_locked=false;frames.car_unlock_deform_vertices(source);
        destination_locked=false;frames.car_unlock_deform_vertices(destination);
    }catch(...){
        const auto primary=std::current_exception();
        const auto cleanup=[&](bool locked,typename Frames::Frame lod){if(locked)try{frames.car_unlock_deform_vertices(lod);}
            catch(...){frames.record_actor_cleanup_failure(std::current_exception());}};
        cleanup(source_locked,source);cleanup(destination_locked,destination);
        std::rethrow_exception(primary);
    }
}
inline long double clamp_car_deformation(long double value,float limit){
    return limit<std::abs(value)?(value<=0?-static_cast<long double>(limit):limit):value;
}
//004FF8B0's distinct native FLOAT spills; shared CRT draw, no private RNG.
inline CarContactVector deform_car_vertex(const CarContactVector& original,const CarContactVector& current,
        const CarContactVector& direction,const CarContactVector& limits,float ratio,int draw){
    const auto factor=(static_cast<long double>(draw%10)*.10000000149011612f)*.30000001192092896f+.699999988079071f;
    const float x=static_cast<float>((direction[0]*factor)*ratio+current[0]);
    const float y_step=static_cast<float>(factor*direction[1]);
    const float z_step=static_cast<float>(factor*direction[2]);
    const float z_scaled=static_cast<float>(static_cast<long double>(z_step)*ratio);
    const float z=static_cast<float>(static_cast<long double>(z_scaled)+current[2]);
    const float dx=x-original[0];
    const float dy=static_cast<float>((static_cast<long double>(y_step)*ratio+current[1])-original[1]);
    auto result=current;
    result[0]=static_cast<float>(original[0]+clamp_car_deformation(dx,limits[0]));
    result[1]=static_cast<float>(original[1]+clamp_car_deformation(dy,limits[1]));
    result[2]=static_cast<float>(original[2]+clamp_car_deformation(static_cast<long double>(z)-original[2],limits[2]));
    return result;
}
} //namespace hd2::game
#endif
