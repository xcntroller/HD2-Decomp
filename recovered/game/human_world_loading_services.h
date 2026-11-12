#ifndef HD2_GAME_HUMAN_WORLD_LOADING_SERVICES_H
#define HD2_GAME_HUMAN_WORLD_LOADING_SERVICES_H
#include "game/car_actor_authority.h"
#include "game/car_targeted_placement.h"
#include "game/car_scene_actor.h"
#include "game/first_person_human_items.h"
#include "game/human_player_item_notification.h"
#include "game/human_player_shot_direction.h"
#include "game/human_hud_loading.h"

#include "game/world_loading_dependency_link.h"
#include "game/human_camera_state.h"
#include "frontend/spatial_sound.h"
#include "game/human_speech_loading.h"
#include "game/human_speech_notifications.h"
#include "game/human_sight_relations.h"
#include "game/collision_ray.h"
#include "game/collision_water_queries.h"
#include "game/dynamic_collision_point_queries.h"
#include "game/human_frame_feedback.h"
#include "game/human_item_frame_execution.h"
#include "game/human_scene_frame.h"
#include "game/human_mine_contact.h"
#include "game/human_state_action_completion.h"
#include "game/human_objective_experience.h"
#include "game/human_ai_shot_direction.h"
#include "game/human_weapon_reload.h"
#include "game/human_frame_stats.h"
#include "game/human_actor_contact.h"
#include "game/human_frame_commands.h"
#include "game/scene_light_sample.h"
#include "game/geometry_scene_actors.h"
#include "game/simple_scene_actors.h"
#include "game/static_weapon_actor.h"
#include "game/car_steering.h"
#include "game/shot_surface_marks.h"
#include "game/human_state_damage.h"
#include "game/human_inventory_retirement.h"
#include "game/mission_particle_emission.h"
#include "game/human_death_inventory.h"
#include "game/human_damage_presentation.h"
#include "game/human_damage_policy.h"
#include "game/human_damage_messages.h"
#include "game/human_damage_score.h"
#include "game/human_packet_receiver.h"
#include "game/human_packet_notifications.h"
#include "game/human_script_loading_controls.h"
#include "game/human_squad_snapshot.h"
#include "game/human_score_presentation.h"
#include "game/human_death_objective.h"
#include "game/human_peer_removal.h"
#include "game/human_scene_retirement.h"
#include "game/session_peer_lifetime.h"
#include "game/session_spawn_match.h"
#include "game/session_transmit_targets.h"
#include "game/prepared_human_loading.h"
#include "game/human_spawn_position.h"
#include "game/zone_scene_actor.h"
#include "game/session_human_spawn.h"
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
    auto* create_session_spawn_human(std::uint32_t peer,std::uint32_t identity,const HumanVector& position,const HumanVector& direction,
            frontend::PreMissionSoldier& soldier,const std::array<std::uint32_t,12>* stats,bool alternate){
        PreparedHumanLoading<Dependencies> factory(dependencies()); //Only a borrowed adapter; all scene/input/model ownership stays in World/process.
        return factory.create_session_human(peer,human_frame_signed_word(identity),position,direction,soldier,stats,-1,alternate);
    }
    auto* spawn_scene_human(std::uint32_t identity){
        using Human=typename Dependencies::Composition::Human;
        auto* actor=dependencies().scene.frames().scene_actor_command_identity(human_frame_signed_word(identity)); //Actual00475D50 reserved-hole retirement fallback.
        auto* human=dynamic_cast<Human*>(actor);if(actor&&!human)throw std::logic_error("Presentation spawn identity is not its actual Human");return human;
    }
    template<class Spawn>void spawn_authority_human(Spawn& spawn,std::uint32_t peer,frontend::PreMissionSoldier* soldier,const std::array<std::uint32_t,12>* stats){spawn_session_human(spawn,peer,soldier,stats,*this);}
    std::int32_t first_free_spawn_identity(){return dependencies().scene.frames().first_free_player_identity();}
    template<class Spawn>void initial_spawn_pose(Spawn& spawn,HumanVector& position,HumanVector& direction){read_prepared_spawn_pose(dependencies().scene.frames(),spawn,position,direction);}
    bool spawn_cycle_enabled(){return dependencies().startup.game_configuration().rules228[6]!=0;}
    void reject_session_spawn(std::uint32_t peer){dependencies().startup.queue_application_command(15,0,peer,1);}
    auto* select_authority_spawn_point(std::uint32_t kind,std::uint32_t side){return dependencies().scene.frames().select_session_spawn(kind,side,[this]{return human_frame_random_word();});}
    template<class Spawn>void prepare_authority_spawn_soldier(Spawn& spawn,frontend::PreMissionSoldier& soldier){
        if(soldier.equipment[1].item==-1)(void)frontend::transfer_pre_mission_equipment(soldier,dependencies().catalogue,2,{-1,prepared_identity_word(human_frame_signed_word(spawn.properties.fielda4)),1});
        soldier.split_spawn_stacks();
    }
    auto* create_authority_spawn_human(std::uint32_t peer,std::int32_t identity,const HumanVector& position,const HumanVector& direction,
            frontend::PreMissionSoldier* soldier,const std::array<std::uint32_t,12>* stats,std::int32_t clothing,bool alternate){
        if(!soldier)throw std::logic_error("Authority Human factory requires its actual soldier argument");
        PreparedHumanLoading<Dependencies> factory(dependencies());return factory.create_session_human(peer,identity,position,direction,*soldier,stats,clothing,alternate);
    }
    template<class Spawn>auto* require_authority_spawn_human(Spawn& spawn){
        using Human=typename Dependencies::Composition::Human;auto* human=dynamic_cast<Human*>(spawn.spawned_human);
        if(!human)throw std::logic_error("Authority spawn lost its actual A8 Human borrower");return human;
    }
    void bind_authority_spawn_record(std::uint32_t peer){dependencies().spawn_records.bind_spawned_player(peer,0,"");}
    template<class Spawn>void publish_authority_spawn(std::uint32_t peer,std::int32_t identity,Spawn& spawn,frontend::PreMissionSoldier* soldier,
            const std::array<std::uint32_t,12>* stats,const HumanVector& position,const HumanVector& direction){dependencies().spawn_records.publish_session_spawn(peer,identity,spawn,soldier,stats,position,direction);}
    template<class Spawn>void notify_authority_spawn_created(Spawn& spawn){PreparedHumanLoading<Dependencies> factory(dependencies());factory.spawn_model_created(spawn.base().frame());}
    auto spawn_position_sweep(const HumanVector& start,const HumanVector& displacement,float radius){return dependencies().sweep.sweep(start,displacement,radius,0,0,false);} //007184C0, no output normal.
    bool spawn_position_occupied(const HumanVector& position){return human_spawn_position_occupied(*this,position);}
    std::int32_t zone_style(){return dependencies().startup.game_configuration().style224;}
    bool zone_next_spawn_pose(std::int32_t identity,HumanVector& position,HumanVector& direction,bool cycle){
        auto* actor=dependencies().scene.frames().scene_actor_command_identity(identity);
        auto* zone=dynamic_cast<ZoneSceneActor<typename Dependencies::ActorFrames>*>(actor);
        if(!zone)throw std::logic_error("Team spawn requires its actual type37 zone actor");
        return zone->next_spawn_pose(position,direction,cycle,*this);
    }
    template<class Zone>void publish_zone_owner(Zone& zone,std::uint32_t command){ //005ED320 through SAME module13 stream.
        auto& packet=dependencies().broadcast_packets.begin(13,0);packet.write(static_cast<std::uint32_t>(zone.base().id10()),10);
        packet.write(command,2);if(command==2)packet.write(zone.field98,2);packet.finish_broadcast();
    }
    void orient_spawn_toward_zone(HumanVector& direction,const HumanVector& position,std::uint32_t side){ //0069E0A0.
        auto& scene=dependencies().scene.frames();std::int32_t selected=0,best=0;
        for(std::size_t n=0;n<scene.scene_actor_identity_count();++n){
            if(n>static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()))throw std::length_error("Zone orientation exceeds native signed identity");
            auto* zone=dynamic_cast<ZoneSceneActor<typename Dependencies::ActorFrames>*>(scene.scene_actor_command_identity(static_cast<std::int32_t>(n)));
            if(!zone)continue;
            const auto value=human_frame_signed_word(zone->field90);
            if((side==0&&best<value)||(side==1&&value==0)){best=value;selected=static_cast<std::int32_t>(n);}
        }
        auto* zone=dynamic_cast<ZoneSceneActor<typename Dependencies::ActorFrames>*>(scene.scene_actor_command_identity(selected));if(!zone)return;
        const auto& target=zone->base().world_position();
        for(unsigned n=0;n!=3;++n)direction[n]=static_cast<float>(static_cast<long double>(target[n])-position[n]);
        direction[1]=0;direction=normalize_zone_spawn_direction(direction); //Fourth native output lane was unwritten; preserve known caller lane.
    }
    std::uint32_t spawn_result(){return dependencies().session.words.template get<std::uint32_t>(0x88);}
    void set_spawn_match_phase(std::uint32_t phase){set_session_spawn_match_phase(dependencies().session,*this,phase);}
    void spawn_hud_mode(std::uint32_t phase){
        dependencies().hud.spawn_mode(phase,[this]{return mode_flags();},[this](bool enabled){spawn_pause(enabled);});
    }
    void spawn_pause(bool enabled){ //0069B8B0's scalar prefix; normal non7 spawn does not reach this offline callee.
        auto& game=dependencies().session;
        game.words.set(0x64,static_cast<std::uint8_t>(game_word28()>=2500&&(mode_flags()&7u)==7u&&enabled));
        throw std::runtime_error("Game pause requires actual0071F640/005E8D50/00428F00 propagation");
    }
    void spawn_camera_command(std::uint32_t command,bool present,std::uint32_t value){
        auto refresh=[this](auto&){refresh_human_presentation_camera();};
        dependencies().camera.command_spawn(command,present,value,mode_flags(),refresh,[this]{return human_frame_random_word();});
    }
    template<class Human>bool spawn_roster_contains(Human& human){return dependencies().players.control().human_roster().contains(&human);}
    template<class Human>void append_spawn_human(Human& human){(void)dependencies().players.control().add_human(&human);}
    template<class Human>std::uint32_t spawn_state_kind(Human& human){return human.loading_state?human.loading_state->words().template get<std::uint32_t>(0x48):0u;} //005A5FE0 null B0 gate.
    template<class Human>void spawn_human_side(Human& human,std::uint32_t side){set_session_peer_side(dependencies().startup.game_control_records(),dependencies().startup,static_cast<std::uint32_t>(human.base().id10()),side,false);} //005A8800.
    template<class Human>void select_spawn_human(Human& human){dependencies().players.control().select_human(human);}
    template<class Human>void spawn_selection_suppressed(Human& human,std::uint8_t value){
        if(!human.loading_state)throw std::logic_error("Spawn flags require actual Human B0");
        human.loading_state->selection_suppressed(value);
    }
    template<class Human>void spawn_player_flag(Human& human,std::uint8_t value){
        using Player=typename Dependencies::Composition::StateFactory::Player;
        auto* state=dynamic_cast<Player*>(human.loading_state);if(!state)throw std::logic_error("Spawn Player flag requires actual Player B0");
        state->player_words().set(0x550,value);
    }
    template<class Human>void spawn_remote_flag(Human& human,std::uint8_t value){
        using Remote=typename Dependencies::Composition::StateFactory::Remote;
        auto* state=dynamic_cast<Remote*>(human.loading_state);if(!state)throw std::logic_error("Spawn Remote flag requires actual Remote B0");
        state->spawn_remote_flag(value);
    }
    template<class Human>void present_local_spawn(std::uint32_t peer,Human& human){present_session_local_spawn(dependencies().session,*this,peer,human);}
    void spawn_hide_map(){dependencies().hud.hide_map();}
    template<class Human>bool spawn_selected_suppressed(Human& human){return dependencies().presentation.human_selected_suppressed(human);}
    bool spawn_voice_enabled(){return dependencies().startup.profile_settings().bytes[0x38]!=0;}
    std::uint32_t human_transmit_side(void* actor){return human_specialization(human_score_actor(actor),false);}
    void submit_human_transmit_targets(void* object,const std::uint32_t* ids,std::uint32_t count){dependencies().startup.submit_human_transmit_targets(object,ids,count);}
    void update_human_transmit_targets(){update_session_transmit_targets(dependencies().startup.game_control_records(),*this);}
    bool human_integrity_matches(std::size_t stage){
        auto& tables=dependencies().startup.mission_integrity_tables();
        return tables.expected(stage)==tables.observed(stage);
    }
    template<class Human>void apply_human_damage(Human& human,std::uint32_t kind,float amount,const HumanVector& point,
            const HumanVector& impulse,SceneActor<typename Dependencies::ActorFrames>* source,std::uint32_t hit_kind){
        auto* cause=source?dynamic_cast<Human*>(source):nullptr;
        if(source&&!cause)throw std::logic_error("Human damage requires its actual Human source borrower");
        damage_human(human,kind,amount,point,impulse,cause,hit_kind,*this,dependencies().scene.frames());
    }
    std::int32_t human_damage_console(unsigned index){return dependencies().startup.game_console_variables().integer(index);}
    bool human_damage_protection()const{return dependencies().session.words.template get<std::uint8_t>(0x35)!=0;}
    template<class Human>bool human_damage_roster_contains(Human& human){return dependencies().players.control().human_roster().contains(&human);}
    template<class Human>HumanVector human_damage_target_direction(Human& human){return human_camera_target_direction(human);}
    template<class Human>HumanVector human_damage_random_position(Human& human){ //005A3CC0 has no stack arguments; its caller retains the D4 tail on the stack.
        constexpr std::array<unsigned,17> bones{{22,21,20,18,17,16,15,14,13,11,10,9,12,8,3,1,24}};
        auto& frames=dependencies().scene.frames();const auto visual=frames.actor_find(human.base().frame(),"a",1); //00839100.
        const auto bone=frames.human_visual_bone(visual,bones.at(human_frame_random_word()%17));
        if(!bone)throw std::logic_error("Human damage point requires its actual skeletal frame");
        const auto world=frames.actor_world_matrix(bone);return {{world[12],world[13],world[14],world[15]}};
    }
    template<class Human>void human_damage_hit(Human& human,std::uint32_t flags,const HumanVector& point,const HumanVector& impulse){
        human_look_control(human).hit_command(flags,point,impulse,dependencies().scene.frames(),*this); //005ABBD0→0077F7B0.
    }
    template<class Human>void human_damage_death_source(Human& human,Human* source){human_look_control(human).death_source(source);} //005ABBE0.
    template<class Human>void refresh_human_damage_geometry(Human& human){
        auto* control=human_inline(human).control();if(control)control->refresh_ragdoll_geometry(true,true); //005AC420's real null-DD0 gate.
    }
    template<class Human>void stop_human_damage_motion(Human& human){ //005AC440, SAME DD0 and face.
        if(auto* control=human_inline(human).control()){
            auto& words=control->physical().control_frame_words();words.set(0x1ac,0u);words.set(0x1b0,0u);words.set(0x18c,std::uint8_t{1});control->face().activate_air_pose();
        }
    }
    template<class Human>void human_damage_limb_limit(Human& human){
        auto publish=[&](std::uint32_t command){if(auto* state=human_state_owner(human).get())state->publish_frame_command_flags(command);};
        human_inline(human).damage_limb_limit(publish);
    }
    template<class Human>void human_damage_stop_speech(Human& human){dependencies().startup.stop_game_dubbing(human.base().frame());}
    template<class Human>bool human_damage_speaking(Human& human){return dependencies().startup.playing_game_dubbing(human.base().frame());}
    template<class Human>void human_damage_speech(Human& human,std::uint32_t category,std::uint32_t delay,std::uint32_t flags){
        auto* state=human_state_owner(human).get();if(!state)throw std::logic_error("Human damage speech requires actual B0");
        HumanSpeechLoading speech(dependencies().scene.frames(),*this);(void)speech.request(*state,static_cast<std::int32_t>(category),delay,flags,false);
    }
    template<class Human>std::uint32_t human_specialization(Human& human,bool alternate=false){ //005A8820→00730530.
        const bool offline=(mode_flags()&7u)==7u;const auto kind=human.properties.field19c;
        if(offline||human_remote_ai_enabled()){
            if(kind==2||(!offline&&kind==5))return 0u;
            if(kind==1||kind==4)return 1u;
            return 2u;
        }
        const auto identity=static_cast<std::uint32_t>(human.base().id10());const auto& records=dependencies().startup.game_control_records().records;
        return identity<records.size()?records[identity].word(alternate?0x68:0x64):2u;
    }
    std::uint32_t human_local_specialization(){ //007304B0 has no native bounds fallback; source requires its actual produced record.
        const auto identity=static_cast<std::uint32_t>(human_current_player_id());const auto& records=dependencies().startup.game_control_records().records;
        if(identity>=records.size())throw std::logic_error("Local Human specialization requires actual session player record");
        return records[identity].word(0x64);
    }
    template<class Human>bool human_damage_network_policy(Human& human,std::uint32_t kind,Human* source,std::int32_t& reflected){
        auto team=[&](Human& actor){return human_specialization(actor);};
        return allow_human_network_damage(human,kind,source,reflected,dependencies().startup.game_configuration(),team);
    }
    void enter_control_lock(unsigned index){dependencies().startup.enter_control_lock(index);}
    void leave_control_lock(unsigned index)noexcept{dependencies().startup.leave_control_lock(index);}
    std::uint32_t human_score_style(){return static_cast<std::uint32_t>(dependencies().startup.game_configuration().style224);}
    std::int32_t human_objective_score_health(void* actor){return human_frame_signed_word(human_score_actor(actor).properties.stats_ec[0]);}
    bool human_objective_score_suppressed(void* actor){
        auto& human=human_score_actor(actor);if(!human.loading_state)throw std::logic_error("Objective scoring requires actual Human B0");
        return human.loading_state->selection_suppressed();
    }
    std::uint32_t human_objective_score_side(void* actor){return human_specialization(human_score_actor(actor));}
    void award_human_objective_score(std::uint32_t amount,std::uint32_t side){award_human_objective_points(dependencies().startup.game_control_records(),amount,side,*this);}
    void queue_human_objective_status(std::int32_t identity,std::int32_t status){
        dependencies().startup.queue_application_command(10,0,static_cast<std::uint32_t>(identity),static_cast<std::uint32_t>(status));
    }
    std::uint32_t human_score_victim_kind(std::uint32_t id){auto* human=dependencies().scene.frames().scene_human_identity(human_frame_signed_word(id));return human?human->properties.field19c:0;}
    auto& human_score_actor(void* actor){
        using Human=typename Dependencies::Composition::Human;auto* human=static_cast<Human*>(actor);
        if(!human||dependencies().scene.frames().scene_human_identity(human->base().id10())!=human)
            throw std::logic_error("Score record lost its actual Scene2 Human borrower");
        return *human;
    }
    std::uint32_t human_score_state_kind(void* actor){auto& human=human_score_actor(actor);if(!human.loading_state)throw std::logic_error("Scoring requires actual source B0");return human.loading_state->words().template get<std::uint32_t>(0x48);}
    bool queue_human_score_rank(void* actor,std::uint32_t rank){
        auto& human=human_score_actor(actor);auto* state=human_state_owner(human).get();if(!state)return false;state->queue_score_rank(rank);return true;
    }
    void remove_human_score_peer(std::uint32_t identity,std::uint32_t reason){
        remove_human_peer(dependencies().startup.game_control_records(),identity,reason,*this);
    }
    void queue_human_peer_removal(std::uint32_t reason,std::uint32_t identity){dependencies().startup.queue_application_command(17,0,reason,identity);}
    const void* peer_hud_human(){return dependencies().hud.peer_human();}
    void refresh_peer_hud_human(){
        using Human=typename Dependencies::Composition::Human;
        dependencies().hud.refresh_peer_human(static_cast<Human*>(nullptr),mode_flags(),dependencies().startup.local_player_identity());
    }
    void* peer_override_human(){return dependencies().players.control().override_player();}
    void clear_peer_override_human(){dependencies().players.control().override_player(nullptr);}
    void* local_peer_human(){return local_session_peer_human(dependencies().startup.game_control_records(),dependencies().startup.local_player_identity());}
    bool peer_human_has_state(void* actor){return human_score_actor(actor).loading_state!=nullptr;}
    void remove_peer_auxiliary_human(void* actor){(void)dependencies().players.auxiliary().human_roster().remove(&human_score_actor(actor));} //0045FDF0→00461C70, no selection release.
    void remove_peer_runtime_links(void* actor){dependencies().world_actors.runtime().remove_human_links(human_score_actor(actor));}
    void retire_peer_scene_human(void* actor){dependencies().world_actors.retire_scene_human(human_score_actor(actor));}
    void retire_session_peer(std::uint32_t transport){
        auto& d=dependencies();game::retire_session_peer(d.startup.game_control_records(),d.session,transport,*this);
    }
    template<class Human>void retire_scene_human(Human& human){
        auto& d=dependencies();auto& owner=composition(human);
        game::retire_scene_human(human,d.sight.source(),d.hearing,d.proximity,owner.control(),
            [this]{return mode_flags();},[this](auto& actor,std::int32_t slot){this->detach_human_attachment(actor,slot);},
            [&owner](auto& state,bool second){
                auto* actual=owner.states().get();if(actual!=&state)throw std::logic_error("Scene retirement lost SAME actual B0 owner");
                owner.state_factory().flush_scene_retirement_commands(*actual,second);
            });
    }
    void emit_human_peer_removal_log(const std::string& text){ //006C4B40, called with SAME current World owner.
        dependencies().startup.emit_application_log(text);
    }
    template<class Human>void human_damage_score(Human& human,std::int32_t source){score_human_death(dependencies().startup.game_control_records(),static_cast<std::uint32_t>(human.base().id10()),static_cast<std::uint32_t>(source),*this);}
    std::string human_score_rank_label(frontend::ControlStartupRecord& record){ //00725B00, SAME holder4 and definition44.
        auto side=record.word(0x64);
        if(human_score_style()==0)if(auto* actor=record.human_actor()){
            auto& human=human_score_actor(actor);auto* storage=human.base().storage();auto* holder=storage?storage->holder(4):nullptr;
            if(!holder)throw std::logic_error("Rank label requires actual Human holder4");
            const auto item=holder->entry();side=item&&dependencies().world_actors.items().runtime_item(item).definition().fields38[3]==0?1u:0u;
        }
        return human_damage_localize((side==1?0x2792u:0x279cu)+record.word(0x54));
    }
    void emit_human_rank_text(const std::string& message,const std::array<float,3>& colour){
        if(!(mode_flags()&2u)){dependencies().startup.append_game_console(message);return;}
        dependencies().hud.append_message(message.c_str(),colour);
    }
    template<class Human>void receive_human_score_rank(Human& human,std::uint32_t rank,bool player=true){ //005406C0 case16.
        auto& records=dependencies().startup.game_control_records();const auto identity=static_cast<std::uint32_t>(human.base().id10());
        if(identity>=records.records.size())throw std::out_of_range("Rank decoder requires actual peer record");
        if(player)records.records[identity].word(0x54,rank); //0072F4C0 returns the actual40..60 subobject, unlocked here.
        const auto* profile=dependencies().profiles.record(human_frame_signed_word(human.properties.field170));
        publish_human_rank_statistics(human,human_rank_statistics(profile,records.records[identity].word(0x54)));
        present_human_score_rank(records,rank,identity,*this);
    }
    template<class Human>void human_damage_death_packet(Human& human,Human* source){human_damage_score(human,source?source->base().id10():-1);}
    template<class Human>void human_damage_message(Human& human,std::uint32_t kind,std::uint32_t hit,std::int32_t source){present_human_damage_message(human,kind,hit,source,*this);}
    bool human_damage_team_style(){return dependencies().startup.game_configuration().style224!=0;}
    std::string human_damage_localize(std::uint32_t id){return frontend::lookup_localized_text(dependencies().startup.text(),static_cast<std::int32_t>(id));}
    auto human_damage_source(std::int32_t id){return dependencies().scene.frames().scene_human_identity(id);} //00475E00, actual Scene2/EC.
    template<class Human>std::string human_damage_display_name(Human& human){ //005A9980/005A8320, Actor80 remains the SAME name owner.
        const auto kind=human.properties.field19c;std::string value;
        if(kind==1||kind==4||kind==5){
            if((mode_flags()&7u)!=7u&&kind==1){const auto id=static_cast<std::uint32_t>(human.base().id10());const auto& records=dependencies().startup.game_control_records().records;
                if(id<records.size())value=records[id].name();} //0072EBD0: invalid identity genuinely returns NULL/empty COW.
            else if(human.profile_record)value=human.profile_record->user_name.empty()?human.profile_record->full_name:human.profile_record->user_name;
            else value=dependencies().scene.frames().frame_name(human.base().frame());
        }else if(human.properties.field1a8!=0xffffffffu)value=human_damage_localize(human.properties.field1a8);
        else if(kind==2)value=human_damage_localize(0x1bef);
        else if(kind==3)value=human_damage_localize(0x1bfd);
        else return human.base().name();
        human.base().name(value);return value;
    }
    void emit_human_damage_text(std::uint32_t id,const std::array<float,3>& colour,const std::array<std::string,2>& arguments){ //005A4600→0069BF70.
        const auto message=format_human_damage_text(human_damage_localize(id),arguments);
        if(!(mode_flags()&2u)){dependencies().startup.append_game_console(message);return;}
        dependencies().hud.append_message(message.c_str(),colour);
    }
    template<class Human>void human_damage_packet(Human& human,std::uint32_t kind,std::uint32_t hit,Human& source){human_damage_message(human,kind,hit,source.base().id10());}
    template<class Human>void human_damage_impact_sound(Human& human,std::uint32_t material,const HumanVector& point,bool network){ //00451B00, actual SOUND request, not blood particles.
        auto& d=dependencies();const auto& materials=d.car_definitions.materials();
        if(!(mode_flags()&2u)||material==0xffffffffu||material>=materials.size())return;
        auto& state=d.startup.shot_manager_process();auto& request=state.hit_sound;
        request.group=4;state.hit_sound_position=point;request.position={{point[0],point[1],point[2]}};
        request.entry=human_frame_signed_word(materials[material].word(0x30)); //Native param5 is false for Human damage.
        if(request.entry==-1)return;
        request.source=&human;request.flags=network?1u:0u;request.relative=nullptr;request.voice_group=1;
        (void)d.sound.emit(request);
    }
    void human_damage_blood_surface(const HumanVector& point,const HumanVector& direction){
        auto& d=dependencies();emit_shot_blood_mark(d.startup.shot_manager_process(),d.scene.frames(),*this,point,direction);
    }
    template<class Frame>std::int32_t emit_human_particle(std::uint32_t definition,const HumanVector& position,
            const HumanVector& direction,Frame parent,bool orientation,bool flag){
        auto& d=dependencies();return emit_mission_particle(d.startup.mission_effects_process(),d.scene.frames(),definition,position,direction,parent,orientation,flag);
    }
    void human_damage_material_effect(std::uint32_t material,const HumanVector& point,const HumanVector& normal){ //00451C00.
        auto& d=dependencies();HumanVector direction,position;
        const auto have_direction=d.camera.query_direction(0,direction);const auto have_position=d.camera.query_vector(0,position);
        if(!have_direction||!have_position)
            throw std::logic_error("Damage particles require the actual active camera outputs");
        if(!damage_material_particle_visible(point,position,direction))return;
        const auto& materials=d.car_definitions.materials();
        if(material>=materials.size())throw std::out_of_range("Damage particle material lacks its actual definition");
        (void)emit_human_particle(materials[material].word(0x3c),point,normal,typename Dependencies::ActorFrames::Frame{},true,false);
    }
    void spawn_human_blood(const HumanVector& point,const HumanVector& direction){ //0044F4F0: draw BEFORE0044F520 quality/texture gates.
        auto& d=dependencies();const auto index=1u+human_frame_random_word()%3u;
        emit_shot_blood_mark_texture(d.startup.shot_manager_process(),d.scene.frames(),*this,point,direction,index);
    }
    template<class Human>void human_damage_kill_credit(Human& source,Human& victim,std::uint32_t kind){ //005A0BE0, offline only.
        if((mode_flags()&7u)!=7u)return;
        auto& sight=dependencies().sight;
        if(source.properties.field19c==1){
            std::int32_t experience;
            if(!sight.human_sight_hostile(&source,&victim)||human_unavailable(victim))experience=-1000;
            else if(victim.properties.field19c==2&&victim.loading_state&&victim.loading_state->suppress_hostile_selection())experience=-500;
            else{
                if(kind==1){ //005A9480 updates Human D0, distinct from D4 stealth progression.
                    float old;auto& word=source.fieldc0.at(4);std::memcpy(&old,&word,4);
                    const auto sum=static_cast<long double>(old)+.03999999910593033f;const float value=sum>2.f?2.f:static_cast<float>(sum);std::memcpy(&word,&value,4);
                }
                if(!victim.loading_state)throw std::logic_error("Human kill credit requires victim B0");
                if(victim.loading_state->words().template get<std::uint32_t>(0x48)==5){increment_human_frame_stat(source,false,.029999999329447746f,mode_flags());experience=300;}
                else{
                    const auto& hostiles=victim.perception.lists()[0];const bool known=std::find(hostiles.rbegin(),hostiles.rend(),&source)!=hostiles.rend();
                    if(known||static_cast<float>(human_frame_signed_word(victim.loading_state->words().template get<std::uint32_t>(0xb0)))<=60000.f)experience=100;
                    else{increment_human_frame_stat(source,false,.019999999552965164f,mode_flags());experience=300;}
                }
            }
            HumanObjectiveExperience(dependencies().startup.application_flags_owner()).add_experience(source,experience);
        }
        std::int32_t target=-1;
        if(sight.human_sight_hostile(&source,&victim)){
            const auto& list=source.perception.lists()[0];
            for(auto n=list.size();n;)if(list[--n]==&victim){target=human_frame_signed_word(victim.field25c);break;}
            if(!source.loading_state)throw std::logic_error("Human kill notification requires source B0");
            source.loading_state->sight_enemy_killed(victim);
        }else{
            const auto a=source.properties.field19c,b=victim.properties.field19c;
            const bool neutral=((a==1&&b==2)||(a==2&&b==1))&&sight.human_sight_friendly(&source,&victim); //005A0A50.
            const auto& list=source.perception.lists()[neutral?3u:1u];if(list.empty())return;
            for(auto n=list.size();n;)if(list[--n]==&victim)target=human_frame_signed_word(victim.field25c);
        }
        if(target>=0){sight.human_sight_changed(&source,true,static_cast<std::uint32_t>(target));sight.human_sight_changed(&source,false,static_cast<std::uint32_t>(target));}
    }
    template<class Human>void human_damage_script_death(Human& human){dependencies().contexts.human_script_death(human);}
    template<class Human>HumanVector human_damage_head_position(Human& human){
        if(!human.head_marker)throw std::logic_error("Initial dead hit requires actual Human260 head marker");
        const auto matrix=dependencies().scene.frames().actor_world_matrix(human.head_marker);return {{matrix[12],matrix[13],matrix[14],matrix[15]}};
    }
    template<class Human>void human_damage_presentation_cleanup(Human& human){ //005A03E0, not the larger authority005A0220.
        auto* state=human_state_owner(human).get();if(!state)throw std::logic_error("Presentation death requires actual B0");
        human_state_factory(human).retire_presented_dead_state(*state);if(mode_flags()&1u)return;
        human_damage_stop_speech(human);human.activation.field154=0;human.field204=0;
        auto& look=human_look_control(human);look.force_frame_commands(true);auto publish=[&](std::uint32_t flags){state->publish_frame_command_flags(flags);};
        try{look.frame_movement_animation(0x71,publish,true);}catch(...){look.force_frame_commands(false);throw;}
        look.force_frame_commands(false);
    }
    template<class Human>void human_damage_presentation_player(Human& human,std::uint32_t kind){
        if(!human_is_player(human))return;
        auto* state=human_state_owner(human).get();if(!state)throw std::logic_error("Presented Player damage requires actual B0");
        human_player_damage_feedback(*state,kind,static_cast<Human*>(nullptr));
    }
    template<class Player>void retire_presented_dead_player(Player& player){ //0053BE10 prefix, SAME actual Car/FPV/Human owners.
        auto* human=player.human();if(!human)throw std::logic_error("Presented Player death requires actual Human");
        if((mode_flags()&7u)!=7u&&human->field160&&human->field160->base().type()==4){
            auto& car=human_pose_car(human->field160);if(!car.occupants.empty()&&car.occupants[0]==human){
                if(!car.loading)throw std::logic_error("Presented Player death lost actual Car control owner");car.loading->release_presented_driver_pedal();
            }
        }
        player.player_words().set(0x4d4,std::uint8_t{0});player.player_words().set(0x4d8,0u);
        human->activation.field17c=human->activation.field180=human->activation.field184=0;
        if(human_is_player(*human))dependencies().first_person.models().bind_weapon(mode_flags(),typename Dependencies::ActorFrames::Frame{},typename Dependencies::ActorFrames::Frame{},-1);
    }
    template<class Player>void retire_presented_dead_player_tail(Player& player){
        if((mode_flags()&7u)==7u||player.human()->base().id10()!=human_current_player_id())return;
        auto& d=dependencies();if(!human_remote_ai_enabled()){d.hud.set_injury_message(0x4f58,5000);return;}
        const auto& configuration=d.startup.game_configuration();const auto limit=configuration.rules228[18]; //Native270.
        if(!configuration.rules228[21]){ //Native27C, lock0072EC70 remains held across the actual HUD call.
            d.startup.enter_control_lock(0);
            struct Unlock{decltype(d.startup)& startup;~Unlock(){startup.leave_control_lock(0);}}unlock{d.startup};
            const auto& records=d.startup.game_control_records().records;const auto identity=static_cast<std::uint32_t>(human_current_player_id());
            if(identity>=records.size())throw std::logic_error("Presented Player death requires actual local respawn record");
            if(limit<=human_frame_signed_word(records[identity].word(0xb0)))d.hud.set_injury_message(0x4f56,5000);
        }else if(limit<=d.session.words.template get<std::int32_t>(0x84))d.hud.set_injury_message(0x4f56,5000);
    }
    template<class Human>void human_damage_death_cleanup(Human& human){ //005A0220; the Human and B0 remain live throughout.
        auto& d=dependencies();human.activation.field154=0;human.field204=0;
        auto* state=human_state_owner(human).get();if(!state)throw std::logic_error("Human death requires actual B0");
        human_state_factory(human).retire_dead_state(*state);
        auto& look=human_look_control(human);look.force_frame_commands(true);
        auto publish=[&](std::uint32_t flags){state->publish_frame_command_flags(flags);};
        try{look.frame_movement_animation(0x71,publish,true);}catch(...){look.force_frame_commands(false);throw;}
        look.force_frame_commands(false); //005ABBF0, before proximity and selection.
        const bool offline=(mode_flags()&7u)==7u;
        if(offline||human_remote_ai_enabled()){
            (void)d.proximity.speed_kind(human.field188,1);
            if(human.properties.field19c==2){
                if(offline)player_add_enemy(&human);
                else d.players.auxiliary().selection().acquire(human,mode_flags(),*this);
            }
            if(human.field258!=0xffffffffu){
                d.sight.source().observer_enabled(human_frame_signed_word(human.field258),false);
                d.sight.source().target_mode(human_frame_signed_word(human.field25c),2);
            }
            if(human.field2a4!=0xffffffffu){d.hearing.remove_listener(human_frame_signed_word(human.field2a4));human.field2a4=0xffffffffu;}
        }
        if(human.properties.field19c==1||human.properties.field19c==4){
            if(offline){if(d.players.control().human_roster().contains(&human)){
                notify_dead_roster_human(human);
                d.startup.select_game_secondary_music(7);
            }}else if(human_remote_ai_enabled())notify_dead_auxiliary_human(human);
        }
        if(human.field160&&human.field160->base().type()==4)human_pose_dead_car_controls(human.field160);
        if(human.formation&&human.formation->has_slots())for(unsigned n=6;n;){ //00595710, reverse and reread the actual owner.
            if(!human.formation||!human.formation->has_slots())break;
            auto* member=human.formation->slots()[--n].member;
            if(member&&member!=human.formation->leader()&&member->loading_state&&member->loading_state->words().template get<std::uint32_t>(0x48)==3){
                auto* member_state=human_state_owner(*member).get();if(!member_state)throw std::logic_error("Death formation lost its B0 owner");
                human_state_factory(*member).notify_dead_formation(*member_state);
            }
        }
        retire_human_formation(human); //005A9120, after all member notifications.
    }
    template<class State>void abort_human_dead_action(State& state){complete_human_state_action<false>(state,false,*this,*this);}
    template<class Human>void drop_human_dead_inventory(Human& human){auto& d=dependencies();drop_human_death_inventory(human,d.world_actors.items(),*this,d.scene.frames());}
    template<class Player>void retire_dead_player_models(Player& state){
        if((mode_flags()&7u)==7u&&human_is_player(*state.human()))
            dependencies().first_person.models().bind_weapon(mode_flags(),typename Dependencies::ActorFrames::Frame{},typename Dependencies::ActorFrames::Frame{},-1);
    }
    std::uint32_t human_player_objective_team(){ //0069E070 then00730530; this is game78, NOT local peer00982104.
        auto& d=dependencies();std::uint32_t identity=0xffffffffu;
        if(d.session.words.template get<std::uint8_t>(0x71)){
            const auto selected=d.session.words.template get<std::int32_t>(0x78);
            if(selected!=-1&&d.scene.frames().scene_actor_identity(selected))identity=static_cast<std::uint32_t>(selected);
        }
        const auto& records=d.startup.game_control_records().records;
        return identity<records.size()?records[identity].word(0x64):2u;
    }
    void human_player_objective_status(std::int32_t identity,std::int32_t status){dependencies().script_link.set_human_objective_status(identity,status);}
    template<class Player>void retire_dead_player_objective(Player&){finish_human_player_death_objective(dependencies().session.mission,*this);}
    template<class Human>void notify_dead_roster_human(Human& human){ //0045E790, no roster removal or Human deletion.
        auto& d=dependencies();auto& players=d.players.control();if(!players.human_roster().contains(&human))return;
        const auto& enemies=human.perception.lists()[0];
        for(auto n=enemies.size();n;){auto* enemy=enemies.at(--n);player_remove_enemy(enemy);}
        d.hud.retire_dead_human_marker(&human); //0049B960 has no HUD-flags gate.
        if(players.previous_player()==&human)players.previous_index()=-1; //0045DCC0(-1), genuine clear-only branch.
        for(auto n=players.human_roster().size();n;){
            auto* member=players.human_roster().at(static_cast<std::int32_t>(--n));
            if(!member)throw std::logic_error("Death roster lost its Human borrower");
            if(human_frame_signed_word(member->properties.stats_ec[0])>0&&member->properties.field19c!=4)return;
        }
        if((mode_flags()&7u)==7u)d.script_link.fail_human_mission(); //0069BED0(false), SAME objective owner and HUD hint.
    }
    template<class Human>void notify_dead_auxiliary_human(Human& human){ //0045FE10, no roster removal/HUD marker retirement.
        auto& auxiliary=dependencies().players.auxiliary();if(!auxiliary.human_roster().contains(&human))return;
        const auto& enemies=human.perception.lists()[0];
        for(auto n=enemies.size();n;){auto* enemy=enemies.at(--n);if(!enemy)throw std::logic_error("Auxiliary death lost its actual enemy borrower");
            auxiliary.selection().release(selection_human(*enemy),mode_flags(),*this);
        }
    }
    void human_pose_dead_car_controls(void* actor){
        auto& car=human_pose_car(actor);if(!car.loading)throw std::logic_error("Human death requires actual Car controls owner");
        car.loading->dead_human_controls();
    }
    template<class Human>void notify_human_damage(Human& human,std::uint32_t kind,std::uint32_t hit_kind,std::int32_t health,Human* source,const HumanVector& point,const HumanVector& direction){
        auto* state=human_state_owner(human).get();if(!state)throw std::logic_error("Human damage requires actual B0 receiver");
        human_state_factory(human).notify_damage(*state,kind,hit_kind,health,source,point,direction);
    }
    template<class Entry>void stop_human_item_voice(Entry entry){ //007D6610.
        auto& item=dependencies().world_actors.items().runtime_item(entry);
        if(item.field5c&&human_frame_signed_word(item.field5c-game_word28())>200&&item.fielda8!=0xffffffffu){
            auto voice=human_frame_signed_word(item.fielda8);dependencies().startup.stop_game_sound_voice(voice);
        }
        item.fielda8=0xffffffffu;
    }
    template<class Human>void reset_human_item_player_mode(Human& human){ //005A8190(0), actual005A0890 gate.
        if(!human_is_player(human))return;
        auto* state=human_state_owner(human).get();if(!state)throw std::logic_error("Human item removal requires its actual Player B0");
        human_state_factory(human).reset_item_player_mode(*state);
    }
    template<class Human>void notify_human_inventory_changed(Human& human){
        if(dependencies().players.control().current_player()==&human)dependencies().hud.inventory_changed();
    }
    template<class Human,class Entry>void retire_human_inventory_model(Human& human,Entry entry,unsigned slot){ //007DBD80.
        auto& d=dependencies();auto& items=d.world_actors.items();auto& item=items.runtime_item(entry);
        const auto model=item.model(); //Obtain model before changing its holder or equipment borrowers.
        detach_human_inventory_entry(human,entry,slot,items,*this);
        if(!items.equipment_notifications_enabled())return;
        auto* equipment=human_inline(human).equipment();if(!equipment)throw std::logic_error("Human item retirement requires actual230");
        if(slot==3)human_frame_item_action_fpv(human,20);
        if(slot==0||slot==1||slot==2||slot==3||slot==5)equipment->transfers().removed(entry,static_cast<std::int32_t>(slot),nullptr,0);
        auto& state=equipment->state();
        if(model){
            auto& frames=d.scene.frames();item.flag1c=true;
            frames.human_equipment_link(model,typename Dependencies::ActorFrames::Frame{},0);frames.object_visible(model,true);
            const unsigned kind=slot==0?0u:slot==2?2u:slot==5?1u:3u;
            auto position=human_retired_item_position(state,kind,frames);
            const bool prone=slot==3&&human_inline(human).frame_posture_mode()==2;
            if(prone||slot==5){
                const auto& forward=human.base().world_direction();
                for(unsigned axis=0;axis!=3;++axis){
                    const float offset=forward[axis]*.25f;
                    position[axis]=static_cast<float>((static_cast<long double>(position[axis])+(prone&&axis==1?.44999998807907104f:0.f))-offset);
                }
            }
            frames.physics_publish_position(model,position);frames.update_scene_object(model);frames.place_human_reaction_model(model);
        }
        if(slot==3){state.words().set(0xd4,std::int32_t{-10000});state.model(0,typename Dependencies::ActorFrames::Frame{});
            state.words().set(0x30,std::int32_t{-1});state.words().set(0x34,std::int32_t{-1});}
    }
    template<class Human>bool consume_human_mine_inventory(Human& human,std::int32_t definition){ //00478B90/00478D30 after factory frame release.
        auto& items=dependencies().world_actors.items();
        const auto entry=find_human_inventory_definition(human,items,definition);
        if(!entry)return false;
        //Native performs a SECOND identity lookup005A7B70. Preserve holder
        //priority/duplicate-identity behavior rather than retiring this entry directly.
        const auto identity=items.runtime_item(entry).identity;
        remove_human_inventory_item(human,items,identity,true,*this);return true;
    }
    template<class Human,class Entry>void drop_human_damage_item(Human& human,Entry requested,const HumanVector& velocity){ //005A3490→00478EE0.
        drop_human_damage_item_identity(human,dependencies().world_actors.items().runtime_item(requested).identity,velocity);
    }
    template<class Human>void drop_human_damage_item_identity(Human& human,std::int32_t identity,const HumanVector& velocity){ //005A3490 native identity argument.
        auto& d=dependencies();auto& items=d.world_actors.items();auto& frames=d.scene.frames();
        const auto marker=frames.actor_find(human.base().frame(),"j",0x400);
        if(!marker)throw std::logic_error("Human damage item drop requires its actual j marker");
        FirstPersonHumanItems hand(items);if(const auto weapon=hand.weapon(human);weapon&&items.runtime_item(weapon).identity==identity)stop_human_item_voice(weapon);
        const auto matrix=frames.actor_world_matrix(marker);const HumanVector position{{matrix[12],matrix[13],matrix[14],matrix[15]}};
        const auto entry=find_human_inventory_item_entry(human,items,identity);
        if(!entry)return; //Original emits only a debug diagnostic for missing inventory identity.
        auto& item=items.runtime_item(entry);const auto model=item.model();
        remove_human_inventory_item(human,items,identity,false,*this);
        if(!model)throw std::logic_error("Human damage item transfer requires its actual model");
        auto direction=velocity;
        const auto square=(static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[1])*direction[1];
        normalize_human_ai_shot(direction,square);
        frames.human_loose_item_pose(model,position,direction);
        auto& runtime=d.world_actors.runtime();auto* actor=runtime.create_loose_item(model);
        try{actor->bind_item(entry,velocity);}catch(...){runtime.remove(actor);throw;}
        if((mode_flags()&7u)!=7u){
            if(item.definition().kind()==20)actor->thrown_lifetime(15000);
            auto* state=human_state_owner(human).get();if(!state)throw std::logic_error("Human damage drop lost actual B0 command owner");
            const float square=static_cast<float>((static_cast<long double>(velocity[2])*velocity[2]+static_cast<long double>(velocity[1])*velocity[1])+static_cast<long double>(velocity[0])*velocity[0]);
            std::int32_t bits;std::memcpy(&bits,&square,4);const float speed=bits<0x322bcc77?0.f:std::abs(static_cast<long double>(square)-1.L)<9.99999993922529e-09f?1.f:static_cast<float>(std::sqrt(static_cast<long double>(square)));
            state->queue_drop_command(actor->base().id10(),identity,direction,speed);
        }
        human.activation.flag178=true;
    }
    template<class Human>void present_human_dropped_item(Human& human,const HumanDropCommand& command){ //005A3750→004790C0, existing owner case.
        if(mode_flags()&1u)return;
        auto& d=dependencies();auto& items=d.world_actors.items();auto& frames=d.scene.frames();
        const auto marker=frames.actor_find(human.base().frame(),"j",0x400);if(!marker)throw std::logic_error("Presented Human drop requires its actual j marker");
        const auto matrix=frames.actor_world_matrix(marker);const HumanVector position{{matrix[12],matrix[13],matrix[14],matrix[15]}};
        const auto entry=find_human_inventory_item_entry(human,items,command.item);
        if(!entry){human.activation.flag178=true;return;} //004790C0's diagnostic returns NULL;005A3750 still flags178.
        const auto model=items.runtime_item(entry).model();remove_human_inventory_item(human,items,command.item,false,*this);
        if(!model){human.activation.flag178=true;return;} //Native missing model returns NULL after inventory retirement.
        HumanVector velocity=command.direction;for(unsigned axis=0;axis!=3;++axis)velocity[axis]*=command.speed;
        auto direction=velocity;normalize_human_ai_shot(direction,(static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[1])*direction[1]);
        frames.human_loose_item_pose(model,position,direction);
        auto& runtime=d.world_actors.runtime();auto* actor=runtime.create_loose_item(model,command.actor);
        try{actor->bind_item(entry,velocity);}catch(...){runtime.remove(actor);throw;}
        human.activation.flag178=true;
    }
    template<class State,class Human>void human_player_damage_feedback(State& state,std::uint32_t kind,Human* source){ //0053DB90 after its command/drop producers.
        auto* human=state.human();if(!human)throw std::logic_error("Player damage requires actual Human");
        if(human_is_player(*human)){
            if(human_view_offset_enabled()){auto random=[&]{return human_frame_random_word();};dependencies().hud.show_human_damage(random);}
            if(kind==1||kind==2||kind==3)perturb_player_damage_direction(state,*this,dependencies().scene.frames());
        }
        if((mode_flags()&7u)==7u&&source&&dependencies().sight.human_sight_hostile(source,human)){
            auto& d=dependencies();auto& frames=d.scene.frames();const auto origin=frames.actor_world_matrix(human->base().frame());
            auto& roster=d.players.control().human_roster();
            for(auto n=roster.size();n;){ //0045F110, reverse same roster, strict30m test.
                auto* member=roster.at(static_cast<std::int32_t>(--n));
                if(!member||human_frame_signed_word(member->properties.stats_ec[0])<1||!member->loading_state||
                        member->loading_state->words().template get<std::uint32_t>(0x48)!=3)continue;
                const auto action=member->loading_state->player_handover().action;if(action!=14&&action!=15)continue;
                const auto position=frames.actor_world_matrix(member->base().frame());
                const float x=origin[12]-position[12],y=origin[13]-position[13],z=origin[14]-position[14];
                const auto square=(static_cast<long double>(y)*y+static_cast<long double>(x)*x)+static_cast<long double>(z)*z;
                if(square<900.f){auto* current=human_state_owner(*member).get();if(!current)throw std::logic_error("Player damage notification lost actual B0");
                    human_state_factory(*member).notify_nearby_player_damage(*current);}
            }
        }
    }
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
    void human_action_request_door(void* actor,std::uint32_t command){ //00751480 case1, real005FF4B0(command,1,0,0).
        human_navigation_door(actor).change_state(command,true,false,false,dependencies().world_actors);
    }
    bool human_emplacement_claimed(void* actor,std::int32_t seat){
        if(human_pose_actor_type(actor)==4){auto& car=human_pose_car(actor);if(!car.loading)throw std::logic_error("Seat claim requires actual Car loading owner");return car.loading->seat_claimed(seat);}
        using Weapon=StaticWeaponSceneActor<typename Dependencies::ActorFrames>;auto* weapon=dynamic_cast<Weapon*>(&human_pose_actor(actor));
        if(!weapon||!weapon->loading)throw std::logic_error("Seat claim requires actual mounted actor owner");
        return weapon->loading->seat_claimed(seat);
    }
    template<class Human,class Actor>bool human_command_enter_emplacement(Human& human,Actor* actor,std::int32_t seat){ //00585D10 reached cases4/15.
        if(!actor)return false;
        const auto kind=actor->base().type();
        if(kind!=4&&kind!=0x15)throw std::runtime_error("Human entry requires the actual selected actor E0 receiver");
        if(kind==4){
            if((mode_flags()&7u)!=7u&&human_emplacement_claimed(actor,seat))return false;
            auto& car=human_pose_car(actor);if(!car.loading)throw std::logic_error("Human entry requires actual Car loading owner");
            const bool offline=(mode_flags()&7u)==7u;
            for(auto n=car.occupants.size();n;){auto* occupant=car.occupants[--n];if(!occupant)continue;auto& other=selection_human(*occupant);
                if(offline){if(dependencies().sight.human_sight_hostile(&human,&other)&&human_frame_signed_word(other.properties.stats_ec[0])>0)return false;}
                else if(human_frame_signed_word(other.properties.stats_ec[0])>0&&script_human_player_controlled(other,mode_flags())&&!human_selected_suppressed(other)){
                    const auto first=human_specialization(other,false);if(first!=human_specialization(human,false))return false;
                }
            }
            car.loading->claim_seat(seat,true);human_look_control(human).interaction_command(actor,0x402u,static_cast<std::uint32_t>(seat));return true;
        }
        using Weapon=StaticWeaponSceneActor<typename Dependencies::ActorFrames>;auto* weapon=dynamic_cast<Weapon*>(actor);
        if(!weapon||!weapon->loading)throw std::logic_error("Human entry requires actual StaticWeapon owner");
        if(weapon->loading->seat_claimed(seat))return false;
        weapon->loading->claim_seat(seat,true);human_look_control(human).interaction_command(actor,0x406u,static_cast<std::uint32_t>(seat));return true;
    }
    template<class State>void human_guard_enter_emplacement(State& state,void* actor){ //00571C10 executes DC BEFORE allocating its separate kind3/94 copy, ignoring status.
        HumanPacketCommand command;command.kind=3;command.fields.set(8,3u);command.fields.set(0xc,human_pose_actor(actor).base().id10());command.fields.set(0x10,0u);
        (void)composition(*state.human()).state_factory().common_command(state,command);state.queue_frame_command(std::move(command),true);
    }
    template<class Human>void human_command_exit_emplacement(Human& human){ //00585A60, attached Car/StaticWeapon branch.
        auto* actor=human.field160;if(!actor)throw std::runtime_error("Unattached exit requires actual00585A60 special-state continuation");
        const auto seat=static_cast<std::int32_t>(human.field164);const auto kind=actor->base().type();
        if(kind==4){auto& car=human_pose_car(actor);if(!car.loading)throw std::logic_error("Car exit command requires actual seat owner");car.loading->claim_seat(seat,false);human_look_control(human).interaction_command(actor,0x403u,human.field164);if((mode_flags()&7u)!=7u)throw std::runtime_error("Car exit requires actual non7 map notification");return;}
        using Weapon=StaticWeaponSceneActor<typename Dependencies::ActorFrames>;auto* weapon=dynamic_cast<Weapon*>(actor);
        if(!weapon||!weapon->loading)throw std::runtime_error("Attached exit requires actual selected actor E4 receiver");
        weapon->loading->claim_seat(seat,false);human_look_control(human).interaction_command(actor,0x407u,human.field164);
    }
    template<class State>void human_guard_exit_emplacement(State& state){ //0054DC10→00571C10; immediate DC then separate deferred kind3 copy.
        HumanPacketCommand command;command.kind=3;command.fields.set(8,5u);command.fields.set(0xc,0u);command.fields.set(0x10,0u);
        (void)composition(*state.human()).state_factory().common_command(state,command);state.queue_frame_command(std::move(command),true);
    }
    template<class State,class Runtime>void human_navigation_request_ladder(State& state,void* actor,Runtime& runtime){
        HumanPacketCommand command;command.kind=2;command.fields.set(8,0x1eu);
        command.fields.set(0xc,human_pose_actor(actor).base().id10());
        (void)composition(*state.human()).state_factory().common_command(state,command,runtime);
    }
    template<class State>void human_navigation_request_door(State& state,void* actor){ //00587540→00586200 subtype1B.
        HumanPacketCommand command;command.kind=2;command.fields.set(8,0x1bu);command.fields.set(0xc,human_pose_actor(actor).base().id10());
        (void)composition(*state.human()).state_factory().common_command(state,command);
    }
    template<class Human,class Actor>void human_command_open_door(Human& human,Actor* actor,bool direct){
        if(direct){if(actor)human_navigation_door(actor).change_state(1,true,false,false,dependencies().world_actors);return;}
        human_look_control(human).interaction_command(actor,1u,1u); //005ABD90 kind6, actual E090/94/08.
    }
    auto* human_floor_actor(std::uint32_t identity){ //005AE230, SAME B4 command-neighbour lookup as Scene2 movement.
        using Human=typename Dependencies::Composition::Human;
        auto* actor=dependencies().scene.frames().scene_actor_command_identity(human_frame_signed_word(identity));
        auto* human=dynamic_cast<Human*>(actor);if(!human)throw std::logic_error("AI floor path neighbour has no actual Human owner");return human;
    }
    template<class Human>bool human_floor_actor_suspended(const Human& human){
        const auto* state=human.loading_state;return state&&state->words().template get<std::uint32_t>(0x48)==5&&state->derived_loading_word(0x1d2)!=0;
    }
    template<class Human>bool human_floor_actor_attached(const Human& human){return human.field160!=nullptr;}
    template<class Human>bool human_floor_actor_state(const Human& human){return human.loading_state!=nullptr;}
    template<class Human>std::int32_t human_floor_actor_health(const Human& human){return human_frame_signed_word(human.properties.stats_ec[0]);}
    template<class Human>HumanVector human_floor_actor_velocity(const Human& human){return human.loading_state->words().template get<HumanVector>(0x30);}
    bool human_floor_detour_ground(HumanVector position){ //00580790, actual007183A0 ray, original floor height is not copied back.
        position[1]+=.30000001192092896f;return human_geometry_ray(position,HumanVector{{0,-10,0,0}},0xffffffffu,0u).has_value();
    }
    auto human_ai_resolve_actor(std::uint32_t handle){ //005ACA50→00475DD0, Scene2 identity.
        auto& scene=dependencies().scene.frames();
        return handle&0x80000000u?nullptr:scene.scene_actor_identity(static_cast<std::int32_t>(handle));
    }
    template<class Actor>bool human_scene_actor_matches(Actor& actor){return dependencies().scene.frames().scene_actor_identity(actor.base().id10())==&actor;} //00475DD0.
    auto& human_perception_record_pool(){return dependencies().startup.human_perception_record_pool();}
    bool human_face_quality_enabled(){return dependencies().startup.human_face_quality_enabled();}
    bool human_geometry_effect_quality(){return dependencies().startup.profile_settings().quality_byte(5)!=0;}
    bool human_surface_quality_enabled(){return dependencies().startup.profile_settings().quality_byte(2)!=0;}
    void emit_human_surface_mark(const HumanVector& position,const HumanVector& normal,float limit){
        auto& d=dependencies();
        emit_shot_surface_mark(d.startup.shot_manager_process(),d.scene.frames(),*this,position,normal,limit);
    }
    void retain_human_surface_position(const HumanVector& position){
        dependencies().presentation_shared.projection_models().retain_surface_position(position,mode_flags());
    }
    std::uint32_t human_projection_enabled(){return dependencies().presentation_shared.projection_models().enabled();}
    bool near_human_surface_position(const HumanVector& position){return dependencies().presentation_shared.projection_models().near_surface_position(position,mode_flags());}
    std::int32_t human_footstep_sound(std::uint32_t material){return human_frame_signed_word(dependencies().car_definitions.materials().at(material).word(0x20));}
    void emit_human_footstep_sound(const frontend::SpatialSoundRequest& request){(void)dependencies().sound.emit(request);}
    template<class Human,class Frame>void emit_human_footprint(Human& human,unsigned index,bool left,const HumanVector& position,
            float angle,Frame parent,float alpha,float colour){
        auto& d=dependencies();auto& shared=d.presentation_shared;auto& frames=d.scene.frames();const auto model=human.base().frame();
        bool hidden=false;
        const auto hide=[&]{if(frames.human_control_visible(model)){frames.object_visible(model,false);hidden=true;}};
        try{shared.projection_models().emit_footprint(human.base().id10(),index,left,shared.footprint_texture(index,left),position,angle,parent,alpha,colour,*this,hide);}
        catch(...){const auto failure=std::current_exception();if(hidden)try{frames.object_visible(model,true);}catch(...){frames.record_actor_cleanup_failure(std::current_exception());}std::rethrow_exception(failure);}
        if(hidden)frames.object_visible(model,true);
    }
    void* human_geometry_containing_actor(const HumanVector& point,std::uint32_t mask){
        auto& d=dependencies();
        if(!d.session.loaded)throw std::logic_error("Human geometry point query requires actual loaded World");
        using Scene=std::remove_reference_t<decltype(d.scene)>;
        DynamicCollisionPointQueries<typename Scene::Descriptor,typename Scene::Tree::DynamicGrid,typename Dependencies::ActorFrames>
            query(d.scene.buckets(),d.scene.frames());
        const auto* descriptor=query.containing(point,mask);
        if(!descriptor||!(descriptor->flags&0xc0u)||!descriptor->frame)return nullptr;
        return d.sight.sight_actor(descriptor->frame);
    }
    bool presentation_application_fallback(){return human_remote_ai_enabled();} //00785BE0→006B66B0.
    const auto& presentation_mission_name()const{return dependencies().request.preparation.mission_resource;}
    auto& human_face_resources(){return dependencies().faces;}
    std::int32_t human_face_camouflage(){return static_cast<std::int32_t>(dependencies().presentation_shared.settings().terrain);} //Actual0098A6B0.
    auto human_animation_resource(unsigned table,unsigned row){return dependencies().animations.resource(table,row);}
    const auto& human_animation_row(unsigned table,unsigned row){return dependencies().animations.row(table,row);}
    auto& human_animation_arbitration(){return dependencies().animations.arbitration_process();}
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
    auto human_hand_action_model(){auto& d=dependencies();return d.startup.item_hand_models().get(d.scene.frames(),0);} //007D76C0(0), SAME process cards cache.
    template<class Human,class Frame>Human* human_mounted_model_actor(Frame frame){
        auto* actor=dependencies().sight.sight_actor(frame);if(!actor)return nullptr;
        auto* human=dynamic_cast<Human*>(actor);
        if(!human)throw std::runtime_error("Human mount model is bound to a different actor family");
        return human;
    }
    template<class Frame>void restore_human_mount_camera_amount(Frame frame){
        (void)dependencies().scene.effects().model_camera_amount(frame,0.f); //00486510 GEffects virtual50=00486D30.
    }
    auto& human_pose_actor(void* value){
        if(!value)throw std::logic_error("Human attachment pose requires its actual source actor");
        return *static_cast<SceneActor<typename Dependencies::ActorFrames>*>(value);
    }
    auto& human_pose_car(void* value){
        using Car=typename std::decay_t<decltype(dependencies().world_actors)>::Car;
        auto* car=dynamic_cast<Car*>(&human_pose_actor(value));
        if(!car||!car->geometry)throw std::logic_error("Human attachment pose requires actual Car geometry");
        return *car;
    }
    std::uint32_t human_pose_actor_type(void* actor){return human_pose_actor(actor).base().type();}
    auto* human_seat_occupant(void* actor,std::int32_t seat){
        using Human=typename Dependencies::Composition::Human;using Frames=typename Dependencies::ActorFrames;
        SceneActor<Frames>* occupant=nullptr;
        if(human_pose_actor_type(actor)==4){const auto& seats=human_pose_car(actor).occupants;if(seat>=0&&static_cast<std::size_t>(seat)<seats.size())occupant=seats[static_cast<std::size_t>(seat)];}
        else{auto* weapon=dynamic_cast<StaticWeaponSceneActor<Frames>*>(&human_pose_actor(actor));if(!weapon||!weapon->loading)throw std::logic_error("Seat query requires actual StaticWeapon lifetime");occupant=static_cast<SceneActor<Frames>*>(weapon->loading->seat_occupant(seat));}
        auto* human=dynamic_cast<Human*>(occupant);if(occupant&&!human)throw std::logic_error("Seat occupant is not an actual Human");return human;
    }
    template<class Human>void human_seat_assign(void* actor,std::int32_t seat,Human* human){
        if(human_pose_actor_type(actor)==4){auto& car=human_pose_car(actor);if(!car.loading)throw std::logic_error("Seat entry requires actual Car lifetime");car.loading->assign_seat(seat,human);return;}
        auto* weapon=dynamic_cast<StaticWeaponSceneActor<typename Dependencies::ActorFrames>*>(&human_pose_actor(actor));
        if(!weapon||!weapon->loading)throw std::logic_error("Seat entry requires actual StaticWeapon lifetime");weapon->loading->assign_seat(seat,human);
    }
    bool human_seat_exit(void* actor,std::int32_t seat){
        if(human_pose_actor_type(actor)==4){auto& car=human_pose_car(actor);if(!car.loading)throw std::logic_error("Seat exit requires actual Car lifetime");return car.loading->exit_seat(seat);}
        auto* weapon=dynamic_cast<StaticWeaponSceneActor<typename Dependencies::ActorFrames>*>(&human_pose_actor(actor));
        if(!weapon||!weapon->loading)throw std::logic_error("Seat exit requires actual StaticWeapon lifetime");return weapon->loading->exit_seat(seat);
    }
    std::optional<std::uint32_t> human_seat_weapon_kind(void* actor,std::int32_t seat){
        if(human_pose_actor_type(actor)==4){const auto& slot=human_pose_car(actor).fields24b4.at(static_cast<std::size_t>(seat));return slot?slot->loading->weapon_kind():std::nullopt;}
        auto* weapon=dynamic_cast<StaticWeaponSceneActor<typename Dependencies::ActorFrames>*>(&human_pose_actor(actor));
        if(!weapon||!weapon->loading)throw std::logic_error("Seat item query requires actual StaticWeapon lifetime");return weapon->loading->seat_weapon_kind();
    }
    template<class Human>std::int32_t human_seat_health(const Human& human){return human_frame_signed_word(human.properties.stats_ec[0]);}
    template<class Human>void human_seat_clear_frame_command(Human& human){auto* state=human_state_owner(human).get();if(!state)throw std::logic_error("Seat entry requires actual B0");state->publish_frame_command_flags(state->words().template get<std::uint32_t>(0x50)&0xffffff07u);}
    template<class Human,class Actor,class Runtime>void human_seat_displace(Human& human,Actor* actor,std::uint32_t flags,std::uint32_t seat,Runtime& runtime){
        auto& owner=human_inline(human);auto* control=owner.control();if(!control)throw std::logic_error("Seat displacement requires actual DD0");
        control->apply_look_interaction(human_look_control(human),actor,flags,seat,runtime);
    }
    template<class Human>void human_seat_retire_dead(Human& human,const HumanVector& position){auto* control=human_inline(human).control();if(!control)throw std::logic_error("Dead seat displacement requires actual DD0");control->retire_dead_seat(position);}
    void refresh_human_seat_visibility(){dependencies().presentation.refresh_first_person_visibility();}
    void seat_human_direction(void* actor,const HumanVector& direction){auto& human=human_score_actor(actor);auto* state=human_state_owner(human).get();if(!state)throw std::logic_error("Seat direction requires actual B0");state->set_target_direction(direction);state=human_state_owner(human).get();if(!state)throw std::logic_error("Seat direction lost actual B0");state->set_current_direction(direction);}
    HumanVector mounted_human_target(void* actor){auto* state=human_state_owner(human_score_actor(actor)).get();if(!state)throw std::logic_error("Mounted aim requires actual B0");return state->target_direction();}
    void mounted_human_target(void* actor,const HumanVector& value){auto* state=human_state_owner(human_score_actor(actor)).get();if(!state)throw std::logic_error("Mounted aim requires actual B0");state->set_target_direction(value);}
    bool mounted_human_is_player(void* actor){return human_is_player(human_score_actor(actor));}
    std::uint32_t seat_human_state_kind(void* actor){return human_score_state_kind(actor);}
    std::int32_t seat_human_health(void* actor){return human_seat_health(human_score_actor(actor));}
    void seat_human_proximity_kind(void* actor,unsigned kind){(void)dependencies().proximity.speed_kind(human_score_actor(actor).field188,kind);}
    void remove_human_reaction_seat(void* actor,std::int32_t seat){
        if(human_pose_actor_type(actor)==4){auto& car=human_pose_car(actor);if(!car.loading)throw std::logic_error("Human reaction requires actual Car seat owner");car.loading->remove_reaction_seat(seat);return;}
        using Weapon=StaticWeaponSceneActor<typename Dependencies::ActorFrames>;
        auto* weapon=dynamic_cast<Weapon*>(&human_pose_actor(actor));
        if(!weapon||!weapon->loading)throw std::logic_error("Human reaction requires actual attachment seat owner");
        weapon->loading->remove_reaction_seat(seat);
    }
    std::uint32_t human_reaction_seat_flags(void* actor,std::int32_t seat){
        if(human_pose_actor_type(actor)==4){auto& car=human_pose_car(actor);if(!car.loading)throw std::logic_error("Human reaction requires actual Car seat metadata");return car.loading->reaction_seat_flags(seat);}
        using Weapon=StaticWeaponSceneActor<typename Dependencies::ActorFrames>;
        auto* weapon=dynamic_cast<Weapon*>(&human_pose_actor(actor));
        if(!weapon||!weapon->loading)throw std::logic_error("Human reaction requires actual attachment seat metadata");
        return weapon->loading->reaction_seat_flags(seat);
    }
    auto human_pose_actor_model(void* actor){return human_pose_actor(actor).base().frame();}
    HumanVector human_pose_car_velocity(void* actor){return dependencies().scene.car_callbacks().car_position4a0(&human_pose_car(actor));} //Actual native4A0 velocity.
    std::uint32_t human_pose_car_state(void* actor){return static_cast<std::uint32_t>(human_pose_car(actor).occupants.size());} //004D0E30, native1040 seat count.
    void human_pose_clear_car_steering(void* actor){ //007ADD30→004F2420(0), SAME live steering owner.
        auto& car=human_pose_car(actor);
        if(!car.loading)throw std::logic_error("Human attachment retirement requires actual Car runtime");
        request_car_steering(car.loading->steering_state(),0.f);
    }
    bool human_pose_actor_has_control(void* actor,std::uint32_t seat){ //Car004DA020, StaticWeapon00604D20.
        if(human_pose_actor_type(actor)==4)return static_cast<bool>(human_pose_car(actor).fields24b4.at(seat));
        if(human_pose_actor_type(actor)==21)return seat==0;
        throw std::logic_error("Human attachment control query requires Car or StaticWeapon");
    }
    template<class Actor>std::array<float,3> human_grenade_lead_position(Actor& actor,float time){ //00558390 snapshots native1B0 before its model query.
        const auto velocity=selection_human(actor).activation.vector1b0;const auto p=sight_position(actor.base().frame());std::array<float,3> result;for(unsigned n=0;n<3;++n)result[n]=static_cast<float>(static_cast<long double>(time)*velocity[n]+p[n]);return result;
    }
    std::int32_t human_grenade_animation_duration(unsigned posture){ //005AC200→0075CC30, id4F and type0 use original98A47C/table2.
        if(!dependencies().animations.available())return -1; //Original NULL table2 returns-1, without manufacturing a clip.
        const auto clip=human_animation_resource(2,79u*3+posture);return clip?dependencies().scene.frames().human_clip_duration(clip):-1;
    }
    template<class Human>bool human_guard_mounted(Human& human){ //00548590.
        auto* actor=human.field160;if(!actor)return false;switch(actor->base().type()){case 4:case 0x14:case 0x15:case 0x22:return human_pose_actor_has_control(actor,human.field164);default:return false;}
    }
    template<class Human,class Entry>void human_guard_drop_hand(Human& human,Entry entry,const HumanVector& direction){ //005A2C50→Guard70/00587BE0→005A3490.
        const auto x=static_cast<float>(static_cast<long double>(human_sound_random())*.20000000298023224f);
        const auto y=static_cast<float>(static_cast<long double>(human_sound_random())*.10000000149011612f);
        const auto z=human_sound_random();HumanVector velocity{};
        velocity[0]=static_cast<float>((static_cast<long double>(x)+direction[0])*2.5f);
        velocity[1]=static_cast<float>((static_cast<long double>(y)+direction[1])*2.5f);
        const float last=static_cast<float>(static_cast<long double>(z)*.20000000298023224f+direction[2]);velocity[2]=last*2.5f;
        drop_human_damage_item(human,entry,velocity);human.activation.flag178=true;
    }
    template<class State>void human_attachment_frame_inputs(State& state){ //00583BC0, actual Human state virtual00.
        auto& human=*state.human();auto* actor=human.field160;if(!actor)throw std::logic_error("Attached state input requires actual actor");
        if(!human_is_player(human)&&human.field164!=0)return;
        const auto* first=state.first_frame_inputs();const auto* second=state.second_frame_inputs();
        if(!first||!second)throw std::logic_error("Attached state input requires actual74/78 buffers");
        if(actor->base().type()==4){auto& car=human_pose_car(actor);if(!car.loading)throw std::logic_error("Car driver input requires actual runtime");car.loading->driver_frame_inputs(*first,*second);}
        else if(actor->base().type()==0x14)throw std::runtime_error("Attached aircraft input requires actual005E0DD0 receiver");
    }
    void human_emplacement_frame_inputs(void* actor,std::uint32_t seat,const HumanFrameInputBuffer& first,const HumanFrameInputBuffer& second){ //Actual ActorC0.
        if(human_pose_actor_type(actor)==4){auto& control=human_pose_car(actor).fields24b4.at(seat);if(control)control->loading->frame_inputs(first,second);return;}
        using Weapon=StaticWeaponSceneActor<typename Dependencies::ActorFrames>;auto* weapon=dynamic_cast<Weapon*>(&human_pose_actor(actor));
        if(!weapon||!weapon->loading)throw std::logic_error("Mounted input requires actual Car/StaticWeapon runtime");
        weapon->loading->seat_frame_inputs(static_cast<std::int32_t>(seat),first,second);
    }
    template<class Human>bool human_guard_root_sector(Human& human){return human_inline(human).scene_sector()==dependencies().scene.frames().camera_root_sector();} //00553CE0.
    template<class Human>bool human_guard_clear_neighbours(Human& human){ //005AE890.
        const auto origin=sight_position(human.base().frame());if(!human.command)throw std::logic_error("Guard prone query requires actual command owner");
        const auto& ids=human.command->lists[0];for(auto i=ids.size();i;){auto* actor=human_ai_resolve_actor(ids[--i]);if(!actor)continue;const auto point=sight_position(actor->base().frame());const auto x=static_cast<long double>(point[0])-origin[0],y=static_cast<long double>(point[1])-origin[1],z=static_cast<long double>(point[2])-origin[2];if((x*x+y*y)+z*z<4.f)return false;}return true;
    }
    template<class Actor>bool human_guard_prone_ray(Actor& actor,const HumanVector& origin){ //00552190.
        const auto target=sight_position(selection_human(actor).head_marker);auto delta=target;for(unsigned n=0;n<3;++n)delta[n]=static_cast<float>(static_cast<long double>(target[n])-origin[n]);return !human_geometry_ray(origin,delta,3,0);
    }
    template<class Actor>HumanVector human_emplacement_target(Actor& actor){ //0054D6B0, native W is unclaimed outside the standing branch.
        auto& human=selection_human(actor);if(!human.loading_state)throw std::logic_error("Guard emplacement target requires actual B0");
        const auto flags=human.loading_state->words().template get<std::uint32_t>(0x50);
        auto point=sight_position(flags&6u?human.head_marker:human.base().frame());if(flags&6u)point[1]=static_cast<float>(static_cast<long double>(point[1])-.30000001192092896f);
        for(unsigned n=0;n<3;++n){const float lead=human.activation.vector1b0[n]*.125f;point[n]=static_cast<float>(static_cast<long double>(point[n])+lead);}return point;
    }
    HumanVector human_emplacement_position(void* actor,std::uint32_t seat=0){ //Actual Actor A8.
        if(human_pose_actor_type(actor)==4){const auto& control=human_pose_car(actor).fields24b4.at(seat);return control?control->loading->mounted_position():HumanVector{};}
        using Weapon=StaticWeaponSceneActor<typename Dependencies::ActorFrames>;auto* weapon=dynamic_cast<Weapon*>(&human_pose_actor(actor));
        if(!weapon||!weapon->loading)throw std::logic_error("Guard emplacement requires actual mounted actor runtime");
        return weapon->loading->mounted_position();
    }
    bool human_emplacement_can_target(void* actor,std::uint32_t seat,const HumanVector& target){ //Actual Actor BC.
        if(human_pose_actor_type(actor)==4){const auto& control=human_pose_car(actor).fields24b4.at(seat);return control&&control->loading->can_target(target);}
        using Weapon=StaticWeaponSceneActor<typename Dependencies::ActorFrames>;auto* weapon=dynamic_cast<Weapon*>(&human_pose_actor(actor));
        if(!weapon||!weapon->loading)throw std::logic_error("Guard capability requires actual mounted actor runtime");
        return weapon->loading->can_target(target);
    }
    std::uint32_t human_emplacement_ammunition(void* actor,std::int32_t seat){return dependencies().first_players.seat_presentation().current(human_pose_actor(actor),seat);} //SAME actual9C/004C6550 owner already bound for occupied actors.
    template<class Human,class Actor>std::uint32_t human_guard_sight_feedback(Human& human,Actor& actor,bool& visible){ //0046F320, real existing observer relation.
        return dependencies().sight.source().human_relation_feedback(static_cast<std::int32_t>(human.field258),static_cast<std::int32_t>(selection_human(actor).field25c),nullptr,&visible);
    }
    template<class Human>bool human_guard_mounted_target_clear(Human& human,const HumanVector& target){ //0054F910, dynamic-only006F78F0.
        const auto origin=human_emplacement_position(human.field160,human.field164);HumanVector delta{};for(unsigned n=0;n<3;++n)delta[n]=static_cast<float>(static_cast<long double>(target[n])-origin[n]);
        auto& d=dependencies();if(!d.session.loaded)throw std::logic_error("Mounted target ray requires actual loaded World/tree");
        auto queries=d.scene.door_queries();using Descriptor=typename std::remove_reference_t<decltype(d.scene)>::Descriptor;
        const Descriptor* hit=nullptr;auto& control=human_inline(human);control.exclude_query(false);
        try{hit=queries.nearest(target,delta,0,0);}catch(...){control.restore_query(false);throw;}
        control.restore_query(false); //Native restores BEFORE inspecting descriptor48.
        if(!hit||!hit->actor)return true;
        auto& actor=human_pose_actor(hit->actor);const auto kind=actor.base().type();
        if(kind==0x19)return false;
        if(kind!=4)return true;
        auto& car=human_pose_car(&actor);const bool offline=(mode_flags()&7u)==7u;
        for(auto n=car.occupants.size();n;){auto* occupant=car.occupants[--n];if(!occupant)continue;auto& other=selection_human(*occupant);
            if(offline){if(d.sight.human_sight_hostile(&human,&other)&&human_frame_signed_word(other.properties.stats_ec[0])>0)return true;}
            else if(human_frame_signed_word(other.properties.stats_ec[0])>0&&script_human_player_controlled(other,mode_flags())&&!human_selected_suppressed(other)){const auto side=human_specialization(other,false);if(side!=human_specialization(human,false))return true;}
        }
        return false; //004DDF70 negates the SAME004DDE20 source/occupant policy.
    }
    HumanVector human_pose_actor_direction(void* actor,std::uint32_t seat){ //Actual virtualA4.
        if(human_pose_actor_type(actor)==4){
            const auto& control=human_pose_car(actor).fields24b4.at(seat);
            return control?control->loading->aim_direction():HumanVector{}; //004DA150→004C5870, actual null-control output.
        }
        using Weapon=StaticWeaponSceneActor<typename Dependencies::ActorFrames>;
        auto* weapon=dynamic_cast<Weapon*>(&human_pose_actor(actor));
        if(!weapon||!weapon->loading)throw std::logic_error("Human attachment aim requires actual StaticWeapon runtime");
        return weapon->loading->aim_direction(); //00604D70→004C5870.
    }
    std::array<float,4> human_pose_actor_limits(void* actor,std::uint32_t seat){ //Actual virtualB8.
        if(human_pose_actor_type(actor)==4){
            const auto& control=human_pose_car(actor).fields24b4.at(seat);
            if(!control)throw std::logic_error("Human pose blending requires actual populated Car gun control limits"); //Native null branch leaves all four outputs unwritten.
            return control->loading->limits(); //004DA1F0, same128..134 control fields.
        }
        using Weapon=StaticWeaponSceneActor<typename Dependencies::ActorFrames>;
        auto* weapon=dynamic_cast<Weapon*>(&human_pose_actor(actor));
        if(!weapon)throw std::logic_error("Human pose limits require Car or StaticWeapon");
        const auto& control=weapon->control;
        return {{control.field128,control.field12c,control.field130,control.field134}}; //00604DD0, actual embedded160+128.
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
    std::uint32_t player_item_spawn_delay_ms(){ //Actual game configuration250, DWORD multiplication before signed comparison.
        return static_cast<std::uint32_t>(dependencies().startup.game_configuration().rules228[10])*1000u;
    }
    bool player_item_controls_available(){return dependencies().first_person.view().item_controls_available();}
    std::int32_t player_mission_ordinal()const{return dependencies().session.words.template get<std::int32_t>(0x3c);}
    bool player_recoil_disabled(){return dependencies().startup.game_console_variables().integer(6)!=0;} //00720B80(6).
    bool player_zoom_enabled(){return dependencies().first_person.view().zoom_enabled();}
    auto& player_item_sound_request(){return dependencies().startup.item_sound_request();}
    template<class Human>std::int32_t player_shot_stamina(const Human& human){return human_hud_stamina_percent(human);}
    void player_item_fpv_request(std::int32_t animation){dependencies().first_person.view().request(mode_flags(),animation,0);}
    void player_item_reset_zoom(){dependencies().first_person.view().restore_game_zoom(mode_flags());}
    template<class Player,class Item>void prepare_player_item_shot(Player& state,Item& item,HumanVector& position,HumanVector& direction){
        if(human_view_offset_enabled()){position=human_projectile_position();direction=human_projectile_direction();return;}
        auto& d=dependencies();auto shot=item.shot_frame();if(!shot)shot=item.model();else shot=item.shot_frame();
        if(!shot)throw std::runtime_error("Player fire reached native missing muzzle/model frame");
        const auto matrix=d.scene.frames().actor_world_matrix(shot);position={{matrix[12],matrix[13],matrix[14],matrix[15]}};
        HumanVector camera{};
        if(!d.camera.query_vector(0,camera))throw std::runtime_error("Player fire camera0 output has no native producer");
        if(!d.camera.query_direction(1,direction))throw std::runtime_error("Player fire camera1 output has no native producer");
        auto* human=state.human();if(!human)throw std::logic_error("Player shot query requires its actual Human");
        const auto hit=[&]{
            auto& owner=human_inline(*human);owner.exclude_query(false);
            struct Restore{std::remove_reference_t<decltype(owner)>& owner;~Restore(){owner.restore_query(false);}}restore{owner};
            HumanVector displacement=direction;for(unsigned axis=0;axis!=3;++axis)displacement[axis]=static_cast<float>(static_cast<long double>(direction[axis])*500.f);
            return human_geometry_ray(camera,displacement,0x8001u,0u,false);
        }(); //Native restores exclusions BEFORE inspecting the result.
        if(hit){
            HumanVector aim=direction;
            for(unsigned axis=0;axis!=2;++axis)aim[axis]=static_cast<float>(static_cast<long double>(hit->position[axis])-position[axis]);
            const auto z=static_cast<long double>(hit->position[2])-position[2];aim[2]=static_cast<float>(z);
            const auto square=(z*aim[2]+static_cast<long double>(aim[1])*aim[1])+static_cast<long double>(aim[0])*aim[0];
            if(square>9.f){aim[3]=camera[3];normalize_human_ai_shot(aim,static_cast<float>(square));direction=aim;}
        }
    }
    template<class Player>std::uint32_t player_item_reload_duration(Player& state){
        const auto entry=state.item();if(!entry)throw std::runtime_error("Player reload notification reached native missing cached item");
        return human_frame_output_item(entry).reload_duration();
    }
    template<class Player>void player_item_alignment(Player& state,float amount){
        auto* human=state.human();if(!human)throw std::logic_error("Player item alignment requires its actual Human");
        set_human_sight_alignment(*human,amount,mode_flags());
    }
    template<class Player>void player_item_first_shot(Player& state){
        auto* human=state.human();if(!human)throw std::logic_error("First Player shot requires its actual Human");
        using Factory=std::remove_reference_t<decltype(composition(*human).state_factory())>;
        auto& d=dependencies();d.startup.players_process().template first_player_shot<typename Factory::Active>(d.players.control(),mode_flags());
    }
    template<class Player>void publish_player_item_input(Player& state,const PlayerItemNotification& notification){
        auto& d=dependencies();auto& items=d.world_actors.items();const auto entry=state.item();
        auto* item=entry?&items.runtime_item(entry):nullptr;
        hd2::game::publish_player_item_input(state,d.startup.game_human_inputs(),notification,mode_flags(),item);
    }
    template<class Player>void set_player_frame_mode(Player& state,std::int32_t mode){
        if(!state.human())throw std::logic_error("Player mode requires the actual bound Human");
        dependencies().presentation.set_player_mode(state,*state.human(),mode,mode_flags());
    }
    template<class Player>void notify_player_item_readiness(Player& state,std::uint32_t command){
        if(!state.human())throw std::logic_error("Player readiness requires the actual bound Human");
        composition(*state.human()).state_factory().notify_inline_frame_command(state,command);
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
        //SAME process0098B02C, now also published by console007205E2.
        return reload_human_weapon(item,human,ammunition,retain_last,dependencies().startup.game_console_variables().happy_ammunition(),*this);
    }
    auto& human_reload_items(){return dependencies().world_actors.items();}
    template<class Human>std::uint32_t human_reload_animation_family(Human& human){ //00791C10, SAME hand holder and lazy Item virtual40.
        auto* storage=human.base().storage();auto* holder=storage?storage->holder(3):nullptr;
        if(!holder)throw std::runtime_error("Human reload animation requires actual hand holder");
        if(!holder->entry())return 0;
        return dependencies().world_actors.items().runtime_item(holder->entry()).reload_animation_family();
    }
    template<class Human,class Entry>void consume_human_ammunition(Human& human,Entry entry,unsigned slot){
        retire_human_inventory_model(human,entry,slot);
        dependencies().world_actors.items().remove_runtime_item(entry);
    }
    template<class Item,class Human>void emit_human_weapon_reload(Item& item,Human& human){
        if(!(mode_flags()&2u))return;
        auto& d=dependencies();
        auto& request=d.startup.item_sound_request(); //SAME0098AFE0 as empty/shot writers.
        request.flags=human_is_player(human)&&human_view_offset_enabled()?0x40u:0u;
        request.relative=item.shot_frame();request.source=&human;
        request.group=3;request.entry=item.reload_sound();request.position={{0,0,0}};
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
    template<class Human>bool human_guard_equip_rocket(Human& human){ //00559590, real hand3 and shoulder2 commands.
        auto* storage=human.base().storage();if(!storage||!storage->holder(3)||!storage->holder(2))throw std::logic_error("Guard rocket selection requires actual holders3/2");
        const auto hand=storage->holder(3)->entry(),shoulder=storage->holder(2)->entry();auto& items=dependencies().world_actors.items();
        if(hand&&items.runtime_item(hand).definition().kind()==6)return true;
        if(!shoulder||items.runtime_item(shoulder).definition().kind()!=6)return false;
        auto& inventory=composition(human).inventory();return hand?inventory.swap(hand,3,human,shoulder,2):inventory.move(shoulder,human,2,3,0);
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
    template<class State>void human_ai_release_feedback(State& state){composition(*state.human()).state_factory().notify_inline_frame_command(state,0x10u);} //0058BAF0→B0 virtual08, not contactC8.
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
    template<class Human>void human_frame_contact(Human& human){ //0059FFD0's reached006FCB60→005FEFB0.
        auto& d=dependencies();if(!d.session.loaded)throw std::logic_error("Human Mine contact query requires actual loaded World");
        if(!d.scene.buckets().bound())throw std::logic_error("Loaded Human Mine contact requires actual World collision grid");
        using Scene=std::remove_reference_t<decltype(d.scene)>;
        DynamicCollisionPointQueries<typename Scene::Descriptor,typename Scene::Tree::DynamicGrid,typename Dependencies::ActorFrames>
            query(d.scene.buckets(),d.scene.frames());
        dispatch_human_mine_contact(human,d.scene.frames(),query,[&](void* borrowed,auto* touching,std::uint32_t slot){
            using Actor=typename std::remove_reference_t<decltype(d.world_actors)>::Runtime::Instance;
            d.world_actors.runtime().contact_mine(static_cast<Actor*>(borrowed),touching,slot);
        });
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
        dependencies().scene.frames().observe_data("execution","Human B0 frame",authority?"authority":"presentation",kind);
        if(auto* control=owner.control().control())
            dependencies().scene.frames().observe_data("execution","Human DD0 frame","field184",control->physical().control_frame_words().template get<std::uint32_t>(0x184));
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
    template<class Human,class Actor>void human_area_force_sight(Human& human,Actor& target){(void)dependencies().sight.force_human_target(human,selection_human(target));} //0055B6C0.
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
        const auto& contact=control->physical().ground_contact();
        if(!contact)return std::nullopt;
        const auto material=human_geometry_flags(*contact)>>24; //Original byte754+3, SAME collision identity flags.
        dependencies().scene.frames().observe_data("execution","Sight material754","GEffects94 lookup",material);
        return dependencies().startup.effects_loading_process().decorations.material_sight_factor(material); //GEffects+94.
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
    template<class Human>bool project_human_walk_position(Human& human,HumanVector& position){ //005A66F0.
        return human.loading_state&&human.loading_state->project_ground_position(position);
    }
    template<class Human,class Actor,class Runtime>bool human_actor_collision(Human& human,Actor& actor,const HumanVector& point,Runtime& runtime){
        return receive_human_actor_contact(human,actor,point,*this,runtime);
    }
    template<class Human>bool human_loading_actor_contact(Human& human,void* source,const HumanVector& point){
        if(!human.loading_state)return false;
        if(!source)throw std::logic_error("Human contact requires its actual source actor");
        auto& actor=*static_cast<SceneActor<typename Dependencies::ActorFrames>*>(source);
        return receive_human_actor_contact(human,actor,point,*this,*this);
    }
    template<class Commands>
    void publish_human_frame_commands(std::int32_t identity,bool second,const Commands& commands){
        for(const auto& command:commands)dependencies().scene.frames().observe_data("execution","Human command publication attempted",second?"broadcast stream0":"request stream0",command.kind);
        //005B2700/005B2D20 sample actual owners WHEN serialized, not when queued.
        const auto prepare=[&](const HumanPacketCommand& value){
            auto command=value;
            if(command.kind==20){
                auto* actor=dependencies().scene.frames().scene_actor_identity(command.fields.template get<std::int32_t>(8));
                using Human=typename Dependencies::Composition::Human;auto* human=dynamic_cast<Human*>(actor);
                if(!human)throw std::runtime_error("005B2D20 equipment publication requires its actual referenced Human");
                command.equipment.clear_equipment();snapshot_human_equipment(*human,command.equipment,dependencies().world_actors.items(),mode_flags());
            }else if(command.kind==13){
                using Human=typename Dependencies::Composition::Human;
                auto* human=dynamic_cast<Human*>(dependencies().scene.frames().scene_actor_identity(identity));
                if(!human)throw std::runtime_error("005B2700 inventory publication requires its actual sending Human");
                auto* storage=human->base().storage();if(!storage||storage->single())throw std::runtime_error("005B2700 requires actual Human storage");
                command.inventory.clear();auto& items=dependencies().world_actors.items();
                const auto append=[&](std::uint32_t slot,const auto& entry){if(entry){auto& item=items.runtime_item(entry);
                    command.inventory.push_back({slot,item.definition().identifier,static_cast<std::uint32_t>(item.identity)});}};
                for(const auto pair:std::array<std::pair<unsigned,unsigned>,5>{{{0,0},{4,4},{5,5},{2,2},{3,3}}}){
                    auto* holder=storage->holder(pair.first);if(!holder)throw std::logic_error("Inventory packet lost its actual holder");append(pair.second,holder->entry());}
                for(const auto slot:{1u,6u})if(auto* stacks=storage->slots(slot))
                    for(std::size_t n=0;n<stacks->occupied_stacks();++n)for(const auto entry:stacks->slots().at(n).entries)append(slot,entry);
            }
            return command;
        };
        write_human_frame_commands(dependencies().broadcast_packets,mode_flags(),identity,second,commands,prepare);
    }
    template<class Actor>void receive_human_authority_packet(Actor* actor,SessionPacketStream& packet,bool reserved){
        receive_human_process_packet(actor,packet,true,reserved);
    }
    template<class Actor>void receive_human_presentation_packet(Actor* actor,SessionPacketStream& packet,bool reserved){
        receive_human_process_packet(actor,packet,false,reserved);
    }
    template<class Actor>void receive_human_process_packet(Actor* actor,SessionPacketStream& packet,bool authority,bool reserved){
        using Human=typename Dependencies::Composition::Human;
        const auto identity=actor?actor->base().id10():-1;
        const auto resolve=[&]()->Human*{
            if(!actor)return nullptr;
            auto* current=dependencies().scene.frames().scene_actor_identity(identity);
            return current==actor?dynamic_cast<Human*>(current):nullptr;
        };
        receive_human_packet(packet,mode_flags(),authority,reserved,*this,resolve);
    }
    template<class Actor>bool try_human_presentation_packet(Actor& actor,SessionPacketStream& packet){
        using Human=typename Dependencies::Composition::Human;
        if(!dynamic_cast<Human*>(&actor))return false;
        receive_human_presentation_packet(&actor,packet,false);return true;
    }
    void receive_human_equipment_packet(const HumanPacketCommand& command){ //005B2A90, including discard.
        using Human=typename Dependencies::Composition::Human;
        auto* human=dynamic_cast<Human*>(dependencies().scene.frames().scene_actor_identity(command.fields.template get<std::int32_t>(8)));
        if(!human||!human->loading_state||human->loading_state->words().template get<std::uint32_t>(0x48)!=7u)return;
        throw std::runtime_error("Human PACK command20 requires Remote equipment replacement0059EB60 on actual Items/B0/DD0");
    }
    auto& human_command_frames(){return dependencies().scene.frames();}
    auto* human_command_actor(std::int32_t identity){return dependencies().scene.frames().scene_actor_command_identity(identity);} //00475D50.
    template<class Entry>std::uint32_t human_command_item_kind(Entry item){return dependencies().world_actors.items().runtime_item(item).definition().kind();}
    template<class Human>auto human_command_head(Human& human){
        if(!human.head_marker)throw std::logic_error("Human common command requires actual260 head marker");return human.head_marker;
    }
    template<class Human>HumanVector human_command_direction(Human& human){
        if(!human.loading_state)throw std::logic_error("Human common command lost current B0 direction receiver");
        return human.loading_state->target_direction(); //005A80E0, fresh Human+B0/4C.
    }
    template<class Actor>float human_command_car_health(Actor& target){return dependencies().scene.car_callbacks().car_field6a0(&target);} //004DDCF0.
    template<class Actor,class Entry,class Human>bool human_command_targeted(Actor& target,Entry item,Human& human,
            const std::optional<CarTargetedPlacement>& placement,bool authority){
        return dependencies().scene.car_callbacks().car_targeted_command(&target,item,&human,placement,authority);
    }
    template<class Human>void apply_human_packet_command(Human& human,const HumanPacketCommand& command,HumanPacketGrammar grammar){
        using Factory=typename Dependencies::Composition::StateFactory;
        const bool authority=grammar==HumanPacketGrammar::authority_player;
        const bool player=grammar==HumanPacketGrammar::presentation_player||authority;
        auto& owner=composition(human);auto* state=owner.states().get();
        if(!state||state!=human.loading_state)throw std::logic_error("Human packet requires SAME actual published B0 owner");
        const auto& f=command.fields;
        switch(command.kind){
        case 0:
            if(authority){if(human.base().id10()!=human_current_player_id())static_cast<typename Factory::Player&>(*state).receive_packet_inputs(command);}
            else if(!player)static_cast<typename Factory::Remote&>(*state).receive_packet_inputs(command);
            return; //Player presentation only consumes;005AC540 is genuine AL1.
        case 4:
            if(!authority&&!player)state->words().set(0x10,f.template get<HumanVector>(0x30)); //00583120, Remote virtual54 copies direction only.
            return; //Player/authority only decode this payload.
        case 6:{
            const auto identity=human.base().id10();
            auto& control_owner=owner.control();auto* control=control_owner.control();auto* look=control_owner.look();
            if(!control||!look)throw std::logic_error("Human Ladder command requires the actual DD0/E0 owners");
            auto* target=dependencies().scene.frames().scene_actor_identity(command.actor_identity);
            (void)control->request_frame_ladder(target,*look,*this); //00540DB0/005406C0/00592FE0 ignore result.
            if(authority&&(mode_flags()&7u)!=7u){ //00547600 only AFTER action; even when its result is false.
                auto* current=dependencies().scene.frames().scene_human_identity(identity);
                if(current!=&human||current->loading_state!=state)
                    throw std::logic_error("Ladder authority forwarding lost its actual Human/B0 during action");
                auto* live=composition(*current).states().get();
                if(live!=state)throw std::logic_error("Ladder authority forwarding requires SAME B0 owner");
                live->queue_frame_command(command,true);
            }
            return;
        }
        case 13:if(!(mode_flags()&1u))throw std::runtime_error("Human inventory diagnostic requires actual005B2340 nonauthority receiver");return;
        case 15:
            if((mode_flags()&7u)!=7u)throw std::runtime_error("Human Ladder feedback requires actual007866F0 nonoffline receiver");
            if(authority)state->queue_frame_command(command,true); //00547600 even offline;007866F0 RET in mode7.
            return; //Player presentation consumes only; Remote007866F0 mode7 RET.
        case 16:
            if(authority)return; //00540DB0 only decodes this command.
            receive_human_score_rank(human,f.template get<std::uint32_t>(8),player);return;
        case 19:{
            if((mode_flags()&7u)!=7u)throw std::runtime_error("Human speech packet requires actual nonoffline00588170 receiver");
            if(!human.speech_definition)throw std::runtime_error("Human speech packet has no actual speech definition");
            HumanSpeechLoading speech(dependencies().scene.frames(),*this);
            speech.offline(*state,static_cast<std::int32_t>(f.template get<std::uint32_t>(8)),f.template get<std::uint32_t>(0xc)!=0);return;
        }
        case 21:{ //Authority00540DB0 synthesizes20 in SAME second90 vector, including offline.
            HumanPacketCommand response;response.kind=20;response.fields.set(8,f.template get<std::int32_t>(8));state->queue_frame_command(std::move(response),true);return;
        }
        case 22:{
            if(authority||player)return; //Actual Player receivers consume only.
            auto& d=dependencies();
            using Controls=HumanScriptLoadingControls<typename Dependencies::ActorFrames,Human,HumanWorldLoadingServices,
                std::remove_reference_t<decltype(d.sight.source())>,std::remove_reference_t<decltype(d.hearing)>,
                std::remove_reference_t<decltype(d.first_players)>,std::remove_reference_t<decltype(d.first_person.view())>,HumanWorldLoadingServices>;
            Controls controls(d.scene.frames(),*this,d.sight.source(),d.hearing,d.first_players,d.first_person.view(),*this);
            controls.suspend_human(human,f.template get<std::uint32_t>(8)!=0);return;
        }
        case 23:
            if(authority||player)return;
            static_cast<typename Factory::Remote&>(*state).clear_packet_action_suppression(); //00592FE0 clears2C1, irrespective of decoded bool.
            owner.control().action_command(0x4c);return; //Existing005A45F0→005ABB50 receiver, no retained B0 after call.
        case 1:{
            present_human_damage(human,human_damage_command_value(command),*this,dependencies().scene.frames());return;
        }
        case 2:throw std::runtime_error(authority?"Human PACK command2 requires active clothing005A6930":"Human PACK command2 requires active clothing005A6A00");
        case 3:{
            const auto identity=human.base().id10();
            receive_human_common_command(*state,command,authority,player,*this,
                [&](auto& current,const auto& value){return owner.state_factory().common_command(current,value);},
                [&](const auto& value){ //00540DB0 only after success and fresh non7 mode.
                    auto* current=dependencies().scene.frames().scene_human_identity(identity);
                    if(current!=&human||current->loading_state!=state)
                        throw std::logic_error("Human command3 forwarding lost its actual Human/B0");
                    composition(*current).states().get()->queue_frame_command(value,true);
                });
            return;
        }
        case 7:throw std::runtime_error(authority?"Human PACK command7 requires inventory validation005A2D50":"Human PACK command7 requires inventory apply005A2E10");
        case 8:throw std::runtime_error(authority?"Human PACK command8 requires inventory swap005A2E90":"Human PACK command8 requires inventory swap apply005A2F50");
        case 9:throw std::runtime_error(authority?"Human PACK command9 requires item validation005A39D0":"Human PACK command9 requires item apply005A3A40");
        case 10:{
            const auto drop=human_drop_command_value(command);
            if(authority){auto velocity=drop.direction;for(unsigned axis=0;axis!=3;++axis)velocity[axis]*=drop.speed;
                drop_human_damage_item_identity(human,drop.item,velocity);
            }else present_human_dropped_item(human,drop);
            return;
        }
        case 11:throw std::runtime_error(authority?"Human PACK command11 requires item validation005A37C0":"Human PACK command11 requires item apply005A38F0");
        case 12:throw std::runtime_error(authority?"Human PACK command12 requires Player action validation0053D5D0":player?"Human PACK command12 requires Player action apply0053D870":"Human PACK command12 requires Remote action apply00594110");
        case 14:{
            using PacketHuman=typename Dependencies::Composition::Human;
            auto* target=dynamic_cast<PacketHuman*>(dependencies().scene.frames().scene_actor_identity(static_cast<std::int32_t>(f.template get<std::uint32_t>(8))));
            receive_human_offline_notification(human,target,dependencies().scene.frames(),dependencies().startup.text(),dependencies().hud,mode_flags(),human_current_player_id());return;
        }
        case 17:{
            auto* source=human_damage_source(human_frame_signed_word(f.template get<std::uint32_t>(0x14)));
            damage_human(human,f.template get<std::uint32_t>(8),static_cast<float>(f.template get<std::int32_t>(0x10)),
                HumanVector{},HumanVector{},source,f.template get<std::uint32_t>(0xc),*this,dependencies().scene.frames());return;
        }
        case 18:throw std::runtime_error("Human PACK command18 requires Remote special-animation005A9C20/005AC4A0/0077CE10 loading and007757E0/00775970 control receivers");
        default:throw std::logic_error("Human command dispatched outside original grammar");
        }
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
    template<class Hit>auto human_ragdoll_contact_actor(const Hit& hit){
        using Actor=SceneActor<typename Dependencies::ActorFrames>;
        if(!hit.dynamic_identity||!hit.dynamic_identity->frame)return static_cast<Actor*>(nullptr);
        const auto* data=dependencies().scene.frames().find_actor_frame_data(hit.dynamic_identity->frame);
        return data?reinterpret_cast<Actor*>(data->words[0]):nullptr; //Actual frameEC word0, read when the retained frame changes.
    }
    template<class Actor>bool human_ragdoll_car_authority(Actor& actor){
        if(actor.base().type()!=4)throw std::logic_error("Human ragdoll Car query requires its actual Car actor");
        auto& callbacks=dependencies().scene.car_callbacks();
        return car_actor_authority([this]{return mode_flags();},[&]{return callbacks.car_occupant(&actor,0);},
            [this]{using Human=typename Dependencies::Composition::Human;
                return static_cast<SceneActor<typename Dependencies::ActorFrames>*>(static_cast<Human*>(local_peer_human()));},
            [&](auto* occupant){return callbacks.car_human_role(occupant);});
    }
    template<class Actor>void human_ragdoll_car_impulse(Actor& actor,const HumanVector& impulse,const HumanVector& point){
        dependencies().scene.car_callbacks().human_ragdoll_car_impulse(&actor,impulse,point);
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
    template<class Frame>bool human_light_observation(Frame model,float& shadowed,float& plain){ //005A6710 via actual modelEC/Actor/B0.
        auto* actor=dependencies().sight.sight_actor(model);
        if(!actor)throw std::logic_error("Human light observation requires the actual model Actor binding");
        auto& human=selection_human(*actor);
        return human.loading_state&&human.loading_state->observe_navigation_light(shadowed,plain);
    }
    template<class Frame>long double human_scene_light(const HumanVector& position,Frame sector,bool shadows){
        struct Queries {
            HumanWorldLoadingServices& world;
            auto ray(const HumanVector& start,const HumanVector& displacement,std::uint32_t layers,std::uint32_t flags,bool){
                return world.sight_ray(start,displacement,layers,flags);
            }
        } queries{*this};
        auto& d=dependencies();
        return sample_scene_light(d.scene.frames(),queries,position,sector,shadows,d.session.status);
    }
    template<class Actor>bool human_action_car_disabled(Actor& actor){return dependencies().scene.car_callbacks().car_field6a0(&actor)==0;} //004DDCF0, actual Car body health.
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
    template<class Human>CommandArea human_ai_command_area(Human& human){
        if(!human.command||!human.command->area_position)throw std::logic_error("Area AI handover requires produced B4 sniper area005AE610");
        float radius;std::memcpy(&radius,&human.command->field10,4);return {*human.command->area_position,radius};
    }
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
    auto human_contact_voice_active(std::int32_t handle){return dependencies().startup.game_sound_voice_active(handle);}
    void stop_human_contact_voice(std::int32_t& handle){dependencies().startup.stop_game_sound_voice(handle);}
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
    std::int32_t human_spawn_protection_seconds(){return dependencies().startup.game_configuration().rules228[10];} //Actual configuration250.
    void hud_human_spawn_protection(std::uint32_t identity,std::uint32_t duration){dependencies().hud.set_injury_message(identity,duration);} //0049DE50, before local initialization.
    template<class Human>void human_initial_dead_notification(Human& human){HumanDamageCommand command;command.kind=14;present_human_damage(human,command,*this,dependencies().scene.frames());}
    template<class Actor>void detach_human_attachment(Actor& actor,std::int32_t seat){
        remove_human_reaction_seat(&actor,seat); //SAME actor+80: Car004D9A40 / StaticWeapon00604630, before Human160/164 clear.
    }
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
