#ifndef HD2_GAME_FOWL_LOADING_LIFETIME_H
#define HD2_GAME_FOWL_LOADING_LIFETIME_H
#include <cstddef>
#include <cstdint>
namespace hd2::game {
class FowlLoadingLifetime {
public:
    virtual ~FowlLoadingLifetime()=default;
    virtual void activate(bool)=0;
    virtual void deactivate(bool)=0;
    virtual void update_presentation(std::uint32_t)=0;
    virtual void update_authority(std::uint32_t)=0;
    virtual void launch(bool,std::int32_t)=0;
    virtual void heard(std::uint32_t,float)=0;
    virtual std::size_t bird_count()const=0;
};
} //namespace hd2::game
#endif
