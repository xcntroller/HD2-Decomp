#ifndef HD2_GAME_SUPPLY_BOX_GEOMETRY_H
#define HD2_GAME_SUPPLY_BOX_GEOMETRY_H

#include "game/car_geometry_math.h"
#include <string>
#include <utility>

namespace hd2::game {

//005F98E0 and geometry part of005FA6A0. The containing actor must run00610200
//and retain the model; these frame handles are borrowed. Item production and
//the later SupplyBox resource/effect owners are separate required consumers.
template<class Frames>
class SupplyBoxGeometry {
public:
    using Frame=typename Frames::Frame;
    using Vector=typename Frames::Vector;
    using Rotation=decltype(std::declval<Frames&>().actor_world_rotation(std::declval<Frame>()));
    struct SnapshotOnly {};
    SupplyBoxGeometry(Frames& frames,Frame model) : SupplyBoxGeometry(frames,model,SnapshotOnly{}) {
        complete(frames,model);
    }
    //005FA6A0 takes these snapshots before resetting lists/parameters and
    //calling00610200. The actor calls complete only after that common retain.
    SupplyBoxGeometry(Frames& frames,Frame model,SnapshotOnly) : model_(model) {
        if(!model)throw std::runtime_error("SupplyBox initialization requires a frame");
        const auto world=frames.actor_world_matrix(model);
        for(std::size_t i=0;i!=4;++i)position_[i]=world[12+i];
        rotation_=frames.actor_world_rotation(model);
    }
    void complete(Frames& frames,Frame model) {
        if(model!=model_||completed_)throw std::logic_error("SupplyBox geometry initialization repeated or mismatched");
        wall_=frames.actor_find(model,"Steny02",0x10001);
        if(wall_&&frames.visual_type(wall_)!=0x5f4a424f)wall_=Frame{}; //OBJ_.
        for(unsigned i=0;i!=ammo_.size();++i) {
            const auto name=std::string(i<10?"Ammo0":"Ammo")+std::to_string(i);
            ammo_[i]=frames.actor_find(model,name.c_str(),0x10001);
        }
        if(!wall_)throw std::runtime_error("SupplyBox requires Steny02 object geometry");
        const auto bounds=frames.actor_bounds(model);
        //Original uses half the EXTENT, without adding the minimum Y.
        const auto threshold=(bounds.maximum[1]-bounds.minimum[1])*.5f;
        if(!std::isfinite(threshold))throw std::runtime_error("Non-finite SupplyBox bounds");
        std::size_t selected=0;
        frames.actor_vertices(wall_,[&](std::size_t count,const auto& vertex) {
            for(std::size_t i=0;i!=count;++i) {
                const auto point=vertex(i);
                for(unsigned axis=0;axis!=3;++axis)if(!std::isfinite(point[axis]))
                    throw std::runtime_error("Non-finite SupplyBox vertex");
                if(point[1]<threshold&&selected<corners_.size())corners_[selected++]=point;
            }
        });
        //The original leaves missing corners undefined. Reject an unusable
        //required mesh instead of publishing fabricated collision coordinates.
        if(selected!=corners_.size())throw std::runtime_error("SupplyBox requires four lower vertices");
        if(distance(corners_[0],corners_[2])<distance(corners_[0],corners_[1]))
            std::swap(corners_[1],corners_[2]);
        else if(distance(corners_[0],corners_[3])<distance(corners_[0],corners_[1]))
            std::swap(corners_[1],corners_[3]);
        if(distance(corners_[0],corners_[2])>distance(corners_[0],corners_[3]))
            std::swap(corners_[2],corners_[3]);
        completed_=true;
    }
    Frame wall() const {return wall_;}
    void refresh(Frames& frames){completed_=false;complete(frames,model_);} //005F98E0 is also called at every actual activation.
    const std::array<Frame,14>& ammo_frames() const {return ammo_;}
    const Vector& position() const {return position_;}
    const Rotation& rotation() const {return rotation_;}
    const std::array<Vector,4>& corners() const {return corners_;}
private:
    static float distance(const Vector& a,const Vector& b) { //0040E0B0/inlined.
        const auto x=a[0]-b[0],y=a[1]-b[1],z=a[2]-b[2];
        const auto sum=x*x+y*y+z*z;
        if(!std::isfinite(sum))throw std::runtime_error("Non-finite SupplyBox corner distance");
        constexpr float epsilon=9.99999993922529e-09f;
        return sum<epsilon?0.f:std::abs(sum-1.f)<epsilon?1.f:std::sqrt(sum);
    }
    Frame wall_{};
    Frame model_{};
    bool completed_=false;
    std::array<Frame,14> ammo_{};
    Vector position_{};
    Rotation rotation_{};
    std::array<Vector,4> corners_{};
};

} //namespace hd2::game
#endif
