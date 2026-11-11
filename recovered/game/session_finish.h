#ifndef HD2_GAME_SESSION_FINISH_H
#define HD2_GAME_SESSION_FINISH_H
#include "game/session_activation.h"
#include "frontend/cleanup.h"
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

//Source rollback/retry guard around00698F80. Native finish has no prefix
//journal. Only entered receivers are retired here; a throwing receiver may have
//mutated its owner before returning. Normal successful ordering is unchanged.
//A failed mandatory receiver stops dependent retirement. The graph remains
//alive for retry; shared resources cannot be freed under unfinished consumers.
template<class Services>
bool finish_offline_session_prefix(SessionActivationState& state,SessionActivationProgress& p,Services& services){
    if(p.finished)return true;
    if((services.mode_flags()&7u)!=7u)throw std::runtime_error("Session prefix finish requires actual offline owners");
    frontend::CleanupErrors errors;
    if(p.spawn.entered&&!p.snapshot_taken){
        p.snapshot_taken=true; //Never overwrite the live squad again after Human removal, even on snapshot failure.
        errors.attempt([&]{services.snapshot_live_squad();});
    }
    errors.attempt([&]{ //Snapshot failure is preserved while actual ownership retirement still runs.
        p.first_person.retire([&]{services.close_first_person();});
        p.lights.retire([&]{services.mission_light_visibility().restore();});
        auto modules=[&](bool authority){
            auto& registry=services.module_registry();auto& steps=authority?p.authority:p.presentation;
            if(registry.size()!=steps.size())throw std::logic_error("Session finish registry changed size");
            for(auto n=registry.size();n;){--n;
                steps[n].retire([&]{
                    auto* module=registry[n];
                    if(!module)throw std::logic_error("Entered session module lost its mandatory retirement receiver");
                    services.deactivate_mission_module(*module,authority);
                });
            }
        };
        modules(false);
        p.dubbing.retire([&]{services.finish_loaded_dubbing();});
        modules(true);
        p.hud.retire([&]{services.close_loaded_hud();});
        auto tail=[&](std::size_t n,auto&& work){if(!p.tail[n]){work();p.tail[n]=true;}};
        //The resource prefix starts only after the real loader succeeded. A cold
        //load failure instead executes file retirement, not this activated tail.
        if(p.resources.entered){
            tail(0,[&]{(void)services.query_finish_focus();});
            tail(1,[&]{services.release_finished_collision_tree();});
            tail(2,[&]{services.clear_car_definition_tables();});
            tail(3,[&]{if(state.status==1)state.status=6;});
            tail(4,[&]{services.clear_shared_finish_resources();});
            tail(5,[&]{services.clear_loaded_mission_text();});
            tail(6,[&]{services.reset_navigation_runtime_links();});
            tail(7,[&]{services.clear_car_particle_pool();});
            tail(8,[&]{services.clear_human_face_resources();});
        }
        p.finished=true;
    });
    errors.rethrow();return true;
}
} //namespace hd2::game
#endif
