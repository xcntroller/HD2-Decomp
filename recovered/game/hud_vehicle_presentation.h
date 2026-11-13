#ifndef HD2_GAME_HUD_VEHICLE_PRESENTATION_H
#define HD2_GAME_HUD_VEHICLE_PRESENTATION_H
#include <stdexcept>
namespace hd2::game {
//0049BD50's vehicle tail. Read actual Car state only after all modal gates;
//this is presentation, never a second Car update or an occupied-seat producer.
template<class Artwork,class Queries>void advance_hud_vehicle(Artwork& artwork,Queries& queries){
    if((queries.hud_flags()&0x20u)||queries.command_visible()||queries.inventory_visible()||queries.roll_visible()){artwork.show_vehicle(false);return;}
    auto* human=queries.current_human();if(!human)throw std::logic_error("Vehicle HUD requires actual current Human");
    auto* attached=queries.attachment(*human);
    if(!attached||queries.kind(*attached)!=4){artwork.show_vehicle(false);return;}
    artwork.show_vehicle(true);
    const float fuel=queries.fuel_ratio(*attached);
    const float speed=static_cast<float>(static_cast<long double>(queries.car_speed(*attached))*3.5999999046325684f);
    const float rpm=queries.car_rpm(*attached);
    artwork.update_vehicle(speed,rpm,fuel);
}
} //namespace hd2::game
#endif
