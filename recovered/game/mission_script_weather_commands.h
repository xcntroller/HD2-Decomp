#ifndef HD2_GAME_MISSION_SCRIPT_WEATHER_COMMANDS_H
#define HD2_GAME_MISSION_SCRIPT_WEATHER_COMMANDS_H
#include "game/mission_script_context.h"
#include "game/session_broadcast_packets.h"
#include <algorithm>

namespace hd2::game {
namespace script_weather {
inline std::int32_t signed_word(std::uint32_t word){std::int32_t value;std::memcpy(&value,&word,4);return value;}
inline std::uint32_t bits(float value){std::uint32_t word;std::memcpy(&word,&value,4);return word;}
inline float scaled(std::uint32_t word,float low,float high){ //00418A9C/00418B7E: FILD, multiply, compare EXTENDED, final FLOAT spill.
    const auto value=static_cast<long double>(signed_word(word))*.0010000000474974513f;
    return static_cast<float>(std::max(static_cast<long double>(low),std::min(static_cast<long double>(high),value)));
}
inline std::array<std::uint32_t,3> direction(const std::array<std::uint32_t,4>& values){ //004186C1: Z retains FILD precision in square.
    std::array<float,3> v{{static_cast<float>(signed_word(values[0])),static_cast<float>(signed_word(values[1])),static_cast<float>(signed_word(values[2]))}};
    const auto square=(static_cast<long double>(signed_word(values[2]))*v[2]+static_cast<long double>(v[1])*v[1])+static_cast<long double>(v[0])*v[0];
    const float stored=static_cast<float>(square);constexpr float epsilon=9.99999993922529e-09f;
    if(!(std::abs(square-1.L)<epsilon)){
        if(!(stored<epsilon)){const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));for(auto& axis:v)axis=static_cast<float>(axis*inverse);}
        else if(v[0]!=0)v[0]=v[0]<0?-1.f:1.f;
        else if(v[2]!=0)v[2]=v[2]<0?-1.f:1.f;
        else v[1]=v[1]<0?-1.f:1.f;
    }
    return {{bits(v[0]),bits(v[1]),bits(v[2])}};
}
inline std::uint32_t colour(const std::array<std::uint32_t,4>& values){
    std::uint32_t word=0;for(unsigned n=0;n!=4;++n)word|=static_cast<std::uint32_t>(std::clamp(signed_word(values[n]),0,255))<<(8*n);return word;
}
inline bool accepts(std::uint32_t opcode){switch(opcode){case 0xb8:case 0xb9:case 0xba:case 0xbd:case 0xbe:case 0xbf:case 0xc1:case 0xc2:case 0xc3:return true;default:return false;}}
}
//Actual module4 packet/FIFO and Engine scene+11C. GEffects40 is a separate
//process flag, updated by the presentation receiver even when no scene exists.
template<class Frames,class Runtime>class MissionScriptWeatherCommands {
public:
    MissionScriptWeatherCommands(Frames& frames,Runtime& runtime,SessionBroadcastPackets& packets,std::uint32_t& mode)
        :frames_(frames),runtime_(runtime),packets_(packets),mode_(mode){}
    template<class Context>std::optional<bool> execute(Context& context,const MissionScriptInstruction& instruction){
        const auto op=instruction.opcode;if(!script_weather::accepts(op))return std::nullopt;if(!(mode_&1u))return false;
        const unsigned count=op==0xbe||op==0xbf?4:op==0xc3?3:op==0xbd||op==0xc2?2:1;
        std::array<std::uint32_t,4> values{};for(unsigned n=0;n<count;++n)values[n]=context.program().integers.at(instruction.operands.at(n)).value;
        auto payload=values;unsigned words=count,width=32;
        switch(op){
        case 0xb8:payload[0]=values[0]!=0;width=1;break;
        case 0xba:payload[0]=script_weather::bits(script_weather::scaled(values[0],0,200));break;
        case 0xbd:payload[0]=script_weather::bits(script_weather::scaled(values[0],.00009999999747378752f,20));payload[1]=script_weather::bits(script_weather::scaled(values[1],0,500));break;
        case 0xbe:case 0xbf:payload[0]=script_weather::colour(values);words=1;break;
        case 0xc1:payload[0]=values[0]==1;width=1;break;
        case 0xc3:{const auto v=script_weather::direction(values);std::copy(v.begin(),v.end(),payload.begin());break;}
        default:break;
        }
        auto& packet=packets_.begin(4,0);packet.write(op,8);
        for(unsigned n=0;n<words;++n)packet.write(payload[n],width);
        packet.finish_broadcast();
        if(!(mode_&2u)){
            //Original authority-only direct path uses signed Space/FILD and raw
            //Mode DWORD, unlike the packet decoder's unsigned Space/one-bit Mode.
            if(op==0xc1)payload[0]=values[0];
            if(op==0xc2)for(unsigned n=0;n<2;++n)payload[n]=script_weather::bits(static_cast<float>(script_weather::signed_word(values[n])));
            publish(op,payload,false);
        }
        context.advance(runtime_);return true;
    }
    bool receive_opcode(std::uint32_t op,SessionPacketStream& packet){
        if(!script_weather::accepts(op))return false;
        std::array<std::uint32_t,4> values{};const unsigned count=op==0xbd||op==0xc2?2:op==0xc3?3:1;
        for(unsigned n=0;n<count;++n)values[n]=packet.read(op==0xb8||op==0xc1?1:32);
        if(op==0xc2)for(unsigned n=0;n<2;++n)values[n]=script_weather::bits(static_cast<float>(values[n])); //0042B90C FILD64 with zero high DWORD.
        publish(op,values,true);return true;
    }
private:
    void publish(std::uint32_t op,const std::array<std::uint32_t,4>& v,bool presentation){
        if(!presentation||frames_.has_scene())switch(op){
        case 0xb8:runtime_.script_weather_parameter(0,v[0]);break;
        case 0xb9:runtime_.script_weather_parameter(10,v[0]);break;
        case 0xba:runtime_.script_weather_parameter(5,v[0]);break;
        case 0xbd:runtime_.script_weather_parameter(6,v[0]);runtime_.script_weather_parameter(7,v[1]);break;
        case 0xbe:runtime_.script_weather_parameter(3,v[0]);break;
        case 0xbf:runtime_.script_weather_parameter(4,v[0]);break;
        case 0xc1:runtime_.script_weather_parameter(14,v[0]);break;
        case 0xc2:runtime_.script_weather_parameter(8,v[0]);runtime_.script_weather_parameter(9,v[1]);break;
        case 0xc3:runtime_.script_weather_parameter(11,v[0]);runtime_.script_weather_parameter(12,v[1]);runtime_.script_weather_parameter(13,v[2]);break;
        default:throw std::logic_error("Unknown weather publication opcode");
        }
        if(presentation&&op==0xb8)runtime_.script_weather_splashes(v[0]!=0);
        if(presentation&&op==0xc1)runtime_.script_weather_splashes(v[0]==0);
    }
    Frames& frames_;Runtime& runtime_;SessionBroadcastPackets& packets_;std::uint32_t& mode_;
};
} //namespace hd2::game
#endif
