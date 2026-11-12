#ifndef HD2_GAME_HUD_CAR_OCCUPANTS_H
#define HD2_GAME_HUD_CAR_OCCUPANTS_H
#include <cstdint>
namespace hd2::game {
//004DDE20: shared Car occupant query reached by crosshair004A9F50 and command
//roll004B1EE0. Its stack Human is the OFFLINE observer/ECX of005A0980. Reverse
//seats, friendship BEFORE health offline; health/controlled/suppressed/team order
//nonoffline. No Car or Human is retained and no simulation is advanced.
template<class Actor,class Queries>bool hud_car_hostile_occupants(Actor& car,Actor& observer,std::uint32_t mode,Queries& queries){
    for(auto n=queries.occupant_count(car);n;){--n;if(auto* occupant=queries.occupant(car,n)){
        if((mode&7u)==7u){
            if(queries.hostile(observer,*occupant)&&queries.health(*occupant)>0)return true;
        }else if(queries.health(*occupant)>0&&queries.car_player_controlled(*occupant)&&!queries.car_player_suppressed(*occupant)){
            const auto first=queries.car_human_team(*occupant),second=queries.car_human_team(observer);
            if(first!=second)return true;
        }
    }}
    return false;
}
} //namespace hd2::game
#endif
