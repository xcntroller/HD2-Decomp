#ifndef HD2_GAME_EFFECTS_SPECIAL_MODEL_H
#define HD2_GAME_EFFECTS_SPECIAL_MODEL_H

#include "frontend/cleanup.h"
#include "io/resource_observations.h"
#include <cstdint>
#include <cstring>
#include <optional>
#include <stdexcept>
#include <utility>

namespace hd2::game {

//GEffects2E0..2F0,004870A0/004871A0 and00487416..004874BF. The native
//"special model" is a kind2 frame (MP_Lightning for the timed producer).
//Keep its WORD-E4 reference in the existing frame owner, not a resource cache.
template<class Frames>class EffectsSpecialModel {
public:
    using Frame=typename Frames::Frame;
    using Owner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    explicit EffectsSpecialModel(Frames& frames):frames_(frames){}
    ~EffectsSpecialModel(){try{configure(false,Frame{},true);}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}}
    EffectsSpecialModel(const EffectsSpecialModel&)=delete;
    EffectsSpecialModel& operator=(const EffectsSpecialModel&)=delete;
    Frame frame()const noexcept{return frame_?frame_->get():Frame{};}
    bool pending()const noexcept{return pending_;}
    std::int32_t remaining()const noexcept{return signed_word(remaining_);}
    void configure(bool enabled,Frame value,bool immediate){ //004870A0, ret0C.
        if(enabled){
            if(frame_||!value)return;
            frame_.emplace(frames_.retain_actor_frame(value));
            try{
                mode_=frames_.effects_special_mode(value);mode_known_=true;
                frames_.effects_special_mode(value,0x11);
                frames_.object_visible(value,true);frames_.update_scene_object(value);
                power_=frames_.effects_special_power(value);power_known_=true;
                pending_=false;remaining_=0xffffffffu;
                io::observe_bound_data(frames_,"execution","004870A0 enabled","mode160",mode_);
                observe_power("004870A0 saved power168",power_);
            }catch(...){
                frontend::CleanupErrors errors(std::current_exception());
                errors.attempt([&]{retire();});errors.rethrow();
            }
        }else if(frame_){
            if(!immediate){pending_=true;io::observe_bound_data(frames_,"execution","004870A0 deferred retirement");return;}
            retire();
        }
    }
    void lightning(std::int32_t duration){ //004871A0, ret4.
        //Native overwrites2E0 even on invalid/repeated input. Reject that unsafe
        //source prefix rather than leak or release an unacquired reference.
        if(frame_)throw std::logic_error("Retire the actual GEffects special frame before timed replacement");
        const auto value=frames_.find_scene_frame("MP_Lightning",0xffff);
        io::observe_bound_data(frames_,"population","004871A0 selected frame","MP_Lightning",value!=Frame{});
        if(!value)return;
        if(frames_.frame_kind(value)!=2)throw std::runtime_error("GEffects MP_Lightning requires its actual kind2 frame");
        configure(true,value,false);std::memcpy(&remaining_,&duration,4);
    }
    void update(std::uint32_t elapsed){
        if(!frame_)return;
        //FSTS keeps the extended product for comparison, but stores FLOAT for
        //virtual88. One SAME RandFloat call, including pending retirement.
        const long double amount=static_cast<long double>(frames_.volume_random_unit())*power_;
        const float stored=static_cast<float>(amount);
        observe_power("00487429 flicker FLOAT",stored);
        const auto value=frame();
        if(amount>.5L){
            if(!frames_.car_frame_visible(value))frames_.object_visible(value,true);
            frames_.effects_special_power(value,stored);frames_.update_scene_object(value);
        }else if(frames_.car_frame_visible(value)){
            //The native unordered/NaN comparison also reaches this branch.
            frames_.object_visible(value,false);frames_.update_scene_object(value);
        }
        if(pending_)configure(false,Frame{},true);
        //Native still subtracts2F0 after retiring2E0. Expiry marks retirement
        //for the NEXT update, which consumes its random draw before restoring.
        if(signed_word(remaining_)>0){remaining_-=elapsed;if(signed_word(remaining_)<=0)pending_=true;}
    }
private:
    static std::int32_t signed_word(std::uint32_t bits){std::int32_t result;std::memcpy(&result,&bits,4);return result;}
    void retire(){
        const auto value=frame();frontend::CleanupErrors errors;
        if(mode_known_)errors.attempt([&]{frames_.effects_special_mode(value,mode_);});
        if(power_known_)errors.attempt([&]{frames_.effects_special_power(value,power_);});
        errors.attempt([&]{frames_.object_visible(value,false);});
        frame_.reset();mode_known_=power_known_=false; //Release before clearing native2E0.
        io::observe_bound_data(frames_,"execution","004870A0 retired");
        errors.rethrow();
    }
    void observe_power(const char* operation,float power)const noexcept{
        std::uint32_t bits;std::memcpy(&bits,&power,4);
        io::observe_bound_data(frames_,"execution",operation,"FLOAT bits",bits);
    }
    Frames& frames_;std::optional<Owner> frame_;std::uint32_t mode_=0,remaining_=0;
    float power_=0;bool mode_known_=false,power_known_=false,pending_=false;
};
} //namespace hd2::game
#endif
