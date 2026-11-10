#ifndef HD2_HOST_GAME_BLAST_WAVES_H
#define HD2_HOST_GAME_BLAST_WAVES_H
#include "host/game_session_activation.h"
#include "game/dynamic_collision_point_queries.h"
#include "game/collision_ray.h"
namespace hd2::host {
//Actual fixed-update/finish bridge to SAME Startup008AED48 storage. Constructor
//does not clear records or invent a World; positive generation uses actual
//loaded tree/buckets/frame data/camera. Damage/Physics positive receivers still
//reject at their actual boundary, before publishing an unhandled hit.
class GameBlastWaves final : public GameSessionTransientFinish {
public:
    using Process=StartupSession::BlastWaves;using Vector=Process::Vector;using Matrix=GameActorServices::Matrix;
    using Actor=GameSceneActors::Instance;using Descriptor=world_loading::Scene::Descriptor;
    explicit GameBlastWaves(world_loading::Owners& owners):d_(owners.dependencies()),
        points_(d_.scene.buckets(),d_.scene.frames()){}
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
    std::int32_t blast_material_index(const Actor&,EngineFrame*){
        throw std::runtime_error("Blast damage requires actual actor virtualD8 material receiver");
    }
    std::uint32_t blast_material_absorption(std::int32_t index)const{return d_.startup.car_definitions().materials().at(index).word(0x28);}
    void blast_actor_damage(Actor&,std::uint32_t,float,const Vector&,const Vector&,std::uint32_t){
        throw std::runtime_error("Blast damage requires actual actor virtualD4 receiver");
    }
    void blast_physics_impulse(EngineFrame* frame,std::int32_t,unsigned){
        const auto* data=d_.scene.frames().find_actor_frame_data(frame);
        if(data&&data->words[5])throw std::runtime_error("Blast force requires actual0043C6A0 Physics owner receiver");
    }
private:
    static world_loading::Human& human(Actor& actor){
        auto* value=dynamic_cast<world_loading::Human*>(&actor);
        if(!value||value->base().type()!=3)throw std::logic_error("Blast Human requires actual containing actor lifetime");return *value;
    }
    void require_loaded()const{if(!d_.session.loaded)throw std::logic_error("Blast process requires actual loaded containing World");}
    world_loading::Dependencies& d_;
    game::DynamicCollisionPointQueries<Descriptor,world_loading::Scene::Tree::DynamicGrid,GameActorServices> points_;
};
} //namespace hd2::host
#endif
