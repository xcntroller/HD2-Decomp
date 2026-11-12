#ifndef HD2_GAME_CAR_SCENE_ACTOR_H
#define HD2_GAME_CAR_SCENE_ACTOR_H

#include "game/scene_actor.h"
#include "game/car_geometry.h"
#include "game/car_ancillary_model.h"
#include "game/car_callback_services.h"
#include "game/car_loading_lifetime.h"
#include "game/mounted_weapon_control.h"
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
        std::function<Contact*(Contact&)> contact6a8;
        std::function<Contact*(Contact&,unsigned)> surface6ac;
        std::function<void()> empty6b0,empty6b4,state6a4; //Actual core6A4/actor734 is cold NULL.
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
        loading.reset(); //Actual runtime borrowers precede DM/geometry/frame retirement.
        ancillary.reset(); //004E0EE0 precedes registry removal.
        if(registered_)this->backend_.unregister_car_actor(this);
        lists259c.clear();lists258c.clear();
        lists2538.clear();lists2528.clear();for(auto& list:lists2518)list.clear();
        callbacks.reset();
        geometry.reset(); //004E7F10 before00610170's base storage/frame/reference cleanup.
    }
    bool initialize(Frame frame) override { //004D10A0.
        flag24a1=false;for(auto& control:fields24b4)control.reset();
        if(!this->base_.initialize_frame(frame))return false;
        if(geometry)throw std::logic_error("Car geometry initialization repeated");
        geometry.emplace(this->backend_,frame,services_.car_tyre_ids());
        occupants.assign(geometry->auxiliary().seats().size(),nullptr); //004E2E30's actual FA8+30 zeroed seats.
        bind_callbacks();
        ancillary.emplace();
        ancillary->bind_particles(services_);
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
    std::unique_ptr<CarLoadingLifetime<Backend>> loading;
    std::vector<SceneActor<Backend>*> occupants;
    SceneActor<Backend>* driver24ac=nullptr; //004D0B60, separate from seat0's current occupant.
    bool driver_enabled2321=false; //004D11C0 publishes with driver24AC.
    std::array<bool,4> input_flags2329{}; //Reached004D9430 publications.
    std::uint32_t control_mask24a8=0; //004D16A0 produces before004D9280 consumes.
    Services& callback_services(){return services_;}
    void restart_clock(){clock2470=services_.car_clock();fields2474[1]=1;} //0073EAC0.
    bool clock_expired(){ //0073EA90 on the actual0073EA50 owner at actor2470.
        if(!fields2474[1])return false;
        if(car_contact_signed_word(clock2470+fields2474[0])>=car_contact_signed_word(services_.car_clock()))return false;
        clock2470=services_.car_clock();return true;
    }
    //Actual runtime producers must fill these owners before invoking callbacks;
    //cold loading alone does not initialize a current simulation state.
    std::vector<CarSeatContactCooldown> cooldowns;
    std::array<std::unique_ptr<CarMountedWeaponControl>,10> fields24b4{};
    std::array<std::uint32_t,2> fields2474{};
    std::uint32_t clock2470,field24dc=0xffffffffu;
    using DamageModelOwner=decltype(std::declval<Backend&>().retain_car_frame(std::declval<Frame>()));
    std::optional<DamageModelOwner> model254c; //Native owned reference, initialized NULL by004D16A0.
    bool flag2548=false;
    bool flag24a1=false,flag254a=false; //254A reset by actual004D16A0; set by004DBF00.
    std::optional<bool> flag2544; //Actual004DB760 case8 publication; no cold producer is invented.
    std::optional<std::int32_t> source24b0; //Native24B0 is produced by authority activation or actualD4.
    std::optional<std::int32_t> damage2550,source2554; //Produced by004D16A0/004DD0D0, armed by004DEAD0.
    Vector target2560{},target2570{}; //004DEB50 publishes before template/factory.
    bool flag2549=false; //004D16A0/004DD0D0; non7 water request latch.
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
            return car_detach_contact_part(s,geometry->deformables().parts(),geometry->lighting().lights(),index,velocity,impulse);
        };
        c.wheel698=[this,&s](std::size_t index,const Vector& velocity,const Vector* impulse) {
            return car_detach_contact_wheel(s,s.car_mode_flags(),s.car_local_actor_id(),
                static_cast<SceneActor<Backend>*>(this),s.car_wheel_contact_states(this),index,velocity,impulse);
        };
        c.hide69c=[this,&s](std::size_t index){car_hide_contact_part(s,geometry->deformables().parts().at(index));};
        c.contact6a8=[this,&s](Contact& contact){return car_physics_contact_filter(s,contact,s.car_field5e0(this),s.car_position4a0(this));};
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
