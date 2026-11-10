#ifndef HD2_HOST_GAME_LOADING_MODULES_H
#define HD2_HOST_GAME_LOADING_MODULES_H

#include "frontend/module_registration.h"
#include "game/mission_module_dispatch.h"
#include "game/offline_spawn_presentation.h"
#include <cstring>

namespace hd2::host {

//Typed binding of the eighteen ORIGINAL startup registrations to the SAME
//world's actual owners. This supplies neither a world constructor nor default
//services. The containing session retains all borrowers through both kinds of
//retirement. Replacement registrations and slot18 have no successful fallback.
template<class Dependencies>class GameLoadingModules {
public:
    explicit GameLoadingModules(Dependencies& dependencies):d_(dependencies){}
    auto& module_registry(){return d_.startup.game_module_registry();}
    std::uint32_t mode_flags()const{return d_.startup.application_flags();}
    template<class FrameServices>void tick_modules(bool authority,std::uint32_t elapsed,FrameServices& frames) {
        //A reference-only adapter supplies the mandatory positive frame owners.
        //No module can fall through to a successful generic frame handler.
        struct Pass {
            GameLoadingModules& owner;FrameServices& frames;
            void tick_mission_module(frontend::ClientModuleRegistration& module,bool authority,std::uint32_t elapsed){
                owner.tick_mission_module(module,authority,elapsed,frames);
            }
        } pass{*this,frames};
        game::tick_mission_module_pass(module_registry(),pass,authority,elapsed);
    }
    template<class FrameServices>void tick_mission_module(frontend::ClientModuleRegistration& module,bool authority,
            std::uint32_t elapsed,FrameServices& frames) { //Original module+0C/+10 targets.
        switch(checked_kind(module)) {
        case 0:bound(module,d_.tracks);if(!authority)d_.tracks.update(d_.session.status,elapsed);return;
        case 1:bound(module,d_.game_effects);if(!authority)d_.game_effects.update_loading(elapsed,d_.session,d_.players.control());return;
        case 2:frames.tick_physics(authority,elapsed);return;
        case 3:d_.presentation_shared.tick_item_projections(authority,elapsed);return;
        case 4:bound(module,d_.scripts.module());
            if(authority)d_.scripts.module().tick(elapsed);else d_.scripts.module().frame_update(signed_elapsed(elapsed));return;
        case 5:if(authority)d_.sight.tick(elapsed,frames);return;
        case 6:if(!authority)d_.startup.update_sound(elapsed);return; //00436820 RET4/00436830→004368D0.
        case 7:bound(module,d_.versions);return; //Both original RET4.
        case 8:bound(module,d_.lightmap_groups);return; //Both original RET4.
        case 9:if(authority)d_.vertices.update(signed_elapsed(elapsed));else d_.vertices.render_update(signed_elapsed(elapsed));return;
        case 10:bound(module,d_.texture_shifts);if(!authority)d_.texture_shifts.update(elapsed);return;
        case 11:bound(module,d_.players);
            if(authority)d_.players.tick(elapsed,d_.session.words.template get<std::uint32_t>(0x28),d_.startup.delayed_player_speech(),d_.human_services);return;
        case 12:frames.tick_runtime_actors(authority,elapsed);return;
        case 13:frames.tick_scene_actors(authority,elapsed);return;
        case 14:bound(module,d_.shots);d_.shots.tick(authority,elapsed);return;
        case 15:if(!authority)frames.tick_camera(elapsed);return;
        case 16:bound(module,d_.heavy_fire);d_.heavy_fire.update(authority,elapsed);return;
        case 17:bound(module,d_.particles);if(!authority)d_.particles.update(elapsed);return; //004455A0 RET4.
        }
        throw std::logic_error("Mission frame module has no actual receiver");
    }
    void finish_loading_modules(){game::finish_mission_modules(module_registry(),*this,d_.startup.application_flags_owner());}
    void activate_modules(bool authority){game::activate_mission_module_pass(module_registry(),*this,authority);}
    void deactivate_modules(bool authority){game::deactivate_mission_module_pass(module_registry(),*this,authority);}
    void unload_modules(bool authority){game::unload_mission_module_pass(module_registry(),*this,authority);}
    void finish_mission_module(frontend::ClientModuleRegistration& module,bool authority) { //virtual14.
        switch(checked_kind(module)) {
        case 0:bound(module,d_.tracks);d_.tracks.finish_loading(authority,files(),resource());return;
        case 1:bound(module,d_.game_effects);d_.game_effects.finish_loading(authority);return;
        case 2:return; //004106E0 RET4.
        case 3:d_.world_actors.items().finish_loading(authority);return;
        case 4:bound(module,d_.scripts.module());return; //004285C0 RET4.
        case 5:if(authority)d_.sight.source().clear();return; //0046EEE0.
        case 6:d_.startup.finish_game_sound_loading(authority);return;
        case 7:bound(module,d_.versions);d_.versions.finish_loading(authority);return;
        case 8:bound(module,d_.lightmap_groups);d_.lightmap_groups.finish_loading(authority);return;
        case 9:d_.vertices.finish_loading(authority,files(),resource());return;
        case 10:bound(module,d_.texture_shifts);d_.texture_shifts.finish_loading(authority);return;
        case 11:bound(module,d_.players);return; //004115E0 RET4.
        case 12:return; //00476FA0 RET4.
        case 13:return; //00475750 RET4.
        case 14:bound(module,d_.shots);d_.shots.finish_loading(authority);return;
        case 15:return; //004115E0 RET4.
        case 16:bound(module,d_.heavy_fire);d_.heavy_fire.finish_loading(authority);return;
        case 17:bound(module,d_.particles);d_.particles.finish_loading(authority);return;
        }
        throw std::logic_error("Mission loading module has no actual receiver");
    }
    void activate_mission_module(frontend::ClientModuleRegistration& module,bool authority) { //virtual1C.
        switch(checked_kind(module)) {
        case 0:bound(module,d_.tracks);d_.tracks.activate(authority);return;
        case 1:bound(module,d_.game_effects);d_.game_effects.activate(authority);return;
        case 2:d_.world_actors.physics().activate(authority);return;
        case 3:d_.world_actors.items().activate(authority,resource());return;
        case 4:
            bound(module,d_.scripts.module());
            d_.scripts.module().activate(authority,d_.request.preparation.standalone?1u:0u);
            if(!d_.scripts.module().failed_bindings().empty())throw std::runtime_error("Required mission script bindings failed");
            return;
        case 5:d_.sight.source().initialize_loading(authority);return;
        case 6:d_.startup.activate_game_sound(authority);return;
        case 7:bound(module,d_.versions);d_.versions.activate(authority);return;
        case 8:bound(module,d_.lightmap_groups);d_.lightmap_groups.activate(authority);return;
        case 9:d_.vertices.activate();return;
        case 10:bound(module,d_.texture_shifts);d_.texture_shifts.activate(authority);return;
        case 11:bound(module,d_.players);d_.players.activate(authority);return;
        case 12:d_.world_actors.runtime().activate(authority);return;
        case 13:d_.scene.frames().activate_scene_actors(authority,d_.world_actors,d_.scene.buckets());return;
        case 14:bound(module,d_.shots);d_.shots.activate(authority);return;
        case 15:{
            const auto& fields=d_.scene.documents().fields();
            d_.camera.activate(authority,files(),fields.field_of_view,fields.near_clip,fields.far_clip);return;
        }
        case 16:bound(module,d_.heavy_fire);d_.heavy_fire.activate(authority);return;
        case 17:bound(module,d_.particles);d_.particles.activate(authority);return;
        }
        throw std::logic_error("Mission activation module has no actual receiver");
    }
    void deactivate_mission_module(frontend::ClientModuleRegistration& module,bool authority) { //virtual20, reverse pass.
        switch(checked_kind(module)) {
        case 0:bound(module,d_.tracks);d_.tracks.finish(authority);return;
        case 1:bound(module,d_.game_effects);d_.game_effects.finish(authority);return;
        case 2:if(authority||!(mode_flags()&1))d_.world_actors.physics().clear();return; //00410A70.
        case 3:d_.world_actors.items().finish(authority,d_.scene.frames(),d_.startup.item_hand_models());return;
        case 4:bound(module,d_.scripts.module());d_.scripts.module().finish(authority);return;
        case 5:d_.sight.source().close_loading(authority);return;
        case 6:d_.startup.finish_game_sound(authority);return;
        case 7:bound(module,d_.versions);d_.versions.finish(authority);return;
        case 8:bound(module,d_.lightmap_groups);d_.lightmap_groups.finish(authority);return;
        case 9:d_.vertices.deactivate();return;
        case 10:bound(module,d_.texture_shifts);d_.texture_shifts.finish(authority);return;
        case 11:bound(module,d_.players);d_.players.finish(authority);return;
        case 12:d_.world_actors.runtime().finish(authority);return;
        case 13:d_.scene.frames().deactivate_scene_actors(authority,d_.world_actors,d_.scene.buckets());return;
        case 14:bound(module,d_.shots);d_.shots.finish(authority);return;
        case 15:d_.camera.deactivate(authority);return;
        case 16:bound(module,d_.heavy_fire);d_.heavy_fire.finish(authority);return;
        case 17:bound(module,d_.particles);d_.particles.finish(authority);return;
        }
        throw std::logic_error("Mission finish module has no actual receiver");
    }
    void unload_mission_module(frontend::ClientModuleRegistration& module,bool authority) { //virtual18, ascending pass.
        switch(checked_kind(module)) {
        case 0:bound(module,d_.tracks);d_.tracks.unload(authority);return;
        case 1:bound(module,d_.game_effects);d_.game_effects.unload(authority);return;
        case 2:return; //004106F0 RET4.
        case 3:d_.world_actors.items().unload();return; //007DCFB0 ignores authority.
        case 4:bound(module,d_.scripts.module());d_.scripts.module().unload(authority);return;
        case 5:d_.sight.source().close_loading(authority);return; //0046EEF0, same authority clear.
        case 6:d_.startup.unload_game_sound(authority);return;
        case 7:return; //0043DCA0 RET4.
        case 8:bound(module,d_.lightmap_groups);d_.lightmap_groups.unload(authority);return;
        case 9:d_.vertices.release_loading(authority);return;
        case 10:bound(module,d_.texture_shifts);d_.texture_shifts.unload(authority);return;
        case 11:return; //004115F0 RET4.
        case 12:d_.world_actors.runtime().clear();return; //00476FB0→00477430, unconditional.
        case 13:d_.scene.frames().clear_scene_actors();return; //00475760→00475FB0, unconditional.
        case 14:case 15:case 16:return; //004115F0 RET4.
        case 17:return; //00444130 RET4.
        }
        throw std::logic_error("Mission file-unload module has no actual receiver");
    }
    bool dispatch_packet(std::uint32_t kind,game::SessionPacketStream& packet,bool authority){ //0048DC50/70, fresh registration.
        if(kind>=module_registry().size())return false;
        auto* module=module_registry()[kind];if(!module)return false;
        (void)checked_kind(*module);
        if(kind==13&&!authority)return game::receive_scene2_presentation_packet(d_.scene.frames(),packet,d_.world_actors,*this);
        throw std::runtime_error("Required actual authority/presentation module packet receiver is unavailable");
    }
    template<class Actor>void receive_player_packet(Actor*,game::SessionPacketStream&){
        throw std::runtime_error("Reserved Scene2 packet requires actual005A5FB0/005418E0 player-state receiver");
    }
private:
    static std::int32_t signed_elapsed(std::uint32_t elapsed){std::int32_t value;std::memcpy(&value,&elapsed,4);return value;}
    std::uint32_t checked_kind(frontend::ClientModuleRegistration& module) {
        const auto kind=module.kind();
        if(&d_.startup.game_module_registration(kind)!=&module)
            throw std::logic_error("Mission module receiver requires its original startup registration");
        return kind;
    }
    template<class Owner>static void bound(frontend::ClientModuleRegistration& registration,Owner& owner) {
        if(&owner.registration()!=&registration)throw std::logic_error("Mission module owner belongs to another registration");
    }
    auto& files(){return d_.startup.pre_mission_files();}
    const std::string& resource()const{return d_.request.preparation.mission_resource;}
    Dependencies& d_;
};
} //namespace hd2::host
#endif
