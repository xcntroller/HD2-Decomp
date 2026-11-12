#ifndef HD2_GAME_MOUNTED_WEAPON_CONTROL_H
#define HD2_GAME_MOUNTED_WEAPON_CONTROL_H

#include <array>
#include <cstdint>
#include <memory>
#include <vector>
#include <optional>
#include "game/human_frame_inputs.h"
#include <cmath>

namespace hd2::game {

//004C19D0's native180 control state is embedded at StaticWeapon160 and
//allocated separately for actual Car gun seats. Both retain the same owners.
struct StaticWeaponControl {
    std::array<std::uint32_t,24> fields60{};
    std::vector<std::uint32_t> field10c;
    //Native120/124 are real SOUND owners in MountedWeaponControlLoading.
    std::uint32_t fieldc8=0xffffffffu,fieldcc=0,fieldd0=0,
        field13c=0,field140=0,field144=0,field148=0,field14c=0,field158=0,field16c=0,field170=0;
    float field128=-1,field12c=-1,field130=-1,field134=-1;
    std::uint32_t fieldd8=0,field160=0x3ec90fdb,field164=0x3ec90fdb;
    std::uint32_t fielde0=0,fielde4=0,field154=0; //004C1AD0 clears reload and154 timer.
    std::optional<std::uint32_t> field118,field11c; //Aim writes; ctor/init do not produce these angles.
    bool flagd4=false,flagdc=false,flag104=false,flag168=false;
};

//004C3B60 aim-rate prefix. The caller checks the actual FC occupant first;
//shoot/reload inputs then continue to the separate H6 receivers.
inline void request_mounted_aim_rates(StaticWeaponControl& control,const HumanFrameInputBuffer& first){
    const auto real=[](std::uint32_t bits){float value;std::memcpy(&value,&bits,4);return value;};
    const auto word=[](float value){std::uint32_t bits;std::memcpy(&bits,&value,4);return bits;};
    const auto rate=[&](unsigned offset,std::uint32_t& value,std::uint32_t limit,bool& idle){
        const auto input=first.get<float>(offset);idle=input==0;
        if(!control.flag168)value=word(static_cast<float>(static_cast<long double>(input)*15.f));
        else if(input!=0)value=word(static_cast<float>(static_cast<long double>(input)+real(value)));
        const auto current=real(value),maximum=real(limit);
        if(maximum<std::abs(current))value=current<=0?word(-maximum):limit;
    };
    rate(0xd0,control.fieldcc,control.field160,control.flagd4);
    rate(0xd4,control.fieldd8,control.field164,control.flagdc);
}

class MountedWeaponControlLifetime {
public:
    virtual ~MountedWeaponControlLifetime()=default;
    virtual void fire_marker(void* barrel)=0;
    virtual std::array<float,4> aim_direction()=0;
    virtual std::array<float,4> mounted_position()=0;
    virtual bool can_target(const std::array<float,4>&)=0;
    virtual void frame_inputs(const HumanFrameInputBuffer&,const HumanFrameInputBuffer&)=0;
    virtual std::array<float,4> limits()const=0;
    virtual void view(bool visible)=0;
    virtual void clear_occupant()=0;
    virtual void set_occupant(void*)=0;
    virtual void reset()=0;
    virtual std::optional<std::uint32_t> weapon_kind()=0;
    virtual void advance(bool authority,std::uint32_t elapsed)=0;
    virtual void retire()=0;
};

struct CarMountedWeaponControl {
    StaticWeaponControl state;
    std::unique_ptr<MountedWeaponControlLifetime> loading;
};

} //namespace hd2::game
#endif
