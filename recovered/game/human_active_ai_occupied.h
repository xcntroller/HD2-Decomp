#ifndef HD2_GAME_HUMAN_ACTIVE_AI_OCCUPIED_H
#define HD2_GAME_HUMAN_ACTIVE_AI_OCCUPIED_H
#include "game/human_active_ai_authority_body.h"
#include "game/human_packet_commands.h"
namespace hd2::game {
inline long double active_mounted_length(long double square){
    const float stored=static_cast<float>(square);std::int32_t bits;std::memcpy(&bits,&stored,4);
    return bits<0x322bcc77?0.L:std::abs(static_cast<long double>(stored)-1.f)<9.99999993922529e-09f?1.L:std::sqrt(static_cast<long double>(stored));
}

//0052C510: actual mounted origin is queried twice, surrounding target callbacks.
//Raw bytes recover destination/lead/random temporaries lost in the C export.
template<class AI,class Actor,class Frames,class World>HumanVector active_mounted_actor_direction(AI& ai,Actor& actor,Frames& frames,World& world){
    const auto matrix=frames.actor_world_matrix(actor.base().frame());HumanVector point{{matrix[12],matrix[13],matrix[14],matrix[15]}};
    auto* human=ai.human();if(!human||!human->field160)throw std::logic_error("Mounted target requires actual Human attachment");
    const auto origin=world.human_emplacement_position(human->field160,human->field164);
    switch(actor.base().type()){
    case 3:{
        auto& target=world.human_ai_target_human(actor);
        if(!(world.human_script_target_flags(target)&1u)){
            const auto head=frames.actor_world_matrix(target.head_marker);point={{head[12],static_cast<float>(static_cast<long double>(head[13])-.30000001192092896f),head[14],head[15]}};
        }
        const auto velocity=target.activation.vector1b0;
        for(unsigned axis=0;axis!=3;++axis){const float lead=static_cast<float>(static_cast<long double>(velocity[axis])*.125f);point[axis]=static_cast<float>(static_cast<long double>(lead)+point[axis]);}
        const auto draw=[&]{const auto unit=static_cast<long double>(human_frame_signed_word(world.human_frame_random_word()))*3.0518509447574615e-05f;return unit+unit-1.f;};
        const float z=static_cast<float>(draw()),y=static_cast<float>(draw());const auto x=draw();
        const std::array<float,3> noise{{static_cast<float>(x*.10000000149011612f),static_cast<float>(static_cast<long double>(y)*.10000000149011612f),static_cast<float>(static_cast<long double>(z)*.10000000149011612f)}};
        for(unsigned axis=0;axis!=3;++axis){point[axis]=static_cast<float>(static_cast<long double>(noise[axis])+point[axis]);}
        break;
    }
    case 4:case 25:{
        point=frames.active_target_bound_centre(actor.base().frame());HumanVector delta{};
        for(unsigned axis=0;axis!=3;++axis)delta[axis]=static_cast<float>(static_cast<long double>(point[axis])-origin[axis]);
        const auto square=(static_cast<long double>(delta[0])*delta[0]+static_cast<long double>(delta[1])*delta[1])+static_cast<long double>(delta[2])*delta[2];
        const auto distance=active_mounted_length(square);
        point[1]=static_cast<float>(distance*.004999999888241291f+point[1]);break;
    }
    case 30:{const auto bone=frames.actor_find(actor.base().frame(),world.active_dog_target_bone(),0xffffu);
        if(!bone)throw std::runtime_error("Mounted Dog target requires actual native target bone");
        const auto head=frames.actor_world_matrix(bone);point={{head[12],head[13],head[14],head[15]}};break;}
    case 34:case 36:point=world.active_special_mounted_target(ai,actor,point,origin);break;
    default:break; //Native switch retains the initial model position.
    }
    human=ai.human();if(!human||!human->field160)throw std::logic_error("Mounted target lost late actual attachment");
    const auto late=world.human_emplacement_position(human->field160,human->field164);HumanVector direction=point;
    for(unsigned axis=0;axis!=3;++axis)direction[axis]=static_cast<float>(static_cast<long double>(point[axis])-late[axis]);
    const auto square=(static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[1])*direction[1])+static_cast<long double>(direction[2])*direction[2];
    normalize_human_ai_shot(direction,square);return direction;
}
//00532910: non-Human aim becomes a vector; Human aim retains the actual actor.
template<class AI,class Actor,class Frames,class World>void bind_active_mounted_target(AI& ai,Actor& actor,Frames& frames,World& world){
    const auto direction=active_mounted_actor_direction(ai,actor,frames,world);
    if(actor.base().type()==3)ai.resources().target().bind_actor(&actor);else ai.resources().target().bind_vector(direction);
    ai.resources().direction().set_target(direction);ai.resources().direction().set_current(direction);
    const bool precise=world.active_mounted_precise(*ai.human());auto& words=ai.resources().target().state;
    words.set(0x3c,precise?2000u:1500u);words.set(0x40,1000u);words.set(0x44,precise?2000u:300u);words.set(0x48,precise?2000u:400u);
}
//00532BA0. All three timers publish before task/target callbacks. Controlled
//mounts share the actual target/direction owners with the on-foot policy.
template<class AI,class Inline,class Frames,class World>void advance_active_mounted_target_policy(AI& ai,std::uint32_t elapsed,Inline& control,Frames& frames,World& world){
    auto* human=ai.human();if(!human||!human->field160)throw std::logic_error("Mounted policy lost actual attachment");
    auto& state=ai.state();const auto validate=world.human_common_receiver_guard(*human);
    const auto decrement=[&](unsigned offset){const auto old=state.template get<std::uint32_t>(offset);if(old){const auto next=old-elapsed;state.set(offset,next);if(human_frame_signed_word(next)<0)state.set(offset,0u);}};
    decrement(0x198);decrement(0x1a0);decrement(0x19c);
    const auto bind=[&](auto& actor){bind_active_mounted_target(ai,actor,frames,world);validate();};
    if(human->field160->base().type()==34){
        if(!state.template get<std::uint32_t>(0x1ac)){if(!(state.template get<std::uint32_t>(0x174)&4u))world.active_idle_look(ai);else state.set(0x23c,0u);state.set(0x1ac,1000u);}
        else decrement(0x1ac);
        if(auto* actor=world.active_mounted_idle_actor(ai))bind(*actor);
        return;
    }
    if(state.template get<std::uint32_t>(0x174)&2u)return;
    if(!state.template get<std::uint8_t>(0x202)){
        const auto task=state.template get<std::uint32_t>(0x220);
        if(task==25||task==26){
            ai.require_live_task_target();if(!ai.task_target)throw std::runtime_error("Mounted task dereferences native NULL224");
            if(world.active_mounted_task_health(*ai.task_target)<1){for(auto offset:{0x206u,0x204u,0x202u})state.set(offset,std::uint8_t{1});}
            else{const bool hostile=world.active_event_hostile(*human,ai.task_target);state.set(0x201,std::uint8_t(hostile));
                if(hostile){state.set(0x230,std::uint8_t{0});bind(*ai.task_target);return;}}
        }else if(task==22){
            ai.require_live_task_target();if(!ai.task_target)throw std::runtime_error("Mounted task22 dereferences native NULL224");
            if(world.active_task_actor_unavailable(*ai.task_target)){for(auto offset:{0x202u,0x206u,0x204u})state.set(offset,std::uint8_t{1});}
            else{const bool visible=world.active_mounted_task_visible(*ai.task_target,*human);state.set(0x201,std::uint8_t{0});
                if(visible){state.set(0x230,std::uint8_t{0});bind(*ai.task_target);return;}}
        }
    }
    if(state.template get<std::uint32_t>(0x174)&0x10u){
        const auto own=human->base().world_position();
        if(!state.template get<std::uint32_t>(0x194)){ai.saved_frame_position(false,own);validate();state.set(0x230,std::uint8_t{0});return;}
        const auto direction=active_saved_position_direction(state.template get<HumanVector>(0x140),own);
        if(auto* actor=select_active_movement_target(ai,frames,world,&direction))ai.resources().target().bind_actor(actor);else ai.resources().target().bind_vector(direction);
        state.set(0x230,std::uint8_t{0});return;
    }
    using Actor=std::remove_reference_t<decltype(*ai.task_target)>;
    Actor* selected=select_human_hostile<std::remove_reference_t<decltype(*human)>>(ai,frames,world);validate();
    if(!selected)selected=world.active_process_target();
    if(!selected)selected=world.active_mounted_dog_target(ai,20.f);
    if(selected&&!world.active_mounted_actor_excluded(*selected)){
        bool admit=false;
        if(ai.has_pending_orders_loading()){
            const auto flags=ai.words().template get<std::uint32_t>(0x50);
            if(!(flags&0x400u))admit=!(flags&0x200u)||active_cautious_attacker(ai,true,world);
        }else if(!(state.template get<std::uint32_t>(0x174)&6u)){const auto mode=ai.active_event_mode();admit=mode!=0&&mode!=2;}
        if(admit){state.set(0x230,std::uint8_t{0});state.set(0x19c,5000u);bind(*selected);return;}
    }
    ai.resources().target().bind_actor(nullptr);
    if(advance_active_cover_decision(ai,frames,world)){state.set(0x230,std::uint8_t{0});return;}
    if(state.template get<std::uint32_t>(0x198))return;
    HumanVector direction{};std::uint32_t timer;
    if(human_frame_signed_word(world.human_frame_random_word())%3==0){
        auto matrix=human_identity();const HumanVector up{{0,1,0,0}};
        const float roll=static_cast<float>(static_cast<long double>(world.human_sound_random())*6.2831854820251465f);
        frames.human_set_direction_roll(matrix,up,roll);direction={{matrix[4],matrix[5],matrix[6],matrix[7]}};
        timer=human_packet_ftol(static_cast<long double>(world.human_sound_random())*2000.f+8000.f);state.set(0x230,std::uint8_t{0});
    }else{
        const auto matrix=frames.actor_world_matrix(human->base().frame());direction={{matrix[8],matrix[9],matrix[10],matrix[11]}};
        normalize_human_ai_shot(direction,(static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[1])*direction[1]);
        state.set(0x160,direction);timer=human_packet_ftol(static_cast<long double>(world.human_sound_random())*2000.f+10000.f);state.set(0x230,std::uint8_t{1});
    }
    state.set(0x198,timer);ai.resources().direction().set_target(direction);ai.resources().direction().set_current(direction);
    (void)control;
}
//00527FD0, reached controlled Car/StaticWeapon. The return is always AL=1;
//reload/fire writes use the SAME first/second frame input buffers as ActorC0.
template<class AI,class Inline,class Frames,class World>void advance_active_mounted_authority(AI& ai,std::uint32_t elapsed,Inline& control,Frames& frames,World& world){
    auto* human=ai.human();if(!human)throw std::logic_error("Mounted authority lost Human44");
    ai.look_command(human->perception.lists()[0].empty()?3u:0u,control);
    advance_active_mounted_target_policy(ai,elapsed,control,frames,world);
    auto& target=ai.resources().target();auto desired=target.state.template get<HumanVector>(0x10);
    const auto square=(static_cast<long double>(desired[2])*desired[2]+static_cast<long double>(desired[1])*desired[1])+static_cast<long double>(desired[0])*desired[0];
    if(!target.actor&&square<1.0000000168623835e-16f)return;
    if(target.actor)desired=active_mounted_actor_direction(ai,*target.actor,frames,world);
    const auto current=ai.target_direction(); //Native virtual4C copy precedes desired normalization.
    (void)current;
    normalize_human_ai_shot(desired,(static_cast<long double>(desired[1])*desired[1]+static_cast<long double>(desired[2])*desired[2])+static_cast<long double>(desired[0])*desired[0]);
    ai.resources().direction().set_target(desired);ai.resources().direction().set_current(desired);
    human=ai.human();if(!human||!human->field160)throw std::logic_error("Mounted firing lost actual attachment");
    const auto threshold=world.active_mounted_fire_threshold(*human);if(!threshold)return;
    auto* first=ai.first_frame_inputs();if(!first)throw std::logic_error("Mounted authority requires first74 input");
    if(!world.human_emplacement_ammunition(human->field160,static_cast<std::int32_t>(human->field164))){first->set(0x34,1.f);return;}
    const auto actual=world.human_pose_actor_direction(human->field160,human->field164);
    const auto dot=(static_cast<long double>(desired[1])*actual[1]+static_cast<long double>(desired[2])*actual[2])+static_cast<long double>(desired[0])*actual[0];
    if(!(dot>*threshold))return;
    first->set(0x18,1.f);auto* second=ai.second_frame_inputs();if(!second)throw std::logic_error("Mounted firing requires second78 input");second->set(0x18,1.f);
}
}
#endif
