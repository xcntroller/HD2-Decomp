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
    void snapshot_body_interpolation(Body& body){ //007CDA90, before the next physical step.
        const auto local=body.template get<Matrix>(0xb0);
        body.set(0xf0,local);
        body.set(0x140,body.template get<Vector>(0x130));
        body.set(0x150,body.template get<std::uint32_t>(0x154));
        body.set(0x270,body.template get<std::uint32_t>(0x154));
        body.set(0x60,body.parent?frames_.rigid_matrix_product(local,body.parent->template get<Matrix>(0x60)):local);
        for(auto* child:body.children)if(child)snapshot_body_interpolation(*child);
    }
    void interpolate_body_pose(Body& body,float factor){ //007CDC90, retained previous/current local poses.
        const float complement=1.f-factor;
        const auto rotation=frames_.physics_slerp(body.template get<Vector>(0x140),body.template get<Vector>(0x130),factor,false);
        body.set(0xa0,rotation);
        auto local=body.template get<Matrix>(0x20);
        frames_.human_set_matrix_rotation(local,rotation);
        //Native X products remain extended, previous Y and both Z products spill.
        local[12]=static_cast<float>(static_cast<long double>(complement)*body.template get<float>(0x120)+
            static_cast<long double>(factor)*body.template get<float>(0xe0));
        const float previous_y=complement*body.template get<float>(0x124);
        local[13]=static_cast<float>(previous_y+static_cast<long double>(factor)*body.template get<float>(0xe4));
        const float previous_z=complement*body.template get<float>(0x128);
        const float current_z=factor*body.template get<float>(0xe8);
        local[14]=previous_z+current_z;local[15]=1;
        body.set(0x20,local);
        body.set(0x60,body.parent?frames_.rigid_matrix_product(local,body.parent->template get<Matrix>(0x60)):local);
        body.set(0x270,static_cast<float>(static_cast<long double>(factor)*body.template get<float>(0x154)+
            static_cast<long double>(complement)*body.template get<float>(0x150)));
        for(auto* child:body.children)if(child)interpolate_body_pose(*child,factor);
    }
    void project_body_bones(Body& body){ //007C59C0, actual per-body attached-bone matrices to native bone50.
        for(std::size_t index=0;index!=body.used;++index){
            const auto bone=body.bones.at(index);
            bone_world_.at(static_cast<std::size_t>(bone))=frames_.rigid_matrix_product(body.matrices.at(index),body.template get<Matrix>(0x60));
        }
        for(auto* child:body.children)if(child)project_body_bones(*child);
    }
    void publish_body_bones(Body& body,float factor,Frame model){ //007C5A90, preserve actual frame translations.
        for(std::size_t index=0;index!=body.used;++index){
            const auto bone=body.bones.at(index);
            const auto& node=skeleton_.bones().at(static_cast<std::size_t>(bone));
            const auto parent=node.parent<0?model:bone_frame(node.parent);
            auto inverse=human_transpose(frames_.actor_world_matrix(parent));
            inverse[3]=inverse[7]=inverse[11]=0;
            auto local=frames_.rigid_matrix_product(bone_world_.at(static_cast<std::size_t>(bone)),inverse);
            if(factor<1){
                const auto actual=frames_.car_local_matrix(node.frame); //004C8140 precedes the native quaternion capture.
                const auto current=frames_.human_matrix_rotation(actual);
                const auto desired=frames_.human_matrix_rotation(local);
                frames_.human_set_matrix_rotation(local,frames_.physics_slerp(desired,current,factor,false));
            }
            orthonormalize_human_bone_matrix(local);
            frames_.human_publish_bone_rotation(node.frame,local,true);
        }
        for(auto* child:body.children)if(child)publish_body_bones(*child,factor,model);
    }
    void snapshot_body_endpoints(Body& body,bool capture=true,bool rebuild=false,const std::array<Point*,16>* points=nullptr){ //007C80E0.
        if(!body.points[0]||!body.points[1])throw std::runtime_error("Human joint snapshot requires actual endpoint points");
        if(rebuild){
            if(!points)throw std::logic_error("Human joint rebuild requires its actual point array");
            rebuild_body_frame(body,*points);
        }
        if(capture){
            body.set(0x160,body.points[0]->template get<Vector>(0x30));
            body.set(0x170,body.points[1]->template get<Vector>(0x30));
            body.set(0x188,body.template get<std::uint32_t>(0x270));
            body.set(0x210,body.template get<Vector>(0x200));
        }
        for(auto* child:body.children)if(child)snapshot_body_endpoints(*child,capture,rebuild,points);
    }
    static void correct_body_length(const Body& body,Vector& first,Vector& last){ //007CA390, untouched outputs inside tolerance.
        const auto length=body.template get<float>(0x270),rest=body.template get<float>(0x184);
        const auto difference=static_cast<long double>(length)-rest;
        if(!(std::abs(difference)>=.0010000000474974513))return;
        for(unsigned axis=0;axis!=3;++axis){
            const auto direction=body.template get<float>(0x200+axis*4);
            first[axis]=static_cast<float>(difference*.5f*direction);
            last[axis]=static_cast<float>((static_cast<long double>(rest)-length)*.5f*direction);
        }
    }
    void correct_body_angle(Body& body,Vector& first,Vector& last){ //007CA470, ordered Z/X/Y minimum/maximum bounds.
        if(!body.parent)return;
        const auto parent=body.parent->template get<Matrix>(0x60);
        const auto axis=body.template get<Vector>(0x200);
        Vector direction=axis;
        for(unsigned row=0;row!=3;++row){
            const float projection=static_cast<float>((static_cast<long double>(axis[2])*parent[row*4+2]+
                static_cast<long double>(axis[1])*parent[row*4+1])+static_cast<long double>(axis[0])*parent[row*4]);
            direction[row]=std::clamp(projection,-1.f,1.f);
        }
        struct Bound {unsigned axis,other,zero,offset;std::uint8_t flag;bool maximum;};
        constexpr std::array<Bound,6> bounds{{
            {2,0,1,0x1b8,0x40,false},{2,0,1,0x1d8,0x80,true},
            {0,1,2,0x1b0,4,false},{0,1,2,0x1d0,8,true},
            {1,0,2,0x1b4,0x10,false},{1,0,2,0x1d4,0x20,true}
        }};
        std::uint8_t flags=0;
        for(const auto& bound:bounds){
            const auto bits=body.template get<std::uint32_t>(bound.offset);
            if(bits==0x7fc00000u||bits==0xffc00000u)continue;
            const auto limit=body.template get<float>(bound.offset);
            if(!(bound.maximum?direction[bound.axis]>limit:direction[bound.axis]<limit))continue;
            const auto denominator=std::sqrt(1.L-static_cast<long double>(direction[bound.axis])*direction[bound.axis]);
            const auto transverse=body.template get<float>(bound.offset+0x10);
            if(denominator>9.99999993922529e-09f){
                const auto scale=transverse/denominator;
                direction[bound.other]=static_cast<float>(direction[bound.other]*scale);
                direction[bound.zero]=static_cast<float>(direction[bound.zero]*scale);
            }else{
                direction[bound.other]=transverse;direction[bound.zero]=0;
            }
            direction[bound.axis]=limit;flags|=bound.flag;
        }
        //The original still evaluates the rotation/difference with no active bound.
        const auto world=frames_.collision_sweep_rotate(direction,parent);
        const auto length=body.template get<float>(0x270),rest=body.template get<float>(0x184);
        Vector correction=world;
        for(unsigned component=0;component!=3;++component){
            const float current=length*axis[component],desired=rest*world[component];
            correction[component]=desired-current;
        }
        if(!flags)return;
        for(unsigned component=0;component!=3;++component)last[component]=correction[component]*.800000011920929f;
        for(unsigned component=0;component!=3;++component)first[component]=correction[component]*-.20000000298023224f;
        if(!body.points[1])throw std::runtime_error("Human angular correction requires its actual endpoint point");
        body.points[1]->set(0x158,static_cast<std::uint8_t>(body.points[1]->template get<std::uint8_t>(0x158)|flags));
    }
    static void correct_body_overlap(Body& body,const Vector& first,const Vector& last,Vector& first_correction,Vector& last_correction){ //007CB510.
        auto* parent=body.parent;
        if(!parent)return;
        const std::array<Body*,4> neighbours{{parent->parent,parent->children[0],parent->children[1],parent->children[2]}};
        for(auto* neighbour:neighbours)if(neighbour&&neighbour!=&body)
            correct_sibling_overlap(*neighbour,parent,body,first,last,first_correction,last_correction);
    }
    static void correct_sibling_overlap(Body& other,Body* previous,Body& body,const Vector& first,const Vector& last,
            Vector& first_correction,Vector& last_correction){ //007CB5D0, parent then three children.
        if(!other.points[0]||!other.points[1])throw std::runtime_error("Human body overlap requires actual endpoint points");
        const auto origin=other.points[0]->template get<Vector>(0x30);
        const auto end=other.points[1]->template get<Vector>(0x30);
        Vector direction=end,body_direction=last,closest=origin,body_closest=first;
        for(unsigned axis=0;axis!=3;++axis){direction[axis]-=origin[axis];body_direction[axis]-=first[axis];}
        float parameter=0,body_parameter=0;
        const auto distance=human_joint_segment_distance(origin,direction,first,body_direction,closest,body_closest,parameter,body_parameter);
        if(!human_joint_unwritten(distance)){
            const float radius=static_cast<float>((static_cast<long double>(other.template get<float>(0x18c))+
                body.template get<float>(0x18c))*.5f);
            if(distance<radius&&distance>9.99999993922529e-09f){
                const auto scale=static_cast<long double>(radius)/distance;
                Vector correction=end;
                for(unsigned axis=0;axis!=3;++axis){
                    auto difference=static_cast<long double>(body_closest[axis])-closest[axis];
                    if(axis==1)difference=static_cast<float>(difference);
                    correction[axis]=static_cast<float>(difference*scale);
                }
                for(auto* output:{&first_correction,&last_correction}){
                    if(human_joint_unwritten((*output)[0]))*output=correction;
                    else{
                        //Native X halves remain extended, previous Y and both Z halves spill.
                        (*output)[0]=static_cast<float>(static_cast<long double>((*output)[0])*.5f+static_cast<long double>(correction[0])*.5f);
                        const float previous_y=(*output)[1]*.5f;
                        (*output)[1]=static_cast<float>(previous_y+static_cast<long double>(correction[1])*.5f);
                        const float previous_z=(*output)[2]*.5f,current_z=correction[2]*.5f;
                        (*output)[2]=previous_z+current_z;
                    }
                }
                const auto inverse=1.L/radius;
                for(auto* point:body.points){
                    if(!point)throw std::runtime_error("Human body overlap requires its actual contact point");
                    const auto flags=point->template get<std::uint8_t>(0x158);
                    if(flags&2u)continue;
                    point->set(0x158,static_cast<std::uint8_t>(flags|2u));
                    auto normal=correction;
                    for(unsigned axis=0;axis!=3;++axis)normal[axis]=static_cast<float>(correction[axis]*inverse);
                    point->set(0x120,normal);
                }
            }
        }
        const std::array<Body*,4> neighbours{{other.parent,other.children[0],other.children[1],other.children[2]}};
        for(auto* neighbour:neighbours)if(neighbour&&neighbour!=previous)
            correct_sibling_overlap(*neighbour,&other,body,first,last,first_correction,last_correction);
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
    void rebuild_body_frame(Body& body,const std::array<Point*,16>& points){ //007C80E0 rebuild=true.
        constexpr float epsilon=9.99999993922529e-09f;
        const auto first=body.points[0]->template get<Vector>(0x30);
        const auto last=body.points[1]->template get<Vector>(0x30);
        auto axis=body.template get<Vector>(0x200);
        for(unsigned component=0;component!=3;++component)axis[component]=last[component]-first[component];
        body.set(0x200,axis);
        const float square=static_cast<float>((static_cast<long double>(axis[0])*axis[0]+
            static_cast<long double>(axis[1])*axis[1])+static_cast<long double>(axis[2])*axis[2]);
        std::int32_t bits;std::memcpy(&bits,&square,sizeof bits);
        const auto length=bits<0x322bcc77?0.L:std::abs(static_cast<long double>(square)-1)<epsilon?1.L:std::sqrt(static_cast<long double>(square));
        body.set(0x270,static_cast<float>(length));
        if(length>epsilon){
            const auto inverse=1.L/length;
            for(unsigned component=0;component!=3;++component)axis[component]=static_cast<float>(axis[component]*inverse);
            body.set(0x200,axis);
        }
        const auto index=body.template get<std::uint32_t>(0);
        Matrix world;
        if(body.parent){
            const auto parent=body.parent->template get<Matrix>(0x60);
            Vector direction=axis;
            for(unsigned row=0;row!=3;++row)direction[row]=static_cast<float>((static_cast<long double>(axis[0])*parent[row*4]+
                static_cast<long double>(axis[1])*parent[row*4+1])+static_cast<long double>(axis[2])*parent[row*4+2]);
            float angle=body.template get<float>(0x180);
            if(index==7||index==9)angle+=3.1415927410125732421875f;
            const bool lateral=index==4||index==6||index==7||index==8||index==9||index==10;
            const auto basis=lateral?human_lateral_basis(direction):human_vertical_basis(direction);
            world=frames_.rigid_matrix_product(frames_.human_z_rotation(angle),basis);
            body.set(0x60,world);
            world=frames_.rigid_matrix_product(world,parent);
        }else{
            world=body.template get<Matrix>(0x60);
            const Vector forward{{world[8],world[9],world[10],world[11]}};
            Matrix alignment{};human_align_vectors(forward,axis,alignment);alignment[15]=1;
            world=frames_.rigid_matrix_product(world,alignment); //007A2ED0.
            orthonormalize_human_bone_matrix(world);
        }
        for(unsigned component=0;component!=3;++component)world[12+component]=first[component];
        world[15]=1;body.set(0x60,world);
        if(index>1)return;
        const std::array<unsigned,2> endpoints=index==0?std::array<unsigned,2>{{10,13}}:std::array<unsigned,2>{{4,7}};
        std::array<Vector,2> offsets{};
        for(unsigned endpoint=0;endpoint!=2;++endpoint){
            const auto* point=points[endpoints[endpoint]];
            if(!point)throw std::runtime_error("Human joint orientation requires its actual cross-axis points");
            for(unsigned component=0;component!=3;++component)
                offsets[endpoint][component]=point->template get<float>(0x30+component*4)-first[component];
            const auto projection=(static_cast<long double>(offsets[endpoint][2])*axis[2]+
                static_cast<long double>(offsets[endpoint][1])*axis[1])+static_cast<long double>(offsets[endpoint][0])*axis[0];
            for(unsigned component=0;component!=3;++component){
                auto product=projection*axis[component];
                if(endpoint||component)product=static_cast<float>(product);
                offsets[endpoint][component]=static_cast<float>(offsets[endpoint][component]-product);
            }
        }
        Vector cross{};
        for(unsigned component=0;component!=3;++component)cross[component]=offsets[1][component]-offsets[0][component];
        const float cross_square=static_cast<float>(index==0?
            (static_cast<long double>(cross[0])*cross[0]+static_cast<long double>(cross[1])*cross[1])+static_cast<long double>(cross[2])*cross[2]:
            (static_cast<long double>(cross[0])*cross[0]+static_cast<long double>(cross[2])*cross[2])+static_cast<long double>(cross[1])*cross[1]);
        std::memcpy(&bits,&cross_square,sizeof bits);
        if(bits<=0x322bcc76)return;
        const auto cross_length=std::abs(static_cast<long double>(cross_square)-1)<epsilon?1.L:std::sqrt(static_cast<long double>(cross_square));
        if(cross_length<=epsilon)return;
        const auto inverse=1.L/cross_length;
        for(unsigned component=0;component!=3;++component)cross[component]=static_cast<float>(cross[component]*inverse);
        Vector local=cross;
        for(unsigned row=0;row!=3;++row){
            const auto middle=index==0?1u:2u,last_axis=index==0?2u:1u;
            local[row]=static_cast<float>((static_cast<long double>(cross[0])*world[row*4]+
                static_cast<long double>(cross[middle])*world[row*4+middle])+static_cast<long double>(cross[last_axis])*world[row*4+last_axis]);
        }
        const auto aligned=frames_.collision_sweep_rotate(local,body.template get<Matrix>(0x230));
        const auto right=frames_.collision_sweep_rotate(aligned,world);
        for(unsigned component=0;component!=3;++component)world[component]=right[component];
        world[4]=static_cast<float>(static_cast<long double>(world[2])*world[9]-static_cast<long double>(world[10])*world[1]);
        world[5]=static_cast<float>(static_cast<long double>(world[10])*world[0]-static_cast<long double>(world[2])*world[8]);
        world[6]=static_cast<float>(static_cast<long double>(world[8])*world[1]-static_cast<long double>(world[0])*world[9]);
        world[3]=world[7]=world[11]=0;body.set(0x60,world);
    }
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
    std::array<Matrix,23> bone_world_{}; //Original geometry bone50 scratch, separate from IK chain200.
    std::vector<std::unique_ptr<Body>> bodies_;
    std::vector<std::unique_ptr<Point>> points_;
};
} //namespace hd2::game
#endif
