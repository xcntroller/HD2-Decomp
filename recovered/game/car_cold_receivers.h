#ifndef HD2_GAME_CAR_COLD_RECEIVERS_H
#define HD2_GAME_CAR_COLD_RECEIVERS_H

#include "game/car_cold_deformation.h"
#include "game/car_cold_initialization.h"
#include "game/car_collision_queries.h"
#include "game/car_scene_actor.h"
#include "game/car_deform_damage.h"

namespace hd2::game {

//Real callbacks and source notification receivers for the unoccupied cold
//solver. All mutations target the SAME initializer, geometry and actor slots.
//The shared process random stream is also used by road displacements.
template<class Frames,class Collisions,class Random>class CarColdReceivers {
public:
    using Actor=CarSceneActor<Frames>;using Vector=typename Frames::Vector;
    using Hit=CarCollisionHit<typename Frames::Frame,SceneActor<Frames>*>;
    using Initial=CarColdInitialization<Frames,Collisions>;
    CarColdReceivers(Frames& frames,Actor& actor,Initial& state,Random& random)
        :frames_(frames),actor_(actor),state_(state),random_(random),deformation_(frames,actor.geometry.value(),state.body(),state.wheels,
            state.solver.substeps.flags,state.finalizer.dirty_flags,random,actor.callbacks.value().empty6b0,
            [this](std::size_t index,float impulse){damage_car_deformed_light(frames_,*actor_.geometry,index,impulse,*this);},
            [this](std::size_t index,float absorbed,const Vector& direction,const Vector* impulse){
                damage_car_deformed_part(frames_,*actor_.geometry,state_.body(),state_.finalizer.dirty_flags,index,absorbed,direction,impulse,*this);},
            [this](std::size_t index,float impulse){damage_car_deformed_wheel(frames_,state_.body(),state_.wheels,state_.finalizer.dirty_flags,index,impulse,*this);}){
        if(!actor_.callbacks)throw std::logic_error("Car runtime requires its actual installed callbacks");
    }
    Hit* surface(Hit& hit,unsigned index){ //004D4B50/6AC preserves or rejects this SAME contact.
        auto& callback=actor_.callbacks->surface6ac;if(!callback)return &hit;
        const auto result=callback(hit.contact,index);if(!result)return nullptr;
        if(result!=&hit.contact)throw std::runtime_error("Car surface callback replaced the actual query contact");
        return &hit;
    }
    void deform(unsigned,const Vector& point,const Vector& normal,float radius,float impulse){deformation_(point,normal,radius,impulse);}
    void deform(const Vector& point,const Vector& normal,float radius,float impulse,unsigned flags=0,const Vector* incoming=nullptr){deformation_(point,normal,radius,impulse,flags,incoming);}
    void variant_glass(std::size_t index,const Vector& direction){
        if(!actor_.loading)throw std::logic_error("Car deformation glass requires its actual loading owner");actor_.loading->glass_damage(index,direction);
    }
    template<class Light>void variant_light(const Light& light){
        if(!(light.flags10&0x80000000u))return;
        const auto mask=frames_.car_position(light.frame)[0]>=0?2u:1u;
        damage_car_paired_light(frames_,actor_.geometry->lighting(),state_.paired_flags,light.projector,mask);
    }
    void variant_detach(std::size_t index,const Vector& velocity,const Vector* impulse){
        const auto& callback=actor_.callbacks->part694;if(!callback)throw std::logic_error("Car deformation lost actual694 callback");(void)callback(index,velocity,impulse);
    }
    void variant_wheel(std::size_t index,const Vector& velocity){
        const auto& callback=actor_.callbacks->wheel698;if(!callback)throw std::logic_error("Car deformation lost actual698 callback");(void)callback(index,velocity,nullptr);
    }
    void damage(float impulse){damage_car_health(actor_.geometry->physics(),state_.drive,state_.health,state_.finalizer.dirty_flags,impulse);}
    void passengers(const CarSuspensionImpact& impact,const Vector& point){ //00512390's fifth stack argument is0, unlike BODYCOL.
        if(auto& callback=actor_.callbacks->passenger690)callback(impact.impulse,point,false,nullptr);
    }
    void sound(float volume){state_.audio.suspension(volume,state_.drive.audible,state_.finalizer.inside);}
    void sound(float impulse,bool gear){state_.audio.collision(actor_.geometry->physics(),impulse,gear,state_.finalizer.inside,random_);}
    void clutch_changed(){throw std::runtime_error("Car gear change requires its actual clutch sound transition");}
private:
    Frames& frames_;Actor& actor_;Initial& state_;Random& random_;CarColdDeformation<Frames> deformation_;
};

} //namespace hd2::game
#endif
