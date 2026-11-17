#ifndef HD2_GAME_DOG_LOADING_H
#define HD2_GAME_DOG_LOADING_H
#include "game/dog_body_state.h"
#include "game/dog_animation_events.h"
namespace hd2::game {
//005D4DB0. A missing validation marker returns from this body only;5290
//still enters Body binding. Do not turn that partial prefix into live readiness.
template<class Dog,class Frames,class World>void initialize_dog_live(Dog& dog,Frames& frames,World& world){
    for(const char* name="bcdefghijklmnopqrstuvwxy";*name;++name){
        const char marker[2]{*name,0};if(!frames.actor_find(dog.base().frame(),marker,0x400u))return;
    }
    world.bind_dog_head_animation(dog);
    dog.head_look.initialize();dog.suspended530=false;dog.flag538=false;
    world.dog_fade_in_immediate(dog.base().frame());
    const auto matrix=frames.actor_world_matrix(dog.base().frame());
    dog.frame_history.initialize({{matrix[12],matrix[13],matrix[14],matrix[15]}});
    dog.sight_marker544=frames.actor_find(dog.base().frame(),"f",0x400u);
    world.request_dog_head_look(dog,{{0,0,0,0}},0x18u,1u);
    const auto mode=world.mode_flags();std::int32_t kind=7;
    if((mode&7u)==7u||(mode&1u)){
        const auto* data=frames.find_actor_frame_data(dog.base().frame());kind=data&&data->words[1]?5:4;
    }
    world.replace_dog_control(dog,kind);
    dog.sight_source53c=world.dog_add_sight_source(dog.base().frame());
    dog.sight_listener540=world.dog_add_sight_target(dog.base().frame());
    dog.sound548=world.dog_add_sound_listener(dog.base().frame());
    dog.friendly54c.clear();
    //The actual Scene2 identity count and getter are read afresh each time.
    for(std::size_t n=0;n<world.dog_scene_count();++n){
        if(!world.dog_scene_actor(n))continue;
        if(world.dog_scene_human_kind(n)!=1)continue;
        dog.friendly54c.push_back(world.dog_scene_actor(n));
    }
    dog.elapsed55c=-1000;world.dog_enable_sight_target(dog.require_sight_listener());
}
//005D5290 calls common activation on BOTH passes; derived work is authority-only.
template<class Dog,class Frames,class World>void activate_dog(Dog& dog,bool authority,Frames& frames,World& world){
    dog.base().activate_frame(authority,world);if(!authority)return;
    if(!frames.ensure_dog_animations())return;
    dog.initial_properties4b0=dog.properties.fields4a0;dog.initial_health4b0=dog.properties.fields4a0[0];
    const auto matrix=frames.actor_world_matrix(dog.base().frame());
    dog.initial_position4c0=typename Frames::Vector{{matrix[12],matrix[13],matrix[14],matrix[15]}};
    dog.initial_rotation4d0=frames.actor_world_rotation(dog.base().frame());
    initialize_dog_live(dog,frames,world);
    world.bind_live_dog_body(dog);
}
//005D5390. Common deactivation precedes resource retirement. Explicit source
//provenance guards preserve failure at an unproduced native positive owner.
template<class Dog,class Frames,class World>void deactivate_dog(Dog& dog,bool authority,Frames& frames,World& world){
    dog.base().deactivate_frame(authority,world);if(!authority)return;
    if(dog.head_animation4e0){dog.head_animation4e0->close();dog.head_animation4e0.reset();}
    dog.require_controller().dog_control_close();dog.field534.reset();
    if(!dog.collisions)throw std::runtime_error("Dog retirement requires actual Body collision owner");
    retire_dog_body(*dog.collisions,dog.body_actions,world);
    dog.spotted574.clear();
    for(auto n=dog.heard564.size();n;)world.dog_controller_process().heard.release(dog.heard564[--n]);
    dog.heard564.clear();
    if(!dog.sight_source53c)throw std::runtime_error("Dog retirement requires actual observer producer");
    if(*dog.sight_source53c>=0){world.dog_remove_sight_source(*dog.sight_source53c);dog.sight_source53c=-1;}
    if(!dog.sight_listener540)throw std::runtime_error("Dog retirement requires actual target producer");
    if(*dog.sight_listener540>=0){world.dog_remove_sight_target(*dog.sight_listener540);dog.sight_listener540=-1;}
    if(!dog.sound548)throw std::runtime_error("Dog retirement requires actual SOUND producer");
    if(*dog.sound548!=-1){world.dog_remove_sound_listener(*dog.sound548);dog.sound548=-1;}
    frames.reset_dog_shared_state();
    //Typed callback loans retire before the native E0 model reload invalidates
    //their marker. No clip/marker may borrow the reloaded model's old hierarchy.
    dog.clip.reset();dog.animation_events.reset();dog.collisions.reset();dog.body.initialized=false;
    world.restore_dog_model(dog);
    if(!dog.initial_properties4b0)throw std::runtime_error("Dog retirement requires actual4B0 properties snapshot");
    dog.properties.fields4a0=*dog.initial_properties4b0;
}
} //namespace hd2::game
#endif
