#ifndef HD2_GAME_SESSION_RENDER_H
#define HD2_GAME_SESSION_RENDER_H
#include <cstdint>
namespace hd2::game {
//0069B870 (ABSENT frozen inventory): offline7 always renders actual scene18;
//nonoffline mode consults actual configuration24.flag170 first. Engine/scene
//callback bodies remain required, never a successful substitute renderer.
template<class Services>void render_session(Services& services){
    if((services.mode_flags()&7u)==7u||!services.external_loading_progress())services.render_actual_mission_scene();
}
} //namespace hd2::game
#endif
