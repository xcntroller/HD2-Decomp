#ifndef HD2_GAME_SESSION_ACTIVATION_H
#define HD2_GAME_SESSION_ACTIVATION_H

#include "game/activation_flags.h"
#include "game/mission_module_dispatch.h"
#include "game/human_joint_owners.h"
#include "frontend/game_data.h"

namespace hd2::game {

//Known game fields used by006989B0. DescriptorA8 stays a typed borrower of the
//owned request. Loaded14 must be written by the SAME006CA780 stage completion;
//neither construction nor activation policy fabricates that success.
struct SessionActivationState {
    SessionActivationState(){words.set(0x1c,std::uint32_t{0});} //00698510, normal session packet kind; separate from status04/configuration04.
    std::uint32_t status=0;bool loaded=false;
    const frontend::GameDataNode* mission=nullptr;
    HumanJointWords<0x110> words; //Scalar slots only; native pointers are separate.
};

//006989B0 normal offline7 path. All calls below require actual owners, including
//module registration/loader/VM/HUD and spawning; no fallback endpoint is supplied.
//Nonoffline descriptor/transport and protection-wrapper ABI remain separate.
//This source policy does not itself install a live world or enable the host UI.
template<class Services>
bool activate_offline_session(SessionActivationState& state,Services& services,
                              const frontend::GameDataNode* mission) {
    if(state.status==5)return false; //Original returns0 even for a previously active owner.
    if((services.mode_flags()&7u)!=7u)throw std::runtime_error("Session activation requires its actual nonoffline descriptor/transport consumers");
    const auto root=services.application_data().field_e8;
    const auto ordinal=services.selected_mission_ordinal(root);
    state.words.set(0x3c,root?static_cast<std::uint32_t>(services.application_data().mission_count(0))+ordinal:ordinal);
    state.words.set(0x34,std::uint8_t{1});state.words.set(0x35,std::uint8_t{0});state.words.set(0x36,std::uint8_t{1});
    for(auto offset:{0x28u,0x2cu,0x30u})state.words.set(offset,std::uint32_t{0});
    state.mission=nullptr;state.words.set(0x40,std::uint8_t{0});
    return with_activation_flags(services,[&] {
        if(mission) {
            if(!services.load_mission(mission->text24,mission))return false;
            if(!state.loaded)throw std::runtime_error("Mission loader returned without publishing its actual loaded stages");
            state.mission=mission;
        }else if(!state.loaded) {
            if(!services.load_mission(services.current_mission_resource(),nullptr))return false;
            if(!state.loaded)throw std::runtime_error("Mission loader returned without publishing its actual loaded stages");
        }
        (void)services.query_activation_ticks(); //00698AAE→real GetTickCount; private table effects are separate.
        services.loading_progress(5,.05f);
        state.words.set(0x6e,std::uint8_t{1});state.words.set(0x6f,std::uint8_t{0});
        (void)services.car_definitions().load(services.files()); //0071F9B0 original result ignored.
        (void)services.car_tyres().load(services.files()); //004D6900 original result ignored.
        services.car_materials().rebuild(services.car_definitions().materials(),services); //004D57A0.
        (void)services.car_particles().initialize(services.car_definitions().particles()); //004D5A50.
        services.rebuild_session_world_bounds(); //0072D2B0(3,0,0), SAME input/session owner.
        services.loading_progress(5,.1f);
        services.mission_light_visibility().collect(); //0069BFF0(false), SAME loaded scene.
        if((services.mode_flags()&7u)==7u&&services.human_face_loading_enabled())services.human_faces().initialize(); //006B6740→006B33E0/00796430.
        if(services.mode_flags()&1u)activate_mission_module_pass(services.module_registry(),services,true);
        services.loading_progress(5,.35f);
        if(services.mode_flags()&2u) {
            services.initialize_loaded_hud(); //0049A940 BEFORE presentation module pass.
            activate_mission_module_pass(services.module_registry(),services,false);
        }
        services.loading_progress(5,.5f);
        if(services.mode_flags()&2u)services.initialize_loaded_first_person(); //004909D0.
        services.loading_progress(5,.8f);
        (void)services.query_activation_visible_window(); //00698C09→actual GetIGraph/virtual48/IsWindowVisible.
        (void)services.query_activation_capture(); //00698C5D→GetCapture, before status5/spawning.
        state.status=5;state.words.set(0x20,std::uint32_t{500});state.words.set(0x24,std::uint32_t{0});
        if((services.mode_flags()&7u)==7u) {
            //008AE910 is the LIVE SQUAD, distinct from008AE90C HumanStore.
            //Use the owned request's actual prepared records, rereading count
            //and entry after every spawn callback.
            for(std::size_t n=0;n<services.live_squad().size();++n) {
                auto& soldier=services.live_squad().at(n);
                if(!soldier.active)continue; //Native live-soldier44.
                auto* spawn=services.loading_spawn_point(0);if(!spawn)break; //00475940, queried anew for each live record.
                services.spawn_prepared_human(*spawn,soldier); //00611C90(0,record,0), consumes SAME prepared equipment.
            }
        }
        if(services.mode_flags()&2u)services.initialize_loaded_dubbing(); //0071DE10 known zero state, not generic stop.
        state.words.set(0x64,std::uint8_t{0});services.loading_progress(5,.9f);
        if(services.has_mission_text_definition())services.initialize_loaded_mission_text(); //0048EB70(actual6A+8).
        for(auto offset:{0x74u,0x8cu,0x90u,0x7cu,0x84u})state.words.set(offset,std::uint32_t{0});
        for(auto offset:{0x70u,0x71u,0x94u,0x80u})state.words.set(offset,std::uint8_t{0});
        state.words.set(0x78,std::int32_t{-1});state.words.set(0x88,std::uint32_t{0});
        state.words.set(0xb8,std::uint32_t{1});state.words.set(0xb4,std::int32_t{-1});
        if((services.mode_flags()&7u)!=7u)throw std::runtime_error("Session activation mode changed into unrecovered nonoffline finalization");
        auto& music=services.game_music();music.countdown=music.references=0;
        state.words.set(0x38,std::uint32_t{2500});music.select(false,services.mode_flags(),services);
        services.reset_loading_clock(); //006C95C0 after final loading progress.
        if((services.mode_flags()&7u)==7u&&services.has_application_game_context())
            state.words.set(0x0c,static_cast<std::uint8_t>(services.restore_session_context())); //006DC940→006C4BD0.
        return true;
    });
}
} //namespace hd2::game
#endif
