#ifndef HD2_GAME_ANIMATION_ACTOR_LOADING_H
#define HD2_GAME_ANIMATION_ACTOR_LOADING_H
#include "game/model_scene_actors.h"
#include "game/animation_actor_frame.h"
#include "game/actor_material.h"
#include "frontend/cleanup.h"
#include <cctype>
namespace hd2::game {
//005B3BE0/005B5000/005B4790 loading and positive005B5190/005B55E0 frames.
//The containing world supplies actual resources/callbacks/materials/buckets.
//Same static interaction flags, track/model and SOUND/particle/Script owners.
//Special escape-aircraft wing and selected non7 publication stay required.
template<class Frames,class Services,class Collisions>class AnimationActorLoading final:public AnimationLoadingLifetime<typename Frames::Frame> {
public:
    using Actor=AnimationSceneActor<Frames>;using Frame=typename Frames::Frame;
    using ModelOwner=decltype(std::declval<Frames&>().create_animation_actor_model());
    AnimationActorLoading(Frames& frames,Services& services,Collisions& collisions,Actor& actor)
        :frames_(frames),services_(services),collisions_(collisions),actor_(actor){}
    ~AnimationActorLoading()override{try{retire();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}}
    Frame model()const override{return model_?model_->get():Frame{};}
    void activate(bool authority)override { //005B3BE0.
        actor_.flaga0=false;actor_.flaga3=false;actor_.fieldd4=0;flag_b8_=false;count_bc_=0;
        actor_.base().name(actor_.properties.fieldb0==0xffffffffu?std::string{}:services_.actor_localized(actor_.properties.fieldb0));
        actor_.base().activate_frame(authority,services_);
        if(authority&&(services_.mode_flags()&2u))return;
        if(model_||partial_)throw std::logic_error("Retire existing/partial Animation helper model before retry");
        partial_=true;actor_.field98=Frame{};particle_=frames_.actor_find(actor_.base().frame(),"particle",0xffff);
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
        if(!actor_.fieldc4.empty())throw std::runtime_error("Animation retirement requires actual ancillary wing owners");
        if(actor_.field98){interaction_collision(actor_.field98,true);actor_.field98=Frame{};}
        retire();
    }
    void update(std::uint32_t elapsed)override {advance_animation_actor_frame(actor_,*this,elapsed);}
    void start(bool replicated)override {start_animation_actor_frame(actor_,*this,replicated);}
    void finish_animation(){finish_animation_actor_frame(actor_,*this);}
    std::uint32_t mode_flags()const{return services_.mode_flags();}
    bool replay_active(){return services_.animation_actor_process().restoring;}
    Frame find_interaction_frame(const std::string& name){return frames_.game_scene_find(name,0xffffu);}
    void interaction_collision(Frame frame,bool restore){services_.animation_interaction_collision(frame,restore);}
    void clear_collisions(){if(actor_.collisions)actor_.collisions->clear();}
    void rebuild_collisions(Frame frame){
        if(!actor_.collisions)actor_.collisions=std::make_unique<AnimationActorCollisions<Frames,Collisions,Actor>>(frames_,collisions_,actor_);
        actor_.collisions->rebuild_from_model(frame);
    }
    void update_collisions(){if(actor_.collisions)actor_.collisions->update();}
    void update_wings(){if(!actor_.fieldc4.empty())throw std::runtime_error("Animation wing updates require actual ancillary owners");}
    std::uint32_t collision_countdown()const{return count_bc_;}
    void decrement_collision_countdown(){--count_bc_;}
    bool paused()const{return flag_b8_;}
    Frame particle_frame()const{return particle_;}
    Frame clip_zero(){if(!model())throw std::runtime_error("Animation track query reached native NULL helper model");return frames_.animation_actor_clip(model());}
    void clip_start_flag(Frame clip){if(!clip)throw std::runtime_error("Animation start reached native missing second track query");frames_.animation_actor_clip_start(clip);}
    std::uint32_t clip_time(){return frames_.animation_actor_clip_time(model());}
    std::uint32_t clip_duration(Frame clip){if(!clip)throw std::runtime_error("Animation duration reached native missing second track query");return frames_.animation_actor_clip_duration(clip);}
    void tick_model(std::uint32_t elapsed){frames_.supply_model_tick(model(),elapsed);}
    void model_visibility(Frame frame,bool visible){frames_.object_visible(frame,visible);frames_.update_scene_object(frame);}
    void emit_start_sound(std::uint32_t id){services_.animation_start_sound(actor_,particle_?particle_:actor_.base().frame(),id);}
    void emit_start_particle(std::uint32_t id){services_.animation_start_particle(particle_,id);}
    void script_death(Frame frame){services_.animation_script_death(frame);}
    void publish_started(bool replicated){services_.publish_animation_started(actor_,replicated);}
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
    Frame particle_{};std::uint32_t count_bc_=0;bool flag_b8_=false,partial_=false;
};
} //namespace hd2::game
#endif
