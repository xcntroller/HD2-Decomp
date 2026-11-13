#ifndef HD2_GAME_CAR_PART_VARIANTS_H
#define HD2_GAME_CAR_PART_VARIANTS_H
#include "game/car_body_integration.h"
#include <limits>

namespace hd2::game {
template<class Frames>CarContactVector car_part_damage_velocity(Frames& frames,CarBodyIntegration<Frames>& body,
        typename Frames::Frame frame,bool spill_offset=true){
    const auto world=frames.actor_world_matrix(frame);std::array<long double,3> offset{},cross{};
    for(unsigned a=0;a<3;++a){offset[a]=static_cast<long double>(world[12+a])-body.pose().centre[a];if(spill_offset)offset[a]=static_cast<float>(offset[a]);}
    const auto& angular=body.motion().angular_velocity;
    for(unsigned a=0;a<3;++a){const auto n=(a+1)%3,l=(a+2)%3;
        cross[a]=angular[n]*offset[l]-angular[l]*offset[n];if(spill_offset||a<2)cross[a]=static_cast<float>(cross[a]);}
    auto result=body.motion().velocity;for(unsigned a=0;a<3;++a)result[a]=static_cast<float>(result[a]-cross[a]);return result;
}
//004FEFD0, RET12: index, optional impulse, allow child detachment. Geometry,
//Door references, lights and callback borrowers remain with their real owners.
template<class Frames,class Geometry,class Runtime>void update_car_part_variant(Frames& frames,Geometry& geometry,
        CarBodyIntegration<Frames>& body,std::size_t index,const CarContactVector* impulse,bool detach,Runtime& runtime){
    auto& parts=geometry.deformables().parts();
    if(index<parts.size()&&parts[index].type==8&&parts[index].health<=0){
        for(auto n=parts.size();n;){const auto child=--n;auto& part=parts[child];
            if(part.type!=3||(part.flags&1u)||frames.object_parent(part.frame)!=parts[index].frame)continue;
            part.health=0;part.flags|=2u;runtime.variant_glass(child,car_part_damage_velocity(frames,body,part.frame));
        }
    }
    auto& part=parts.at(index);if(!part.variants)return;
    auto* door=geometry.auxiliary().part_door(index);const auto count=part.alternatives.size();
    const auto& applied=geometry.physics().geometry().parts.at(index);
    const auto interval=1.L/(static_cast<long double>(count)+1);
    const auto scaled=(static_cast<long double>(part.health)/applied.scalars.get(0x1c))*(1.L/interval);
    if(!std::isfinite(scaled)||scaled<-2147483648.L||scaled>=2147483648.L||count>0x7fffffffu)
        throw std::runtime_error("Car variant selection exceeds native FTOL range");
    const auto selected=static_cast<std::int64_t>(count)-static_cast<std::int32_t>(std::trunc(scaled));
    if(selected<=0)return;
    for(auto n=count;n;){const auto alternative=--n;const auto frame=part.alternatives[alternative];
        if(!frame)throw std::runtime_error("Car selected variant has no actual frame");
        if(static_cast<std::int64_t>(alternative)==selected-1){
            frames.object_visible(frame,part.health>0||part.type==1);frames.car_copy_variant_position(frame,part.frame);frames.update_scene_object(frame);
            if(door)door->frame=frame; //AFTER selected frame24, before later alternatives/base hide.
        }else{frames.object_visible(frame,false);frames.update_scene_object(frame);}
    }
    frames.object_visible(part.frame,false); //Native does not call base24 here.
    for(auto n=geometry.lighting().lights().size();n;){auto& light=geometry.lighting().lights()[--n];
        if(light.part1a!=index&&frames.object_parent(light.frame)!=part.frame)continue;
        if(light.flags10&1u)continue;
        light.field14=0;light.flags10|=1u;runtime.variant_light(light);
    }
    for(auto n=parts.size();n;){const auto child=--n;auto& value=parts[child];
        if((value.flags&1u)||frames.object_parent(value.frame)!=part.frame)continue;
        if(value.variants){
            for(auto m=value.alternatives.size();m;){const auto alternative=--m;const auto frame=value.alternatives[alternative];
                if(!frame)throw std::runtime_error("Car child variant has no actual frame");
                if(alternative==value.alternatives.size()-1){
                    frames.object_visible(frame,value.health>0||value.type==1);frames.car_copy_variant_position(frame,value.frame);
                }else frames.object_visible(frame,false);
                frames.update_scene_object(frame);
            }
            frames.object_visible(value.frame,false);
        }
        value.health=0;value.flags|=2u;
        if(value.type==3)runtime.variant_glass(child,car_part_damage_velocity(frames,body,value.frame));
        else if(detach)runtime.variant_detach(child,car_part_damage_velocity(frames,body,value.frame),impulse);
    }
}
} //namespace hd2::game
#endif
