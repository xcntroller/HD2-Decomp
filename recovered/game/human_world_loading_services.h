#ifndef HD2_GAME_HUMAN_WORLD_LOADING_SERVICES_H
#define HD2_GAME_HUMAN_WORLD_LOADING_SERVICES_H

#include "game/world_loading_dependency_link.h"
#include "game/human_camera_state.h"
#include "frontend/spatial_sound.h"
#include "game/human_speech_loading.h"
#include "game/human_speech_notifications.h"
#include "game/human_sight_relations.h"
#include "game/collision_ray.h"
#include <stdexcept>

namespace hd2::game {

//Concrete shared loading consumers for DD0/230, B0 and Human activation.
//Dependencies owns/borrows the SAME startup, session, scene, world_actors,
//players, first_person, faces, animations, presentation_shared, music, sound,
//dubbing, subtitles and hud. Speech borrows actual process SOUND and Players.
//Composition is resolved only when a callback runs, permitting the containing
//owner's mutually borrowing types to be declared before construction. It must
//publish actual WorldActors lifetimes before activation; no secondary map.
template<class Dependencies>class HumanWorldLoadingServices {
public:
    HumanWorldLoadingServices()=default;
    explicit HumanWorldLoadingServices(Dependencies& dependencies):dependencies_(dependencies){}
    void bind_dependencies(Dependencies& dependencies){dependencies_.bind(dependencies);}
    void unbind_dependencies(Dependencies& dependencies)noexcept{dependencies_.unbind(dependencies);}
    bool dependencies_bound()const noexcept{return dependencies_.bound();}
    std::uint32_t mode_flags()const{return dependencies().startup.application_flags();}
    std::int32_t human_current_player_id()const{return dependencies().startup.local_player_identity();}
    bool human_remote_ai_enabled(){return dependencies().startup.human_remote_ai_enabled();}
    auto human_difficulty(){return dependencies().startup.human_difficulty();}
    std::uint32_t game_word28()const{return dependencies().session.words.template get<std::uint32_t>(0x28);}
    std::uint32_t game_frame_serial()const{return dependencies().session.words.template get<std::uint32_t>(0x2c);}
    auto& human_ai_event_pool(){return dependencies().startup.human_ai_event_pool();}
    auto& human_perception_record_pool(){return dependencies().startup.human_perception_record_pool();}
    bool human_face_quality_enabled(){return dependencies().startup.human_face_quality_enabled();}
    bool presentation_application_fallback(){return human_remote_ai_enabled();} //00785BE0→006B66B0.
    const auto& presentation_mission_name()const{return dependencies().request.preparation.mission_resource;}
    auto& human_face_resources(){return dependencies().faces;}
    std::int32_t human_face_camouflage(){return static_cast<std::int32_t>(dependencies().presentation_shared.settings().terrain);} //Actual0098A6B0.
    auto human_animation_resource(unsigned table,unsigned row){return dependencies().animations.resource(table,row);}
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
    bool human_view_offset_enabled(){auto& view=dependencies().first_person.view();return view.enabled()||view.secondary_enabled();} //0069A4B0, actual58 OR59.
    template<class Human>void player_select_human(Human& human){dependencies().players.control().select_human(human);} //00461410→0045DA60.
    template<class Human>void remove_human_player(Human& human){(void)dependencies().players.control().remove_human(&human);} //0045E670.
    template<class Human>bool player_knows_enemy(Human& human){return dependencies().players.control().selection().knows(&human);} //0045E920.
    template<class Human>void player_add_enemy(Human* human){
        if(!human)throw std::runtime_error("Player enemy acquisition requires its actual Human");
        dependencies().players.control().selection().acquire(selection_human(*human),mode_flags(),*this); //0045E410 with actual feedback, before record publication.
    }
    template<class Human>void player_remove_enemy(Human* human){
        if(!human)throw std::runtime_error("Player enemy release requires its actual Human");
        dependencies().players.control().selection().release(selection_human(*human),mode_flags(),*this); //0045E540.
    }
    template<class Human>void hud_selected(Human& human,bool selected){dependencies().hud.select_human(&human,selected,mode_flags(),human_difficulty());} //0049B980.
    template<class Human>bool human_hidden(const Human& human)const{return human.activation.flag150;}
    template<class Human>void show_human(Human& human,std::uint32_t duration){ //005A8740, genuine already-shown RET.
        if(!human.activation.flag150)return;
        dependencies().scene.effects().human_fades().visibility(human,signed_duration(duration),true);
        human.activation.flag150=false;
        composition(human).control().shadow_visible(dependencies().scene.frames(),true); //005AC0F0, SAME A9C shadow, not SOUND.
        dependencies().presentation_shared.projection_models().visibility(dependencies().scene.frames(),&human,true,mode_flags());
    }
    template<class Human>void hide_human(Human& human,std::uint32_t duration){ //005A86F0.
        if(human.activation.flag150)return;
        dependencies().scene.effects().human_fades().visibility(human,signed_duration(duration),false);
        human.activation.flag150=true;
        composition(human).control().shadow_visible(dependencies().scene.frames(),false);
        dependencies().presentation_shared.projection_models().visibility(dependencies().scene.frames(),&human,false,mode_flags());
    }
    auto& human_roster(){return dependencies().players.control().human_roster();}
    template<class Human>std::uint32_t human_sight_alignment(Human& human){
        return game::human_sight_alignment(human,human_roster(),dependencies().session.words.template get<std::uint8_t>(0x34)!=0,
            [&]{return human_remote_ai_enabled();},[](auto& value){return value.activation.field16c;},
            [&](auto entry){return dependencies().world_actors.items().human_entry_alignment(entry);});
    }
    template<class Human,class Actor>bool human_sound_friendly(Human& human,Actor actor){return dependencies().sight.human_sight_friendly(&human,actor);}
    template<class Human>auto& human_state_factory(Human& human){return composition(human).state_factory();}
    template<class Human>auto& human_state_owner(Human& human){return composition(human).states();}
    template<class Human>auto& human_inline(Human& human){return composition(human).control();}
    template<class Human>bool sight_controlled_target(Human& human){ //005A59E0's SAME00461410/20 roster membership.
        if((mode_flags()&7u)!=7u&&human_remote_ai_enabled()&&(mode_flags()&1u))return dependencies().players.auxiliary().human_roster().contains(&human);
        return dependencies().players.control().human_roster().contains(&human);
    }
    template<class Human>float sight_range_factor(Human& human){ //005AC350, actual DD0+DA4 field0, not smoothed opacity.
        auto* control=human_inline(human).control();if(!control)throw std::runtime_error("Sight range requires actual Human DD0");
        return control->opacity().field0();
    }
    template<class Human>std::optional<float> sight_surface_factor(Human& human){ //005AC430 reads actual DD0+754.
        auto* control=human_inline(human).control();if(!control)throw std::runtime_error("Sight surface query requires actual Human DD0");
        if(control->physical().state().template get<std::uint32_t>(0x754))throw std::runtime_error("Sight surface requires actual populated DD0+754 material and GEffects94 receiver");
        return std::nullopt; //Actual cold NULL754 branch; no producer for a synthetic surface.
    }
    long double sight_view_distance(){return dependencies().scene.frames().sight_view_distance();}
    template<class Human>bool sight_attachment_direction(Human&,HumanVector&){throw std::runtime_error("Sight attachment direction requires actual actor virtual98/A4 receivers");}
    std::uint32_t sight_time()const{return game_word28();}
    template<class Frame>HumanVector sight_position(Frame frame){
        if(!frame)throw std::runtime_error("Sight position requires actual marker frame");
        const auto matrix=dependencies().scene.frames().actor_world_matrix(frame);return {{matrix[12],matrix[13],matrix[14],matrix[15]}};
    }
    template<class Actor>HumanVector sight_human_origin(Actor actor){return sight_position(selection_human(*actor).head_marker);}
    template<class Actor>auto sight_human_attachment(Actor actor){return selection_human(*actor).field160;}
    template<class Actor>void sight_exclude_human(Actor actor){human_inline(selection_human(*actor)).exclude_query(false);}
    template<class Actor>void sight_restore_human(Actor actor){human_inline(selection_human(*actor)).restore_query(false);}
    void sight_restore_dog(){dependencies().scene.exclusions().clear();} //0046DF80 clears even with no populated Dog exclusions.
    struct SightRayHit{std::uint32_t flags;const void* actor;};
    std::optional<SightRayHit> sight_ray(const HumanVector& start,const HumanVector& displacement,std::uint32_t layers,std::uint32_t flags){
        auto& d=dependencies();if(!d.session.loaded)throw std::logic_error("Sight ray requires actual loaded World/tree");
        auto& scene=d.scene;using Scene=std::remove_reference_t<decltype(scene)>;
        using Resources=std::remove_reference_t<decltype(scene.collision_resources())>;
        CollisionRay<typename Dependencies::Frame,typename Scene::Descriptor,Resources,typename Dependencies::ActorFrames>
            ray(&scene.collision_tree(),&scene.buckets(),scene.collision_resources(),scene.frames());
        const auto hit=ray.ray(start,displacement,layers,flags,false);if(!hit)return std::nullopt;
        if(hit->dynamic_identity)return SightRayHit{hit->dynamic_identity->flags,hit->dynamic_identity->actor};
        if(!hit->static_identity)throw std::logic_error("Sight ray lost actual collision identity");
        const auto id=*hit->static_identity;const auto& tree=scene.collision_tree();
        return SightRayHit{id.kind?tree.shapes.at(id.kind-0x80).at(id.index).flags:tree.triangles.at(id.index).flags,nullptr};
    }
    template<class Composition,class Human>Composition& human_loading_owner(Human& human){return dependencies().world_actors.template human_loading_owner<Composition>(human);}
    template<class Human>bool human_selected_suppressed(Human& human){return dependencies().presentation.human_selected_suppressed(human);}
    template<class Human>auto human_attached_actor(Human& human){return human.field160;}
    float human_sound_random(){return dependencies().scene.frames().volume_random_unit();} //Actual LS3DF RandFloat, distinct from CRT rand.
    template<class Human,class Actor>void player_sight_dog(Human&,Actor& actor){dependencies().players.control().add_sighted_dog(&actor);} //0045D4A0.
    template<class Human,class Actor>bool network_human_sight_friendly(Human&,Actor&){throw std::runtime_error("Nonoffline Sight requires actual005A09A0 friendship receiver");}
    template<class Actor>void dog_sight_changed(Actor& actor,bool,std::uint32_t){ //005D5CB0's genuine cold null534 gate.
        using Dog=typename std::decay_t<decltype(dependencies().scene.frames())>::Dog;
        auto* dog=dynamic_cast<Dog*>(&actor);if(!dog)throw std::logic_error("Dog Sight requires its actual actor family");
        if(dog->field534)throw std::runtime_error("Populated Dog Sight requires actual534 control receiver");
    }
    template<class Direction>void hud_sound_direction(const Direction& direction,std::uint32_t kind){dependencies().hud.hud_sound_direction(direction,kind);}
    template<class Actor>bool car_field5e0(Actor* actor){
        using Car=typename std::decay_t<decltype(dependencies().world_actors)>::Car;
        auto* car=dynamic_cast<Car*>(actor);
        if(!car||!car->geometry)throw std::runtime_error("Human weapon context requires actual initialized Car geometry");
        return car->geometry->physics().scalars().get(0x550)!=0; //Actor5E0, SAME owner as collision callbacks.
    }
    struct CommandArea{std::array<float,4> position;float radius;};
    template<class Human>CommandArea human_ai_command_area(Human&){throw std::runtime_error("Area AI handover requires the actual Human B4 area producer");}
    template<class State>void adjust_active_ai_stance(State& state,bool){
        if(!state.words().template get<std::uint32_t>(0x74)||!state.words().template get<std::uint32_t>(0x78))return;
        throw std::runtime_error("Populated Active AI stance requires actual74/78 command and A4/A8 posture producers");
    }
    template<class Human>bool human_unavailable(const Human& human)const { //005A6380, only its native AL result.
        std::int32_t health;const auto bits=human.properties.stats_ec[0];std::memcpy(&health,&bits,4);
        if(health<1||!human.loading_state||human.properties.field19c==1)return false;
        return (human.loading_state->words().template get<std::uint32_t>(0x50)&0x4000000u)!=0;
    }
    template<class Actor>auto actor_inventory_capacity(const Actor& actor){return dependencies().scene.frames().actor_inventory_capacity(actor);}
    template<class Actor>void set_scene_actor_active(Actor& actor,bool enabled){dependencies().scene.frames().set_scene_actor_active(actor,enabled);}
    template<class Actor>void set_runtime_actor_active(Actor& actor,bool enabled){dependencies().world_actors.runtime().set_active(actor,enabled);}
    void human_special_music_reference(bool enabled){dependencies().music.reference(enabled,mode_flags(),dependencies().startup);} //0069E920/70, SAME gameC8..D4/SOUND.
    std::int32_t emit_human_equipment_sound(const frontend::SpatialSoundRequest& request){return dependencies().sound.emit(request);}
    std::uint32_t speech_random(){return dependencies().startup.random_word();}
    std::uint32_t projection_game_time(){return dependencies().session.words.template get<std::uint32_t>(0x28);}
    std::int32_t projection_random_word(){return static_cast<std::int32_t>(dependencies().startup.random_word());}
    std::uint32_t projection_quality_byte(){return dependencies().startup.profile_settings().quality_byte(2);}
    bool speech_special_mode43(){return dependencies().startup.players_process().words().template get<std::uint8_t>(0x43)!=0;} //00459480.
    bool speech_group0_active(){return !dependencies().startup.players_process().groups()[0].empty();} //00456B80(0), SAME process group48.
    std::uint32_t human_speech_length(std::int32_t identity){return dependencies().startup.game_speech_length(identity);}
    const char* missing_speech_caption(){return dependencies().startup.missing_speech_caption();}
    template<class Human>void play_human_speech(Human& human,std::int32_t identity,float near_range,float far_range,bool subtitles){
        (void)dependencies().startup.play_game_speech(&human,nullptr,identity,near_range,far_range,subtitles,dependencies().dubbing);
    }
    template<class Human>void player_speech_notification(Human& human,std::int32_t identity){
        HumanSpeechNotifications notifications(dependencies().subtitles,dependencies().hud,*this);notifications.notify(human,identity);
    }
    template<class State>void queue_active_ai_speech(State& state,std::uint32_t category,bool variant,std::uint32_t delay,bool quiet){
        HumanSpeechLoading speech(dependencies().scene.frames(),*this);speech.active(state,speech_signed_word(category),variant,delay,quiet);
    }
    template<class State>void queue_player_ai_speech(State& state,std::uint32_t category,std::uint32_t delay){
        HumanSpeechLoading speech(dependencies().scene.frames(),*this);speech.player(state,speech_signed_word(category),delay);
    }
    void record_state_cleanup_failure(std::exception_ptr failure)noexcept{dependencies().scene.frames().record_actor_cleanup_failure(std::move(failure));}
    //These reached branches require populated nonoffline/attachment producers.
    //The normal offline, freshly unattached loading path never invokes them.
    std::int32_t human_respawn_delay_seconds(){throw std::runtime_error("Human respawn requires its actual nonoffline configuration consumer");}
    void hud_human_respawn(std::uint32_t,std::uint32_t){throw std::runtime_error("Human respawn requires its actual live HUD message consumer");}
    template<class Human>void human_initial_dead_notification(Human&){throw std::runtime_error("Nonoffline dead Human requires actual005A15D0 event dispatch");}
    template<class Actor>void detach_human_attachment(Actor&,std::int32_t){throw std::runtime_error("Occupied Human attachment requires its actual actor virtual80 receiver");}
private:
    template<class Actor>auto& selection_human(Actor& actor){
        using Human=typename Dependencies::Composition::Human;
        auto* human=dynamic_cast<Human*>(&actor);
        if(!human)throw std::logic_error("Enemy selection requires the actual Human family");
        return *human;
    }
    static std::int32_t signed_duration(std::uint32_t value){std::int32_t result;std::memcpy(&result,&value,4);return result;}
    template<class Human>auto& composition(Human& human){return dependencies().world_actors.template human_loading_owner<typename Dependencies::Composition>(human);}
    Dependencies& dependencies()const{return dependencies_.get();}
    WorldLoadingDependencyLink<Dependencies> dependencies_;
};
} //namespace hd2::game
#endif
