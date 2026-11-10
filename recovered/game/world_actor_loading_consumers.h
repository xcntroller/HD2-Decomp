#ifndef HD2_GAME_WORLD_ACTOR_LOADING_CONSUMERS_H
#define HD2_GAME_WORLD_ACTOR_LOADING_CONSUMERS_H

#include "game/car_scene_actor.h"
#include "frontend/localized_text.h"
#include "frontend/spatial_sound.h"
#include <limits>

namespace hd2::game {

//Containing source receivers for GameWorldActors and SceneActorLoadingRuntime.
//Dependencies borrows the SAME live Startup, Scene2/EC, Human factory, Players,
//session, script link, SOUND, HUD, proximity, Car resources/tables and packet
//owners. Constructing this borrower creates none of them and grants no readiness.
//Required callbacks cannot run before those real owners are constructed/bound.
template<class Dependencies>class WorldActorLoadingConsumers {
public:
    explicit WorldActorLoadingConsumers(Dependencies& dependencies):d_(dependencies){}
    std::uint32_t mode_flags()const{return d_.startup.application_flags();}
    std::int32_t human_current_player_id()const{return d_.startup.local_player_identity();}
    template<class Frames,class World,class Human>auto make_human_loading_owner(Frames& frames,World& world,Human& human){
        return d_.human_factory.make_human_loading_owner(frames,world,human);
    }
    template<class Actor>auto actor_inventory_capacity(const Actor& actor)const{return d_.scene.frames().actor_inventory_capacity(actor);}
    auto* script_context_for_frame(typename Dependencies::Frame frame){return d_.script_link.context_for_frame(frame);}
    void door_script_use(typename Dependencies::Frame frame,std::uint32_t) { //005FF4B0→0041D960; command argument ignored.
        auto* context=script_context_for_frame(frame);
        if(!context)throw std::runtime_error("Door notification has no actual Script context");
        context->use((mode_flags()&1u)!=0);
    }
    auto door_current_player_frame(){auto* human=d_.players.control().current_player();return human?human->base().frame():typename Dependencies::Frame{};}
    std::uint32_t session_mission_ordinal()const{return d_.session.words.template get<std::uint32_t>(0x3c);}
    auto& session_broadcast_packets(){return d_.broadcast_packets;}
    auto& spawn_packet_decoder(){return d_.spawn_decoder;}
    auto& actor_proximity(){return d_.proximity;}
    void clear_retired_actor_record(std::int32_t identity){d_.startup.clear_retired_actor_record(identity);}
    void clear_human_player_slot(std::uint32_t identity){d_.startup.clear_human_player_slot(identity);}
    template<class Actor>void remove_actor_presentation_references(Actor& actor){d_.hud.remove_actor_references(mode_flags(),1,&actor);}
    auto car_loading_resources(){return d_.car_resources;}
    auto& car_definitions(){return d_.car_definitions;}
    bool car_camera_setting264(){return d_.startup.car_camera_setting264();}
    auto car_current_attachment(){
        using Car=CarSceneActor<typename Dependencies::ActorFrames>;
        auto* human=d_.players.control().current_player();
        return human?dynamic_cast<const Car*>(human->field160):nullptr;
    }
    unsigned car_game_kind()const{return d_.session.status;} //Actual game04, distinct from configuration/request kinds and game1C.
    std::int32_t fowl_random_word(){return static_cast<std::int32_t>(d_.startup.random_word());}
    float fowl_cull_distance(){return d_.scene.documents().fields().far_clip;} //SAME parsed gameC4.
    void stop_actor_sound_voice(std::int32_t& handle){d_.startup.stop_game_sound_voice(handle);}
    void resume_door_sound(std::int32_t handle){d_.startup.resume_game_sound_voice(handle);}
    void pause_door_sound(std::int32_t handle){d_.startup.set_game_sound_voice_enabled(handle,false);}
    std::uint32_t door_sound_cursor(std::int32_t handle){return d_.startup.game_sound_voice_cursor(handle);}
    void door_sound_cursor(std::int32_t handle,std::uint32_t cursor){d_.startup.game_sound_voice_cursor(handle,cursor);}
    std::int32_t emit_scene_actor_sound(const frontend::SpatialSoundRequest& request){return d_.sound.emit(request);}
    std::int32_t emit_door_sound(const frontend::SpatialSoundRequest& request){return d_.sound.emit(request);}
    void emit_supply_sound(const frontend::SpatialSoundRequest& request){(void)d_.sound.emit(request);}
    void emit_fowl_sound(const frontend::SpatialSoundRequest& request){(void)d_.sound.emit(request);}
    void emit_animation_actor_sound(const frontend::SpatialSoundRequest& request){(void)d_.sound.emit(request);}
    template<class Resolve,class Type>std::int32_t add_game_sound_listener(typename Dependencies::Frame frame,void* callback,Resolve&& resolve,Type&& type){
        return d_.startup.add_game_sound_listener(frame,callback,std::forward<Resolve>(resolve),std::forward<Type>(type));
    }
    bool remove_game_sound_listener(std::int32_t slot){return d_.startup.remove_game_sound_listener(slot);}
    template<class Actor>void dynamic_item_contact_sound(Actor& actor){actor.contact_sound(d_.sound);}
    float dynamic_item_random_unit(){return d_.scene.frames().volume_random_unit();} //Actual engine random, separate from CRT.
    std::string dynamic_item_caption(std::uint32_t id){
        if(id>static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::max()))throw std::runtime_error("Dynamic item caption exceeds localized ID range");
        return frontend::lookup_localized_text(d_.startup.text(),static_cast<std::int32_t>(id));
    }
    bool dynamic_item_server_policy(){return d_.startup.human_remote_ai_enabled();} //005B69B0→006B66B0, SAME application configuration predicate.
    template<class Actor>void dynamic_item_sleep_notification(Actor&){throw std::runtime_error("Nonoffline item sleep requires actual network publication");}
private:
    Dependencies& d_;
};
} //namespace hd2::game
#endif
