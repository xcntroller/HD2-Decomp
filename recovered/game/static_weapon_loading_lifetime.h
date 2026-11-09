#ifndef HD2_GAME_STATIC_WEAPON_LOADING_LIFETIME_H
#define HD2_GAME_STATIC_WEAPON_LOADING_LIFETIME_H
#include <cstdint>
namespace hd2::game {
class StaticWeaponLoadingLifetime {
public:
    virtual ~StaticWeaponLoadingLifetime()=default;
    virtual void activate(bool authority)=0;
    virtual void deactivate(bool authority)=0;
    virtual void update(bool authority,std::uint32_t elapsed)=0;
    virtual void fire_marker()=0;
};
}
#endif
