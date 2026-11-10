#ifndef HD2_GAME_SESSION_FINISH_H
#define HD2_GAME_SESSION_FINISH_H
#include "game/session_activation.h"
namespace hd2::game {

//00698F80. Not the file unload0069A3D0/destructor or a reordered VM finish.
//Services must supply real writeback, overlay/light/module/dubbing/HUD and
//process tail receivers. No successful missing finalizer/default service here.
template<class Services>
bool finish_offline_session(SessionActivationState& state,Services& services){
    if((services.mode_flags()&7u)!=7u)throw std::runtime_error("Session finish requires actual nonoffline squad owner cleanup");
    services.snapshot_live_squad(); //Before overlay, lights or any Human removal.
    if(services.mode_flags()&2u)services.close_first_person();
    services.mission_light_visibility().restore();
    if(services.mode_flags()&2u){
        deactivate_mission_module_pass(services.module_registry(),services,false); //Fresh REVERSE.
        services.finish_loaded_dubbing(); //0071DE60; don't neutral dangling/retired morph borrowers.
    }
    if(services.mode_flags()&1u)deactivate_mission_module_pass(services.module_registry(),services,true);
    if(services.mode_flags()&2u)services.close_loaded_hud();
    (void)services.query_finish_focus(); //00699112→actual GetFocus, not private table equality.
    services.release_finished_collision_tree(); //00718380(0), after descriptor/module retirement.
    services.clear_car_definition_tables(); //Actual0097B8A4 then0097B89C definitions, not live emitters.
    if(state.status==1)state.status=6;
    services.clear_shared_finish_resources(); //006DF390(0,true) still a REQUIRED actual process owner.
    services.clear_loaded_mission_text();
    services.reset_navigation_runtime_links(); //00523E30→0051F230 first dataset.
    services.clear_car_particle_pool(); //004ED0C0, AFTER serialized definition arrays.
    services.clear_human_face_resources(); //007967A0, after every actual face borrower.
    return true;
}
} //namespace hd2::game
#endif
