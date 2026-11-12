#ifndef HD2_GAME_CAR_COLD_INITIALIZATION_H
#define HD2_GAME_CAR_COLD_INITIALIZATION_H
#include "game/car_control_requests.h"

#include "game/car_cold_finalizer.h"
#include "game/car_geometry.h"
#include "game/car_light_receivers.h"
#include "game/car_mass_loading.h"
#include "game/car_skid_models.h"
#include "frontend/cleanup.h"

namespace hd2::game {

//004E9DB0's additional mutable runtime words, separate from coefficients.
//Wider driving/plane-cache consumers remain required at their existing gates.
struct CarPhysicalResetFields {
    float field780=0,field78c=0,fieldf68=0;
    std::array<std::uint32_t,4> fields4f0{};
    bool field22c1=false,signal2294=false,signal2295=false;
};

//004EA780 and004EC5C0's cold, unoccupied source owner. The containing actor
//must supply its SAME geometry, callbacks, shared buckets, process receiver
//lists and particle pool. No query/solver/activation backend is manufactured.
//This initializer is not004D16A0 or a world-loading readiness transition.
template<class Frames,class Collisions,class Geometry=CarGeometry<Frames>>class CarColdInitialization {
public:
    using Frame=typename Frames::Frame;using Vector=typename Frames::Vector;
    using Body=CarBodyIntegration<Frames>;using Wheels=std::vector<CarWheelRuntime<Frames>>;
    CarColdInitialization(Frames& frames,Geometry& geometry,Collisions& collisions,
            const std::vector<Frame>& sound_receivers,const std::vector<Frame>& light_receivers,
            std::vector<CarParticleState<Frame>>& particles)
        :frames_(frames),geometry_(geometry),sound_receivers_(sound_receivers),light_receivers_(light_receivers),particles_(particles),
         mass(geometry.physics(),geometry.auxiliary()),collision(frames,collisions),
         skids(frames,geometry.wheels().wheels().size()),sounds(frames,geometry.model(),geometry.auxiliary().head()),
         audio(frames,sounds),solver(geometry.physics()),finalizer(geometry){}
    ~CarColdInitialization(){discard();}
    CarColdInitialization(const CarColdInitialization&)=delete;CarColdInitialization& operator=(const CarColdInitialization&)=delete;
    template<class Callbacks>void initialize(void* actor,const CarTyres& tyres,Callbacks& callbacks){
        if(started_)throw std::logic_error("Car cold initializer repeated without retirement");
        started_=true;
        try{
            const auto model=geometry_.model();const auto& physics=geometry_.physics();
            if(!physics.applied_parameters())throw std::logic_error("Car initializer requires its actual applied parameters");
            frames_.update_scene_object(model);body_.emplace(frames_,model,physics.scalars());
            frames_.car_place_in_scene(model,body_->pose().centre);
            collision.initialize(model,geometry_.auxiliary().collision_bounds(),geometry_.auxiliary().special(),actor);
            controller_.emplace(*body_);
            const auto& authored=geometry_.wheels().wheels();const auto& coefficients=physics.geometry().wheels;
            if(authored.size()!=coefficients.size())throw std::logic_error("Car initializer lost its SAME wheel geometry");
            wheels.resize(authored.size());
            for(auto n=wheels.size();n;){const auto index=--n;wheels[index].initialize(frames_,model,authored[index],coefficients[index],tyres,body_->pose().up,static_cast<unsigned>(index));}
            skids.load(physics.geometry().flags);
            mass.initialize(solver.mass_changed);
            sounds.load_slots(physics);sounds.register_receivers(sound_receivers_);
            reset_car_health(frames_,physics,drive,health,wheels,geometry_.lighting(),paired_flags);
            geometry_.deformables().reset_loading_health(physics,callbacks.empty6b4);
            geometry_.deformables().load_glass_textures(frames_,*physics.applied_parameters());
            //004F6070 is genuine RET4. No substituted receiver is required.
            light_borrowers_=true;register_car_light_receivers(frames_,geometry_.lighting(),light_receivers_);
            geometry_.axes().apply(frames_,authored,solver.substeps.flags,finalizer.previous_features); //00500FD0.
            //Fresh slots are initialized -1 above. Only the three existing
            //impact slots use004ECFC0 here; a new lifetime has none to retire.
            ready_=completed_=true;
        }catch(...){discard();throw;}
    }
    template<class Callbacks>void retire(Callbacks& callbacks){ //004EC5C0, before model/geometry retirement.
        if(!completed_)throw std::logic_error("Car cold retirement requires a completed initializer");
        //No occupant/seat attachment or positive deformation producer may be
        //installed in this cold owner. Those require their actual restore path.
        frontend::CleanupErrors errors;
        if(ready_){errors.attempt([&]{collision.set_enabled(false,false);});collision.close();}
        errors.attempt([&]{sounds.clear(&sound_receivers_);});
        const auto model=geometry_.model();
        for(auto index:geometry_.wheels().linked_order()){
            auto& wheel=wheels.at(index);
            errors.attempt([&]{
                if((wheel.flags&1u)&&!(wheel.flags&0x400u)&&frames_.object_parent(wheel.parent)==model){
                    frames_.car_steering_rotation(wheel.parent,frames_.car_axis_quaternion({{0,1,0,0}},0));
                    (void)frames_.car_local_matrix(wheel.parent);frames_.update_scene_object(wheel.parent);
                }
                frames_.object_visible(wheel.frame,true);
            });
        }
        for(std::size_t n=0;n<wheels.size();++n){auto& wheel=wheels[n];
            skids.retire(n,wheel.skid_cursor);
            errors.attempt([&]{frames_.car_suspension_position(wheel.parent,wheel.vectors.at(0x20));frames_.update_scene_object(wheel.parent);});
            skids.clear_wheel(n);
        }
        skids.clear();
        errors.attempt([&]{geometry_.deformables().restore_cold(frames_,geometry_.physics(),callbacks.empty6b4);});
        finalizer.lights.flags=0;
        errors.attempt([&]{update_car_loading_lights(frames_,geometry_.lighting(),finalizer.lights,.10000000149011612f,solver.substeps.flags,finalizer.previous_features);});
        errors.attempt([&]{sounds.unregister_receivers(sound_receivers_);});
        errors.attempt([&]{unregister_car_light_receivers(frames_,geometry_.lighting(),light_receivers_);light_borrowers_=false;});
        for(auto n=finalizer.particles.exhausts.size();n;){auto& slot=finalizer.particles.exhausts[--n];retire_car_particle(particles_,slot);slot=-1;}
        for(auto& wheel:wheels){retire_car_particle(particles_,wheel.effect);wheel.effect=-1;}
        retire_car_particle(particles_,finalizer.particles.damage);finalizer.particles.damage=-1;
        for(auto n=finalizer.particles.impacts.size();n;){auto& slot=finalizer.particles.impacts[--n];retire_car_particle(particles_,slot);slot=-1;}
        ready_=false;errors.rethrow();
    }
    Body& body(){return body_.value();}CarSettlingControllerState<Frames>& controller(){return controller_.value();}
    bool ready()const{return ready_;}
    bool completed()const{return completed_;}
    bool set_fuel(float amount){ //004F2670, upper bound only; native always returns1.
        const auto capacity=geometry_.physics().scalars().get(0x2220);if(capacity<amount)amount=capacity;fuel=amount;return true;
    }
    void configure_offline_actor_loading(){ //004D1AC2..004D1BE9, after004EA780;6A4 is cold null.
        if(!ready_)throw std::logic_error("Car actor loading settings require its initialized runtime");
        collision.use_special(true);set_fuel(1000.f);
        auto& flags=finalizer.lights.flags;
        flags|=0x8808u;flags&=~0x80u;flags&=~0x10u;flags&=~0x20u;
        geometry_.lighting().scale_projectors(frames_,0xffffffffu,{{12,12,40,0}});
        flags&=0xf00fffffu;
        wheel_layers=0x98302u;body_layers=0xd8002u;
        frames_.update_scene_object(geometry_.model());
    }
private:
    void discard()noexcept{
        //A failed prefix cannot publish a complete initializer. Retract every
        //actually acquired borrower even if its next engine operation failed.
        ready_=false;collision.close();
        try{sounds.clear(&sound_receivers_);}catch(...){}
        if(light_borrowers_)try{unregister_car_light_receivers(frames_,geometry_.lighting(),light_receivers_);light_borrowers_=false;}catch(...){}
        skids.clear();
    }
    Frames& frames_;Geometry& geometry_;const std::vector<Frame>& sound_receivers_;const std::vector<Frame>& light_receivers_;
    std::vector<CarParticleState<Frame>>& particles_;std::optional<Body> body_;std::optional<CarSettlingControllerState<Frames>> controller_;
    bool started_=false,ready_=false,completed_=false,light_borrowers_=false;
public:
    CarMassLoading<Frames> mass;CarCollisionLoading<Frames,Collisions> collision;CarSkidModels<Frames> skids;
    CarSoundLoading<Frames> sounds;CarSoundUpdates<Frames> audio;
    Wheels wheels;CarDrivetrainState drive;CarHealthState health;CarSteeringState steering;
    CarControlRequests control_requests;
    CarPhysicalResetFields reset_fields;
    CarColdSolverState<Frames> solver;CarColdFinalizerState<Frames> finalizer;
    std::vector<std::uint16_t> paired_flags;std::int32_t update_hold=0;float fuel=0;
    float engine_timeout=0,brake_ramp=0,brake_amount=0;bool brake_latch=false; //F48/710/740/22C7.
    std::uint32_t wheel_layers=0xf02u,wheel_flags=0,body_layers=2u,body_flags=0; //004E2900;004D16A0 replaces the layers.
};

} //namespace hd2::game
#endif
