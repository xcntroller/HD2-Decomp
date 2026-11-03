#ifndef HD2_GAME_SCENE_CALLBACK_H
#define HD2_GAME_SCENE_CALLBACK_H

#include <cstdint>

namespace hd2::game {

//006C9260. All service operations are required; there are no successful default
// renderers. This is the client dispatch body, not implementations of its HUD,
// control/debug/network/effects callees. The engine ABI is stdcall(stage,word,word)
// with RET12; the last two arguments are unused by the original client body.
// Services reread the live owner/flags on each query, as the original does.
template<class Services>
std::uint32_t scene_callback(std::uint32_t stage, Services& services) {
    if (services.has_game() && services.game_status() == 14) return 0;
    if (stage == 1) {
        if (!services.suppress_hud() && services.hud_enabled()) services.before_hud(); //0049B9B0
        services.control_before_scene(); //00486510 virtual+A4
        if (services.debug_enabled()) services.render_debug(); //0079B760
    } else if (stage == 2) {
        if (!services.full_viewport()) {
            if (!services.suppress_hud() && services.hud_enabled()) services.render_hud(); //0049B9F0
        } else {
            // Original copies six DWORDs, preserving the near/far planes.
            auto viewport = services.graphics_viewport();
            viewport.x = 0;
            viewport.y = 0;
            viewport.width = services.graphics_width();
            viewport.height = services.graphics_height();
            services.graphics_viewport(viewport);
        }
        services.render_mission_text(); //0048F390
        services.render_network_overlay(); //00724CB0; its own mode gate is not moved here.
        if (services.game_word24() == 0 &&
            (services.game_status() == 5 || services.game_status() == 6))
            services.render_game_fade(); //0069AE10
        services.control_after_scene(); //00486510 virtual+A0
        services.render_effects_overlay(); //006E8490
    }
    return 0;
}

} // namespace hd2::game
#endif
