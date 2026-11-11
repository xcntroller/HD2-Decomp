#ifndef HD2_GAME_HUMAN_LADDER_CONTROL_H
#define HD2_GAME_HUMAN_LADDER_CONTROL_H

#include "game/human_animation_tracks.h"
#include "game/human_state_frame_timers.h"
#include <cmath>

namespace hd2::game {

template<class Physical,class Look,class Geometry,class Sweep,class Notify>
bool probe_human_ladder_down(Physical& physical,const Look& look,const Geometry& ladder,
        Sweep& sweep,bool remote,bool offline,Notify& notify){ //0074BBE0.
    if(remote)return true;
    physical.rebind_frame_posture(look.state().template get<std::uint32_t>(0x4c));
    physical.prepare_ladder_secondary();
    struct Restore {Physical& physical;bool armed=true;~Restore(){if(armed)physical.restore_query(false);}} restore{physical};
    const auto sphere=physical.posture_spheres()[0];
    if(sphere[1]<static_cast<long double>(ladder.up_position()[1])-.3f)return false;
    physical.exclude_query(false);
    const auto& direction=ladder.direction();
    const HumanVector displacement{{
        static_cast<float>(-static_cast<long double>(direction[0])*sphere[3]+(static_cast<long double>(ladder.up_position()[0])-sphere[0])),
        -direction[1]*sphere[3],
        static_cast<float>((static_cast<long double>(ladder.up_position()[2])-sphere[2])-static_cast<long double>(direction[2])*sphere[3]),0}};
    HumanVector origin=sphere;origin[1]=sphere[1]+.1f;
    const float radius=sphere[3]-.1f;
    if(sweep.sweep(origin,displacement,radius,0x8c002u,0xffffffffu))return false;
    HumanVector destination=sphere;
    for(unsigned axis=0;axis!=3;++axis)destination[axis]=sphere[axis]+displacement[axis];
    HumanVector traversal{};
    for(unsigned axis=0;axis!=3;++axis)traversal[axis]=ladder.down_position()[axis]-ladder.up_position()[axis];
    const auto square=(static_cast<long double>(traversal[0])*traversal[0]+
        static_cast<long double>(traversal[1])*traversal[1])+static_cast<long double>(traversal[2])*traversal[2];
    normalize_human_ai_shot(traversal,square);
    for(unsigned axis=0;axis!=3;++axis)traversal[axis]*=1.8f;
    if(sweep.sweep(destination,traversal,radius,0x8c002u,0xffffffffu))return false;
    HumanVector step{};
    for(unsigned axis=0;axis!=3;++axis){destination[axis]+=traversal[axis];step[axis]=-traversal[axis]*sphere[3];}
    physical.publish_ladder_secondary(destination,step);
    physical.restore_query(false);restore.armed=false; //Original notification follows exclusion restoration.
    if(!offline)notify(4u); //005A9680→actual B0 virtualF8.
    return true;
}

template<class Physical,class Geometry,class Human,class Services>
bool probe_human_ladder_up(Physical& physical,const Geometry& ladder,Human& human,
        Services& services){ //0074C1E0.
    HumanVector traversal{};
    for(unsigned axis=0;axis!=3;++axis)traversal[axis]=ladder.up_position()[axis]-ladder.down_position()[axis];
    const auto square=(static_cast<long double>(traversal[1])*traversal[1]+
        static_cast<long double>(traversal[0])*traversal[0])+static_cast<long double>(traversal[2])*traversal[2];
    normalize_human_ai_shot(traversal,square);
    HumanVector origin{},displacement{};
    for(unsigned axis=0;axis!=3;++axis){
        traversal[axis]*=1.8f;
        origin[axis]=static_cast<float>(static_cast<long double>(traversal[axis])-
            static_cast<long double>(ladder.direction()[axis])*.25f+ladder.down_position()[axis]);
        displacement[axis]=-traversal[axis];
    }
    bool blocked=false;
    {
        struct Restore {Physical& physical;~Restore(){physical.restore_query(false);}} restore{physical};
        physical.exclude_query(false);
        blocked=bool(services.sight_ray(origin,displacement,0x14002u,0xffffffffu)); //Actual007183D0 segment query.
    }
    if(blocked)return false;
    if(services.human_is_player(human)){
        const auto& position=human.base().world_position();
        HumanVector toward{{ladder.down_position()[0]-position[0],0,ladder.down_position()[2]-position[2],0}};
        auto direction=ladder.direction();direction[1]=0;
        const auto toward_square=static_cast<long double>(toward[0])*toward[0]+static_cast<long double>(toward[2])*toward[2];
        const auto direction_square=static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[2])*direction[2];
        normalize_human_ai_shot(toward,toward_square);normalize_human_ai_shot(direction,direction_square);
        if((static_cast<long double>(direction[1])*toward[1]+static_cast<long double>(direction[0])*toward[0])+
                static_cast<long double>(direction[2])*toward[2]<0)return false;
    }
    return true;
}

template<class Movement,class Human>std::int32_t human_ladder_turn_delay(const Movement& movement,
        const Human& human,const HumanVector& target,bool player){ //0076CE30, actual AC0+8C.
    const auto& current=human.base().world_direction();
    auto angle=std::atan2(static_cast<long double>(current[2]),static_cast<long double>(current[0]))-
        std::atan2(static_cast<long double>(target[2]),static_cast<long double>(target[0])); //007885B0.
    if(angle< -3.1415927410125732f)angle+=6.2831854820251465f;
    if(angle>3.1415927410125732f)angle-=6.2831854820251465f;
    const auto stored=static_cast<float>(angle);
    const auto rate=player?.006f:movement.state().template get<float>(0x8c);
    const auto delay=std::abs(static_cast<long double>(stored)/rate)*(player?1.L:2.5L);
    if(!std::isfinite(delay)||delay>=0x1p63L)return 0; //Native __ftol indefinite low DWORD.
    return human_frame_signed_word(static_cast<std::uint32_t>(static_cast<std::int64_t>(delay)));
}

template<class Control,class Look,class Human,class Services,class Runtime,class Publish>
std::int32_t begin_human_frame_ladder(Control& control,Look& look,Human& human,
        Services& services,Runtime& runtime,Publish& publish){ //0074B4F0, actual AA0 five-link owner.
    auto& physical=control.physical();auto& state=physical.control_frame_words();
    auto& movement=control.movement();auto& movement_words=movement.frame_words();
    auto& ladder=services.human_ladder(physical.control_action_actor());
    const auto& geometry=*ladder.geometry;
    const auto position=human.base().world_position();
    HumanVector to_up{},to_down{};
    for(unsigned axis=0;axis!=3;++axis){
        to_up[axis]=position[axis]-geometry.up_position()[axis];
        to_down[axis]=position[axis]-geometry.down_position()[axis];
    }
    const auto up_square=(static_cast<long double>(to_up[2])*to_up[2]+
        static_cast<long double>(to_up[1])*to_up[1])+static_cast<long double>(to_up[0])*to_up[0];
    const auto down_square=(static_cast<long double>(to_down[2])*to_down[2]+
        static_cast<long double>(to_down[1])*to_down[1])+static_cast<long double>(to_down[0])*to_down[0];
    const bool ascend=down_square<=up_square; //0074B7AC compares down distance against up distance.
    const auto queue_animation=[&](std::int32_t animation){
        state.set(0x988,2u);state.set(0x1fc,animation);normalize_human_primary_animation(state,0x1fc);
    };
    const auto clear_fall=[&]{movement_words.set(0xf4,0.f);movement_words.set(0xf8,0.f);};
    auto& equipment=services.human_presentation_equipment(human);
    const bool offline=(control.mode_flags()&7u)==7u;
    const bool remote=!offline&&!control.human_is_player()&&!control.animation_authority_actor();
    if(remote){
        look.control_movement_mode(0,true,publish);
        state.set(0x988,2u);
        if(ascend){queue_animation(106);look.frame_movement_animation(106,publish);equipment.set_hidden(true);}
        clear_fall();return 1;
    }
    if(state.template get<std::int32_t>(0x988)!=-10000)return 1;
    const auto detail=state.template get<std::int32_t>(0x1e8);
    if(state.template get<std::int32_t>(0xa30)!=-10000||(detail>=80&&detail<=82))return -1;
    state.set(0x988,std::int32_t{-10000});
    const auto forced=[&](auto&& operation){
        look.force_frame_commands(true);
        try{operation();}catch(...){look.force_frame_commands(false);throw;}
        look.force_frame_commands(false);
    };
    const auto clear_velocity=[&]{
        HumanVector output{};control.tracks().motion_velocity({{0,0,0,0}},0,0,output);
        movement.control_frame_track_velocity(output);
    };
    if(ascend){
        if(static_cast<long double>(9.81f)*1.1f<movement_words.template get<float>(0xf4))return 0;
        if(!probe_human_ladder_up(physical,geometry,human,services))return 0;
        queue_animation(106);
        forced([&]{look.frame_movement_animation(106,publish);});
        equipment.set_hidden(true);
        clear_velocity();clear_velocity();
        control.tracks().clear_motion_displacement();
    }else{
        auto notify=[&](std::uint32_t kind){runtime.notify_human_ladder_probe(human,kind);}; //005A9680→B0 virtualF8, nonoffline only.
        if(!probe_human_ladder_down(physical,look,geometry,services.human_geometry_sweep(),remote,offline,notify))return 0;
        queue_animation(8);
        forced([&]{
            look.frame_movement_animation(8,publish);look.control_frame_animation_lock(true);
            look.position_command({{0,0,0,0}});look.frame_posture(0,publish);
        });
        clear_velocity();control.tracks().clear_motion_displacement();
        auto next=geometry.up_position();
        next[1]=std::max(next[1],human.base().world_position()[1]);
        for(unsigned axis=0;axis!=3;++axis)
            next[axis]=static_cast<float>(static_cast<long double>(geometry.direction()[axis])*.1f+next[axis]);
        human.base().world_position(next);
        equipment.set_hidden(true);
    }
    clear_fall();
    state.set(0x998,human_ladder_turn_delay(movement,human,geometry.direction(),control.human_is_player()));
    if(look.state().template get<std::uint32_t>(0xb8)==5){
        look.control_movement_mode(4,false,publish);
        control.tracks().secondary_motion_impulse({{0,-.6f,0,0}},500);
    }
    forced([&]{look.current_direction(geometry.direction());});
    look.control_movement_mode(0,true,publish);
    ladder.add_occupant(human);
    return 1;
}

template<class Human,class Geometry>void project_human_ladder_position(const Human& human,
        const Geometry* ladder,HumanVector& output){ //007709C0 reads the actual ActorBase position, independently of output.
    const auto& position=human.base().world_position();
    if(ladder){
        auto height=position[1];
        if(height>ladder->up_position()[1])height=ladder->up_position()[1];
        else if(height<ladder->down_position()[1])height=ladder->down_position()[1];
        const float fraction=static_cast<float>((static_cast<long double>(height)-ladder->down_position()[1])/
            (static_cast<long double>(ladder->up_position()[1])-ladder->down_position()[1]));
        const float x=static_cast<float>((position[0]-((static_cast<long double>(ladder->up_position()[0])-
            ladder->down_position()[0])*fraction+ladder->down_position()[0]))+static_cast<long double>(ladder->direction()[0])*.17f);
        const float z=static_cast<float>((position[2]-((static_cast<long double>(ladder->up_position()[2])-
            ladder->down_position()[2])*fraction+ladder->down_position()[2]))+static_cast<long double>(ladder->direction()[2])*.17f);
        if(static_cast<long double>(x)*x+static_cast<long double>(z)*z!=0){
            output=position;output[0]=position[0]-x;output[2]=position[2]-z;return;
        }
    }
    output=position;
}

template<class Control,class Look,class Human,class Services,class Runtime,class Publish>
void advance_human_ladder_motion(Control& control,Look& look,Human& human,Services& services,
        std::int32_t elapsed,Runtime& runtime,Publish& publish){ //00768230, SAME AC0 movement and DD0 actor borrower.
    auto position=human.base().world_position();
    if((control.mode_flags()&7u)!=7u&&!control.human_is_player()&&!control.animation_authority_actor())return;
    control.prepare_motion_velocity(look,true); //0076B670, actual shared animation velocity selector.
    auto& physical=control.physical();auto& state=physical.control_frame_words();
    auto& movement=control.movement().frame_words();
    const auto displacement=control.tracks().words().template get<HumanVector>(0x270);
    movement.set(0x10,displacement);state.set(0x228,0u);
    auto& ladder=services.human_ladder(physical.control_action_actor());
    const auto& geometry=*ladder.geometry;
    const auto direction=state.template get<std::int32_t>(0x998)==-10000?human.base().world_direction():geometry.direction();
    HumanVector offset{};for(unsigned axis=0;axis!=3;++axis)offset[axis]=-direction[axis]*.05f;
    const auto animation=state.template get<std::int32_t>(0x1d8);
    const auto phase=control.tracks().words().template get<std::int32_t>(0x14);
    const auto near_top=[&]{
        HumanVector delta{};for(unsigned axis=0;axis!=3;++axis)delta[axis]=position[axis]-geometry.up_position()[axis];
        const float square=static_cast<float>((static_cast<long double>(delta[0])*delta[0]+
            static_cast<long double>(delta[1])*delta[1])+static_cast<long double>(delta[2])*delta[2]);
        const auto length=square<1.e-8f?0.L:std::abs(static_cast<long double>(square)-1)<1.e-8f?1.L:std::sqrt(static_cast<long double>(square));
        return length<1.7f;
    };
    const auto finish_top=[&]{
        state.set(0x1fc,10u);normalize_human_primary_animation(state,0x1fc);
        if(!runtime.prepare_human_ladder_exit(control)){ //0074A690, actual queued primary descriptor.
            state.set(0x1fc,206u);normalize_human_primary_animation(state,0x1fc);return false;
        }
        look.control_movement_mode(0,false,publish);state.set(0x988,std::int32_t{-10000});
        look.frame_movement_animation(206,publish,false);
        services.human_presentation_equipment(human).set_hidden(false);ladder.remove_occupant(human);
        return true;
    };
    if((animation>=106&&animation<=108)&&phase<1){
        const bool ascending=animation==106;
        const auto requested=look.state().template get<std::int32_t>(0x44);
        if((requested>=106&&requested<=108)||requested==8||requested==14)
            look.frame_movement_animation(ascending?190:191,publish);
        const auto hit=[&]{
            struct Restore {decltype(physical)& owner;~Restore(){owner.restore_query(false);}} restore{physical};
            physical.exclude_query(false);
            physical.rebind_frame_posture(look.state().template get<std::uint32_t>(0x4c));
            const auto count=physical.primary_count();
            if(count<1)throw std::runtime_error("Ladder movement requires its actual posture spheres");
            const auto& sphere=physical.posture_spheres().at(ascending?static_cast<std::size_t>(count-1):0u);
            HumanVector origin=sphere;for(unsigned axis=0;axis!=3;++axis)origin[axis]=sphere[axis]+offset[axis];
            return services.human_geometry_sweep().sweep(origin,displacement,sphere[3]-.05f,0xcc002u,0xffffffffu);
        }();
        if(hit){runtime.resolve_human_ladder_contact(control,*hit);return;} //00788840 with one actual contact.
        for(unsigned axis=0;axis!=3;++axis)position[axis]+=displacement[axis];
        if(ascending){if(near_top()&&!finish_top())return;}
        else if(position[1]<geometry.down_position()[1]){
            if((control.mode_flags()&7u)!=7u)runtime.notify_human_ladder_probe(human,5u);
            position[1]=geometry.down_position()[1];
            state.set(0x988,std::int32_t{-10000});state.set(0x1fc,45u);normalize_human_primary_animation(state,0x1fc);
            state.set(0x98c,std::uint8_t{1});
            auto impulse=human.base().world_direction();
            const bool jump=ladder.properties.flag180!=0;
            for(unsigned axis=0;axis!=3;++axis)impulse[axis]*=jump?-1.25f:-.15f;
            if(jump)impulse[1]-=.25f;
            control.tracks().motion_impulse(impulse,200);
            look.control_movement_mode(0,false,publish);look.frame_movement_animation(45,publish,false);
            services.human_presentation_equipment(human).set_hidden(false);ladder.remove_occupant(human);
        }
        human.base().world_position(position);project_human_ladder_position(human,&geometry,position);
        human.base().world_position(position);movement.set(0x51,std::uint8_t{1});return;
    }
    const auto square=(static_cast<long double>(displacement[2])*displacement[2]+
        static_cast<long double>(displacement[1])*displacement[1])+static_cast<long double>(displacement[0])*displacement[0];
    if(!((square>=1.e-16f&&animation<190)||phase>0))return;
    for(unsigned axis=0;axis!=3;++axis)position[axis]+=displacement[axis];
    const auto queued=state.template get<std::int32_t>(0x1fc);
    if(near_top()&&animation==106&&(queued==106||queued==206)&&!finish_top())return;
    project_human_ladder_position(human,&geometry,position);
    look.force_frame_commands(true);
    try{look.current_direction(geometry.direction());}catch(...){look.force_frame_commands(false);throw;}
    look.force_frame_commands(false);
    const auto& previous=human.base().world_position();
    HumanVector delta{};for(unsigned axis=0;axis!=3;++axis)delta[axis]=position[axis]-previous[axis];
    const float stored=static_cast<float>((static_cast<long double>(delta[0])*delta[0]+
        static_cast<long double>(delta[1])*delta[1])+static_cast<long double>(delta[2])*delta[2]);
    const auto distance=stored<1.e-8f?0.L:std::abs(static_cast<long double>(stored)-1)<1.e-8f?1.L:std::sqrt(static_cast<long double>(stored));
    if(static_cast<long double>(elapsed)*.001f<distance)
        for(unsigned axis=0;axis!=3;++axis)
            position[axis]=static_cast<float>(previous[axis]+static_cast<long double>(delta[axis])/distance*.001f*static_cast<float>(elapsed));
    human.base().world_position(position);movement.set(0x51,std::uint8_t{1});
}

} //namespace hd2::game
#endif
