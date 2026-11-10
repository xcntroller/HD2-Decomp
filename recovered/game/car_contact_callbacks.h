#ifndef HD2_GAME_CAR_CONTACT_CALLBACKS_H
#define HD2_GAME_CAR_CONTACT_CALLBACKS_H

#include "game/car_geometry_math.h"
#include "game/actor_frame_data.h"
#include <cstdint>
#include <cstring>
#include <vector>

namespace hd2::game {

using CarContactVector=std::array<float,4>;
using CarContactMatrix=std::array<float,16>;
enum class CarCollisionIdentityKind : std::uint8_t {triangle,shape,dynamic}; //Typed native primitive owner, independent of mutable flags.

//Borrowed native collision descriptor fields; never a cast of serialized or
//original client memory. The actual collision producer must supply real owners.
template<class Actor,class Frame>
struct CarContact {
    std::uint8_t flags,surface;
    Frame frame;
    std::uint32_t flags8;
    Actor actor;
};
//004F32A0's CURRENT simulation inputs, relative to the geometry owner. Loading
//a model/parameter block does not fabricate these dynamic fields or matrices.
struct CarContactState {
    CarContactVector centre1b0,angular210,linear410,inertia220;
    CarContactMatrix matrix310,matrix350;
    float mass918;
};
struct CarContactResponse {
    CarContactVector offset,velocity,angular_response;
    float mass,coefficient;
    bool enabled;
};

template<class Frames>
void car_other_contact_response(Frames& frames,const CarContactState& state,
        const CarContactVector& point,const CarContactVector& normal,CarContactResponse& response) { //004F32A0.
    for(unsigned i=0;i!=3;++i)response.offset[i]=point[i]-state.centre1b0[i];
    response.velocity=state.linear410;
    const auto& r=response.offset;const auto& w=state.angular210;
    response.velocity[0]=static_cast<float>(static_cast<long double>(response.velocity[0])-
        (static_cast<long double>(w[1])*r[2]-static_cast<long double>(w[2])*r[1]));
    response.velocity[1]=static_cast<float>(static_cast<long double>(response.velocity[1])-
        (static_cast<long double>(r[0])*w[2]-static_cast<long double>(w[0])*r[2]));
    //004F3355 stores the Z cross component before subtracting it, unlike X/Y.
    const auto cross_z=static_cast<float>(static_cast<long double>(w[0])*r[1]-static_cast<long double>(r[0])*w[1]);
    response.velocity[2]-=cross_z;
    response.mass=state.mass918;
    auto& a=response.angular_response;
    a[0]=static_cast<float>(static_cast<long double>(normal[2])*r[1]-static_cast<long double>(normal[1])*r[2]);
    a[1]=static_cast<float>(static_cast<long double>(normal[0])*r[2]-static_cast<long double>(r[0])*normal[2]);
    a[2]=static_cast<float>(static_cast<long double>(r[0])*normal[1]-static_cast<long double>(normal[0])*r[1]);
    frames.car_transform_in_place(a,state.matrix350);
    for(unsigned i=0;i!=3;++i)a[i]*=state.inertia220[i];
    frames.car_transform_in_place(a,state.matrix310);
    //Original offset/angular fourth lanes contain unwritten stack data. Native
    //inputs keep their existing lanes; no undefined bytes are manufactured.
}

template<class Backend,class Contact>
bool car_contact_response(Backend& backend,const Contact& contact,const CarContactVector& point,
        const CarContactVector& normal,CarContactResponse& response) { //004D4130, geometry callback688.
    if(!(contact.flags&0x40)||!contact.actor)
        throw std::runtime_error("Car contact response requires its actual collision actor");
    const auto type=backend.actor_type(contact.actor);
    if(type==3) {
        for(unsigned i=0;i!=3;++i)response.offset[i]=response.velocity[i]=response.angular_response[i]=0;
        response.mass=0;response.coefficient=-1;response.enabled=false;return true;
    }
    if(type==4) {
        car_other_contact_response(backend,backend.car_contact_state(contact.actor),point,normal,response);
        response.coefficient=-1.2f;response.enabled=true;return true;
    }
    response.enabled=true;return false;
}

template<class Backend,class Actor,class Contact>
bool car_contact_filter(Backend& backend,Actor self,float speed958,const Contact& contact,bool&) { //004D4620,68C.
    //The third fastcall argument is pushed by00515D5A and consumed by RET4,
    //but this particular callback never reads or writes that output byte.
    if(!(contact.flags&0x40))return true;
    if(!contact.actor)return false;
    switch(backend.actor_type(contact.actor)) {
    case 0:case 1:case 2:case 4:case 10:case 11:case 12:case 22:case 29:case 32:case 33:case 37:return false;
    case 3:
        if(speed958<4.f) {
            const auto other=backend.actor_world_position(contact.actor);
            const auto current=backend.actor_world_position(self);
            return static_cast<long double>(other[1])<static_cast<long double>(current[1])+.3f;
        }
        return false;
    case 30:return speed958<4.f;
    default:return true;
    }
}

inline CarContactVector car_contact_normalize(CarContactVector value) { //004D447C..004D45AC.
    const auto sum=static_cast<long double>(value[2])*value[2]+static_cast<long double>(value[1])*value[1]+
                   static_cast<long double>(value[0])*value[0];
    const auto stored=static_cast<float>(sum);
    if(!std::isfinite(stored))throw std::runtime_error("Non-finite Car contact direction");
    constexpr float epsilon=9.99999993922529e-09f;
    if(std::abs(sum-1.f)>=epsilon) {
        if(stored>=epsilon) {
            const auto scale=1.L/std::sqrt(static_cast<long double>(stored));
            for(unsigned i=0;i!=3;++i)value[i]=static_cast<float>(value[i]*scale);
        }else {
            const auto index=value[0]!=0?0u:value[2]!=0?2u:1u;
            value[index]=value[index]<0?-1.f:1.f;
        }
    }
    return value;
}

template<class Backend,class Actor,class Contact>
void car_human_contact(Backend& backend,Actor self,float speed958,const CarContactVector& axis2d0,
        float mass9a8,const Contact& contact,const CarContactVector& point,
        const CarContactVector& direction,const CarContactVector& normal) { //004D4450,684.
    if(!(contact.flags&0x40))throw std::runtime_error("Car Human contact requires a collision actor descriptor");
    if(!contact.actor||backend.actor_type(contact.actor)!=3)return;
    CarContactVector reverse=direction;
    for(unsigned i=0;i!=3;++i)reverse[i]=-reverse[i];
    reverse=car_contact_normalize(reverse);
    const auto dot=static_cast<long double>(axis2d0[2])*normal[2]+static_cast<long double>(axis2d0[1])*normal[1]+
                   static_cast<long double>(axis2d0[0])*normal[0];
    if(!(speed958<2.f)||std::abs(dot)>.7f)
        backend.human_car_contact(contact.actor,self,point,direction,reverse,mass9a8,contact.frame); //Actual actor+58.
}

template<class Backend,class Part>
void car_hide_contact_part(Backend& backend,const Part& part) { //004D4A10,69C.
    if(!part.frame)throw std::runtime_error("Car contact part has no actual frame");
    if(backend.car_frame_visible(part.frame)&&part.mesh) {
        backend.emit_car_contact_fragments(part.frame,.01f); //Actual GEffects+B4.
        backend.object_visible(part.frame,false);
    }
}
inline void car_empty_contact_callback() noexcept {} //004D49F0 genuine RET10, unused stack inputs.
inline void car_empty_part_callback() noexcept {} //004D4A00 genuine RET.

template<class Backend,class Contact>
Contact* car_physics_contact_filter(Backend& backend,Contact& contact,bool field5e0,
        const CarContactVector& position4a0) { //004D4A70,6A8; two unused stack vectors.
    if(contact.surface==0x46||contact.surface==0x1f||contact.surface==0x20)return nullptr;
    if(!(contact.flags&0x40))return &contact;
    const auto* data=backend.find_actor_frame_data(contact.frame);
    const auto physics=data?data->words[4]:ActorFrameData::empty_text_token();
    if(physics!=ActorFrameData::empty_text_token()) {
        backend.car_physics_contact(physics,field5e0?2u:1u,position4a0,CarContactVector{{0,0,0,0}});
        return nullptr; //00410630→physics module2+4C, before timed-target/flags tests.
    }
    //Original rereads the frame's userdata after its first word4 test.
    data=backend.find_actor_frame_data(contact.frame);
    if(data&&data->words[5]) {
        backend.car_timed_contact(data->words[5],1000,0); //0043C6A0, return ignored.
        return nullptr;
    }
    return (contact.flags8&2)?nullptr:&contact;
}

template<class Backend,class Actor,class Contact>
Contact* car_surface_contact_filter(Backend& backend,const std::uint32_t& mode_flags,
        Actor self,float field6a0,Contact& contact,std::uint32_t wheel) { //004D4B50,6AC.
    if(contact.surface==0x46||contact.surface==0x1f||contact.surface==0x20)return nullptr;
    if((mode_flags&1)&&field6a0!=0) { //004DDCF0 returns1 only when field6A0==0.
        if((contact.flags&0x40)&&(contact.flags8&0x40000)) {
            if(!contact.actor)throw std::runtime_error("Car item contact has no actual item actor");
            backend.car_item_contact(contact.actor,self,wheel); //005FEFB0, not a Door callback.
            return nullptr;
        }
        if((mode_flags&7)==7&&(contact.surface==0x44||contact.surface==0x45||contact.surface==0x62)) {
            const auto driver=backend.car_driver_actor(self);
            const auto& position=backend.car_wheel_world_position(self,wheel); //Wheel+50, borrowed live value.
            backend.car_collision_blast(driver,position,3.f,5000.f,3);
            constexpr std::array<std::uint32_t,3> effects{{0x2b,0x30,0x31}};
            const auto effect=effects[backend.car_random_word()%3];
            backend.car_surface_effect(position,CarContactVector{{0,1,0,0}},effect,contact.surface,3);
        }
    }
    return &contact;
}

struct CarSeatContactCooldown {
    std::int32_t actor_id;
    std::uint32_t timestamp;
};
struct CarHumanDamageEvent { //004D43DC, native values for the actual network producer.
    std::uint32_t kind=9;
    std::int32_t target;
    std::uint32_t amount=1000;
    CarContactVector impulse,point;
};
inline std::int32_t car_contact_signed_word(std::uint32_t bits) noexcept {
    std::int32_t result;std::memcpy(&result,&bits,sizeof(result));return result;
}

template<class Backend,class Actor,class Contact>
void car_actor_contact(Backend& backend,const std::uint32_t& mode_flags,Actor self,float speed958,
        const CarContactVector& position4a0,std::size_t seat_count,const std::vector<CarSeatContactCooldown>& cooldowns,
        const Contact& contact,const CarContactVector& point,const CarContactVector& impulse,
        const CarContactVector& normal,float mass) { //004D4230,680; actual four stack arguments.
    if(!(contact.flags&0x40)||!contact.frame)return;
    const auto target=backend.car_actor_from_frame(contact.frame); //Frame userdata word0, not descriptor+48.
    if(!target)return;
    const auto type=backend.actor_type(target);
    if(type==4) {
        backend.car_actor_hit58(target,self,point,impulse,normal,mass,decltype(contact.frame){});
        return;
    }
    if(type!=3) {
        const auto driver=backend.car_driver_actor(self);
        backend.car_actor_hit5c(target,self,point,position4a0,normal,mass,decltype(contact.frame){},driver);
        return;
    }
    //The Human victim is independently captured from descriptor+48. These
    //owners must not be merged merely because ordinary contacts match them.
    const auto victim=contact.actor;
    if((mode_flags&7)!=7) {
        if(cooldowns.size()<seat_count)throw std::runtime_error("Car seat contact cooldown owners are incomplete");
        for(std::size_t seat=0;seat!=seat_count;++seat)if(cooldowns[seat].actor_id!=-1) {
            const auto now=backend.car_clock(); //00748960, fresh query for each populated slot.
            if(car_contact_signed_word(cooldowns[seat].timestamp+3000u)>car_contact_signed_word(now))return;
        }
    }
    const auto driver=backend.car_driver_actor(self);
    backend.car_actor_hit5c(target,self,point,position4a0,normal,mass,decltype(contact.frame){},driver);
    if(!(speed958<4.f)) { //Original x87 branch also enters for unordered speed.
        if(!victim)throw std::runtime_error("Car Human contact has no actual victim owner");
        if(backend.actor_health(victim)>0) {
            if(mode_flags&1) {
                const auto occupant=backend.car_occupant(self,0); //Actual virtual+78.
                backend.car_human_impact(victim,5,1000.f,point,impulse,occupant,0); //Victim+D4.
            }else {
                CarHumanDamageEvent event;
                event.target=backend.actor_id(victim);event.impulse=impulse;event.point=point;
                backend.submit_car_human_damage(self,event); //004DB2E0, actual network queue required.
            }
        }
    }
}

//004D4740, callback690: EDX carries the impulse. Of seven stack arguments,
//only hit point (second), collision flag (fifth) and contact (sixth) are read.
template<class Backend,class Actor,class Contact>
void car_passenger_contact(Backend& backend,const std::uint32_t& mode_flags,Actor self,
        float mass9a8,const CarContactVector& axis2f0,bool field2319,std::size_t seat_count,
        const CarContactVector& impulse,const CarContactVector& point,bool collision,
        const Contact* contact) {
    if((mode_flags&7)!=7) {
        bool controlled=false;
        if(mode_flags&2) {
            if(backend.car_occupant(self,0)) {
                const auto driver=backend.car_occupant(self,0);
                if(!driver)throw std::runtime_error("Car driver disappeared during control query");
                controlled=backend.car_human_controlled(driver); //005A0890.
            }
        }
        if(!controlled) {
            if(!(mode_flags&1))return;
            if(backend.car_occupant(self,0)) {
                if(!(mode_flags&1)||!backend.car_occupant(self,0))return;
                const auto driver=backend.car_occupant(self,0);
                if(!driver)throw std::runtime_error("Car driver disappeared during role query");
                if(backend.car_human_role(driver)==1)return; //Human+19C.
            }
        }
    }
    const auto sum=static_cast<float>(static_cast<long double>(impulse[2])*impulse[2]+
        static_cast<long double>(impulse[0])*impulse[0]+static_cast<long double>(impulse[1])*impulse[1]);
    std::uint32_t bits;std::memcpy(&bits,&sum,sizeof(bits));
    const auto length=car_contact_signed_word(bits)<0x322bcc77?0.L:
        std::abs(static_cast<long double>(sum)-1.f)<9.99999993922529e-9f?1.L:
        std::sqrt(static_cast<long double>(sum));
    const auto ratio=static_cast<float>(length/mass9a8);
    auto damage=static_cast<float>(static_cast<long double>(ratio)*(collision?3.L:5.L));
    const auto angle=backend.car_vector_angle(impulse,axis2f0); //Actual S_vector::AngleTo.
    if(angle>2.7f&&!collision) {
        if(!field2319)damage=1000.f;
        else damage*=15.f;
    }
    if(!(damage>20.f))return;
    //0069BFE0 is a genuine empty diagnostic. It has no runtime consumer.
    if(contact&&(contact->flags&0x40)&&contact->actor&&backend.actor_type(contact->actor)==3)return;
    if(!(mode_flags&1)) {
        if(!std::isfinite(damage)||static_cast<long double>(damage)>=2147483648.L||damage<-2147483648.L)
            throw std::runtime_error("Car passenger damage exceeds the native event range");
        CarHumanDamageEvent event;
        event.target=-1;event.amount=static_cast<std::uint32_t>(static_cast<std::int32_t>(damage));
        event.impulse=impulse;event.point=point;
        backend.submit_car_human_damage(self,event); //004DB2E0.
        return;
    }
    for(auto seat=seat_count;seat!=0;) {
        const auto victim=backend.car_occupant(self,--seat);
        if(victim&&backend.actor_health(victim)>0) {
            const auto driver=backend.car_occupant(self,0);
            backend.car_human_impact(victim,5,damage,point,impulse,driver,0); //005A1C90, RET18.
        }
    }
}

//Known values passed to005FDC50/005FDC00. Native owners deliberately expose
//defined fields instead of copying the originals' unwritten stack/padding bytes.
struct CarDetachedPartRequest {
    CarContactVector zero20{{0,0,0,0}},zero30{{0,0,0,0}},velocity;
    float mass,coefficient=.5f;
    std::uint32_t field38=0;
    std::uint8_t field40=0,field41=0;
};
struct CarDetachedWheelRequest {
    CarContactVector velocity;
    float field10,field14,mass=10,field1c,field20;
    std::uint32_t field24=0;
    std::uint8_t field28=0;
};
struct CarWheelDamageEvent {
    std::uint32_t kind=2,wheel;
    std::int32_t value;
    CarContactVector point;
};

template<class Backend,class Frame,class Request>
bool car_launch_contact_item(Backend& backend,Frame frame,const Request& request,unsigned mode) { //004D5140.
    if(!frame)throw std::runtime_error("Car detached item requires an actual frame");
    if(backend.frame_kind(frame)!=1)return true;
    const auto matrix=backend.car_world_matrix(frame);
    CarContactVector direction{{matrix[8],matrix[9],matrix[10],matrix[11]}};
    const auto sum=static_cast<long double>(direction[1])*direction[1]+
        static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[2])*direction[2];
    const auto stored=static_cast<float>(sum);
    if(!std::isfinite(stored))throw std::runtime_error("Non-finite Car detached item direction");
    if(std::abs(sum-1.f)>=9.99999993922529e-9f) {
        if(stored>=9.99999993922529e-9f) {
            const auto scale=1.L/std::sqrt(static_cast<long double>(stored));
            for(unsigned i=0;i!=3;++i)direction[i]=static_cast<float>(direction[i]*scale);
        }else {
            const auto axis=direction[0]!=0?0u:direction[2]!=0?2u:1u;
            direction[axis]=direction[axis]<0?-1.f:1.f;
        }
    }
    //Original checks world validity again before borrowing position+70.
    const auto current=backend.car_world_matrix(frame);
    const CarContactVector position{{current[12],current[13],current[14],current[15]}};
    const auto item=backend.clone_car_contact_item(frame,position,direction); //00479450→runtime004770B0 type12.
    if(mode>1)return false;
    if(!item)throw std::runtime_error("Car detached item construction failed");
    backend.launch_car_contact_item(item,request,mode); //005FDC00 /005FDC50(request,1).
    return true;
}

template<class Backend,class Parts,class Links>
bool car_detach_contact_part(Backend& backend,Parts& parts,Links& links,
        std::size_t index,const CarContactVector& velocity,const CarContactVector* impulse) { //004D4CA0,694.
    auto& part=parts.at(index);
    float mass;
    switch(part.type) {
    case 2:mass=10;break;
    case 4:case 7:mass=2;break;
    case 5:mass=4;break;
    case 6:mass=7;break;
    case 8:mass=20;break;
    default:return false;
    }
    CarDetachedPartRequest request;request.velocity=velocity;request.mass=mass;
    if(impulse) {
        const auto reciprocal=1.L/mass;
        request.velocity[0]=static_cast<float>(request.velocity[0]+reciprocal*(*impulse)[0]);
        for(unsigned i=1;i!=3;++i)request.velocity[i]+=static_cast<float>(reciprocal*(*impulse)[i]);
    }
    if(!car_launch_contact_item(backend,part.frame,request,1))return false;
    for(auto link=links.rbegin();link!=links.rend();++link) {
        if((link->flags10&1)||link->part1a==0xffff)continue;
        const auto& child=parts.at(link->part1a);
        if(!(child.flags&1)&&child.frame&&backend.object_parent(child.frame)==part.frame) {
            link->field14=0;link->flags10|=1;break;
        }
    }
    part.flags|=1;
    backend.object_visible(part.frame,false);backend.update_scene_object(part.frame);
    return true;
}

inline std::int32_t car_contact_integer(float value) {
    if(!std::isfinite(value)||static_cast<long double>(value)>=2147483648.L||value<-2147483648.L)
        throw std::runtime_error("Car contact value exceeds native signed integer range");
    return static_cast<std::int32_t>(value);
}

template<class Backend,class Actor,class Wheels>
bool car_detach_contact_wheel(Backend& backend,const std::uint32_t& mode_flags,const std::int32_t& local_actor_id,
        Actor self,Wheels& wheels,std::size_t index,const CarContactVector& velocity,
        const CarContactVector* impulse) { //004D4EB0,698.
    auto& wheel=wheels.at(index);
    const bool create_item=(mode_flags&2)!=0;
    if(create_item) {
        CarDetachedWheelRequest request;
        request.velocity=velocity;request.field10=wheel.scalars.get(0x1d8);request.field14=wheel.scalars.get(0x214);
        request.field1c=wheel.scalars.get(0x1b0);request.field20=wheel.scalars.get(0x1c0);
        if(impulse) {
            for(unsigned i=0;i!=2;++i)request.velocity[i]=static_cast<float>(request.velocity[i]+
                static_cast<long double>((*impulse)[i])*.1f);
            request.velocity[2]+=static_cast<float>(static_cast<long double>((*impulse)[2])*.1f);
        }
        if(!car_launch_contact_item(backend,wheel.frame,request,0))return false;
    }
    wheel.flags|=0x40000000;
    backend.object_visible(wheel.frame,false);backend.update_scene_object(wheel.frame);
    if(!create_item||(mode_flags&1)) {
        const CarWheelDamageEvent event{2,static_cast<std::uint32_t>(index),car_contact_integer(wheel.scalars.get(0x230)),velocity};
        backend.apply_car_wheel_damage(self,event); //004DB2B0.
    }else if(backend.car_occupant(self,0)) {
        const auto driver=backend.car_occupant(self,0);
        if(!driver)throw std::runtime_error("Car driver disappeared during wheel damage query");
        if(backend.actor_id(driver)==local_actor_id) {
            const CarWheelDamageEvent event{2,static_cast<std::uint32_t>(index),car_contact_integer(wheel.scalars.get(0x230)),velocity};
            backend.submit_car_wheel_damage(self,event); //004DB2E0.
        }
    }
    return true;
}

} //namespace hd2::game
#endif
