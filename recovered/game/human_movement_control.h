#ifndef HD2_GAME_HUMAN_MOVEMENT_CONTROL_H
#define HD2_GAME_HUMAN_MOVEMENT_CONTROL_H

#include "game/human_joint_owners.h"
#include "game/collision_vertical_limits.h"
#include <type_traits>

namespace hd2::game {

//DD0+AC0 binding00763780 and loading reset007634F0. Required Controller
//supplies its actual live descriptor list, SAME world exclusion owner and A90
//falling flag; Sweep is the concrete loaded-tree CollisionSweep. No no-hit or
//successful optional callback substitutes for the real ground query.
template<class Human,class Controller>
class HumanMovementControl {
public:
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
    void close_loading() { //007637C0→007706F0(true), the proven loading branch.
        //No motion-sound producer is exposed by this owner. A populated native
        //11C requires its real pool-return/volume lifetime, never a silent clear.
        if(state_.template get<std::uint32_t>(0x11c))throw std::runtime_error("Human movement sound requires its actual return owner");
        state_.set(0x120,std::int32_t{-10000});
    }
private:
    Controller* controller_=nullptr;Human* human_=nullptr;
    const void* aa0_=nullptr;const void* c00_=nullptr;
    HumanJointWords<0x140> state_; //Unwritten native/SIMD padding excluded.
};
} //namespace hd2::game
#endif
