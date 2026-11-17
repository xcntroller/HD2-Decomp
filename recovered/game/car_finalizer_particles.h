#ifndef HD2_GAME_CAR_FINALIZER_PARTICLES_H
#define HD2_GAME_CAR_FINALIZER_PARTICLES_H
#include "game/car_cold_particles.h"
#include "game/car_body_integration.h"
#include "game/mission_particle_publication.h"

namespace hd2::game {

//004ECE20 and00506F00/00508350/00508EE0/00509540. Borrow the actual
//0089BC40 pool,0097B89C definitions and installed6BC/6C0 callbacks.
template<class Frames,class Callbacks>class CarFinalizerParticles {
public:
    using Frame=typename Frames::Frame;using Vector=typename Frames::Vector;
    CarFinalizerParticles(Frames& frames,Callbacks& callbacks,const std::vector<CarParticleDefinition>& definitions,
            std::vector<CarParticleState<Frame>>& pool):frames_(frames),callbacks_(callbacks),definitions_(definitions),pool_(pool){}
    std::int32_t allocate(std::uint32_t definition,std::int32_t prior){ //004ECE20.
        if(callbacks_.reuse6c0&&prior>=0&&static_cast<std::size_t>(prior)<pool_.size()){
            const auto reused=callbacks_.reuse6c0(static_cast<unsigned>(prior),definition);if(reused>=0)return reused;
        }
        std::int32_t free=-1,matching=-1;
        for(auto n=pool_.size();n;){const auto index=--n;auto& state=pool_[index];
            if(matching==-1&&state.definition==definition)matching=static_cast<std::int32_t>(index);
            if(!state.active){if(free==-1)free=static_cast<std::int32_t>(index);
                if(state.definition==definition){state.active=true;frames_.activate_car_particle(state.visual,true);return static_cast<std::int32_t>(index);}}
        }
        if(free<0)return -1;
        auto& state=pool_.at(static_cast<std::size_t>(free));
        if(matching>=0){
            const auto& source=pool_.at(static_cast<std::size_t>(matching));frames_.copy_car_targeted_frame(state.visual,source.visual);
            state.definition=source.definition;state.vector0=source.vector0;state.vector10=source.vector10;
        }else if(!callbacks_.particle6bc||!callbacks_.particle6bc(state,definition))return -1;
        state.active=true;frames_.activate_car_particle(state.visual,true);return free;
    }
    template<class Geometry>bool damage(const Geometry& geometry,const CarBodyIntegration<Frames>& body,
            const CarHealthState& health,CarColdParticleSlots& slots,std::uint32_t features,bool live,float speed,float light){ //00506F00.
        const auto& fields=geometry.physics().scalars();
        if((features&0x200u)&&live&&health.body<fields.get(0x624)&&speed<fields.get(0x62c)){
            auto definition=word(fields.get(0x630));
            if(health.body<fields.get(0x628)){const auto second=word(fields.get(0x634));
                if(second&&second<pool_.size())definition=second;} //Native compares pool count, not table count.
            if(slots.damage<0)slots.damage=allocate(definition,slots.damage);
            //Native00506FB2 compares the SLOT INDEX to the requested ID.
            if(slots.damage>=0&&static_cast<std::uint32_t>(slots.damage)!=definition){retire_car_particle(pool_,slots.damage);slots.damage=allocate(definition,slots.damage);}
            if(slots.damage>=0){
                prepare(slots.damage,light);const auto point=frames_.car_transform(geometry.auxiliary().smoke(),body.pose().world);
                publish(slots.damage,point,velocity_at(body,point),.10000000149011612f);
            }
            return false;
        }
        retire(slots.damage);return true;
    }
    void wheels(const CarMaterialRegistry& materials,std::vector<CarWheelRuntime<Frames>>& wheels,
            std::uint32_t features,bool live,float speed,unsigned game_kind,float light){ //00508350.
        if(game_kind==8)return;
        for(auto n=wheels.size();n;){auto& wheel=wheels[--n];
            if(!(features&0x200u)||!live){retire(wheel.effect);continue;}
            const auto flags=wheel.flags;if(flags&0x400u)continue;
            if(!(flags&0x40000000u)&&(flags&8u)&&(flags&0x800u)){
                const auto& velocity=wheel.vectors.at(0xa0);const auto& normal=wheel.vectors.at(0x80);
                const auto square=(static_cast<long double>(velocity[2])*velocity[2]+static_cast<long double>(velocity[1])*velocity[1])+static_cast<long double>(velocity[0])*velocity[0];
                const bool slipping=(flags&0x80u)||((flags&0x1000020u)&&4.f<square);
                const auto& material=materials.responses().at(materials.indices().at(wheel.surface_id));
                const auto definition=slipping?material.words20[1]:(speed>3.f?material.words20[0]:0u);
                if(car_contact_signed_word(definition)>0){
                    select(wheel.effect,definition);
                    if(wheel.effect>=0){
                        prepare(wheel.effect,light);auto tangent=cross(normal,cross(velocity,normal));
                        if(!slipping)for(unsigned axis=0;axis<3;++axis)tangent[axis]*=.6499999761581421f;
                        publish(wheel.effect,wheel.vectors.at(0x50),tangent,slipping?.009999999776482582f:.0010000000474974513f);
                    }
                    continue;
                }
            }
            retire(wheel.effect);
        }
    }
    template<class Geometry>void exhaust(const Geometry& geometry,const CarBodyIntegration<Frames>& body,const CarDrivetrainState& drive,
            CarColdParticleSlots& slots,std::uint32_t features,bool live,bool transition,float speed,float light){ //00508EE0.
        const auto& physics=geometry.physics();const auto& fields=physics.scalars();
        if(!(features&0x200u)||!live||!transition||fields.get(0x758)<=speed||physics.field_754()<1){
            for(auto n=slots.exhausts.size();n;)retire(slots.exhausts[--n]);
            return;
        }
        const auto& exhausts=geometry.auxiliary().exhausts();
        if(exhausts.size()!=slots.exhausts.size())throw std::logic_error("Car exhaust slots do not match retained geometry");
        for(auto n=exhausts.size();n;){const auto index=--n;auto& slot=slots.exhausts[index];
            if(slot<0)slot=allocate(static_cast<std::uint32_t>(physics.field_754()),slot);
            if(slot<0)continue;
            prepare(slot,light);
            auto direction=frames_.car_transform(exhausts[index].direction,body.pose().rotation);
            const float scale=static_cast<float>((static_cast<long double>(fields.get(0x760))-fields.get(0x75c))*(static_cast<long double>(drive.engine_speed)/fields.get(0x221c))+fields.get(0x75c));
            for(unsigned axis=0;axis<3;++axis)direction[axis]*=scale;
            const auto point=frames_.car_transform(exhausts[index].position,body.pose().world);auto velocity=velocity_at(body,point);
            for(unsigned axis=0;axis<3;++axis)velocity[axis]+=direction[axis];
            publish(slot,point,velocity,.10000000149011612f);
        }
    }
    void impacts(const CarMaterialRegistry& materials,const std::vector<CarBodyContactRecord>& records,const CarBodyIntegration<Frames>& body,
            CarColdParticleSlots& slots,std::uint32_t features,bool live,float light){ //00509540.
        for(auto n=slots.impacts.size();n;){const auto index=--n;auto& slot=slots.impacts[index];
            if((features&0x200u)&&live&&index<records.size()){
                const auto signed_id=car_contact_signed_word(materials.responses().at(materials.indices().at(records[index].surface)).words20[2]);
                const auto definition=signed_id<0?0u-static_cast<std::uint32_t>(signed_id):static_cast<std::uint32_t>(signed_id);
                if(car_contact_signed_word(definition)>0){
                    select(slot,definition);if(slot>=0){prepare(slot,light);publish(slot,body.physical_position(),velocity_at(body,body.physical_position()),.009999999776482582f);}continue;
                }
            }
            retire(slot);
        }
    }
private:
    static std::uint32_t word(float value){std::uint32_t bits;std::memcpy(&bits,&value,4);return bits;}
    static Vector cross(const Vector& a,const Vector& b){Vector c{};for(unsigned n=0;n<3;++n)c[n]=static_cast<float>(static_cast<long double>(a[(n+1)%3])*b[(n+2)%3]-static_cast<long double>(a[(n+2)%3])*b[(n+1)%3]);return c;}
    static Vector velocity_at(const CarBodyIntegration<Frames>& body,const Vector& point){
        //00507146..005071EE: offsets stay extended, cross X/Y spill,
        //cross Z stays on x87 through the final velocity subtraction.
        const auto x=static_cast<long double>(point[0])-body.pose().centre[0];
        const auto y=static_cast<long double>(point[1])-body.pose().centre[1];
        const auto z=static_cast<long double>(point[2])-body.pose().centre[2];
        const auto& angular=body.motion().angular_velocity;auto result=body.motion().velocity;
        const float first=static_cast<float>(z*angular[1]-y*angular[2]);
        const float second=static_cast<float>(x*angular[2]-z*angular[0]);
        result[0]-=first;result[1]-=second;
        result[2]=static_cast<float>(static_cast<long double>(result[2])-(y*angular[0]-x*angular[1]));
        return result;
    }
    void retire(std::int32_t& slot){if(slot>=0){retire_car_particle(pool_,slot);slot=-1;}}
    void select(std::int32_t& slot,std::uint32_t definition){
        if(slot<0)slot=allocate(definition,slot);
        if(slot>=0&&pool_.at(static_cast<std::size_t>(slot)).definition!=definition){retire_car_particle(pool_,slot);slot=allocate(definition,slot);}
    }
    void prepare(std::int32_t slot,float light){
        const auto& state=pool_.at(static_cast<std::size_t>(slot));const auto world=frames_.car_world_matrix(state.visual);
        frames_.car_place_in_scene(state.visual,{{world[12],world[13],world[14],world[15]}});
        const auto& definition=definitions_.at(state.definition);
        for(unsigned n=0;n<2;++n){const auto colour=definition.vector(0x40+n*16);auto hsv=particle_rgb_to_hsv(colour);hsv[2]*=light;
            //006E9D30 writes XYZ only; original scratch W is undefined.
            //Retain serialized padding here; it is not a recovered alpha producer.
            auto rgb=particle_hsv_to_rgb(hsv);rgb[3]=colour[3];frames_.car_particle_vector_property(state.visual,0x1d+n,rgb);}
    }
    void publish(std::int32_t slot,const Vector& point,const Vector& velocity,float spread){
        const auto& state=pool_.at(static_cast<std::size_t>(slot));Vector first{},second{};
        for(unsigned axis=0;axis<3;++axis){first[axis]=velocity[axis]+state.vector0[axis];
            if(axis==2){const float product=velocity[axis]*spread;second[axis]=product+state.vector10[axis];}
            else second[axis]=static_cast<float>(static_cast<long double>(velocity[axis])*spread+state.vector10[axis]);}
        for(unsigned n=0;n<6;++n){const auto value=n<3?first[n]:second[n-3];std::uint32_t bits;std::memcpy(&bits,&value,4);frames_.car_particle_property(state.visual,0xd+n,bits);}
        frames_.car_suspension_position(state.visual,point);frames_.update_scene_object(state.visual);
    }
    Frames& frames_;Callbacks& callbacks_;const std::vector<CarParticleDefinition>& definitions_;std::vector<CarParticleState<Frame>>& pool_;
};
} //namespace hd2::game
#endif
