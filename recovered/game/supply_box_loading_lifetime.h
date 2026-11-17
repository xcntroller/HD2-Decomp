#ifndef HD2_GAME_SUPPLY_BOX_LOADING_LIFETIME_H
#define HD2_GAME_SUPPLY_BOX_LOADING_LIFETIME_H
#include <cstdint>
namespace hd2::game {
template<class Frame>class SupplyBoxLoadingLifetime {
public:
    virtual ~SupplyBoxLoadingLifetime()=default;
    virtual void activate(bool)=0;virtual void deactivate(bool)=0;virtual void update(std::uint32_t)=0;
    virtual Frame helper(unsigned)const=0;
    virtual std::uint32_t pickup_flags()const=0;
    virtual void consume_item()=0;
};
} //namespace hd2::game
#endif
