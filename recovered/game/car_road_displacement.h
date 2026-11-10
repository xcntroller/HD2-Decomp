#ifndef HD2_GAME_CAR_ROAD_DISPLACEMENT_H
#define HD2_GAME_CAR_ROAD_DISPLACEMENT_H

#include "game/car_wheel_runtime.h"
#include "game/car_definition_tables.h"

namespace hd2::game {

//0050B180's active-body road displacement pass. Uses the SAME applied wheel
//parameters/material registry and process rand stream. Initial26C=1000 selects
//three draws per contacted wheel, even when the material amplitude is zero.
template<class Frames,class Random>void update_car_road_displacement(const CarPhysics& physics,
        const CarMaterialRegistry& materials,std::vector<CarWheelRuntime<Frames>>& wheels,
        float enabled,std::uint32_t& features,Random&& random){
    for(auto n=wheels.size();n;){auto& wheel=wheels[--n];auto& fields=wheel.scalars;
        if(wheel.flags&0x40000400u){features|=0x10u;continue;}
        if(enabled==0||!(wheel.flags&8u))continue;
        const float period=fields.get(0x260)+fields.get(0x25c),distance=fields.get(0x26c);
        if(distance<=period){if(fields.get(0x25c)<=distance)fields.set(0x264,fields.get(0x268));continue;}
        fields.set(0x264,0);
        if(period>0){
            //007E6368 sets x87 control word173F (RC=down), calls FRNDINT,
            //then restores the old word: floor of the DOUBLE division input.
            const auto turns=std::floor(static_cast<double>(static_cast<long double>(distance)/period));
            if(fields.get(0x25c)<=static_cast<long double>(distance)-turns*static_cast<long double>(period))
                fields.set(0x264,fields.get(0x268));
        }
        fields.set(0x26c,0);
        float first_min,first_max,second_min,second_max,amplitude;
        if(physics.flags().at(0x228d)){
            first_min=fields.get(0x248);first_max=fields.get(0x24c);second_min=fields.get(0x250);second_max=fields.get(0x254);amplitude=fields.get(0x258);
        }else{
            const auto& material=materials.responses().at(materials.indices().at(wheel.surface_id));
            first_min=material.field4;first_max=material.field8;second_min=material.fieldc;second_max=material.field10;amplitude=material.field14;
        }
        const auto first=random()%100u;
        fields.set(0x25c,static_cast<float>((static_cast<long double>(first_max)-first_min)*static_cast<float>(first)*.009999999776482582f+first_min));
        const auto second=random()%100u;
        fields.set(0x260,static_cast<float>((static_cast<long double>(second_max)-second_min)*static_cast<float>(second)*.009999999776482582f+second_min));
        const auto third=static_cast<std::int32_t>(random()%200u)-100;
        const auto offset=static_cast<float>(static_cast<long double>(third)*.009999999776482582f*amplitude);
        const float minimum=amplitude*.5f;
        fields.set(0x268,std::abs(offset)<minimum?(offset>0?minimum:-minimum):offset);
    }
}
} //namespace hd2::game
#endif
