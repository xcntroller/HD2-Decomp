#ifndef HD2_GAME_CAR_LOADING_LIGHTS_H
#define HD2_GAME_CAR_LOADING_LIGHTS_H

#include "game/car_lighting.h"

namespace hd2::game {

struct CarLoadingLightState {
    std::uint32_t flags=0,previous=0; //004EA780's5B0/5B4.
    float blink_elapsed=0,hazard_elapsed=0; //004E2900's5C0/5CC.
    float blink_on=.4000000059604645f,blink_off=.30000001192092896f,hazard_interval=.30000001192092896f; //5B8/5BC/5C8.
    float steering_threshold=.20000000298023224f; //5C4, replaced by004D3110 before presentation.
};
struct CarLoadingLightInputs {std::int32_t gear;float brake,steering;};

//0041D200 and00501CB0's inlined world-forward normalization. Preserve W,
//extended near-unit comparison, stored square for sqrt and unspilled reciprocal.
//The inlined paths sum X/Z/Y; the helper sums Y/Z/X.
template<class Vector>Vector car_light_normalize(Vector value,bool inlined){
    const auto x=static_cast<long double>(value[0])*value[0];
    const auto y=static_cast<long double>(value[1])*value[1];
    const auto z=static_cast<long double>(value[2])*value[2];
    const auto sum=inlined?(x+z)+y:(y+z)+x;const float square=static_cast<float>(sum);
    constexpr float epsilon=9.99999993922529e-09f;
    if(std::abs(sum-1)>=epsilon){
        if(square>=epsilon){const auto factor=1/std::sqrt(static_cast<long double>(square));for(unsigned axis=0;axis<3;++axis)value[axis]=static_cast<float>(value[axis]*factor);}
        else{const auto axis=value[0]!=0?0u:value[2]!=0?2u:1u;value[axis]=value[axis]<0?-1.f:1.f;}
    }
    return value;
}

//00501CB0's full nine-family policy. Geometry, sector544, physical240 and
//paired5A4 flags are the SAME owners used by initialization/damage/reset.
//No light-command producer or successful rendering backend is manufactured.
template<class Frames>void update_car_loading_lights(Frames& frames,const CarLighting<Frames>& lighting,
        CarLoadingLightState& state,float elapsed,std::uint32_t features,std::uint32_t previous_features,
        std::vector<std::uint16_t>& paired_flags,typename Frames::Frame sector,const typename Frames::Vector& forward,
        const CarLoadingLightInputs* inputs=nullptr){
    using Frame=typename Frames::Frame;using Vector=typename Frames::Vector;
    if(!((features|previous_features)&0x80u))return;
    state.hazard_elapsed+=elapsed;
    if(state.flags&0x80u){
        if(state.hazard_interval<state.hazard_elapsed){state.hazard_elapsed=0;state.flags^=0x20000000u;}
        if(state.flags&0x20000000u)state.flags=(state.flags&~0x20u)|0x10u;
        else state.flags=(state.flags&~0x10u)|0x20u;
    }else if(state.previous&0x80u)state.flags&=~0x30u;
    state.blink_elapsed+=elapsed;
    if(state.blink_elapsed>=((state.flags&0x10000000u)?state.blink_on:state.blink_off)){
        state.blink_elapsed=0;state.flags^=0x10000000u;
    }
    if((state.flags&0x8808u)&&!inputs)throw std::logic_error("Car automatic lights require SAME gear/brake/steering inputs");
    if(state.flags&0x8000u){if(inputs->gear<0)state.flags|=0x1000u;else state.flags&=~0x1000u;}
    if(state.flags&0x800u){if(inputs->brake<=0)state.flags&=~0x100u;else state.flags|=0x100u;}
    if(state.flags&8u){
        if(std::abs(inputs->steering)<=state.steering_threshold)state.flags&=~3u;
        else if(inputs->steering<=0)state.flags=(state.flags&~2u)|1u;
        else state.flags=(state.flags&~1u)|2u;
    }
    //Native reverse5A4 loop clears only the transient low-byte bit7. Damage's
    //bits0/1 and the high byte survive; unified unpaired entries are unused.
    for(auto n=paired_flags.size();n;)paired_flags[--n]&=static_cast<std::uint16_t>(~0x80u);
    const auto& projectors=lighting.projectors();
    for(auto n=lighting.lights().size();n;){const auto& light=lighting.lights()[--n];
        const auto type=light.type;const bool shared=(light.flags10&0x80000000u)!=0;
        const auto visual=light.projector==CarLighting<Frames>::none?Frame{}:projectors.at(light.projector).visual.get();
        auto companion=[&](){const auto frame=projectors.at(light.projector).light.get();
            if(!frame)throw std::runtime_error("Car active projector has no actual companion light");
            return frame;};
        auto seen=[&](){return shared&&(paired_flags.at(light.projector)&0x80u);};
        auto hide_projector=[&](){if(visual&&!seen())frames.object_visible(visual,false);};
        auto hide_both=[&](){if(visual&&!seen()){frames.object_visible(visual,false);frames.object_visible(companion(),false);}};
        auto show=[&](Frame target,bool xyz_only,bool car_forward){
            if(frames.object_parent(target)!=sector)frames.car_reset_link(target,sector);
            if(shared)paired_flags.at(light.projector)|=0x80u;
            const auto source=shared?light.projector_marker:light.frame;
            if(!source)throw std::runtime_error("Car active light lost its actual source marker");
            const auto position=frames.car_world_matrix(source);
            frames.car_light_position(target,Vector{{position[12],position[13],position[14],position[15]}},xyz_only);
            if(car_forward)frames.car_light_direction(target,forward);
            else{const auto world=frames.car_world_matrix(source);
                frames.car_light_direction(target,car_light_normalize(Vector{{world[8],world[9],world[10],world[11]}},xyz_only));}
            frames.object_visible(target,true);frames.update_scene_object(target);
        };
        bool active=false;std::uint32_t family=0;bool indicator=false;
        switch(type){
        case 1:case 2:indicator=true;family=0x100000u;active=(state.flags&(type==1?1u:2u))&&(state.flags&0x10000000u);break;
        case 3:case 4:indicator=(state.flags&(type==3?1u:2u))!=0;family=0x400000u;
            active=indicator?(state.flags&0x10000000u)!=0:(state.flags&0x100u)!=0;break;
        case 5:case 6:family=0x200000u;active=(state.flags&(type==5?0x10u:0x20u))!=0;break;
        case 7:family=0x800000u;active=(state.flags&0x10000u)!=0;break;
        case 8:family=0x1000000u;active=(state.flags&0x100u)!=0;break;
        case 9:family=0x2000000u;active=(state.flags&0x1000u)!=0;break;
        default:break;
        }
        if(type>=1&&type<=9){
            active=active&&!(light.flags10&1u)&&(features&0x80u);
            if(!active){if(light.billboard)frames.object_visible(light.billboard,false);hide_both();}
            else{
                if(light.billboard)frames.object_visible(light.billboard,true);
                if((state.flags&family)&&visual){
                    //Early native cases inline XYZ writes; helper0041D110 in
                    //the remaining paths copies all four position words.
                    const bool inline_visual=type<=3;
                    const bool inline_companion=type<=2||(type==3&&indicator);
                    if(features&0x100u)show(visual,inline_visual,false);else hide_projector();
                    if(features&0x1000u)show(companion(),inline_companion,type==7&&shared);
                    else if(type==7){if(!seen())frames.object_visible(companion(),false);}
                    else hide_projector(); //Original hides VISUAL here, preserving companion state.
                }
            }
        }
        if(light.billboard)frames.update_scene_object(light.billboard);
    }
    state.previous=state.flags;
}
} //namespace hd2::game
#endif
