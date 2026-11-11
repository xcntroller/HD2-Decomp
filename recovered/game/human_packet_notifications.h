#ifndef HD2_GAME_HUMAN_PACKET_NOTIFICATIONS_H
#define HD2_GAME_HUMAN_PACKET_NOTIFICATIONS_H

#include "game/human_camera_state.h"
#include "frontend/localized_text.h"
#include <array>
#include <string_view>
#include <stdexcept>
#include <vector>

namespace hd2::game {

//Typed string arguments for the native varargs message calls. Reject unsafe
//conversions/overflow instead of reading nonexistent arguments or stack storage.
//The two formatting passes of005A4600→0069BF70 remain distinct.
inline std::string format_human_packet_message(std::string_view format,const std::vector<std::string>& arguments,std::size_t capacity){
    if(!capacity)throw std::invalid_argument("Zero packet message capacity");
    format=format.substr(0,format.find('\0'));std::string result;std::size_t argument=0;
    const auto append=[&](std::string_view value){
        if(value.size()>=capacity-result.size())throw std::length_error("Human packet message exceeds native buffer");
        result.append(value);
    };
    for(std::size_t i=0;i<format.size();){
        if(format[i]!='%'){append(format.substr(i++,1));continue;}
        ++i;if(i<format.size()&&format[i]=='%'){append("%");++i;continue;}
        bool left=false;if(i<format.size()&&format[i]=='-'){left=true;++i;}
        const auto number=[&](){std::size_t value=0;
            while(i<format.size()&&format[i]>='0'&&format[i]<='9'){
                const auto digit=static_cast<unsigned>(format[i++]-'0');
                if(value>capacity/10||(value==capacity/10&&digit>capacity%10))throw std::length_error("Human packet format width exceeds native buffer");
                value=value*10+digit;
            }
            return value;
        };
        const auto width=number();std::size_t precision=std::string_view::npos;
        if(i<format.size()&&format[i]=='.'){++i;precision=number();}
        if(i==format.size()||format[i++]!='s'||argument==arguments.size())
            throw std::runtime_error("Human packet message requires unavailable native varargs arguments/conversion");
        std::string_view value=arguments[argument++];value=value.substr(0,value.find('\0'));value=value.substr(0,precision);
        const auto padding=width>value.size()?width-value.size():0;
        if(padding>=capacity-result.size()||value.size()>=capacity-result.size()-padding)
            throw std::length_error("Human packet message padding exceeds native buffer");
        if(!left)result.append(padding,' ');
        append(value);
        if(left)result.append(padding,' ');
    }
    return result;
}

//005A8320 differs from005A82C0: user14 falls back to FULL name0C. Without
//a profile it reads the actual model's name, not Actor virtual04.
template<class Human,class Frames>std::string human_packet_notification_name(const Human& human,Frames& frames){
    if(const auto* profile=human.profile_record)return profile->user_name.empty()?profile->full_name:profile->user_name;
    const auto frame=human.base().frame();if(!frame)throw std::runtime_error("Human notification name requires actual model");
    return frames.frame_name(frame);
}

template<class Hud>void publish_human_packet_notification(const frontend::LocalizedTextCache& text,Hud& hud,
        std::int32_t caption,const std::vector<std::string>& names,const std::array<float,3>& colour){ //005A4600.
    const auto format=frontend::lookup_localized_text(text,caption);
    if(format.find('\0')>=512&&format.size()>=512)throw std::length_error("Human notification caption exceeds native512 buffer");
    const auto intermediate=format_human_packet_message(format,names,1024);
    const auto message=format_human_packet_message(intermediate,{},512); //0069BF70 has NO further arguments in this caller.
    hud.append_message(message.c_str(),colour);
}

//00588870, B0 virtual10, complete ordinary offline branch. Client-only medical
//item/health changes are behind !mode1 and are not part of offline7 presentation.
template<class Human,class Frames,class Hud>void receive_human_offline_notification(Human& source,Human* target,Frames& frames,
        const frontend::LocalizedTextCache& text,Hud& hud,std::uint32_t mode,std::int32_t local_player){
    if((mode&7u)!=7u)throw std::runtime_error("Human notification requires actual nonoffline medical item receiver");
    if(!target)return;
    constexpr std::array<float,3> green{{0,160,0}},white{{255,255,255}};
    const bool source_player=human_camera_is_player(source,mode,local_player);
    if(&source==target){
        if(source_player)publish_human_packet_notification(text,hud,0x2277,{},green);
        else publish_human_packet_notification(text,hud,0x2278,{human_packet_notification_name(source,frames)},white);
    }else if(source_player)publish_human_packet_notification(text,hud,0x2274,{human_packet_notification_name(*target,frames)},green);
    else if(human_camera_is_player(*target,mode,local_player))publish_human_packet_notification(text,hud,0x2275,{human_packet_notification_name(source,frames)},green);
    else{
        //005889DD gets target first, then source; stack argument order is source,target.
        const auto target_name=human_packet_notification_name(*target,frames),source_name=human_packet_notification_name(source,frames);
        publish_human_packet_notification(text,hud,0x2276,{source_name,target_name},white);
    }
    //No actor/state borrower is dereferenced after the callback-visible HUD call.
}
} //namespace hd2::game
#endif
