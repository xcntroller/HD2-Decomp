#ifndef HD2_GAME_RIGID_BODY_H
#define HD2_GAME_RIGID_BODY_H

#include "game/rigid_body_bounds.h"
#include "game/rigid_body_geometry.h"
#include "game/dynamic_collision_grid.h"
#include <algorithm>
#include <cstring>
#include <functional>
#include <map>
#include <optional>
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

//Loading/registration portion of004C84A0/004C8820/004CD6E0/004C8980/
//004C8EA0/004C8770. Share the actual registry and tree's dynamic buckets.
//Simulation/contact queries are separate receivers, not successful defaults.
//Frame is borrowed from its actor; close BEFORE releasing the actor/model/grid.
template<class Frames,class Collisions>
class RigidBody {
public:
    using Frame=typename Frames::Frame;
    using Matrix=typename Frames::Matrix;
    using Vector=typename Frames::Vector;
    using Descriptor=RigidBodyCollisionDescriptor<Frame>;
    using Registry=std::vector<RigidBody*>;
    using Callback=std::function<int(unsigned,void*,Descriptor*)>;
    struct Parameters {
        Vector motion{};
        float mass,coefficient;
        Callback callback;
        void* callback_context=nullptr;
        bool flag40=false,flag41=false;
    };
    RigidBody(Frames& frames,Collisions& collisions,Registry& registry)
        : frames_(frames),collisions_(collisions),registry_(registry) {
        //Only original cold writes are initialized. Unwritten scalar reads
        //throw instead of turning uninitialized client memory into zeroes.
        for(auto offset:{0x560u,0x58cu,0x590u,0x200u,0x204u,0x208u,0x210u,0x214u,0x218u})word(offset,0);
        word(0x580,0x41200000);word(0x568,0x3ce56042);
        word(0xe0,0);word(0xe4,0x3f800000);word(0xe8,0);
        word(0x5ac,0xffffffffu);word(0x5b0,0xffffffffu);
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
    void reset() { //004C8980.
        if(!frame_||!geometry_)throw std::logic_error("Rigid body reset requires frame and actual geometry");
        if(descriptor_.bucket)throw std::logic_error("Rigid body reset requires prior collision removal");
        actor(nullptr);
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
    void close() noexcept { //004C8EA0; populated contacts require their actual future producer.
        static_assert(noexcept(std::declval<Collisions&>().remove_for_cleanup(std::declval<Descriptor&>())),
                      "Rigid body descriptor must detach before its owner is destroyed");
        collisions_.remove_for_cleanup(descriptor_);
    }
private:
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
    void copy_matrices() {
        world_=frames_.actor_world_matrix(frame_);inverse_=frames_.car_inverse_matrix(world_);
        previous_world_=world_;previous_inverse_=inverse_;
        for(auto* matrix:{&previous_world_,&previous_inverse_})for(unsigned axis=0;axis!=3;++axis)(*matrix)[12+axis]=0;
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
    std::array<std::uint8_t,14> flags550_{},flags5ea_{};
    std::uint16_t field5e8_=0;
    bool flag4c_=false,flag4d_=false,flag4e_=false,flag5fa_=false,flag5fb_=false;
};

} //namespace hd2::game
#endif
