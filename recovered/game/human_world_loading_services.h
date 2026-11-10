#ifndef HD2_GAME_HUMAN_WORLD_LOADING_SERVICES_H
#define HD2_GAME_HUMAN_WORLD_LOADING_SERVICES_H

#include "game/human_camera_state.h"
#include "frontend/spatial_sound.h"
#include <stdexcept>

namespace hd2::game {

//Concrete shared loading consumers for DD0/230, B0 and Human activation.
//Dependencies owns/borrows the SAME startup, session, scene, world_actors,
//players, first_person, faces, animations, presentation_shared, music and sound.
//Composition is resolved only when a callback runs, permitting the containing
//owner's mutually borrowing types to be declared before construction. It must
//publish actual WorldActors lifetimes before activation; no secondary map.
template<class Dependencies>class HumanWorldLoadingServices {
public:
    explicit HumanWorldLoadingServices(Dependencies& dependencies):dependencies_(dependencies){}
    std::uint32_t mode_flags()const{return dependencies_.startup.application_flags();}
    std::int32_t human_current_player_id()const{return dependencies_.startup.local_player_identity();}
    bool human_remote_ai_enabled(){return dependencies_.startup.human_remote_ai_enabled();}
    auto human_difficulty(){return dependencies_.startup.human_difficulty();}
    std::uint32_t game_word28()const{return dependencies_.session.words.template get<std::uint32_t>(0x28);}
    std::uint32_t game_frame_serial()const{return dependencies_.session.words.template get<std::uint32_t>(0x2c);}
    auto& human_ai_event_pool(){return dependencies_.startup.human_ai_event_pool();}
    auto& human_perception_record_pool(){return dependencies_.startup.human_perception_record_pool();}
    bool human_face_quality_enabled(){return dependencies_.startup.human_face_quality_enabled();}
    auto& human_face_resources(){return dependencies_.faces;}
    std::int32_t human_face_camouflage(){return static_cast<std::int32_t>(dependencies_.presentation_shared.settings().terrain);} //Actual0098A6B0.
    auto human_animation_resource(unsigned table,unsigned row){return dependencies_.animations.resource(table,row);}
    template<class Human>auto& human_look_control(Human& human){
        auto* look=composition(human).control().look();
        if(!look)throw std::runtime_error("Human animation requires its actual inline E0");
        return *look;
    }
    template<class Human,class Shadow>void refresh_human_equipment_visibility(Human& human,Shadow& shadow){
        auto& control=composition(human).control();
        if(!control.control()||&control.control()->shadow()!=&shadow||!control.equipment())
            throw std::runtime_error("Human visibility requires the SAME DD0 shadow and230 owners");
        control.equipment()->refresh(human.base().frame());
    }
    template<class Human>bool human_is_player(const Human& human)const{return human_camera_is_player(human,mode_flags(),human_current_player_id());} //005A0890, AL.
    template<class Human>bool human_is_view_actor(const Human& human)const{return human_is_player(human);}
    bool human_view_offset_enabled(){auto& view=dependencies_.first_person.view();return view.enabled()||view.secondary_enabled();} //0069A4B0, actual58 OR59.
    template<class Human>void player_select_human(Human& human){dependencies_.players.control().select_human(human);} //00461410→0045DA60.
    template<class Human>void remove_human_player(Human& human){(void)dependencies_.players.control().remove_human(&human);} //0045E670.
    template<class Human>bool player_knows_enemy(Human& human){return dependencies_.players.control().selection().knows(&human);} //0045E920.
    template<class Human>bool human_unavailable(const Human& human)const { //005A6380, only its native AL result.
        std::int32_t health;const auto bits=human.properties.stats_ec[0];std::memcpy(&health,&bits,4);
        if(health<1||!human.loading_state||human.properties.field19c==1)return false;
        return (human.loading_state->words().template get<std::uint32_t>(0x50)&0x4000000u)!=0;
    }
    template<class Actor>auto actor_inventory_capacity(const Actor& actor){return dependencies_.scene.frames().actor_inventory_capacity(actor);}
    template<class Actor>void set_scene_actor_active(Actor& actor,bool enabled){dependencies_.scene.frames().set_scene_actor_active(actor,enabled);}
    template<class Actor>void set_runtime_actor_active(Actor& actor,bool enabled){dependencies_.world_actors.runtime().set_active(actor,enabled);}
    void human_special_music_reference(bool enabled){dependencies_.music.reference(enabled,mode_flags(),dependencies_.startup);} //0069E920/70, SAME gameC8..D4/SOUND.
    std::int32_t emit_human_equipment_sound(const frontend::SpatialSoundRequest& request){return dependencies_.sound.emit(request);}
    void record_state_cleanup_failure(std::exception_ptr failure)noexcept{dependencies_.scene.frames().record_actor_cleanup_failure(std::move(failure));}
    //These reached branches require populated nonoffline/attachment producers.
    //The normal offline, freshly unattached loading path never invokes them.
    std::int32_t human_respawn_delay_seconds(){throw std::runtime_error("Human respawn requires its actual nonoffline configuration consumer");}
    void hud_human_respawn(std::uint32_t,std::uint32_t){throw std::runtime_error("Human respawn requires its actual live HUD message consumer");}
    template<class Human>void human_initial_dead_notification(Human&){throw std::runtime_error("Nonoffline dead Human requires actual005A15D0 event dispatch");}
    template<class Actor>void detach_human_attachment(Actor&,std::int32_t){throw std::runtime_error("Occupied Human attachment requires its actual actor virtual80 receiver");}
private:
    template<class Human>auto& composition(Human& human){return dependencies_.world_actors.template human_loading_owner<typename Dependencies::Composition>(human);}
    Dependencies& dependencies_;
};
} //namespace hd2::game
#endif
