#ifndef HD2_FRONTEND_GAME_CONFIGURATION_H
#define HD2_FRONTEND_GAME_CONFIGURATION_H
#include "frontend/multiplayer_server.h"
#include "frontend/profile_settings.h"
#include <memory>
#include <string>
namespace hd2::frontend {
//Application+24's ACTUAL configuration DATA, initialized by006B4A60→006AD350.
//This owner supplies no transport/socket/service success. Remaining manager
//activation, network consumers and294 producer/retirement are not exposed here.
class GameConfiguration {
public:
    //0061B440 lifetime of a rotation descriptor: strings44, raw table38, raw
    //table28, string05, string01. Unknown payload bytes/producer stay unexposed.
    struct Rotation {
        std::shared_ptr<const std::string> text1,text5,metadata44;
        std::vector<std::uint8_t> table28,table38;
        ~Rotation(){metadata44.reset();std::vector<std::uint8_t>().swap(table38);std::vector<std::uint8_t>().swap(table28);text5.reset();text1.reset();}
    };
    inline static constexpr std::array<std::int32_t,22> startup_rules{{
        32,20,0,3,30,10,1,1,1,0,1,100,1,1,1,1,0,3,-1,1,0,1
    }}; //Distinct from00618A00's menu presets. Raw native228..27C.
    GameConfiguration(){reset();}
    ~GameConfiguration(){clear_rotations();} //Data-only tail of006AD2B0/006B4DA0; no live294 owner is fabricated.
    GameConfiguration(const GameConfiguration&)=delete;GameConfiguration& operator=(const GameConfiguration&)=delete;
    void reset() { //006AD350 initialized data and reverse descriptor retirement, preserving vector capacity.
        field04=0;name218.clear();password21c=R"(\x01\x02\x03)"; //Twelve literal ASCII bytes at00842ADC, not three control bytes.
        field178=field174=0;flag170=0;server17c.reset();identity08.fill(0);
        style224=0;port220=11001;rules228=startup_rules;rotation290=0;
        while(!rotations284.empty())rotations284.pop_back();
    }
    std::int32_t hud_style224()const{return style224;}
    std::uint32_t hud_teams240()const{return static_cast<std::uint32_t>(rules228[6]);}
    bool crosshair274()const{return rules228[19]!=0;}
    bool car_camera264()const{return rules228[15]!=0;} //004D16A0 reads current configuration264.
    void clear_rotations()noexcept {while(!rotations284.empty())rotations284.pop_back();rotation290=0xffffffffu;}
    std::uint32_t field04=0;std::array<std::uint8_t,0x168> identity08{};
    std::uint8_t flag170=0;std::uint32_t field174=0,field178=0;
    MultiplayerServer server17c;std::string name218,password21c;
    std::uint32_t port220=0;std::int32_t style224=0;std::array<std::int32_t,22> rules228{};
    std::vector<std::unique_ptr<Rotation>> rotations284;std::uint32_t rotation290=0;
    //Native unknown/unwritten storage is represented only where needed with
    //defined zero guards. No original0x298 ABI layout/constructor credit claimed.
};
//006B66D0: resolve current application24 config or mode7 application20 profile73
//on EVERY call. Existing ProfileSettings bytes start at native38, hence73→3B.
inline std::uint8_t hud_crosshair_preference(std::uint32_t mode,const GameConfiguration& game,const ProfileSettings& profile) {
    return (mode&7u)==7u?profile.bytes[0x3b]:static_cast<std::uint8_t>(game.crosshair274());
}
} //namespace hd2::frontend
#endif
