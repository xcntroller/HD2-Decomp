#ifndef HD2_GAME_PLACED_ITEM_ACTOR_H
#define HD2_GAME_PLACED_ITEM_ACTOR_H

#include "game/scene_actor.h"
#include "game/mission_navigation.h"
#include "frontend/cleanup.h"
#include <array>
#include <string>
#include <utility>

namespace hd2::game {

//Runtime type35,005E20A0/005E2130/005E2240/005E2250. It is not another
//Scene2 factory family. The containing004770B0 registry sets registered=true,
//binds frame userdata, assigns its runtime identity, then invokes both activation
//passes BEFORE the physics producer assigns the borrowed native object90.
//Deactivate before deleting and before clearing the shared navigation table.
//Pickup/interaction/velocity simulation/save receivers remain separate.
template<class Backend,class Activation>
class PlacedItemActor final : public SceneActor<Backend> {
public:
    using Frame=typename Backend::Frame;
    PlacedItemActor(Backend& backend,Activation& activation,MissionNavigation& navigation,std::string localized_name)
        : SceneActor<Backend>(backend,35),activation_(activation),navigation_(navigation) {
        this->base_.name(std::move(localized_name)); //Actual localization1BF6.
    }
    bool initialize(Frame frame) override {return this->base_.initialize_frame(frame);} //005E2250 tail00610200.
    void load(SerializedBytes&) override {} //005B8870 is genuine constant1/RET4; caller ignores result.
    void* physics_object() const {return physics_object_;}
    void physics_object(void* value) {physics_object_=value;} //004794D0 publishes90 after runtime construction/activation.
    const std::array<std::uint16_t,2>& navigation_points() const {return points_;}
    const std::array<float,3>& motion() const {return motion_;}
    template<class Services>bool apply_motion(const std::array<float,4>& motion,bool hide,Services& services){ //005E2510.
        if(!physics_object_)throw std::logic_error("Placed motion requires actual Physics90 borrower");
        if(!services.placed_physics_contact(physics_object_,motion))return false;
        if(hide)services.hide_placed_replacement(physics_object_); //00408750, independent of pickup branch.
        if((activation_.mode_flags()&7u)==7u){ //005E2480, no authority gate.
            (void)navigation_.connect_runtime_points(points_[0],points_[1]);
            for(auto point:points_)if(point!=0xffff)navigation_.point_byte(point,7); //005E2420, pair failure still marks.
        }
        services.publish_placed_motion(this->base_.id10(),motion);
        services.placed_script_use(this->base_.frame()); //Fresh EC/context after packet finish, optional native absence.
        for(unsigned n=0;n!=3;++n)motion_[n]=motion[n];
        motion_w_=motion[3];
        return true;
    }
    void activate(bool authority) { //005E2260.
        this->base_.activate_frame(authority,activation_);
        motion_.fill(0); //A0/A4/A8; the original unwritten fourth lane has no value contract.
        if(!authority)return;
        points_.fill(0xffff);
        if((activation_.mode_flags()&7)!=7)return;
        const auto world=this->backend_.actor_world_matrix(this->base_.frame());
        const std::array<float,3> position{{world[12],world[13],world[14]}};
        points_[0]=navigation_.nearest(position,5.f,-1.f);
        if(points_[0]!=0xffff) {
            const auto nearest=navigation_.position(points_[0]);
            std::array<float,3> mirror;
            for(unsigned i=0;i!=3;++i)mirror[i]=static_cast<float>((static_cast<long double>(position[i])-nearest[i])+position[i]);
            points_[1]=navigation_.nearest(mirror,5.f,-1.f);
        }
    }
    void deactivate(bool authority) { //005E23E0/005E2420.
        this->base_.deactivate_frame(authority,activation_);
        if(!authority)return;
        frontend::CleanupErrors errors;
        for(auto& point:points_)if(point!=0xffff) {
            const auto index=std::exchange(point,std::uint16_t{0xffff});
            errors.attempt([&]{navigation_.point_byte(index,0);});
        }
        errors.rethrow();
    }
private:
    Activation& activation_;
    MissionNavigation& navigation_;
    void* physics_object_=nullptr; //Borrowed00407390 object, NOT a007D instance or actor entry.
    std::array<std::uint16_t,2> points_{{0xffff,0xffff}};
    std::array<float,3> motion_{};
    float motion_w_=0; //Written by005E2510; original activation writes only XYZ, W is unclaimed until motion.
};

} //namespace hd2::game
#endif
