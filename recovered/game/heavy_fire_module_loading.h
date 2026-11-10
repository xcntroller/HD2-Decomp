#ifndef HD2_GAME_HEAVY_FIRE_MODULE_LOADING_H
#define HD2_GAME_HEAVY_FIRE_MODULE_LOADING_H

#include "frontend/module_registration.h"

namespace hd2::game {

//Process module16. Neither native owned fire-support list190/1A0 has a source
//producer in loading. This cold state does not offer a successful simulation,
//spawn or populated-list cleanup receiver.
class HeavyFireModuleLoading;
class HeavyFireLoadingState {
public:
    const auto& flags()const{return flags_;}
private:
    friend class HeavyFireModuleLoading;
    std::array<std::uint8_t,4> flags_{}; //Native1AC..1AF, process BSS zero.
    const void* world_=nullptr;
};
class HeavyFireModuleLoading {
public:
    HeavyFireModuleLoading(frontend::ClientModuleRegistry& registry,frontend::ClientModuleRegistration& registration,HeavyFireLoadingState& state)
        :registration_(registry,registration,16,"HeavyFireSupportModule"),state_(state){
        if(state_.world_)throw std::logic_error("HeavyFireSupport already belongs to an actual world");
        state_.world_=this;
    }
    ~HeavyFireModuleLoading(){close();}
    HeavyFireModuleLoading(const HeavyFireModuleLoading&)=delete;
    HeavyFireModuleLoading& operator=(const HeavyFireModuleLoading&)=delete;
    auto& registration(){return registration_.get();}
    void finish_loading(bool){require_open();} //004115E0 genuine RET4.
    void activate(bool){require_open();state_.flags_.fill(0);} //00472250, BOTH paths, no mode predicate.
    void finish(bool){require_open();} //00472270: both owned lists are genuinely empty.
    void update(bool,std::uint32_t){require_open();} //00472C60/00472510: SAME unpopulatable cold190/1A0 lists, no live fire-support producer.
    void close()noexcept{if(state_.world_==this)state_.world_=nullptr;}
private:
    void require_open()const{if(state_.world_!=this)throw std::logic_error("HeavyFireSupport world has been retired");}
    frontend::ClientModuleBinding registration_;HeavyFireLoadingState& state_;
};
} //namespace hd2::game
#endif
