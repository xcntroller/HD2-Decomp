#ifndef HD2_GAME_HUD_MESSAGE_EDIT_H
#define HD2_GAME_HUD_MESSAGE_EDIT_H
#include "game/hud_message_state.h"
#include "game/text_glyphs.h"
#include "frontend/keyboard_preprocessing.h"
#include "frontend/edit_character_target.h"
#include <algorithm>
#include <array>
#include <cstring>
#include <string_view>
namespace hd2::game {
inline std::int32_t hud_message_signed(std::uint32_t bits){std::int32_t value;std::memcpy(&value,&bits,4);return value;}
inline std::size_t hud_message_input_length(const HudMessageState& state){
    const auto end=std::find(state.input68.begin(),state.input68.end(),'\0');
    if(end==state.input68.end())throw std::logic_error("HUD message input is not terminated");
    return static_cast<std::size_t>(end-state.input68.begin());
}
//00495190: BYTE font3, including native ADD of the negative DOUBLE -.4
//fallback. This constant differs from the draw worker's positive .4 width.
//The returned index deliberately is NOT incremented. No copied font/cache.
template<class Fonts>std::uint32_t hud_message_scroll_index(const HudMessageState& state,const Fonts& fonts){
    const auto& bounds=state.field10?state.second58:state.first48;
    const auto height=text_ftol(static_cast<long double>(text_real(state.heightd370))*state.height44);
    if(!fonts.contains(3))return 0;
    const float stored_height=static_cast<float>(height);
    const auto cursor_width=text_ftol(static_cast<long double>(height)*fonts.glyph(3,'_').aspect);
    auto x=text_sub(text_ftol(bounds[2]),cursor_width);
    for(auto n=hud_message_input_length(state);n;){--n;
        const auto product=static_cast<long double>(stored_height)*fonts.glyph(3,static_cast<unsigned char>(state.input68[n])).aspect;
        x=product==0?text_add(x,text_ftol(static_cast<long double>(height)*-.4)):text_sub(x,text_ftol(product));
        if(static_cast<long double>(x)<=bounds[0])return static_cast<std::uint32_t>(n);
    }
    return 0;
}
template<class Input>void hud_message_input_mode(HudMessageState& state,std::uint8_t enabled,Input& input){ //004961E0.
    if(enabled){state.caret_d380=0;input.clear_text_characters();state.flagd385=enabled;}
    else{state.input68[0]=0;state.flagd387=state.flagd386=state.flagd385=0;}
}
//00494D20. Submission is REQUIRED and may fail; echo occurs before dispatch
//and the edit buffer/mode clear only after dispatch returns, just as native.
template<class Input,class Fonts,class Submit,class Echo>
void edit_hud_message(HudMessageState& state,std::size_t record_count,bool wide,Input& input,const Fonts& fonts,Submit& submit,Echo&& echo){
    auto key=input.next_scan();auto& repeat=state.words30;
    if(key){repeat[1]=key;repeat[3]=150;repeat[2]=0;}
    else if(repeat[1]){
        if(hud_message_signed(repeat[2])>hud_message_signed(repeat[3])){
            if(input.key_down(static_cast<std::uint8_t>(repeat[1]))){key=repeat[1];repeat[3]=50;repeat[2]=0;}
            else repeat[1]=0;
        }
    }else repeat[1]=0;
    if(key==200){if(state.field10==1&&repeat[0]<static_cast<std::uint32_t>(record_count)-1u)++repeat[0];return;}
    if(key==208){if(state.field10==1&&repeat[0])--repeat[0];return;}
    const auto scroll=[&]{state.field168=hud_message_scroll_index(state,fonts);};
    const auto send=[&]{
        //Original substitutes008399C4 (one space) for empty ECHO only.
        echo(state.input68[0]?state.input68.data():" ");
        if(state.flagd386)submit.say(state.input68.data(),false);
        else if(state.flagd387)submit.say(state.input68.data(),true);
        else submit.console(state.input68.data());
        state.input68[0]=0;state.field168=0;
        if(state.field10!=1)hud_message_input_mode(state,0,input);
    };
    if(wide){
        std::array<char16_t,1024> units{};
        if(!frontend::read_frontend_characters(input,units.data(),1023)||!units[0])return;
        for(const auto unit:units){if(!unit)return;
            if(unit==9)return;if(unit==13){send();return;}
            if(unit==8){
                const auto length=hud_message_input_length(state);
                if(length){
                    auto first=static_cast<std::ptrdiff_t>(length)-1;
                    while(first>=0){const auto byte=static_cast<unsigned char>(state.input68[static_cast<std::size_t>(first)]);if(byte<0x80u||byte>=0xc0u)break;--first;}
                    if(first<0)return; //Native would read input[-1] on a malformed all-continuation string.
                    const auto count=length-static_cast<std::size_t>(first);const auto lead=static_cast<unsigned char>(state.input68[static_cast<std::size_t>(first)]);
                    if(!((count==1&&lead<0x80u)||(count==2&&lead>=0xc0u&&lead<0xe0u)||(count==3&&lead>=0xe0u&&lead<0xf0u)||(count==4&&lead>=0xf0u&&lead<0xf8u)))return;
                    state.input68[static_cast<std::size_t>(first)]=0;
                }
                scroll();return; //Remaining converted units are discarded after backspace.
            }
            std::array<char,8> encoded{};const auto count=frontend::encode_edit_character(encoded.data(),unit,6);
            if(count>0){const auto length=hud_message_input_length(state);
                if(length+static_cast<unsigned>(count)<255u){std::memcpy(state.input68.data()+length,encoded.data(),static_cast<unsigned>(count));state.input68[length+static_cast<unsigned>(count)]=0;scroll();}
            }
        }
    }else{
        const auto character=input.take_text_character();if(!character||character==9)return;
        if(character==13){send();return;}
        const auto length=hud_message_input_length(state);
        if(character==8){if(length)state.input68[length-1]=0;scroll();return;}
        if(length<255u){const auto byte=static_cast<char>(static_cast<std::uint8_t>(character));
            //Native strncat(...,1) does not append the zero low byte of a
            //nonzero DWORD character, but it still recomputes scrolling.
            if(byte){state.input68[length]=byte;state.input68[length+1]=0;}scroll();
        }
    }
}
} //namespace hd2::game
#endif
