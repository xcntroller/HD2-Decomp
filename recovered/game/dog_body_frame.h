#ifndef HD2_GAME_DOG_BODY_FRAME_H
#define HD2_GAME_DOG_BODY_FRAME_H
#include "game/dog_body_motion.h"
#include "game/dog_body_turn.h"
#include <exception>
namespace hd2::game {
//005C49A0. The second target getter is intentional: callbacks may change the
//actual controller's target between admission and damage. Retail0069BFE0 is RET.
template<class Dog,class Control,class World>
void apply_dog_bite(Dog& dog,Control& control,World& world){
    if(!control.dog_control_target()||(dog.body_actions.current364!=5&&dog.body_actions.current364!=6))return;
    const auto speed=control.dog_control_speed();
    if(!(speed<=2.0999999046325684f)||dog.body.field24c)return;
    if(world.mode_flags()&1u){
        const bool equal=world.human_integrity_matches(5); //008CAB3A/008CB23A, SAME process tables.
        const auto direction=dog.base().world_direction();
        auto* target=control.dog_control_target();
        if(!target)throw std::runtime_error("Dog bite reaches native NULL second control target");
        auto amount=dog.properties.fields4a0[1];if(!equal)amount*=10u; //Native low32 BEFORE signed FILD.
        world.dog_bite_damage(*target,static_cast<float>(dog_order_signed(amount)),{{0,0,0,0}},direction);
        world.dog_bite_observed(dog);
    }
}
//005C8B30 tail: temporary model placement exists only to update the SAME five
//descriptors. Normal restoration follows descriptor moves; exception cleanup
//restores the borrowed model without replacing the first failed requirement.
template<class Dog,class Frames>
void refresh_dog_body_collision_pose(Dog& dog,Frames& frames){
    using Vector=std::array<float,4>;
    const auto matrix=frames.actor_world_matrix(dog.base().frame());
    const Vector original{{matrix[12],matrix[13],matrix[14],matrix[15]}};
    const auto position=dog.base().world_position();
    const auto dx=static_cast<long double>(original[0])-position[0],dy=static_cast<long double>(original[1])-position[1],dz=static_cast<long double>(original[2])-position[2];
    const bool move=(dz*dz+dy*dy)+dx*dx>9.999999747378752e-05f;
    const auto refresh_sector=[&](Vector point, bool mark_moved, bool& moved){
        point[1]=static_cast<float>(static_cast<long double>(point[1])+.5f);
        const auto parent=frames.actor_parent(dog.base().frame());
        const auto sector=frames.human_scene_sector(point);
        if(mark_moved)moved=true; //Native005C8E54, AFTER SceneE0, BEFORE parent-kind/read/link.
        if(parent&&frames.frame_kind(parent)==5&&parent!=sector)frames.dog_body_link_sector(dog.base().frame(),sector);
        frames.update_scene_object(dog.base().frame());
    };
    bool moved=false,placed=false;
    const auto restore=[&]{
        frames.physics_replacement_position(dog.base().frame(),original);
        bool unused=false;refresh_sector(original,false,unused);placed=false;
    };
    try{
        if(move){
            frames.physics_replacement_position(dog.base().frame(),position);placed=true;
            refresh_sector(dog.base().world_position(),true,moved);
        }
        if(!dog.collisions)throw std::runtime_error("Dog Body frame requires actual collision descriptor owner");
        dog.collisions->update(); //Refresh anchors ONCE; alive first two, then last three regardless of health.
        if(moved)restore();
    }catch(...){
        const auto error=std::current_exception();
        if(placed)try{restore();}catch(...){frames.record_actor_cleanup_failure(std::current_exception());}
        std::rethrow_exception(error);
    }
}
template<class Dog,class World,class Frames,class Control>
void advance_dog_body_frame(Dog& dog,std::uint32_t elapsed,World& world,Frames& frames,Control& control){
    auto direction=dog.body.direction180;
    turn_dog_body_direction(direction,dog,frames,control);
    if(!dog.clip)throw std::runtime_error("Dog Body frame requires actual clip owner");
    dog.clip->advance_motion(dog_order_signed(elapsed),direction);
    advance_dog_body_motion(dog,dog_order_signed(elapsed),world,frames,control);
    if(dog.body.flag268){dog.body.flag268=false;apply_dog_bite(dog,control,world);}
    auto& timer=dog.body_actions.timer370;if(timer>0)timer=dog_order_signed(static_cast<std::uint32_t>(timer)-elapsed);
    dog.body_transform.queue_position(dog.base().world_position(),dog,frames);
    if(world.mode_flags()&1u){
        float rate=0;std::uint32_t kind=6;
        switch(dog.body.material1f0){
        case 0x46:rate=.009999999776482582f;break;
        case 0x47:rate=.029999999329447746f;break;
        case 0x5a:rate=.019999999552965164f;break;
        case 0x48:rate=.5f;break;
        case 0x5d:kind=9;break;
        default:break;
        }
        if(rate!=0.f||kind==9)world.dog_surface_damage(dog,kind,kind==9?-1.f:static_cast<float>(static_cast<long double>(elapsed)*rate),{{0,0,0,0}},{{0,0,0,0}});
    }
    refresh_dog_body_collision_pose(dog,frames);
}
}
#endif
