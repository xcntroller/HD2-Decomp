#ifndef HD2_GAME_CAR_PHYSICAL_RESET_H
#define HD2_GAME_CAR_PHYSICAL_RESET_H

#include "game/car_cold_initialization.h"

namespace hd2::game {

//004E95B0(2), reached by004DD0D0. This suspends the SAME live physical graph;
//it does not retire/reconstruct its owners. Bit1's permanent skid detachment
//belongs to other callers and is not implied by this reset entry.
template<class Frames,class Geometry,class State,class Pool,class Contacts>
void suspend_car_for_reset(Frames& frames,Geometry& geometry,State& state,Pool& pool,Contacts& contacts){
    state.skids.hide_first();
    frames.car_transform_in_place(state.body().motion().angular_velocity,state.body().pose().inverse_rotation);
    frames.car_transform_in_place(state.body().motion().velocity,state.body().pose().inverse_rotation);
    for(auto n=geometry.lighting().lights().size();n;){const auto& light=geometry.lighting().lights()[--n];
        if(light.projector==CarLighting<Frames>::none||(light.flags10&0x80000000u))continue;
        const auto& projector=geometry.lighting().projectors().at(light.projector);
        if(projector.visual){frames.car_reset_link(projector.visual.get(),typename Frames::Frame{});
            if(projector.light)frames.car_reset_link(projector.light.get(),typename Frames::Frame{});}
    }
    for(auto n=geometry.lighting().projectors().size();n;){const auto index=--n;const auto& projector=geometry.lighting().projectors()[index];
        bool paired=false;for(const auto& light:geometry.lighting().lights())if(light.shared&&light.projector==index){paired=true;break;}
        if(!paired)continue;
        frames.car_reset_link(projector.visual.get(),typename Frames::Frame{});
        if(projector.light)frames.car_reset_link(projector.light.get(),typename Frames::Frame{});
    }
    for(auto& wheel:state.wheels)if(wheel.effect>=0){retire_car_particle(pool,wheel.effect);wheel.effect=-1;}
    for(auto n=state.finalizer.particles.exhausts.size();n;){auto& slot=state.finalizer.particles.exhausts[--n];if(slot>=0){retire_car_particle(pool,slot);slot=-1;}}
    if(state.finalizer.particles.damage>=0){retire_car_particle(pool,state.finalizer.particles.damage);state.finalizer.particles.damage=-1;}
    state.collision.set_enabled(false,false);
    std::vector<CarBodyContactRecord>().swap(contacts.records); //Native798/79C/7A0; preserve other BODYCOL state.
    for(auto n=state.finalizer.particles.impacts.size();n;){auto& slot=state.finalizer.particles.impacts[--n];if(slot>=0){retire_car_particle(pool,slot);slot=-1;}}
    state.sounds.stop_all();
}

//004DCE40: DM reset and254A precede the optional original-mesh restore. Missing
//factory/rejected load returns before physical reset, matching native behavior.
template<class Frames,class Car,class PhysicalReset>void restore_car_actor(Frames& frames,Car& car,PhysicalReset&& reset){
    car.ancillary->reset_loading(frames,car.geometry->auxiliary().seats().size());car.flag254a=false;
    if(car.flag2548){
        auto model=frames.create_car_restore_model();if(!model)return;
        auto resource=frames.car_model_resource(car.base().frame());const auto slash=resource.rfind('\\');
        if(slash!=std::string::npos)resource=resource.substr(slash+1);
        if(!frames.load_car_ancillary_model(model.get(),resource))return;
        for(auto n=car.occupants.size();n;){if(auto* occupant=car.occupants[--n])frames.car_reaction_mesh_link(occupant->base().frame(),typename Frames::Frame{});}
        //004DCFE0 uses the same name/mesh/update sequence as004DCDE0.
        frames.car_destroyed_meshes(car.base().frame(),model.get());frames.update_scene_object(car.base().frame());
        for(auto n=car.occupants.size();n;){const auto index=--n;if(auto* occupant=car.occupants[index]){
            frames.car_reaction_mesh_link(occupant->base().frame(),car.geometry->auxiliary().seats().at(index).frame);
            frames.update_scene_object(car.occupants[index]->base().frame());
        }}
        car.flag2548=false;
        auto& parts=car.geometry->deformables().parts();for(auto n=parts.size();n;){auto& part=parts[--n];part.mesh=frames.car_mesh(part.frame);}
    }
    reset();
}

//004E9DB0, RET8. Engine and part restore receivers borrow the SAME initialized
//Car. A failing prefix stays published; this is not a new cold initialization.
template<class Frames,class Geometry,class State,class Engine,class Restore>
void reset_car_physics(Frames& frames,Geometry& geometry,State& state,float speed,bool restore,
        Engine&& engine,Restore&& restore_parts){
    const auto model=geometry.model();frames.update_scene_object(model);
    for(std::size_t n=0;n<state.wheels.size();++n){auto& wheel=state.wheels[n];
        if((wheel.flags&1u)&&!(wheel.flags&0x400u)&&frames.object_parent(wheel.parent)==model){
            const auto matrix=frames.car_local_matrix(wheel.parent);
            const auto turn=frames.car_axis_quaternion({{matrix[4],matrix[5],matrix[6],matrix[7]}},0.f);
            const auto& base=wheel.rotation;std::array<float,4> rotation{};
            //004066C0 scales turn.xyz by base.w; its pointer is not a vector
            //subtrahend.00454660 returns extended Z/Y/X dot, without FLOAT spill.
            const float scalar=turn[3]*base[3];
            rotation[3]=static_cast<float>(static_cast<long double>(scalar)-
                ((static_cast<long double>(turn[2])*base[2]+static_cast<long double>(turn[1])*base[1])+static_cast<long double>(turn[0])*base[0]));
            for(unsigned a=0;a!=3;++a){const auto b=(a+1)%3,c=(a+2)%3;
                const float first=turn[a]*base[3],second=turn[3]*base[a];
                const float cross=static_cast<float>(static_cast<long double>(base[b])*turn[c]-static_cast<long double>(base[c])*turn[b]);
                const float sum=first+second;rotation[a]=sum+cross;
            }
            frames.car_steering_rotation(wheel.parent,rotation);(void)frames.car_local_matrix(wheel.parent);
        }
        frames.car_suspension_position(wheel.parent,wheel.vectors.at(0x20));frames.update_scene_object(wheel.parent);
    }
    state.body().reset_pose();
    state.finalizer.sector=frames.object_parent(model);frames.car_place_in_scene(model,state.body().pose().centre);
    state.drive.changing_gear=false; //2293, distinct from engine-running22A2.
    for(unsigned a=0;a!=3;++a){state.body().motion().angular_velocity[a]=0;state.body().motion().velocity[a]=0;}
    state.body().motion().direction=state.body().motion().velocity;state.drive.body_flags=1;
    state.audio.reset_skid();state.brake_ramp=0;state.solver.mass_changed=false;state.solver.speed=0;
    state.drive.engine_speed=state.drive.engine_load=0;
    if(speed!=0){
        for(unsigned a=0;a!=3;++a)state.body().motion().velocity[a]=speed*state.body().pose().forward[a];
        auto direction=state.body().motion().velocity;
        const float square=static_cast<float>((static_cast<long double>(direction[0])*direction[0]+
            static_cast<long double>(direction[1])*direction[1])+static_cast<long double>(direction[2])*direction[2]);
        constexpr float epsilon=9.99999993922529e-09f;
        if(std::abs(static_cast<long double>(square)-1)>=epsilon){
            if(square>=epsilon){const auto scale=1/std::sqrt(static_cast<long double>(square));for(unsigned a=0;a!=3;++a)direction[a]=static_cast<float>(scale*direction[a]);}
            else{const auto a=direction[0]!=0?0u:direction[2]!=0?2u:1u;direction[a]=direction[a]<0?-1.f:1.f;}
        }
        state.body().motion().direction=direction;state.solver.speed=std::abs(speed);
    }
    engine(speed!=0); //004F2530(speed!=0,true), BEFORE gear/wheel/health reset.
    state.drive.gear=state.drive.requested_gear=1;state.reset_fields.field78c=0;state.brake_amount=0;
    state.solver.substeps.field724=0;state.drive.clutch=1;state.steering.current=0;state.drive.brake=0;
    state.health.destroyed=false;state.drive.throttle=state.drive.second_throttle=0;state.steering.requested=0;
    state.reset_fields.signal2294=state.reset_fields.signal2295=false;state.reset_fields.field780=0;state.steering.difference=0;
    for(auto n=state.wheels.size();n;){auto& wheel=state.wheels[--n];
        const auto world=frames.actor_world_matrix(wheel.frame);wheel.vectors[0x40]={{world[12],world[13],world[14],world[15]}};
        wheel.flags&=0xffffff07u;
        if(restore){state.finalizer.puncture_elapsed=30000;wheel.flags&=0x3fffffffu;frames.object_visible(wheel.frame,true);}
        wheel.skid_cursor=0;wheel.flags&=0xefffffffu;
        for(auto offset:{0x16cu,0x1f0u,0x188u,0x25cu,0x260u,0x264u,0x200u,0x168u,0x13cu,0x140u})wheel.scalars.set(offset,0);
        for(auto offset:{0xa0u,0xb0u})for(unsigned a=0;a!=3;++a)wheel.vectors.at(offset)[a]=0;
        wheel.scalars.set(0x26c,1000);wheel.vectors.at(0x130)={{0,1,0,0}};
        const auto& up=state.body().pose().up;auto anchor=wheel.vectors.at(0x50),previous=wheel.vectors.at(0x60);
        const float last=up[2]*wheel.scalars.get(0x1e4);
        for(unsigned a=0;a!=3;++a){anchor[a]=a==2?wheel.vectors.at(0x40)[a]-last:
            static_cast<float>(static_cast<long double>(wheel.vectors.at(0x40)[a])-static_cast<long double>(wheel.scalars.get(0x1e4))*up[a]);previous[a]=anchor[a]-up[a];}
        wheel.vectors[0x50]=anchor;wheel.vectors[0x60]=previous;
    }
    state.controller().desired_clutch=state.drive.clutch_target=1;state.drive.transmission&=0xffffffe0u;state.reset_fields.fieldf68=0;
    if(restore){reset_car_health(frames,geometry.physics(),state.drive,state.health,state.wheels,geometry.lighting(),state.paired_flags);restore_parts();}
    state.finalizer.air_requested=false; //228E.
    for(auto n=geometry.lighting().lights().size();n;){const auto& light=geometry.lighting().lights()[--n];
        if(light.projector==CarLighting<Frames>::none||(light.flags10&0x80000000u))continue;
        const auto& projector=geometry.lighting().projectors().at(light.projector);
        if(projector.visual){frames.car_reset_link(projector.visual.get(),state.finalizer.sector);if(projector.light)frames.car_reset_link(projector.light.get(),state.finalizer.sector);}
    }
    for(auto n=geometry.lighting().projectors().size();n;){const auto& projector=geometry.lighting().projectors()[--n];
        //Native5A4 contains only paired projectors; discover's shared links
        //retain their indices into this unified typed owner.
        bool paired=false;for(const auto& light:geometry.lighting().lights())if(light.shared&&light.projector==n){paired=true;break;}
        if(!paired)continue;
        frames.car_reset_link(projector.visual.get(),state.finalizer.sector);if(projector.light)frames.car_reset_link(projector.light.get(),state.finalizer.sector);
    }
    update_car_wheel_rotation(frames,geometry.physics(),state.body(),state.wheels,state.drive,state.solver.queries,state.solver.wheel_spin,1.f);
    state.collision.pose(state.body().pose().world,state.body().pose().inverse);state.collision.set_enabled(true,true);
    state.solver.mass_changed=true;state.reset_fields.field22c1=false;state.drive.brake=0;
    state.control_requests.second=state.control_requests.first=0;state.reset_fields.fields4f0={};
}

//004DCD10 and004DD040. Keep occupants detached during mesh enumeration. The
//engine callback004DCDE0 swaps each matching visual immediately during traversal.
template<class Frames,class Car>void present_destroyed_car_meshes(Frames& frames,Car& car,typename Frames::Frame helper){
    if(car.flag2548)return;
    for(auto n=car.occupants.size();n;){auto* occupant=car.occupants[--n];if(occupant)frames.car_reaction_mesh_link(occupant->base().frame(),typename Frames::Frame{});}
    frames.car_destroyed_meshes(car.base().frame(),helper);frames.update_scene_object(car.base().frame());
    for(auto n=car.occupants.size();n;){const auto index=--n;if(auto* occupant=car.occupants[index]){
        frames.car_reaction_mesh_link(occupant->base().frame(),car.geometry->auxiliary().seats().at(index).frame);
        frames.update_scene_object(car.occupants[index]->base().frame());
    }}
    auto& parts=car.geometry->deformables().parts();for(auto n=parts.size();n;){auto& part=parts[--n];part.mesh=frames.car_mesh(part.frame);}
    car.flag2548=true;
}

} //namespace hd2::game
#endif
