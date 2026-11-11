#ifndef HD2_GAME_HUMAN_MOVEMENT_CONTROL_H
#define HD2_GAME_HUMAN_MOVEMENT_CONTROL_H

#include "game/human_joint_owners.h"
#include "game/collision_vertical_limits.h"
#include "game/human_state_frame_timers.h"
#include "game/human_ai_shot_direction.h"
#include <optional>
#include <type_traits>

namespace hd2::game {

//DD0+AC0 binding00763780 and loading reset007634F0. Required Controller
//supplies its actual live descriptor list, SAME world exclusion owner and A90
//falling flag; Sweep is the concrete loaded-tree CollisionSweep. No no-hit or
//successful optional callback substitutes for the real ground query.
template<class Human,class Controller>
class HumanMovementControl {
public:
    using SoundOwner=typename Controller::SoundOwner;
    template<class Frames,class Resources,class Sweep,class Frame>
    void bind(Controller& controller,Human& human,const void* aa0,const void* c00,
        const CollisionTree<Frame>* tree,Frames& frames,Resources& resources,Sweep& sweep) {
        controller_=&controller;human_=&human;aa0_=aa0;c00_=c00;
        const auto limits=collision_vertical_limits(tree,resources,frames);
        state_.set(0x118,limits.maximum); //Min output overwrites caller's unused stack argument.
        reset(frames,sweep);
    }
    template<class Frames,class Sweep>void reset(Frames& frames,Sweep& sweep) {
        if(!controller_||!human_)throw std::runtime_error("Human movement control has no actual owners");
        constexpr std::uint32_t sentinel=0xc61c4066u; //00816774=-10000.099609375.
        for(auto offset:{0x5cu,0x84u,0x20u,0x10cu,0x108u,0x114u,0xfcu})state_.set(offset,sentinel);
        for(auto offset:{0x60u,0x64u,0xd0u,0x120u,0x130u})state_.set(offset,std::int32_t{-10000});
        for(auto offset:{0xf4u,0xf8u,0x70u,0x74u,0x78u,0x80u,0xdcu,0xe0u,0x9cu,0x54u,0xecu,0xd8u,
            0x90u,0x110u,0x30u,0x11cu,0x34u,0x124u,0x38u,0xf0u})state_.set(offset,std::uint32_t{0});
        for(auto offset:{0x88u,0x50u,0x51u,0xe4u,0x104u,0x98u,0x12cu})state_.set(offset,std::uint8_t{0});
        state_.set(0x68,std::uint32_t{250});state_.set(0xe8,std::uint32_t{1});state_.set(0x94,std::uint32_t{10000});
        state_.set(0x128,std::uint32_t{20});state_.set(0x58,1.f);state_.set(0x8c,std::uint32_t{0x3c75c28f});
        for(auto offset:{0u,4u,8u,0x10u,0x14u,0x18u})state_.set(offset,std::uint32_t{0});
        state_.set(0xa0,human_->base().world_direction());
        if(human_->properties.field19c!=1)return;
        auto& exclusions=controller_->exclusions();
        const auto descriptors=controller_->query_descriptors();
        exclusions.many(descriptors.data(),descriptors.size()); //0075C960(false)→006EFB10.
        using Exclusions=std::remove_reference_t<decltype(exclusions)>;
        struct Restore {Exclusions& value;~Restore(){value.clear();}} restore{exclusions}; //0075CAC0(false)→006EFA70(0,0).
        const auto model=frames.actor_world_matrix(human_->base().frame());
        std::array<float,4> origin{{model[12],model[13]+1,model[14],0}};
        const auto hit=sweep.sweep(origin,{{0,-1000,0,0}},.2f,0,0);
        if(hit) {
            const auto current=frames.actor_world_matrix(human_->base().frame());
            const float dx=hit->position[0]-current[12],dy=hit->position[1]-current[13],dz=hit->position[2]-current[14];
            const auto square=(static_cast<long double>(dx)*dx+static_cast<long double>(dy)*dy)+static_cast<long double>(dz)*dz;
            if(square<=400||hit->position[1]<=state_.template get<float>(0x118))return;
        }
        controller_->falling(true);
    }
    const auto& state() const {return state_;}
    auto& frame_words(){return state_;}
    void control_frame_track_velocity(const HumanVector& value){ //00787270, DD0+B70. Native W is unwritten.
        for(unsigned n=0;n!=3;++n)state_.set(0xb0+4*n,value[n]);
    }
    void control_frame_motion_mode(std::uint32_t mode){state_.set(0xe8,mode);} //DD0+BA8.
    template<class Countdown,class Replay>void advance_presentation_replay(Countdown&& countdown,Replay&& replay){ //00787D50, DD0+B48/BA4/B4C/AE0.
        constexpr float inactive=-10000.099609375f; //00816C6C.
        if(!(state_.template get<float>(0x20)>inactive))return;
        state_.set(0x88,std::uint8_t{1});
        state_.set(0xe4,std::uint8_t{0});
        countdown();
        state_.set(0x8c,.008f); //0084E8F0.
        if(replay())state_.set(0x20,inactive);
    }
    void cancel_presentation_replay(){state_.set(0x88,std::uint8_t{0});}
    void frame_climb_direction(bool backward){state_.set(0x12c,static_cast<std::uint8_t>(backward));} //DD0+BEC.
    void animation_control_delay(std::uint32_t value){state_.set(0x130,value);} //DD0+BF0,007870D0.
    template<class Frames,class State>void align_ground_direction(Frames& frames,Human& human,State& control,
            std::int32_t elapsed,const HumanVector* ladder_delta){ //0076FBC0, actual ActorBase+40 and DD0+250.
        auto direction=human.base().world_direction();
        const auto animation=control.template get<std::int32_t>(0x1d8);
        if(animation>=106&&animation<=108){
            if(!ladder_delta)return;
            auto change=static_cast<long double>(frames.car_vector_angle(*ladder_delta,{{0,1,0,0}}))*-.8333333134651184f-direction[1];
            if(std::abs(change)<9.99999993922529e-09)return; //0080FAC8, DOUBLE.
            if(static_cast<long double>(elapsed)*.0017999999690800905f<std::abs(change))
                change=(change>0?1.L:-1.L)*static_cast<float>(elapsed)*.0017999999690800905f;
            if(std::abs(change)>.15000000596046448)
                change=(change>0?1.L:-1.L)*.15000000596046448f;
            direction[1]=static_cast<float>(direction[1]+change);
            normalize_ground_direction(direction);
            human.base().world_direction(direction);
            return; //Native ladder branch does not publish AC0+50 or level DD0+250.
        }
        if(direction[1]!=0){
            auto change=-static_cast<long double>(direction[1]);
            if(std::abs(change)>=.05000000074505806){ //00810900, DOUBLE.
                const float rate=control.template get<std::uint8_t>(0x98c)?.018f:.0009f;
                control.set(0x98c,std::uint8_t{1});
                if(static_cast<long double>(elapsed)*rate<std::abs(change))
                    change=(change>0?1.L:-1.L)*static_cast<float>(elapsed)*rate;
                direction[1]=static_cast<float>(change+direction[1]);
            }else{
                direction[1]=0;control.set(0x98c,std::uint8_t{0});
            }
            normalize_ground_direction(direction);
            human.base().world_direction(direction);
            state_.set(0x50,std::uint8_t{1});
        }
        const auto value=control.template get<float>(0x250);
        if(value==0)return;
        auto change=-static_cast<long double>(value);
        if(std::abs(change)<.05000000074505806){control.set(0x250,0.f);return;}
        if(static_cast<long double>(elapsed)*.001f<std::abs(change))
            change=(change>0?1.L:-1.L)*static_cast<float>(elapsed)*.001f;
        control.set(0x250,static_cast<float>(change+value));
    }
    template<class Frames,class Pool>void advance_motion_audio(Frames& frames,Pool& pool,std::uint32_t elapsed,
            bool floor_mode,bool below_height,bool current_player,bool alternate,
            typename Controller::Frame head,typename Controller::Frame body){ //00770770.
        const auto kind=state_.template get<std::uint32_t>(0x128);
        const auto allowed=[&]{return (current_player||!kind)&&(alternate!=(kind==0));};
        if(!sound_){
            if(floor_mode){
                state_.set(0x128,static_cast<std::uint32_t>(below_height));
                const auto selected=static_cast<std::uint32_t>(below_height);
                if((current_player||!selected)&&(alternate!=(selected==0))){
                    acquire_motion_audio(frames,pool,selected,head,true);
                    auto fade=state_.template get<std::int32_t>(0x120);
                    if(fade==-10000)fade=0;
                    else if(fade<0)fade=human_frame_signed_word(0u-static_cast<std::uint32_t>(fade));
                    state_.set(0x120,fade);
                }
                return;
            }
        }else if(floor_mode){
            auto fade=state_.template get<std::int32_t>(0x120);
            if(fade!=-10000){
                if(fade<0){
                    fade=human_frame_signed_word(static_cast<std::uint32_t>(fade)+elapsed);
                    state_.set(0x120,fade);
                    if(fade<0)frames.human_sound_volume(sound_->get(),static_cast<float>(static_cast<long double>(fade)*-.002f));
                    else close_motion_audio(frames,pool);
                    return;
                }
                if(!allowed()){
                    state_.set(0x120,std::int32_t{-10000});fade_motion_audio();
                }else{
                    fade=human_frame_signed_word(static_cast<std::uint32_t>(fade)+elapsed);
                    state_.set(0x120,fade);
                    if(fade<500){
                        frames.human_sound_volume(sound_->get(),static_cast<float>(static_cast<long double>(fade)*.002f));
                        return;
                    }
                    state_.set(0x120,std::int32_t{-10000});
                }
            }else if(!allowed()){
                state_.set(0x120,std::int32_t{-10000});
                fade_motion_audio();
                return;
            }
            const auto timer=state_.template get<std::uint32_t>(0x124)-elapsed;
            state_.set(0x124,timer);
            const auto selected=static_cast<std::uint32_t>(below_height);
            if(human_frame_signed_word(timer)<1||selected!=kind){
                auto previous=std::move(*sound_);sound_.reset();
                pool.give(std::move(previous));state_.set(0x128,selected);
                acquire_motion_audio(frames,pool,selected,body,false);
                if(!sound_)return;
            }
            frames.human_sound_volume(sound_->get(),1.f);
            return;
        }
        fade_motion_audio();
    }
    template<class Frames,class Pool>void close_loading(Frames& frames,Pool& pool) { //007637C0→007706F0(true).
        if(state_.template get<std::uint32_t>(0x11c))throw std::runtime_error("Human movement sound requires its actual return owner");
        close_motion_audio(frames,pool);
    }
    void fade_motion_audio(){ //007706F0(false), also called by0076AE70.
        const auto timer=state_.template get<std::int32_t>(0x120);
        if(timer==-10000)state_.set(0x120,std::int32_t{-500});
        else if(timer>0)state_.set(0x120,0u-static_cast<std::uint32_t>(timer));
    }
private:
    static void normalize_ground_direction(HumanVector& value){
        const auto square=(static_cast<long double>(value[2])*value[2]+
            static_cast<long double>(value[0])*value[0])+static_cast<long double>(value[1])*value[1];
        normalize_human_ai_shot(value,square);
    }
    template<class Frames,class Pool>void acquire_motion_audio(Frames& frames,Pool& pool,std::uint32_t kind,typename Controller::Frame parent,bool initial){ //007705E0/00770770.
        auto sound=pool.play(kind,parent,{{0,0,0}});
        if(sound){
            state_.set(0x124,frames.human_sound_duration(sound.get()));
            if(initial)frames.human_sound_volume(sound.get(),0.f);
            sound_.emplace(std::move(sound));
        }
    }
    template<class Frames,class Pool>void close_motion_audio(Frames& frames,Pool& pool){ //007706F0(true).
        if(sound_){
            frames.human_sound_volume(sound_->get(),0.f);
            auto previous=std::move(*sound_);sound_.reset();
            pool.give(std::move(previous));
        }
        state_.set(0x120,std::int32_t{-10000});
    }
    Controller* controller_=nullptr;Human* human_=nullptr;
    const void* aa0_=nullptr;const void* c00_=nullptr;
    HumanJointWords<0x140> state_; //Unwritten native/SIMD padding excluded.
    std::optional<SoundOwner> sound_; //Native11C, returned before the shared SOUND lease.
};
} //namespace hd2::game
#endif
