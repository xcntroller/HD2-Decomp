#ifndef HD2_GAME_CAR_LOADING_LIFETIME_H
#define HD2_GAME_CAR_LOADING_LIFETIME_H

#include "game/car_cold_solver.h"
#include "game/car_health_reset.h"
#include "game/car_steering.h"
#include "game/human_frame_inputs.h"

namespace hd2::game {
template<class Frames>class SceneActor;

//Borrowed state of one actual initialized Car, shared with its installed
//callbacks. The contact snapshot is refreshed on each read; wheel mutation
//borrows the live vector directly. No independent callback simulation exists.
template<class Frames>struct CarActorColdState {
    CarBodyIntegration<Frames>& body;std::vector<CarWheelRuntime<Frames>>& wheels;
    CarDrivetrainState& drive;CarColdSolverState<Frames>& solver;CarHealthState& health;std::uint32_t& dirty;
    const CarContactState& contact_state(){contact_=body.contact_state();return contact_;}
private:
    CarContactState contact_{};
public:
    CarActorColdState(CarBodyIntegration<Frames>& b,std::vector<CarWheelRuntime<Frames>>& w,
            CarDrivetrainState& d,CarColdSolverState<Frames>& s,CarHealthState& h,std::uint32_t& changed)
        :body(b),wheels(w),drive(d),solver(s),health(h),dirty(changed){}
};
template<class Frames>class CarLoadingLifetime {
public:
    virtual ~CarLoadingLifetime()=default;
    virtual void activate(bool authority)=0;
    virtual void deactivate(bool authority)=0;
    virtual void update(bool authority,std::uint32_t milliseconds)=0;
    virtual CarActorColdState<Frames>& cold_state()=0;
    virtual CarSteeringState& steering_state()=0;
    virtual std::uint32_t& collision_clock()=0;
    virtual void set_fuel(float amount)=0;
    virtual void deform_hit(const CarContactVector& point,const CarContactVector& direction,float radius,float impulse,unsigned flags=0,const CarContactVector* incoming=nullptr)=0;
    virtual void remove_reaction_seat(std::int32_t index)=0;
    virtual void assign_seat(std::int32_t index,SceneActor<Frames>* human)=0;
    virtual bool exit_seat(std::int32_t index)=0;
    virtual bool seat_exit_pose(std::size_t index,CarContactVector& position,CarContactVector& direction)=0;
    virtual void driver_input(float pedal,float steering)=0;
    virtual void driver_frame_inputs(const HumanFrameInputBuffer&,const HumanFrameInputBuffer&)=0;
    virtual bool seat_claimed(std::int32_t)const=0;
    virtual void claim_seat(std::int32_t,bool)=0;
    virtual std::uint32_t reaction_seat_flags(std::int32_t index)const=0;
    virtual void dead_human_controls()=0;
    virtual void release_presented_driver_pedal()=0;
    virtual void puncture_sound(std::size_t)=0;
    virtual void destruction_controls()=0;
    virtual void physical_reset(float speed,bool restore)=0;
    virtual void reset_event()=0; //004DB2A0→Car virtual44/004DD0D0(true).
    virtual void destroyed_meshes()=0;
    virtual void glass_damage(std::size_t index,const CarContactVector& direction)=0;
};

} //namespace hd2::game
#endif
