#ifndef HD2_GAME_SCENE_LIGHT_SAMPLE_H
#define HD2_GAME_SCENE_LIGHT_SAMPLE_H

#include "game/car_contact_callbacks.h"
#include <algorithm>

namespace hd2::game {

//Actual cached light inputs read by0069E9D0. These are live engine values,
//including colour1A0/position224/direction230, not serialized scene snapshots.
struct SceneLightSample {
    CarContactVector colour{},position{},direction{},attenuation{};
    float range=0,outer_cosine=0,inner_cosine=0,cone_scale=0;
    std::uint32_t kind=0,flags=0;bool visible=false;
};

//0069E9D0, used twice by0050B180 to compute shadowed/unshadowed intensity.
//An absent sector returns the original zero. The SAME scene's light list and
//world collision queries are mandatory; a missing backend is not darkness.
template<class Frames,class Queries>long double sample_scene_light(Frames& frames,Queries& queries,
        const typename Frames::Vector& position,typename Frames::Frame sector,bool shadows,unsigned game_kind){
    using Vector=typename Frames::Vector;
    if(!sector){sector=frames.human_scene_sector(position);if(!sector)return 0;}
    const std::uint32_t layers=game_kind==1||game_kind==5||game_kind==6?0x20c101u:0x20c301u;
    const auto dot=[](const Vector& a,const Vector& b){return (static_cast<long double>(a[0])*b[0]+
        static_cast<long double>(a[1])*b[1])+static_cast<long double>(a[2])*b[2];};
    float ambient=0,direct=0;
    for(auto n=frames.human_sector_light_count(sector);n;){
        const auto light=frames.human_sector_light(sector,--n);const auto source=frames.scene_light_sample(light);
        if(!source.visible||(source.flags&0x41u)!=0x41u)continue;
        float amount=static_cast<float>((static_cast<long double>(source.colour[0])+source.colour[0])*.30000001192092896f+
            (static_cast<long double>(source.colour[1])+source.colour[1])*.6000000238418579f+
            (static_cast<long double>(source.colour[2])+source.colour[2])*.10000000149011612f);
        switch(source.kind){case 0:case 5:case 6:case 7:continue;
        case 4:ambient+=amount;continue;
        default:break;}
        if(source.kind==1||source.kind==2){
            Vector delta{};for(unsigned axis=0;axis!=3;++axis)delta[axis]=position[axis]-source.position[axis];
            const float squared=static_cast<float>(dot(delta,delta));
            if(source.kind==2){
                auto direction=delta;
                if(std::abs(static_cast<long double>(squared)-1)>=9.99999993922529e-09f){
                    if(squared>=9.99999993922529e-09f){const auto scale=1/std::sqrt(static_cast<long double>(squared));for(unsigned axis=0;axis!=3;++axis)direction[axis]=static_cast<float>(direction[axis]*scale);}
                    else{const auto axis=direction[0]!=0?0u:direction[2]!=0?2u:1u;direction[axis]=direction[axis]<0?-1.f:1.f;}
                }
                const float cone=static_cast<float>(dot(direction,source.direction));
                if(!(source.outer_cosine<cone))continue;
                if(cone<=source.inner_cosine)amount=static_cast<float>((static_cast<long double>(cone)-source.outer_cosine)*source.cone_scale*amount);
            }
            std::int32_t bits;std::memcpy(&bits,&squared,4);
            const auto distance=bits<0x322bcc77?0.f:std::abs(static_cast<long double>(squared)-1)<9.99999993922529e-09f?1.f:
                static_cast<float>(std::sqrt(static_cast<long double>(squared)));
            if(!(distance<source.range))continue;
            amount=static_cast<float>(amount/((static_cast<long double>(distance)*source.attenuation[1]+
                static_cast<long double>(distance)*distance*source.attenuation[2])+source.attenuation[0]));
        }
        if(shadows){
            Vector displacement{};
            if(source.kind==3){for(unsigned axis=0;axis!=3;++axis)displacement[axis]=-source.direction[axis]*100.f;}
            else{const auto world=frames.car_world_matrix(light);for(unsigned axis=0;axis!=3;++axis)displacement[axis]=world[12+axis]-position[axis];}
            const auto hit=queries.ray(position,displacement,layers,0x910000u,false);
            if(hit){const auto surface=hit->flags>>24;if(surface!=0x4du&&surface!=0x4cu)continue;}
        }
        direct+=amount;
    }
    return std::clamp(static_cast<long double>(ambient)+direct,0.L,1.L); //DOUBLE0080FB00 is1.
}
} //namespace hd2::game
#endif
