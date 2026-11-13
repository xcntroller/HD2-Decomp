#ifndef HD2_GAME_HUMAN_PLAYER_PACKET_ACTION_H
#define HD2_GAME_HUMAN_PLAYER_PACKET_ACTION_H
#include "game/human_packet_commands.h"
namespace hd2::game {
//Original0083C348/0083C34C, including native aliases into0083C4A0 after42.
//53/56 are selected separately before lookup. Larger packet values have no
//proven action producer; reject instead of reading unrelated native storage.
inline std::array<std::uint32_t,2> human_packet_action_pair(std::uint32_t action){
    static constexpr std::array<std::array<std::uint32_t,2>,57> pairs{{
        {{76u,0u}},
        {{0u,1u}},
        {{76u,2u}},
        {{1u,3u}},
        {{76u,4u}},
        {{2u,5u}},
        {{76u,6u}},
        {{3u,7u}},
        {{76u,8u}},
        {{4u,9u}},
        {{76u,125u}},
        {{2u,126u}},
        {{76u,10u}},
        {{5u,11u}},
        {{76u,12u}},
        {{6u,13u}},
        {{76u,14u}},
        {{7u,15u}},
        {{76u,16u}},
        {{76u,18u}},
        {{76u,20u}},
        {{76u,22u}},
        {{76u,24u}},
        {{76u,26u}},
        {{76u,28u}},
        {{76u,30u}},
        {{76u,32u}},
        {{76u,34u}},
        {{76u,36u}},
        {{76u,38u}},
        {{76u,40u}},
        {{76u,42u}},
        {{76u,44u}},
        {{76u,138u}},
        {{76u,138u}},
        {{76u,130u}},
        {{76u,131u}},
        {{76u,132u}},
        {{76u,133u}},
        {{76u,134u}},
        {{76u,135u}},
        {{76u,136u}},
        {{76u,137u}},
        {{46u,48u}},
        {{50u,52u}},
        {{54u,56u}},
        {{58u,60u}},
        {{62u,62u}},
        {{62u,62u}},
        {{62u,62u}},
        {{64u,66u}},
        {{68u,138u}},
        {{72u,74u}},
        {{138u,78u}},
        {{80u,82u}},
        {{84u,86u}},
        {{86u,88u}},
    }};
    if(action>=pairs.size())throw std::out_of_range("Human packet action exceeds recovered original table");
    return pairs[action];
}
inline std::uint32_t human_packet_action_speech(std::uint32_t index){
    static constexpr std::array<std::uint32_t,39> categories{{46u,48u,50u,52u,54u,56u,58u,60u,62u,62u,62u,62u,62u,62u,64u,66u,68u,138u,72u,74u,138u,78u,80u,82u,84u,86u,86u,88u,90u,129u,138u,138u,138u,58u,138u,138u,86u,18u,50u}};
    return index<categories.size()?categories[index]:0x8au;
}
//0053D870/00594110. Presentation does not publish speech or delayed commands.
template<class State,class Services>void present_human_packet_action(State& state,const HumanPacketCommand& command,Services& services){
    const auto action=command.fields.template get<std::uint32_t>(0x40);
    if(action==53||action==56)return;
    const auto pair=human_packet_action_pair(action);
    if(pair[0]!=0x4cu)services.human_packet_inline_action(state,pair[0]);
}
//0053D520 calls actual Players virtual2C, independent of Human DD0 commands.
template<class Process>void set_human_packet_action_speech_mode(Process& process,std::uint32_t action){
    if(action<=17)process.speech_mode(static_cast<std::uint8_t>(action&1u));
    else if(action==18||action==19||action==20||action==53)process.speech_mode(0);
    else if(action==56)process.speech_mode(1);
}
//0053D5D0. Preserve mutation prefixes on exceptions; no whole-action replay.
template<class State,class Services>bool apply_player_packet_action(State& state,const HumanPacketCommand& command,Services& services){
    const auto& f=command.fields;const auto action=f.template get<std::uint32_t>(0x40);
    auto validate=services.human_packet_action_validator(state);
    if(action==33||action==34)return services.human_packet_support_action(state,command);
    if((services.mode_flags()&7u)!=7u){
        if(action!=53&&action!=56){
            present_human_packet_action(state,command,services);validate();
            (void)services.human_packet_action_speech(state,human_packet_action_pair(action)[1]);
        }
        return true;
    }
    auto& process=services.human_packet_players_process();
    set_human_packet_action_speech_mode(process,action);
    auto& words=state.player_words();words.set(0xc0,process.speech_mode());words.set(0x520,0u);
    if(action==53||action==56){
        auto category=human_packet_action_speech(f.template get<std::uint32_t>(0x44));
        if(category!=0x8au){
            if(action==56)++category;
            const auto duration=services.human_packet_action_speech(state,category);validate();words.set(0x520,duration);
        }
    }else{
        present_human_packet_action(state,command,services);validate();
        const auto category=human_packet_action_pair(action)[1]+(action>=21&&action<=32&&words.template get<std::uint8_t>(0xc0)?1u:0u);
        const auto duration=services.human_packet_action_speech(state,category);validate();words.set(0x520,duration);
    }
    if(!words.template get<std::uint32_t>(0x520)){
        const auto delay=human_packet_ftol(static_cast<long double>(services.human_sound_random())*500.f+1000.f);validate();words.set(0x520,delay);
    }
    words.set(0x4e4,f.template get<std::uint32_t>(0x24));
    for(unsigned n=0;n!=9;++n)words.set(0x4f0+4*n,f.template get<std::uint32_t>(0x30+4*n));
    return true;
}
} //namespace hd2::game
#endif
