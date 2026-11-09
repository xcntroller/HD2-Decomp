#ifndef HD2_GAME_HUMAN_SKELETON_H
#define HD2_GAME_HUMAN_SKELETON_H

#include "game/actor_activation_math.h"
#include <array>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace hd2::game {

//00816D18:23 fixed twelve-byte names. This is the native bone-index order,
//not the hierarchy traversal order used by0079C320.
inline constexpr std::array<const char*,23> human_bone_names{{
    "u","q","t","p","e","f1","f","d1","d","b","s","o",
    "c1","c","g","k","h","l","i","m","j","n","a"
}};

struct HumanBoneConstraint {
    std::array<float,3> maximum{{3.1415927410125732421875f,3.1415927410125732421875f,3.1415927410125732421875f}};
    std::array<float,3> minimum{{-3.1415927410125732421875f,-3.1415927410125732421875f,-3.1415927410125732421875f}};
    std::array<std::uint32_t,3> fields20{};
    std::uint32_t flags=7;
    float weight=1;
};

//The graph/state portion of0079BD30/0079C320. Frames are borrowed from the
//containing model. Native node stride440 is not imposed on host pointer sizes.
//Camera ownership, model reset, preserve-pose and joint construction belong to
//the containing6950 owner; this type never reports complete model activation.
template<class Frame>
class HumanSkeleton {
public:
    using Matrix=std::array<float,16>;
    using Vector=std::array<float,4>;
    struct Bone {
        int index=-1;
        Frame frame{};
        std::array<int,3> children{{-1,-1,-1}};
        int parent=-1;
        std::array<HumanBoneConstraint,6> constraints;
        std::array<Vector,3> offsets{};
        std::array<float,3> lengths{};
        std::array<Matrix,3> bases{};
    };
    void clear() {bones_={};root_=-1;count_=0;}
    //007A42A0:missing frame is deliberately ignored; fourth child is rejected
    //after storing the new node's frame/index/parent, matching the original.
    void add(int index,int parent,Frame frame) {
        if(index<0||index>=23||parent>=23||!frame)return;
        auto& node=bones_[std::size_t(index)];node.index=index;node.frame=frame;
        node.children.fill(-1);node.parent=parent<0?-1:parent;
        if(parent<0){root_=index;++count_;return;}
        for(auto& child:bones_[std::size_t(parent)].children)if(child<0){child=index;++count_;return;}
    }
    void flags(int index,int neighbour,std::uint32_t value) { //007A4D50.
        auto& node=bones_.at(std::size_t(index));
        for(unsigned edge=0;edge!=3;++edge)if(node.children[edge]==neighbour&&neighbour>=0) {
            node.constraints[edge].flags=value;return;
        }
        if(node.parent==neighbour&&neighbour>=0)for(unsigned edge=3;edge!=6;++edge)node.constraints[edge].flags=value;
    }
    void weight(int index,int neighbour,float value) { //007A4DF0.
        auto& node=bones_.at(std::size_t(index));
        for(unsigned edge=0;edge!=3;++edge)if(node.children[edge]==neighbour&&neighbour>=0) {
            node.constraints[edge].weight=value;return;
        }
        if(node.parent==neighbour&&neighbour>=0)for(unsigned edge=3;edge!=6;++edge)node.constraints[edge].weight=value;
    }
    template<class Frames>void bind_graph(Frames& frames,Frame model) { //0079C320 normal graph branch.
        clear();
        struct Link {int bone,parent;};
        constexpr std::array<Link,23> links{{
            {22,-1},{9,22},{12,9},{13,12},{7,13},{8,7},{4,8},{5,4},{6,5},
            {14,8},{16,14},{18,16},{20,18},{15,8},{17,15},{19,17},{21,19},
            {11,22},{3,11},{1,3},{10,22},{2,10},{0,2}
        }};
        for(const auto& link:links)add(link.bone,link.parent,frames.actor_find(model,human_bone_names[std::size_t(link.bone)],0xffff));
        root_=22; //0079C66E writes this even when a find was null.
        struct Setting {int from,to;std::uint32_t flags;};
        constexpr std::array<Setting,44> settings{{
            {22,9,0},{9,22,0},{9,12,7},{12,9,0},{12,13,0},{13,12,7},
            {13,7,7},{7,13,0},{7,8,7},{8,7,7},{8,15,0},{15,8,3},
            {8,14,0},{14,8,3},{8,4,0},{4,8,3},{4,5,3},{5,4,3},{5,6,0},{6,5,0},
            {15,17,3},{17,15,3},{17,19,3},{19,17,3},{19,21,3},{21,19,7},
            {14,16,3},{16,14,3},{16,18,3},{18,16,3},{18,20,3},{20,18,7},
            {22,10,0},{10,22,3},{10,2,3},{2,10,1},{2,0,1},{0,2,1},
            {22,11,0},{11,22,3},{11,3,3},{3,11,1},{3,1,1},{1,3,1}
        }};
        for(const auto& value:settings)flags(value.from,value.to,value.flags);
        struct Weight {int from,to;float value;};
        constexpr std::array<Weight,11> weights{{
            {9,12,.2f},{13,7,.4f},{7,8,.7f},{15,17,.9f},{17,19,1},{19,21,1},
            {14,16,.9f},{16,18,1},{18,20,1},{8,7,.5f},{13,12,.5f}
        }};
        for(const auto& value:weights)weight(value.from,value.to,value.value);
        for(auto& bone:bones_)if(bone.index>=0)for(unsigned slot=0;slot!=3;++slot) {
            const auto child=bone.children[slot];if(child<0)continue;
            const auto local=frames.car_local_matrix(bones_[std::size_t(child)].frame);
            auto& offset=bone.offsets[slot];for(unsigned axis=0;axis!=4;++axis)offset[axis]=local[12+axis];
            bone.lengths[slot]=length(offset);
            bone.bases[slot]=basis(offset);
        }
    }
    const auto& bones() const {return bones_;}
    int root() const {return root_;}
    std::uint32_t count() const {return count_;}
    static float length(const Vector& v) { //0079C9D6's stored sum and signed bit comparison.
        const float square=static_cast<float>((static_cast<long double>(v[0])*v[0]+
            static_cast<long double>(v[1])*v[1])+static_cast<long double>(v[2])*v[2]);
        if(!std::isfinite(square))throw std::runtime_error("Non-finite Human bone length");
        std::int32_t bits;std::memcpy(&bits,&square,4);
        if(bits<0x322bcc77)return 0;
        if(std::abs(static_cast<long double>(square)-1)<9.99999993922529e-09f)return 1;
        return static_cast<float>(std::sqrt(static_cast<long double>(square)));
    }
    //007A23F0 with the identity matrix supplied by0079C320. Project X onto
    //the normal plane; use Y when X is parallel. Cross-product order matters.
    static Matrix basis(Vector direction) {
        const auto z=normalize(direction);
        const auto dot=static_cast<long double>(z[0]);
        Vector x{{static_cast<float>(1-dot*z[0]),static_cast<float>(-dot*z[1]),static_cast<float>(-dot*z[2]),0}};
        const auto square=(static_cast<long double>(x[0])*x[0]+static_cast<long double>(x[1])*x[1])+static_cast<long double>(x[2])*x[2];
        Matrix result{};Vector y{};
        if(square<1.0000000195414814e-16f) { //0080F948.
            const auto projection=static_cast<long double>(z[1]);
            y=normalize(Vector{{static_cast<float>(-projection*z[0]),static_cast<float>(1-projection*z[1]),static_cast<float>(-projection*z[2]),0}});
            x=cross(y,z);
        } else {x=normalize(x);y=cross(z,x);}
        for(unsigned axis=0;axis!=3;++axis){result[axis]=x[axis];result[4+axis]=y[axis];result[8+axis]=z[axis];}
        result[15]=1;return result;
    }
private:
    static Vector normalize(Vector v) { //007A23F0: X*X+Y*Y+Z*Z order.
        const auto square=(static_cast<long double>(v[0])*v[0]+static_cast<long double>(v[1])*v[1])+static_cast<long double>(v[2])*v[2];
        if(!std::isfinite(square))throw std::runtime_error("Non-finite Human bone direction");
        constexpr float epsilon=9.99999993922529e-09f;
        if(std::abs(square-1)>=epsilon) {
            if(square>=epsilon) {const auto scale=1/std::sqrt(square);for(unsigned axis=0;axis!=3;++axis)v[axis]=static_cast<float>(scale*v[axis]);}
            else {const auto axis=v[0]!=0?0u:v[2]!=0?2u:1u;v[axis]=v[axis]<0?-1.f:1.f;}
        }
        return v;
    }
    static Vector cross(const Vector& a,const Vector& b) {
        return {{static_cast<float>(static_cast<long double>(a[1])*b[2]-static_cast<long double>(a[2])*b[1]),
            static_cast<float>(static_cast<long double>(a[2])*b[0]-static_cast<long double>(a[0])*b[2]),
            static_cast<float>(static_cast<long double>(a[0])*b[1]-static_cast<long double>(a[1])*b[0]),0}};
    }
    std::array<Bone,23> bones_;
    int root_=-1;
    std::uint32_t count_=0;
};

} //namespace hd2::game
#endif
