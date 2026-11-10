#ifndef HD2_GAME_CAR_COLD_DEFORMATION_H
#define HD2_GAME_CAR_COLD_DEFORMATION_H

#include "game/car_body_integration.h"
#include "game/car_geometry.h"

namespace hd2::game {

//004FF8B0's actual cold skip gates. Positive mesh deformation, damaged light
//textures, variant switching and puncture/detachment require their real wider
//owners; they are rejected before mutation. No successful deformation service
//is substituted for those receivers. The actual initializer/settler supplies
//the intact parts, light records, wheel state and current cached inverse.
template<class Frames,class Geometry=CarGeometry<Frames>>class CarColdDeformation {
public:
    using Vector=typename Frames::Vector;
    CarColdDeformation(Frames& frames,Geometry& geometry,CarBodyIntegration<Frames>& body,
            std::vector<CarWheelRuntime<Frames>>& wheels,const std::uint32_t& features)
        :frames_(frames),geometry_(geometry),body_(body),wheels_(wheels),features_(features){}
    void operator()(const Vector& point,const Vector&,float radius,float impulse,unsigned flags=0){
        if(!std::isfinite(radius)||!std::isfinite(impulse))throw std::runtime_error("Non-finite Car deformation request");
        const auto& physics=geometry_.physics();const auto& parts=geometry_.deformables().parts();
        const bool damage=(features_&0x4000u)&&!(flags&2u);
        if(damage&&((flags&1u)||physics.scalars().get(0x55c)<impulse)){
            const auto& lights=geometry_.lighting().lights();
            for(auto n=lights.size();n;){const auto& light=lights[--n];
                if((light.flags10&1u)||(light.part1a!=0xffffu&&parts.at(light.part1a).type==5))continue;
                const auto matrix=frames_.actor_world_matrix(light.frame);
                const float x=matrix[12]-point[0],y=matrix[13]-point[1],z=matrix[14]-point[2];
                if((static_cast<long double>(x)*x+static_cast<long double>(y)*y)+static_cast<long double>(z)*z<static_cast<long double>(radius)*radius)
                    throw std::runtime_error("Car light impact requires its actual damage/projector texture receiver");
            }
        }
        const auto& applied=physics.geometry().parts;
        if(parts.size()!=applied.size())throw std::logic_error("Car deformation lost its SAME applied parts");
        for(auto n=parts.size();n;){const auto index=--n;const auto& part=parts[index];
            if(part.flags&1u)continue;
            if((flags&1u)||applied[index].scalars.get(0x18)<impulse)
                throw std::runtime_error("Car positive part deformation requires its actual mesh backup/variant/glass owner");
        }
        if(damage){
            const auto local=frames_.car_transform(point,body_.pose().inverse);
            for(const auto& wheel:wheels_){
                if((wheel.flags&0x40000400u)||!(wheel.scalars.get(0x240)<impulse))continue;
                const auto& authored=wheel.vectors.at(0x30);
                const float x=authored[0]-local[0],y=authored[1]-local[1],z=authored[2]-local[2];
                if((static_cast<long double>(x)*x+static_cast<long double>(y)*y)+static_cast<long double>(z)*z<1.f)
                    throw std::runtime_error("Car wheel impact requires its actual damage/puncture/detachment owner");
            }
        }
    }
private:
    Frames& frames_;Geometry& geometry_;CarBodyIntegration<Frames>& body_;
    std::vector<CarWheelRuntime<Frames>>& wheels_;const std::uint32_t& features_;
};

} //namespace hd2::game
#endif
