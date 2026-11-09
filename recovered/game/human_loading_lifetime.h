#ifndef HD2_GAME_HUMAN_LOADING_LIFETIME_H
#define HD2_GAME_HUMAN_LOADING_LIFETIME_H
namespace hd2::game {
//Source lifetime dispatch for the actual containing Human owner. No native ABI,
//default successful service or replacement for original derived bodies.
class HumanLoadingLifetime {
public:
    virtual ~HumanLoadingLifetime()=default;
    virtual void activate(bool authority)=0;
    virtual void deactivate(bool authority)=0;
};
} //namespace hd2::game
#endif
