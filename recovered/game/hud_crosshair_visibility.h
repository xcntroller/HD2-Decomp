#ifndef HD2_GAME_HUD_CROSSHAIR_VISIBILITY_H
#define HD2_GAME_HUD_CROSSHAIR_VISIBILITY_H
#include <cstdint>
#include <stdexcept>
namespace hd2::game {
//0049D530, separate from spread004AA840 and target/name004A9F50. Queries
//borrow real command/camera/Human/Items owners; no simulation is performed.
template<class Crosshair,class Queries>void update_hud_crosshair_visibility(std::uint32_t flags,Crosshair& crosshair,Queries& queries){
    if(flags&0x180u){queries.clear_target();crosshair.show(0);return;}
    if(queries.command_visible()||queries.camera_suppressed()){crosshair.show(0);return;}
    auto* human=queries.current_player();if(!human)throw std::logic_error("Crosshair visibility requires actual current Human");
    if(auto* attached=queries.attachment(*human))switch(queries.kind(*attached)){
    case 4:{
        const auto seat=queries.seat_flags(*human,*attached);
        if(seat&0x100u){crosshair.show(!queries.seat_hides_cursor(*human,*attached)&&!queries.first_person_enabled());return;}
        if(!(seat&0x20u)){crosshair.show(0);return;}
        break;
    }
    case 21:{
        auto visible=!queries.first_person_enabled(); //Native0069A4B0 is queried even if virtualCC then suppresses.
        if(queries.seat_hides_cursor(*human,*attached))visible=false;
        crosshair.show(visible);return;
    }
    case 34:crosshair.show(0);return;
    default:break;
    }
    if(queries.weapon_suppresses(*human)){crosshair.show(0);return;}
    const auto mode=queries.player_mode(*human);
    crosshair.show(!(mode>=1&&mode<=3));
}
} //namespace hd2::game
#endif
