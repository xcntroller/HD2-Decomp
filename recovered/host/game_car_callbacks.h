#ifndef HD2_HOST_GAME_CAR_CALLBACKS_H
#define HD2_HOST_GAME_CAR_CALLBACKS_H

#include "host/game_car_callback_frames.h"
#include "host/startup_session.h"
#include "game/car_external_impulses.h"
#include "game/human_actor_contact.h"
#include "game/car_cold_deformation.h"
#include "game/car_particle_pool.h"
#include "game/car_definition_tables.h"
#include "game/car_mesh_fragments.h"
#include "game/physics_contact_link.h"
#include "game/version_contact_link.h"
#include "game/explosion_contact_link.h"

namespace hd2::host {

//Shared Car callback owner. Frame/EC/Scene2, physics, process flags/clock/CRT,
//definitions, damage and emitter borrowers come from the containing world.
//Wider deformation, occupied control and full frame/input dependencies remain
//checked at their actual consumers.
//Construction supplies no readiness and performs no pool/resource preparation.
class GameCarCallbacks final : public GameCarCallbackFrames {
public:
    using Car=game::CarSceneActor<GameActorServices>;
    GameCarCallbacks(GameActorServices& actors,StartupSession& startup,game::CarDefinitionTables& definitions,const game::CarTyres& tyres)
        :GameCarCallbackFrames(actors,startup.application_flags_owner(),startup.local_player_identity(),
            tyres,[&startup]{return startup.control_milliseconds();}),startup_(startup),definitions_(definitions),particles_(actors,*this),physics_contact_(actors),versions_contact_(actors),runtime_contact_(actors),explosion_contact_(actors){}
    template<class Receiver>void bind_damage_packets(Receiver& receiver){
        if(&receiver.frames()!=&frames_||damage_)throw std::logic_error("Car packets require SAME unbound containing damage receiver");
        auto next=std::make_unique<DamageReceivers>();next->owner=&receiver;
        next->packet=[&receiver](Actor actor,game::SessionPacketStream& packet,bool authority){
            if(authority)receiver.receive_car_damage_request(*actor,packet);else receiver.receive_car_damage(*actor,packet);};
        next->wheel=[&receiver](Actor actor,const game::CarWheelDamageEvent& event,bool request){receiver.car_contact_event(actor,event,request);};
        next->human=[&receiver](Actor actor,const game::CarHumanDamageEvent& event){receiver.car_contact_request(actor,event);};
        next->destroy=[&receiver](Actor actor){receiver.car_damage_destroy(actor,false);};
        next->hit=[&receiver](Car& car,Actor source,const Vector& point){return receiver.car_collision_hit(car,source,point);};
        damage_=std::move(next); //All borrowers publish together; preparation failures leave no live endpoint.
    }
    template<class Receiver>void unbind_damage_packets(Receiver& receiver)noexcept{if(damage_&&damage_->owner==&receiver)damage_.reset();}
    void receive_damage_packet(Actor actor,game::SessionPacketStream& packet)override{
        required_damage().packet(actor,packet,false);
    }
    void receive_damage_request(Actor actor,game::SessionPacketStream& packet)override{required_damage().packet(actor,packet,true);}
    void car_timed_destroy(Actor actor)override{required_damage().destroy(actor);}
    bool initialize(const std::vector<game::CarParticleDefinition>& definitions){
        if(&definitions!=&car_particle_definitions())throw std::logic_error("Car particles require SAME actual definition owner");
        return initialize_particles();
    }
    bool initialize_particles(){return particles_.initialize(definitions_.particles());} //004D5A50, after real table preparation.
    const std::vector<game::CarParticleDefinition>& car_particle_definitions()const override{return definitions_.particles();}
    std::vector<game::CarParticleState<Frame>>& car_particle_pool()override{return particles_.states();}
    void clear_particles(){particles_.clear();}
    void car_retire_particle(std::int32_t identity)override{startup_.mission_effects_process().stop(identity);}
    std::size_t car_active_particles()const override{return startup_.mission_effects_process().active_particles();}
    void update_particles(std::uint32_t elapsed){particles_.update(elapsed);} //004ED130, same actual emitter pool.
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
        if((car_mode_flags()&7u)!=7u&&required_damage().hit(car,source,point))return;
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
            auto random=[this]{return static_cast<int>(startup_.random_word());};
            game::CarColdDeformation<GameActorServices> deformation(frames_,*car.geometry,state.body,state.wheels,
                state.solver.substeps.flags,state.dirty,random,car.callbacks.value().empty6b0);
            game::apply_car_contact_impulse(frames_,state,1.2000000476837158f,point,incoming,normal,source!=nullptr,
                deformation,[&](float magnitude){game::damage_car_health(physics,state.drive,state.health,state.dirty,magnitude);},
                [&](const Vector& value,const Vector& at,bool collision){if(car.callbacks->passenger690)car.callbacks->passenger690(value,at,collision,nullptr);},[]{});
        }
    }
    void car_actor_hit5c(Actor actor,Actor source,const Vector& point,const Vector&,const Vector&,float,Frame,Actor)override{
        if(actor_type(actor)==3){
            (void)game::dispatch_human_actor_contact(human_actor(actor),source,point);return;
        }
        //Known Scene2 nonHuman5C entries and runtime Mine00812F08 bind
        //00611070, genuine AL0/RET28. Unknown runtime families are not assumed.
        switch(actor_type(actor)){
        case 2:case 4:case 6:case 8:case 9:case 10:case 12:case 13:case 21:case 22:case 25:case 28:case 29:case 30:case 33:return;
        default:throw std::runtime_error("Car contact requires its actual derived actor5C receiver");
        }
    }
    void human_car_contact(Actor actor,Actor source,const Vector& point,const Vector& impulse,const Vector& normal,float mass,Frame contact_frame)override{
        car_actor_hit5c(actor,source,point,impulse,normal,mass,contact_frame,nullptr); //0059C2D0 preserves sixth frame argument and appends null seventh.
    }
    void human_ragdoll_car_impulse(Actor actor,const Vector& impulse,const Vector& point)override{
        //007CBF90/007CC740 already checked004DE800, then call004F2790 directly.
        auto& state=car_state(actor);
        game::apply_car_direct_impulse(frames_,state,impulse,point,0,Vector{{1,1,1,0}},[]{}); //Actual cold6A4 is null.
    }
    void car_human_impact(Actor actor,int kind,float amount,const Vector& point,const Vector& impulse,Actor source,int hit_kind)override{
        auto& human=human_actor(actor);auto* cause=source?&human_actor(source):nullptr;
        if(!human.loading_lifetime)throw std::logic_error("Car Human damage requires the actual containing Human lifetime");
        human.loading_lifetime->damage(static_cast<std::uint32_t>(kind),amount,point,impulse,cause,static_cast<std::uint32_t>(hit_kind));
    }
    void submit_car_human_damage(Actor actor,const game::CarHumanDamageEvent& event)override{
        required_damage().human(actor,event);
    }
    void apply_car_wheel_damage(Actor actor,const game::CarWheelDamageEvent& event)override{
        required_damage().wheel(actor,event,false);
    }
    void submit_car_wheel_damage(Actor actor,const game::CarWheelDamageEvent& event)override{
        required_damage().wheel(actor,event,true);
    }
    template<class Versions>void bind_versions(Versions& versions){versions_contact_.bind(versions);}
    template<class Versions>void unbind_versions(Versions& versions)noexcept{versions_contact_.unbind(versions);}
    void car_timed_contact(std::uintptr_t handle,unsigned damage,unsigned effects)override{
        std::int32_t amount;std::memcpy(&amount,&damage,4);(void)versions_contact_.contact(handle,amount,effects);
    }
    template<class Runtime>void bind_runtime(Runtime& runtime){runtime_contact_.bind(runtime);}
    template<class Runtime>void unbind_runtime(Runtime& runtime)noexcept{runtime_contact_.unbind(runtime);}
    template<class Blast>void bind_explosion(Blast& blast){explosion_contact_.bind(blast);}
    template<class Blast>void unbind_explosion(Blast& blast)noexcept{explosion_contact_.unbind(blast);}
    void car_item_contact(Actor actor,Actor touching,unsigned slot)override{runtime_contact_.contact(actor,touching,slot);}
    void car_collision_blast(Actor source,const Vector& position,float radius,float power,unsigned sound)override{
        std::int32_t entry;std::memcpy(&entry,&sound,4);explosion_contact_.explode(source,position,radius,power,entry);
    }
    void car_surface_effect(const Vector&,const Vector&,unsigned,unsigned,unsigned)override{
        throw std::runtime_error("Car surface impact requires actual GEffects particle producer");
    }
    void emit_car_contact_fragments(Frame frame,float area)override{ //00485BA0, actual GEffects+B4.
        game::emit_car_mesh_fragments(frames_,physics_contact_,frame,area);
    }
    Actor clone_car_contact_item(Frame frame,const Vector& position,const Vector& direction)override{
        return runtime_contact_.clone(frame,position,direction);
    }
    void launch_car_contact_item(Actor actor,const game::CarDetachedPartRequest& request,unsigned mode)override{
        runtime_contact_.launch(actor,request,mode);
    }
    void launch_car_contact_item(Actor actor,const game::CarDetachedWheelRequest& request,unsigned mode)override{
        runtime_contact_.launch(actor,request,mode);
    }
private:
    struct DamageReceivers{
        const void* owner=nullptr;
        std::function<void(Actor,game::SessionPacketStream&,bool)> packet;
        std::function<void(Actor,const game::CarWheelDamageEvent&,bool)> wheel;
        std::function<void(Actor,const game::CarHumanDamageEvent&)> human;
        std::function<void(Actor)> destroy;
        std::function<bool(Car&,Actor,const Vector&)> hit;
    };
    DamageReceivers& required_damage(){if(!damage_)throw std::runtime_error("Car damage requires its actual containing damage owner");return *damage_;}
    std::unique_ptr<DamageReceivers> damage_;
    StartupSession& startup_;game::CarDefinitionTables& definitions_;game::CarParticlePool<GameActorServices,GameCarCallbacks> particles_;
    game::PhysicsContactLink<GameActorServices> physics_contact_;
    game::VersionContactLink<GameActorServices> versions_contact_;
    game::RuntimeActorContactLink<GameActorServices,game::SceneActor<GameActorServices>> runtime_contact_;
    game::ExplosionContactLink<GameActorServices,game::SceneActor<GameActorServices>> explosion_contact_;
};

inline auto make_game_car_callback_factory(StartupSession& startup,game::CarDefinitionTables& definitions,const game::CarTyres& tyres){
    return std::function<std::unique_ptr<game::CarCallbackServices<GameActorServices>>(GameActorServices&)>(
        [&startup,&definitions,&tyres](GameActorServices& frames){return std::make_unique<GameCarCallbacks>(frames,startup,definitions,tyres);});
}

} //namespace hd2::host
#endif
