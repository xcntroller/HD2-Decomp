#ifndef HD2_GAME_SESSION_WARNING_H
#define HD2_GAME_SESSION_WARNING_H
#include "game/human_damage_messages.h"
#include <array>
#include <cstdio>
#include <string>

namespace hd2::game {
//0069A560 warning2536 first uses00405100(format,0); warning19C7 skips
//that step. BOTH then use0069BF70 with no further varargs. Preserve the two
//format passes (including percent escapes), rather than treating text as raw.
//Only the supplied native INT argument is available: undefined stack reads,
//pointer/float/extra argument conversions and overflow fail as source guards.
inline std::string format_session_warning(const std::string& format,bool zero_argument){
    if(!zero_argument)return format_human_game_message(format);
    bool argument=false;char conversion=0;
    for(std::size_t n=0;n<format.size();++n){
        if(format[n]!='%')continue;
        if(++n==format.size())throw std::runtime_error("Session warning has an incomplete format");
        if(format[n]=='%')continue;
        if(argument)throw std::runtime_error("Session warning requires another unavailable argument");
        argument=true;
        while(n<format.size()&&std::string("-+ #0").find(format[n])!=std::string::npos)++n;
        const auto number=[&]{
            unsigned value=0;
            while(n<format.size()&&format[n]>='0'&&format[n]<='9'){
                value=value*10+static_cast<unsigned>(format[n++]-'0');
                if(value>=512)throw std::length_error("Session warning format exceeds native game buffer");
            }
        };
        number();
        if(n<format.size()&&format[n]=='.'){++n;number();}
        if(n==format.size()||std::string("diuoxXc").find(format[n])==std::string::npos)
            throw std::runtime_error("Session warning requires an unavailable native argument conversion");
        conversion=format[n];
    }
    std::array<char,512> buffer{};
    const auto size=conversion=='u'||conversion=='o'||conversion=='x'||conversion=='X'
        ?std::snprintf(buffer.data(),buffer.size(),format.c_str(),0u)
        :std::snprintf(buffer.data(),buffer.size(),format.c_str(),0);
    if(size<0||static_cast<std::size_t>(size)>=buffer.size())
        throw std::length_error("Session warning exceeds native game buffer");
    return format_human_game_message(buffer.data());
}
}
#endif
