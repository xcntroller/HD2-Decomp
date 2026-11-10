#ifndef HD2_GAME_CAR_WATER_LOADING_H
#define HD2_GAME_CAR_WATER_LOADING_H

#include "game/car_health_reset.h"
#include <limits>

namespace hd2::game {

//004D2C50's offline path. Bounds and the upward100-unit water query come
//from the actual shared collision tree. Occupants are the SAME FA8+30 owners;
//positive Human damage uses its installed receiver, never an empty service.
template<class Frames,class Queries,class Occupants,class Services>void update_car_loading_water(
        Frames& frames,typename Frames::Frame model,Queries& queries,CarHealthState& health,
        const Occupants& occupants,Services& services,std::uint32_t mode){
    if((mode&7u)!=7u)throw std::runtime_error("Car water loading requires actual non7 notification owners");
    const auto limits=queries.water_bounds();if(limits.maximum==-std::numeric_limits<float>::max())return;
    const auto world=frames.actor_world_matrix(model);
    if(!(world[13]<limits.maximum&&health.body>0))return;
    //006FDA00's SECOND output is the upper surface, used by004D2C50.
    const typename Frames::Vector position{{world[12],world[13],world[14],world[15]}},up{{0,100,0,0}},zero{};
    if(!queries.water_segment(position,up,0))return;
    health.body=0;
    for(auto n=occupants.size();n;){const auto victim=occupants[--n];if(victim)
        services.car_human_impact(victim,10,1000.f,zero,zero,typename Occupants::value_type{},0);
    }
}

} //namespace hd2::game
#endif
