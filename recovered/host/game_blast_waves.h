#ifndef HD2_HOST_GAME_BLAST_WAVES_H
#define HD2_HOST_GAME_BLAST_WAVES_H
#include "host/game_session_activation.h"
#include "game/dynamic_collision_point_queries.h"
#include "game/collision_ray.h"
#include "game/human_blast_damage.h"
#include "game/car_actor_damage.h"
#include "game/car_destruction.h"
#include "game/human_objective_experience.h"
#include "game/mission_script_player_predicates.h"
namespace hd2::host {
//Actual fixed-update/finish bridge to SAME Startup008AED48 storage. Constructor
//does not clear records or invent a World; positive generation uses actual
//loaded tree/buckets/frame data/camera and actual Human damage owner. Other
//derived damage/Physics receivers still reject at their actual boundary.
class GameBlastWaves final : public GameSessionTransientFinish {
public:
    using Process=StartupSession::BlastWaves;using Vector=Process::Vector;using Matrix=GameActorServices::Matrix;
    using Actor=GameSceneActors::Instance;using Descriptor=world_loading::Scene::Descriptor;
    using Request=Process::Request;
    explicit GameBlastWaves(world_loading::Owners& owners):d_(owners.dependencies()),services_(owners.human_services()),
        points_(d_.scene.buckets(),d_.scene.frames()){
        car_=dynamic_cast<GameCarCallbacks*>(&d_.scene.car_callbacks());
        if(!car_)throw std::logic_error("Explosion binding requires actual scene Car callback owner");
        try{d_.world_actors.bind_explosion(*this);car_->bind_explosion(*this);car_->bind_damage_packets(*this);}
        catch(...){car_->unbind_damage_packets(*this);d_.world_actors.unbind_explosion(*this);car_->unbind_explosion(*this);throw;}
    }
    ~GameBlastWaves()override{if(car_)car_->unbind_damage_packets(*this);d_.world_actors.unbind_explosion(*this);if(car_)car_->unbind_explosion(*this);}
    GameActorServices& frames(){return d_.scene.frames();}
    void explode(Actor* source,const Vector& position,float radius,float power,std::int32_t sound){
        game::emit_shot_explosion(d_.startup.shot_manager_process(),d_.sound,*this,source,position,radius,power,sound);
    }
    bool create(const Process::Request& request){require_loaded();return d_.startup.blast_wave_process().create(request,*this);}
    void update(std::uint32_t elapsed){require_loaded();d_.startup.blast_wave_process().update(elapsed,*this);}
    void clear_all_shared_records()override{d_.startup.blast_wave_process().clear();} //006DF390(0,true), including positive linked records.
    std::uint32_t mode_flags()const{return d_.startup.application_flags();}
    EngineFrame* blast_scene_sector(const Vector& centre){return d_.scene.frames().human_scene_sector(centre);}
    auto blast_radius_candidates(const Vector& centre,float radius,std::uint32_t mask){return points_.radius(centre,radius,mask);}
    const Descriptor* blast_containing_object(const Vector& centre,std::uint32_t mask){return points_.containing(centre,mask);}
    Actor* blast_direct_actor(EngineFrame* frame){
        const auto* data=d_.scene.frames().find_actor_frame_data(frame);
        return data?reinterpret_cast<Actor*>(data->words[0]):nullptr;
    }
    Actor* blast_actor_for_frame(EngineFrame* frame){ //006DDD00, direct EC then model100(kind9) EC.
        if(auto* actor=blast_direct_actor(frame))return actor;
        const auto model=d_.scene.frames().frame_model_owner(frame);
        return model&&d_.scene.frames().frame_kind(model)==9?blast_direct_actor(model):nullptr;
    }
    std::uint32_t blast_actor_type(const Actor& actor)const{return actor.base().type();}
    EngineFrame* blast_find(EngineFrame* frame,const char* name,std::uint32_t mask){return d_.scene.frames().actor_find(frame,name,mask);}
    auto retain_blast_frame(EngineFrame* frame){return d_.scene.frames().retain_actor_frame(frame);}
    auto blast_world_matrix(EngineFrame* frame){return d_.scene.frames().actor_world_matrix(frame);}
    auto blast_inverse_matrix(const Matrix& matrix){return d_.scene.frames().car_inverse_matrix(matrix);}
    auto blast_transform(const Vector& vector,const Matrix& matrix){return d_.scene.frames().car_transform(vector,matrix);}
    auto blast_frame_bounds(EngineFrame* frame){return d_.scene.frames().blast_frame_bounds(frame);}
    void blast_camera_impulse(const Vector& centre,std::uint32_t range){d_.camera.blast_impulse(centre,range,mode_flags());}
    EngineFrame* blast_human_sector(Actor& actor){
        return d_.world_actors.template human_loading_owner<world_loading::Composition>(human(actor)).control().scene_sector();
    }
    bool blast_human_clear(Actor& actor,const Vector& centre){ //006DE560: first clear of six shortened static rays wins.
        return d_.scene.frames().visit_blast_human_markers(human(actor).base().frame(),[&](const auto marker){
            const auto matrix=d_.scene.frames().actor_world_matrix(marker);
            const Vector start{{matrix[12],matrix[13],matrix[14],matrix[15]}};Vector displacement{};
            for(unsigned a=0;a!=3;++a)displacement[a]=static_cast<float>(static_cast<long double>(centre[a])-start[a]);
            const auto direction=game::actor_activation_direction(displacement);
            for(unsigned a=0;a!=3;++a)displacement[a]=static_cast<float>(static_cast<long double>(displacement[a])-static_cast<long double>(direction[a])*.05000000074505806f);
            return !blast_obstructed(start,displacement);
        });
    }
    bool blast_obstructed(const Vector& start,const Vector& displacement){
        game::CollisionRay<EngineFrame*,Descriptor,GameCollisionResources,GameActorServices> ray(
            &d_.scene.collision_tree(),&d_.scene.buckets(),d_.scene.collision_resources(),d_.scene.frames());
        return bool(ray.ray(start,displacement,0xffffffffu,0xb10000u,false)); //B10000 is a MASK, not a pointer.
    }
    std::int32_t blast_material_index(const Actor& actor,EngineFrame*){
        if(actor.base().type()==3)return game::human_blast_material; //0059C2A0, absent frozen inventory.
        if(actor.base().type()==12){
            if(!dynamic_cast<const world_loading::Actors::Runtime::DetachedItem*>(&actor))throw std::logic_error("Blast material requires actual runtime Item12 owner");
            return 11; //005FDDC0,00812E18 D8, genuine constant11/RET4; absent frozen.
        }
        if(actor.base().type()==2){
            const auto* mine=dynamic_cast<const world_loading::Actors::Runtime::Mine*>(&actor);
            if(!mine)throw std::logic_error("Blast material requires actual runtime Mine owner");
            return mine->material(); //005FEEB0, actual bound100, absent frozen.
        }
        throw std::runtime_error("Blast damage requires actual actor virtualD8 material receiver");
    }
    std::uint32_t blast_material_absorption(std::int32_t index)const{return d_.startup.car_definitions().materials().at(index).word(0x28);}
    void blast_actor_damage(Actor& actor,std::uint32_t kind,float amount,const Vector& point,const Vector& impulse,Actor* source){
        if(actor.base().type()==2||actor.base().type()==12)return; //Mine/Item12 D4=005B88A0, genuine RET24.
        if(actor.base().type()==3){
            auto& target=d_.world_actors.human_loading_actor(&actor);
            auto* cause=source?&d_.world_actors.human_loading_actor(source):nullptr;
            game::apply_human_blast_damage(target,kind,amount,point,impulse,cause);return;
        }
        if(actor.base().type()==4){
            auto* car=dynamic_cast<game::CarSceneActor<GameActorServices>*>(&actor);
            if(!car||!car->geometry)throw std::logic_error("Car blast damage requires actual initialized containing Car owner");
            game::apply_car_actor_damage(frames(),*this,*car,[&]()->auto&{if(!car->loading)throw std::logic_error("Car damage requires actual loading lifetime");return car->loading->cold_state();},kind,amount,point,impulse,source);return;
        }
        throw std::runtime_error("Blast damage requires actual actor virtualD4 receiver");
    }
    std::uint32_t car_damage_mode()const{return mode_flags();}
    std::uint32_t car_damage_role(Actor* actor){return d_.world_actors.human_loading_actor(actor).properties.field19c;}
    std::int32_t car_damage_health(Actor* actor){return game::car_contact_signed_word(d_.world_actors.human_loading_actor(actor).properties.stats_ec[0]);}
    bool car_damage_hostile(Actor* observer,Actor* target){return d_.sight.human_sight_hostile(&d_.world_actors.human_loading_actor(observer),&d_.world_actors.human_loading_actor(target));}
    bool car_damage_player(Actor* actor){return game::script_human_player_controlled(d_.world_actors.human_loading_actor(actor),mode_flags());}
    bool car_damage_suppressed(Actor* actor){return d_.presentation.human_selected_suppressed(d_.world_actors.human_loading_actor(actor));}
    std::uint32_t car_damage_side(Actor* actor){return services_.human_specialization(d_.world_actors.human_loading_actor(actor),false);}
    Actor* car_damage_occupant(Actor* actor,std::size_t index){
        auto* car=dynamic_cast<game::CarSceneActor<GameActorServices>*>(actor);
        if(!car)throw std::logic_error("Car damage seat query requires actual Car owner");
        return index<car->occupants.size()?car->occupants[index]:nullptr;
    }
    void car_damage_human(Actor* target,std::uint32_t kind,float amount,const Vector& point,const Vector& direction,Actor* source){
        blast_actor_damage(*target,kind,amount,point,direction,source); //SAME guarded Human lifetime and final hit0.
    }
    std::uint32_t car_damage_absorption(std::int32_t index){return d_.car_definitions.materials().at(index).word(0x28);}
    void car_damage_event(Actor* actor,const game::CarDamageEvent& event){game::publish_car_damage(d_.broadcast_packets,actor->base().id10(),event);}
    void car_damage_destroy(Actor* actor,bool water){
        auto* car=dynamic_cast<game::CarSceneActor<GameActorServices>*>(actor);
        if(!car||!car->geometry)throw std::logic_error("Car destruction requires actual containing Car geometry");
        game::destroy_car_actor(frames(),*this,*car,[&]()->auto&{
            if(!car->loading)throw std::logic_error("Car destruction requires actual loading lifetime");return car->loading->cold_state();
        },water);
    }
    bool car_destruction_remote_ai(){return d_.startup.human_remote_ai_enabled();}
    std::int32_t car_destruction_style(){return d_.startup.game_configuration().style224;}
    void car_destruction_controls(game::CarSceneActor<GameActorServices>& car){
        if(!car.loading)throw std::logic_error("Car destruction requires its actual control lifetime");car.loading->destruction_controls();
    }
    void car_destruction_script(game::CarSceneActor<GameActorServices>& car){
        const auto frame=car.base().frame();const auto* data=frames().find_actor_frame_data(frame);
        if(data&&data->words[1]){auto* context=d_.script_link.context_for_frame(frame);
            if(!context)throw std::logic_error("Car death Script metadata has no actual context");context->death(d_.script_link);}
    }
    Actor* car_destruction_source(const std::optional<std::int32_t>& identity){
        if(!identity)throw std::runtime_error("Car source24B0 has no native producer");
        return *identity==-1?nullptr:d_.scene.frames().scene_actor_command_identity(*identity);
    }
    std::uint32_t car_destruction_random_word(){return d_.startup.random_word();}
    float car_destruction_random_float(){return frames().volume_random_unit();} //ENGINE RandFloat, independent from CRT.
    void car_destruction_experience(Actor* actor,std::int32_t amount){
        game::HumanObjectiveExperience experience(d_.startup.application_flags_owner());
        experience.add_experience(d_.world_actors.human_loading_actor(actor),amount);
    }
    std::size_t car_area_count(){return d_.scene.frames().scene_actor_identity_count();}
    Actor* car_area_actor(std::size_t index){if(index>0x7fffffffu)throw std::length_error("Car area Scene2 index exceeds signed range");return d_.scene.frames().scene_actor_identity(static_cast<std::int32_t>(index));}
    std::uint32_t car_area_type(Actor* actor){return actor->base().type();}
    EngineFrame* car_area_head(Actor* actor){auto frame=d_.world_actors.human_loading_actor(actor).head_marker;
        if(!frame)throw std::logic_error("Car area damage requires actual Human260 head marker");return frame;}
    bool car_area_obstructed(const Vector& centre,const Vector& displacement){
        game::CollisionRay<EngineFrame*,Descriptor,GameCollisionResources,GameActorServices> ray(
            &d_.scene.collision_tree(),&d_.scene.buckets(),d_.scene.collision_resources(),frames());
        return bool(ray.ray(centre,displacement,0xffffffffu,0,false)); //007183A0(...,NULL,NULL,-1,0): static only, no normal.
    }
    Vector car_area_random_point(Actor* actor){return services_.human_damage_random_position(d_.world_actors.human_loading_actor(actor));}
    std::int32_t car_emit_particle(std::int32_t definition,const Matrix& matrix,EngineFrame* parent){
        return d_.startup.mission_effects_process().emit(static_cast<std::uint32_t>(definition),matrix,parent,true,false);
    }
    void car_emit_dm_sound(std::int32_t entry,EngineFrame* relative){
        frontend::SpatialSoundRequest request;request.flags=1;request.group=6;request.entry=entry;request.relative=relative;
        (void)d_.sound.emit(request);
    }
    void receive_car_damage(Actor& actor,game::SessionPacketStream& packet){ //004DB360, actual actor1C; no early mode gate.
        auto* car=dynamic_cast<game::CarSceneActor<GameActorServices>*>(&actor);
        if(!car||!car->geometry||!car->ancillary)throw std::logic_error("Car packet requires actual Car/DM/geometry owners");
        game::receive_car_damage_packet(frames(),*this,*car,[&]()->auto&{
            if(!car->loading)throw std::logic_error("Car packet requires actual initialized loading lifetime");return car->loading->cold_state();
        },packet);
    }
    void car_present_destroyed(game::CarSceneActor<GameActorServices>& car){
        game::present_destroyed_car(frames(),*this,car,[&]()->auto&{
            if(!car.loading)throw std::logic_error("Destroyed Car presentation requires actual initialized physical lifetime");return car.loading->cold_state();
        });
    }
    void car_physical_reset(game::CarSceneActor<GameActorServices>& car,float speed,bool restore){car.loading->physical_reset(speed,restore);}
    bool car_presentation_authority(game::CarSceneActor<GameActorServices>& car){return services_.human_ragdoll_car_authority(car);}
    void car_presentation_impulse(game::CarSceneActor<GameActorServices>& car,const Vector& impulse,const Vector& point){
        auto& state=car.loading->cold_state();game::apply_car_direct_impulse(frames(),state,impulse,point,0,Vector{{1,1,1,0}},[&]{if(car.callbacks->state6a4)car.callbacks->state6a4();});
    }
    void car_destroyed_meshes(game::CarSceneActor<GameActorServices>& car){car.loading->destroyed_meshes();}
    void car_present_glass(game::CarSceneActor<GameActorServices>& car,std::size_t index,const Vector& direction){
        if(!car.loading)throw std::logic_error("Car glass presentation requires actual initialized lifetime");car.loading->glass_damage(index,direction);
    }
    void car_present_occupant(Actor* actor){
        auto& human=d_.world_actors.human_loading_actor(actor);
        auto* control=d_.world_actors.template human_loading_owner<world_loading::Composition>(human).control().control();
        if(control)control->opacity().car_destruction(human.base().frame(),*control);
    }
    void car_puncture_sound(game::CarSceneActor<GameActorServices>& car,std::size_t index){car.loading->puncture_sound(index);}
    void blast_physics_impulse(EngineFrame* frame,std::int32_t damage,unsigned effects){
        const auto* data=d_.scene.frames().find_actor_frame_data(frame);
        //The existing source method name is historical: EC+14 borrows a
        //Versions group here;0043C6A0 damages its parts, not a rigid-body force.
        if(data&&data->words[5])(void)d_.world_actors.version_contact(data->words[5],damage,effects);
    }
private:
    static world_loading::Human& human(Actor& actor){
        auto* value=dynamic_cast<world_loading::Human*>(&actor);
        if(!value||value->base().type()!=3)throw std::logic_error("Blast Human requires actual containing actor lifetime");return *value;
    }
    void require_loaded()const{if(!d_.session.loaded)throw std::logic_error("Blast process requires actual loaded containing World");}
    world_loading::Dependencies& d_;world_loading::Services& services_;
    GameCarCallbacks* car_=nullptr;
    game::DynamicCollisionPointQueries<Descriptor,world_loading::Scene::Tree::DynamicGrid,GameActorServices> points_;
};
} //namespace hd2::host
#endif
