#ifndef HD2_HOST_GAME_CAR_CALLBACKS_H
#define HD2_HOST_GAME_CAR_CALLBACKS_H

#include "host/game_car_callback_frames.h"
#include "host/startup_session.h"
#include "game/car_external_impulses.h"
#include "game/car_cold_deformation.h"
#include "game/car_particle_pool.h"
#include "game/car_definition_tables.h"
#include "game/physics_contact_link.h"

namespace hd2::host {

//Actual unoccupied offline loading callback owner. Frame/EC/Scene2, physics,
//process flags/clock/CRT, definitions and emitter frames are shared with the
//containing world. Required positive Human damage, tipping, Versions changes,
//blast/surface effects and detached Items remain explicit unrecovered branches.
//Construction supplies no readiness and performs no pool/resource preparation.
class GameCarCallbacks final : public GameCarCallbackFrames {
public:
    using Car=game::CarSceneActor<GameActorServices>;
    GameCarCallbacks(GameActorServices& actors,StartupSession& startup,game::CarDefinitionTables& definitions,const game::CarTyres& tyres)
        :GameCarCallbackFrames(actors,startup.application_flags_owner(),startup.local_player_identity(),
            tyres,[&startup]{return startup.control_milliseconds();}),startup_(startup),definitions_(definitions),particles_(actors,*this),physics_contact_(actors){}
    bool initialize_particles(){return particles_.initialize(definitions_.particles());} //004D5A50, after real table preparation.
    const std::vector<game::CarParticleDefinition>& car_particle_definitions()const override{return definitions_.particles();}
    std::vector<game::CarParticleState<Frame>>& car_particle_pool()override{return particles_.states();}
    void clear_particles(){particles_.clear();}
    std::uint32_t car_random_word()override{return startup_.random_word();}
    template<class Physics>void bind_physics(Physics& physics){physics_contact_.bind(physics);}
    template<class Physics>void unbind_physics(Physics& physics)noexcept{physics_contact_.unbind(physics);}
    void car_physics_contact(std::uintptr_t handle,unsigned kind,const Vector& velocity,const Vector& point)override{
        physics_contact_.contact(handle,kind,velocity,point); //00410630→00410530, not module4 Script.
    }
    void car_actor_hit58(Actor actor,Actor source,const Vector& point,const Vector& impulse,
            const Vector& normal,float mass,Frame contact_frame)override{ //004D3EF0 /0059C2D0.
        if(actor_type(actor)!=4){car_actor_hit5c(actor,source,point,impulse,normal,mass,contact_frame,nullptr);return;}
        auto& car=car_actor(actor);
        if((car_mode_flags()&7u)!=7u)
            throw std::runtime_error("Car hit58 requires actual004DE800/004D3D10 nonoffline ownership and impulse publication");
        //004DE800 is unconditionally true offline.00611080 is genuine RET4.
        auto& state=car_state(actor);
        if(actor_type(source)==3){
            auto force=impulse;for(unsigned axis=0;axis!=3;++axis)force[axis]*=500.f;
            auto origin=state.body.pose().centre;
            origin[1]=static_cast<float>((static_cast<long double>(state.body.initial_position()[1])-origin[1])+origin[1]);
            game::accumulate_car_contact_force(frames_,state,force,origin,1,Vector{{1,1,1,0}},[]{}); //6A4 is cold null.
        }else if(actor_type(source)==4){
            auto& physics=car.geometry.value().physics();const float original_mass=physics.scalars().get(0x918);
            const bool reduced=car_field5e0(actor)&&car_field5e0(source)&&car_field6a0(actor)==0;
            struct Restore {game::CarPhysics& physics;float mass;bool changed;~Restore(){if(changed)physics.contact_mass(mass);}} restore{physics,original_mass,reduced};
            auto incoming=impulse;
            if(reduced){physics.contact_mass(original_mass*.5f);for(unsigned axis=0;axis!=3;++axis)incoming[axis]*=1.399999976158142f;}
            game::CarColdDeformation<GameActorServices> deformation(frames_,*car.geometry,state.body,state.wheels,state.solver.substeps.flags);
            game::apply_car_contact_impulse(frames_,state,1.2000000476837158f,point,incoming,normal,source!=nullptr,
                deformation,[&](float magnitude){game::damage_car_health(physics,state.drive,state.health,state.dirty,magnitude);},
                [&](const Vector& value,const Vector& at,bool collision){if(car.callbacks->passenger690)car.callbacks->passenger690(value,at,collision,nullptr);},[]{});
        }
    }
    void car_actor_hit5c(Actor actor,Actor,const Vector&,const Vector&,const Vector&,float,Frame,Actor)override{
        if(actor_type(actor)==3){
            if(!human_actor(actor).loading_state)return; //005A07C0's genuine absent-B0 gate.
            throw std::runtime_error("Human collision requires actual005A07C0 B0+C8 and damage receivers");
        }
        //The fourteen actual Scene2 constructors' nonHuman5C entries all bind
        //00611070, genuine AL0/RET28. Unknown runtime families are not assumed.
        switch(actor_type(actor)){
        case 4:case 6:case 8:case 9:case 10:case 13:case 21:case 22:case 25:case 28:case 29:case 30:case 33:return;
        default:throw std::runtime_error("Car contact requires its actual derived actor5C receiver");
        }
    }
    void human_car_contact(Actor actor,Actor source,const Vector& point,const Vector& impulse,const Vector& normal,float mass,Frame contact_frame)override{
        car_actor_hit5c(actor,source,point,impulse,normal,mass,contact_frame,nullptr); //0059C2D0 preserves sixth frame argument and appends null seventh.
    }
    void car_human_impact(Actor,int,float,const Vector&,const Vector&,Actor,int)override{
        throw std::runtime_error("Car Human damage requires actual005A1C90 victim/state/packet receivers");
    }
    void submit_car_human_damage(Actor,const game::CarHumanDamageEvent&)override{
        throw std::runtime_error("Car Human damage publication requires actual004DB2E0 event serialization");
    }
    void apply_car_wheel_damage(Actor,const game::CarWheelDamageEvent&)override{
        throw std::runtime_error("Car wheel event requires actual004DB2B0 broadcast serialization");
    }
    void submit_car_wheel_damage(Actor,const game::CarWheelDamageEvent&)override{
        throw std::runtime_error("Car wheel request requires actual004DB2E0 event serialization");
    }
    void car_timed_contact(std::uintptr_t,unsigned,unsigned)override{
        throw std::runtime_error("Car Versions contact requires actual0043C6A0/0043B000 part transitions");
    }
    void car_item_contact(Actor,Actor,unsigned)override{
        throw std::runtime_error("Car trigger contact requires actual005FEFB0 runtime trigger owner");
    }
    void car_collision_blast(Actor,const Vector&,float,float,unsigned)override{
        throw std::runtime_error("Car blast requires actual00451990 explosion/damage owner");
    }
    void car_surface_effect(const Vector&,const Vector&,unsigned,unsigned,unsigned)override{
        throw std::runtime_error("Car surface impact requires actual GEffects particle producer");
    }
    void emit_car_contact_fragments(Frame frame,float)override{ //00485BA0, GEffects+B4, mesh-fragment generation.
        if(!(frames_.actor_visual_flags(frame)&0x800u))return; //Native genuine nonmesh gate.
        throw std::runtime_error("Car contact fragments require actual00485BA0/00484F60 mesh/effect owners");
    }
    Actor clone_car_contact_item(Frame,const Vector&,const Vector&)override{
        throw std::runtime_error("Car detachment requires actual00479450 runtime Item12 constructor");
    }
    void launch_car_contact_item(Actor,const game::CarDetachedPartRequest&,unsigned)override{
        throw std::runtime_error("Car detached part requires actual005FDC50 Item12 movement owner");
    }
    void launch_car_contact_item(Actor,const game::CarDetachedWheelRequest&,unsigned)override{
        throw std::runtime_error("Car detached wheel requires actual005FDC00 Item12 movement owner");
    }
private:
    StartupSession& startup_;game::CarDefinitionTables& definitions_;game::CarParticlePool<GameActorServices,GameCarCallbacks> particles_;
    game::PhysicsContactLink<GameActorServices> physics_contact_;
};

inline auto make_game_car_callback_factory(StartupSession& startup,game::CarDefinitionTables& definitions,const game::CarTyres& tyres){
    return std::function<std::unique_ptr<game::CarCallbackServices<GameActorServices>>(GameActorServices&)>(
        [&startup,&definitions,&tyres](GameActorServices& frames){return std::make_unique<GameCarCallbacks>(frames,startup,definitions,tyres);});
}

} //namespace hd2::host
#endif
