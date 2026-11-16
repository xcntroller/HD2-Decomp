#ifndef HD2_GAME_HUMAN_ACTIVE_AI_TARGET_MODES_H
#define HD2_GAME_HUMAN_ACTIVE_AI_TARGET_MODES_H
#include "game/human_animation_requests.h"
#include "game/human_sight_relations.h"
#include "game/human_joint_owners.h"
#include "game/human_state_frame_timers.h"
#include <cmath>
namespace hd2::game {
inline HumanVector active_target_unit(HumanVector value,long double extended){
    constexpr float epsilon=9.99999993922529e-9f;
    if(!(std::abs(extended-1.f)<epsilon)){
        const float square=static_cast<float>(extended);
        if(!(square<epsilon)){const auto scale=1.L/std::sqrt(static_cast<long double>(square));for(unsigned n=0;n!=3;++n)value[n]=static_cast<float>(static_cast<long double>(value[n])*scale);}
        else if(value[0]!=0)value[0]=value[0]<0?-1.f:1.f;
        else if(value[2]!=0)value[2]=value[2]<0?-1.f:1.f;
        else value[1]=value[1]<0?-1.f:1.f;
    }
    return value;
}
//005382D0: disabling does not age/clear194 or change saved140. Enabling
//executes mode16 before flags/random; interval and look precede second mode6.
template<class AI,class World>void set_active_saved_position_mode(AI& ai,bool enabled,const HumanVector& position,World& world){
    auto& state=ai.state();
    if(!enabled){state.set(0x174,state.template get<std::uint32_t>(0x174)&~0x10u);ai.resources().target().bind_actor(nullptr);ai.refresh_pending_action_mode();return;}
    const auto validate=world.human_common_receiver_guard(*ai.human());
    ai.active_event_action_mode(16);validate();state.set(0x174,(state.template get<std::uint32_t>(0x174)&~4u)|0x10u);
    const auto timer=static_cast<std::uint32_t>(human_animation_time_integer(static_cast<long double>(world.human_sound_random())*2000.f+8000.f));
    state.set(0x194,timer);state.set(0x19c,timer+1000u);state.set(0x140,position);
    ai.resources().direction().look_interval(false,4000u);ai.look_command(0,world.human_inline(*ai.human()));validate();ai.pending_second_mode(6);
}
//0052B810 timed saved-position direction. All differences spill to FLOAT;
//Y subtracts one before its spill. W is the copied native Human position W.
inline HumanVector active_saved_position_direction(const HumanVector& saved,const HumanVector& own){
    HumanVector value{{static_cast<float>(static_cast<long double>(saved[0])-own[0]),static_cast<float>((static_cast<long double>(saved[1])-own[1])-1.f),static_cast<float>(static_cast<long double>(saved[2])-own[2]),own[3]}};
    const auto square=(static_cast<long double>(value[2])*value[2]+static_cast<long double>(value[1])*value[1])+static_cast<long double>(value[0])*value[0];
    return active_target_unit(value,square);
}
//0052A810: retained launcher admission differs from0052AC20 crouch. A
//vector has a genuine no-ray return; actor targets use their actual head frame.
template<class AI,class Frames,class World>bool active_launcher_target_clear(AI& ai,Frames& frames,World& world){
    auto& target=ai.resources().target();auto delta=target.state.template get<HumanVector>(0x10);
    const auto empty=(static_cast<long double>(delta[0])*delta[0]+static_cast<long double>(delta[1])*delta[1])+static_cast<long double>(delta[2])*delta[2]<1.e-16f; //0052ABE0.
    if(!target.actor&&empty)return false;
    const auto own=frames.actor_world_matrix(ai.human()->base().frame());HumanVector origin{{own[12],own[13],own[14],own[15]}};
    if(target.actor){const auto model=frames.actor_world_matrix(target.actor->base().frame());for(unsigned n=0;n!=3;++n)delta[n]=static_cast<float>(static_cast<long double>(model[12+n])-origin[n]);}
    const float xx=static_cast<float>(static_cast<long double>(delta[0])*delta[0]),zz=static_cast<float>(static_cast<long double>(delta[2])*delta[2]);
    const float square=static_cast<float>((static_cast<long double>(delta[1])*delta[1]+zz)+xx);std::uint32_t bits;std::memcpy(&bits,&square,4);
    const auto length=human_frame_signed_word(bits)<0x322bcc77?0.L:std::abs(static_cast<long double>(square)-1.f)<9.99999993922529e-9f?1.L:std::sqrt(static_cast<long double>(square));
    if(length<0||std::abs(static_cast<long double>(delta[1])/length)>.17364799976348877)return false;
    const auto kind=world.active_follow_item_kind(ai.item());if(kind==0||kind==2||kind==6||kind==7||kind==9)return false;
    if(world.active_follow_sector(*ai.human())!=world.active_follow_root_sector())return false;
    long double x=delta[0];float y=0,z=delta[2];const auto horizontal=static_cast<long double>(zz)+xx;
    if(!(std::abs(horizontal-1.f)<9.99999993922529e-9f)){
        const float stored=static_cast<float>(horizontal);
        if(!(stored<9.99999993922529e-9f)){
            const auto scale=1.L/std::sqrt(static_cast<long double>(stored));x*=scale;
            const float spill=static_cast<float>(scale);y=static_cast<float>(static_cast<long double>(spill)*y);z=static_cast<float>(static_cast<long double>(z)*spill);
        }else if(x!=0)x=x<0?-1.f:1.f;else if(z!=0)z=z<0?-1.f:1.f;else y=1.f;
    }
    //52AAEB spills inverse for Y/Z but retains extended inverse*X through
    //the origin addition. Z*.5 spills separately before its addition.
    const float offset_z=static_cast<float>(static_cast<long double>(z)*.5f);
    origin[0]=static_cast<float>(x*.5f+origin[0]);origin[1]=static_cast<float>((static_cast<long double>(origin[1])+.15000000596046448f)+static_cast<long double>(y)*.5f);origin[2]=static_cast<float>(static_cast<long double>(offset_z)+origin[2]);
    if(!target.actor)return true;
    auto& human=world.human_ai_target_human(*target.actor);if(!human.head_marker)throw std::runtime_error("Active launcher requires actual target260 head frame");
    const auto head=frames.actor_world_matrix(human.head_marker);for(unsigned n=0;n!=3;++n)delta[n]=static_cast<float>(static_cast<long double>(head[12+n])-origin[n]);
    return !world.human_geometry_ray(origin,delta,3u,0u,false);
}
//00536A30, admitted close target. D8 executes the actual shared effect before
//its queued copy; ordinary strike uses SAME inline action8. Sound/cooldown and
//alignment run even when D8's shared effect returned false.
template<class AI,class Actor,class Frames,class World>void advance_active_close_target(AI& ai,Actor& target,Frames& frames,World& world){
    if(ai.words().template get<std::uint32_t>(0x50)&1u)return;
    auto* human=ai.human();const auto validate=world.human_common_receiver_guard(*human);
    const auto current=[&]() -> decltype(auto){if(!ai.item())throw std::runtime_error("Active close target requires actual cached item");return world.human_frame_output_item(ai.item());};
    auto& first=current();if(world.game_word28()<first.field14)return;
    auto& second=current();if(human_frame_signed_word(world.game_word28()-second.field58)<0)return;
    const auto direction=ai.current_direction();bool stealth=false;
    if(target.base().type()==3){
        const auto matrix=frames.actor_world_matrix(target.base().frame());validate();
        long double x=matrix[8],y=matrix[9];float z=matrix[10];const auto extended=(y*y+static_cast<long double>(z)*z)+x*x;
        if(!(std::abs(extended-1.f)<9.99999993922529e-9f)){
            const float square=static_cast<float>(extended);
            if(!(square<9.99999993922529e-9f)){const auto scale=1.L/std::sqrt(static_cast<long double>(square));x*=scale;y*=scale;z=static_cast<float>(static_cast<long double>(z)*scale);}
            else if(x!=0)x=x<0?-1.f:1.f;else if(z!=0)z=z<0?-1.f:1.f;else y=y<0?-1.f:1.f;
        }
        if((static_cast<long double>(z)*direction[2]+y*direction[1])+x*direction[0]>.800000011920929f){ai.emit_common_action(0x24u,target.base().id10(),0u);validate();stealth=true;}
    }
    if(!stealth){world.human_inline(*human).set_frame_item_action(8);validate();}
    const bool silent=(world.mode_flags()&7u)!=7u;const auto model=current().model();validate();world.play_player_item_sound(current(),*human,model,silent);validate();
    auto& item=current();const auto interval=item.ai_shot_interval();item.field58=world.game_word28()+interval;
    set_human_sight_alignment(*human,-10.f,world.mode_flags());
}
} //namespace hd2::game
#endif
