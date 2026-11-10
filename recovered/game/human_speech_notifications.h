#ifndef HD2_GAME_HUMAN_SPEECH_NOTIFICATIONS_H
#define HD2_GAME_HUMAN_SPEECH_NOTIFICATIONS_H

#include "game/human_objective_experience.h"
#include <array>
#include <cstring>
#include <stdexcept>
#include <string>

namespace hd2::game {

//005A46A0 offline7 name/caption→0069BF70/0049D2E0. Actual007E6C45
//interprets the already-formatted text as a varargs format with no arguments;
//unsupported percent conversions/overflow reject rather than reading stack data.
//005A9B30 is a genuine offline RET. No timer214/254 or network text is invented.
template<class Subtitles,class Hud,class Runtime>class HumanSpeechNotifications {
public:
    HumanSpeechNotifications(Subtitles& subtitles,Hud& hud,Runtime& runtime):subtitles_(subtitles),hud_(hud),runtime_(runtime){}
    template<class Human>void notify(Human& human,std::int32_t identity){
        if(human.properties.field19c!=1)return;
        if((runtime_.mode_flags()&7u)!=7u)throw std::runtime_error("Player speech notification requires actual nonoffline current-peer/name receiver");
        if(!subtitles_.speech_caption(identity,runtime_.missing_speech_caption()))return;
        const auto* caption=subtitles_.speech_caption(identity,runtime_.missing_speech_caption());if(!caption)return;
        const auto message=offline_human_objective_name(human)+": "+caption;
        if(message.size()>=512)throw std::length_error("Human speech message exceeds native0069BF70 buffer");
        std::string formatted;formatted.reserve(message.size());
        for(std::size_t n=0;n<message.size();++n){
            if(message[n]=='%'){
                if(n+1==message.size()||message[n+1]!='%')throw std::runtime_error("Human speech message has an undefined native varargs conversion");
                ++n;
            }
            formatted+=message[n];
        }
        hud_.append_message(formatted.c_str(),{{50,150,50}});
    }
private:
    Subtitles& subtitles_;Hud& hud_;Runtime& runtime_;
};
} //namespace hd2::game
#endif
