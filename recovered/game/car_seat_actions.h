#ifndef HD2_GAME_CAR_SEAT_ACTIONS_H
#define HD2_GAME_CAR_SEAT_ACTIONS_H
#include "game/car_control_requests.h"
#include "game/car_steering.h"
#include "game/human_ai_shot_direction.h"
#include "game/human_frame_inputs.h"
namespace hd2::game {
//Seat algorithms share the containing loading owner; no separate runtime state.
template<class Owner,class Human>
void assign_car_seat(Owner& owner,std::int32_t index,Human* human){
    auto& actor_=owner.actor_;
    auto& initial_=owner.initial_;
    auto& frames_=owner.frames_;
    auto& services_=owner.services_;
    auto& seats_=owner.seats_;
    auto& body_collision_=owner.body_collision_;
    using Vector=typename Owner::Vector;

 //004D9430, actual virtual74.
        owner.require_mode();owner.require_geometry();if(!initial_)throw std::logic_error("Car seat entry requires initialized physics");
        if(services_.session_mission_ordinal()>5&&!services_.car_seat_integrity_matches())initial_->set_fuel(0); //Stage60, before bounds.
        if(index<0||static_cast<std::size_t>(index)>=actor_.occupants.size())return;
        const auto at=static_cast<std::size_t>(index);auto* occupant=actor_.occupants[at];if(occupant&&occupant!=human)return;
        for(auto n=actor_.occupants.size();n;){const auto old=--n;if(actor_.occupants[old]==human)owner.remove_seat(old);}
        if(auto& control=actor_.fields24b4.at(at))control->loading->set_occupant(human);
        if(human){
            initial_->mass.seat(at,80.f,initial_->solver.mass_changed);
            const auto marker=actor_.geometry->auxiliary().seats().at(at).frame;
            const auto matrix=frames_.actor_world_matrix(marker);Vector direction{{matrix[8],matrix[9],matrix[10],matrix[11]}};
            const float square=static_cast<float>((static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[1])*direction[1]);
            normalize_human_ai_shot(direction,square);
            actor_.occupants[at]=human;
            frames_.human_seat_link(human->base().frame(),marker);frames_.human_seat_origin(human->base().frame());
            const auto current=frames_.actor_world_matrix(human->base().frame());human->base().world_position(owner.position(current));
            services_.seat_human_direction(human,direction);human->base().world_direction(direction);
            if(!index){
                actor_.driver24ac=human;actor_.driver_enabled2321=true; //004D11C0;00611080(0) genuine RET4.
                owner.request_engine(true,!(services_.mode_flags()&2u));
                const auto kind=services_.seat_human_state_kind(human);
                for(auto* flags:{&initial_->wheel_flags,&initial_->body_flags})*flags=((kind==1||kind==3)?*flags&~0x800000u:*flags|0x800000u)|0x200000u;
                body_collision_->query_flags(initial_->body_flags);
                if(services_.mode_flags()&1u){services_.seat_proximity_kind(static_cast<std::int32_t>(actor_.field24dc),3);services_.seat_human_proximity_kind(human,3);}
            }
            actor_.input_flags2329.fill(false);
        }
        frames_.update_scene_object(actor_.geometry->model());services_.seat_script_use(actor_.geometry->model());
        if(human&&actor_.callback_services().car_human_controlled(human))seats_.view(at,services_.seat_view_enabled());
        initial_->drive.body_flags|=0x20000000u;

}
template<class Owner>
bool exit_car_seat(Owner& owner,std::int32_t index){
    auto& actor_=owner.actor_;
    auto& initial_=owner.initial_;
    auto& frames_=owner.frames_;
    auto& services_=owner.services_;
    auto& seats_=owner.seats_;
    using Vector=typename Owner::Vector;

 //004D9BE0→004DA2F0→004DA420, distinct from forced virtual80 release.
        if(index<0||static_cast<std::size_t>(index)>=actor_.occupants.size())return false;
        const auto at=static_cast<std::size_t>(index);auto* human=actor_.occupants[at];if(!human)return false;
        owner.require_mode();if(!initial_)throw std::logic_error("Car exit requires initialized physics");
        auto selected=actor_.occupants.size();for(auto n=actor_.occupants.size();n;){const auto candidate=--n;if(actor_.occupants[candidate]==human){selected=candidate;break;}}
        if(selected==actor_.occupants.size())return false;
        Vector placed{},direction{};
        if(!owner.exit_pose(selected,placed,direction)){
            bool found=false;
            for(std::size_t candidate=0;candidate<actor_.occupants.size();++candidate)if(candidate!=selected&&owner.exit_pose(candidate,placed,direction)){found=true;break;}
            if(!found)return false;
        }
        frames_.human_seat_link(human->base().frame(),frames_.camera_root_sector());
        human->base().world_direction(direction);human->base().world_position(placed);frames_.human_seat_exit_pose(human->base().frame(),placed,direction);
        initial_->mass.seat(at,0,initial_->solver.mass_changed);actor_.occupants[at]=nullptr;
        if(auto& control=actor_.fields24b4.at(at))control->loading->clear_occupant();
        if(!index){actor_.driver_enabled2321=false;actor_.driver24ac=nullptr; //004D1200(false), then004D11C0(NULL).
            owner.request_engine(false,!initial_->finalizer.engine_transition);
            const auto speed=actor_.geometry->physics().scalars().get(0x6f0);initial_->drive.governor=speed>0?speed+.5f:speed;
            initial_->brake_ramp=initial_->brake_amount=0;initial_->brake_latch=false;
            if(services_.mode_flags()&1u){services_.seat_proximity_kind(static_cast<std::int32_t>(actor_.field24dc),4);services_.seat_human_proximity_kind(human,services_.seat_human_health(human)<=0?1u:0u);}
        }
        frames_.update_scene_object(actor_.geometry->model());services_.seat_script_use(actor_.geometry->model());
        if(actor_.callback_services().car_human_controlled(human))seats_.view(at,false);
        bool empty=true;for(auto* value:actor_.occupants)if(value){empty=false;break;}
        if(empty)initial_->drive.body_flags|=0x20000000u;else initial_->drive.body_flags&=~0x20000000u;
        if(actor_.callback_services().car_human_controlled(human))owner.driver_input(0,0);
        return true;

}
template<class Owner>
void request_car_seat_input(Owner& owner,float pedal,float steering){
    auto& actor_=owner.actor_;
    auto& initial_=owner.initial_;
    auto& services_=owner.services_;

 //004D9280; native guard precedes masks and both actual controls.
        if(!actor_.driver24ac||services_.seat_human_health(actor_.driver24ac)<=0)return;
        const auto flags=actor_.control_mask24a8;
        if(flags&4u)pedal=0;
        if((flags&3u)==3u)pedal=steering=0;
        else if(flags&1u){if(steering<0)steering=0;if(pedal!=0&&steering==0){steering=-1;pedal*=.30000001192092896f;}}
        else if(flags&2u){if(steering>0)steering=0;if(pedal!=0&&steering==0){steering=1;pedal*=.30000001192092896f;}}
        if((services_.mode_flags()&7u)!=7u&&!actor_.callback_services().car_human_controlled(actor_.driver24ac))return;
        if(!initial_)throw std::logic_error("Car input requires initialized physics");
        request_car_drive_pedal(initial_->control_requests,initial_->drive,actor_.geometry->physics().scalars(),initial_->solver.substeps,pedal,true);
        request_car_steering(initial_->steering,steering);

}
template<class Owner>
void request_car_driver_frame_inputs(Owner& owner,const HumanFrameInputBuffer& first,const HumanFrameInputBuffer&){ //004D9050; second buffer is genuinely unused.
    auto& actor=owner.actor_;auto& services=owner.services_;
    if(!actor.driver24ac)return;
    if((services.mode_flags()&7u)!=7u&&!actor.callback_services().car_human_controlled(actor.driver24ac))return;
    if(!owner.initial_)throw std::logic_error("Car frame input requires actual initialized physics");
    auto& state=*owner.initial_;const auto& fields=actor.geometry->physics().scalars();
    if(!state.finalizer.engine_transition&&state.fuel>0.019999999552965164f)owner.request_engine(true,false);
    float steering=static_cast<float>(static_cast<long double>(first.get<float>(0xc))-first.get<float>(8));
    float pedal=first.get<float>(0);const auto flags=actor.control_mask24a8;if(flags&4u)pedal=0;
    if(services.seat_human_health(actor.driver24ac)<=0){
        request_car_drive_pedal(state.control_requests,state.drive,fields,state.solver.substeps,0.f,true);request_car_steering(state.steering,0.f);
        request_car_brake_pedal(state.control_requests,state.drive,fields,state.solver.substeps,0.f);
        request_car_emergency_brake(fields,state.solver.substeps,state.brake_ramp,state.brake_amount,state.brake_latch,false,true);
        request_car_clutch_pedal(state.control_requests,fields,state.controller().desired_clutch,0.f);return;
    }
    if((flags&3u)==3u)pedal=steering=0;
    else if(flags&1u){if(steering<0)steering=0;if(pedal!=0&&steering==0){steering=-1;pedal*=.30000001192092896f;}}
    else if(flags&2u){if(steering>0)steering=0;if(pedal!=0&&steering==0){steering=1;pedal*=.30000001192092896f;}}
    request_car_drive_pedal(state.control_requests,state.drive,fields,state.solver.substeps,pedal,true);request_car_steering(state.steering,steering);
    request_car_brake_pedal(state.control_requests,state.drive,fields,state.solver.substeps,first.get<float>(4));
    request_car_emergency_brake(fields,state.solver.substeps,state.brake_ramp,state.brake_amount,state.brake_latch,first.get<float>(0x48)!=0,!(pedal>0));
    const auto speed=fields.get(0x6f0);state.drive.governor=speed>0?speed+.5f:speed;
    request_car_clutch_pedal(state.control_requests,fields,state.controller().desired_clutch,static_cast<float>(1.L-first.get<float>(0xf4)));
}
template<class Owner>
void request_car_seat_engine(Owner& owner,bool running,bool force){
    auto& initial_=owner.initial_;

 //004F2530, reached occupied entry/exit.
        auto& state=*initial_;auto& final=state.finalizer;
        if(force){state.engine_timeout=5000;state.health.destroyed=false;
            if(!running){state.drive.engine_speed=0;final.engine_transition=false;state.drive.running=false;state.sounds.stop_all();}
            else{final.engine_transition=true;state.drive.running=true;state.engine_timeout=0;}return;
        }
        if(running==final.engine_transition)return;
        if(running&&(state.fuel<=.019999999552965164f||((state.solver.substeps.flags&0x4000u)&&state.health.body<=0))){
            if(state.drive.audible)state.audio.failed_engine_start(final.engine_inside2298);
            return;
        }
        if(running)state.audio.start_engine_transition(state.drive,final.inside,final.engine_transition,final.engine_inside2298,state.engine_timeout);
        else state.audio.stop_engine_transition(state.drive,final.inside,final.engine_transition,state.health.destroyed);
        if(!final.engine_transition){state.control_requests.first=state.control_requests.second=0;state.health.destroyed=false;}

}
template<class Owner>
void remove_car_seat(Owner& owner,std::size_t index){
    auto& actor_=owner.actor_;
    auto& initial_=owner.initial_;
    auto& frames_=owner.frames_;
    auto& seats_=owner.seats_;

 //004D9A40, SAME occupied/camera/mass/control owners.
        if(auto* occupant=actor_.occupants.at(index)){
            frames_.human_reaction_link(occupant->base().frame(),frames_.camera_root_sector());
            if(actor_.callback_services().car_human_controlled(occupant))seats_.view(index,false);
        }
        initial_->mass.seat(index,0,initial_->solver.mass_changed);actor_.occupants[index]=nullptr;
        seats_.restore_camera(index);
        if(index==0){actor_.driver24ac=nullptr;actor_.driver_enabled2321=false;
            owner.request_engine(false,!initial_->finalizer.engine_transition);
            const auto speed=actor_.geometry->physics().scalars().get(0x6f0);initial_->drive.governor=speed>0?speed+.5f:speed; //004F2630.
            initial_->brake_ramp=initial_->brake_amount=0;initial_->brake_latch=false; //004F26A0(0,0).
        }
        if(auto& control=actor_.fields24b4.at(index))control->loading->clear_occupant();
        seats_.seats().at(index).flags&=~0x400u;frames_.update_scene_object(actor_.geometry->model());
        bool empty=true;for(auto occupant:actor_.occupants)if(occupant){empty=false;break;}
        if(empty)initial_->drive.body_flags|=0x20000000u;else initial_->drive.body_flags&=~0x20000000u;

}
} //namespace hd2::game
#endif
