#ifndef HD2_GAME_FOWL_BIRD_H
#define HD2_GAME_FOWL_BIRD_H

#include "frontend/cleanup.h"
#include "game/bird_motion.h"
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <utility>

namespace hd2::game {

//004797C0/004799E0 and the generated-model branch of00479C70/0047A2F0.
//Generated birds own a retained frame while the actual scene owns membership.
//Borrowed scene birds/free-list transfer require their separate actual pool.
//Frames/Services/scene outlive this owner; close before resetting the scene.
template<class Frames,class Services>class FowlBird {
public:
    using Frame=typename Frames::Frame;using Vector=typename Frames::Vector;
    using Owner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    FowlBird(Frames& frames,Services& services,Frame frame)
        :frames_(frames),services_(services),frame_(frames.retain_actor_frame(frame)) {
        try {
            motion_state_.timer40=static_cast<std::uint32_t>(fowl_random_word(services_)%5000+5000);
            motion_state_.field48=static_cast<float>(fowl_random_word(services_)%20000+3000);
            frames_.actor_link_preserving_world(frame,frames_.camera_root_sector());frames_.update_scene_object(frame);
            if(frames_.actor_opaque_data(frame))throw std::runtime_error("Generated Fowl frame unexpectedly borrows an existing bird record");
            motion_state_.initial_position=motion_state_.position=frames_.actor_position(frame);motion_state_.rotation=frames_.actor_rotation(frame);
            frames_.actor_opaque_data(frame,this);
            motion_state_.child54=frames_.actor_find(frame,"Object03",0xffff);if(motion_state_.child54)frames_.object_visible(motion_state_.child54,false);
            motion_state_.child58=frames_.actor_find(frame,"Object01",0xffff);if(motion_state_.child58)frames_.object_visible(motion_state_.child58,false);
        }catch(...){
            try{if(frames_.actor_opaque_data(frame)==this)frames_.actor_opaque_data(frame,nullptr);}
            catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
            closed_=true;throw; //Caller retains scene-publication rollback until construction succeeds.
        }
    }
    ~FowlBird(){cleanup();}
    FowlBird(const FowlBird&)=delete;FowlBird& operator=(const FowlBird&)=delete;
    Frame frame()const{return frame_.get();}const Vector& position()const{return motion_state_.position;}
    std::uint32_t state()const{return motion_state_.state64;}bool flying()const{return motion_state_.flag4c;}bool flapping()const{return motion_state_.flag4d;}
    void close() { //004799E0 flag60=true: actual scene removal, no borrowed-pool restoration.
        if(closed_)return;
        closed_=true;frontend::CleanupErrors errors;const auto model=frame();
        errors.attempt([&]{if(frames_.actor_opaque_data(model)==this)frames_.actor_opaque_data(model,nullptr);});
        errors.attempt([&]{frames_.remove_model(model);});motion_state_.child54=motion_state_.child58=Frame{};errors.rethrow();
    }
    void launch() { //0047A2F0.
        require_open();if(motion_state_.flag4c)return;
        motion_state_.speed5c=stored(fowl_random(services_,2.f)+5.f);
        const auto z=stored(fowl_random(services_,.5f)),x=stored(fowl_random(services_,.5f));motion_state_.velocity={{x,1,z,0}};
        const auto world=frames_.actor_world_matrix(frame());long double dx=world[8];float dy=world[9],dz=world[10];
        normalize(dx,dy,dz);
        dx+=motion_state_.velocity[0];motion_state_.velocity[0]=stored(dx);dy=stored(static_cast<long double>(dy)+motion_state_.velocity[1]);dz=stored(static_cast<long double>(dz)+motion_state_.velocity[2]);
        motion_state_.velocity[1]=dy;motion_state_.velocity[2]=dz;normalize(dx,dy,dz);
        const auto scalar=static_cast<long double>(motion_state_.speed5c)*.0010000000474974513f;
        motion_state_.velocity[0]=stored(dx*scalar);motion_state_.velocity[1]=stored(static_cast<long double>(dy)*scalar);motion_state_.velocity[2]=stored(static_cast<long double>(dz)*scalar);
        motion_state_.phase44=stored(3.1415927410125732f/static_cast<long double>(fowl_random_word(services_)%100+1));motion_state_.state64=0;motion_state_.flag4d=true;
        if(motion_state_.child54&&motion_state_.child58){motion_state_.flag4c=true;frames_.object_visible(motion_state_.child54,true);frames_.object_visible(motion_state_.child58,true);}
    }
    bool update(std::uint32_t elapsed) { //0047A2E0 signed DWORD→FLOAT before00479C70.
        require_open();BirdMotion<Frames,Services> motion(frames_,services_,frame(),motion_state_);return motion.update(elapsed);
    }
private:
    static float stored(long double value){return static_cast<float>(value);}
    static void normalize(long double& x,float& y,float& z) {
        const auto square=(static_cast<long double>(y)*y+static_cast<long double>(z)*z)+x*x;
        const auto rounded=stored(square);constexpr float epsilon=9.99999993922529e-09f;
        if(!std::isfinite(square))throw std::runtime_error("Non-finite Fowl direction");
        if(std::abs(square-1)>=epsilon) {
            if(rounded>=epsilon){const auto scale=1.L/std::sqrt(static_cast<long double>(rounded));x*=scale;y=stored(y*scale);z=stored(z*scale);}
            else if(x!=0)x=x<0?-1:1;
            else if(z!=0)z=z<0?-1.f:1.f;
            else y=y<0?-1.f:1.f;
        }
    }
    void require_open()const{if(closed_)throw std::logic_error("Fowl bird is closed");}
    void cleanup()noexcept{try{close();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}}
    Frames& frames_;Services& services_;Owner frame_;BirdMotionState<Frame,Vector> motion_state_;bool closed_=false;
};
} //namespace hd2::game
#endif
