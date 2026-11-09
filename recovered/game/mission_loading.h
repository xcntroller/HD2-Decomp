#ifndef HD2_GAME_MISSION_LOADING_H
#define HD2_GAME_MISSION_LOADING_H

#include "frontend/cleanup.h"
#include <string>
#include <stdexcept>

namespace hd2::game {

//006CA780 ordered body. Services are required real consumers of the selected
//descriptor; this does not supply default/successful implementations of missing
//Scene2, actor, collision, effects or loading presentation systems.
//
//The Scene2 status is checked AFTER actor/sound/check loading, exactly as in the
//original. Native exceptions still unwind acquired scratch owners and run the
//module loading tail; the caller owns actual loaded-world rollback.
template<class Services, class Mission>
bool load_mission(Services& services, const std::string& resource, const Mission* mission) {
    typename Services::ScratchOwner scratch;
    frontend::CleanupErrors errors;
    bool loaded = false;
    try {
        services.reset_loading_fields(); //+66/+68/+D8.
        scratch = services.make_loading_scratch(); //006CB280 then006CB0E0.
        if (!scratch) throw std::runtime_error("Mission loader requires its scratch owners");
        services.prepare_loading(resource, mission); //006D3F20, includes prior unload.
        if (services.load_animation_tables()) { //00783350/00783BC0.
            services.open_scene(resource); //006D4180, real engine scene+F4.
            services.refresh_scene(); //Scene+F0.
            const auto scene2_status = services.load_scene2(resource, *scratch); //006CBF30.
            services.query_parent_window(); //Resolved006CA913→GetParent.
            services.load_actors(resource, *scratch); //006D45C0, stage2.
            services.load_sounds(resource, *scratch); //006D4880, stage3.
            services.load_check_tables(resource); //006D4280.
            services.loading_progress(4, .3f);
            //006D4B50 is a RET; there is no missing successful service here.
            if (scene2_status == 0) {
                services.load_collision_tree(resource, true); //006EE570, tree.klz.
                services.loading_progress(4, .8f);
                services.publish_effects_scene(); //0089A8F4 receives actual scene.
                services.load_effects(resource); //00445D90.
                services.loading_progress(4, 1.f);
                services.mark_mission_loaded(); //+14=1 only after all required stages.
                //006D4B40 is a RET.
                services.finish_loading_screen(); //006C95B0.
                services.query_visible_window(); //Resolved006CAAC0→IsWindowVisible.
                services.query_iconic_window(); //Resolved006CAB29→IsIconic.
                loaded = true;
            }
        }
    } catch (...) { errors = frontend::CleanupErrors(std::current_exception()); }
    //Locals' reverse-order release, then006CADE0's actual module tail. No
    //callback/name-map owner can survive the stack scope by borrowing a local.
    scratch.reset();
    errors.attempt([&] { services.finish_loading_modules(); });
    errors.rethrow();
    return loaded; //Native true corresponds to original006CA780's return0.
}

} // namespace hd2::game
#endif
