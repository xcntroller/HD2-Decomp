#ifndef HD2_GAME_ANIMATION_LOADING_LIFETIME_H
#define HD2_GAME_ANIMATION_LOADING_LIFETIME_H
#include <cstdint>
namespace hd2::game {
template<class Frame>class AnimationLoadingLifetime {
public:
    virtual ~AnimationLoadingLifetime()=default;
    virtual void activate(bool)=0;
    virtual void deactivate(bool)=0;
    virtual void update(std::uint32_t)=0;
    virtual Frame model()const=0;
};
} //namespace hd2::game
#endif
