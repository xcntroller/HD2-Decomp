#ifndef HD2_GAME_HUMAN_SPEECH_LOADING_H
#define HD2_GAME_HUMAN_SPEECH_LOADING_H

#include "game/speech_definitions.h"
#include "game/human_loading_stats.h"
#include <cstdint>
#include <cstring>

namespace hd2::game {

inline std::int32_t speech_signed_word(std::uint32_t bits){std::int32_t value;std::memcpy(&value,&bits,4);return value;}
inline std::int32_t speech_sample_identity(std::uint32_t phrase){return static_cast<std::int32_t>(phrase%10000u+90000000u);} //005B0440, unsigned DIV.
inline std::int32_t speech_near_range(std::uint32_t phrase){ //005B0460, unsigned QWORD FILD then exact DOUBLE00811978.
    return speech_signed_word(0xffffffa6u-human_loading_stat_word(static_cast<long double>(phrase)*-1.e-6));
}
inline std::int32_t speech_range(std::uint32_t phrase){ //005B0490, retain x87 fractional part after first FISTP.
    const auto value=static_cast<long double>(phrase)*1.e-6; //DOUBLE00811980, not a decimal long-double approximation.
    const auto whole=speech_signed_word(human_loading_stat_word(value));
    return speech_signed_word(human_loading_stat_word((value-whole)*100.f)); //FLOAT0080FA04.
}
template<class Human>std::int32_t human_speech_readiness(const Human& human){ //005A6280, signed low DWORD of FLOAT16C FISTP then clamp.
    return std::clamp(speech_signed_word(human_loading_stat_word(human.activation.field16c)),0,100);
}
inline void postpone_speech_category(SpeechDefinition::Category& category,std::uint32_t delay,std::uint32_t now){category.field14=now+delay;} //005AF7D0.
inline bool speech_category_cooling(const SpeechDefinition::Category& category,std::uint32_t now){return now<category.field14;} //005AF7F0, unsigned comparison.
inline bool speech_category_populated(const SpeechDefinition::Category& category){return category.enabled&&!category.phrases.empty();} //005AF850.
inline bool speech_category_available(const SpeechDefinition::Category& category,std::uint32_t now){return !speech_category_cooling(category,now)&&speech_category_populated(category);} //005AF810, timer BEFORE flag/list.

//005AF8E0 and005AFA60 share the actual vector/window rotation. The latter
//ignores cooldown, so a successful common speech rotates TWICE. No copied set.
template<class Random>std::uint32_t rotate_speech_phrase(SpeechDefinition::Category& category,Random& random){
    if(category.phrases.empty()||category.cursor>category.phrases.size())throw std::logic_error("Speech rotation lost its actual phrase window");
    const auto phrase=category.phrases.front();category.phrases.erase(category.phrases.begin());
    if(!category.cursor)category.cursor=category.phrases.size();else --category.cursor;
    const auto draw=random();if(draw>0x7fffu)throw std::runtime_error("Speech rotation requires shared CRT rand");
    const auto index=category.cursor==0?draw%(category.phrases.size()+1u)+1u:
        category.cursor+draw%((category.phrases.size()-category.cursor)+2u);
    category.phrases.insert(category.phrases.begin()+static_cast<std::ptrdiff_t>(std::min(index,category.phrases.size())),phrase);
    return phrase;
}
template<class Random>std::uint32_t select_speech_phrase(SpeechDefinition& definition,std::int32_t identity,
        std::int32_t delay,std::uint32_t now,Random& random){ //005AF8E0;005AF810/005AF7F0 unsigned deadline gate.
    if(identity<0||static_cast<std::size_t>(identity)>=definition.categories.size())return 0xffffffffu;
    auto& category=definition.categories[static_cast<unsigned>(identity)];
    if(!speech_category_available(category,now))return 0xffffffffu;
    if(delay>0)postpone_speech_category(category,static_cast<std::uint32_t>(delay),now); //005AF7D0 BEFORE rotation.
    return rotate_speech_phrase(category,random);
}
template<class Random>std::uint32_t play_speech_phrase(SpeechDefinition& definition,std::int32_t identity,Random& random){ //005AFA60/005AF850, no clock gate.
    if(identity<0||static_cast<std::size_t>(identity)>=definition.categories.size())return 0xffffffffu;
    auto& category=definition.categories[static_cast<unsigned>(identity)];
    return speech_category_populated(category)?rotate_speech_phrase(category,random):0xffffffffu;
}
inline void postpone_speech_category(SpeechDefinition& definition,std::int32_t identity,std::uint32_t delay,std::uint32_t now){ //005AFD10→005AF7D0.
    if(identity<0||static_cast<std::size_t>(identity)>=definition.categories.size())return;
    auto& category=definition.categories[static_cast<unsigned>(identity)];if(category.enabled)postpone_speech_category(category,delay,now);
}

//Actual offline005882F0/00588170 and005383D0/00542060 callers. Services
//borrows the SAME process rand, clocks, Players query43/groups, SOUND/dubbing/
//duration map and required player subtitle-notification owner. It supplies no
//successful defaults. Containing B0/Human/definition/frame owners outlive calls.
template<class Frames,class Services>class HumanSpeechLoading {
public:
    HumanSpeechLoading(Frames& frames,Services& services):frames_(frames),services_(services){}
    template<class State>std::uint32_t request(State& state,std::int32_t category,std::uint32_t delay,std::uint32_t flags,bool quiet){ //005882F0.
        (void)flags; //Offline7 never enters the absent-bit2 SOUND4C branch; flags do not alter this path.
        auto* human=state.human();if(!human)throw std::logic_error("Speech requires its actual bound Human");
        auto* definition=human->speech_definition;if(!definition)return 0;
        auto random=[&]{return services_.speech_random();};
        const auto phrase=select_speech_phrase(*definition,category,0,services_.game_word28(),random);
        //Native consumes the guarded first phrase BEFORE testing B8 or40000.
        if(speech_signed_word(phrase)<0||state.words().template get<std::int32_t>(0xb8)>0||
                (state.words().template get<std::uint32_t>(0x50)&0x40000u))return 0;
        state.words().set(0xb8,std::uint32_t{1000});
        postpone_speech_category(*definition,category,delay,services_.game_word28());
        const auto mode=services_.mode_flags();
        if((mode&7u)!=7u)throw std::runtime_error("Human speech requires actual nonoffline queued kind13 task/peer producer");
        offline(state,category,quiet);
        return services_.human_speech_length(speech_sample_identity(phrase)); //FIRST phrase, even when quiet/distance skips second playback.
    }
    template<class State>void active(State& state,std::int32_t category,bool variant,std::uint32_t delay,bool quiet){ //005383D0.
        if(state.words().template get<std::uint32_t>(0x50)&0x40000u)return;
        if(variant&&(state.state().template get<std::uint8_t>(0x18a)||state.state().template get<std::uint8_t>(0x188)||services_.speech_special_mode43()))
            category=speech_signed_word(static_cast<std::uint32_t>(category)+1u);
        (void)request(state,category,delay,1,quiet);
    }
    template<class State>void player(State& state,std::int32_t category,std::uint32_t delay){ //00542060, offline branch.
        if((services_.mode_flags()&7u)!=7u)throw std::runtime_error("Player speech requires actual nonoffline reserved-Human speech eligibility scan");
        auto* human=state.human();if(!human)throw std::logic_error("Player speech requires actual bound Human");
        if(human_speech_readiness(*human)==0&&services_.speech_group0_active()){
            if(state.player_words().template get<std::uint8_t>(0xc0))category=speech_signed_word(static_cast<std::uint32_t>(category)+1u);
            (void)request(state,category,delay,1,false);
        }else (void)request(state,category,delay,1,true);
    }
private:
    template<class State>void offline(State& state,std::int32_t category,bool quiet){ //00588170.
        auto& human=*state.human();auto random=[&]{return services_.speech_random();};
        const auto phrase=play_speech_phrase(*human.speech_definition,category,random);
        if(human.properties.field19c==1)services_.player_speech_notification(human,speech_sample_identity(phrase));
        if(quiet)return;
        const auto camera=frames_.active_game_camera();if(!camera||!human.head_marker)throw std::runtime_error("Human speech distance requires actual active camera/head marker");
        const auto listener=frames_.actor_world_matrix(camera),source=frames_.actor_world_matrix(human.head_marker);
        const float x=listener[12]-source[12],y=listener[13]-source[13],z=listener[14]-source[14];
        const auto square=(static_cast<long double>(z)*z+static_cast<long double>(y)*y)+static_cast<long double>(x)*x;
        const auto distance=speech_range(phrase)*2;
        //588288 compares radius² against square: unordered also reaches SOUND.
        if(!(static_cast<long double>(distance)*distance<square))
            services_.play_human_speech(human,speech_sample_identity(phrase),static_cast<float>(speech_near_range(phrase)),static_cast<float>(distance),false);
    }
    Frames& frames_;Services& services_;
};
} //namespace hd2::game
#endif
