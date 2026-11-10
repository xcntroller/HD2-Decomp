#ifndef HD2_GAME_CAR_STRIP_UPDATE_H
#define HD2_GAME_CAR_STRIP_UPDATE_H

#include "game/car_strip_connections.h"
#include "game/car_cold_solver.h"

namespace hd2::game {

struct CarStripUpdateState {float left=0,right=0;}; //004EA780's51C/520.

//00506700 uses the actual private meshes/connection indices recovered during
//004E2E30. Positions are transformed through the strip LOCAL matrix, moved to
//the wheel parent's current local Y plus stored offset, then transformed back.
template<class Frames>void update_car_strips(Frames& frames,const CarStripConnections<Frames>& strips,
        const CarPhysics& physics,const CarBodyIntegration<Frames>& body,
        const std::vector<CarWheelRuntime<Frames>>& wheels,const CarDrivetrainState& drive,
        bool wheel_spin,float elapsed,CarStripUpdateState& state){
    using Frame=typename Frames::Frame;using Matrix=typename Frames::Matrix;
    Frame cached{};Matrix local{},inverse{};
    for(const auto* list:{&strips.left(),&strips.right()})for(auto n=list->size();n;){
        const auto& record=(*list)[--n];if(cached!=record.visual){cached=record.visual;local=frames.car_local_matrix(cached);inverse=frames.car_inverse_matrix(local);}
        const auto y=frames.car_position(wheels.at(record.wheel).parent)[1];
        if(record.offsets.size()!=record.vertices.size())throw std::runtime_error("Car strip lost its actual vertex offsets");
        frames.car_update_strip_vertices(record.visual,[&](std::size_t,auto&& get,auto&& set,auto&&){
            for(auto vertex=record.vertices.size();vertex;){--vertex;const auto index=record.vertices[vertex];auto position=get(index);
                frames.car_transform_in_place(position,local);position[1]=y+record.offsets[vertex];frames.car_transform_in_place(position,inverse);set(index,position);
            }
        });
    }
    for(unsigned side=0;side!=2;++side){
        const auto& list=side?strips.right():strips.left();if(list.empty())continue;
        float velocity;
        if(!wheel_spin){
            const auto& wheel=wheels.at(wheels.size()/2+(side?0:1));const auto& motion=wheel.vectors.at(0xa0);const auto& forward=body.pose().forward;
            velocity=static_cast<float>((static_cast<long double>(forward[0])*motion[0]+static_cast<long double>(forward[1])*motion[1])+static_cast<long double>(forward[2])*motion[2]);
        }else{velocity=static_cast<float>((static_cast<long double>(drive.engine_speed)/car_gear_ratio(physics,drive.gear))*physics.scalars().get(0x8d8));if(drive.gear<0)velocity=-velocity;}
        float amount=static_cast<float>(static_cast<long double>(velocity)*physics.scalars().get(0x518)*elapsed);
        if(amount>1)amount=static_cast<float>(static_cast<long double>(amount)-std::floor(static_cast<double>(amount)));
        else if(amount < -1)amount=static_cast<float>(static_cast<long double>(amount)-std::ceil(static_cast<double>(amount)));
        (side?state.right:state.left)=amount;Frame previous{};
        for(auto n=list.size();n;){const auto frame=list[--n].visual;if(frame==previous)continue;previous=frame;frames.car_scroll_strip(frame,amount);}
    }
}
} //namespace hd2::game
#endif
