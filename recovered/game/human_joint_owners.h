#ifndef HD2_GAME_HUMAN_JOINT_OWNERS_H
#define HD2_GAME_HUMAN_JOINT_OWNERS_H

#include "game/human_skeleton.h"
#include "game/human_joint_math.h"
#include <algorithm>
#include <memory>
#include <type_traits>
#include <vector>

namespace hd2::game {

//Scalar words keep original offsets without storing native32-bit pointers in
//host64-bit words. Original pointers are represented separately by typed owners
//and borrowed links. Reading/writing any pointer slot through words is forbidden.
template<std::size_t Size>struct alignas(16) HumanJointWords {
    template<class Value>Value get(std::size_t offset) const {
        static_assert(std::is_trivially_copyable_v<Value>);
        if(offset>Size||sizeof(Value)>Size-offset)throw std::out_of_range("Human joint scalar offset");
        Value value;std::memcpy(&value,bytes.data()+offset,sizeof value);return value;
    }
    template<class Value>void set(std::size_t offset,const Value& value) {
        static_assert(std::is_trivially_copyable_v<Value>);
        if(offset>Size||sizeof(Value)>Size-offset)throw std::out_of_range("Human joint scalar offset");
        std::memcpy(bytes.data()+offset,&value,sizeof value);
    }
    std::array<std::uint8_t,Size> bytes{};
};

//007C5080/007C5D50 and007C51A0/007C6050. This owner supplies the real cold
//body/point trees and loading bindings. Simulation/contact producers remain
//outside this interface. Frame matrices/quaternions use mandatory real endpoints.
template<class Frames>
class HumanJointOwners {
public:
    using Frame=typename Frames::Frame;
    using Matrix=HumanMatrix;
    using Vector=HumanVector;
    struct Point;
    struct Body : HumanJointWords<0x280> {
        Body* parent=nullptr;
        std::array<Body*,3> children{};
        std::array<Point*,2> points{};
        std::vector<int> bones;
        std::vector<Matrix> matrices;
        std::size_t used=0;
    };
    struct Point : HumanJointWords<0x160> {
        Point* parent=nullptr;
        std::array<Point*,3> children{};
        std::array<Body*,4> bodies{};
        Frame contact_frame{}; //Native150 retains an engine frame, produced by007CC262/007CCD48.
    };
    explicit HumanJointOwners(Frames& frames,const HumanSkeleton<Frame>& skeleton):frames_(frames),skeleton_(skeleton) {}
    ~HumanJointOwners(){clear();}
    HumanJointOwners(const HumanJointOwners&)=delete;
    HumanJointOwners& operator=(const HumanJointOwners&)=delete;
    Body* create_body(Body* parent,std::size_t capacity) { //007C5080 stdcall(parent,capacity),ret8.
        if(capacity>16)throw std::runtime_error("Human joint bone capacity exceeds native limit");
        if(parent&&std::find(parent->children.begin(),parent->children.end(),nullptr)==parent->children.end())return nullptr;
        auto owner=std::make_unique<Body>();auto& body=*owner;body.parent=parent;
        body.set(0x20,human_identity());body.set(0x60,human_identity());
        body.set(0x190,std::int32_t{-1});body.set(0x194,std::int32_t{-1});body.set(0x198,1.f);
        for(auto offset:{0x1b0u,0x1b4u,0x1b8u,0x1d0u,0x1d4u,0x1d8u,0x220u,0x224u,0x228u})body.set(offset,std::uint32_t{0x7fc00000});
        body.bones.resize(capacity);body.matrices.resize(capacity);
        auto* result=owner.get();bodies_.push_back(std::move(owner));
        if(parent)*std::find(parent->children.begin(),parent->children.end(),nullptr)=result;
        return result;
    }
    void bind_body(Body* body,int first,int last,bool length,bool radius,bool angle) { //007C52E0.
        if(!body)return;
        if(first<0)first=body->template get<std::int32_t>(0x190);else body->set(0x190,first);
        if(last<0)last=body->template get<std::int32_t>(0x194);else body->set(0x194,last);
        auto world=frames_.actor_world_matrix(bone_frame(first));body->set(0x60,world);
        const auto start=frames_.actor_world_matrix(bone_frame(first));
        const auto end=frames_.actor_world_matrix(bone_frame(last));
        Vector direction{};for(unsigned axis=0;axis!=3;++axis)direction[axis]=end[12+axis]-start[12+axis];
        const float square=static_cast<float>((static_cast<long double>(direction[2])*direction[2]+
            static_cast<long double>(direction[1])*direction[1])+static_cast<long double>(direction[0])*direction[0]);
        std::int32_t bits;std::memcpy(&bits,&square,4);
        if(bits<=0x322bcc76)return;
        const float distance=std::abs(static_cast<long double>(square)-1)<9.99999993922529e-09f?1.f:static_cast<float>(std::sqrt(static_cast<long double>(square)));
        if(!std::isfinite(distance))throw std::runtime_error("Non-finite Human joint distance");
        if(distance<9.99999993922529e-09f)return;
        if(length){body->set(0x270,distance);body->set(0x184,distance);}
        if(radius)body->set(0x18c,static_cast<float>(static_cast<long double>(body->template get<float>(0x184))*.2f));
        for(unsigned axis=0;axis!=3;++axis)direction[axis]=static_cast<float>(static_cast<long double>(direction[axis])/distance);
        Matrix alignment{};Vector old_forward{};for(unsigned axis=0;axis!=4;++axis)old_forward[axis]=world[8+axis];
        human_align_vectors(old_forward,direction,alignment);alignment[15]=1;
        world=frames_.rigid_matrix_product(world,alignment); //007A2ED0.
        const auto position=frames_.actor_world_matrix(bone_frame(first));
        for(unsigned axis=0;axis!=4;++axis)world[12+axis]=position[12+axis];
        body->set(0x60,world);
        auto local=world;
        if(body->parent) {
            auto inverse=human_transpose(body->parent->template get<Matrix>(0x60));inverse[3]=inverse[7]=inverse[11]=0;
            for(unsigned axis=0;axis!=3;++axis)local[12+axis]-=body->parent->template get<float>(0x90+axis*4);
            local=frames_.rigid_matrix_product(local,inverse);
            if(angle) {
                Vector forward{};for(unsigned axis=0;axis!=4;++axis)forward[axis]=local[8+axis];
                const auto relative=frames_.rigid_matrix_product(local,human_transpose(human_vertical_basis(forward)));
                body->set(0x180,static_cast<float>(std::atan2(static_cast<long double>(relative[1]),static_cast<long double>(relative[0]))));
            }
        }
        body->set(0x20,local);body->set(0xa0,frames_.human_matrix_rotation(local));body->set(0xb0,local);body->set(0xf0,local);
        Vector forward{};for(unsigned axis=0;axis!=4;++axis)forward[axis]=world[axis];
        body->set(0x130,forward);body->set(0x140,forward);
        body->set(0x150,body->template get<float>(0x270));body->set(0x154,body->template get<float>(0x270));
    }
    bool attach_bone(Body& body,int index) { //007C5620.
        if(body.used==16)return false;
        if(std::find(body.bones.begin(),body.bones.begin()+static_cast<std::ptrdiff_t>(body.used),index)!=body.bones.begin()+static_cast<std::ptrdiff_t>(body.used))return false;
        if(body.used==body.bones.size())throw std::runtime_error("Human joint bone allocation exhausted");
        body.bones[body.used]=index;
        auto inverse=human_transpose(body.template get<Matrix>(0x60));
        inverse[3]=inverse[7]=inverse[11]=0;
        auto local=frames_.actor_world_matrix(bone_frame(index));
        for(unsigned axis=0;axis!=3;++axis)local[12+axis]-=body.template get<float>(0x90+axis*4);
        body.matrices[body.used]=frames_.rigid_matrix_product(local,inverse);
        ++body.used;body.set(0x1a4,std::uint32_t(body.used));return true;
    }
    Point* create_point(Point* parent,const Vector& position,std::array<Body*,4> bodies,Body* first,Body* second) { //007C5D50.
        auto owner=std::make_unique<Point>();auto& point=*owner;point.parent=parent;point.bodies=bodies;
        point.set(0x30,position);point.set(0x40,position);point.set(0x100,position);point.set(0x24,1.f);point.set(0x140,std::uint32_t{0x7fc00000});
        auto* endpoint=first?first:second;
        if(endpoint)point.set(0x90,to_local(position,*endpoint,true));
        if(first&&first->parent)point.set(0x110,to_local(position,*first->parent,false));
        auto* result=owner.get();points_.push_back(std::move(owner));
        if(parent) {auto slot=std::find(parent->children.begin(),parent->children.end(),nullptr);if(slot!=parent->children.end())*slot=result;}
        if(first)first->points[0]=result;
        if(second)second->points[1]=result;
        return result;
    }
    void reset_point_tree(Point& point,const Vector& position,bool previous){ //007C6100's actual recursive point/body writes.
        const auto old=point.template get<Vector>(0x30);
        Vector delta{};
        for(unsigned axis=0;axis!=3;++axis)delta[axis]=position[axis]-old[axis];
        point.set(0x30,position);
        if(previous)point.set(0x40,position);
        point.set(0x100,position);
        for(unsigned offset:{0x50u,0x60u,0x70u,0xe0u,0x80u})
            for(unsigned axis=0;axis!=3;++axis)point.set(offset+axis*4,0.f);
        point.set(0x24,1.f);point.set(0x158,std::uint8_t{0});
        for(auto* body:point.bodies)if(body&&body->points[0]==&point)body->set(0x90,position);
        for(auto* child:point.children)if(child){
            auto target=child->template get<Vector>(0x30);
            for(unsigned axis=0;axis!=3;++axis)target[axis]=delta[axis]+target[axis];
            //Native child scratch writes XYZ only before passing a four-word
            //vector. Retain the child's existing padding lane on that boundary.
            reset_point_tree(*child,target,previous);
        }
    }
    void capture_body_pose(Body& body){ //007CDB70, original parent-relative B0/130/154 capture.
        auto local=body.template get<Matrix>(0x60);
        if(body.parent){
            auto inverse=human_transpose(body.parent->template get<Matrix>(0x60));
            inverse[3]=inverse[7]=inverse[11]=0;
            for(unsigned axis=0;axis!=3;++axis)local[12+axis]-=body.parent->template get<float>(0x90+axis*4);
            local[15]=1;
            body.set(0xb0,local); //Native stores the translated matrix before the engine multiply.
            local=frames_.rigid_matrix_product(local,inverse);
        }
        body.set(0xb0,local);
        body.set(0x130,frames_.human_matrix_rotation(local));
        body.set(0x154,body.template get<std::uint32_t>(0x270));
        for(auto* child:body.children)if(child)capture_body_pose(*child);
    }
    void snapshot_body_endpoints(Body& body){ //007C80E0 capture=true, rebuild=false branch used by007B0910.
        if(!body.points[0]||!body.points[1])throw std::runtime_error("Human joint snapshot requires actual endpoint points");
        body.set(0x160,body.points[0]->template get<Vector>(0x30));
        body.set(0x170,body.points[1]->template get<Vector>(0x30));
        body.set(0x188,body.template get<std::uint32_t>(0x270));
        body.set(0x210,body.template get<Vector>(0x200));
        for(auto* child:body.children)if(child)snapshot_body_endpoints(*child);
    }
    void destroy_body(Body* body) noexcept { //007C51A0, child order8/C/10.
        if(!body)return;
        for(auto* child:body->children)destroy_body(child);
        if(body->parent)detach(body->parent->children,body);
        for(auto* point:body->points)if(point)detach(point->bodies,body);
        erase(bodies_,body);
    }
    void destroy_point(Point* point) noexcept { //007C6050, child order18/1C/20.
        if(!point)return;
        for(auto* child:point->children)destroy_point(child);
        if(point->parent)detach(point->parent->children,point);
        //Guard partial/native-orphan lifetimes; no live body may borrow freed data.
        for(auto& body:bodies_)for(auto& endpoint:body->points)if(endpoint==point)endpoint=nullptr;
        erase(points_,point);
    }
    void clear() noexcept {
        while(!bodies_.empty())destroy_body(bodies_.front().get());
        while(!points_.empty())destroy_point(points_.front().get());
    }
    std::size_t body_count() const {return bodies_.size();}
    std::size_t point_count() const {return points_.size();}
private:
    Frame bone_frame(int index) const {
        if(index<0||index>=23||!skeleton_.bones()[std::size_t(index)].frame)throw std::runtime_error("Required Human joint bone is absent");
        return skeleton_.bones()[std::size_t(index)].frame;
    }
    static Vector to_local(const Vector& position,const Body& body,bool length) {
        Vector result{};std::array<float,3> delta{};
        for(unsigned axis=0;axis!=3;++axis)delta[axis]=position[axis]-body.template get<float>(0x90+axis*4);
        for(unsigned row=0;row!=3;++row)result[row]=static_cast<float>((static_cast<long double>(delta[0])*body.template get<float>(0x60+row*16)+
            static_cast<long double>(delta[1])*body.template get<float>(0x64+row*16))+static_cast<long double>(delta[2])*body.template get<float>(0x68+row*16));
        if(length) {
            const auto divisor=body.template get<float>(0x184);if(!divisor)throw std::runtime_error("Zero Human joint endpoint length");
            result[2]/=divisor;
        }
        return result;
    }
    template<class T,std::size_t Count>static void detach(std::array<T*,Count>& links,T* value) noexcept {
        for(auto& link:links)if(link==value){link=nullptr;break;}
    }
    template<class T>static void erase(std::vector<std::unique_ptr<T>>& owners,T* value) noexcept {
        const auto found=std::find_if(owners.begin(),owners.end(),[value](const auto& owner){return owner.get()==value;});
        if(found!=owners.end())owners.erase(found);
    }
    Frames& frames_;const HumanSkeleton<Frame>& skeleton_;
    std::vector<std::unique_ptr<Body>> bodies_;
    std::vector<std::unique_ptr<Point>> points_;
};
} //namespace hd2::game
#endif
