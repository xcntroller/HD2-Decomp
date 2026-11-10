#ifndef HD2_GAME_BIRD_MOTION_H
#define HD2_GAME_BIRD_MOTION_H
#include "game/fowl_bird_random.h"
#include <array>
#include <cmath>
#include <stdexcept>
namespace hd2::game {
//Actual004797C0 scalar/movement values, shared by borrowed and generated birds.
//Frame ownership, registry/free-list membership and saved-pose validity belong
//to their distinct containing owners. No model membership is synthesized here.
template<class Frame,class Vector>struct BirdMotionState {
    Vector velocity{},initial_position{},position{},rotation{};
    std::uint32_t timer40=0;float phase44=0,field48=0,speed5c=5;
    bool flag4c=false,flag4d=false,spawned60=false,position_ready=false,rotation_ready=false;
    std::int32_t state64=3;Frame child54{},child58{};
};
//0047A2E0/00479C70; applies to SAME state/model, not a movement snapshot.
template<class Frames,class Services>class BirdMotion {
public:
    using Frame=typename Frames::Frame;using State=BirdMotionState<Frame,typename Frames::Vector>;
    BirdMotion(Frames& frames,Services& services,Frame model,State& state):frames_(frames),services_(services),model_(model),state_(state){}
    bool update(std::uint32_t elapsed){return update_float(static_cast<float>(fowl_signed_word(elapsed)),0);}
private:
    Frame frame()const{return model_;}
    static float stored(long double value){return static_cast<float>(value);}
    void normalize_velocity() { //479D9C stores (X²+Z²)+Y² before BOTH comparisons.
        const auto square=stored((static_cast<long double>(state_.velocity[0])*state_.velocity[0]+static_cast<long double>(state_.velocity[2])*state_.velocity[2])+static_cast<long double>(state_.velocity[1])*state_.velocity[1]);
        if(!std::isfinite(square))throw std::runtime_error("Non-finite Fowl velocity");
        if(std::abs(static_cast<long double>(square)-1)>=9.99999993922529e-09f) {
            if(square>=9.99999993922529e-09f){const auto scale=1.L/std::sqrt(static_cast<long double>(square));for(unsigned a=0;a!=3;++a)state_.velocity[a]=stored(state_.velocity[a]*scale);}
            else {const auto a=state_.velocity[0]!=0?0u:state_.velocity[2]!=0?2u:1u;state_.velocity[a]=state_.velocity[a]<0?-1.f:1.f;}
        }
    }
    bool update_float(float elapsed,unsigned depth) { //00479C70, native recursion on state0/1 transitions.
        if(!state_.child54||!state_.child58)return false;
        if(depth>3||!std::isfinite(elapsed))throw std::runtime_error("Invalid Fowl update recursion/time");
        bool changed=false;std::int32_t repeat=0;
        switch(state_.state64) {
        case 0:
            if(static_cast<long double>(state_.initial_position[1])+10.f<state_.position[1]){state_.state64=1;state_.speed5c=stored(static_cast<long double>(state_.speed5c)*.75f);repeat=repeat_time(elapsed);}
            break;
        case 1: {
            state_.velocity[1]=stored(static_cast<long double>(state_.velocity[1])-static_cast<long double>(elapsed)*5.000000555810402e-07f);normalize_velocity();
            if(state_.velocity[1]<0) {
                state_.state64=2;state_.velocity[1]=-.1f;
                //479EB4 stores this X/Z/.01 square BEFORE the unit comparison.
                const auto square=stored((static_cast<long double>(state_.velocity[0])*state_.velocity[0]+static_cast<long double>(state_.velocity[2])*state_.velocity[2])+.010000000707805157f);
                if(std::abs(static_cast<long double>(square)-1)>=9.99999993922529e-09f) {
                    if(square>=9.99999993922529e-09f){const auto scale=1.L/std::sqrt(static_cast<long double>(square));for(unsigned a=0;a!=3;++a)state_.velocity[a]=stored(state_.velocity[a]*scale);}
                    else if(state_.velocity[0]!=0)state_.velocity[0]=state_.velocity[0]<0?-1.f:1.f;
                    else if(state_.velocity[2]!=0)state_.velocity[2]=state_.velocity[2]<0?-1.f:1.f;
                    else state_.velocity[1]=-1;
                }
                repeat=repeat_time(elapsed);
            }
            const auto scale=static_cast<long double>(state_.speed5c)*.0010000000474974513f;for(unsigned a=0;a!=3;++a)state_.velocity[a]=stored(state_.velocity[a]*scale);
            break;
        }
        case 2: {
            const auto camera=services_.fowl_camera_position();
            const auto x=static_cast<long double>(camera[0])-state_.position[0],y=static_cast<long double>(camera[1])-state_.position[1],z=static_cast<long double>(camera[2])-state_.position[2];
            const auto square=stored((y*y+z*z)+x*x);const auto range=services_.fowl_cull_distance();
            if(static_cast<long double>(range)*range<square||250000.f<square){state_.child54=state_.child58=Frame{};state_.position[1]=stored(static_cast<long double>(state_.initial_position[1])+10);return false;}
            break;
        }
        case 3: {
            const auto next=static_cast<long double>(state_.field48)-elapsed;state_.field48=stored(next);
            if((!state_.flag4d&&next<0)||(state_.flag4d&&state_.position[1]<state_.initial_position[1])) {
                state_.flag4d=!state_.flag4d;if(!state_.flag4d)state_.field48=static_cast<float>(fowl_random_word(services_)%20000+20000);
                frames_.object_visible(state_.child54,state_.flag4d);frames_.object_visible(state_.child58,state_.flag4d);
                frames_.object_position(frame(),{{state_.initial_position[0],state_.initial_position[1],state_.initial_position[2]}});state_.position=frames_.actor_position(frame());
                const auto jump=fowl_random(services_,.0010000000474974513f)+.0001;
                state_.velocity={{0,state_.flag4d?stored(jump):0.f,0,0}};changed=true;
            }else if(state_.flag4d)state_.velocity[1]=stored(static_cast<long double>(state_.velocity[1])-static_cast<long double>(elapsed)*9.999999974752427e-07f);
            break;
        }
        default:throw std::runtime_error("Unsupported Fowl bird movement state");
        }
        if(state_.state64!=3||state_.flag4d) {
            const auto dz=stored(static_cast<long double>(elapsed)*state_.velocity[2]);
            for(unsigned a=0;a!=2;++a)state_.position[a]=stored(static_cast<long double>(elapsed)*state_.velocity[a]+state_.position[a]);
            state_.position[2]=stored(static_cast<long double>(dz)+state_.position[2]);frames_.object_position(frame(),{{state_.position[0],state_.position[1],state_.position[2]}});changed=true;
        }
        const auto phase=static_cast<long double>(elapsed)*state_.speed5c*.007000000216066837f+state_.phase44;state_.phase44=stored(phase);
        if(state_.flag4d){const auto angle=stored(std::sin(phase)*.75-.4000000059604645);frames_.fowl_wing_rotations(state_.child54,state_.child58,angle);}
        if(changed||state_.flag4d)frames_.update_scene_object(frame());
        if(repeat)update_float(static_cast<float>(repeat),depth+1);
        return true;
    }
    static std::int32_t repeat_time(float elapsed) {
        if(static_cast<long double>(elapsed)<-2147483648.L||static_cast<long double>(elapsed)>2147483647.L)throw std::runtime_error("Fowl elapsed exceeds native integer conversion");
        return static_cast<std::int32_t>(elapsed);
    }
    Frames& frames_;Services& services_;Frame model_;State& state_;
};
} //namespace hd2::game
#endif
