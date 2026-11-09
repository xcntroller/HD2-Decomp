#ifndef HD2_GAME_DOOR_PORTAL_BINDING_H
#define HD2_GAME_DOOR_PORTAL_BINDING_H

#include <cmath>
#include <cstdint>
#include <stdexcept>

namespace hd2::game {

//00600A50, reached by00601AC0 after Door properties. Both fields are borrowed
//from the real engine scene. The scene must outlive the containing actor.
template<class Frames>
class DoorPortalBinding {
public:
    using Frame=typename Frames::Frame;
    void refresh(Frames& frames,Frame model,bool enabled) {
        if(!enabled){portal_=Frame{};attachment_=0;return;}
        if(!model)throw std::runtime_error("Door portal binding requires a frame");
        const auto centre=frames.door_sphere_centre(model);
        for(unsigned i=0;i!=3;++i)if(!std::isfinite(centre[i]))throw std::runtime_error("Non-finite Door bounds");
        auto sector=frames.actor_parent(model);unsigned depth=0;
        while(sector&&frames.frame_kind(sector)!=5) {
            if(++depth>0x100000)throw std::runtime_error("Door sector hierarchy exceeds bounds");
            sector=frames.actor_parent(sector);
        }
        if(!sector)sector=frames.door_sector(centre);
        if(!sector)throw std::runtime_error("Door sector lookup returned null");
        auto count=frames.door_portal_count(sector,0);unsigned side=0;
        if(!count){side=1;count=frames.door_portal_count(sector,1);}
        if(count>0x100000)throw std::runtime_error("Door sector portal count exceeds bounds");
        Frame selected{};float nearest=10.f;
        for(std::uint32_t i=0;i!=count;++i) {
            const auto portal=frames.door_portal(sector,side,i);
            if(!portal)throw std::runtime_error("Door sector contains a null portal");
            const auto position=frames.door_portal_centre(portal);
            const auto x=position[0]-centre[0],y=position[1]-centre[1],z=position[2]-centre[2];
            const auto sum=x*x+y*y+z*z;
            if(!std::isfinite(sum))throw std::runtime_error("Non-finite Door portal distance");
            constexpr float epsilon=9.99999993922529e-09f;
            const auto distance=sum<epsilon?0.f:std::abs(sum-1.f)<epsilon?1.f:std::sqrt(sum);
            if(distance<=nearest&&distance<=frames.door_portal_radius(portal)) {selected=portal;nearest=distance;}
        }
        //No candidate preserves the previous binding; only the disabled branch
        //clears it. Equal-distance candidates choose the later entry.
        if(selected){const auto attachment=frames.door_portal_attachment(selected);portal_=selected;attachment_=attachment;}
    }
    Frame portal() const {return portal_;}
    std::uintptr_t attachment() const {return attachment_;}
private:
    Frame portal_{};
    std::uintptr_t attachment_=0;
};

} //namespace hd2::game
#endif
