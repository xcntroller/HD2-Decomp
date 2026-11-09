#ifndef HD2_GAME_RIGID_BODY_BOUNDS_H
#define HD2_GAME_RIGID_BODY_BOUNDS_H

#include <array>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <unordered_set>

namespace hd2::game {

//004C7DB0/004C7E50. The root's own OBJ_ bounds are copied without its local
//transform; descendant OBJ_ boxes use local*ancestor transforms. A non-OBJ_
//descendant stops that branch, including its children. This is not the driver's
//different aggregate-bounds method used by Car geometry.
template<class Frames>
auto rigid_body_bounds(Frames& frames,typename Frames::Frame root) -> typename Frames::Bounds {
    using Frame=typename Frames::Frame;
    using Matrix=typename Frames::Matrix;
    using Vector=typename Frames::Vector;
    if(!root)throw std::runtime_error("Rigid body bounds require a frame");
    typename Frames::Bounds result;
    if(frames.visual_type(root)==0x5f4a424fu)result=frames.rigid_visual_bounds(root);
    else {
        result.minimum={{1.e16f,1.e16f,1.e16f,0}};
        result.maximum={{-1.e16f,-1.e16f,-1.e16f,0}};
    }
    std::unordered_set<Frame> visited{root};
    Matrix identity{{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1}};
    const auto walk=[&](auto&& self,Frame frame,const Matrix& parent,unsigned depth)->void {
        if(depth>1024||visited.size()>=1000000u||!visited.insert(frame).second)
            throw std::runtime_error("Rigid body bounds hierarchy exceeds bounds or repeats a frame");
        if(frames.visual_type(frame)!=0x5f4a424fu)return;
        const auto matrix=frames.rigid_matrix_product(frames.car_local_matrix(frame),parent);
        const auto box=frames.rigid_visual_bounds(frame);
        if(box.minimum[0]<box.maximum[0]) {
            for(unsigned corner=0;corner!=8;++corner) {
                Vector value{};
                for(unsigned axis=0;axis!=3;++axis)value[axis]=(corner&(1u<<axis))?box.maximum[axis]:box.minimum[axis];
                value=frames.car_transform(value,matrix);
                for(unsigned axis=0;axis!=3;++axis) {
                    if(!std::isfinite(value[axis]))throw std::runtime_error("Non-finite rigid body bounds transform");
                    if(value[axis]<result.minimum[axis])result.minimum[axis]=value[axis];
                    if(result.maximum[axis]<value[axis])result.maximum[axis]=value[axis];
                }
            }
        }
        for(auto child=frames.rigid_first_child(frame);child;child=frames.rigid_next_sibling(child))self(self,child,matrix,depth+1);
    };
    for(auto child=frames.rigid_first_child(root);child;child=frames.rigid_next_sibling(child))walk(walk,child,identity,1);
    return result;
}

} //namespace hd2::game
#endif
