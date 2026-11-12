#ifndef HD2_GAME_CAR_COLD_DEFORMATION_H
#define HD2_GAME_CAR_COLD_DEFORMATION_H

#include "game/car_body_integration.h"
#include "game/car_geometry.h"

namespace hd2::game {

//004FF8B0's mesh capture/deformation uses the SAME part owner and shared CRT.
//Reached light/health/variant/wheel damage remains required after its actual
//earlier mutations; no successful damage service substitutes for those bodies.
template<class Frames,class Geometry=CarGeometry<Frames>>class CarColdDeformation {
public:
    using Vector=typename Frames::Vector;
    template<class Random>CarColdDeformation(Frames& frames,Geometry& geometry,CarBodyIntegration<Frames>& body,
            std::vector<CarWheelRuntime<Frames>>& wheels,const std::uint32_t& features,std::uint32_t& dirty,
            Random& random,const std::function<void()>& notify)
        :frames_(frames),geometry_(geometry),body_(body),wheels_(wheels),features_(features),dirty_(dirty),
         random_([&random]{return random();}),notify_(notify){}
    void operator()(const Vector& point,const Vector& direction,float radius,float impulse,unsigned flags=0){
        if(!std::isfinite(radius)||!std::isfinite(impulse))throw std::runtime_error("Non-finite Car deformation request");
        const auto& physics=geometry_.physics();auto& deformables=geometry_.deformables();auto& parts=deformables.parts();
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
            if(!((flags&1u)||applied[index].scalars.get(0x18)<impulse))continue;
            const auto impact=deformables.deform_mesh(frames_,geometry_.resource(),index,applied[index],point,direction,radius,impulse,random_);
            if(!impact)continue;
            if(notify_)notify_(); //Installed6B0 is genuine empty004D49F0; before dirty/rebuild.
            if(part.type==2)impulse=impact.absorbed;
            dirty_|=1u;deformables.finish_mesh_impact(frames_,index,impact);
            if(damage)throw std::runtime_error("Car deformed part requires its actual health/variant/glass damage consumers");
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
    std::uint32_t& dirty_;std::function<int()> random_;const std::function<void()>& notify_;
};

} //namespace hd2::game
#endif
