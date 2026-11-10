#ifndef HD2_GAME_EFFECTS_BIRDS_LOADING_H
#define HD2_GAME_EFFECTS_BIRDS_LOADING_H

#include "frontend/cleanup.h"
#include <memory>
#include <string>
#include <vector>

namespace hd2::game {

//0089BA80 bird owner, separate from actor Fowl and GEffects+20 decorations.
//0047A790/A7B0 supplies model enumeration; only004797C0's borrowed-model false
//branch is reached here. No moving/spawned bird producer or simulation exists.
template<class Frames,class Random>class EffectsBirdsLoading {
public:
    using Frame=typename Frames::Frame;using Vector=typename Frames::Vector;
    using Owner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    struct Bird {
        Bird(Owner owner,std::uint32_t timer,float amount):model(std::move(owner)),timer40(timer),field48(amount){}
        Owner model;Vector velocity{},initial_position{},position{},rotation{};
        std::uint32_t timer40=0,field44=0;float field48=0,speed5c=5;
        bool flag4c=false,flag4d=false,spawned60=false,position_ready=false,rotation_ready=false;
        std::int32_t state64=3;Frame child54{},child58{};
        //Native velocity W is unwritten; no consumer of it is supplied.
    };
    EffectsBirdsLoading(Frames& frames,Random& random):frames_(frames),random_(random){}
    ~EffectsBirdsLoading(){try{clear();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}}
    EffectsBirdsLoading(const EffectsBirdsLoading&)=delete;EffectsBirdsLoading& operator=(const EffectsBirdsLoading&)=delete;
    const auto& values()const{return birds_;}
    void load(){ //0047A790 scene44 mask80, lowercase bounded NAME, not user metadata.
        frames_.enumerate_activation_scene(false,0x80,[&](Frame frame){
            auto name=frames_.frame_name(frame);if(name.size()>=256)throw std::runtime_error("Bird frame name exceeds native callback buffer");
            for(auto& c:name)if(c>='A'&&c<='Z')c=char(c-'A'+'a');
            if(name.find("ptacekletacek")!=std::string::npos)add(frame);
        });
    }
    void add(Frame frame){ //004797C0; two shared CRT draws precede engine transforms.
        const auto timer=random_()%5000u+5000u;const auto amount=random_()%20000u+3000u;
        auto bird=std::make_unique<Bird>(frames_.retain_actor_frame(frame),timer,static_cast<float>(amount));
        //Publish local rollback ownership before any frame mutation.
        auto* current=bird.get();birds_.push_back(std::move(bird));
        frames_.effects_link_root(frame);frames_.update_scene_object(frame);
        auto position=frames_.actor_position(frame);
        const auto old=frames_.actor_opaque_data(frame);
        if(old){
            const Bird* prior=nullptr;
            for(const auto& candidate:birds_)if(candidate.get()==old){prior=candidate.get();break;}
            if(!prior||!prior->position_ready)throw std::runtime_error("Bird metadata requires its actual saved-position owner");
            position=prior->initial_position;frames_.object_position(frame,{{position[0],position[1],position[2]}});frames_.update_scene_object(frame);
        }
        current->initial_position=current->position=position;current->position_ready=true;
        frames_.actor_opaque_data(frame,current);
        current->rotation=frames_.actor_rotation(frame);current->rotation_ready=true;
        current->child54=frames_.actor_find(frame,"Object03",0xffff);
        if(current->child54)frames_.object_visible(current->child54,false);
        current->child58=frames_.actor_find(frame,"Object01",0xffff);
        if(current->child58)frames_.object_visible(current->child58,false);
    }
    void clear(){ //0047AA10: reverse bird004799E0 retirement, then reverse deferred show/update.
        frontend::CleanupErrors errors;
        while(!birds_.empty()){
            auto bird=std::move(birds_.back());birds_.pop_back();const auto model=bird->model.get();
            if(frames_.actor_opaque_data(model)==bird.get())frames_.actor_opaque_data(model,nullptr);
            if(bird->position_ready)errors.attempt([&]{const auto& p=bird->initial_position;frames_.object_position(model,{{p[0],p[1],p[2]}});});
            if(bird->rotation_ready)errors.attempt([&]{frames_.object_rotation(model,bird->rotation);});
            errors.attempt([&]{frames_.object_visible(model,false);});errors.attempt([&]{frames_.update_scene_object(model);});
            errors.attempt([&]{deferred_.push_back(std::move(bird->model));});
        }
        while(!deferred_.empty()){
            auto owner=std::move(deferred_.back());deferred_.pop_back();
            errors.attempt([&]{frames_.object_visible(owner.get(),true);});errors.attempt([&]{frames_.update_scene_object(owner.get());});
        }
        errors.rethrow();
    }
private:
    Frames& frames_;Random& random_;std::vector<std::unique_ptr<Bird>> birds_;std::vector<Owner> deferred_;
};
} //namespace hd2::game
#endif
