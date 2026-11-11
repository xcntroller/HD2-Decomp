#ifndef HD2_GAME_HUMAN_WORLD_LOADING_SERVICES_H
#define HD2_GAME_HUMAN_WORLD_LOADING_SERVICES_H

#include "game/world_loading_dependency_link.h"
#include "game/human_camera_state.h"
#include "frontend/spatial_sound.h"
#include "game/human_speech_loading.h"
#include "game/human_speech_notifications.h"
#include "game/human_sight_relations.h"
#include "game/collision_ray.h"
#include "game/collision_water_queries.h"
#include "game/human_frame_feedback.h"
#include "game/human_item_frame_execution.h"
#include "game/human_scene_frame.h"
#include "game/human_state_action_completion.h"
#include "game/human_objective_experience.h"
#include "game/human_ai_shot_direction.h"
#include "game/human_weapon_reload.h"
#include "game/human_frame_stats.h"
#include "game/human_actor_contact.h"
#include "game/human_frame_commands.h"
#include "game/geometry_scene_actors.h"
#include "game/simple_scene_actors.h"
#include <algorithm>
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
    auto& human_ai_frame_inputs(){return dependencies().startup.human_ai_frame_inputs();}
    auto& human_ai_navigation_jobs(){return dependencies().startup.human_ai_navigation_jobs();}
    std::uint32_t human_frame_random_word(){return dependencies().startup.random_word();}
    std::uint32_t human_control_microseconds(){return dependencies().clock.microseconds();}
    float human_control_animation_scale(){return dependencies().startup.human_animation_time_scale();}
    float human_control_world_floor()const{return dependencies().bounds.scene_minimum_y();}
    bool human_alternate_motion_sound()const{return dependencies().presentation_shared.alternate_motion_sound();}
    auto& human_ladder(void* actor){
        using Frames=typename Dependencies::ActorFrames;
        auto* ladder=dynamic_cast<LadderSceneActor<Frames>*>(static_cast<SceneActor<Frames>*>(actor));
        if(!ladder||!ladder->geometry)throw std::runtime_error("Human climbing requires actual Ladder marker geometry");
        return *ladder;
    }
    const auto& human_ladder_geometry(void* actor){return *human_ladder(actor).geometry;}
    auto& human_navigation_door(void* actor){
        using Frames=typename Dependencies::ActorFrames;
        auto* door=dynamic_cast<DoorSceneActor<Frames>*>(static_cast<SceneActor<Frames>*>(actor));
        if(!door)throw std::runtime_error("AI navigation reference requires an actual Door");
        return *door;
    }
    void human_navigation_request_door(void* actor,std::uint32_t flags){ //00586200 subtype1B.
        auto& door=human_navigation_door(actor);
        if((flags&1u)&&(mode_flags()&1u)){
            door.change_state(1,true,false,false,dependencies().world_actors);
            return;
        }
        throw std::runtime_error("AI Door action requires the actual Human kind6 interaction receiver");
    }
    auto human_ai_resolve_actor(std::uint32_t handle){ //005ACA50→00475DD0, Scene2 identity.
        auto& scene=dependencies().scene.frames();
        return handle&0x80000000u?nullptr:scene.scene_actor_identity(static_cast<std::int32_t>(handle));
    }
    template<class Actor>bool human_scene_actor_matches(Actor& actor){return dependencies().scene.frames().scene_actor_identity(actor.base().id10())==&actor;} //00475DD0.
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
    template<class Human>auto& human_presentation_equipment(Human& human){
        auto* equipment=composition(human).control().equipment();
        if(!equipment)throw std::logic_error("Human presentation requires actual equipment");
        return *equipment;
    }
    template<class Human>bool human_is_player(const Human& human)const{return human_camera_is_player(human,mode_flags(),human_current_player_id());} //005A0890, AL.
    template<class Human>bool human_is_view_actor(const Human& human)const{return human_is_player(human);}
    bool human_view_offset_enabled(){auto& view=dependencies().first_person.view();return view.enabled()||view.secondary_enabled();} //0069A4B0, actual58 OR59.
    template<class Human>void player_select_human(Human& human){dependencies().players.control().select_human(human);} //00461410→0045DA60.
    auto* current_human_frame_player(){return dependencies().players.control().current_player();}
    auto human_geometry_camera_target(){return dependencies().camera.target_frame();} //004650B0, Camera virtual6C.
    auto& human_geometry_sweep(){return dependencies().sweep;}
    void refresh_human_presentation_camera(){ //0046C370, Camera virtual40(0).
        auto& d=dependencies();auto& view=d.first_person.view();
        d.camera.refresh_current_loading_target(d.players.control(),view.enabled(),view.secondary_enabled(),
            [&](auto& human){return d.first_players.control_sector(human);},d.first_players.camera_retirement());
    }
    template<class Human>void human_frame_first_person_state(Human& human,std::uint32_t state){
        const auto mode=mode_flags();
        if((mode&2u)&&dependencies().players.control().current_player()==&human)
            dependencies().first_person.view().set_state(mode,state); //0077F3D0→00491640.
    }
    template<class Human>void human_frame_item_action_fpv(Human& human,std::int32_t animation){ //0058C4D0.
        const auto mode=mode_flags();
        if((mode&2u)&&dependencies().players.control().current_player()==&human)
            dependencies().first_person.view().request(mode,animation,0);
    }
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
    template<class Human>void adjust_human_sight_alignment(Human& human,float amount){
        game::adjust_human_sight_alignment(human,amount,mode_flags(),dependencies().session.words.template get<std::uint8_t>(0x34)!=0,
            [&](auto entry){return dependencies().world_actors.items().human_entry_alignment(entry);});
    }
    template<class Human>void refresh_human_frame_perception(Human& human){
        dependencies().sight.refresh_human_perception(human);human.list2ac.expire(dependencies().hearing.human_sound_ticks());
    }
    template<class Human>void advance_human_command_events(Human& human,std::uint32_t elapsed){
        if(!human.command)throw std::logic_error("Human frame requires actual commandB4 owner");
        human.command->advance_events(elapsed);
    }
    template<class Entry>bool human_feedback_item_ready(Entry entry){return dependencies().world_actors.items().runtime_item(entry).field14<=game_word28();}
    template<class Entry>auto& human_frame_output_item(Entry entry){return dependencies().world_actors.items().runtime_item(entry);}
    template<class Item,class Human>bool reload_human_ai_weapon(Item& item,Human& human,
            std::int32_t& ammunition,bool retain_last){
        //0098B02C starts zero. Its only observed writer is the developer
        //command at007205E2, outside the ordinary mission runtime.
        return reload_human_weapon(item,human,ammunition,retain_last,false,*this);
    }
    auto& human_reload_items(){return dependencies().world_actors.items();}
    template<class Human,class Entry>void consume_human_ammunition(Human& human,Entry entry,unsigned slot){
        auto& d=dependencies();
        auto& items=d.world_actors.items();
        auto& magazine=items.runtime_item(entry);
        const auto model=magazine.model(); //007DBD80 obtains the model before mutating the holder.
        auto* storage=human.base().storage();
        auto* slots=storage?storage->slots(slot):nullptr;
        if(!slots)throw std::logic_error("Reload retirement requires the actual ammunition holder");
        auto* equipment=human_inline(human).equipment();
        if(!equipment)throw std::logic_error("Reload retirement requires the actual 230 owner");
        magazine.holder_actor=nullptr;
        const bool notify=items.equipment_notifications_enabled();
        if((mode_flags()&2u)&&notify&&d.players.control().current_player()==&human)d.hud.inventory_changed();
        human.activation.flag178=true;
        const float weight=static_cast<float>(static_cast<long double>(
            frontend::pre_mission_detail::Bytes::real(human.base().field7c()))-magazine.scalar());
        std::uint32_t weight_bits;std::memcpy(&weight_bits,&weight,sizeof weight_bits);
        human.base().field7c(weight_bits);
        if(slots->detach(entry)<1)throw std::logic_error("Reload magazine retired from its actual holder");
        if(notify){
            if(slot==1)equipment->transfers().removed(entry,1,nullptr,0);
            if(model){
                auto& frames=d.scene.frames();
                magazine.flag1c=true;frames.link_scene_object(model,nullptr);frames.object_visible(model,true);
                const auto reference=equipment->state().first_volume();
                const auto position=reference?sight_position(reference):HumanVector{};
                frames.physics_publish_position(model,position);
                frames.auto_parent_object(model);
            }
        }
        items.remove_runtime_item(entry);
    }
    template<class Item,class Human>void emit_human_weapon_reload(Item& item,Human& human){
        if(!(mode_flags()&2u))return;
        auto& d=dependencies();
        frontend::SpatialSoundRequest request;
        request.flags=human_is_player(human)&&human_view_offset_enabled()?0x40u:0u;
        request.relative=item.shot_frame();request.source=&human;
        request.group=3;request.entry=item.reload_sound();
        const auto voice=d.sound.emit(request);
        item.fielda8=d.startup.game_sound_voice_live(voice)?static_cast<std::uint32_t>(voice):0xffffffffu;
        human.activation.flag178=true;
    }
    template<class AI,class Inline,class Item>void prepare_human_ai_shot_direction(AI& state,Inline& control,
            Item& item,HumanVector& direction){
        auto& human=*state.human();
        auto random=[&]{return dependencies().scene.frames().volume_random_unit();};
        const auto kind=state.words().template get<std::uint32_t>(0x48);
        const auto flags=state.words().template get<std::uint32_t>(0x50);
        const auto skill=human_frame_signed_word(human.properties.stats_ec[4]);
        if(auto* target=state.resources().aimed_actor();target&&item.primary_payload_kind()==2){
            const auto origin=sight_position(human.base().frame());
            const auto destination=sight_position(target->base().frame());
            (void)aim_human_ai_ballistic_shot(direction,origin,destination,item.primary_payload_ballistic_speed(),kind,flags,skill,random);
            return;
        }
        std::uint32_t stance=0xffffffffu;
        if(flags&0xf8u){ //005A5CC0.
            stance=control.frame_movement_mode();
            if(stance==5&&!(flags&8u))stance=flags&0x10u?1u:2u;
            else if(stance==2&&(flags&0x10u))stance=1;
        }
        const auto spread=static_cast<float>(human_ai_shot_spread(skill,control.frame_posture_mode(),stance,
            state.words().template get<std::int32_t>(0xb0),item.definition().equipment_command(),random));
        spread_human_ai_shot(direction,spread,kind,kind==3?0u:human_difficulty(),random);
    }
    template<class Human>auto human_guard_holster_weapon(Human& human){
        return find_player_mode_holster(human,dependencies().world_actors.items(),2,game_word28());
    }
    template<class Human>auto human_guard_shoulder_weapon(Human& human){
        auto* storage=human.base().storage();
        if(!storage||storage->single()||!storage->holder(2))throw std::logic_error("Guard weapon selection requires the actual shoulder holder");
        return storage->holder(2)->entry();
    }
    template<class State>bool prepare_guard_frame_weapon(State& state){ //005594B0.
        state.refresh_item();
        auto* human=state.human();
        auto& inventory=composition(*human).inventory();
        auto& items=dependencies().world_actors.items();
        if(state.item()&&items.runtime_item(state.item()).definition().kind()!=6)return true;
        const auto equip=[&](auto entry,std::int32_t slot){
            if(!entry)return false;
            return state.item()?inventory.swap(state.item(),3,*human,entry,slot):inventory.move(entry,*human,slot,3,0);
        };
        const auto shoulder=human_guard_shoulder_weapon(*human);
        if(shoulder&&items.runtime_item(shoulder).definition().kind()!=6&&equip(shoulder,2))return true;
        return equip(human_guard_holster_weapon(*human),1);
    }
    template<class Human>auto human_feedback_kind9(Human& human){return find_player_mode_holster(human,dependencies().world_actors.items(),9,game_word28());}
    auto human_projectile_direction(){return dependencies().first_person.view().projectile_direction();}
    auto human_projectile_position(){return dependencies().first_person.view().projectile_position(mode_flags());}
    template<class State,class Entry,class Fire>void execute_human_feedback_item(State& state,Entry entry,std::uint32_t command,Fire& fire){
        auto* human=state.human();if(!human)throw std::logic_error("Human feedback execution requires bound B0+44");
        //The offline command receiver below genuinely ignores its vector/scalar
        //arguments. Nonoffline needs the actual0x70 input packet and must not
        //consume unwritten vectors from0058B6C0's remote early return.
        if((mode_flags()&7u)!=7u)throw std::runtime_error("Nonoffline feedback execution requires actual0073E5A0 packet publication");
        auto& items=dependencies().world_actors.items();
        auto* item=entry?&items.runtime_item(entry):nullptr;
        HumanVector position,direction;
        (void)execute_human_item_frame(state,item,true,0,position,direction,*this,fire);
        composition(*human).state_factory().notify_inline_frame_command(state,command); //AFTER actual item virtual24.
    }
    template<class Human,class Fire>void apply_human_frame_feedback(Human& human,std::uint32_t flags,Fire& fire){
        auto& owner=composition(human);auto* state=owner.states().get();
        if(!state)throw std::logic_error("Human feedback requires actual current B0");
        HumanFrameFeedbackReceivers<HumanWorldLoadingServices,Fire> receivers{*this,fire};
        owner.state_factory().apply_inline_feedback(*state,flags,receivers);
    }
    template<class Human,class Runtime>void advance_human_scene_frame(Human& human,bool authority,std::uint32_t elapsed,Runtime& runtime){
        game::advance_human_scene_frame(human,authority,elapsed,*this,runtime,dependencies().scene.frames());
    }
    template<class Human,class Actor>void human_action_completion_command(Human& human,std::uint32_t subtype,Actor* target){
        //005ABD90 kind6→005ABCC0. The completion subtypes reach E0 without
        //the extra player notification used by interaction-start subtypes.
        if((mode_flags()&7u)!=7u)throw std::runtime_error("Nonoffline interaction requires its actual 00751480 receiver");
        human_look_control(human).interaction_command(target,subtype,0);
    }
    template<class Actor,class Human>void finish_human_lockpick(Actor& target,Human& human){
        human_navigation_door(&target).complete_lockpick(dependencies().broadcast_packets);
        if((mode_flags()&7u)!=7u)return; //005A95D0.
        auto& bits=human.fieldc0.at(7);
        float value;
        std::memcpy(&value,&bits,sizeof value);
        value+=.5f;
        if(value>5.f)value=5.f;
        std::memcpy(&bits,&value,sizeof bits);
        //The subsequent formatted retail logger 0069BFE0 is empty.
    }
    template<class Actor>void finish_human_explosive_disarm(Actor& actor){
        auto& d=dependencies();
        action_explosive_place(actor).disarm(d.world_actors,d.scene.buckets());
    }
    template<class State,class Human>void finish_human_action_unavailability(State& state,Human& human){
        //All observed B0 families use 00524D50/60 for virtual8C/90.
        auto& words=state.words();
        words.set(0x50,words.template get<std::uint32_t>(0x50)|0x4000000u);
        if(!(mode_flags()&1u))return; //005A6360 ends after virtual90.
        auto& d=dependencies();
        HumanObjectiveExperience(d.startup.application_flags_owner()).add_experience(human,500);
        if(human.field258!=0xffffffffu)d.sight.source().observer_enabled(human_frame_signed_word(human.field258),false);
        if(human.field25c!=0xffffffffu)d.sight.source().target_mode(human_frame_signed_word(human.field25c),2);
        if(human.field2a4!=0xffffffffu){
            d.hearing.remove_listener(human_frame_signed_word(human.field2a4));
            human.field2a4=0xffffffffu;
        }
    }
    template<class Human,class Runtime>void advance_human_state(Human& human,bool authority,std::uint32_t elapsed,Runtime& runtime){
        auto& owner=composition(human);
        auto* state=owner.states().get();
        if(!state||!owner.states().ready())throw std::logic_error("Human frame requires its actual bound B0 state");
        const auto kind=state->words().template get<std::uint32_t>(0x48);
        if(kind==6)return; //Actual005A4330/005A4340 are RET4.
        auto& factory=owner.state_factory();
        if(kind==5&&!authority){factory.advance_script_presentation(*state,elapsed,runtime);return;}
        if(kind!=1&&kind!=4&&kind!=5&&kind!=8){runtime.advance_human_state(human,authority,elapsed);return;}
        using Actor=SceneActor<typename Dependencies::ActorFrames>;
        using Base=std::remove_reference_t<decltype(*state)>;
        struct Queries {
            HumanWorldLoadingServices& world;
            Runtime& runtime;
            auto& human_ai_frame_inputs(){return world.human_ai_frame_inputs();}
            bool human_scene_actor_matches(Actor& actor){return world.human_scene_actor_matches(actor);}
            bool human_action_target_list_contains(Actor* actor){
                const auto& active=world.dependencies().world_actors.runtime().active_actors(); //0089B4C0+20.
                return std::find(active.rbegin(),active.rend(),actor)!=active.rend();
            }
            std::uint32_t human_action_target_field94(Actor& actor){
                return world.action_explosive_place(actor).field94;
            }
            bool human_action_target_alive(Actor& actor){
                auto* target=dynamic_cast<Human*>(&actor);
                if(!target)throw std::runtime_error("Human action timer requires its actual Human target");
                return human_frame_signed_word(target->properties.stats_ec[0])>0;
            }
            void finish_human_state_action(Base& state,bool success){complete_human_state_action(state,success,world,runtime);}
        } queries{*this,runtime};
        if(kind==1){factory.advance_player_frame(*state,authority,elapsed,queries,runtime);return;}
        if(kind==5){factory.advance_script_authority(*state,elapsed,queries,runtime);return;}
        if(factory.advance_guard_or_area_frame(*state,authority,elapsed,queries,runtime)){
            owner.states().change(5); //005AF680, retire the previous state after its frame returns.
            factory.resume_script_after_ai(*owner.states().get());
        }
    }
    template<class Actor>auto& action_explosive_place(Actor& actor){
        auto* target=dynamic_cast<ExplosivePlaceSceneActor<typename Dependencies::ActorFrames>*>(&actor);
        if(!target)throw std::runtime_error("Human explosive action requires its actual ExplosivePlace");
        return *target;
    }
    template<class Human,class Drops>void advance_human_bleeding(Human& human,std::uint32_t elapsed,Drops& drops){
        auto& frames=dependencies().scene.frames();auto random=[&]{return frames.volume_random_unit();};
        advance_human_frame_bleeding(human,frames,random,drops,elapsed,mode_flags());
    }
    template<class Human>void advance_human_periodic_stats(Human& human,std::uint32_t elapsed){
        auto* state=composition(human).states().get();if(!state)throw std::logic_error("Human periodic stats require actual B0");
        game::advance_human_periodic_stats(human,*state,elapsed,mode_flags());
    }
    template<class Human,class Actor>bool human_sound_friendly(Human& human,Actor actor){return dependencies().sight.human_sight_friendly(&human,actor);}
    template<class Human>auto& human_state_factory(Human& human){return composition(human).state_factory();}
    template<class Human>auto& human_state_owner(Human& human){return composition(human).states();}
    struct HumanRespawnConfiguration{std::int32_t style;std::uint32_t delay,enabled;};
    HumanRespawnConfiguration human_respawn_configuration(){
        const auto& config=dependencies().startup.game_configuration();
        return {config.style224,static_cast<std::uint32_t>(config.rules228[5]),static_cast<std::uint32_t>(config.rules228[6])};
    }
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
        if(control->physical().ground_contact())throw std::runtime_error("Sight surface requires actual populated DD0+754 material and GEffects94 receiver");
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
    auto human_geometry_ray(const HumanVector& start,const HumanVector& displacement,std::uint32_t layers,std::uint32_t flags,bool normal=true){
        auto& d=dependencies();if(!d.session.loaded)throw std::logic_error("Human ray requires actual loaded World/tree");
        auto& scene=d.scene;using Scene=std::remove_reference_t<decltype(scene)>;
        using Resources=std::remove_reference_t<decltype(scene.collision_resources())>;
        CollisionRay<typename Dependencies::Frame,typename Scene::Descriptor,Resources,typename Dependencies::ActorFrames>
            ray(&scene.collision_tree(),&scene.buckets(),scene.collision_resources(),scene.frames());
        return ray.ray(start,displacement,layers,flags,normal);
    }
    template<class Actor>float human_collision_car_speed(Actor& actor){return dependencies().scene.car_callbacks().car_speed958(&actor);}
    template<class Actor>auto& human_collision_car_occupants(Actor& actor){
        auto* car=dynamic_cast<CarSceneActor<typename Dependencies::ActorFrames>*>(&actor);
        if(!car)throw std::logic_error("Human Car contact requires the actual Car actor");
        return car->occupants;
    }
    template<class Actor>auto* human_collision_car_driver(Actor& actor){return dependencies().scene.car_callbacks().car_driver_actor(&actor);}
    template<class Actor>void human_collision_alignment(Actor& actor,float value){set_human_sight_alignment(selection_human(actor),value,mode_flags());}
    template<class Human,class Actor,class Runtime>bool human_actor_collision(Human& human,Actor& actor,const HumanVector& point,Runtime& runtime){
        return receive_human_actor_contact(human,actor,point,*this,runtime);
    }
    template<class Commands>
    void publish_human_frame_commands(std::int32_t identity,bool second,const Commands& commands){
        write_human_frame_commands(dependencies().broadcast_packets,mode_flags(),identity,second,commands);
    }
    template<class Actor>
    void receive_human_authority_packet(Actor* actor,SessionPacketStream& packet,bool reserved){
        if((mode_flags()&7u)!=7u)
            throw std::runtime_error("Human authority commands require actual nonoffline forwarding");
        using Human=typename Dependencies::Composition::Human;
        auto* human=dynamic_cast<Human*>(actor);
        if(!reserved&&(!human||!human->loading_state||human->loading_state->words().template get<std::uint32_t>(0x48)!=1u))
            throw std::runtime_error("Scene2 authority packet requires its actual actor20/B0+C4 receiver");
        auto request=[&](std::uint32_t identity){
            if(!human||!human->loading_state||human->loading_state->words().template get<std::uint32_t>(0x48)!=1u)return;
            auto& owner=composition(*human).control();
            auto* control=owner.control();auto* look=owner.look();
            if(!control||!look)throw std::logic_error("Player Ladder command requires the actual DD0/E0 owners");
            auto* target=dependencies().scene.frames().scene_actor_identity(static_cast<std::int32_t>(identity));
            (void)control->request_frame_ladder(target,*look,*this); //00540DB0 case6 ignores the result.
        };
        receive_human_ladder_commands(packet,request);
    }
    template<class Human>void notify_human_ladder_probe(Human&,std::uint32_t){
        throw std::runtime_error("Nonoffline Ladder probe requires actual00588D20 command15 publication");
    }
    template<class Descriptor>const ActorFrameData* human_geometry_contact_data(const Descriptor& descriptor){
        return dependencies().scene.frames().find_actor_frame_data(descriptor.frame);
    }
    void human_geometry_physics_contact(std::uintptr_t handle,const HumanVector& velocity){
        dependencies().scene.car_callbacks().car_physics_contact(handle,0u,velocity,{{0,0,0,0}}); //SAME00410630 module2 binding.
    }
    template<class Human>void human_contact_door(void* actor,Human& human){
        auto& door=human_navigation_door(actor);
        if(!door.properties.fields220[1])door.change_state(2,false,false,human.properties.field19c!=1,dependencies().world_actors);
    }
    auto human_geometry_water_ray(const HumanVector& start,const HumanVector& displacement,std::uint32_t flags=0){
        auto& d=dependencies();if(!d.session.loaded)throw std::logic_error("Human water ray requires actual loaded World/tree");
        auto& scene=d.scene;using Resources=std::remove_reference_t<decltype(scene.collision_resources())>;
        CollisionWaterQueries<typename Dependencies::Frame,Resources,typename Dependencies::ActorFrames>
            query(scene.collision_tree(),scene.collision_resources(),scene.frames());
        using Hit=typename std::remove_reference_t<decltype(d.sweep)>::Hit;
        const auto water=query.ray(start,displacement,flags);std::optional<Hit> result;
        if(water){result.emplace();result->position=water->position;result->normal=water->normal;result->static_identity=water->identity;}
        return result;
    }
    template<class Hit>std::uint32_t human_geometry_flags(const Hit& hit){
        if(hit.dynamic_identity)return hit.dynamic_identity->flags;
        if(!hit.static_identity)throw std::logic_error("Human geometry hit has no actual identity");
        const auto id=*hit.static_identity;const auto& tree=dependencies().scene.collision_tree();
        return id.kind?tree.shapes.at(id.kind-0x80).at(id.index).flags:tree.triangles.at(id.index).flags;
    }
    template<class Hit>std::uint32_t human_geometry_material(const Hit& hit){ //0075C940, SAME0071F9B0 process table/count.
        const auto material=human_geometry_flags(hit)>>24;
        return material<dependencies().startup.car_definitions().materials().size()?material:0xffffffffu;
    }
    template<class Hit>std::uint32_t human_geometry_mask(const Hit& hit){
        if(!hit.dynamic_identity)throw std::logic_error("Human dynamic contact has no actual descriptor");
        return hit.dynamic_identity->mask;
    }
    template<class Hit>std::uint32_t human_geometry_human_mode(const Hit& hit){ //0071BEBC calls005A5D10 on actual descriptor48.
        if(!hit.dynamic_identity||!hit.dynamic_identity->actor)throw std::logic_error("Human motion contact lost its actual actor");
        auto& human=selection_human(*static_cast<SceneActor<typename Dependencies::ActorFrames>*>(hit.dynamic_identity->actor));
        auto* state=human_state_owner(human).get();
        if(!state)throw std::logic_error("Human motion contact requires actual B0 state");
        return state->words().template get<std::uint32_t>(0x50)&7u;
    }
    std::optional<SightRayHit> sight_ray(const HumanVector& start,const HumanVector& displacement,std::uint32_t layers,std::uint32_t flags){
        const auto hit=human_geometry_ray(start,displacement,layers,flags,false);if(!hit)return std::nullopt;
        if(hit->dynamic_identity)return SightRayHit{hit->dynamic_identity->flags,hit->dynamic_identity->actor};
        if(!hit->static_identity)throw std::logic_error("Sight ray lost actual collision identity");
        const auto id=*hit->static_identity;const auto& tree=dependencies().scene.collision_tree();
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
        if(!state.first_frame_inputs()||!state.second_frame_inputs())return;
        throw std::runtime_error("Populated Active AI stance requires actual74/78 command and A4/A8 posture producers");
    }
    template<class Human>bool human_unavailable(const Human& human)const { //005A6380, only its native AL result.
        std::int32_t health;const auto bits=human.properties.stats_ec[0];std::memcpy(&health,&bits,4);
        if(health<1||!human.loading_state||human.properties.field19c==1)return false;
        return (human.loading_state->words().template get<std::uint32_t>(0x50)&0x4000000u)!=0;
    }
    template<class Human>std::uint32_t human_hostile_relation_mask(const Human& observer,const Human& target){
        return dependencies().sight.source().human_relation_mask(human_frame_signed_word(observer.field258),
            human_frame_signed_word(target.field25c));
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
    template<class State>void queue_guard_frame_speech(State& state,std::int32_t category,std::uint32_t delay){
        HumanSpeechLoading speech(dependencies().scene.frames(),*this);
        (void)speech.request(state,category,delay,1,false); //005882F0.
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
