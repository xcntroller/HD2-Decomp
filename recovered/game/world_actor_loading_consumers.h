#ifndef HD2_GAME_WORLD_ACTOR_LOADING_CONSUMERS_H
#define HD2_GAME_WORLD_ACTOR_LOADING_CONSUMERS_H

#include "game/world_loading_dependency_link.h"
#include "game/car_scene_actor.h"
#include "game/session_packet_stream.h"
#include "frontend/localized_text.h"
#include "frontend/spatial_sound.h"
#include "game/shot_effect.h"
#include "game/mission_particle_emission.h"
#include "game/runtime_actor_packets.h"
#include <limits>

namespace hd2::game {

//Containing source receivers for GameWorldActors and SceneActorLoadingRuntime.
//Dependencies borrows the SAME live Startup, Scene2/EC, Human factory, Players,
//session, script link, SOUND, HUD, proximity, Car resources/tables and packet
//owners. Constructing this borrower creates none of them and grants no readiness.
//Required callbacks cannot run before those real owners are constructed/bound.
template<class Dependencies>class WorldActorLoadingConsumers {
public:
    WorldActorLoadingConsumers()=default;
    //Scene2 constructs Humans BEFORE the collision tree/WorldActors exist.
    //Their cold properties require the actual process mode at that point.
    explicit WorldActorLoadingConsumers(const std::uint32_t& mode):initial_mode_(&mode){}
    explicit WorldActorLoadingConsumers(Dependencies& dependencies):d_(dependencies){}
    void bind_dependencies(Dependencies& dependencies){
        if(initial_mode_&&initial_mode_!=&dependencies.startup.application_flags_owner())
            throw std::logic_error("Scene2 cold mode and completed world dependencies belong to different process owners");
        d_.bind(dependencies);
    }
    void unbind_dependencies(Dependencies& dependencies)noexcept{d_.unbind(dependencies);}
    bool dependencies_bound()const noexcept{return d_.bound();}
    std::uint32_t mode_flags()const{
        if(d_.bound())return d().startup.application_flags();
        if(initial_mode_)return *initial_mode_;
        return d().startup.application_flags(); //Checked error for an actually unbound mode.
    }
    std::int32_t human_current_player_id()const{return d().startup.local_player_identity();}
    template<class Frames,class World,class Human>auto make_human_loading_owner(Frames& frames,World& world,Human& human){
        return d().human_factory.make_human_loading_owner(frames,world,human);
    }
    template<class Actor>auto actor_inventory_capacity(const Actor& actor)const{return d().scene.frames().actor_inventory_capacity(actor);}
    auto* script_context_for_frame(typename Dependencies::Frame frame){return d().script_link.context_for_frame(frame);}
    void door_script_use(typename Dependencies::Frame frame,std::uint32_t) { //005FF4B0→0041D960; command argument ignored.
        auto* context=script_context_for_frame(frame);
        if(!context)throw std::runtime_error("Door notification has no actual Script context");
        context->use((mode_flags()&1u)!=0);
    }
    auto door_current_player_frame(){auto* human=d().players.control().current_player();return human?human->base().frame():typename Dependencies::Frame{};}
    std::uint32_t session_mission_ordinal()const{return d().session.words.template get<std::uint32_t>(0x3c);}
    auto& session_broadcast_packets(){return d().broadcast_packets;}
    std::int32_t zone_style(){return d().startup.game_configuration().style224;}
    bool zone_remote_ai(){return d().startup.human_remote_ai_enabled();}
    void zone_hud_objective(std::uint32_t index,std::int32_t side){d().hud.select_objective(index,side);}
    auto& spawn_packet_decoder(){return d().spawn_decoder;}
    template<class Actor>void receive_human_presentation_packet(Actor* actor,SessionPacketStream& packet){d().human_factory.world_services().receive_human_presentation_packet(actor,packet,false);}
    auto& actor_proximity(){return d().proximity;}
    void clear_retired_actor_record(std::int32_t identity){d().startup.clear_retired_actor_record(identity);}
    void clear_human_player_slot(std::uint32_t identity){d().startup.clear_human_player_slot(identity);}
    template<class Human>void retire_scene_human(Human& human){d().human_factory.world_services().retire_scene_human(human);}
    template<class Actor>void remove_actor_presentation_references(Actor& actor){d().hud.remove_actor_references(mode_flags(),1,&actor.base());}
    auto car_loading_resources(){return d().car_resources;}
    auto& car_definitions(){return d().car_definitions;}
    bool car_camera_setting264(){return d().startup.car_camera_setting264();}
    auto car_current_attachment(){
        using Car=CarSceneActor<typename Dependencies::ActorFrames>;
        auto* human=d().players.control().current_player();
        return human?dynamic_cast<const Car*>(human->field160):nullptr;
    }
    unsigned car_game_kind()const{return d().session.status;} //Actual game04, distinct from configuration/request kinds and game1C.
    std::int32_t fowl_random_word(){return static_cast<std::int32_t>(d().startup.random_word());}
    float fowl_cull_distance(){return d().scene.documents().fields().far_clip;} //SAME parsed gameC4.
    void stop_actor_sound_voice(std::int32_t& handle){d().startup.stop_game_sound_voice(handle);}
    void resume_door_sound(std::int32_t handle){d().startup.resume_game_sound_voice(handle);}
    void pause_door_sound(std::int32_t handle){d().startup.set_game_sound_voice_enabled(handle,false);}
    std::uint32_t door_sound_cursor(std::int32_t handle){return d().startup.game_sound_voice_cursor(handle);}
    void door_sound_cursor(std::int32_t handle,std::uint32_t cursor){d().startup.game_sound_voice_cursor(handle,cursor);}
    std::int32_t emit_scene_actor_sound(const frontend::SpatialSoundRequest& request){return d().sound.emit(request);}
    auto mounted_sound_entry(std::int32_t id)const{return d().startup.mounted_sound_entry(id);}
    std::int32_t emit_door_sound(const frontend::SpatialSoundRequest& request){return d().sound.emit(request);}
    void emit_supply_sound(const frontend::SpatialSoundRequest& request){(void)d().sound.emit(request);}
    void emit_fowl_sound(const frontend::SpatialSoundRequest& request){(void)d().sound.emit(request);}
    void emit_animation_actor_sound(const frontend::SpatialSoundRequest& request){(void)d().sound.emit(request);}
    void emit_mine_effect(const HumanVector& position,const HumanVector& direction,std::uint32_t particle,std::int32_t sound){
        auto& world=d();emit_shot_effect_without_material(world.startup.shot_manager_process(),world.sound,
            world.startup.mission_effects_process(),world.scene.frames(),position,direction,particle,sound);
    }
    void rigid_water_particle(const HumanVector& position){
        auto& world=d();
        (void)emit_mission_particle(world.startup.mission_effects_process(),world.scene.frames(),45,position,
            HumanVector{{0,1,0,0}},typename Dependencies::Frame{},false,false);
    }
    template<class Actor>bool consume_mine_inventory(Actor* borrowed,std::int32_t definition){
        auto& world=d();auto& human=world.world_actors.human_loading_actor(borrowed);
        return world.human_factory.world_services().consume_human_mine_inventory(human,definition);
    }
    void publish_mine(std::int32_t identity,std::int32_t human,std::int32_t definition,const HumanVector& position,const HumanVector& direction){
        publish_runtime_mine(d().broadcast_packets,identity,human,definition,position,direction);
    }
    void publish_mine_contact(std::int32_t identity,std::uint32_t command){publish_runtime_mine_contact(d().broadcast_packets,identity,command);}
    template<class Resolve,class Type>std::int32_t add_game_sound_listener(typename Dependencies::Frame frame,void* callback,Resolve&& resolve,Type&& type){
        return d().startup.add_game_sound_listener(frame,callback,std::forward<Resolve>(resolve),std::forward<Type>(type));
    }
    bool remove_game_sound_listener(std::int32_t slot){return d().startup.remove_game_sound_listener(slot);}
    template<class Actor>void dynamic_item_contact_sound(Actor& actor){actor.contact_sound(d().sound);}
    float dynamic_item_random_unit(){return d().scene.frames().volume_random_unit();} //Actual engine random, separate from CRT.
    std::string dynamic_item_caption(std::uint32_t id){
        if(id>static_cast<std::uint32_t>(std::numeric_limits<std::int32_t>::max()))throw std::runtime_error("Dynamic item caption exceeds localized ID range");
        return frontend::lookup_localized_text(d().startup.text(),static_cast<std::int32_t>(id));
    }
    bool dynamic_item_server_policy(){return d().startup.human_remote_ai_enabled();} //005B69B0→006B66B0, SAME application configuration predicate.
    template<class Actor>void dynamic_item_sleep_notification(Actor&){throw std::runtime_error("Nonoffline item sleep requires actual network publication");}
private:
    Dependencies& d()const{return d_.get();}
    WorldLoadingDependencyLink<Dependencies> d_;
    const std::uint32_t* initial_mode_=nullptr;
};
} //namespace hd2::game
#endif
