#ifndef HD2_GAME_MOUNTED_WEAPON_CONTROL_H
#define HD2_GAME_MOUNTED_WEAPON_CONTROL_H

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

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
    std::uint32_t fielde0=0,fielde4=0; //004C1AD0 clears;004C7670 reads actual reload progress.
    bool flagd4=false,flagdc=false,flag104=false,flag168=false;
};

class MountedWeaponControlLifetime {
public:
    virtual ~MountedWeaponControlLifetime()=default;
    virtual void fire_marker()=0;
    virtual std::array<float,4> aim_direction()=0;
    virtual std::array<float,4> limits()const=0;
    virtual void view(bool visible)=0;
    virtual void clear_occupant()=0;
    virtual void retire()=0;
};

struct CarMountedWeaponControl {
    StaticWeaponControl state;
    std::unique_ptr<MountedWeaponControlLifetime> loading;
};

} //namespace hd2::game
#endif
