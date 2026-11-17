#ifndef HD2_GAME_DOG_BODY_PRESENTATION_H
#define HD2_GAME_DOG_BODY_PRESENTATION_H
#include "game/dog_body_transform.h"
#include "game/dog_body_state.h"
#include "game/dog_clip_mixer.h"
#include "game/human_motion_velocity.h"
namespace hd2::game {
inline bool dog_body_turn_admitted(float&){return true;} //005C1DB0: MOV AL,1;RET4, no angle write.
//005C1B80. Snapshot actual direction before any rotation; native keeps its Y.
template<class Dog,class Frames>bool turn_dog_presentation(Dog& dog,std::int32_t elapsed,Frames& frames){
    auto target=dog.body.direction180;target[1]=0;
    auto direction=dog.base().world_direction();const auto y=direction[1];
    const auto extended=human_motion_planar_angle(target,direction);float angle=static_cast<float>(extended);
    constexpr double epsilon=9.999999747378752e-05;
    if(std::abs(extended)<epsilon)return true;
    const auto admitted=dog_body_turn_admitted(angle);
    if(std::abs(static_cast<long double>(angle))<epsilon)return true;
    const auto factor=dog.require_controller().dog_control_kind()==1?3.f:dog.body_orders.next_posture()==1?.009999999776482582f:.0005000000237487257f;
    const auto magnitude=std::abs(static_cast<long double>(angle));
    const auto step=(magnitude+1.0)*factor*(angle>0?-1.L:1.L)*elapsed;
    const float stored=static_cast<float>(step);
    if(std::abs(static_cast<long double>(stored))>=magnitude&&admitted){
        target[1]=y;dog.base().world_direction(target);dog.body.flag231=true;return true;
    }
    frames.car_transform_in_place(direction,frames.human_axis_rotation({{0,1,0,0}},stored));
    direction[1]=y;dog.base().world_direction(direction);dog.body.flag231=true;return false;
}
//005C1430. The physical Actor30 and model translation are distinct owners.
template<class Dog,class Frames>void interpolate_dog_model(Dog& dog,Frames& frames){
    auto& body=dog.body;auto& transform=dog.body_transform;
    body.flag232=false;transform.requested3c0=false;
    bool publish=false;long double fraction=0;
    if(!dog.body_actions.flags3e0&&!body.flag3e4){fraction=static_cast<long double>(dog_order_signed(body.field388))*.009999999776482582f;publish=true;}
    else if(transform.interval38c>0){
        const auto limit=transform.interval38c;
        if(dog_order_signed(body.field388)>=limit)body.field388=static_cast<std::uint32_t>(limit);
        const auto elapsed=dog_order_signed(body.field388);fraction=static_cast<long double>(elapsed)/limit;publish=true;
        if(elapsed==limit)transform.interval38c=-1;
    }
    if(publish){
        if(!transform.reference3a0)throw std::runtime_error("Dog model interpolation requires actual Body3A0 reference");
        const auto& origin=*transform.reference3a0;HumanVector position=origin;
        position[0]=static_cast<float>(fraction*transform.displacement3d0[0]+origin[0]);
        for(unsigned n=1;n!=3;++n){const float component=static_cast<float>(fraction*transform.displacement3d0[n]);position[n]=static_cast<float>(static_cast<long double>(component)+origin[n]);}
        body.flag232=true;
        frames.physics_replacement_position(dog.base().frame(),position);
        position[1]=static_cast<float>(static_cast<long double>(position[1])+.5f);
        frames.dog_place_body_scene(dog.base().frame(),position);
        if(body.flag232)frames.update_scene_object(dog.base().frame());
    }
    body.flag232=false;
}
//005C1600: consume/reset each actual group count BEFORE iterating its live
//event storage. Sound callbacks can change later events; never copy the array.
template<class Dog,class World,class Frames>void dispatch_dog_body_events(Dog& dog,World& world,Frames& frames){
    if(!dog.clip)throw std::runtime_error("Dog Body events require actual clip owner");
    for(unsigned group=0;group!=2;++group){
        const auto count=dog.clip->take_event_count(group);
        for(std::int32_t index=0;index<count;++index){
            if(!dog.clip)throw std::runtime_error("Dog Body event callback retired actual clip");
            const auto event=dog.clip->event_value(group,static_cast<unsigned>(index));
            if(event==0){if(dog.body.material1f0>=0)world.dog_footstep(dog,dog.body.material1f0);}
            else if(event==1){
                auto direction=dog.base().world_direction();dog.body.flag268=true;
                if(dog.body_actions.current364==5){
                    if(direction[1]!=0){direction[1]=0;normalize_human_ai_shot(direction,static_cast<long double>(direction[2])*direction[2]+static_cast<long double>(direction[0])*direction[0]);}
                    const bool running=dog.body_orders.next_posture()==1;direction[1]=running?14.f:12.f;
                    const float scale=running?.800000011920929f:0.f;direction[0]*=scale;direction[2]*=scale;
                    dog.clip->decelerate(direction,1000);
                }
            }else if(event==2)dog.body.flag268=false;
        }
    }
    (void)frames; //The World footstep endpoint owns its actual fresh model query.
}
//005C18C0, actual presentation on SAME live Body/clip/control/transform graph.
template<class Dog,class World,class Frames>void present_dog_body(Dog& dog,std::uint32_t elapsed,World& world,Frames& frames){
    auto& body=dog.body;auto& actions=dog.body_actions;auto& transform=dog.body_transform;
    if(!body.initialized)throw std::logic_error("Dog Body presentation requires actual completed005C7D30");
    const auto allowance=static_cast<long double>(elapsed)*.0010000000474974513f;
    if(actions.speed248!=body.speed244){
        if(std::abs(static_cast<long double>(actions.speed248)-body.speed244)<=allowance)body.speed244=actions.speed248;
        else body.speed244=static_cast<float>(static_cast<long double>(body.speed244)+(actions.speed248>body.speed244?allowance:-allowance));
    }
    const auto clip=[&]() -> auto& {if(!dog.clip)throw std::runtime_error("Dog Body presentation requires actual clip");return *dog.clip;};
    clip().rate(0,body.speed244);
    if(dog.require_controller().dog_control_kind()==1)world.poll_dog_manual_control(dog);
    clip().tick(dog_order_signed(elapsed),dog_order_signed(elapsed),false,nullptr);
    (void)turn_dog_presentation(dog,dog_order_signed(elapsed),frames);
    if(dog.require_controller().dog_control_kind()==1)world.dog_manual_camera_direction(dog.base().world_direction());
    transform.request_direction(dog.base().world_direction(),dog,frames);
    const auto changed=clip().consume_changed();
    if(changed&&actions.current364!=5&&actions.current364!=6){
        actions.animation2_168=0;frames.dog_clear_animation2_position(body.animation2_164);actions.animation2_position170.fill(0);
    }
    if(transform.requested3c0){if(body.flag3e4)body.flag3e4=false;body.field388=elapsed;}
    else{if(actions.flags3e0)body.flag3e4=true;body.field388+=elapsed;}
    dispatch_dog_body_events(dog,world,frames);
    interpolate_dog_model(dog,frames);
}
//005D5810: actual history/controller/head/Body/model ordering. Missing cold
//production is distinct from native produced530=true or final death374=4.
template<class Dog,class World,class Frames>void present_dog_scene_actor(Dog& dog,std::uint32_t elapsed,World& world,Frames& frames){
    if(!dog.suspended530)throw std::runtime_error("Dog presentation requires actual005D4DB0 flag530 producer");
    if(*dog.suspended530||dog.body_actions.death374==4)return;
    dog.frame_history.advance(dog,dog_signed_word(elapsed),frames);
    dog.require_controller().dog_control_presentation(elapsed);
    world.present_dog_head_look(dog,elapsed);
    present_dog_body(dog,elapsed,world,frames);
    frames.supply_model_tick(dog.base().frame(),elapsed);
    frames.update_scene_object(dog.base().frame());
}
} //namespace hd2::game
#endif
