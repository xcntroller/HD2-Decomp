#ifndef HD2_HOST_GAME_DECORATIONS_RUNTIME_H
#define HD2_HOST_GAME_DECORATIONS_RUNTIME_H
#include "game/effects_decorations_loading.h"
#include "game/collision_ray.h"
#include "host/game_actor_services.h"
#include <cstdlib>
namespace hd2::host {
//A temporary adapter over SAME World's existing Frames/tree/Players/camera.
//Actual models retain only engine references and the owning Frames borrower;
//GEffects clears their process pools before that World/Frames can die.
template<class Dependencies>class GameDecorationsRuntime final:public game::EffectsDecorationsLoading::Runtime {
    using Policy=game::EffectsDecorationsLoading;using Vector=Policy::Vector;
    using ModelBase=Policy::RuntimeModel;
    class Model final:public ModelBase {
    public:
        Model(GameActorServices& frames,const std::uint32_t& mode):frames_(frames),mode_(mode),owner_(frames.create_decoration_model()){
            if(!owner_.get())throw std::runtime_error("Actual decoration engine allocation returned null");
        }
        void visible(bool value)override{frames_.object_visible(owner_.get(),value);}
        void update()override{frames_.update_scene_object(owner_.get());}
        void opacity(float value)override{if(mode_&2u)frames_.effects_model_opacity(owner_.get(),value);}
        void request_opacity(float value)override{frames_.effects_model_opacity(owner_.get(),value);} //0047F200 directly calls model50, without GEffects mode gate.
        void base_opacity(float value)override{frames_.decoration_base_opacity(owner_.get(),value);}
        void position(const Vector& value)override{frames_.object_position(owner_.get(),{{value[0],value[1],value[2]}});}
        void direction(const Vector& value,float roll)override{frames_.decoration_direction(owner_.get(),value,roll);}
        void scale(float value)override{frames_.decoration_scale(owner_.get(),value);}
        Vector world_position()override{const auto m=frames_.actor_world_matrix(owner_.get());return {{m[12],m[13],m[14],m[15]}};}
        Vector world_forward()override{const auto m=frames_.actor_world_matrix(owner_.get());return {{m[8],m[9],m[10],m[11]}};}
        Vector request_direction()override{return frames_.decoration_request_direction(owner_.get());}
        float base_opacity()const override{return frames_.decoration_base_opacity(owner_.get());}
        void visual_amount(float value)override{frames_.decoration_visual_amount(owner_.get(),value);}
        auto frame()const{return owner_.get();}auto& frames()const{return frames_;}
    private:GameActorServices& frames_;const std::uint32_t& mode_;MenuDriver::Owner owner_;
    };
public:
    explicit GameDecorationsRuntime(Dependencies& d):d_(d){}
    void seed(std::uint32_t value)override{std::srand(value);} //Native process CRT seed, no private/replacement stream.
    long double random(float maximum)override{return maximum>0?static_cast<long double>(std::rand())*maximum*.000030517578125f:0.L;} //00475190, native32768 divisor.
    Vector centre()override{ //0047BF32: listener first; current player requery after detached-camera getter.
        auto& frames=d_.scene.frames();const auto camera=frames.active_game_camera();
        if(!camera)throw std::runtime_error("Decoration grid requires actual scene15C listener");
        auto matrix=frames.actor_world_matrix(camera);Vector position{{matrix[12],matrix[13],matrix[14],matrix[15]}};
        auto& view=d_.first_person.view();
        if(!view.enabled()&&!view.secondary_enabled()&&d_.players.control().current_player()&&!d_.camera.detached().enabled()){
            auto* human=d_.players.control().current_player();
            if(!human)throw std::runtime_error("Decoration current player disappeared after camera query");
            matrix=frames.actor_world_matrix(human->base().frame());position={{matrix[12],matrix[13],matrix[14],matrix[15]}};
        }
        return position;
    }
    std::optional<Policy::Ground> ground(const Vector& point)override{
        auto& scene=d_.scene;using Scene=std::remove_reference_t<decltype(scene)>;
        game::CollisionRay<typename Scene::Frames::Frame,typename Scene::Descriptor,GameCollisionResources,GameActorServices>
            query(&scene.collision_tree(),&scene.buckets(),scene.collision_resources(),scene.frames());
        const auto hit=query.ray(point,{{0,-500,0,0}},0xffffffffu,0x210000u,true);if(!hit)return {};
        if(!hit->static_identity||hit->dynamic_identity)throw std::logic_error("Decoration ground query lost actual static identity");
        const auto id=*hit->static_identity;const auto& tree=scene.collision_tree();
        const auto flags=id.kind?tree.shapes.at(id.kind-0x80).at(id.index).flags:tree.triangles.at(id.index).flags;
        return Policy::Ground{hit->position,hit->normal,flags>>24};
    }
    std::unique_ptr<ModelBase> create_model()override{return std::make_unique<Model>(d_.scene.frames(),d_.startup.application_flags_owner());}
    std::int32_t load_model(ModelBase& model,const std::string& name)override{return d_.scene.frames().load_decoration_model(checked(model).frame(),name);}
    void clone_model(ModelBase& to,ModelBase& from)override{d_.scene.frames().clone_decoration_model(checked(to).frame(),checked(from).frame());}
    void link_model(ModelBase& model)override{d_.scene.frames().link_decoration_model(checked(model).frame());}
    void prepare_material(ModelBase& model)override{d_.scene.frames().prepare_decoration_material(checked(model).frame());}
    void update_contacts(Policy& policy,std::uint32_t elapsed)override{
        auto& frames=d_.scene.frames();auto* human=d_.players.control().current_player();
        if(human&&!human->field160){
            const auto matrix=frames.actor_world_matrix(human->base().frame());
            const Vector point{{matrix[12],matrix[13],matrix[14],matrix[15]}},velocity=human->activation.vector1b0;
            auto& services=d_.human_factory.world_services();const auto posture=services.human_inline(*human).frame_posture_mode();
            policy.update_player_contact(point,velocity,posture,[&services]{return services.human_view_offset_enabled();},elapsed,*this);
        }
        const auto& cars=frames.car_actors();
        const float range=static_cast<float>(static_cast<long double>(policy.width())*policy.density()*.5f);
        const auto anchor=policy.grid_centre();
        for(auto n=cars.size();n;){auto* car=cars[--n];const auto& position=car->base().world_position();
            const auto x=static_cast<long double>(position[0])-anchor[0],z=static_cast<long double>(position[2])-anchor[1];
            if(z*z+x*x<static_cast<long double>(range)*range&&!(d_.scene.car_callbacks().car_speed958(car)<.1f))
                throw std::runtime_error("Decoration vehicle contact requires actual0047F730 wheel/bend producers");
        }
    }
private:
    Model& checked(ModelBase& base){auto* model=dynamic_cast<Model*>(&base);
        if(!model||&model->frames()!=static_cast<GameActorServices*>(&d_.scene.frames()))throw std::logic_error("Decoration model belongs to another actual World");
        return *model;
    }
    Dependencies& d_;
};
}
#endif
