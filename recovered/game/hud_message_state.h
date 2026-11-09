#ifndef HD2_GAME_HUD_MESSAGE_STATE_H
#define HD2_GAME_HUD_MESSAGE_STATE_H
#include <array>
#include <cstdint>
namespace hd2::game {
//00494980 at HUD+2F90, then00495F40 loading of that SAME receiver. Fields
//not written by the constructor retain defined cold guards until loading.
struct HudMessageState {
    std::array<float,3> colours{{255.f,255.f,255.f}};
    std::uint32_t field10=0,word14=0,value2c=50,fieldd37c=0,fieldd36c=0,field168=0;
    std::array<std::uint32_t,4> words30{};
    float width40=0,height44=0;std::array<float,4> first48{},second58{};
    std::uint32_t heightd370=0x3cb43958,time_d374=0,time_d378=0,caret_d380=0;
    std::array<char,256> input68{}; //First BYTE is the original flag68/string[0], one owner.
    std::uint8_t flagd384=1,flagd385=0,flagd386=0,flagd387=0;
};
} //namespace hd2::game
#endif
