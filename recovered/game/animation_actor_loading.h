#ifndef HD2_GAME_ANIMATION_ACTOR_LOADING_H
#define HD2_GAME_ANIMATION_ACTOR_LOADING_H
#include "game/model_scene_actors.h"
#include "game/actor_material.h"
#include "frontend/cleanup.h"
#include <cctype>
namespace hd2::game {
//005B3BE0/005B5000/005B4790 loading and005B5190 cold first update.
//The containing world supplies actual resources/callbacks/materials/buckets.
//Interaction98, starting/advancing tracks and special escape-aircraft wings
//remain required distinct receivers and explicitly reject when reached.
template<class Frames,class Services,class Collisions>class AnimationActorLoading final:public AnimationLoadingLifetime<typename Frames::Frame> {
public:
    using Actor=AnimationSceneActor<Frames>;using Frame=typename Frames::Frame;
    using ModelOwner=decltype(std::declval<Frames&>().create_animation_actor_model());
    AnimationActorLoading(Frames& frames,Services& services,Collisions& collisions,Actor& actor)
        :frames_(frames),services_(services),collisions_(collisions),actor_(actor){}
    ~AnimationActorLoading()override{try{retire();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}}
    Frame model()const override{return model_?model_->get():Frame{};}
    void activate(bool authority)override { //005B3BE0.
        actor_.flaga0=false;flag_a3_=false;actor_.fieldd4=0;flag_b8_=false;count_bc_=0;
        actor_.base().name(actor_.properties.fieldb0==0xffffffffu?std::string{}:services_.actor_localized(actor_.properties.fieldb0));
        actor_.base().activate_frame(authority,services_);
        if(authority&&(services_.mode_flags()&2u))return;
        if(model_||partial_)throw std::logic_error("Retire existing/partial Animation helper model before retry");
        partial_=true;actor_.field98=0;particle_=frames_.actor_find(actor_.base().frame(),"particle",0xffff);
        if(!actor_.collisions)actor_.collisions=std::make_unique<AnimationActorCollisions<Frames,Collisions,Actor>>(frames_,collisions_,actor_);
        actor_.collisions->rebuild();model_.emplace(frames_.create_animation_actor_model());
        if(!model())throw std::runtime_error("Cannot create required Animation helper model");
        if(frames_.load_animation_actor_model(model(),actor_.properties.name0)!=0)throw std::runtime_error("Cannot load required Animation model: "+actor_.properties.name0);
        services_.install_animation_actor_events(model(),model());
        for(auto n=frames_.actor_child_count(model());n;)services_.install_animation_actor_events(frames_.actor_child(model(),--n),model());
        const auto frame=actor_.base().frame();const auto position=frames_.actor_position(frame);
        frames_.object_position(model(),{{position[0],position[1],position[2]}});frames_.object_rotation(model(),frames_.actor_rotation(frame));
        frames_.animation_actor_scale(model(),frames_.actor_local_scale(frame));
        frames_.link_scene_object(model(),frames_.actor_parent(frame));frames_.animation_actor_name(model(),frames_.frame_name(frame)+"_aanim");
        frames_.object_visible(model(),false);frames_.update_scene_object(model());
        frames_.enumerate_animation_actor_materials(frame,[&](Frame child){ //005B45B0.
            auto id=actor_material_id(frames_.actor_user_data(child));if(id<0)id=11; //005B5F20 fallback.
            const auto word=services_.animation_actor_material_word(static_cast<std::uint32_t>(id),0x28);
            if(word>1500){actor_.fieldd8=1;return true;}
            if(word>505){actor_.fieldd8=2;return true;}
            return false;
        });
        if(frames_.frame_kind(frame)==9) {
            auto resource=frames_.car_model_resource(frame);for(auto& c:resource)if(c>='A'&&c<='Z')c=static_cast<char>(c+('a'-'A'));
            if(resource=="_mchescape") {
                if(frames_.actor_find(frame,"right_wing",0xffff)||frames_.actor_find(frame,"left_wing",0xffff))throw std::runtime_error("Animation escape-aircraft wings require actual ancillary wing owners");
            }
        }
        partial_=false;
    }
    void deactivate(bool authority)override { //005B5000 common-first, actual mode2 authority skip.
        actor_.base().deactivate_frame(authority,services_);
        if(authority&&(services_.mode_flags()&2u))return;
        actor_.flaga0=actor_.flaga1=false;
        if(actor_.field98)throw std::runtime_error("Animation retirement requires actual interaction98 receiver");
        retire();
    }
    void update(std::uint32_t)override { //005B5190, reached initial inactive flags and actual A2 colliders.
        if(flag_a3_||actor_.flaga0)throw std::runtime_error("Animation first update requires actual start/track receiver");
        if(actor_.properties.flaga2&&((!actor_.flaga1&&!actor_.flaga0)||count_bc_)) {
            if(count_bc_)--count_bc_;
            if(actor_.collisions)actor_.collisions->update();
            if(!actor_.fieldc4.empty())throw std::runtime_error("Animation wing updates require actual ancillary owners");
        }
    }
private:
    void retire() { //005B4790 ordering, before common Actor frame destruction.
        frontend::CleanupErrors errors;
        if(actor_.collisions)actor_.collisions->clear();
        if(model()) {
            const auto helper=model();errors.attempt([&]{services_.remove_animation_actor_events(helper);});
            //Callback bindings include direct children; bridge removes all owners
            //of this helper before the actual parent unlink/frame release.
            errors.attempt([&]{frames_.link_scene_object(helper,Frame{});});
        }
        model_.reset(); //Also retire a failed factory's engaged null owner.
        if(const auto frame=actor_.base().frame()) {
            errors.attempt([&]{frames_.object_visible(frame,true);});errors.attempt([&]{frames_.update_scene_object(frame);});
        }
        actor_.fieldd4=0;partial_=false;errors.rethrow();
    }
    Frames& frames_;Services& services_;Collisions& collisions_;Actor& actor_;std::optional<ModelOwner> model_;
    Frame particle_{};std::uint32_t count_bc_=0;bool flag_a3_=false,flag_b8_=false,partial_=false;
};
} //namespace hd2::game
#endif
