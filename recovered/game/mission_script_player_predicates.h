#ifndef HD2_GAME_MISSION_SCRIPT_PLAYER_PREDICATES_H
#define HD2_GAME_MISSION_SCRIPT_PLAYER_PREDICATES_H

#include "game/mission_script_context.h"
#include "game/scene_actor.h"
#include <type_traits>

namespace hd2::game {

//005A63B0 uses the actual HumanB0 presence, faction19C and activation20C.
//It does not substitute health, state kind or roster membership for that query.
template<class Human>bool script_human_player_controlled(const Human& human,std::uint32_t mode) {
    if((mode&7u)==7u)return true;
    if(!human.loading_state)return false;
    return human.properties.field19c!=1||human.activation.flag20c;
}

//0041E3F0 player predicates use the SAME Players control/auxiliary rosters.
//Only these original cases are claimed; nullopt requires another real receiver.
//Runtime supplies006B66B0 and, on its exact branch,005A63F0's virtual64 query.
template<class Frames,class Players>class MissionScriptPlayerPredicates {
public:
    using Frame=typename Frames::Frame;
    MissionScriptPlayerPredicates(Frames& frames,Players& players):frames_(frames),players_(players){}
    template<class Context,class Runtime>std::optional<std::uint32_t> evaluate(Context& context,std::uint32_t opcode,
            const std::array<std::uint32_t,4>& values,const MissionScriptProgram& program,Runtime& runtime) {
        if(!runtime.script_authority())return 0;
        auto& roster=players_.control().human_roster();
        switch(opcode) {
        case 0: {
            const auto radius=program.integers.at(values[0]).value;
            const auto threshold=static_cast<long double>(signed_word(radius*radius)); //Native IMUL then signed FILD; NO FLOAT conversion.
            std::uint32_t count=0;
            const auto test=[&](auto& list,bool remote) {
                for(std::size_t n=list.size();n;) {
                    auto* human=list.at(static_cast<std::int32_t>(--n));if(!human||!alive(*human))continue;
                    if(remote) {
                        if(!script_human_player_controlled(*human,runtime.mode_flags()))continue;
                        if((runtime.mode_flags()&7u)!=7u&&human->loading_state&&human->properties.field19c==1&&runtime.human_selected_suppressed(*human))continue;
                    }
                    const auto frame=human->base().frame();if(!frame||!context.frame())continue;
                    if(distance_squared(frame,context.frame())<threshold)++count;
                }
            };
            if(runtime.human_remote_ai_enabled())test(players_.auxiliary().human_roster(),true);else test(roster,false);
            return count;
        }
        case 2: {
            const auto radius=static_cast<long double>(signed_word(program.integers.at(values[0]).value));bool any=false;
            for(std::size_t n=roster.size();n;) {
                auto* human=roster.at(static_cast<std::int32_t>(--n));
                if(!human)throw std::runtime_error("AllPlayersInRange has a null native roster member");
                if(!alive(*human))continue;
                any=true;const auto frame=human->base().frame();if(!frame||!context.frame())continue;
                if(distance(frame,context.frame())>radius)return 0;
            }
            return std::uint32_t(any);
        }
        case 0xd:case 0x12:{
            using Human=std::remove_pointer_t<decltype(roster.at(0))>;
            const auto* data=context.frame()?frames_.find_actor_frame_data(context.frame()):nullptr;
            auto* actor=data?reinterpret_cast<SceneActor<Frames>*>(data->words[0]):nullptr;
            if(!actor)return 0;
            if(opcode==0x12&&actor->base().type()==30)throw std::runtime_error("Dog alarm predicate requires its actual005CE6F0 receiver");
            if(actor->base().type()!=3)return 0;
            auto* human=dynamic_cast<Human*>(actor);if(!human)throw std::logic_error("Script predicate requires actual EC Human");
            return opcode==0xd?std::uint32_t(runtime.script_human_sees_enemy(*human)):runtime.script_human_alarm_type(*human);
        }
        case 0xf:{
            const auto frame=context.slot(values[0]);if(!frame)return 1000u;
            if((runtime.mode_flags()&7u)!=7u){
                using Human=std::remove_pointer_t<decltype(roster.at(0))>;
                const auto* data=frames_.find_actor_frame_data(frame);
                auto* actor=data?reinterpret_cast<SceneActor<Frames>*>(data->words[0]):nullptr;
                if(actor&&actor->base().type()==3){
                    auto* human=dynamic_cast<Human*>(actor);if(!human)throw std::logic_error("Frame range requires actual target Human");
                    if(!alive(*human)||!script_human_player_controlled(*human,runtime.mode_flags())||runtime.human_selected_suppressed(*human))return 1000u;
                }
            }
            if(!context.frame())throw std::logic_error("Frame range requires its actual context frame");
            const auto value=distance(frame,context.frame());
            if(!std::isfinite(value)||value< -9223372036854775808.L||value>=9223372036854775808.L)return 0u;
            return static_cast<std::uint32_t>(static_cast<std::int64_t>(value)); //Original007E5E00 low DWORD.
        }
        case 0x13:return std::uint32_t(runtime.script_inventory_has_definition(signed_word(program.integers.at(values[0]).value)));
        case 5:return std::uint32_t(roster.live_count()==0);
        case 0x14:return roster.live_count();
        case 0x16:return std::uint32_t(roster.live_count()!=roster.size());
        default:return std::nullopt;
        }
    }
private:
    template<class Human>static bool alive(const Human& human){return signed_word(human.properties.stats_ec[0])>0;}
    long double distance_squared(Frame first,Frame second) {
        //Native updates the context frame first, then the Human model frame.
        const auto b=frames_.actor_world_matrix(second),a=frames_.actor_world_matrix(first);
        const auto x=static_cast<long double>(a[12])-b[12],y=static_cast<long double>(a[13])-b[13],z=static_cast<long double>(a[14])-b[14];
        return (z*z+y*y)+x*x;
    }
    long double distance(Frame first,Frame second) {
        const float stored=static_cast<float>(distance_squared(first,second));std::int32_t bits;std::memcpy(&bits,&stored,4);
        if(bits<0x322bcc77)return 0;
        if(std::abs(static_cast<long double>(stored)-1.L)<static_cast<long double>(9.99999993922529e-9f))return 1;
        return std::sqrt(static_cast<long double>(stored));
    }
    static std::int32_t signed_word(std::uint32_t word){std::int32_t result;std::memcpy(&result,&word,4);return result;}
    Frames& frames_;Players& players_;
};
} //namespace hd2::game
#endif
