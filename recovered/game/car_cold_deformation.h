#ifndef HD2_GAME_CAR_COLD_DEFORMATION_H
#define HD2_GAME_CAR_COLD_DEFORMATION_H

#include "game/car_body_integration.h"
#include "game/car_geometry.h"

namespace hd2::game {

//004FF8B0's mesh capture/deformation uses the SAME part owner and shared CRT.
//Damage borrowers below bind the actual containing light/part/wheel consumers;
//an unbound reached consumer fails instead of accepting a geometry-only hit.
template<class Frames,class Geometry=CarGeometry<Frames>>class CarColdDeformation {
public:
    using Vector=typename Frames::Vector;
    template<class Random>CarColdDeformation(Frames& frames,Geometry& geometry,CarBodyIntegration<Frames>& body,
            std::vector<CarWheelRuntime<Frames>>& wheels,const std::uint32_t& features,std::uint32_t& dirty,
            Random& random,const std::function<void()>& notify,
            std::function<void(std::size_t,float)> light_damage={},
            std::function<void(std::size_t,float,const Vector&,const Vector*)> part_damage={},
            std::function<void(std::size_t,float)> wheel_damage={})
        :frames_(frames),geometry_(geometry),body_(body),wheels_(wheels),features_(features),dirty_(dirty),
         random_([&random]{return random();}),notify_(notify),light_damage_(std::move(light_damage)),part_damage_(std::move(part_damage)),wheel_damage_(std::move(wheel_damage)){}
    void operator()(const Vector& point,const Vector& direction,float radius,float impulse,unsigned flags=0,const Vector* incoming=nullptr){
        if(!std::isfinite(radius)||!std::isfinite(impulse))throw std::runtime_error("Non-finite Car deformation request");
        const auto& physics=geometry_.physics();auto& deformables=geometry_.deformables();auto& parts=deformables.parts();
        const auto damage=[&]{return (features_&0x4000u)&&!(flags&2u);};
        if(damage()&&((flags&1u)||physics.scalars().get(0x55c)<impulse)){
            const auto& fields=physics.scalars();
            const float amount=static_cast<float>(((static_cast<long double>(impulse)-
                (static_cast<long double>(fields.get(0x568))*.009999999776482582f)*impulse)/fields.get(0x558))*fields.get(0x560));
            const auto& lights=geometry_.lighting().lights();
            for(auto n=lights.size();n;){const auto& light=lights[--n];
                if((light.flags10&1u)||(!(light.part1a&0x8000u)&&parts.at(light.part1a).type==5))continue;
                const auto matrix=frames_.actor_world_matrix(light.frame);
                const auto x=static_cast<long double>(matrix[12])-point[0],y=static_cast<long double>(matrix[13])-point[1],z=static_cast<long double>(matrix[14])-point[2];
                if((z*z+y*y)+x*x<static_cast<long double>(radius)*radius){
                    if(!light_damage_)throw std::runtime_error("Car light impact requires its actual damage/projector texture receiver");
                    light_damage_(n,amount);
                }
            }
        }
        const auto& applied=physics.geometry().parts;
        if(parts.size()!=applied.size())throw std::logic_error("Car deformation lost its SAME applied parts");
        for(auto n=parts.size();n;){const auto index=--n;const auto& part=parts[index];
            if(part.flags&1u)continue;
            if(!((flags&1u)||applied[index].scalars.get(0x18)<impulse))continue;
            const auto impact=deformables.deform_mesh(frames_,geometry_.resource(),index,applied[index],point,direction,radius,impulse,random_);
            if(!impact)continue;
            if(notify_)notify_(); //Installed6B0 is genuine empty004D49F0; before dirty/rebuild.
            if(part.type==2)impulse=impact.absorbed;
            dirty_|=1u;deformables.finish_mesh_impact(frames_,index,impact);
            if(damage()){
                if(!part_damage_)throw std::runtime_error("Car deformed part requires its actual health/variant/glass damage consumers");
                part_damage_(index,impact.absorbed,direction,incoming);
            }
        }
        if(damage()){
            const auto local=frames_.car_transform(point,body_.pose().inverse);
            for(auto n=wheels_.size();n;){const auto index=--n;const auto& wheel=wheels_[index];
                if((wheel.flags&0x40000400u)||!(wheel.scalars.get(0x240)<impulse))continue;
                const auto& authored=wheel.vectors.at(0x30);
                const auto x=static_cast<long double>(authored[0])-local[0],y=static_cast<long double>(authored[1])-local[1],z=static_cast<long double>(authored[2])-local[2];
                if((z*z+y*y)+x*x<1.f){
                    if(!wheel_damage_)throw std::runtime_error("Car wheel impact requires its actual damage/puncture/detachment owner");
                    wheel_damage_(index,impulse);
                }
            }
        }
    }
private:
    Frames& frames_;Geometry& geometry_;CarBodyIntegration<Frames>& body_;
    std::vector<CarWheelRuntime<Frames>>& wheels_;const std::uint32_t& features_;
    std::uint32_t& dirty_;std::function<int()> random_;const std::function<void()>& notify_;
    std::function<void(std::size_t,float)> light_damage_;
    std::function<void(std::size_t,float,const Vector&,const Vector*)> part_damage_;
    std::function<void(std::size_t,float)> wheel_damage_;
};

} //namespace hd2::game
#endif
