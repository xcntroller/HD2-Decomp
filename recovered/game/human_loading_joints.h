#ifndef HD2_GAME_HUMAN_LOADING_JOINTS_H
#define HD2_GAME_HUMAN_LOADING_JOINTS_H

#include "game/human_joint_owners.h"
#include "game/human_state_frame_timers.h"

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
        field6900=6;field690c=0;flag6910=0;flag6911=flag6912=false;field6914=field6918=-1;
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
        //0079BAC0 releases retained150 frames BEFORE either tree. Scan actual
        //references also when a C++ failure interrupts the later690C publication.
        retire_contact_frames();
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
            if(human_joint_frame_basis_square(frames_,frame)<9.99999993922529e-09f){
                //These five native branches retire leaf bodies and leaf points.
                constexpr std::array<unsigned,11> endpoint{{1,2,3,5,6,8,9,11,12,14,15}};
                const auto point=endpoint[index];
                owners_.destroy_body(body);bodies_[index]=nullptr;
                owners_.destroy_point(points_[point]);points_[point]=nullptr;
            }
        }
    }
    void initialize_effect_points(const Vector& velocity){ //007B0910, actual prior-position displacement.
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
            for(unsigned axis=0;axis!=3;++axis)position[axis]-=velocity[axis];
            position[3]=point->template get<float>(0x4c);
            point->set(0x40,position);
        }
    }
    void retire_unavailable_effect_limbs(){ //007B0910, unavailable Human with a null source.
        auto* junction=points_[2];
        if(!junction)throw std::runtime_error("Human reaction requires its actual limb junction");
        for(unsigned index:{3u,5u}){
            const auto found=std::find(junction->bodies.begin(),junction->bodies.end(),bodies_[index]);
            if(found!=junction->bodies.end())*found=nullptr;
        }
        for(unsigned side=0;side!=2;++side){
            const unsigned body=3+side*2,point=5+side*3;
            owners_.destroy_body(bodies_[body]);bodies_[body]=bodies_[body+1]=nullptr;
            owners_.destroy_point(points_[point]);points_[point]=points_[point+1]=nullptr;
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
    void consume_impulses(float seconds){ //007CD730, primary queue before secondary queue.
        if(!impulses_||!secondary_impulses_)throw std::logic_error("Human impulse step requires its actual initialized queues");
        for(auto& impulse:*impulses_){
            if(!impulse.point)continue;
            const float duration=impulse.duration-seconds;
            //Native tests the stored sign bit, including negative zero and signed NaNs.
            if(std::signbit(duration)||(impulse.point->template get<std::uint8_t>(0x158)&3u)){
                impulse.point=nullptr;continue;
            }
            impulse.duration=duration;
            for(unsigned axis=0;axis!=3;++axis){
                const auto offset=0x80+axis*4;
                impulse.point->set(offset,impulse.point->template get<float>(offset)+impulse.value[axis]);
            }
        }
        for(auto& impulse:*secondary_impulses_){
            if(!impulse.point)continue;
            const float duration=impulse.duration-seconds;
            if(std::signbit(duration)){impulse.point=nullptr;continue;}
            impulse.duration=duration;
            for(unsigned axis=0;axis!=3;++axis){
                const auto offset=0x80+axis*4;
                const auto difference=static_cast<long double>(impulse.value[axis])-impulse.point->template get<float>(0x30+axis*4);
                const auto delta=axis==2?static_cast<long double>(static_cast<float>(difference)):difference;
                impulse.point->set(offset,static_cast<float>(delta+impulse.point->template get<float>(offset)));
            }
        }
    }
    void update_contact_frames(std::uint32_t elapsed,std::uint8_t mode){ //007CD840, actual retained frame motion.
        if(!(mode&2u))return;
        field690c=0;
        const auto signed_elapsed=static_cast<std::int32_t>(elapsed);
        for(auto* point:points_){
            if(!point||!point->contact_frame)continue;
            Vector velocity{};
            bool release=frames_.item_frame_references(point->contact_frame)==1;
            if(!release){
                const auto world=frames_.actor_world_matrix(point->contact_frame);
                const float inverse_elapsed=static_cast<float>(1.L/signed_elapsed);
                for(unsigned axis=0;axis!=3;++axis){
                    const auto difference=static_cast<long double>(world[12+axis])-point->template get<float>(0x140+axis*4);
                    const auto delta=axis==2?static_cast<long double>(static_cast<float>(difference)):difference;
                    velocity[axis]=static_cast<float>(delta*inverse_elapsed);
                }
                const auto square=(static_cast<long double>(velocity[2])*velocity[2]+static_cast<long double>(velocity[1])*velocity[1])+
                    static_cast<long double>(velocity[0])*velocity[0];
                release=square>9.999999747378752e-05f;
            }
            if(release){
                if(field68d0<9.99999993922529e-09f){
                    field68f0=4;field68d0=field68ec=1;
                    for(auto* current:points_)if(current)current->set(0x24,1.f);
                }
                const auto scale=static_cast<long double>(signed_elapsed)*.10000000149011612f;
                for(unsigned axis=0;axis!=3;++axis){
                    const auto product=scale*velocity[axis];
                    const auto delta=axis?static_cast<long double>(static_cast<float>(product)):product;
                    point->set(0x40+axis*4,static_cast<float>(point->template get<float>(0x40+axis*4)-delta));
                }
                frames_.release_item_frame(point->contact_frame);
                point->contact_frame=Frame{};
            }
            if(point->contact_frame)++field690c;
        }
    }
    static float contact_restitution(std::uint32_t material){ //007CDFC0, native material ranges.
        if((material>=11&&material<=20)||(material>=66&&material<=72)||(material>=85&&material<=95))return .5f;
        if((material>=21&&material<=30)||(material>=38&&material<=65))return .25f;
        if((material>=31&&material<=37)||(material>=73&&material<=84))return .75f;
        return 1;
    }
    void integrate_point_tree(Point& point,float seconds){ //007C79F0, contact response before gravity/queued forces.
        const auto position=point.template get<Vector>(0x30);
        const auto previous=point.template get<Vector>(0x40);
        const bool contact=(point.template get<std::uint8_t>(0x158)&3u)!=0;
        auto velocity=point.template get<Vector>(0x50);
        auto inverse=1.L/seconds;
        if(contact)inverse=static_cast<float>(inverse);
        for(unsigned axis=0;axis!=3;++axis){
            auto difference=static_cast<long double>(position[axis])-previous[axis];
            if(!contact)difference=static_cast<float>(difference);
            velocity[axis]=static_cast<float>(difference*inverse);
        }
        point.set(0x50,velocity);
        if(contact){
            const auto normal=point.template get<Vector>(0x120);
            const float projection=static_cast<float>((static_cast<long double>(velocity[2])*normal[2]+
                static_cast<long double>(velocity[1])*normal[1])+static_cast<long double>(normal[0])*velocity[0]);
            if(std::signbit(projection)){
                const auto twice=static_cast<long double>(projection)+projection;
                for(unsigned axis=0;axis!=3;++axis){
                    const float reflected=static_cast<float>(twice*normal[axis]);
                    velocity[axis]-=reflected;
                }
                point.set(0x50,velocity);
                if(normal[1]>.30000001192092896f){
                    const auto reflected_projection=(static_cast<long double>(normal[2])*velocity[2]+
                        static_cast<long double>(normal[1])*velocity[1])+static_cast<long double>(normal[0])*velocity[0];
                    Vector normal_velocity=velocity,tangent=velocity;
                    for(unsigned axis=0;axis!=3;++axis){
                        normal_velocity[axis]=static_cast<float>(reflected_projection*normal[axis]);
                        tangent[axis]=velocity[axis]-normal_velocity[axis];
                    }
                    if(projection>-.009999999776482582f)
                        for(unsigned axis=0;axis!=3;++axis)tangent[axis]*=.75f;
                    const auto restitution=contact_restitution(point.template get<std::uint32_t>(0x154));
                    for(unsigned axis=0;axis!=3;++axis){
                        const float rebound=normal_velocity[axis]*restitution;
                        velocity[axis]=rebound+tangent[axis];
                    }
                    point.set(0x50,velocity);
                }
            }
        }
        if(flag6912){
            for(unsigned axis=0;axis!=3;++axis)velocity[axis]*=.8999999761581421f;
            point.set(0x50,velocity);
        }
        auto acceleration=point.template get<Vector>(0x60);
        auto motion=point.template get<Vector>(0x70);
        for(unsigned axis=0;axis!=3;++axis){
            const float force=point.template get<float>(0x80+axis*4)+gravity[axis];
            acceleration[axis]=force*seconds;
            motion[axis]=acceleration[axis]+velocity[axis];
        }
        point.set(0x60,acceleration);point.set(0x70,motion);
        for(unsigned axis=0;axis!=3;++axis)motion[axis]*=field68d0;
        if(motion[1]>20)motion[1]=20;
        if(flag6910){
            const float damping=static_cast<float>(1.L-static_cast<long double>(flag6910)*.0625f*.6000000238418579f);
            for(unsigned axis=0;axis!=3;++axis)motion[axis]*=damping;
        }
        point.set(0x70,motion);
        auto next=position;
        for(unsigned axis=0;axis!=3;++axis){
            const float displacement=seconds*motion[axis];
            next[axis]+=displacement;
        }
        point.set(0x30,next);point.set(0x40,position);
        for(unsigned axis=0;axis!=3;++axis)point.set(0x80+axis*4,0.f);
        for(auto* child:point.children)if(child)integrate_point_tree(*child,seconds);
    }
    void apply_body_corrections(){ //007CBAE0, retained endpoint corrections weighted by actual linked bodies.
        for(auto* point:points_){
            if(!point)continue;
            float total=0;
            for(auto* body:point->bodies)if(body)total+=body->template get<float>(0x198);
            long double x=0;
            float y=0,z=0;
            for(unsigned slot=0;slot!=point->bodies.size();++slot){
                const auto* body=point->bodies[slot];
                if(!body)continue;
                const auto weight=static_cast<long double>(body->template get<float>(0x198))/total;
                const auto offset=0xa0+slot*0x10;
                const auto dx=weight*point->template get<float>(offset);
                const auto dy=weight*point->template get<float>(offset+4);
                const auto dz=weight*point->template get<float>(offset+8);
                if(slot){
                    x+=static_cast<float>(dx);
                    y+=static_cast<float>(dy);
                    z=static_cast<float>(z+dz);
                }else{
                    x=dx;y=static_cast<float>(dy);z=static_cast<float>(dz);
                }
            }
            if(flag6911&&(point->template get<std::uint8_t>(0x158)&3u)){
                const auto normal=point->template get<Vector>(0x120);
                const float projection=static_cast<float>((static_cast<long double>(z)*normal[2]+
                    static_cast<long double>(y)*normal[1])+x*normal[0]);
                if(std::signbit(projection)){
                    x-=static_cast<long double>(projection)*normal[0];
                    const float dy=projection*normal[1],dz=projection*normal[2];
                    y-=dy;z-=dz;
                }
            }
            point->set(0x30,static_cast<float>(x+point->template get<float>(0x30)));
            point->set(0x34,y+point->template get<float>(0x34));
            point->set(0x38,z+point->template get<float>(0x38));
        }
    }
    void solve_body_constraints(Body& body,float seconds){ //007C8B10, corrections are retained until007CBAE0.
        auto* first=body.points[0];auto* last=body.points[1];
        if(!first||!last)throw std::runtime_error("Human body constraints require actual endpoint points");
        const auto first_slot=correction_offset(*first,body),last_slot=correction_offset(*last,body);
        if(!first_slot||!last_slot)return;
        for(auto* point:{first,last}){
            const auto slot=point==first?first_slot:last_slot;
            for(unsigned axis=0;axis!=3;++axis)point->set(slot+axis*4,0.f);
        }
        if(!body.parent||body.parent->points[1]!=first)
            first->set(0x158,static_cast<std::uint8_t>(first->template get<std::uint8_t>(0x158)&1u));
        last->set(0x158,static_cast<std::uint8_t>(last->template get<std::uint8_t>(0x158)&1u));
        Vector first_value{},last_value{};
        first_value[0]=last_value[0]=std::numeric_limits<float>::quiet_NaN();
        owners_.correct_body_length(body,first_value,last_value);
        if(!human_joint_unwritten(first_value[0]))store_correction(*first,first_slot,first_value);
        if(!human_joint_unwritten(last_value[0]))store_correction(*last,last_slot,last_value);
        first_value[0]=last_value[0]=std::numeric_limits<float>::quiet_NaN();
        owners_.correct_body_overlap(body,first->template get<Vector>(0x30),last->template get<Vector>(0x30),first_value,last_value);
        if(!human_joint_unwritten(first_value[0]))blend_correction(*first,first_slot,first_value,.5f,.5f);
        if(!human_joint_unwritten(last_value[0]))blend_correction(*last,last_slot,last_value,.5f,.5f);
        first_value[0]=last_value[0]=std::numeric_limits<float>::quiet_NaN();
        owners_.correct_body_angle(body,first_value,last_value);
        if(!human_joint_unwritten(first_value[0]))blend_correction(*first,first_slot,first_value,.25f,.75f);
        if(!human_joint_unwritten(last_value[0]))blend_correction(*last,last_slot,last_value,.25f,.75f);
        const auto index=body.template get<std::uint32_t>(0);
        if(index<=1){
            const std::array<unsigned,2> endpoints=index==0?std::array<unsigned,2>{{10,13}}:std::array<unsigned,2>{{4,7}};
            std::array<Vector,2> targets{};
            for(auto endpoint:endpoints){
                auto* point=points_[endpoint];
                if(!point)throw std::runtime_error("Human body constraints require actual cross-axis points");
                const auto slot=required_correction_offset(*point,body);
                for(unsigned axis=0;axis!=3;++axis)point->set(slot+axis*4,0.f);
            }
            for(unsigned endpoint=0;endpoint!=endpoints.size();++endpoint){
                auto* point=points_[endpoints[endpoint]];
                auto target=frames_.car_transform(point->template get<Vector>(0x110),body.template get<Matrix>(0x60));
                targets[endpoint]=target;
                for(unsigned axis=0;axis!=3;++axis)target[axis]-=point->template get<float>(0x30+axis*4);
                store_correction(*point,required_correction_offset(*point,body),target);
            }
            if(index==1&&!correct_torso_twist(body,targets))return;
        }else if(index==3||index==5||index==7||index==9){
            const auto anchor=index==3||index==5?2u:0u;
            auto* point=points_[anchor];
            auto* parent=bodies_[anchor==2?1u:0u];
            if(!point||!parent)throw std::runtime_error("Human body constraints require actual parent anchors");
            const auto slot=required_correction_offset(*point,body);
            for(unsigned axis=0;axis!=3;++axis)point->set(slot+axis*4,0.f);
            const auto target=frames_.car_transform(first->template get<Vector>(0x110),parent->template get<Matrix>(0x60));
            auto correction=point->template get<Vector>(slot);
            for(unsigned axis=0;axis!=3;++axis)correction[axis]=first->template get<float>(0x30+axis*4)-target[axis];
            store_correction(*point,slot,correction);
        }
        if(index==4||index==6||index==8||index==10){
            auto* parent=body.parent;
            if(!parent||!parent->points[0])throw std::runtime_error("Human limb constraints require an actual parent body");
            if(last->template get<float>(0x34)>first->template get<float>(0x34)&&
               last->template get<float>(0x34)>parent->points[0]->template get<float>(0x34)&&
               !(last->template get<std::uint8_t>(0x158)&1u)&&(parent->points[0]->template get<std::uint8_t>(0x158)&1u)){
                const bool grounded=(first->template get<std::uint8_t>(0x158)&1u)!=0;
                auto* reference=grounded?parent:parent->parent;
                if(!reference)throw std::runtime_error("Human limb constraints require their actual grandparent body");
                add_upward_correction(body,*reference,*last,last_slot,grounded?.15000000596046448f:-.15000000596046448f);
            }
        }else if(index==7||index==9){
            auto* child=body.children[0];
            if(!child||!child->points[1]||!body.parent)throw std::runtime_error("Human leg constraints require their actual child body");
            if(last->template get<float>(0x34)>first->template get<float>(0x34)&&
               last->template get<float>(0x34)<child->points[1]->template get<float>(0x34)&&
               !(last->template get<std::uint8_t>(0x158)&1u)&&(first->template get<std::uint8_t>(0x158)&1u))
                add_upward_correction(body,*body.parent,*last,last_slot,-.10000000149011612f);
        }
        for(auto* child:body.children)if(child)solve_body_constraints(*child,seconds);
    }
    template<class Contacts,class Sleep>void advance_physics(std::uint32_t elapsed,std::uint32_t frame_flags,Contacts& contacts,Sleep& sleep){ //007C77E0.
        const auto previous=field6900;
        field6900=6;
        if(frame_flags&0x10u)field6900=6;
        if(frame_flags&0x20u)field6900=5;
        if(frame_flags&0x40u)field6900=5;
        if(frame_flags&0x80u)field6900=4;
        if(frame_flags&0x100u)field6900=4;
        if(frame_flags&0x200u)field6900=3;
        const float ratio=static_cast<float>(static_cast<long double>(human_frame_signed_word(previous))/human_frame_signed_word(field6900));
        field68d0=static_cast<float>(1.L-(1.L-field68d0)*ratio);
        for(auto* point:points_)if(point)
            point->set(0x24,static_cast<float>(1.L-(1.L-point->template get<float>(0x24))*ratio));
        if(!(field68d0>9.99999993922529e-09f))return;
        field68fc=0;
        field68f8=static_cast<float>(static_cast<long double>(human_frame_signed_word(elapsed))/(human_frame_signed_word(field6900)*1000));
        while(human_frame_signed_word(field68fc)<human_frame_signed_word(field6900)){
            flag6911=(field68fc&1u)||field68fc==field6900-1;
            consume_impulses(field68f8);
            if(!points_[0])throw std::runtime_error("Human ragdoll step requires its actual root point");
            integrate_point_tree(*points_[0],field68f8);
            snapshot_body_endpoints(false,true);
            solve_body_constraints(*bodies_[0],field68f8);
            apply_body_corrections();
            if(flag6911){contacts(field68f8);snapshot_body_endpoints(true,true);}
            sleep(*points_[0]);
            if(flag6911)field68f4=0;else field68f4+=field68f8;
            ++field68fc;
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
    void snapshot_body_interpolation(){
        if(!bodies_[0])throw std::runtime_error("Human joint interpolation requires its actual root body");
        owners_.snapshot_body_interpolation(*bodies_[0]);
    }
    void interpolate_body_pose(float factor){
        if(!bodies_[0])throw std::runtime_error("Human joint interpolation requires its actual root body");
        owners_.interpolate_body_pose(*bodies_[0],factor);
    }
    Vector publish_body_pose(Frame model,Body* body=nullptr,float factor=1){ //007C58B0.
        if(!body)body=bodies_[0];
        if(!body||!bodies_[0]||!model)throw std::runtime_error("Human joint publication requires its actual body tree and model");
        owners_.project_body_bones(*body);
        owners_.publish_body_bones(*body,factor,model);
        const auto bone=bodies_[0]->template get<std::int32_t>(0x190);
        const auto frame=skeleton_.bones().at(static_cast<std::size_t>(bone)).frame;
        if(!frame)throw std::runtime_error("Human joint publication requires its actual root bone");
        const auto anchor=frames_.actor_world_matrix(frame);
        const auto root=bodies_[0]->template get<Vector>(0x90);
        Vector delta{};
        for(unsigned axis=0;axis!=3;++axis)delta[axis]=root[axis]-anchor[12+axis];
        const auto previous=frames_.actor_world_matrix(model);
        frames_.object_position(model,{{delta[0]+previous[12],delta[1]+previous[13],delta[2]+previous[14]}});
        frames_.update_scene_object(model);
        const auto world=frames_.actor_world_matrix(model);
        return {{world[12],world[13],world[14],world[15]}}; //Caller publishes actual Human base30.
    }
    void snapshot_body_endpoints(bool capture=true,bool rebuild=false){
        if(!bodies_[0])throw std::runtime_error("Human joint endpoint snapshot requires its actual root body");
        owners_.snapshot_body_endpoints(*bodies_[0],capture,rebuild,&points_);
    }
    Vector gravity{{0,-9,0,0}};float field68d0=1,field68d4=0,field68d8=0,field68ec=1,field68f0=4,field68f4=0,field68f8=0;
    std::uint32_t field68e8=0,field68fc=0; //Native68E8 is the SIGNED integer100ms presentation countdown.
    std::uint32_t field6900=6,field690c=0;
    std::uint8_t flag6910=0; //Native counts upward-facing contacts, consumed as a byte by007C79F0.
    bool flag6911=false,flag6912=false;
    std::int32_t field6914=-1,field6918=-1;
private:
    static std::size_t correction_offset(const Point& point,const Body& body){
        for(unsigned slot=0;slot!=point.bodies.size();++slot)if(point.bodies[slot]==&body)return 0xa0+slot*0x10;
        return 0;
    }
    static std::size_t required_correction_offset(const Point& point,const Body& body){
        const auto offset=correction_offset(point,body);
        if(!offset)throw std::runtime_error("Human joint correction lost its actual body link");
        return offset;
    }
    static void store_correction(Point& point,std::size_t offset,const Vector& value){
        for(unsigned axis=0;axis!=3;++axis)point.set(offset+axis*4,value[axis]);
    }
    static void blend_correction(Point& point,std::size_t offset,const Vector& value,float previous,float current){
        const auto old=point.template get<Vector>(offset);
        auto result=old;
        result[0]=static_cast<float>(static_cast<long double>(old[0])*previous+static_cast<long double>(value[0])*current);
        const float old_y=old[1]*previous;
        result[1]=static_cast<float>(old_y+static_cast<long double>(value[1])*current);
        const float old_z=old[2]*previous,new_z=value[2]*current;
        result[2]=old_z+new_z;store_correction(point,offset,result);
    }
    static void add_upward_correction(const Body& body,const Body& reference,Point& point,std::size_t offset,float scale){
        const auto axis=body.template get<Vector>(0x200),other=reference.template get<Vector>(0x200);
        Vector cross{};
        cross[0]=static_cast<float>(static_cast<long double>(other[1])*axis[2]-static_cast<long double>(other[2])*axis[1]);
        cross[1]=static_cast<float>(static_cast<long double>(other[2])*axis[0]-static_cast<long double>(other[0])*axis[2]);
        cross[2]=static_cast<float>(static_cast<long double>(other[0])*axis[1]-static_cast<long double>(other[1])*axis[0]);
        Vector correction{};
        correction[0]=static_cast<float>((static_cast<long double>(cross[2])*axis[1]-static_cast<long double>(cross[1])*axis[2])*scale);
        correction[1]=static_cast<float>((static_cast<long double>(cross[0])*axis[2]-static_cast<long double>(cross[2])*axis[0])*scale);
        correction[2]=static_cast<float>((static_cast<long double>(cross[1])*axis[0]-static_cast<long double>(cross[0])*axis[1])*scale);
        for(unsigned component=0;component!=3;++component)point.set(offset+component*4,point.template get<float>(offset+component*4)+correction[component]);
    }
    bool correct_torso_twist(Body& body,const std::array<Vector,2>& targets){ //007C8B10's body1 transverse constraint.
        if(!body.parent)throw std::runtime_error("Human torso twist requires its actual parent body");
        constexpr float epsilon=9.99999993922529e-09f;
        const auto pose=body.template get<Matrix>(0x60),parent=body.parent->template get<Matrix>(0x60);
        Vector relative{};
        for(unsigned row=0;row!=3;++row)relative[row]=static_cast<float>((static_cast<long double>(parent[row*4])*pose[0]+
            static_cast<long double>(parent[row*4+1])*pose[1])+static_cast<long double>(parent[row*4+2])*pose[2]);
        const float square=static_cast<float>(static_cast<long double>(relative[0])*relative[0]+static_cast<long double>(relative[1])*relative[1]);
        if(!(square>epsilon))return true;
        const auto radius=std::sqrt(static_cast<long double>(square));
        if(!(relative[0]/radius<.8659999966621399f))return true;
        relative[1]=static_cast<float>((std::signbit(relative[1])?-.5f:.5f)*radius);
        relative[0]=static_cast<float>(radius*.8659999966621399f);
        auto right=frames_.collision_sweep_rotate(relative,parent);
        const auto projection=(static_cast<long double>(right[0])*pose[8]+static_cast<long double>(right[1])*pose[9])+
            static_cast<long double>(right[2])*pose[10];
        for(unsigned axis=0;axis!=3;++axis){
            auto product=projection*pose[8+axis];
            if(axis)product=static_cast<float>(product);
            right[axis]=static_cast<float>(right[axis]-product);
        }
        const auto extended=(static_cast<long double>(right[0])*right[0]+static_cast<long double>(right[1])*right[1])+
            static_cast<long double>(right[2])*right[2];
        if(std::abs(extended-1)>=epsilon){
            const float stored=static_cast<float>(extended);
            if(stored>=epsilon){
                const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));
                for(unsigned axis=0;axis!=3;++axis)right[axis]=static_cast<float>(right[axis]*inverse);
            }else{
                const auto axis=right[0]!=0?0u:right[2]!=0?2u:1u;
                right[axis]=right[axis]<0?-1.f:1.f;
            }
        }
        auto preferred=pose;
        for(unsigned axis=0;axis!=3;++axis)preferred[axis]=right[axis];
        preferred[4]=static_cast<float>(static_cast<long double>(pose[9])*right[2]-static_cast<long double>(pose[10])*right[1]);
        preferred[5]=static_cast<float>(static_cast<long double>(pose[10])*right[0]-static_cast<long double>(pose[8])*right[2]);
        preferred[6]=static_cast<float>(static_cast<long double>(pose[8])*right[1]-static_cast<long double>(pose[9])*right[0]);
        preferred[3]=preferred[7]=preferred[11]=0;
        const std::array<unsigned,2> shoulders{{4,7}};
        std::array<Vector,2> desired{};
        for(unsigned index=0;index!=2;++index){
            auto* point=points_[shoulders[index]];
            if(!point)throw std::runtime_error("Human torso twist requires its actual shoulder points");
            desired[index]=frames_.car_transform(point->template get<Vector>(0x110),preferred);
        }
        for(unsigned index=0;index!=2;++index){
            auto* point=points_[shoulders[index]];
            for(unsigned axis=0;axis!=3;++axis)desired[index][axis]-=targets[index][axis];
            blend_correction(*point,required_correction_offset(*point,body),desired[index],.6499999761581421f,.3499999940395355f);
        }
        for(auto index:shoulders){
            auto* point=points_[index];
            point->set(0x158,static_cast<std::uint8_t>(point->template get<std::uint8_t>(0x158)|0xfcu));
        }
        const std::array<unsigned,2> hips{{10,13}};
        std::array<std::size_t,2> slots{};
        for(unsigned index=0;index!=2;++index){
            auto* point=points_[hips[index]];
            if(!point)throw std::runtime_error("Human torso twist requires its actual hip points");
            slots[index]=correction_offset(*point,*body.parent);
            if(!slots[index])return false; //Original returns before child constraints when this actual link is absent.
        }
        for(unsigned row=0;row!=3;++row)relative[row]=static_cast<float>((static_cast<long double>(right[0])*pose[row*4]+
            static_cast<long double>(right[1])*pose[row*4+1])+static_cast<long double>(right[2])*pose[row*4+2]);
        relative[1]=-relative[1];
        right=frames_.collision_sweep_rotate(relative,parent);
        preferred=parent;
        for(unsigned axis=0;axis!=3;++axis)preferred[axis]=right[axis];
        preferred[4]=static_cast<float>(static_cast<long double>(parent[9])*right[2]-static_cast<long double>(parent[10])*right[1]);
        preferred[5]=static_cast<float>(static_cast<long double>(parent[10])*right[0]-static_cast<long double>(parent[8])*right[2]);
        preferred[6]=static_cast<float>(static_cast<long double>(parent[8])*right[1]-static_cast<long double>(parent[9])*right[0]);
        preferred[3]=preferred[7]=preferred[11]=0;
        std::array<Vector,2> previous{};
        for(unsigned index=0;index!=2;++index)previous[index]=frames_.car_transform(points_[hips[index]]->template get<Vector>(0x110),parent);
        for(unsigned index=0;index!=2;++index)desired[index]=frames_.car_transform(points_[hips[index]]->template get<Vector>(0x110),preferred);
        for(unsigned index=0;index!=2;++index){
            auto* point=points_[hips[index]];
            for(unsigned axis=0;axis!=3;++axis)desired[index][axis]-=previous[index][axis];
            blend_correction(*point,slots[index],desired[index],.6499999761581421f,.3499999940395355f);
        }
        for(auto index:hips){
            auto* point=points_[index];
            point->set(0x158,static_cast<std::uint8_t>(point->template get<std::uint8_t>(0x158)|0xfcu));
        }
        return true;
    }
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
