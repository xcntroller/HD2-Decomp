#ifndef HD2_GAME_STATIC_WEAPON_LOADING_LIFETIME_H
#define HD2_GAME_STATIC_WEAPON_LOADING_LIFETIME_H
#include <cstdint>
#include <array>
#include <optional>
#include "game/human_frame_inputs.h"
namespace hd2::game {
class StaticWeaponLoadingLifetime {
public:
    virtual ~StaticWeaponLoadingLifetime()=default;
    virtual void activate(bool authority)=0;
    virtual void deactivate(bool authority)=0;
    virtual void update(bool authority,std::uint32_t elapsed)=0;
    virtual void fire_marker(void* barrel)=0;
    virtual std::array<float,4> aim_direction()=0;
    virtual std::array<float,4> mounted_position()=0;
    virtual bool can_target(const std::array<float,4>&)=0;
    virtual void seat_frame_inputs(std::int32_t,const HumanFrameInputBuffer&,const HumanFrameInputBuffer&)=0;
    virtual void remove_reaction_seat(std::int32_t index)=0;
    virtual void* seat_occupant(std::int32_t index)const=0;
    virtual void assign_seat(std::int32_t index,void* human)=0;
    virtual bool exit_seat(std::int32_t index)=0;
    virtual std::optional<std::uint32_t> seat_weapon_kind()=0;
    virtual bool seat_claimed(std::int32_t)const=0;
    virtual void claim_seat(std::int32_t,bool)=0;
    virtual std::uint32_t reaction_seat_flags(std::int32_t index)const=0;
};
}
#endif
