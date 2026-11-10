#ifndef HD2_GAME_CAR_SOUND_UPDATES_H
#define HD2_GAME_CAR_SOUND_UPDATES_H

#include "game/car_sound_loading.h"
#include "game/car_body_integration.h"
#include "game/car_drivetrain.h"
#include "game/car_definition_tables.h"
#include "game/car_auxiliary_geometry.h"

namespace hd2::game {

//Required0050B180 sound consumers, on the SAME81 sound slots.0083BEC4 is
//FLOAT1 in the original image; its xrefs are reads only. Calls below retain
//the actual78(state,0),98(rate),8C(volume) and30(enable) engine boundaries.
template<class Frames>class CarSoundUpdates {
public:
    using Frame=typename Frames::Frame;using Vector=typename Frames::Vector;
    CarSoundUpdates(Frames& frames,CarSoundLoading<Frames>& sounds):frames_(frames),sounds_(sounds){}
    void signal(bool second,bool enabled,bool inside){ //0051A6C0 /0051A7A0.
        const unsigned first=second?0x9d0:0x9cc,other=second?0xca0:0xc9c;
        if(!enabled){stop(first);stop(other);return;}
        const auto selected=sounds_.sound(inside?first:other);
        if(selected&&!frames_.car_sound_state(selected)){
            frames_.human_sound_enabled(selected,true);frames_.human_sound_volume(selected,1.f);
        }
        stop(inside?other:first);
    }
    void rolling(const CarPhysics& physics,const Vector& velocity,bool inside,std::uint32_t features){ //00519A60.
        const float square=static_cast<float>((static_cast<long double>(velocity[2])*velocity[2]+
            static_cast<long double>(velocity[1])*velocity[1])+static_cast<long double>(velocity[0])*velocity[0]);
        std::int32_t bits;std::memcpy(&bits,&square,4);
        const auto speed=bits<0x322bcc77?0.f:std::abs(static_cast<long double>(square)-1)<9.99999993922529e-09f?1.f:
            static_cast<float>(std::sqrt(static_cast<long double>(square)));
        if(inside||(features&0x20u))rate(sounds_.sound(inside?0x9c8:0xc98),physics.scalars(),inside?0xc08:0xef8,speed);
    }
    void puncture(const CarPhysics& physics,const Vector& velocity,const Vector& forward,
            bool inside,std::uint32_t features,bool enabled){ //00519DC0.
        if(!enabled){stop(0xa18);stop(0xce8);return;}
        const auto speed=static_cast<float>(std::abs((static_cast<long double>(forward[0])*velocity[0]+
            static_cast<long double>(forward[1])*velocity[1])+static_cast<long double>(forward[2])*velocity[2]));
        if(inside||(features&0x20u))rate(sounds_.sound(inside?0xa18:0xce8),physics.scalars(),inside?0xc20:0xf20,speed);
    }
    void skidding(const CarPhysics& physics,const std::vector<CarWheelRuntime<Frames>>& wheels,
            const CarMaterialRegistry& materials,const CarDrivetrainState& drive,float speed,float elapsed,
            bool inside){ //0051A130, current5F0 belongs to this same per-Car state.
        const auto& fields=physics.scalars();float maximum=0;
        for(auto n=wheels.size();n;){const auto& wheel=wheels[--n];const auto flags=wheel.flags;
            if(!(flags&8u)||(flags&0x400u))continue;
            if(!materials.responses().at(materials.indices().at(wheel.surface_id)).flag2c||!(speed>.6000000238418579f))continue;
            const auto& scalars=wheel.scalars;float amount=1;
            if(!(flags&0x20u)&&scalars.get(0x1d4)<=scalars.get(0x224)){
                amount=0;
                if(scalars.get(0x220)<scalars.get(0x1d4))amount=std::max(0.f,static_cast<float>(
                    (static_cast<long double>(scalars.get(0x1d4))-scalars.get(0x220))/(static_cast<long double>(scalars.get(0x224))-scalars.get(0x220))));
            }
            const auto& velocity=wheel.vectors.at(0xa0);
            const float square=static_cast<float>((static_cast<long double>(velocity[2])*velocity[2]+
                static_cast<long double>(velocity[1])*velocity[1])+static_cast<long double>(velocity[0])*velocity[0]);
            if(square<4.f)amount=static_cast<float>(static_cast<long double>(square)*amount*.25f);
            if(flags&0x80u){
                const float rolling=static_cast<float>((static_cast<long double>(drive.engine_speed)/car_gear_ratio(physics,drive.gear))*fields.get(0x8d8));
                const float lower=fields.get(0x5e4)+speed;
                if(lower<rolling){const float additional=fields.get(0x5e8)+speed<rolling?1.f:
                    static_cast<float>((static_cast<long double>(rolling)-lower)/fields.get(0x5e8));
                    amount=std::max(amount,additional);}
            }
            maximum=std::max(maximum,amount);if(maximum>.9999899864196777f){maximum=1;break;}
        }
        const float increment=elapsed*fields.get(0x5ec);
        skid_=increment<=static_cast<long double>(maximum)-skid_?skid_+increment:maximum;
        if(skid_>=.9999899864196777f)skid_=1;
        if(skid_>=.009999999776482582f){
            const auto selected=sounds_.sound(inside?0x9e0:0xcb0);
            if(selected){if(!frames_.car_sound_state(selected))frames_.human_sound_enabled(selected,true);
                frames_.human_sound_volume(selected,skid_);}
            stop(inside?0xcb0:0x9e0);
        }else{stop(0x9e0);stop(0xcb0);}
    }
    void door(const CarAuxiliaryGeometry<Frames>& geometry,std::size_t seat,bool opening,bool audible,bool inside){ //0051B660.
        if(!audible)return;
        const auto sound=sounds_.sound(inside?(opening?0xa10:0xa14):(opening?0xce0:0xce4));if(!sound)return;
        frames_.human_sound_enabled(sound,true);frames_.human_sound_volume(sound,1.f);
        const auto index=geometry.seats().at(seat).door;
        if(index==CarAuxiliaryGeometry<Frames>::no_door)throw std::runtime_error("Car door sound has no actual seat door");
        frames_.car_sound_head(sound,geometry.doors().at(index).position);
    }
    void air_brake(std::uint32_t features,bool inside,bool& requested){ //0051A580; caller supplies actual228E.
        if(!(features&0x20u))return;
        const auto sound=sounds_.sound(inside?0x9d4:0xca4);
        if(sound&&!frames_.car_sound_state(sound)&&!requested){frames_.human_sound_enabled(sound,true);frames_.human_sound_volume(sound,1.f);}
        requested=true;
    }
    float skid_amount()const{return skid_;}
private:
    void stop(unsigned slot){if(const auto sound=sounds_.sound(slot))frames_.human_sound_enabled(sound,false);}
    void rate(Frame sound,const CarPhysicsScalars& fields,unsigned offset,float speed){
        if(!sound)return;
        if(fields.get(offset+12)<speed)speed=fields.get(offset+12);
        const auto minimum=fields.get(offset+8);
        if(speed<minimum){if(frames_.car_sound_state(sound))frames_.human_sound_enabled(sound,false);return;}
        float volume=1;const auto fade=fields.get(offset+16);
        if(speed<fade&&fade!=minimum)volume=static_cast<float>((static_cast<long double>(speed)-minimum)/(static_cast<long double>(fade)-minimum));
        const auto value=static_cast<float>((static_cast<long double>(speed)-minimum)/
            ((static_cast<long double>(fields.get(offset+12))-minimum)/(static_cast<long double>(fields.get(offset+4))-fields.get(offset)))+fields.get(offset));
        const auto playing=frames_.car_sound_state(sound);
        if(playing){if(volume<.009999999776482582f){frames_.human_sound_enabled(sound,false);return;}}
        else{if(!(volume>.009999999776482582f))return;frames_.human_sound_enabled(sound,true);}
        frames_.car_sound_parameter98(sound,value);frames_.human_sound_volume(sound,volume);
    }
    Frames& frames_;CarSoundLoading<Frames>& sounds_;float skid_=0; //004EA780 clears5F0.
};
} //namespace hd2::game
#endif
