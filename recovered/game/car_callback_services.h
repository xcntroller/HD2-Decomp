#ifndef HD2_GAME_CAR_CALLBACK_SERVICES_H
#define HD2_GAME_CAR_CALLBACK_SERVICES_H

#include "game/scene_actor.h"
#include "game/car_particles.h"
#include "game/car_wheel_runtime.h"
#include "game/car_tyres.h"
#include "game/session_packet_stream.h"
#include <memory>

namespace hd2::game {

//Actual collision/module receivers required by the installed Car callbacks.
//The containing runtime implements these against its native actor/item/script/
//network/effect owners. No default successful operations or original code calls.
template<class Backend>
class CarCallbackServices {
public:
    using Frame=typename Backend::Frame;
    using Actor=SceneActor<Backend>*;
    using Vector=CarContactVector;
    using Matrix=CarContactMatrix;
    struct Texture {virtual ~Texture()=default;virtual Frame get() const=0;};
    class TextureOwner {
    public:
        explicit TextureOwner(std::unique_ptr<Texture> value={}) : value_(std::move(value)) {}
        Frame get() const {return value_?value_->get():Frame{};}
        explicit operator bool() const {return get()!=Frame{};}
    private:
        std::unique_ptr<Texture> value_;
    };
    virtual ~CarCallbackServices()=default;
    virtual const std::uint32_t& car_mode_flags() const=0;
    virtual const std::int32_t& car_local_actor_id() const=0;
    virtual const CarTyres& car_tyres() const=0;
    std::vector<std::int32_t> car_tyre_ids()const{return car_tyres().material_ids();}
    virtual std::uint32_t car_clock()=0;
    virtual void receive_damage_packet(Actor,SessionPacketStream&)=0;
    virtual void car_retire_particle(std::int32_t)=0;
    virtual std::size_t car_active_particles()const=0;
    virtual Actor car_actor_from_frame(Frame)=0;
    virtual std::uint32_t actor_type(Actor)=0;
    virtual std::int32_t actor_id(Actor)=0;
    virtual std::int32_t actor_health(Actor)=0;
    virtual Vector actor_world_position(Actor)=0;
    virtual const CarContactState& car_contact_state(Actor)=0;
    virtual float car_speed958(Actor)=0;
    virtual Vector car_position4a0(Actor)=0;
    virtual Vector car_axis2d0(Actor)=0;
    virtual Vector car_axis2f0(Actor)=0;
    virtual bool car_flag2319(Actor)=0;
    virtual bool car_field5e0(Actor)=0;
    virtual float car_field6a0(Actor)=0;
    using WheelContactState=CarWheelRuntime<Backend>; //Borrow the SAME runtime, including mutable flags/health.
    virtual std::vector<WheelContactState>& car_wheel_contact_states(Actor)=0;
    virtual Actor car_driver_actor(Actor)=0;
    virtual Actor car_occupant(Actor,std::size_t)=0;
    virtual bool car_human_controlled(Actor)=0;
    virtual std::uint32_t car_human_role(Actor)=0;
    virtual void car_actor_hit58(Actor,Actor,const Vector&,const Vector&,const Vector&,float,Frame)=0;
    virtual void car_actor_hit5c(Actor,Actor,const Vector&,const Vector&,const Vector&,float,Frame,Actor)=0;
    virtual void human_car_contact(Actor,Actor,const Vector&,const Vector&,const Vector&,float,Frame)=0;
    virtual void human_ragdoll_car_impulse(Actor,const Vector&,const Vector&)=0;
    virtual void car_human_impact(Actor,int,float,const Vector&,const Vector&,Actor,int)=0;
    virtual void submit_car_human_damage(Actor,const CarHumanDamageEvent&)=0;
    virtual void apply_car_wheel_damage(Actor,const CarWheelDamageEvent&)=0;
    virtual void submit_car_wheel_damage(Actor,const CarWheelDamageEvent&)=0;
    virtual void car_physics_contact(std::uintptr_t,unsigned,const Vector&,const Vector&)=0;
    virtual void car_timed_contact(std::uintptr_t,unsigned,unsigned)=0;
    virtual void car_item_contact(Actor,Actor,unsigned)=0;
    virtual const Vector& car_wheel_world_position(Actor,unsigned)=0;
    virtual void car_collision_blast(Actor,const Vector&,float,float,unsigned)=0;
    virtual std::uint32_t car_random_word()=0;
    virtual void car_surface_effect(const Vector&,const Vector&,unsigned,unsigned,unsigned)=0;
    virtual void emit_car_contact_fragments(Frame,float)=0;
    virtual Actor clone_car_contact_item(Frame,const Vector&,const Vector&)=0;
    virtual void launch_car_contact_item(Actor,const CarDetachedPartRequest&,unsigned)=0;
    virtual void launch_car_contact_item(Actor,const CarDetachedWheelRequest&,unsigned)=0;
    virtual void car_transform_in_place(Vector&,const Matrix&)=0;
    virtual float car_vector_angle(const Vector&,const Vector&)=0;
    virtual bool car_frame_visible(Frame)=0;
    virtual std::uint32_t frame_kind(Frame)=0;
    virtual Frame object_parent(Frame)=0;
    virtual void object_visible(Frame,bool)=0;
    virtual void update_scene_object(Frame)=0;
    virtual void link_scene_object(Frame,Frame)=0;
    virtual Matrix car_world_matrix(Frame)=0;
    virtual const ActorFrameData* find_actor_frame_data(Frame)=0;
    virtual TextureOwner load_car_particle_texture(const std::string&,const std::string&,unsigned)=0;
    virtual void car_particle_texture(Frame,Frame)=0;
    virtual void car_particle_name(Frame,const char*)=0;
    virtual void car_particle_unit_scale(Frame)=0;
    virtual void car_direction(Frame,const Vector&)=0;
    virtual void reset_car_particle(Frame)=0;
    virtual void car_particle_property(Frame,unsigned,std::uint32_t)=0;
    virtual void car_particle_vector_property(Frame,unsigned,const Vector&)=0;
    virtual std::size_t car_particle_region_count(Frame)=0;
    virtual Vector car_particle_region(Frame,std::size_t)=0;
    virtual void select_car_particle_region(Frame,std::size_t)=0;
    virtual void clear_car_particle_region(Frame,std::size_t)=0;
    virtual void add_car_particle_region(Frame,const CarParticleRegion&,std::int32_t)=0;
    virtual std::string car_particle_texture_name(Frame)=0;
    virtual void activate_car_particle(Frame,bool)=0;
    virtual const std::vector<CarParticleDefinition>& car_particle_definitions() const=0;
    virtual std::vector<CarParticleState<Frame>>& car_particle_pool()=0;
};

} //namespace hd2::game
#endif
