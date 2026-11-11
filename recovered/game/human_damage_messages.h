#ifndef HD2_GAME_HUMAN_DAMAGE_MESSAGES_H
#define HD2_GAME_HUMAN_DAMAGE_MESSAGES_H
#include "game/human_state_frame_timers.h"
#include <array>
#include <cstdio>
#include <stdexcept>
#include <string>

namespace hd2::game {
//005A4600's two string arguments, then0069BF70's second zero-argument
//formatter. Reject native buffer overflow or a conversion without its actual
//argument, like the existing speech formatter; do not read unwritten stack.
inline std::string format_human_damage_text(const std::string& format,const std::array<std::string,2>& arguments){
    if(format.size()>=512)throw std::length_error("Damage message format exceeds native buffer");
    unsigned strings=0;
    for(std::size_t n=0;n<format.size();++n)if(format[n]=='%'){
        if(++n>=format.size())throw std::runtime_error("Damage format has unfinished native conversion");
        if(format[n]=='%')continue;
        while(n<format.size()&&(format[n]=='-'||format[n]=='+'||format[n]==' '||format[n]=='#'||format[n]=='0'))++n;
        while(n<format.size()&&format[n]>='0'&&format[n]<='9')++n;
        if(n<format.size()&&format[n]=='.'){++n;while(n<format.size()&&format[n]>='0'&&format[n]<='9')++n;}
        if(n>=format.size()||format[n]!='s'||++strings>2)throw std::runtime_error("Damage format requires an unprovided native vararg");
    }
    std::array<char,1024> bytes{};const auto count=std::snprintf(bytes.data(),bytes.size(),format.c_str(),arguments[0].c_str(),arguments[1].c_str());
    if(count<0||static_cast<std::size_t>(count)>=bytes.size())throw std::length_error("Damage message exceeds native first formatter buffer");
    std::string message;for(std::size_t n=0;n<static_cast<std::size_t>(count);++n){
        if(bytes[n]=='%'){if(n+1>=static_cast<std::size_t>(count)||bytes[n+1]!='%')throw std::runtime_error("Damage message has undefined native second-format conversion");++n;}
        message+=bytes[n];
    }
    if(message.size()>=512)throw std::length_error("Damage message exceeds native game formatter buffer");
    return message;
}
//005A0DA0. Zone localization precedes source lookup, even for a dead victim.
//The local-player predicate applies to SOURCE in other-Human branches. Native
//name virtual04 may replace Actor80; retain values before the next name query.
template<class Human,class World>void present_human_damage_message(Human& victim,std::uint32_t kind,std::uint32_t hit,
        std::int32_t source_id,World& world){
    if((world.mode_flags()&7u)==7u||kind==0||kind==4||(kind==5&&human_frame_signed_word(victim.properties.stats_ec[0])>0))return;
    std::uint32_t zone_id=0;
    switch(hit){case 0:zone_id=0x2261;break;case 1:zone_id=0x2262;break;case 2:case 3:zone_id=0x2263;break;
        case 4:case 5:zone_id=0x2264;break;case 6:case 7:zone_id=0x2265;break;case 8:case 9:zone_id=0x2266;break;case 11:zone_id=0x2260;break;default:break;}
    const auto zone=zone_id?world.human_damage_localize(zone_id):std::string("Unknow damage zone !!!!"); //0083C718, original spelling/punctuation.
    auto* source=world.human_damage_source(source_id);
    const std::array<float,3> white{{255,255,255}},yellow{{255,255,0}},green{{0,255,0}},dark_green{{0,160,0}};
    const auto emit=[&](std::uint32_t id,const auto& colour,const std::string& first=std::string{},const std::string& second=std::string{}){
        world.emit_human_damage_text(id,colour,std::array<std::string,2>{{first,second}});
    };
    if(human_frame_signed_word(victim.properties.stats_ec[0])<1){
        if(!source){emit(0x229e,white,world.human_damage_display_name(victim));return;}
        const auto source_team=world.human_specialization(*source);
        if(source==&victim){
            const auto id=kind==12?0x22a3u:0x2297u;
            if(world.human_is_player(victim))emit(kind==12?0x22a4u:0x2296u,yellow);
            else emit(id,source_team?std::array<float,3>{{0,0,255}}:std::array<float,3>{{255,0,0}},world.human_damage_display_name(victim));
            return;
        }
        if(world.human_is_player(*source)){
            const auto victim_team=world.human_specialization(victim),local_team=world.human_local_specialization();
            const auto id=victim_team==local_team&&world.human_damage_team_style()?0x2298u:hit==1?0x2299u:0x229au;
            emit(id,id==0x2298u?yellow:green,world.human_damage_display_name(victim));return;
        }
        const auto victim_team=world.human_specialization(victim);
        std::uint32_t id;std::array<float,3> colour;
        if(source_team==victim_team&&world.human_damage_team_style()){id=0x229b;colour=yellow;}
        else{id=hit==1?0x229cu:0x229du;const auto amount=hit==1?255.f:160.f;colour=source_team?std::array<float,3>{{0,0,amount}}:std::array<float,3>{{amount,0,0}};}
        const auto victim_name=world.human_damage_display_name(victim),source_name=world.human_damage_display_name(*source);
        emit(id,colour,source_name,victim_name);return;
    }
    if(!source||!world.human_is_player(*source))return;
    const auto victim_team=world.human_specialization(victim),local_team=world.human_local_specialization();
    const bool friendly=victim_team==local_team&&world.human_damage_team_style();
    emit(friendly?0x2292u:0x2293u,friendly?yellow:dark_green,world.human_damage_display_name(victim),zone);
}
} //namespace hd2::game
#endif
