#ifndef HD2_GAME_HUMAN_AI_ALIGNMENT_H
#define HD2_GAME_HUMAN_AI_ALIGNMENT_H
#include "game/human_ai_target_output.h"
#include "game/human_ai_trigger_output.h"
#include "game/human_ai_grenade.h"
namespace hd2::game {
//00558E50: actual friendly278 list and physical6E0 posture spheres. The
//native test uses the infinite line, not a clamped segment; keep its200 scale.
template<class State,class Frames,class World>
bool human_ai_friendly_line_clear(State& state,const HumanVector& input,Frames& frames,World& world){
    constexpr float epsilon=9.99999993922529e-09f;
    HumanVector line{},unit=input;
    for(unsigned n=0;n!=3;++n)line[n]=static_cast<float>(static_cast<long double>(input[n])*200.f);
    normalize_human_ai_shot(unit,(static_cast<long double>(unit[1])*unit[1]+static_cast<long double>(unit[2])*unit[2])+static_cast<long double>(unit[0])*unit[0]);
    auto* human=state.human();if(!human||!human->head_marker)throw std::logic_error("Friendly aim test requires actual Human260");
    const auto matrix=frames.actor_world_matrix(human->head_marker);
    const HumanVector origin{{matrix[12],matrix[13],matrix[14],matrix[15]}};
    const auto& friends=human->perception.lists()[1];
    for(auto n=friends.size();n;){
        auto* actor=friends.at(--n);if(!actor)throw std::logic_error("Friendly aim list has null actor");
        auto& other=world.human_ai_target_human(*actor);
        if(human_frame_signed_word(other.properties.stats_ec[0])<=0||&other==human)continue;
        const auto current=frames.actor_world_matrix(other.base().frame());
        long double x=static_cast<float>(static_cast<long double>(current[12])-origin[0]);
        long double z=static_cast<float>(static_cast<long double>(current[14])-origin[2]);float y=0;
        const auto square=z*z+x*x;const float stored=static_cast<float>(square);
        if(!(std::abs(square-1)<epsilon)){
            if(!(stored<epsilon)){const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));x*=inverse;z*=inverse;y=static_cast<float>(0.L*inverse);}
            else if(x!=0)x=x<0?-1:1;
            else if(z!=0)z=z<0?-1:1;
            else y=1;
        }
        if(!((static_cast<long double>(unit[2])*z+static_cast<long double>(unit[1])*y)+static_cast<long double>(unit[0])*x>0))continue;
        std::int32_t count=0;const auto* spheres=world.human_ai_friendly_spheres(other,count);
        if(count<0||count>4||(!spheres&&count))throw std::logic_error("Friendly aim requires actual primary spheres");
        const float length=static_cast<float>((static_cast<long double>(line[2])*line[2]+static_cast<long double>(line[1])*line[1])+static_cast<long double>(line[0])*line[0]);
        while(count){
            const auto& sphere=spheres[--count];long double distance=-1.0000000272564224e16f; //00810BA8.
            if(!(length<epsilon)){
                const auto dx=static_cast<long double>(origin[0])-sphere[0];
                const auto dy=static_cast<long double>(origin[1])-sphere[1];
                const auto dz=static_cast<long double>(origin[2])-sphere[2];const float saved_z=static_cast<float>(dz);
                const auto parameter=((dz*line[2]+dy*line[1])+dx*line[0])/length;
                const float projected_y=static_cast<float>(static_cast<long double>(line[1])*parameter);
                const float projected_z=static_cast<float>(static_cast<long double>(line[2])*parameter);
                const float rx=static_cast<float>(dx-static_cast<long double>(line[0])*parameter);
                const auto ry=dy-projected_y,rz=static_cast<long double>(saved_z)-projected_z;
                const float residual=static_cast<float>((rz*rz+ry*ry)+static_cast<long double>(rx)*rx);
                std::uint32_t bits;std::memcpy(&bits,&residual,4);
                distance=human_frame_signed_word(bits)<0x322bcc77?0.L:
                    !(std::abs(static_cast<long double>(residual)-1)<epsilon)?std::sqrt(static_cast<long double>(residual)):1.L;
            }
            if(distance<sphere[3])return false;
        }
    }
    return true;
}
//00575D50. Geometry/target admission before input production. Clock gates
//read actual game28 separately; alignment keeps native two-pass normalization.
template<class State,class World>std::uint32_t human_ai_alignment_status(State& state,World& world){
    auto& resources=state.resources();auto& words=resources.state();
    const auto firing=[&]{return (state.words().template get<std::uint32_t>(0x50)&0x200000u)!=0;};
    if(!state.item()){if(firing())stop_human_ai_aim(state,400u,world);return 0x10u;}
    auto* entry=state.item();auto& first_item=world.human_frame_output_item(entry);
    const auto first_now=world.game_word28();if(first_now<first_item.field14)return 0x40u;
    entry=state.item();if(!entry)throw std::logic_error("AI alignment lost actual item");
    auto& second_item=world.human_frame_output_item(entry);
    const auto second_now=world.game_word28();if(human_frame_signed_word(second_now-second_item.field58)<0)return 0x40u;
    entry=state.item();if(!entry)throw std::logic_error("AI alignment lost actual item");
    auto& third_item=world.human_frame_output_item(entry);
    const auto third_now=world.game_word28();if(third_now<third_item.field5c)return 1u;
    if(words.template get<std::int32_t>(0xc8)>0)return 4u;
    if(firing()&&!words.template get<std::uint32_t>(0xc4)){
        stop_human_ai_aim(state,static_cast<std::uint32_t>(human_animation_time_integer(static_cast<long double>(world.human_sound_random())*400.f+300.f)),world);return 4u;
    }
    HumanVector desired{};
    const auto actor=[&]{auto* value=resources.target().actor;if(!value)throw std::logic_error("AI alignment lost actual target actor");return value;};
    if(!resources.target().actor)desired=resources.target().state.template get<HumanVector>(0x10);
    else{
        if(!state.item())throw std::logic_error("AI target alignment requires actual item");
        auto& item=world.human_frame_output_item(state.item());
        if(item.amount()!=0.f||world.human_ai_ammunition_count(state)!=0)desired=world.human_frame_actor_target_direction(state,*actor());
        else desired=world.human_frame_frame_target_direction(state,world.human_ai_actor_aim_frame(*actor()));
    }
    if(resources.direction().set_target(desired)==1)return 0x100u;
    if((world.mode_flags()&2u)&&firing()){
        const auto wanted=resources.direction().state().template get<HumanVector>(0x10);
        if(!state.item())throw std::logic_error("AI model alignment requires actual item");
        const auto model=world.human_frame_output_item(state.item()).model();
        if(model){
            const auto matrix=world.human_command_frames().actor_world_matrix(model);
            HumanVector forward{{matrix[8],matrix[9],matrix[10],matrix[11]}};
            normalize_human_ai_shot(forward,(static_cast<long double>(forward[1])*forward[1]+static_cast<long double>(forward[2])*forward[2])+static_cast<long double>(forward[0])*forward[0]);
            long double x=forward[0];float y=forward[1],z=forward[2];
            const auto square=(static_cast<long double>(y)*y+static_cast<long double>(z)*z)+x*x;
            constexpr float epsilon=9.99999993922529e-09f;const float stored=static_cast<float>(square);
            if(!(std::abs(square-1)<epsilon)){
                if(!(stored<epsilon)){
                    const float inverse=static_cast<float>(1.L/std::sqrt(static_cast<long double>(stored))); //576202 FLOAT spill.
                    x*=inverse;y=static_cast<float>(static_cast<long double>(y)*inverse);z=static_cast<float>(static_cast<long double>(z)*inverse);
                }else if(x!=0)x=x<0?-1:1;
                else if(z!=0)z=z<0?-1.f:1.f;
                else y=y<0?-1.f:1.f;
            }
            if((static_cast<long double>(z)*wanted[2]+static_cast<long double>(y)*wanted[1])+static_cast<long double>(wanted[0])*x<.95f)return 0x80u;
        }
    }
    if((!resources.target().actor||!world.human_sound_friendly(*state.human(),resources.target().actor))&&
       !world.human_ai_line_of_fire(state,resources.direction().state().template get<HumanVector>(0x10)))return 2u;
    if(resources.target().actor&&resources.target().actor->base().type()==3u&&
       human_frame_signed_word(world.human_ai_target_human(*resources.target().actor).properties.stats_ec[0])<=0){
        if(firing())stop_human_ai_aim(state,400u,world);
        return 0x1000u;
    }
    return 0u;
}
//005762B0, after trigger output. Reload/input production remains downstream.
template<class State,class World>std::uint32_t continue_human_ai_aim(State& state,World& world){
    auto& words=state.resources().state();
    if(!(words.template get<std::uint32_t>(0xd8)&1u)){
        if(words.template get<std::uint32_t>(0xc4))return 0u;
        stop_human_ai_aim(state,static_cast<std::uint32_t>(human_animation_time_integer(static_cast<long double>(world.human_sound_random())*400.f+300.f)),world);return 4u;
    }
    stop_human_ai_aim(state,world.human_ai_reload_time(state)+100u,world);
    if(state.words().template get<std::uint32_t>(0x48)==3u&&world.human_ai_ammunition_count(state)==0)return 0x800u;
    finish_human_ai_trigger(state);return 0x200u;
}
//00576350: grenade35 returns zero without publishing ordinary status38.
template<class State,class World>
std::uint32_t advance_human_ai_aim(State& state,std::uint32_t elapsed,World& world){
    auto& resources=state.resources();auto& target=resources.target();
    const auto publish=[&](std::uint32_t value){resources.target().state.set(0x38,value);return value;};
    if(target.state.template get<std::uint8_t>(0x35)){
        resources.state().set(0xe0,target.state.template get<HumanVector>(0x20));
        aim_human_ai_grenade(state,world);
        resources.target().state.set(0x35,std::uint8_t{0});return 0u;
    }
    if(target.state.template get<std::uint8_t>(0x34)){
        if(state.item()&&world.human_ai_ammunition_count(state)==0)return publish(0x800u);
        finish_human_ai_trigger(state);stop_human_ai_aim(state,world.human_ai_reload_time(state)+100u,world);
        resources.target().state.set(0x34,std::uint8_t{0});return publish(0x200u);
    }
    const auto delta=target.state.template get<std::array<float,3>>(0x10);
    const auto active_zero=[&]{return (static_cast<long double>(delta[0])*delta[0]+static_cast<long double>(delta[1])*delta[1])+static_cast<long double>(delta[2])*delta[2]<1.e-16f;}; //57643F..4F.
    const auto inactive_zero=[&]{return (static_cast<long double>(delta[2])*delta[2]+static_cast<long double>(delta[1])*delta[1])+static_cast<long double>(delta[0])*delta[0]<1.e-16f;}; //57657C..59F.
    const bool firing=(state.words().template get<std::uint32_t>(0x50)&0x200000u)!=0;
    if(firing){
        if(!target.actor&&active_zero()){stop_human_ai_aim(state,100u,world);return publish(8u);}
        auto status=human_ai_alignment_status(state,world);
        if(status&0x1b19u){stop_human_ai_aim(state,100u,world);return publish(status);}
        if(!(status&0xc6u)){
            advance_human_ai_trigger(state,elapsed,world);status=continue_human_ai_aim(state,world);
            if(status&0x1b19u)stop_human_ai_aim(state,100u,world);
            return publish(status);
        }
        if(resources.target().actor!=resources.aimed_actor()){stop_human_ai_aim(state,100u,world);return publish(0x80u);}
        return publish(status);
    }
    if(!target.actor&&inactive_zero())return publish(8u);
    resources.publish_aimed_actor(target.actor);resources.state().set(0xd8,target.actor?0u:2u);
    const auto status=human_ai_alignment_status(state,world);
    if(status)return publish(status);
    (void)begin_human_ai_aim(state,world);return publish(0x20u);
}
}
#endif
