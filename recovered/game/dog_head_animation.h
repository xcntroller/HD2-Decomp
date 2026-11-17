#ifndef HD2_GAME_DOG_HEAD_ANIMATION_H
#define HD2_GAME_DOG_HEAD_ANIMATION_H
#include "game/human_joint_math.h"
#include <type_traits>
#include <utility>
#include <exception>
namespace hd2::game {
//Actual Actor4E0 animation-set owner. Native first animation receives08/1C.
//Live005D4DB0 must construct/publish the real engine resource; no idle default.
class DogHeadAnimationLifetime {
public:
    virtual ~DogHeadAnimationLifetime()=default;
    virtual void reset()=0;
    virtual void direction(const HumanVector&)=0;
    virtual void close()=0;
};
//005D4DB0/005D5390 engine resource prefix. The actual animation set owns its
//animation array; the model and caller retain this lifetime until retirement.
template<class Frames>class DogHeadAnimation final:public DogHeadAnimationLifetime {
public:
    using Frame=typename Frames::Frame;
    using Owner=decltype(std::declval<Frames&>().create_dog_head_animation_set());
    explicit DogHeadAnimation(Frames& frames):frames_(frames),owner_(frames.create_dog_head_animation_set()){
        if(!owner_)throw std::runtime_error("Dog head animation set allocation failed");
    }
    ~DogHeadAnimation()override{try{close();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}}
    template<class Model>Frame bind_model(Model&& model){
        if(!owner_)throw std::logic_error("Dog head animation owner is retired");
        frames_.add_dog_head_animation(owner_.get(),"f");
        frames_.bind_dog_head_animation(model(),owner_.get());
        return frames_.actor_find(model(),"f",0xffffu);
    }
    void reset()override{require();frames_.reset_dog_head_animation(owner_.get());}
    void direction(const HumanVector& value)override{require();frames_.set_dog_head_animation_direction(owner_.get(),value);}
    void close()override{if(owner_){frames_.clear_dog_head_animations(owner_.get());owner_.reset();}}
private:
    void require()const{if(!owner_)throw std::runtime_error("Dog head animation is retired");}
    Frames& frames_;Owner owner_;
};
} //namespace hd2::game
#endif
