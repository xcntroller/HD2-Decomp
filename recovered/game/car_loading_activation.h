#ifndef HD2_GAME_CAR_LOADING_ACTIVATION_H
#define HD2_GAME_CAR_LOADING_ACTIVATION_H

#include "game/car_actor_features.h"
#include "game/car_destruction.h"
#include "game/car_physical_reset.h"
#include "game/car_glass_damage.h"
#include "game/car_body_collision_controller.h"
#include "game/car_cold_receivers.h"
#include "game/car_loading_seats.h"
#include "game/car_navigation_loading.h"
#include "game/car_settling.h"
#include "game/car_support_checks.h"
#include "game/car_water_loading.h"

namespace hd2::game {

//All values are borrowed from the actual containing world/process owners.
//The constructor requires every dependency; no successful service is supplied.
template<class Frames>struct CarLoadingResources {
    const CarTyres& tyres;const CarMaterialRegistry& materials;
    const std::vector<typename Frames::Frame>& sound_receivers;const std::vector<typename Frames::Frame>& light_receivers;
    std::vector<CarParticleState<typename Frames::Frame>>& particles;CarSeatCameraShared& seat_camera;
    std::function<int()>& random;
};

//004D16A0/004D25C0/004D2790/004D3110's actual offline, unoccupied loading
//lifetime. Services supplies common actor registries, SAME proximity/Players,
//settings264 and game kind; query/tree/bucket owners outlive this object.
//Required positive deformation, occupied/gun seats and timed destruction stay
//explicit errors. Source composition alone does not enable application loading.
template<class Frames,class Services,class Collisions,class Queries>class CarLoadingActivation final : public CarLoadingLifetime<Frames> {
public:
    using Actor=CarSceneActor<Frames>;using Frame=typename Frames::Frame;using Vector=typename Frames::Vector;
    using Initial=CarColdInitialization<Frames,Collisions>;using Random=std::function<int()>;
    using Receivers=CarColdReceivers<Frames,Collisions,Random>;
    using BodyCollision=CarBodyCollisionController<Frames,Collisions,Queries,typename Actor::Callbacks,Receivers>;
    using Model=decltype(std::declval<Frames&>().create_supply_helper(9));
    CarLoadingActivation(Frames& frames,Services& services,Collisions& collisions,Queries queries,
            DynamicCollisionExclusions<RigidBodyCollisionDescriptor<Frame>>& exclusions,MissionNavigation& navigation,
            CarLoadingResources<Frames> resources,Actor& actor)
        :frames_(frames),services_(services),collisions_(collisions),queries_(std::move(queries)),exclusions_(exclusions),resources_(resources),
         actor_(actor),navigation_(frames,navigation),seats_(frames,actor){
        if(!resources_.random)throw std::logic_error("Car loading requires the actual process rand stream");
        if(&resources_.tyres!=&actor.callback_services().car_tyres()||&resources_.particles!=&actor.callback_services().car_particle_pool())
            throw std::logic_error("Car loading and callbacks require the SAME tyre registry and particle pool");
    }
    ~CarLoadingActivation()override{
        frontend::CleanupErrors errors;
        if(actor_.flag24a1||ready_)retire_derived(errors);
        navigation_.close();if(proximity_!=-1)errors.attempt([&]{services_.remove_car_proximity(proximity_);proximity_=-1;});
        if(errors.failed())try{errors.rethrow();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
    }
    void activate(bool authority)override{
        require_mode();require_geometry();
        actor_.flag254a=false;actor_.flag2548=false;actor_.model254c.reset();damage_target_=damage_template_=-1;
        if(authority)actor_.source24b0=-1;
        clock2584_=actor_.callback_services().car_clock();camera_setting_=services_.car_camera_setting264();
        actor_.ancillary->reset_loading(frames_,actor_.geometry->auxiliary().seats().size());
        if(authority){if(proximity_!=-1)throw std::logic_error("Car authority activation repeated before proximity retirement");
            proximity_=services_.add_car_proximity(actor_,4);}
        actor_.base().activate_frame(authority,services_);
        initial_position_=actor_.base().world_position();initial_direction_=actor_.base().world_direction();actor_.fields2474[0]=38000;
        if(authority)return; //Actual offline first pass precedes004EA780.
        if(actor_.flag24a1){if(!ready_)throw std::logic_error("Retire partial Car activation before retry");return;}
        actor_.flag24a1=true;ready_=false;actor_.driver24ac=nullptr;
        body_collision_.reset();receivers_.reset();view_.reset();initial_.reset();
        initial_.emplace(frames_,*actor_.geometry,collisions_,resources_.sound_receivers,resources_.light_receivers,resources_.particles);
        initial_->initialize(&actor_,resources_.tyres,*actor_.callbacks);
        view_.emplace(initial_->body(),initial_->wheels,initial_->drive,initial_->solver,initial_->health,initial_->finalizer.dirty_flags);
        initial_->configure_offline_actor_loading();
        receivers_.emplace(frames_,actor_,*initial_,resources_.random);
        body_collision_.emplace(frames_,actor_.geometry->model(),actor_.geometry->collision(),initial_->collision,queries_,exclusions_,resources_.materials,
            actor_.geometry->physics().scalars(),actor_.geometry->physics().flags().at(0x228c),*actor_.callbacks,*receivers_,initial_->body_layers,initial_->body_flags);
        seats_.initialize(services_);
        auto placement=position(frames_.actor_world_matrix(actor_.geometry->model()));placement[1]+=1.f;
        frames_.car_place_in_scene(actor_.geometry->model(),placement);navigation_elapsed_=0;refresh_navigation();
        helper_.emplace(frames_.create_supply_helper(9));if(!helper_->get())throw std::runtime_error("Cannot create Car camera helper model");
        if(frames_.load_supply_model(helper_->get(),"C"+actor_.geometry->resource(),0x10)!=0)helper_.reset();
        settle_car_loading(frames_,initial_->body(),initial_->wheels,initial_->drive,initial_->solver,initial_->update_hold,20,0,0,
            [&](std::uint32_t elapsed){update_core(elapsed);},[&](std::uint32_t elapsed){(void)finalize_core(elapsed);});
        ready_=true;
    }
    void deactivate(bool authority)override{
        require_mode();frontend::CleanupErrors errors;
        if(actor_.flag24a1)errors.attempt([&]{actor_.ancillary->reset_loading(frames_,actor_.geometry->auxiliary().seats().size());});
        helper_.reset();
        retire_seats(errors);
        if(!authority||actor_.flag24a1)errors.attempt([&]{retire_runtime();});
        errors.attempt([&]{actor_.base().deactivate_frame(authority,services_);});navigation_.close();
        if(authority&&proximity_!=-1)errors.attempt([&]{services_.remove_car_proximity(proximity_);proximity_=-1;});
        errors.rethrow();
    }
    void update(bool authority,std::uint32_t elapsed)override{
        require_mode();if(!ready_||!initial_)throw std::logic_error("Car update requires completed loading");
        require_unoccupied();
        if(authority){ //004D2790: offline authority does not run00509AA0.
            if(actor_.clock_expired())throw std::runtime_error("Car periodic event6 requires its actual event receiver");
            if(damage_target_!=-1)throw std::runtime_error("Car targeted damage requires its actual pending damage owner");
            update_car_loading_water(frames_,actor_.geometry->model(),queries_,initial_->health,actor_.occupants,actor_.callback_services(),services_.mode_flags());
            actor_.ancillary->update_cold(elapsed);
            if(initial_->health.body!=0&&initial_->solver.speed<.10000000149011612f)
                (void)car_support_checks(initial_->health,initial_->fuel,initial_->solver.speed,initial_->wheels,initial_->body().pose().up,2);
            return; //All actual occupied control/exit/UI producers are null here.
        }
        elapsed=std::min(elapsed,std::uint32_t{83});
        initial_->finalizer.lights.steering_threshold=initial_->steering.maximum*.8999999761581421f;
        same_epoch_=frames_.vertex_scene_epoch()==frames_.vertex_frame_epoch(actor_.geometry->model());
        update_core(elapsed);(void)finalize_core(elapsed);
        actor_.base().world_direction(initial_->body().pose().forward);actor_.base().world_position(initial_->body().pose().position); //004D38F0.
        update_car_actor_features(frames_,actor_.geometry->model(),frames_.active_game_camera(),same_epoch_,actor_.flag254a,services_.mode_flags(),
            initial_->solver.substeps.flags,initial_->drive,initial_->audio);
        seats_.update(queries_,actor_.base().world_direction(),initial_->body().motion().velocity,initial_->solver.speed,
            services_.car_current_attachment(),resources_.seat_camera);
        if(initial_->solver.speed!=0||navigation_elapsed_>0){navigation_elapsed_+=elapsed;if(navigation_elapsed_>500){navigation_elapsed_=initial_->solver.speed!=0;refresh_navigation();}}
        if(!actor_.lists259c.empty())throw std::runtime_error("Car queued voice updates require their actual SOUND receiver");
        frames_.supply_model_tick(actor_.geometry->model(),elapsed); //Actual model+20,004D3110's final call.
    }
    CarActorColdState<Frames>& cold_state()override{if(!view_)throw std::logic_error("Car callback requires initialized source state");return *view_;}
    CarSteeringState& steering_state()override{if(!initial_)throw std::logic_error("Car steering requires initialized source state");return initial_->steering;}
    auto& seats(){return seats_;}
    std::uint32_t reaction_seat_flags(std::int32_t index)const override{ //004D9FC0.
        return index<0||static_cast<std::size_t>(index)>=seats_.seats().size()?0u:seats_.seats()[static_cast<std::size_t>(index)].flags;
    }
    void remove_reaction_seat(std::int32_t index)override{ //004D9A40, native signed bounds precede every mutation.
        if(index<0||static_cast<std::size_t>(index)>=actor_.occupants.size())return;
        require_mode();if(!initial_)throw std::logic_error("Car seat release requires actual initialized physics");
        remove_seat(static_cast<std::size_t>(index));
    }
    void dead_human_controls()override{ //005A0220, actual core controls, before reaction removes the occupied seat.
        if(!initial_||!ready_)throw std::logic_error("Dead Human Car controls require initialized runtime");
        auto& state=*initial_;const auto& fields=actor_.geometry->physics().scalars();
        request_car_drive_pedal(state.control_requests,state.drive,fields,state.solver.substeps,0.f,true);
        request_car_steering(state.steering,0.f);
        request_car_brake_pedal(state.control_requests,state.drive,fields,state.solver.substeps,1.f);
        request_car_emergency_brake(fields,state.solver.substeps,state.brake_ramp,state.brake_amount,state.brake_latch,true,true);
        request_car_clutch_pedal(state.control_requests,fields,state.controller().desired_clutch,0.f);
    }
    void release_presented_driver_pedal()override{ //0053BE10 invokes only004F2100(0,1).
        if(!initial_||!ready_)throw std::logic_error("Presented dead driver requires actual initialized Car controls");
        auto& state=*initial_;request_car_drive_pedal(state.control_requests,state.drive,actor_.geometry->physics().scalars(),state.solver.substeps,0.f,true);
    }
    void destruction_controls()override{ //004DBF00/004DC780, actual core flags/control/fuel/health.
        if(!initial_)throw std::logic_error("Car destruction requires its actual initialized control owner");
        clear_destroyed_car_lights(initial_->finalizer.lights.flags,[this]{
            if(actor_.callbacks->state6a4)actor_.callbacks->state6a4();
        });
        //004F2530(false,true) publishes these fields BEFORE0051B8F0.
        initial_->engine_timeout=5000;initial_->health.destroyed=false;initial_->drive.engine_speed=0;
        initial_->finalizer.engine_transition=false;initial_->drive.running=false;initial_->sounds.stop_all();
        initial_->set_fuel(0);initial_->health.body=0;
    }
    void puncture_sound(std::size_t index)override{
        if(!initial_)throw std::logic_error("Car puncture sound requires initialized actual sound/wheel owners");
        initial_->audio.puncture(initial_->drive.audible,initial_->finalizer.inside,initial_->wheels.at(index).vectors.at(0x30));
    }
    void physical_reset(float speed,bool restore)override{
        if(!initial_)throw std::logic_error("Car physical reset requires its actual initialized runtime");
        reset_car_physics(frames_,*actor_.geometry,*initial_,speed,restore,[&](bool running){
            //004F2530(running,true). Keep22BF/22A2 separate from2293.
            initial_->engine_timeout=5000;initial_->health.destroyed=false;
            if(!running){initial_->drive.engine_speed=0;initial_->finalizer.engine_transition=false;initial_->drive.running=false;initial_->sounds.stop_all();}
            else{initial_->finalizer.engine_transition=true;initial_->drive.running=true;initial_->engine_timeout=0;}
        },[&]{actor_.geometry->deformables().restore_cold(frames_,actor_.geometry->physics(),actor_.callbacks->empty6b4);});
    }
    void destroyed_meshes()override{present_destroyed_car_meshes(frames_,actor_,helper_?helper_->get():Frame{});}
    void glass_damage(std::size_t index,const CarContactVector& direction)override{
        if(!initial_)throw std::logic_error("Car glass damage requires actual initialized runtime");
        auto& geometry=*actor_.geometry;
        damage_car_glass(frames_,geometry.deformables().parts().at(index),geometry.physics().geometry().parts.at(index).scalars.get(0x20),direction,
            [&]{const auto& parameters=geometry.physics().applied_parameters();if(!parameters)throw std::logic_error("Car glass sound name requires actual applied parameters");
                return parameters->name(initial_->finalizer.inside?0x299:0x3c0);},
            [&](const auto&,const char*){actor_.callbacks->hide69c(index);}); //004D4A10 genuinely ignores both extra arguments.
    }
private:
    void require_mode()const{if((services_.mode_flags()&7u)!=7u)throw std::runtime_error("Car loading requires actual non7 network/input owners");}
    void require_geometry()const{if(!actor_.geometry||!actor_.ancillary||!actor_.callbacks)throw std::logic_error("Car activation requires its actual geometry/DM/callback owners");}
    void require_unoccupied()const{
        if(actor_.driver24ac)throw std::runtime_error("Car occupied input requires its actual driver owner");
        for(auto occupant:actor_.occupants)if(occupant)throw std::runtime_error("Car occupied update requires its actual Human attachment owner");
        for(const auto& control:actor_.fields24b4)if(control)throw std::runtime_error("Car mounted control requires its actual simulation owner");
    }
    static Vector position(const typename Frames::Matrix& matrix){return {{matrix[12],matrix[13],matrix[14],matrix[15]}};}
    void update_core(std::uint32_t elapsed){update_car_cold(frames_,actor_.geometry->model(),actor_.geometry->physics(),resources_.tyres,initial_->body(),initial_->wheels,
        initial_->drive,initial_->solver,initial_->steering,initial_->controller(),queries_,*receivers_,initial_->update_hold,elapsed,initial_->wheel_layers,initial_->wheel_flags,*body_collision_);}
    unsigned finalize_core(std::uint32_t elapsed){
        CarColdFinalizerContext<Frames,CarGeometry<Frames>,Collisions,Queries,BodyCollision,CarSkidModels<Frames>,Random> context{
            frames_,*actor_.geometry,initial_->body(),initial_->wheels,initial_->drive,initial_->solver,initial_->steering,initial_->controller(),
            initial_->collision,queries_,exclusions_,*body_collision_,initial_->audio,initial_->health,resources_.materials,resources_.particles,initial_->skids,resources_.random};
        return finalize_car_cold(context,initial_->finalizer,elapsed,services_.car_game_kind());
    }
    void refresh_navigation(){navigation_.load(actor_.geometry->model(),actor_.geometry->body(),services_.mode_flags(),initial_->solver.speed,initial_->body().motion().velocity);}
    void remove_seat(std::size_t index){ //004D9A40, SAME occupied/camera/mass/control owners.
        if(auto* occupant=actor_.occupants.at(index)){
            frames_.human_reaction_link(occupant->base().frame(),frames_.camera_root_sector());
            if(actor_.callback_services().car_human_controlled(occupant))seats_.view(index,false);
        }
        initial_->mass.seat(index,0,initial_->solver.mass_changed);actor_.occupants[index]=nullptr;
        seats_.restore_camera(index);
        if(index==0){actor_.driver24ac=nullptr;
            if(!initial_->finalizer.engine_transition){ //004F2530(0,!234F), distinct from actor254A.
                initial_->engine_timeout=5000;initial_->health.destroyed=false;
                initial_->drive.engine_speed=0;initial_->drive.running=false;initial_->finalizer.engine_transition=false;initial_->sounds.stop_all();
            }else{
                initial_->audio.stop_engine_transition(initial_->drive,initial_->finalizer.inside,initial_->finalizer.engine_transition,initial_->health.destroyed);
                if(!initial_->drive.running){initial_->control_requests.first=initial_->control_requests.second=0;initial_->health.destroyed=false;} //004F2530 tail.
            }
            const auto speed=actor_.geometry->physics().scalars().get(0x6f0);initial_->drive.governor=speed>0?speed+.5f:speed; //004F2630.
            initial_->brake_ramp=initial_->brake_amount=0;initial_->brake_latch=false; //004F26A0(0,0).
        }
        if(auto& control=actor_.fields24b4.at(index))control->loading->clear_occupant();
        seats_.seats().at(index).flags&=~0x400u;frames_.update_scene_object(actor_.geometry->model());
        bool empty=true;for(auto occupant:actor_.occupants)if(occupant){empty=false;break;}
        if(empty)initial_->drive.body_flags|=0x20000000u;else initial_->drive.body_flags&=~0x20000000u;
    }
    void retire_runtime(){
        actor_.flag24a1=false;ready_=false;
        if(initial_&&initial_->completed())initial_->retire(*actor_.callbacks);
    }
    void retire_seats(frontend::CleanupErrors& errors){
        if(initial_&&initial_->completed())for(std::size_t index=0;index<seats_.seats().size();++index)errors.attempt([&]{remove_seat(index);});
        errors.attempt([&]{seats_.release_cameras();});
    }
    void retire_derived(frontend::CleanupErrors& errors){
        helper_.reset();
        for(auto index=actor_.fields24b4.size();index;){ //004D25C0, controls precede seats/cameras.
            auto& control=actor_.fields24b4[--index];
            if(control)errors.attempt([&]{control->loading->retire();});
            control.reset();
        }
        retire_seats(errors);
        if(initial_&&initial_->ready())errors.attempt([&]{retire_runtime();});
    }
    Frames& frames_;Services& services_;Collisions& collisions_;Queries queries_;
    DynamicCollisionExclusions<RigidBodyCollisionDescriptor<Frame>>& exclusions_;CarLoadingResources<Frames> resources_;Actor& actor_;
    CarNavigationLoading<Frames> navigation_;CarLoadingSeats<Frames> seats_;std::optional<Model> helper_;
    //Native descriptors live inside the Car core for its entire actor lifetime.
    //Keep their addresses stable across reactivation: other wheels can retain
    //these primitive identities even while registration is temporarily absent.
    std::optional<Initial> initial_;std::optional<CarActorColdState<Frames>> view_;std::optional<Receivers> receivers_;std::optional<BodyCollision> body_collision_;
    Vector initial_position_{},initial_direction_{};std::uint32_t clock2584_=0,navigation_elapsed_=0;
    std::int32_t proximity_=-1,damage_target_=-1,damage_template_=-1;
    bool ready_=false,camera_setting_=false,same_epoch_=false;
};

} //namespace hd2::game
#endif
