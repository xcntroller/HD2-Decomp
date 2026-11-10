#ifndef HD2_GAME_CAR_SEAT_CAMERA_H
#define HD2_GAME_CAR_SEAT_CAMERA_H

#include "game/car_geometry_math.h"

namespace hd2::game {

//Actual process FLOAT0089B0F8. This is not MissionCamera's field258. The
//original Car camera writes it only when the current Human is attached here.
struct CarSeatCameraShared {float player_smoothing=0;};

//00465F00 and00466110's Car(kind4) branch. The real Car runtime supplies
//current direction/velocity/speed; Players supplies the current attachment.
//Queries must borrow the actual world's tree/buckets. No fabricated pose,
//empty collision service, current player or speed is supplied here.
template<class Frames,class Actor>class CarSeatCamera {
public:
    using Frame=typename Frames::Frame;using Vector=typename Frames::Vector;
    CarSeatCamera(Frames& frames,Actor& actor,Frame marker)
        :frames_(frames),actor_(actor),marker_(marker){
        if(!marker_)return; //Original null marker constructor leaves unused pose fields alone.
        rotation_=frames_.car_rotation(marker_);local_=frames_.actor_position(marker_);
        const auto parent=frames_.object_parent(marker_);
        if(!parent)throw std::runtime_error("Car seat camera requires its actual parent");
        parent_position_=position(frames_.actor_world_matrix(parent));
        world_position_=position(frames_.actor_world_matrix(marker_));
    }
    template<class Queries>bool update(Queries& queries,const Vector& direction,const Vector& velocity,
            float speed,const Actor* current_attachment,CarSeatCameraShared& shared){
        if(!marker_)return false;
        const auto parent=frames_.object_parent(marker_);
        if(!parent)throw std::runtime_error("Car seat camera lost its actual parent");
        auto origin=position(frames_.actor_world_matrix(parent));
        const Vector displacement{{0,local_[1],0,0}};
        const auto hit=queries.sweep(origin,displacement,.30000001192092896f,0u,0xffffffffu,false);
        origin[1]=hit?hit->position[1]:origin[1]+local_[1];
        frames_.car_camera_position(marker_,origin); //Actual frame virtual08, not a direct local-position write.
        Vector motion{};
        for(unsigned axis=0;axis!=3;++axis){
            const auto half=static_cast<long double>(direction[axis])*.5f;
            motion[axis]=static_cast<float>((axis==2?static_cast<long double>(static_cast<float>(half)):half)+velocity[axis]);
        }
        motion[1]=0;
        motion=normalize(motion,static_cast<long double>(motion[0])*motion[0]+static_cast<long double>(motion[2])*motion[2]);
        float amount=static_cast<float>((static_cast<long double>(speed)+2.5f)*.30000001192092896f);
        if(snap_){snap_=false;amount=1000.f;}
        if(current_attachment==&actor_)shared.player_smoothing=amount;
        const auto world=frames_.actor_world_matrix(marker_);
        auto axis=normalize(Vector{{world[8],world[9],world[10],world[11]}},
            (static_cast<long double>(world[9])*world[9]+static_cast<long double>(world[10])*world[10])+static_cast<long double>(world[8])*world[8]);
        auto planar=axis;planar[1]=0;
        planar=normalize(planar,static_cast<long double>(planar[0])*planar[0]+static_cast<long double>(planar[2])*planar[2]);
        const auto dot=(static_cast<long double>(planar[1])*motion[1]+static_cast<long double>(planar[0])*motion[0])+static_cast<long double>(motion[2])*planar[2];
        if(dot<.9998999834060669f||axis[1]!=.30000001192092896f){
            motion[1]=-.30000001192092896f;
            const auto square=(static_cast<long double>(motion[0])*motion[0]+static_cast<long double>(motion[2])*motion[2])+.09000000357627869f;
            motion=normalize(motion,square);frames_.car_camera_direction(marker_,motion,0.f);
        }
        frames_.update_scene_object(marker_);return true;
    }
    Frame marker()const{return marker_;}
    const std::array<float,4>& initial_rotation()const{return rotation_;}
    const Vector& initial_local_position()const{return local_;}
    const Vector& initial_parent_position()const{return parent_position_;}
    const Vector& initial_world_position()const{return world_position_;}
    bool& snap_next_update(){return snap_;} //Native byte48, actual later caller owns its producer.
private:
    template<class Matrix>static Vector position(const Matrix& matrix){return {{matrix[12],matrix[13],matrix[14],matrix[15]}};}
    static Vector normalize(Vector value,long double square){
        constexpr float epsilon=9.99999993922529e-09f;
        if(!std::isfinite(square))throw std::runtime_error("Non-finite Car seat camera direction");
        const float stored=static_cast<float>(square);
        if(std::abs(square-1)>=epsilon){
            if(stored>=epsilon){const auto scale=1/std::sqrt(static_cast<long double>(stored));for(unsigned axis=0;axis!=3;++axis)value[axis]=static_cast<float>(value[axis]*scale);}
            else{const auto axis=value[0]!=0?0u:value[2]!=0?2u:1u;value[axis]=value[axis]<0?-1.f:1.f;}
        }
        return value;
    }
    Frames& frames_;Actor& actor_;Frame marker_{};std::array<float,4> rotation_{};
    Vector local_{},parent_position_{},world_position_{};bool snap_=false;
};
} //namespace hd2::game
#endif
