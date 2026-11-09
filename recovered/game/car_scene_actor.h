#ifndef HD2_GAME_CAR_SCENE_ACTOR_H
#define HD2_GAME_CAR_SCENE_ACTOR_H

#include "game/scene_actor.h"
#include "game/car_geometry.h"
#include "game/car_ancillary_model.h"
#include "game/car_callback_services.h"
#include <functional>

namespace hd2::game {

//004D0B60/004D10A0/004D55C0, native cold actor with actual geometry/property
//owners and callable collision slots. Live simulation matrices/velocities and
//module receivers remain owned by the mandatory containing runtime.
template<class Backend>
class CarSceneActor final : public SceneActor<Backend> {
public:
    using Frame=typename Backend::Frame;
    using Services=CarCallbackServices<Backend>;
    using Contact=CarContact<SceneActor<Backend>*,Frame>;
    using Vector=CarContactVector;
    struct Callbacks {
        std::function<void(const Contact&,const Vector&,const Vector&,const Vector&,float)> actor680;
        std::function<void(const Contact&,const Vector&,const Vector&,const Vector&)> human684;
        std::function<bool(const Contact&,const Vector&,const Vector&,CarContactResponse&)> response688;
        std::function<bool(const Contact&,bool&)> filter68c;
        std::function<void(const Vector&,const Vector&,bool,const Contact*)> passenger690;
        std::function<bool(std::size_t,const Vector&,const Vector*)> part694,wheel698;
        std::function<void(std::size_t)> hide69c;
        std::function<Contact*(Contact&)> script6a8;
        std::function<Contact*(Contact&,unsigned)> surface6ac;
        std::function<void()> empty6b0,empty6b4;
        std::function<bool(CarParticleState<Frame>&,unsigned)> particle6bc;
        std::function<std::int32_t(unsigned,unsigned)> reuse6c0;
        //6A0/6B8 remain unassigned cold null;6A4 is explicitly null.
    };
    CarSceneActor(Backend& backend,Services& services,std::string localized_name)
        : SceneActor<Backend>(backend,4),clock2470(services.car_clock()),services_(services) {
        this->base_.name(std::move(localized_name));
        backend.register_car_actor(this); //0089BC28 list, after cold members/name.
        registered_=true;
    }
    ~CarSceneActor() override { //004D0F20; no populated-producer cleanup is invented.
        ancillary.reset(); //004E0EE0 precedes registry removal.
        if(registered_)this->backend_.unregister_car_actor(this);
        lists259c.clear();lists258c.clear();
        lists2538.clear();lists2528.clear();for(auto& list:lists2518)list.clear();
        callbacks.reset();
        geometry.reset(); //004E7F10 before00610170's base storage/frame/reference cleanup.
    }
    bool initialize(Frame frame) override { //004D10A0.
        flag24a1=false;fields24b4.fill(0);
        if(!this->base_.initialize_frame(frame))return false;
        if(geometry)throw std::logic_error("Car geometry initialization repeated");
        geometry.emplace(this->backend_,frame,services_.car_tyre_ids());
        bind_callbacks();
        ancillary.emplace();
        ancillary->load(this->backend_,geometry->resource(),geometry->auxiliary().seats().size());
        return true;
    }
    void load(SerializedBytes& bytes) override { //004D55C0, final004ED270 result ignored by original.
        if(!geometry)throw std::logic_error("Car properties require initialized geometry");
        const auto parameters=read_car_parameters(bytes);
        if(!geometry->physics().apply(parameters,services_.car_tyre_ids()))
            throw std::runtime_error("Car actor physics properties rejected");
    }
    std::optional<CarGeometry<Backend>> geometry;
    std::optional<CarAncillaryModel<Backend>> ancillary;
    std::optional<Callbacks> callbacks;
    //Actual runtime producers must fill these owners before invoking callbacks;
    //cold loading alone does not initialize a current simulation state.
    std::vector<CarSeatContactCooldown> cooldowns;
    std::vector<CarContactLink> contact_links;
    std::array<std::uint32_t,10> fields24b4{};
    std::array<std::uint32_t,2> fields2474{};
    std::uint32_t clock2470,field24dc=0xffffffffu;
    bool flag24a1=false;
    std::array<std::vector<std::uint32_t>,2> lists2518;
    std::vector<std::uint32_t> lists2528,lists2538,lists258c,lists259c;
private:
    Services& services_;
    bool registered_=false;
    void bind_callbacks() {
        auto& s=services_;
        callbacks.emplace();auto& c=*callbacks;
        c.actor680=[this,&s](const Contact& contact,const Vector& point,const Vector& impulse,const Vector& normal,float mass) {
            car_actor_contact(s,s.car_mode_flags(),static_cast<SceneActor<Backend>*>(this),
                s.car_speed958(this),s.car_position4a0(this),geometry->auxiliary().seats().size(),cooldowns,
                contact,point,impulse,normal,mass);
        };
        c.human684=[this,&s](const Contact& contact,const Vector& point,const Vector& direction,const Vector& normal) {
            car_human_contact(s,static_cast<SceneActor<Backend>*>(this),s.car_speed958(this),s.car_axis2d0(this),
                s.car_contact_state(this).mass918,contact,point,direction,normal);
        };
        c.response688=[&s](const Contact& contact,const Vector& point,const Vector& normal,CarContactResponse& result) {
            return car_contact_response(s,contact,point,normal,result);
        };
        c.filter68c=[this,&s](const Contact& contact,bool& unused) {
            return car_contact_filter(s,static_cast<SceneActor<Backend>*>(this),s.car_speed958(this),contact,unused);
        };
        c.passenger690=[this,&s](const Vector& impulse,const Vector& point,bool collision,const Contact* contact) {
            car_passenger_contact(s,s.car_mode_flags(),static_cast<SceneActor<Backend>*>(this),
                s.car_contact_state(this).mass918,s.car_axis2f0(this),s.car_flag2319(this),
                geometry->auxiliary().seats().size(),impulse,point,collision,contact);
        };
        c.part694=[this,&s](std::size_t index,const Vector& velocity,const Vector* impulse) {
            return car_detach_contact_part(s,geometry->deformables().parts(),contact_links,index,velocity,impulse);
        };
        c.wheel698=[this,&s](std::size_t index,const Vector& velocity,const Vector* impulse) {
            return car_detach_contact_wheel(s,s.car_mode_flags(),s.car_local_actor_id(),
                static_cast<SceneActor<Backend>*>(this),s.car_wheel_contact_states(this),index,velocity,impulse);
        };
        c.hide69c=[this,&s](std::size_t index){car_hide_contact_part(s,geometry->deformables().parts().at(index));};
        c.script6a8=[this,&s](Contact& contact){return car_script_contact_filter(s,contact,s.car_field5e0(this),s.car_position4a0(this));};
        c.surface6ac=[this,&s](Contact& contact,unsigned wheel) {
            return car_surface_contact_filter(s,s.car_mode_flags(),static_cast<SceneActor<Backend>*>(this),s.car_field6a0(this),contact,wheel);
        };
        c.empty6b0=[] {car_empty_contact_callback();};c.empty6b4=[] {car_empty_part_callback();};
        c.particle6bc=[&s](CarParticleState<Frame>& state,unsigned index) {
            return initialize_car_particle(s,state,s.car_particle_definitions(),index);
        };
        c.reuse6c0=[&s](unsigned slot,unsigned index) {
            return reuse_car_particle(s,s.car_particle_pool(),s.car_particle_definitions(),slot,index);
        };
    }
};

} //namespace hd2::game
#endif
