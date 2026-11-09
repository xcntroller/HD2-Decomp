#ifndef HD2_GAME_FOWL_BIRD_H
#define HD2_GAME_FOWL_BIRD_H

#include "frontend/cleanup.h"
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <utility>

namespace hd2::game {

inline std::int32_t fowl_signed_word(std::uint32_t word){std::int32_t result;std::memcpy(&result,&word,4);return result;}
//00475190 consumes the SAME CRT stream, returning x87 extended, not a new RNG.
template<class Services>std::int32_t fowl_random_word(Services& services) {
    const auto value=services.fowl_random_word();
    if(value<0||value>32767)throw std::runtime_error("Fowl requires its actual fifteen-bit CRT random stream");
    return value;
}
template<class Services>long double fowl_random(Services& services,float range) {
    if(!(range>0))return 0;
    return static_cast<long double>(fowl_random_word(services))*range*3.0517578125e-05f;
}

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
            timer40_=static_cast<std::uint32_t>(fowl_random_word(services_)%5000+5000);
            timer48_=static_cast<float>(fowl_random_word(services_)%20000+3000);
            frames_.actor_link_preserving_world(frame,frames_.camera_root_sector());frames_.update_scene_object(frame);
            if(frames_.actor_opaque_data(frame))throw std::runtime_error("Generated Fowl frame unexpectedly borrows an existing bird record");
            initial_=current_=frames_.actor_position(frame);rotation_=frames_.actor_rotation(frame);
            frames_.actor_opaque_data(frame,this);
            wing54_=frames_.actor_find(frame,"Object03",0xffff);if(wing54_)frames_.object_visible(wing54_,false);
            wing58_=frames_.actor_find(frame,"Object01",0xffff);if(wing58_)frames_.object_visible(wing58_,false);
        }catch(...){
            try{if(frames_.actor_opaque_data(frame)==this)frames_.actor_opaque_data(frame,nullptr);}
            catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
            closed_=true;throw; //Caller retains scene-publication rollback until construction succeeds.
        }
    }
    ~FowlBird(){cleanup();}
    FowlBird(const FowlBird&)=delete;FowlBird& operator=(const FowlBird&)=delete;
    Frame frame()const{return frame_.get();}const Vector& position()const{return current_;}
    std::uint32_t state()const{return state_;}bool flying()const{return launched_;}bool flapping()const{return flapping_;}
    void close() { //004799E0 flag60=true: actual scene removal, no borrowed-pool restoration.
        if(closed_)return;
        closed_=true;frontend::CleanupErrors errors;const auto model=frame();
        errors.attempt([&]{if(frames_.actor_opaque_data(model)==this)frames_.actor_opaque_data(model,nullptr);});
        errors.attempt([&]{frames_.remove_model(model);});wing54_=wing58_=Frame{};errors.rethrow();
    }
    void launch() { //0047A2F0.
        require_open();if(launched_)return;
        speed_=stored(fowl_random(services_,2.f)+5.f);
        const auto z=stored(fowl_random(services_,.5f)),x=stored(fowl_random(services_,.5f));velocity_={{x,1,z,0}};
        const auto world=frames_.actor_world_matrix(frame());long double dx=world[8];float dy=world[9],dz=world[10];
        normalize(dx,dy,dz);
        dx+=velocity_[0];velocity_[0]=stored(dx);dy=stored(static_cast<long double>(dy)+velocity_[1]);dz=stored(static_cast<long double>(dz)+velocity_[2]);
        velocity_[1]=dy;velocity_[2]=dz;normalize(dx,dy,dz);
        const auto scalar=static_cast<long double>(speed_)*.0010000000474974513f;
        velocity_[0]=stored(dx*scalar);velocity_[1]=stored(static_cast<long double>(dy)*scalar);velocity_[2]=stored(static_cast<long double>(dz)*scalar);
        phase_=stored(3.1415927410125732f/static_cast<long double>(fowl_random_word(services_)%100+1));state_=0;flapping_=true;
        if(wing54_&&wing58_){launched_=true;frames_.object_visible(wing54_,true);frames_.object_visible(wing58_,true);}
    }
    bool update(std::uint32_t elapsed) { //0047A2E0 signed DWORD→FLOAT before00479C70.
        require_open();return update_float(static_cast<float>(fowl_signed_word(elapsed)),0);
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
    void normalize_velocity() { //479D9C stores (X²+Z²)+Y² before BOTH comparisons.
        const auto square=stored((static_cast<long double>(velocity_[0])*velocity_[0]+static_cast<long double>(velocity_[2])*velocity_[2])+static_cast<long double>(velocity_[1])*velocity_[1]);
        if(!std::isfinite(square))throw std::runtime_error("Non-finite Fowl velocity");
        if(std::abs(static_cast<long double>(square)-1)>=9.99999993922529e-09f) {
            if(square>=9.99999993922529e-09f){const auto scale=1.L/std::sqrt(static_cast<long double>(square));for(unsigned a=0;a!=3;++a)velocity_[a]=stored(velocity_[a]*scale);}
            else {const auto a=velocity_[0]!=0?0u:velocity_[2]!=0?2u:1u;velocity_[a]=velocity_[a]<0?-1.f:1.f;}
        }
    }
    bool update_float(float elapsed,unsigned depth) { //00479C70, native recursion on state0/1 transitions.
        if(!wing54_||!wing58_)return false;
        if(depth>3||!std::isfinite(elapsed))throw std::runtime_error("Invalid Fowl update recursion/time");
        bool changed=false;std::int32_t repeat=0;
        switch(state_) {
        case 0:
            if(static_cast<long double>(initial_[1])+10.f<current_[1]){state_=1;speed_=stored(static_cast<long double>(speed_)*.75f);repeat=repeat_time(elapsed);}
            break;
        case 1: {
            velocity_[1]=stored(static_cast<long double>(velocity_[1])-static_cast<long double>(elapsed)*5.000000555810402e-07f);normalize_velocity();
            if(velocity_[1]<0) {
                state_=2;velocity_[1]=-.1f;
                //479EB4 stores this X/Z/.01 square BEFORE the unit comparison.
                const auto square=stored((static_cast<long double>(velocity_[0])*velocity_[0]+static_cast<long double>(velocity_[2])*velocity_[2])+.010000000707805157f);
                if(std::abs(static_cast<long double>(square)-1)>=9.99999993922529e-09f) {
                    if(square>=9.99999993922529e-09f){const auto scale=1.L/std::sqrt(static_cast<long double>(square));for(unsigned a=0;a!=3;++a)velocity_[a]=stored(velocity_[a]*scale);}
                    else if(velocity_[0]!=0)velocity_[0]=velocity_[0]<0?-1.f:1.f;
                    else if(velocity_[2]!=0)velocity_[2]=velocity_[2]<0?-1.f:1.f;
                    else velocity_[1]=-1;
                }
                repeat=repeat_time(elapsed);
            }
            const auto scale=static_cast<long double>(speed_)*.0010000000474974513f;for(unsigned a=0;a!=3;++a)velocity_[a]=stored(velocity_[a]*scale);
            break;
        }
        case 2: {
            const auto camera=services_.fowl_camera_position();
            const auto x=static_cast<long double>(camera[0])-current_[0],y=static_cast<long double>(camera[1])-current_[1],z=static_cast<long double>(camera[2])-current_[2];
            const auto square=stored((y*y+z*z)+x*x);const auto range=services_.fowl_cull_distance();
            if(static_cast<long double>(range)*range<square||250000.f<square){wing54_=wing58_=Frame{};current_[1]=stored(static_cast<long double>(initial_[1])+10);return false;}
            break;
        }
        case 3: {
            const auto next=static_cast<long double>(timer48_)-elapsed;timer48_=stored(next);
            if((!flapping_&&next<0)||(flapping_&&current_[1]<initial_[1])) {
                flapping_=!flapping_;if(!flapping_)timer48_=static_cast<float>(fowl_random_word(services_)%20000+20000);
                frames_.object_visible(wing54_,flapping_);frames_.object_visible(wing58_,flapping_);
                frames_.object_position(frame(),{{initial_[0],initial_[1],initial_[2]}});current_=frames_.actor_position(frame());
                const auto jump=fowl_random(services_,.0010000000474974513f)+.0001;
                velocity_={{0,flapping_?stored(jump):0.f,0,0}};changed=true;
            }else if(flapping_)velocity_[1]=stored(static_cast<long double>(velocity_[1])-static_cast<long double>(elapsed)*9.999999974752427e-07f);
            break;
        }
        default:throw std::runtime_error("Unsupported Fowl bird movement state");
        }
        if(state_!=3||flapping_) {
            const auto dz=stored(static_cast<long double>(elapsed)*velocity_[2]);
            for(unsigned a=0;a!=2;++a)current_[a]=stored(static_cast<long double>(elapsed)*velocity_[a]+current_[a]);
            current_[2]=stored(static_cast<long double>(dz)+current_[2]);frames_.object_position(frame(),{{current_[0],current_[1],current_[2]}});changed=true;
        }
        const auto phase=static_cast<long double>(elapsed)*speed_*.007000000216066837f+phase_;phase_=stored(phase);
        if(flapping_){const auto angle=stored(std::sin(phase)*.75-.4000000059604645);frames_.fowl_wing_rotations(wing54_,wing58_,angle);}
        if(changed||flapping_)frames_.update_scene_object(frame());
        if(repeat)update_float(static_cast<float>(repeat),depth+1);
        return true;
    }
    static std::int32_t repeat_time(float elapsed) {
        if(static_cast<long double>(elapsed)<-2147483648.L||static_cast<long double>(elapsed)>2147483647.L)throw std::runtime_error("Fowl elapsed exceeds native integer conversion");
        return static_cast<std::int32_t>(elapsed);
    }
    void require_open()const{if(closed_)throw std::logic_error("Fowl bird is closed");}
    void cleanup()noexcept{try{close();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}}
    Frames& frames_;Services& services_;Owner frame_;Frame wing54_{},wing58_{};Vector velocity_{},initial_{},current_{};
    std::array<float,4> rotation_{};std::uint32_t timer40_=0,state_=3;float timer48_=0,phase_=0,speed_=5;bool launched_=false,flapping_=false,closed_=false;
};
} //namespace hd2::game
#endif
