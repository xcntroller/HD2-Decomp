#ifndef HD2_GAME_RIGID_BODY_H
#define HD2_GAME_RIGID_BODY_H

#include "game/rigid_body_bounds.h"
#include "game/rigid_body_geometry.h"
#include "game/rigid_body_motion_math.h"
#include "game/dynamic_collision_grid.h"
#include <algorithm>
#include <cstring>
#include <functional>
#include <map>
#include <optional>
#include <type_traits>
#include <vector>

namespace hd2::game {

template<class Frame>struct RigidBodyCollisionDescriptor {
    using Self=RigidBodyCollisionDescriptor<Frame>;
    std::uint32_t flags=0,mask=0;
    Frame frame{};
    DynamicCollisionBucket<Self>* bucket=nullptr;
    std::array<float,4> minimum{},maximum{},position{};
    const std::array<float,16>* matrix=nullptr;
    const std::array<float,16>* inverse=nullptr;
    void* actor=nullptr;
    float radius=0;
};

//004C84A0/004C8820/004CD6E0/004C8980/004C8EA0/004C8770 and actual
//004C9000 authority motion. Share the actual registry and tree's buckets;
//engine math/frame calls and World contact queries remain required borrowers.
//Frame is borrowed from its actor; close BEFORE releasing the actor/model/grid.
template<class Frames,class Collisions>
class RigidBody {
public:
    using Frame=typename Frames::Frame;
    using Matrix=typename Frames::Matrix;
    using Vector=typename Frames::Vector;
    using Descriptor=RigidBodyCollisionDescriptor<Frame>;
    using Registry=std::vector<RigidBody*>;
    struct ContactEvent {Descriptor* primitive;Vector position,normal;};
    using Callback=std::function<int(unsigned,void*,ContactEvent*)>;
    struct MotionHit {Descriptor primitive;Vector position{},normal{};};
    struct Parameters {
        Vector motion{};
        float mass,coefficient;
        Callback callback;
        void* callback_context=nullptr;
        bool flag40=false,flag41=false;
    };
    struct WheelParameters {Vector motion{};float radius,width,mass,coefficient,field594;Callback callback;bool flag4c=false;};
    RigidBody(Frames& frames,Collisions& collisions,Registry& registry)
        : frames_(frames),collisions_(collisions),registry_(registry) {
        //Only original cold writes are initialized. Unwritten scalar reads
        //throw instead of turning uninitialized client memory into zeroes.
        for(auto offset:{0x560u,0x58cu,0x590u,0x200u,0x204u,0x208u,0x210u,0x214u,0x218u})word(offset,0);
        word(0x580,0x41200000);word(0x568,0x3ce56042);
        word(0xe0,0);word(0xe4,0x3f800000);word(0xe8,0);
        word(0x5ac,0xffffffffu);word(0x5b0,0xffffffffu);
        word(0x5c0,0);word(0x5c4,0);word(0x5c8,4);
        registry_.push_back(this); //Actual0089BBC8 list, after cold members.
    }
    ~RigidBody() {
        close(); //Native guard also retracts partial descriptor publication.
        const auto found=std::find(registry_.rbegin(),registry_.rend(),this);
        if(found!=registry_.rend()) {
            const auto index=static_cast<std::size_t>(found.base()-registry_.begin()-1);
            registry_[index]=registry_.back();registry_.pop_back();
        }
    }
    RigidBody(const RigidBody&)=delete;
    RigidBody& operator=(const RigidBody&)=delete;
    std::uint32_t word(std::size_t offset) const {return fields_.at(offset);}
    float real(std::size_t offset) const {const auto bits=word(offset);float result;std::memcpy(&result,&bits,4);return result;}
    void word(std::size_t offset,std::uint32_t value) {fields_[offset]=value;}
    void real(std::size_t offset,float value) {std::uint32_t bits;std::memcpy(&bits,&value,4);word(offset,bits);}
    Frame frame() const {return frame_;}
    Descriptor& descriptor() {return descriptor_;}
    const Matrix& matrix() const {return world_;}
    const Matrix& inverse() const {return inverse_;}
    const RigidBodyGeometry& geometry() const {return geometry_.value();}
    void bind(Frame frame) { //004C8820, no frame AddRef.
        if(!frame||frame_||descriptor_.bucket)throw std::logic_error("Rigid body frame binding is missing or repeated");
        frame_=frame;const auto initial=frames_.actor_world_matrix(frame_);
        copy_vector(0x20,{{initial[12],initial[13],initial[14],initial[15]}});
        copy_matrices();word(0x10,1);
    }
    void configure(const Parameters& parameters) { //004CD6E0, before004C8980.
        if(!frame_)throw std::logic_error("Rigid body parameters require a bound frame");
        callback_=parameters.callback;callback_context_=parameters.callback_context;
        flag4c_=parameters.flag41;flag4d_=parameters.flag40;
        real(0x56c,parameters.mass);real(0x570,parameters.coefficient);
        copy_vector(0x200,parameters.motion);
        if(length_squared(parameters.motion)<9.99999974737875163555145263671875e-6f)state(1);
        geometry_=configure_rigid_body_geometry(rigid_body_bounds(frames_,frame_),parameters.mass,parameters.coefficient,real(0x580));
        const auto& g=*geometry_;
        copy_xyz(0x90,g.centre90);flag5fb_=true;
        copy_xyz(0x1f0,g.inverse_inertia1f0);
        for(unsigned i=0;i!=14;++i)copy_xyz(0x230+i*16,g.points230[i]);
        copy_xyz(0x220,g.gravity220);
        real(0x574,g.coefficient574);real(0x578,g.coefficient578);real(0x57c,g.coefficient57c);
        real(0x584,g.coefficient584);real(0x588,g.radius588);word(0x560,g.mode560);
    }
    void actor(void* value) {actor_=value;descriptor_.actor=value;} //004C8250: body88 and descriptor48.
    void configure_wheel(const WheelParameters& parameters){ //004CD550, same body and motion fields.
        if(!frame_)throw std::logic_error("Rigid wheel parameters require actual bound frame");
        real(0x56c,parameters.mass);real(0x590,parameters.radius);real(0x58c,parameters.width);
        if(!std::isfinite(parameters.radius)||parameters.radius<=0||!std::isfinite(parameters.mass)||parameters.mass<=0)
            throw std::runtime_error("Invalid rigid wheel radius/mass");
        const auto radius=static_cast<long double>(real(0x590));
        const float inverse=static_cast<float>(1.L/(((radius*radius)*real(0x56c))*.5f));
        copy_xyz(0x1f0,{{inverse,inverse,inverse}});copy_vector(0x200,parameters.motion);
        if(length_squared(parameters.motion)<9.99999974737875163555145263671875e-6f)state(1);
        real(0x570,parameters.coefficient);
        const float zero=static_cast<float>(static_cast<long double>(real(0x56c))*0.f),negative=-real(0x56c);
        copy_xyz(0x220,{{zero*real(0x580),negative*real(0x580),zero*real(0x580)}});
        const auto gravity=static_cast<long double>(real(0x580))*real(0x56c);
        real(0x594,parameters.field594);real(0x578,.029999999329447746f);real(0x574,1.0499999523162842f);
        word(0x560,1);real(0x57c,static_cast<float>(gravity*37.03704071044922f));
        callback_=parameters.callback;callback_context_=nullptr;flag4c_=parameters.flag4c;flag4d_=true;wheel_configured_=true;
        //Native does not clear5FB/old geometry when reconfiguring a prior part.
    }
    void reset() { //004C8980.
        if(!frame_||(!geometry_&&!wheel_configured_))throw std::logic_error("Rigid body reset requires frame and actual configured geometry");
        if(descriptor_.bucket)throw std::logic_error("Rigid body reset requires prior collision removal");
        actor(nullptr);
        point_actor_.fill(nullptr);point_surface_.fill(0);
        for(unsigned i=0;i!=14;++i) {word(0x84-i*4,0);flags550_[i]=0;flags5ea_[i]=0;}
        copy_matrices();
        const auto current=frames_.actor_world_matrix(frame_);
        Vector position{{current[12],current[13],current[14],current[15]}};copy_vector(0x20,position);
        const auto bounds=rigid_body_bounds(frames_,frame_);
        descriptor_=Descriptor{};descriptor_.flags=0x40;descriptor_.frame=frame_;
        descriptor_.minimum=bounds.minimum;descriptor_.maximum=bounds.maximum;
        descriptor_.position=position;descriptor_.radius=bounding_radius(bounds);
        descriptor_.matrix=&world_;descriptor_.inverse=&inverse_;descriptor_.mask=flag4d_?0x801:0x80802;
        collisions_.insert(descriptor_);collisions_.move(descriptor_);
        if(flag5fb_)copy_vector(0xa0,frames_.car_transform({{geometry_->centre90[0],geometry_->centre90[1],geometry_->centre90[2],0}},world_));
        else copy_vector(0xa0,position);
        word(0x598,0);copy_xyz(0x210,{{0,0,0}});word(0x5a0,0);copy_xyz(0x3f0,{{0,0,0}});
        word(0x5a4,0x3f19999a);word(0x564,0);copy_vector(0xb0,position);
        for(unsigned i=0;i!=7;++i)word(0x5cc+i*4,0);
        field5e8_=0;word(0x5b4,0);word(0x5b8,0);word(0x5a8,0x3f800000);flag5fa_=true;
        const Vector motion{{real(0x200),real(0x204),real(0x208),0}};
        state(length_squared(motion)<.00999999977648258209228515625f?1:0);
        for(unsigned i=0;i!=14;++i)copy_xyz(0x3e0-i*16,{{0,0,0}});
        flag4e_=true;
    }
    void state(std::uint32_t value) { //004C8270.
        const auto previous=word(0x10);if(value==previous)return;
        if(previous==0&&value==1&&callback_)callback_(0,callback_context_,nullptr);
        word(0x10,value);
        if(value==0){word(0x5a0,0);return;}
        if(!flag4c_)return;
        const auto bounds=rigid_body_bounds(frames_,frame_);
        descriptor_.flags=0x40;descriptor_.radius=static_cast<float>(static_cast<long double>(bounding_radius(bounds))*2);
        descriptor_.frame=frame_;descriptor_.matrix=&world_;descriptor_.inverse=&inverse_;descriptor_.actor=actor_;
        descriptor_.position={{real(0x20),real(0x24),real(0x28),real(0x2c)}};
        for(unsigned axis=0;axis!=3;++axis) {
            descriptor_.minimum[axis]=static_cast<float>(static_cast<long double>(bounds.minimum[axis])*2);
            descriptor_.maximum[axis]=static_cast<float>(static_cast<long double>(bounds.maximum[axis])*2);
        }
        collisions_.move(descriptor_);
    }
    void close() noexcept { //004C8EA0: descriptor removal precedes the owned contact array's free.
        static_assert(noexcept(std::declval<Collisions&>().remove_for_cleanup(std::declval<Descriptor&>())),
                      "Rigid body descriptor must detach before its owner is destroyed");
        collisions_.remove_for_cleanup(descriptor_);
        std::vector<CachedActor>().swap(contacts_);word(0x5c0,0);word(0x5c4,0);
    }
    void wake() { //004C9480, body14; absent from the frozen inventory.
        if(word(0x10)!=0)return;
        if(flag5fa_){word(0x5a0,0);word(0x564,0);}
        flag5fa_=true;
    }
    void apply_impulse(const Vector& force,const Vector& point) { //004CE7A0.
        using namespace rigid_motion;
        state(0);word(0x564,0);word(0x5a0,0);
        const auto offset=subtract(point,vector(0xa0));
        const float inverse_mass=stored(1.L/real(0x56c));
        copy_xyz_vector(0x200,add(vector(0x200),scale(force,inverse_mass)));
        auto torque=cross(offset,force);
        frames_.car_transform_in_place(torque,previous_inverse_);
        torque=multiply(torque,vector(0x1f0));
        frames_.car_transform_in_place(torque,previous_world_);
        copy_xyz_vector(0x210,subtract(vector(0x210),torque));
    }
    void advance_pose(float elapsed,const Vector& displacement) { //004CC520, RET8.
        using namespace rigid_motion;
        auto local_inverse=inverse_;for(unsigned n=0;n!=3;++n)local_inverse[12+n]=0;
        auto angles=scale(vector(0x210),elapsed);frames_.car_transform_in_place(angles,local_inverse);
        auto rotation=frames_.car_axis_quaternion(vector(0xc0),angles[0]);
        rotation=quaternion_product(rotation,frames_.car_axis_quaternion(vector(0xd0),angles[1]),false);
        rotation=quaternion_product(rotation,frames_.car_axis_quaternion(vector(0x30),angles[2]),true);
        auto matrix=frames_.car_quaternion_matrix(rotation);
        auto forward=vector(0x30),up=vector(0xd0);
        frames_.car_transform_in_place(forward,matrix);copy_vector(0x30,forward);
        frames_.car_transform_in_place(up,matrix);copy_vector(0xd0,up);
        if(flag5fb_){
            Matrix before{{1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1}},after=before;
            const auto centre=vector(0xa0);
            for(unsigned n=0;n!=3;++n){before[12+n]=-centre[n];after[12+n]=centre[n];}
            after[15]=centre[3]; //Negative-vector constructor does not produce a fourth lane.
            matrix=frames_.rigid_matrix_product(frames_.rigid_matrix_product(before,matrix),after);
            auto position=vector(0x20);frames_.car_transform_in_place(position,matrix);copy_vector(0x20,position);
        }
        auto right=cross(up,forward);copy_vector(0xc0,right);
        up=cross(forward,right);copy_vector(0xd0,up);
        copy_vector(0xc0,normalize(right));copy_vector(0xd0,normalize(up));copy_vector(0x30,normalize(forward));
        copy_xyz_vector(0x20,add(vector(0x20),displacement));
        frames_.rigid_body_pose(frame_,vector(0x20),vector(0x30),vector(0xd0));
    }
    void contact_impulse(const Vector& normal,const Vector& point,
                         float elapsed,std::uint32_t point_index,void* other_actor) { //004CCCC0, RET28.
        using namespace rigid_motion;
        (void)elapsed;(void)other_actor; //Actual native body does not read these two arguments.
        const auto offset=subtract(point,vector(0xa0));
        const auto velocity=subtract(vector(0x200),cross(vector(0x210),offset));
        auto response=cross(offset,normal);
        frames_.car_transform_in_place(response,previous_inverse_);
        response=multiply(response,vector(0x1f0));frames_.car_transform_in_place(response,previous_world_);
        float incoming=stored((static_cast<long double>(velocity[0])*normal[0]+static_cast<long double>(velocity[1])*normal[1])+static_cast<long double>(velocity[2])*normal[2]);
        if(!(incoming<0))return;
        incoming=stored(-static_cast<long double>(real(0x574))*incoming);
        const auto perpendicular=cross(response,offset);
        const float inverse_mass=stored(1.L/real(0x56c));
        float denominator=stored(dot(perpendicular,normal)+inverse_mass);
        if(std::abs(static_cast<long double>(denominator))<9.999999974752427e-7)denominator=9.999999974752427e-7f;
        const float strength=stored(static_cast<long double>(incoming)/denominator);
        const auto impulse=scale(normal,strength);
        copy_xyz_vector(0x200,add(vector(0x200),scale(impulse,inverse_mass)));
        response=cross(offset,impulse);frames_.car_transform_in_place(response,previous_inverse_);
        response=multiply(response,vector(0x1f0));frames_.car_transform_in_place(response,previous_world_);
        copy_xyz_vector(0x210,subtract(vector(0x210),response));
        const float magnitude=length(impulse);
        if(real(0x5b4)<magnitude){real(0x5b4,magnitude);field5e8_=static_cast<std::uint16_t>(point_index);copy_vector(0x400,point);}
        auto friction=normalize(velocity);friction=normalize(cross(cross(friction,normal),normal));
        auto amount=static_cast<long double>(magnitude)*real(0x570)*std::sin(frames_.rigid_vector_angle(velocity,normal));
        const float cap=stored(((static_cast<long double>(real(0x580))*real(0x570))*real(0x56c))*3.f);
        if(cap<amount)amount=cap;
        friction=scale(friction,amount);copy_xyz_vector(0x200,add(vector(0x200),scale(friction,inverse_mass)));
        response=cross(friction,offset);frames_.car_transform_in_place(response,previous_inverse_);
        response=multiply(response,vector(0x1f0));frames_.car_transform_in_place(response,previous_world_);
        copy_xyz_vector(0x210,add(vector(0x210),response));
    }
    void refresh_frame_state() { //004CEAA0; retain native repeated frame observations.
        copy_motion_matrices();
        if(flag5fb_)copy_vector(0xa0,frames_.car_transform(vector(0x90),world_));
        else {const auto m=frames_.actor_world_matrix(frame_);copy_vector(0xa0,{{m[12],m[13],m[14],m[15]}});}
        const auto m=frames_.actor_world_matrix(frame_);copy_vector(0x20,{{m[12],m[13],m[14],m[15]}});
        copy_axes();
    }
    template<class Runtime>bool filter_contact(MotionHit& hit,std::uint8_t& dynamic,Runtime& runtime){ //004CE990.
        if(callback_){ContactEvent event{&hit.primitive,hit.position,hit.normal};if(callback_(1,callback_context_,&event)==1)return false;}
        const auto surface=static_cast<std::uint8_t>(hit.primitive.flags>>24);
        if(surface==0x55||surface==0x43)return true;
        if(surface==0x1f||surface==0x20||surface==0x46)return false;
        if((hit.primitive.flags&0x40)&&hit.primitive.actor){
            if(flag5fd_)return false;
            const auto type=runtime.rigid_actor_type(hit.primitive.actor);
            dynamic=type==4||type==25?1:0;
        }
        return true;
    }
    bool settle_shape() { //004CF440: elongated parts receive a real toppling impulse before sleep.
        using namespace rigid_motion;
        if(word(0x560)!=2)return true;
        if(!contacts_.empty())return false;
        std::array<Vector,3> axes;
        for(unsigned n=0;n!=3;++n){Vector unit{};unit[n]=1;axes[n]=normalize(subtract(frames_.car_transform(unit,world_),vector(0xa0)));}
        for(auto& axis:axes)if(axis[1]<0)axis=scale(axis,-1);
        unsigned selected;
        if(axes[0][1]>axes[1][1]&&axes[0][1]>axes[2][1])selected=0;
        else if(axes[1][1]>axes[0][1]&&axes[1][1]>axes[2][1])selected=1;
        else if(axes[2][1]>axes[0][1]&&axes[2][1]>axes[1][1])selected=2;
        else return true;
        const auto bounds=rigid_body_bounds(frames_,frame_);
        std::array<float,3> extent{};for(unsigned n=0;n!=3;++n)extent[n]=stored(static_cast<long double>(bounds.maximum[n])-bounds.minimum[n]);
        const unsigned a=selected==0?1:0,b=selected==2?1:2;
        if(extent[selected]<=static_cast<long double>(extent[a])*2.5f&&extent[selected]<=static_cast<long double>(extent[b])*2.5f)return true;
        const float other=static_cast<long double>(extent[a])*2.5f<extent[selected]?extent[a]:extent[b];
        const float ratio=stored(static_cast<long double>(other)/extent[selected]);
        const float inverse_ratio=stored(1.L/ratio);
        const auto direction=normalize(add(axes[a],axes[b]));auto point=vector(0xa0);
        point[1]=stored(static_cast<long double>(length(vector(0x230)))*inverse_ratio+point[1]);
        const float mass=real(0x56c);Vector force{};
        force[0]=stored((static_cast<long double>(mass)*direction[0])*.11999999731779099f);
        force[1]=stored((static_cast<long double>(direction[1])*.11999999731779099f)*mass);
        force[2]=stored((static_cast<long double>(direction[2])*.11999999731779099f)*mass);
        apply_impulse(force,point);return false;
    }
    template<class Runtime>void substeps(std::uint32_t elapsed,Runtime& runtime){ //004C8EE0, absent frozen.
        if(!(real(0x5a0)<=real(0x5a4))||word(0x564)>2000)return;
        real(0x5a8,1);const auto original=elapsed;
        if(elapsed>83){elapsed=83;real(0x5a8,static_cast<float>(83.L/original));}
        else if(!elapsed)return;
        const float seconds=static_cast<float>(static_cast<long double>(elapsed)*.0010000000474974513f);
        std::int32_t count=1;
        if(seconds>real(0x568)){
            const auto quotient=static_cast<long double>(seconds)/real(0x568);
            if(!std::isfinite(quotient)||quotient<0||quotient>2147483646.L)throw std::runtime_error("Rigid substep count exceeds native range");
            count=static_cast<std::int32_t>(quotient)+1;
        }
        const float step=static_cast<float>(static_cast<long double>(seconds)/count);
        for(std::int32_t n=0;n<count;++n){if(word(0x560)==1)step_wheel(step,runtime);else step_part(step,runtime);}
    }
    template<class Runtime>void update(std::uint32_t elapsed,Runtime& runtime){ //004C9000, actual body24.
        using namespace rigid_motion;
        if(word(0x10)==1)return;
        substeps(elapsed,runtime);elapsed=std::min(elapsed,83u);if(!elapsed)return;
        const float seconds=stored(static_cast<long double>(elapsed)*.0010000000474974513f);
        if(length_extended(vector(0x200))>=.30000001192092896f||length_extended(vector(0x210))>=.10000000149011612f)word(0x5a0,0);
        else real(0x5a0,stored(static_cast<long double>(seconds)+real(0x5a0)));
        const auto distance=subtract(vector(0x20),vector(0xb0));
        if(square(distance)>=.03999999910593033f){word(0x564,0);copy_vector(0xb0,vector(0x20));}
        else word(0x564,word(0x564)+elapsed);
        if(real(0x5a4)<real(0x5a0)||word(0x564)>2000){
            if(contacts_.empty()){if(settle_shape())state(1);}
            else for(auto index=contacts_.size();index;){auto& contact=contacts_[--index];
                const auto current=runtime.rigid_actor_position(contact.actor);
                if(square(subtract(contact.position,current))>1.5999999959603883e-5f){
                    contact.position=current;word(0x5a0,0);word(0x564,0);flag4e_=true;
                    copy_vector(0xa0,flag5fb_?frames_.car_transform(vector(0x90),world_):vector(0x20));break;
                }
            }
        }else for(auto index=contacts_.size();index;){auto& contact=contacts_[--index];
            const auto bits=static_cast<std::uint32_t>(contact.remaining)-elapsed;std::memcpy(&contact.remaining,&bits,4);
            if(contact.remaining<0){contacts_.erase(contacts_.begin()+static_cast<std::ptrdiff_t>(index));word(0x5c4,static_cast<std::uint32_t>(contacts_.size()));flag4e_=true;}
        }
        if(real(0x204)<-85.f)state(2);
        frames_.rigid_publish_frame(frame_);refresh_frame_state();
        descriptor_.position=frames_.rigid_descriptor_position(frame_);collisions_.move(descriptor_);
        word(0x5b4,0);field5e8_=0;
    }
    template<class Runtime>void step_wheel(float elapsed,Runtime& runtime){ //004C94B0.
        using namespace rigid_motion;
        Vector force{},torque{};
        if(real(0x598)>0){
            const auto velocity=vector(0x200);const float speed=length(velocity);
            auto rolling=normalize(scale(velocity,-1));if(speed<1)rolling=scale(rolling,speed);
            const float rolling_scale=stored(((static_cast<long double>(real(0x594))/real(0x590))*real(0x580))*real(0x56c));
            force=scale(rolling,rolling_scale);
            const auto normal=vector(0xe0),position=vector(0x20);
            auto radius=scale(normal,-real(0x590));
            //The first torque retains the add/subtract position round trip.
            torque=cross(force,subtract(add(radius,position),position));
            const auto contact_velocity=subtract(velocity,cross(vector(0x210),radius));
            const float friction_scale=stored((-static_cast<long double>(real(0x570))*real(0x580))*real(0x56c));
            auto friction=scale(normalize(contact_velocity),friction_scale);
            const auto sine=std::sin(frames_.rigid_vector_angle(contact_velocity,normal));
            if(sine<1)friction=scale(friction,sine);
            const float contact_speed=length(contact_velocity);if(contact_speed<1)friction=scale(friction,contact_speed);
            force=add(force,friction);torque=add(torque,cross(friction,radius));
            const auto support=scale(normal,real(0x598));force=add(force,support);
            torque=add(torque,cross(support,subtract(vector(0x3f0),position)));
        }
        const float time_mass=stored(static_cast<long double>(elapsed)/real(0x56c));
        copy_xyz_vector(0x200,add(vector(0x200),scale(add(force,vector(0x220)),time_mass)));
        auto travel=scale(vector(0x200),elapsed);
        frames_.car_transform_in_place(torque,previous_inverse_);torque=multiply(torque,vector(0x1f0));
        frames_.car_transform_in_place(torque,previous_world_);
        copy_xyz_vector(0x210,add(vector(0x210),scale(torque,elapsed)));
        const auto old_position=vector(0x20),old_forward=vector(0x30),old_up=vector(0xd0),old_right=vector(0xc0);
        copy_vector(0xa0,old_position);advance_pose(elapsed,travel);word(0x598,0);real(0x59c,real(0x578));
        Vector down{};down[1]=stored(-((static_cast<long double>(real(0x590))+real(0x590))+
            ((static_cast<long double>(travel[2])+travel[0])*0.f-static_cast<long double>(travel[1]))+real(0x578)));
        point_surface_[0]=0;
        auto hit=with_query_exclusion(runtime,[&]{return runtime.rigid_ray(old_position,down,0x8002u,0x210000u);});
        Vector point=vector(0x3f0);std::uint8_t dynamic=0;
        if(hit){copy_vector(0xe0,hit->normal);point=hit->position;}
        if(hit&&filter_contact(*hit,dynamic,runtime)){
            if(flag5fc_&&vector(0xe0)[1]>.8999999761581421f){copy_xyz(0x200,{{0,0,0}});copy_xyz(0x210,{{0,0,0}});state(1);return;}
            auto plane=vector(0xe0);plane[3]=stored(-((static_cast<long double>(point[0])*plane[0]+static_cast<long double>(point[2])*plane[2])+static_cast<long double>(point[1])*plane[1]));
            real(0x59c,stored((static_cast<long double>(stored(dot(vector(0x20),plane)))+plane[3])-real(0x590)));
            if(real(0x59c)<=real(0x578)){
                point_surface_[0]=static_cast<std::uint8_t>(hit->primitive.flags>>24);
                point=wheel_contact_point(plane);real(0x598,stored((static_cast<long double>(real(0x578))-real(0x59c))*real(0x57c)));
            }
        }
        const bool penetrating=real(0x59c)<9.999999747378752e-5f;copy_vector(0x3f0,point);
        if(penetrating&&dot(vector(0x200),vector(0xe0))<0){
            copy_vector(0x30,old_forward);copy_vector(0xd0,old_up);copy_vector(0xc0,old_right);copy_vector(0x20,old_position);
            frames_.rigid_body_pose(frame_,old_position,old_forward,old_up);copy_vector(0xa0,old_position);
            contact_impulse(vector(0xe0),point,elapsed,0,nullptr);
            advance_pose(elapsed,scale(vector(0x200),elapsed));
        }
        real(0x59c,real(0x578));travel=scale(vector(0x200),elapsed);
        const float reach=stored(static_cast<long double>(length(travel))+real(0x590));
        const auto sweep=scale(normalize(travel),reach);
        hit=with_query_exclusion(runtime,[&]{return runtime.rigid_ray(old_position,sweep,0x8002u,0x210000u);});
        //A missing ray does not produce a normal; the native distance gate
        //keeps that scratch out of the impulse path.
        Vector normal{};
        if(hit){normal=hit->normal;point=hit->position;}
        if(hit&&filter_contact(*hit,dynamic,runtime)){
            auto plane=normal;plane[3]=stored(-((static_cast<long double>(normal[1])*point[1]+static_cast<long double>(normal[2])*point[2])+static_cast<long double>(normal[0])*point[0]));
            real(0x59c,stored((static_cast<long double>(stored(dot(vector(0x20),plane)))+plane[3])-real(0x590)));
            if(real(0x59c)<=real(0x578))point=wheel_contact_point(plane);
        }
        if(real(0x59c)<9.999999747378752e-5f&&dot(normal,vector(0x200))<0){copy_vector(0xa0,vector(0x20));contact_impulse(normal,point,elapsed,0,nullptr);}
        copy_motion_matrices();copy_vector(0xa0,vector(0x20));
    }
    template<class Runtime>void step_part(float elapsed,Runtime& runtime){ //004CB2B0.
        using namespace rigid_motion;
        const float time_mass=stored(static_cast<long double>(elapsed)/real(0x56c));
        const auto alternating=fields_.find(0x5f9);
        if(alternating==fields_.end())throw std::runtime_error("Rigid part5F9 alternating-query byte has no native producer");
        word(0x5f9,!alternating->second);
        copy_xyz_vector(0x200,add(vector(0x200),scale(vector(0x220),time_mass)));
        const auto travel=scale(vector(0x200),elapsed),old_centre=vector(0xa0);
        advance_pose(elapsed,travel);auto matrix=frames_.actor_world_matrix(frame_);
        copy_vector(0xa0,flag5fb_?frames_.car_transform(vector(0x90),matrix):vector(0x20));
        Vector correction{};
        std::array<void*,14> actors{};
        std::array<Vector,14> positions{},points{},normals{};
        if(square(vector(0x200))<1){word(0x5f9,1);copy_xyz_vector(0x210,scale(vector(0x210),1.L-static_cast<long double>(elapsed)*.5f));}
        std::uint16_t collision_mask=0;
        for(unsigned index=14;index;){--index;
            positions[index]=frames_.car_transform(vector(0x230+index*16),matrix);
            const auto relative=subtract(positions[index],old_centre);
            if(!(dot(relative,vector(0x200))>0)&&!word(0x5f9))continue;
            auto displacement=add(relative,scale(normalize(relative),.30000001192092896f));
            bool force_query=flag4e_;
            for(auto n=contacts_.size();n;)if(contacts_[--n].point==index){force_query=true;break;}
            if(square(subtract(positions[index],vector(0x310+index*16)))<=real(0x588)&&!force_query){
                for(unsigned other=14;other;){--other;if(!flags550_[other])continue;
                    const auto& plane=cached_planes_[other];const float denominator=stored(dot(displacement,plane));
                    if(std::abs(static_cast<long double>(denominator))<9.99999993922529e-9f)continue;
                    const float factor=stored(-(static_cast<long double>(stored(dot(plane,old_centre)))+plane[3])/denominator);
                    points[index]=add(old_centre,scale(displacement,factor));
                    const float depth=stored(static_cast<long double>(stored(dot(positions[index],plane)))-stored(dot(points[index],plane)));
                    real(0x59c,depth);
                    if(depth<=0){
                        normals[index]=plane;collision_mask|=static_cast<std::uint16_t>(1u<<index);
                        if(point_actor_[other]){actors[index]=point_actor_[other];flags5ea_[index]=flags5ea_[other];}
                        break;
                    }
                }
                continue;
            }
            auto hit=with_query_exclusion(runtime,[&]{
                auto found=runtime.rigid_ray(old_centre,displacement,0x88002u,0x200000u);
                if(found)copy_vector(0xe0,found->normal);
                if(found&&(found->primitive.flags&0x10000u)){
                    real(0x580,9.f);
                    if(length_extended(vector(0x200))>1)copy_xyz_vector(0x200,scale(vector(0x200),.75f));
                    auto water=found->position;water[1]=stored(static_cast<long double>(water[1])+.019999999552965164f);
                    runtime.rigid_water_particle(water); //00445150(45,position,up,NULL,false,false) inside SAME exclusion window.
                    found=runtime.rigid_ray(old_centre,displacement,0x88002u,0x210000u);
                }
                return found;
            });
            copy_vector(0x310+index*16,positions[index]);flags550_[index]=0;
            if(hit)copy_vector(0xe0,hit->normal);
            if(!hit||!filter_contact(*hit,flags5ea_[index],runtime))continue;
            points[index]=hit->position;auto plane=hit->normal;
            plane[3]=stored(-((static_cast<long double>(plane[0])*points[index][0]+static_cast<long double>(plane[1])*points[index][1])+static_cast<long double>(plane[2])*points[index][2]));
            cached_planes_[index]=plane;flags550_[index]=1;point_actor_[index]=nullptr;
            if(hit->primitive.flags&0x40u){actors[index]=hit->primitive.actor;point_actor_[index]=actors[index];}
            point_surface_[index]=static_cast<std::uint8_t>(hit->primitive.flags>>24);
            const float predicted=stored((static_cast<long double>(positions[index][0])*plane[0]+static_cast<long double>(positions[index][2])*plane[2])+static_cast<long double>(positions[index][1])*plane[1]);
            const float depth=stored(static_cast<long double>(predicted)-stored(dot(points[index],plane)));real(0x59c,depth);
            if(depth<=0){normals[index]=hit->normal;collision_mask|=static_cast<std::uint16_t>(1u<<index);}
        }
        if(collision_mask){
            bool low_speed=square(vector(0x200))<=.6000000238418579f;
            for(unsigned index=14;index;){--index;if(!(collision_mask&(1u<<index)))continue;
                const auto offset=subtract(positions[index],vector(0xa0));
                const auto velocity=subtract(vector(0x200),cross(vector(0x210),offset));
                if(dot(velocity,normals[index])<0){
                    low_speed=true;
                    contact_impulse(normals[index],positions[index],elapsed,index,flags5ea_[index]?nullptr:actors[index]);
                }
                if(actors[index])remember_contact(actors[index],index,runtime);
            }
            (void)low_speed; //Original value is assigned again but has no subsequent consumer.
            matrix=frames_.actor_world_matrix(frame_);
            for(unsigned index=14;index;){--index;if(!(collision_mask&(1u<<index)))continue;
                positions[index]=frames_.car_transform(vector(0x230+index*16),matrix);
                const auto& normal=normals[index];
                const float predicted=stored((static_cast<long double>(positions[index][0])*normal[0]+static_cast<long double>(positions[index][1])*normal[1])+static_cast<long double>(positions[index][2])*normal[2]);
                const float observed=stored((static_cast<long double>(points[index][0])*normal[0]+static_cast<long double>(points[index][1])*normal[1])+static_cast<long double>(points[index][2])*normal[2]);
                const float depth=stored(static_cast<long double>(predicted)-observed);real(0x59c,depth);
                if(depth<=0){const auto candidate=scale(normal,-depth);
                    if((!actors[index]||flags5ea_[index])&&square(correction)<square(candidate))correction=candidate;
                }
            }
        }
        copy_xyz_vector(0x20,add(vector(0x20),correction));
        if(real(0x580)<10.f){const float speed=length(vector(0x200));if(speed>.5f){
            const float slowed=stored(static_cast<long double>(speed)-static_cast<long double>(elapsed)*200.f);
            copy_xyz_vector(0x200,scale(normalize(vector(0x200)),slowed<.5f?.5f:slowed));
        }}
        frames_.rigid_body_position(frame_,vector(0x20));frames_.update_scene_object(frame_);
        copy_motion_matrices();copy_vector(0xa0,flag5fb_?frames_.car_transform(vector(0x90),world_):vector(0x20));flag4e_=false;
    }
private:
    struct CachedActor {Vector position;void* actor;std::int32_t remaining;std::uint32_t point;};
    template<class Runtime,class Operation>auto with_query_exclusion(Runtime& runtime,Operation&& operation){
        auto& exclusions=runtime.rigid_exclusions();exclusions.single(&descriptor_);
        struct Clear{std::remove_reference_t<decltype(exclusions)>& owner;~Clear(){owner.clear();}}clear{exclusions};
        return operation();
    }
    template<class Runtime>void remember_contact(void* actor,std::uint32_t point,Runtime& runtime){
        for(auto n=contacts_.size();n;)if(contacts_[--n].actor==actor){contacts_[n].remaining=1000;return;}
        const auto position=runtime.rigid_actor_position(actor);
        if(contacts_.size()>=word(0x5c0)){const auto capacity=word(0x5c0)+word(0x5c8);word(0x5c0,capacity);contacts_.reserve(capacity);}
        contacts_.push_back({position,actor,1000,point});word(0x5c4,static_cast<std::uint32_t>(contacts_.size()));
    }
    Vector wheel_contact_point(const Vector& plane){
        using namespace rigid_motion;
        auto axial=vector(0xc0);const auto position=vector(0x20);
        axial[3]=stored(-((static_cast<long double>(position[0])*axial[0]+static_cast<long double>(axial[1])*position[1])+static_cast<long double>(axial[2])*position[2]));
        Vector point{},direction{};
        const bool intersects=frames_.rigid_intersection_line(axial,plane,point,direction);
        auto edge=position;
        if(intersects){direction=normalize(cross(vector(0xc0),direction));edge=add(scale(direction,real(0x590)),position);}
        float half_width=stored(static_cast<long double>(real(0x58c))*.5f);
        if(dot(vector(0xc0),plane)>0)half_width=-half_width;
        point=add(scale(vector(0xc0),half_width),edge);point[3]=vector(0xc0)[3];
        real(0x59c,stored(static_cast<long double>(stored(dot(point,plane)))+plane[3]));
        if(real(0x59c)<=0)word(0x59c,0);
        return point;
    }
    Vector vector(std::size_t offset)const {
        Vector result{};
        for(unsigned n=0;n!=3;++n)result[n]=real(offset+n*4);
        const auto lane=fields_.find(offset+12);if(lane!=fields_.end())std::memcpy(&result[3],&lane->second,4);
        return result; //Unproduced SIMD padding keeps the initialized source lane.
    }
    void copy_xyz_vector(std::size_t offset,const Vector& value){for(unsigned n=0;n!=3;++n)real(offset+n*4,value[n]);}
    static long double length_squared(const Vector& value) {
        return (static_cast<long double>(value[0])*value[0]+static_cast<long double>(value[1])*value[1])+
            static_cast<long double>(value[2])*value[2];
    }
    template<class Bounds>static float bounding_radius(const Bounds& bounds) {
        float squared=static_cast<float>(length_squared(bounds.minimum));
        const auto other=length_squared(bounds.maximum);
        if(squared<other)squared=static_cast<float>(other);
        if(!std::isfinite(squared)||squared<0)throw std::runtime_error("Invalid rigid body bounding radius");
        std::int32_t bits;std::memcpy(&bits,&squared,4);
        if(bits<0x322bcc77)return 0;
        const auto delta=std::abs(static_cast<long double>(squared)-1);
        return delta>=9.99999993922529029077850282192230224609375e-9f?static_cast<float>(std::sqrt(static_cast<long double>(squared))):1.f;
    }
    void copy_xyz(std::size_t offset,const std::array<float,3>& value) {for(unsigned i=0;i!=3;++i)real(offset+i*4,value[i]);}
    void copy_vector(std::size_t offset,const Vector& value) {for(unsigned i=0;i!=4;++i)real(offset+i*4,value[i]);}
    void copy_motion_matrices() {
        world_=frames_.actor_world_matrix(frame_);inverse_=frames_.car_inverse_matrix(world_);
        previous_world_=world_;previous_inverse_=inverse_;
        for(auto* matrix:{&previous_world_,&previous_inverse_})for(unsigned axis=0;axis!=3;++axis)(*matrix)[12+axis]=0;
    }
    void copy_matrices(){copy_motion_matrices();copy_axes();}
    void copy_axes(){
        copy_vector(0xc0,{{world_[0],world_[1],world_[2],world_[3]}});
        copy_vector(0xd0,{{world_[4],world_[5],world_[6],world_[7]}});
        copy_vector(0x30,{{world_[8],world_[9],world_[10],world_[11]}});
    }
    Frames& frames_;Collisions& collisions_;Registry& registry_;
    Frame frame_{};Descriptor descriptor_{};
    std::map<std::size_t,std::uint32_t> fields_;
    Matrix world_{},inverse_{},previous_world_{},previous_inverse_{};
    std::optional<RigidBodyGeometry> geometry_;
    Callback callback_;void* callback_context_=nullptr;void* actor_=nullptr;
    std::vector<CachedActor> contacts_;
    std::array<void*,14> point_actor_{};
    std::array<std::uint16_t,14> point_surface_{};
    std::array<Vector,14> cached_planes_{};
    std::array<std::uint8_t,14> flags550_{},flags5ea_{};
    std::uint16_t field5e8_=0;
    bool flag4c_=false,flag4d_=false,flag4e_=false,flag5fa_=false,flag5fb_=false,flag5fc_=false,flag5fd_=false;
    bool wheel_configured_=false;
};

} //namespace hd2::game
#endif
