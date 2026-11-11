#ifndef HD2_GAME_HUMAN_LOADING_JOINTS_H
#define HD2_GAME_HUMAN_LOADING_JOINTS_H

#include "game/human_joint_owners.h"

namespace hd2::game {

//007C0D90 normal loading branch, with its eleven body/sixteen point ownership
//trees and original per-body parameters. The6950 owner must bind all23 frames
//before calling this. No solver tick or collision registration is implied.
template<class Frames>
class HumanLoadingJoints {
public:
    using Frame=typename Frames::Frame;
    using Owners=HumanJointOwners<Frames>;
    using Body=typename Owners::Body;
    using Point=typename Owners::Point;
    using Matrix=HumanMatrix;
    using Vector=HumanVector;
    struct alignas(16) Impulse {
        Point* point=nullptr;
        alignas(16) Vector value{};
        float duration=0,remaining=0;
    };
    static_assert(sizeof(Impulse)==0x30&&offsetof(Impulse,value)==0x10);
    HumanLoadingJoints(Frames& frames,const HumanSkeleton<Frame>& skeleton)
        :frames_(frames),skeleton_(skeleton),owners_(frames,skeleton) {}
    ~HumanLoadingJoints(){clear();}
    void build(bool preserve=false) {
        if(preserve)return; //Original007C0D90 returns immediately.
        clear();
        gravity={{0,-9,0,0}};field68d0=1;field68e8=0;field68ec=1;field68f0=4;field68f4=0;
        field6900=6;field690c=0;flag6910=flag6911=flag6912=false;field6914=field6918=-1;
        try {
            struct Definition {int parent,first,last,capacity;float field198,radius_scale,length_scale;std::array<int,5> bones;};
            constexpr std::array<Definition,11> definitions{{
                {-1,22,7,5,2,3,1,{{22,9,12,13,7}}},
                {0,8,4,4,2,3.2f,1,{{8,15,14,4,-1}}},
                {1,4,6,2,1,5.5f,3,{{5,6,-1,-1,-1}}},
                {1,17,19,1,1,1.5f,1,{{17,-1,-1,-1,-1}}},
                {3,19,21,2,1,1.5f,1.25f,{{19,21,-1,-1,-1}}},
                {1,16,18,1,1,1.5f,1,{{16,-1,-1,-1,-1}}},
                {5,18,20,2,1,1.5f,1.25f,{{18,20,-1,-1,-1}}},
                {0,10,2,1,2.5f,1.5f,1,{{10,-1,-1,-1,-1}}},
                {7,2,0,2,2,1,1,{{2,0,-1,-1,-1}}},
                {0,11,3,1,2.5f,1.5f,1,{{11,-1,-1,-1,-1}}},
                {9,3,1,2,2,1,1,{{3,1,-1,-1,-1}}}
            }};
            for(unsigned index=0;index!=definitions.size();++index) {
                const auto& definition=definitions[index];
                auto* body=owners_.create_body(definition.parent<0?nullptr:bodies_[std::size_t(definition.parent)],std::size_t(definition.capacity));
                if(!body)throw std::runtime_error("Cannot attach required Human joint body");
                bodies_[index]=body;
                owners_.bind_body(body,definition.first,definition.last,true,true,true);
                for(auto bone:definition.bones)if(bone>=0)owners_.attach_bone(*body,bone);
                body->set(0x198,definition.field198);
                body->set(0x18c,static_cast<float>(static_cast<long double>(body->template get<float>(0x18c))*definition.radius_scale));
                if(definition.length_scale!=1) {
                    const float length=static_cast<float>(static_cast<long double>(body->template get<float>(0x184))*definition.length_scale);
                    body->set(0x270,length);body->set(0x184,length);
                }
                body->set(0,std::uint32_t(index));
            }
            bounds(1,0x1b8,.3f);bounds(1,0x1b4,-.7f);bounds(1,0x1d4,.7f);bounds(1,0x1b0,-.5f);bounds(1,0x1d0,.5f);
            bounds(2,0x1b8,.707f);
            for(auto index:{3u,5u}) {
                bounds(index,0x1d8,.8f);bounds(index,index==3?0x1d0u:0x1b0u,0);bounds(index,0x1d4,.707f);
            }
            for(auto index:{4u,6u}) {
                bounds(index,0x1b8,-.5f);bounds(index,index==4?0x1b0u:0x1d0u,0);bounds(index,0x1d4,.1f);
            }
            for(auto index:{7u,9u}) {
                bounds(index,0x1d8,-.15f);bounds(index,index==7?0x1d0u:0x1b0u,0);bounds(index,index==7?0x1b0u:0x1d0u,index==7?-.5f:.5f);bounds(index,0x1d4,.707f);
            }
            for(auto index:{8u,10u}) {
                bodies_[index]->set(0x220,Vector{{0,-1,0,0}});
                bounds(index,0x1b8,-.707f);bounds(index,0x1d4,0);bounds(index,0x1b0,0);bounds(index,0x1d0,0);
            }
            struct PointDefinition {int parent,body;bool end;std::array<int,4> links;int first,second;};
            constexpr std::array<PointDefinition,16> points{{
                {-1,0,false,{{0,7,9,-1}},0,-1},
                {0,1,false,{{1,0,-1,-1}},1,0},
                {1,1,true,{{1,3,5,2}},2,1},
                {2,2,true,{{2,-1,-1,-1}},-1,2},
                {2,3,false,{{3,1,-1,-1}},3,-1},
                {4,3,true,{{3,4,-1,-1}},4,3},
                {5,4,true,{{4,-1,-1,-1}},-1,4},
                {2,5,false,{{5,1,-1,-1}},5,-1},
                {7,5,true,{{5,6,-1,-1}},6,5},
                {8,6,true,{{6,-1,-1,-1}},-1,6},
                {0,7,false,{{7,0,-1,-1}},7,-1},
                {10,7,true,{{7,8,-1,-1}},8,7},
                {11,8,true,{{8,-1,-1,-1}},-1,8},
                {0,9,false,{{9,0,-1,-1}},9,-1},
                {13,9,true,{{9,10,-1,-1}},10,9},
                {14,10,true,{{10,-1,-1,-1}},-1,10}
            }};
            for(unsigned index=0;index!=points.size();++index) {
                const auto& definition=points[index];std::array<Body*,4> links{};
                for(unsigned slot=0;slot!=4;++slot)links[slot]=body(definition.links[slot]);
                auto* point=owners_.create_point(definition.parent<0?nullptr:points_[std::size_t(definition.parent)],
                    endpoint(*body(definition.body),definition.end),links,body(definition.first),body(definition.second));
                points_[index]=point;point->set(0,std::uint32_t(index));
            }
            cross_axis(0,10,13);cross_axis(1,4,7);
            constexpr std::array<unsigned,14> saved{{9,12,13,7,8,15,14,4,5,6,0,1,21,20}};
            for(unsigned index=0;index!=saved.size();++index) {
                const auto frame=skeleton_.bones()[saved[index]].frame;
                if(!frame)throw std::runtime_error("Required Human saved joint bone is absent");
                saved_local_[index]=frames_.car_local_matrix(frame);
            }
            work_matrices_={};work_vectors_={};
            impulses_=std::make_unique<std::array<Impulse,8>>(); //007C0D90's aligned180, zeroed after pose snapshots.
            secondary_impulses_=std::make_unique<std::array<Impulse,2>>(); //Native aligned60.
        } catch(...) {clear();throw;}
    }
    void clear() noexcept {
        if(field690c)retire_contact_frames(); //0079BAC0 releases retained150 frames BEFORE either tree.
        owners_.clear();bodies_.fill(nullptr);points_.fill(nullptr);
        impulses_.reset();secondary_impulses_.reset();
    }
    const auto& bodies() const {return bodies_;}
    const auto& points() const {return points_;}
    const auto* impulses()const{return impulses_.get();}
    const auto* secondary_impulses()const{return secondary_impulses_.get();}
    const auto& saved_local() const {return saved_local_;}
    const auto& owners() const {return owners_;}
    void retire_contact_frames(){ //007CDA40, ascending actual6874 array and release-before-clear.
        for(auto* point:points_)if(point&&point->contact_frame){
            frames_.release_item_frame(point->contact_frame);
            point->contact_frame=Frame{};
        }
        field690c=0;
    }
    template<class Unavailable>void rebind_effect_bodies(Unavailable unavailable){ //007B0910's full-joint branch.
        for(unsigned index=0;index!=bodies_.size();++index){
            owners_.bind_body(bodies_[index],-1,-1,false,false,false);
            switch(index){
            case 2:case 8:case 10:break;
            case 4:case 6:if(unavailable())continue;break;
            default:continue;
            }
            auto* body=bodies_[index];
            if(!body)throw std::runtime_error("Human geometry effect requires its actual endpoint body");
            const auto bone=body->template get<std::int32_t>(0x194);
            const auto frame=skeleton_.bones().at(std::size_t(bone)).frame;
            if(!frame)throw std::runtime_error("Human geometry effect requires its actual endpoint bone");
            const auto forward=frames_.actor_world_matrix(frame);
            const auto up=frames_.actor_world_matrix(frame);
            const auto right=frames_.actor_world_matrix(frame);
            const float partial_z=right[2]+up[6];
            const float x=static_cast<float>((static_cast<long double>(right[0])+up[4])+forward[8]);
            const auto y=(static_cast<long double>(right[1])+up[5])+forward[9];
            const auto z=static_cast<long double>(partial_z)+forward[10];
            if((z*z+y*y)+static_cast<long double>(x)*x<9.99999993922529e-09f){
                //These five native branches retire leaf bodies and leaf points.
                constexpr std::array<unsigned,11> endpoint{{1,2,3,5,6,8,9,11,12,14,15}};
                const auto point=endpoint[index];
                owners_.destroy_body(body);bodies_[index]=nullptr;
                owners_.destroy_point(points_[point]);points_[point]=nullptr;
            }
        }
    }
    void initialize_mounted_effect_points(){ //007B0910, source Human nonnull makes prior velocity zero.
        constexpr std::array<unsigned,16> body_index{{0,1,1,2,3,3,4,5,5,6,7,7,8,9,9,10}};
        constexpr std::array<bool,16> end{{false,false,true,true,false,true,true,false,true,true,false,true,true,false,true,true}};
        for(unsigned index=0;index!=points_.size();++index){
            auto* point=points_[index];
            if(!point){
                if(index==3||index==5||index==6||index==8||index==9||index==12||index==15)continue;
                throw std::runtime_error("Human geometry effect lost a required joint point");
            }
            const auto* body=bodies_[body_index[index]];
            if(!body)throw std::runtime_error("Human geometry effect point lost its actual body");
            auto position=body->template get<Vector>(0x90);
            if(end[index]){
                const auto length=body->template get<float>(0x184);
                //Native Y/Z products spill to float before adding the origin.
                const float y=length*body->template get<float>(0x84);
                const float z=length*body->template get<float>(0x88);
                position[0]=static_cast<float>(static_cast<long double>(length)*body->template get<float>(0x80)+position[0]);
                position[1]=y+position[1];position[2]=z+position[2];
                position[3]=point->template get<float>(0x3c); //Native scratch lane is unwritten padding.
            }
            point->set(0x30,position);
            position[3]=point->template get<float>(0x4c);
            point->set(0x40,position);
        }
    }
    void reset_point(Point* point,const Vector& position,bool previous){ //007C6100, null selects actual68B8 root.
        if(!point)point=points_[0];
        field68f0=4;field68d0=field68ec=1;
        if(!point)throw std::runtime_error("Human joint reset requires its actual root point");
        owners_.reset_point_tree(*point,position,previous);
    }
    void queue_point_impulse(Point* point,const Vector& value,std::int32_t duration){ //007CD610, first free of eight.
        if(!point)return;
        if(!impulses_)throw std::logic_error("Human impulse requires its actual initialized joint queue");
        for(auto& impulse:*impulses_)if(!impulse.point){
            impulse.point=point;impulse.value=value;
            impulse.duration=static_cast<float>(static_cast<long double>(duration)*.0010000000474974513f);
            impulse.remaining=impulse.duration;
            field68d0=1;
            for(auto* current:points_)if(current)current->set(0x24,1.f);
            return;
        }
    }
    void queue_secondary_impulse(Point* point,const Vector& value,std::int32_t duration){ //007CD6C0.
        if(!point)return;
        if(!impulses_||!secondary_impulses_)throw std::logic_error("Human secondary impulse requires its actual initialized queues");
        for(unsigned index=0;index!=secondary_impulses_->size();++index){
            auto& impulse=(*secondary_impulses_)[index];
            if(impulse.point)continue;
            impulse.point=point;impulse.value=value;
            //Native7CD71E reloads68E0 and writes its duration, leaving68E4+20 untouched.
            (*impulses_)[index].duration=static_cast<float>(static_cast<long double>(duration)*.0010000000474974513f);
            return;
        }
    }
    template<class Random>void distribute_impulse(Random& random,std::uint32_t mask,const Vector& value,std::int32_t duration){ //007B3270.
        const auto first=random()+1u;
        const auto second=random()+1u;
        const auto third=random()+1u; //All three draws precede the mask and missing-point gates.
        const auto scaled=[&](long double fraction){
            auto part=value; //Native temporary W is unwritten. Retain the known input padding.
            for(unsigned axis=0;axis!=3;++axis)part[axis]=static_cast<float>(fraction*value[axis]);
            return part;
        };
        unsigned first_index,second_index;
        switch(mask&0x3fu){
        case 1:first_index=3;second_index=2;break;
        case 2:first_index=5;second_index=6;break;
        case 4:first_index=8;second_index=9;break;
        case 8:{
            const float total=static_cast<float>(first+second+third);
            queue_point_impulse(points_[0],scaled(static_cast<long double>(first)/total),duration);
            queue_point_impulse(points_[1],scaled(static_cast<float>(static_cast<long double>(second)/total)),duration);
            queue_point_impulse(points_[2],scaled(static_cast<float>(static_cast<long double>(third)/total)),duration);
            return;
        }
        case 16:first_index=11;second_index=12;break;
        case 32:first_index=14;second_index=15;break;
        default:queue_point_impulse(points_[1],value,duration);return;
        }
        const auto optional_index=(mask&0x3fu)==1?first_index:second_index;
        if(!points_[optional_index]){
            const auto retained=(mask&0x3fu)==1?second_index:first_index;
            queue_point_impulse(points_[retained],value,duration);return;
        }
        const float total=static_cast<float>(first+second);
        queue_point_impulse(points_[first_index],scaled(static_cast<long double>(first)/total),duration);
        queue_point_impulse(points_[second_index],scaled(static_cast<float>(static_cast<long double>(second)/total)),duration);
    }
    void capture_body_pose(){
        if(!bodies_[0])throw std::runtime_error("Human joint pose capture requires its actual root body");
        owners_.capture_body_pose(*bodies_[0]);
    }
    void snapshot_body_endpoints(){
        if(!bodies_[0])throw std::runtime_error("Human joint endpoint snapshot requires its actual root body");
        owners_.snapshot_body_endpoints(*bodies_[0]);
    }
    Vector gravity{{0,-9,0,0}};float field68d0=1,field68e8=0,field68ec=1,field68f0=4,field68f4=0;
    std::uint32_t field6900=6,field690c=0;
    bool flag6910=false,flag6911=false,flag6912=false;
    std::int32_t field6914=-1,field6918=-1;
private:
    Body* body(int index) {return index<0?nullptr:bodies_.at(std::size_t(index));}
    void bounds(unsigned index,std::size_t offset,float value) {
        bodies_[index]->set(offset,value);bodies_[index]->set(offset+0x10,static_cast<float>(std::sqrt(1-static_cast<long double>(value)*value)));
    }
    static Vector endpoint(const Body& body,bool end) {
        auto position=body.template get<Vector>(0x90);
        if(end)for(unsigned axis=0;axis!=3;++axis)position[axis]=static_cast<float>(static_cast<long double>(body.template get<float>(0x184))*body.template get<float>(0x80+axis*4)+position[axis]);
        return position;
    }
    void cross_axis(unsigned index,unsigned first,unsigned second) {
        auto& body=*bodies_[index];const auto axis=body.template get<Vector>(0x200);
        std::array<Vector,2> offsets{};const std::array<unsigned,2> points{{first,second}};
        for(unsigned n=0;n!=2;++n) {
            for(unsigned component=0;component!=3;++component)offsets[n][component]=points_[points[n]]->template get<float>(0x30+component*4)-body.template get<float>(0x90+component*4);
            const auto projection=(static_cast<long double>(offsets[n][0])*axis[0]+static_cast<long double>(offsets[n][1])*axis[1])+static_cast<long double>(offsets[n][2])*axis[2];
            const float stored=static_cast<float>(projection);
            for(unsigned component=0;component!=3;++component)offsets[n][component]=static_cast<float>(static_cast<long double>(offsets[n][component])-static_cast<long double>(stored)*axis[component]);
        }
        Vector direction{};for(unsigned component=0;component!=3;++component)direction[component]=offsets[1][component]-offsets[0][component];
        direction=actor_activation_direction(direction);Vector local{};
        for(unsigned row=0;row!=3;++row)local[row]=static_cast<float>((static_cast<long double>(direction[0])*body.template get<float>(0x60+row*16)+
            static_cast<long double>(direction[2])*body.template get<float>(0x68+row*16))+static_cast<long double>(direction[1])*body.template get<float>(0x64+row*16));
        auto matrix=body.template get<Matrix>(0x230);human_align_vectors(local,Vector{{1,0,0,0}},matrix);body.set(0x230,matrix);
    }
    Frames& frames_;const HumanSkeleton<Frame>& skeleton_;
    Owners owners_;
    std::array<Body*,11> bodies_{};
    std::array<Point*,16> points_{};
    alignas(16)std::array<Matrix,14> saved_local_{};
    alignas(16)std::array<Matrix,6> work_matrices_{};
    alignas(16)std::array<Vector,6> work_vectors_{};
    std::unique_ptr<std::array<Impulse,8>> impulses_;
    std::unique_ptr<std::array<Impulse,2>> secondary_impulses_;
};
} //namespace hd2::game
#endif
