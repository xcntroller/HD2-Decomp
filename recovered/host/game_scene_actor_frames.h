#ifndef HD2_HOST_GAME_SCENE_ACTOR_FRAMES_H
#define HD2_HOST_GAME_SCENE_ACTOR_FRAMES_H

#include "game/human_ai_scene_steering.h"
#include "game/human_scene_movement.h"
#include "host/game_scene_actors.h"

namespace hd2::host {

//Binds the shared solver to the containing Startup, Scene2 and Human owners.
//Runtime supplies the original positive local path replanning worker.
template<class Dependencies,class Services,class Runtime>class GameSceneMovementFrames {
public:
    using Actor=GameSceneActors::Instance;
    using Human=GameSceneActors::Human;
    using Car=game::CarSceneActor<GameActorServices>;
    GameSceneMovementFrames(Dependencies& dependencies,Services& services,Runtime& runtime)
        :d_(dependencies),services_(services),runtime_(runtime){}
    void advance(std::uint32_t elapsed,const std::vector<Actor*>& active){
        d_.startup.human_scene_movement_process().advance(elapsed,active,*this);
    }
    std::uint32_t mode_flags()const{return d_.startup.application_flags();}
    bool human_remote_ai_enabled(){return d_.startup.human_remote_ai_enabled();}
    bool human_unavailable(const Human& human)const{return services_.human_unavailable(human);}
    game::HumanVector movement_model_position(const Actor& actor){
        if(!actor.base().frame())throw std::logic_error("Scene movement requires an actual model frame");
        const auto matrix=d_.scene.frames().actor_world_matrix(actor.base().frame());
        return {{matrix[12],matrix[13],matrix[14],matrix[15]}};
    }
    game::HumanVector car_movement_radii(const Car& car){
        if(!car.geometry)throw std::logic_error("Human movement requires actual Car CENTRE bounds");
        const auto& bounds=car.geometry->auxiliary().collision_bounds(); //Embedded Car+90 bounds at 180/190.
        game::HumanVector radii{};
        for(unsigned axis=0;axis!=3;++axis){
            const auto extent=static_cast<long double>(bounds.maximum[axis])-bounds.minimum[axis];
            const auto stored=axis==2?static_cast<long double>(static_cast<float>(extent)):extent;
            radii[axis]=static_cast<float>(stored*.55f);
        }
        return radii;
    }
    std::array<game::HumanVector,3> car_movement_axes(Car& car){
        const auto matrix=d_.scene.frames().actor_world_matrix(car.base().frame());
        return {{{{matrix[0],matrix[1],matrix[2],matrix[3]}},
                 {{matrix[4],matrix[5],matrix[6],matrix[7]}},
                 {{matrix[8],matrix[9],matrix[10],matrix[11]}}}};
    }
    game::HumanVector car_movement_velocity(Car& car){return car.callback_services().car_position4a0(&car);}
    float car_movement_speed(Car& car){return car.callback_services().car_speed958(&car);}
    const auto& movement_neighbour_ids(const Human& human)const{
        if(!human.command)throw std::logic_error("Scene movement requires actual Human B4");
        return human.command->lists[0]; //Native command vector at B4+78.
    }
    Human* movement_neighbour(std::uint32_t identity){
        auto* actor=d_.scene.frames().scene_actor_command_identity(game::human_frame_signed_word(identity));
        if(!actor)return nullptr;
        auto* human=dynamic_cast<Human*>(actor);
        if(!human)throw std::logic_error("Scene movement neighbour has no Human+2C4 owner");
        return human;
    }
    bool human_movement_floor(Human& human){return services_.human_inline(human).frame_navigation_floor();}
    bool human_movement_animation_blocked(Human& human){
        auto* control=services_.human_inline(human).control();
        if(!control)throw std::logic_error("Scene movement requires the actual DD0 owner");
        const auto state=control->animation_state_word(0x1d8);
        return state==8||state==14||(state>=106&&state<=108); //005AC2B0, caller tests nonzero.
    }
    std::uint32_t human_movement_posture(Human& human){return services_.human_inline(human).frame_posture_mode();}
    float human_movement_path_speed(Human& human){
        if(!human.loading_state)throw std::logic_error("Scene movement speed requires actual B0");
        const auto flags=human.loading_state->words().template get<std::uint32_t>(0x50)&0xf8u;
        auto stance=flags?services_.human_inline(human).frame_movement_mode():1u; //005A5CC0 returns -1 without movement.
        if(stance==5&&!(flags&8u))stance=flags&0x10u?1u:2u;
        else if(stance==2&&(flags&0x10u))stance=1;
        const auto posture=human_movement_posture(human);
        constexpr float speeds[6][6]={ //00816878.
            {.63f,1.3f,2.8f,.781f,.457f,4.1f},{.32f,.65f,2.f,.32f,.65f,2.f},
            {.2f,.476f,.8f,.2f,.476f,.8f},{.63f,.85f,.85f,.536f,.85f,.85f},
            {.29f,.62f,.62f,.29f,.62f,.62f},{.2f,.3f,.3f,.2f,.3f,.3f}
        };
        if(posture>=6||stance>=6)throw std::runtime_error("Scene movement requires a valid E0 speed-table index");
        return static_cast<float>(static_cast<long double>(d_.startup.human_animation_time_scale())*speeds[posture][stance]);
    }
    auto* movement_table_record(std::uint16_t index){return d_.navigation.record(index);}
    auto* nearest_movement_record(const game::HumanVector& position){
        return d_.navigation.record(d_.navigation.nearest_volume({{position[0],position[1],position[2]}},5.f,-1.f));
    }
    auto& movement_floor_candidates(){return d_.navigation.movement_floor_candidates();}
    void refresh_movement_floor(Human& human,game::MissionTableRecord* previous){
        auto& movement=*human.scene_movement;
        game::human_scene_movement::refresh_floor(movement.position20,movement.floor_record,previous,*this);
    }
    void replan_movement_path(Human& human){runtime_.replan_movement_path(human);}
    void prepare_movement_prone_axes(Human& human){ //005656E0, actual local frame rotation and X/Z normalization.
        auto& movement=*human.scene_movement;
        if(!human.base().frame())throw std::logic_error("Prone movement requires the actual Human model");
        const auto forward=d_.scene.frames().car_forward(human.base().frame());
        const auto horizontal=game::normalize_ai_scene_horizontal(forward[0],forward[2]);
        movement.forwardB0[0]=static_cast<float>(horizontal[0]);
        movement.forwardB0[1]=0;
        movement.forwardB0[2]=static_cast<float>(horizontal[2]);
        movement.side90[0]=-movement.forwardB0[2];movement.side90[1]=0;movement.side90[2]=movement.forwardB0[0];
        movement.upA0[0]=0;movement.upA0[1]=1;movement.upA0[2]=0;
        movement.centreC0=movement.position20;
        movement.radiiD0={{.7f,.3f,1.3f}};
    }
private:
    Dependencies& d_;Services& services_;Runtime& runtime_;
};

//Actual Scene2 frame facade: SAME registered actors, buckets, proximity and
//Human state owners as loading/activation. Movement must be the real008AADD0
//005647C0 receiver; Derived must provide actual Human/Dog and runtime11 ticks.
//There is no replacement list, successful empty movement or cold Human tick.
template<class Dependencies,class Movement,class Derived>class GameSceneActorFrames {
public:
    using Actor=GameSceneActors::Instance;
    using Human=GameSceneActors::Human;
    GameSceneActorFrames(Dependencies& dependencies,Movement& movement,Derived& derived)
        :d_(dependencies),movement_(movement),derived_(derived){}
    void tick_scene(bool authority,std::uint32_t elapsed){d_.scene.frames().tick_scene_actors(authority,elapsed,*this);}
    void tick_runtime(bool authority,std::uint32_t elapsed){d_.world_actors.runtime().tick(authority,elapsed,derived_);}
    std::uint32_t mode_flags()const{return d_.startup.application_flags();}
    bool human_remote_ai_enabled(){return d_.startup.human_remote_ai_enabled();}
    void flush_retired_scene_actors(std::uint32_t elapsed){
        d_.scene.frames().flush_scene_actor_removals(elapsed,d_.world_actors,d_.scene.buckets());
    }
    void advance_actor_proximity100(){d_.proximity.tick100();}
    void advance_human_movement(std::uint32_t elapsed,const std::vector<Actor*>& active){movement_.advance(elapsed,active);}
    void update_scene_actor(Actor& actor,bool authority,std::uint32_t elapsed){
        game::update_loading_scene_actor(actor,authority,elapsed,d_.world_actors,derived_);
    }
    game::SceneHumanFrameSummary human_frame_summary(Actor& actor){
        auto* human=dynamic_cast<Human*>(&actor);
        if(!human)throw std::runtime_error("Scene2 Human frame summary has a different actual actor family");
        return game::summarize_frame_human(*human,*this);
    }
    bool human_unavailable(const Human& human)const { //005A6380, same actual B0/health/kind gates.
        std::int32_t health;const auto bits=human.properties.stats_ec[0];std::memcpy(&health,&bits,4);
        if(health<1||!human.loading_state||human.properties.field19c==1)return false;
        return (human.loading_state->words().template get<std::uint32_t>(0x50)&0x4000000u)!=0;
    }
private:
    Dependencies& d_;Movement& movement_;Derived& derived_;
};
} //namespace hd2::host
#endif
