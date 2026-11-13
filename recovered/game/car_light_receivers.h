#ifndef HD2_GAME_CAR_LIGHT_RECEIVERS_H
#define HD2_GAME_CAR_LIGHT_RECEIVERS_H

#include "game/car_lighting.h"

namespace hd2::game {

//004F6080/004F61E0 borrow the SAME process0089C0A4 receiver list. The geometry
//owner stores both single-texture and paired projectors; native5A0 stores only
//the paired entries, identified here by the actual shared light bindings.
template<class Frames>void visit_car_light_receivers(Frames& frames,const CarLighting<Frames>& lighting,
        const std::vector<typename Frames::Frame>& receivers,bool retiring){
    using Frame=typename Frames::Frame;const auto& projectors=lighting.projectors();
    std::vector<bool> shared(projectors.size(),false);
    for(const auto& light:lighting.lights())if(light.shared){if(light.projector==CarLighting<Frames>::none)throw std::logic_error("Car shared light lost its actual projector");shared.at(light.projector)=true;}
    const auto notify=[&](Frame light){for(auto n=receivers.size();n;){const auto receiver=receivers.at(--n);if(retiring)frames.car_light_unregister(receiver,light);else frames.car_light_register(receiver,light);}};
    for(auto n=lighting.lights().size();n;){
        const auto& binding=lighting.lights()[--n];if(binding.projector==CarLighting<Frames>::none)continue;
        const auto light=projectors.at(binding.projector).light.get();if(!light)continue;
        bool paired=false;for(auto i=projectors.size();i;)if(shared[--i]&&projectors[i].light.get()==light){paired=true;break;}
        if(!paired)notify(light);
        if(retiring)frames.link_scene_object(light,Frame{}); //Native detaches each binding, including shared duplicates.
    }
    for(auto n=projectors.size();n;){const auto index=--n;if(!shared[index])continue;
        const auto light=projectors[index].light.get();notify(light);
        if(retiring){if(!light)throw std::runtime_error("Car paired projector has no actual companion light");frames.link_scene_object(light,Frame{});}
    }
}
template<class Frames>void register_car_light_receivers(Frames& frames,const CarLighting<Frames>& lighting,
        const std::vector<typename Frames::Frame>& receivers){visit_car_light_receivers(frames,lighting,receivers,false);}
template<class Frames>void unregister_car_light_receivers(Frames& frames,const CarLighting<Frames>& lighting,
        const std::vector<typename Frames::Frame>& receivers){visit_car_light_receivers(frames,lighting,receivers,true);}
//00500CB0. Native searches ONLY the paired projector records by visual.
template<class Frames>void damage_car_paired_light(Frames& frames,const CarLighting<Frames>& lighting,
        std::vector<std::uint16_t>& flags,std::size_t index,unsigned mask){
    bool found=false;for(const auto& light:lighting.lights())if(light.shared&&light.projector==index){found=true;break;}
    if(!found)return;
    auto& state=flags.at(index);state&=static_cast<std::uint16_t>(mask);
    const auto& projector=lighting.projectors().at(index);const auto visual=projector.visual.get(),light=projector.light.get();
    if(state&1u)frames.car_projector_texture(visual,projector.textures[2].get());
    else if(state&2u)frames.car_projector_texture(visual,projector.textures[1].get());
    frames.update_scene_object(visual);frames.car_light_power(light,1.f);frames.update_scene_object(light);
}
} //namespace hd2::game
#endif
