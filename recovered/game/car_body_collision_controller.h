#ifndef HD2_GAME_CAR_BODY_COLLISION_CONTROLLER_H
#define HD2_GAME_CAR_BODY_COLLISION_CONTROLLER_H

#include "game/car_body_contacts.h"
#include "game/car_collision_loading.h"
#include "game/car_substeps.h"

namespace hd2::game {

//0050DE60's10/400 gates,00514D70 call and native pose rollback/retry. Owners
//and callbacks are borrowed from the containing Car/world for the entire tick.
//No readiness is implied by constructing a controller.
template<class Frames,class Collisions,class Queries,class Callbacks,class Receivers>
class CarBodyCollisionController {
public:
    using Body=CarBodyIntegration<Frames>;using Vector=typename Frames::Vector;
    using Object=typename CarCollisionLoading<Frames,Collisions>::Descriptor;
    using Candidate=typename Body::Candidate;
    CarBodyCollisionController(Frames& frames,typename Frames::Frame model,const CarCollisionParts<Frames>& parts,
            CarCollisionLoading<Frames,Collisions>& collisions,Queries& queries,
            DynamicCollisionExclusions<Object>& exclusions,const CarMaterialRegistry& materials,
            const CarPhysicsScalars& coefficients,bool override_material,
            Callbacks& callbacks,Receivers& receivers,std::uint32_t layers,std::uint32_t flags)
        :frames_(frames),model_(model),parts_(parts),collisions_(collisions),queries_(queries),exclusions_(exclusions),
         materials_(materials),callbacks_(callbacks),receivers_(receivers),layers_(layers),flags_(flags){
        state_.override_material=override_material; //Actual applied228C, not a constructor guess.
        state_.friction=coefficients.get(0x6f4);state_.restitution=coefficients.get(0x6f8);
    }
    bool operator()(Body& body,CarSubstepState<Frames>& substeps,const Candidate& candidate,
            float elapsed,float speed,bool mass_changed,bool audible,bool gear,std::uint32_t& body_flags){
        const bool previous=contact_;contact_=false; //2288 is cleared even on the nonfinal !400 substep.
        if(!(substeps.flags&0x10u))return state_.moving_primitive;
        const auto& pose=body.pose();Candidate base{pose.position,pose.forward,pose.up,pose.right};
        if(!(substeps.flags&0x400u)){
            if(substeps.index!=substeps.count)return state_.moving_primitive;
            if(!substeps.previous_world||!substeps.previous_inverse||!substeps.previous_pose)
                throw std::logic_error("Car BODYCOL final substep lost its actual tick snapshot");
            base=*substeps.previous_pose;
        }else{substeps.previous_world=pose.world;substeps.previous_inverse=pose.inverse;}
        const auto square=[](const Vector& v){return (static_cast<long double>(v[2])*v[2]+
            static_cast<long double>(v[1])*v[1])+static_cast<long double>(v[0])*v[0];};
        if(.009999999776482582f<square(body.motion().velocity)||.0024999999441206455f<square(body.motion().angular_velocity)||mass_changed||previous){
            state_.speed=speed;
            state_.correction={};state_.other_correction={};state_.maximum_magnitude=0;state_.maximum_impulse={};state_.records.clear();
            auto motion=body.contact_state();const auto excluded=collisions_.query_exclusions();
            const auto milliseconds=static_cast<long double>(elapsed)*1000.f;
            if(!std::isfinite(milliseconds)||milliseconds<0||milliseconds>std::numeric_limits<std::int32_t>::max())
                throw std::runtime_error("Car BODYCOL interval exceeds native integer range");
            contact_=car_body_contacts(frames_,model_,parts_.roots(),motion,state_,materials_,queries_,exclusions_,excluded,
                callbacks_,receivers_,*substeps.previous_world,pose.inverse,base.up,candidate.up,static_cast<std::uint32_t>(milliseconds),
                layers_,flags_,substeps.flags,audible,gear);
            body.motion().velocity=motion.linear410;body.motion().angular_velocity=motion.angular210;
        }
        if(!contact_)return state_.moving_primitive;
        body.publish(base);
        auto retry=body.advance_pose(elapsed,base,*substeps.previous_inverse,pose.centre);
        //A first contact changes motion but does not apply positional correction.
        if(previous){const auto first=square(state_.correction);
            if(.0024999999441206455f<first){for(unsigned axis=0;axis!=3;++axis)retry.position[axis]+=state_.correction[axis];}
            else if(.0024999999441206455f<square(state_.other_correction))
                for(unsigned axis=0;axis!=3;++axis)retry.position[axis]+=state_.other_correction[axis];
        }
        body.publish(retry);
        if(square(body.motion().velocity)<.03999999910593033f&&square(body.motion().angular_velocity)<.009999999776482582f){
            body.motion().velocity={};body.motion().angular_velocity={};
            if(!(body_flags&0x20000000u))body_flags|=2u;
        }
        return state_.moving_primitive;
    }
    bool contact()const{return contact_;}
    void query_flags(std::uint32_t flags){flags_=flags;} //SAME core540 modified by occupied driver admission.
    CarBodyContactState& state(){return state_;}const CarBodyContactState& state()const{return state_;}
private:
    Frames& frames_;typename Frames::Frame model_;const CarCollisionParts<Frames>& parts_;
    CarCollisionLoading<Frames,Collisions>& collisions_;Queries& queries_;DynamicCollisionExclusions<Object>& exclusions_;
    const CarMaterialRegistry& materials_;Callbacks& callbacks_;Receivers& receivers_;
    std::uint32_t layers_,flags_;bool contact_=true; //004EA780 sets2288=1.
    CarBodyContactState state_;
};
} //namespace hd2::game
#endif
