#ifndef HD2_GAME_DOG_ANIMATION_EVENTS_H
#define HD2_GAME_DOG_ANIMATION_EVENTS_H
#include "game/dog_clip_mixer.h"
#include <map>
#if defined(_WIN32)
#define HD2_DOG_ANIMATION_CALL __stdcall
#else
#define HD2_DOG_ANIMATION_CALL
#endif
namespace hd2::game {
//005C7960 has no callback context parameter. Bindings borrow actual marker,
//clip and actor lifetimes without replacing frame userdata. The live parentEC
//Actor resolution is still required on EACH callback. Publish actor.clip before
//initialize; close that clip before this bridge, its model or frame registry.
template<class Frames,class Dog>class DogAnimationEvents {
public:
    using Frame=typename Frames::Frame;using Clip=DogClipLifetime<Frame>;
    DogAnimationEvents(Frames& frames,Dog& actor):frames_(frames),actor_(actor){}
    ~DogAnimationEvents(){
        for(auto at=bindings_.begin();at!=bindings_.end();){
            if(at->second.owner!=this){++at;continue;}
            try{frames_.human_animation_callback(at->first,nullptr,0);}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
            at=bindings_.erase(at);
        }
    }
    DogAnimationEvents(const DogAnimationEvents&)=delete;
    DogAnimationEvents& operator=(const DogAnimationEvents&)=delete;
    void install_animation_events(Frame marker,Clip& clip){
        const auto found=bindings_.find(marker);
        if(found!=bindings_.end()&&(found->second.owner!=this||found->second.clip!=&clip))throw std::runtime_error("Dog animation marker already belongs to another owner");
        const bool added=found==bindings_.end();if(added)bindings_.emplace(marker,Binding{this,&clip});
        try{frames_.human_animation_callback(marker,&invoke,0x1000u);}catch(...){if(added)bindings_.erase(marker);throw;}
    }
    void remove_animation_events(Frame marker,Clip& clip){
        const auto found=bindings_.find(marker);
        if(found!=bindings_.end()&&(found->second.owner!=this||found->second.clip!=&clip))throw std::runtime_error("Dog animation callback retirement found another owner");
        frames_.human_animation_callback(marker,nullptr,0);if(found!=bindings_.end())bindings_.erase(found);
    }
private:
    struct Binding {DogAnimationEvents* owner;Clip* clip;};
    static std::int32_t HD2_DOG_ANIMATION_CALL invoke(Frame marker,std::uint32_t,const char* text,std::int32_t)noexcept{
        const auto found=bindings_.find(marker);if(found==bindings_.end())return 0; //Failed unregister already latched; never borrow a freed clip.
        auto& binding=found->second;auto& owner=*binding.owner;if(owner.failed_)return 0;
        try{
            if(!marker)throw std::runtime_error("Dog clip callback reaches native NULL marker");
            const auto parent=owner.frames_.actor_parent(marker);
            const auto* data=parent?owner.frames_.find_actor_frame_data(parent):nullptr;
            if(!data||!data->words[0]||reinterpret_cast<void*>(data->words[0])!=static_cast<void*>(&owner.actor_))throw std::runtime_error("Dog clip callback requires SAME actual parentEC Actor binding");
            if(owner.actor_.clip.get()!=binding.clip)throw std::runtime_error("Dog clip callback lost actual Actor+A0 owner");
            binding.clip->event(text);
        }catch(...){owner.failed_=true;owner.frames_.record_actor_cleanup_failure(std::current_exception());}
        return 0;
    }
    inline static std::map<Frame,Binding> bindings_;
    Frames& frames_;Dog& actor_;bool failed_=false;
};
}
#undef HD2_DOG_ANIMATION_CALL
#endif
