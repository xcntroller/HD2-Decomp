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
    void audible(CarDrivetrainState& drive,bool enabled){ //004F2710.
        if(!enabled&&drive.audible)sounds_.stop_all();
        drive.audible=enabled;
    }
    void stop_engine_transition(CarDrivetrainState& drive,bool inside,bool& transition,bool& destroyed){ //00518710(false), native22A2/22BF/22A0.
        //These four observations precede even the stop branch's early return.
        if(drive.audible)for(auto offset:{0x97cu,0x980u,0xc48u,0xc4cu})if(auto sound=sounds_.sound(offset))(void)frames_.car_sound_state(sound);
        if(!drive.running)return; //Native22A2 gate, distinct from pending22BF.
        sounds_.stop_all();
        if(auto sound=sounds_.sound(inside?0x980:0xc4c);sound&&drive.audible){frames_.human_sound_enabled(sound,true);frames_.human_sound_volume(sound,1.f);}
        for(auto offset:{0x97cu,0x980u,0xc48u,0xc4cu})if(auto sound=sounds_.sound(offset))frames_.human_sound_parameter94(sound,0);
        drive.engine_speed=0;drive.running=false;transition=false;destroyed=false;
    }
    void start_engine_transition(CarDrivetrainState& drive,bool inside,bool& transition,bool& last_inside,float& timeout){ //00518710(true).
        bool idle=true;
        if(drive.audible)for(auto offset:{0x97cu,0x980u,0xc48u,0xc4cu})if(auto sound=sounds_.sound(offset))if(frames_.car_sound_state(sound))idle=false;
        if(!drive.running&&!transition&&idle){
            if(auto sound=sounds_.sound(inside?0x97c:0xc48);sound&&drive.audible){
                frames_.human_sound_parameter94(sound,0);frames_.human_sound_enabled(sound,true);frames_.human_sound_volume(sound,1.f);
            }
            drive.running=true;
        }
        if(!drive.running)return;
        idle=true;
        if(drive.audible)for(auto offset:{0x97cu,0x980u,0xc48u,0xc4cu})if(auto sound=sounds_.sound(offset)){
            if(!frames_.car_sound_state(sound))continue;
            const auto duration=frames_.car_sound_duration(sound);
            if(duration>300&&duration-frames_.car_sound_elapsed(sound)>300){timeout=0;idle=false;}
        }
        if(!idle)return;
        transition=true;last_inside=inside;
    }
    void failed_engine_start(bool last_inside){ //0051A620; caller supplies2296 gate, both state queries precede last-inside selection.
        bool idle=true;
        if(auto sound=sounds_.sound(0x984))if(frames_.car_sound_state(sound))idle=false;
        if(auto sound=sounds_.sound(0xc50))if(frames_.car_sound_state(sound))return;
        if(!idle)return;
        if(auto sound=sounds_.sound(last_inside?0x984:0xc50)){frames_.human_sound_enabled(sound,true);frames_.human_sound_volume(sound,1.f);}
    }
    void puncture(bool audible,bool inside,const Vector& wheel_position){ //0051A880.
        if(!audible)return;
        const unsigned slot=inside?0x9ec:0xcbc;
        if(const auto sound=sounds_.sound(slot)){
            frames_.human_sound_enabled(sound,true);
            frames_.human_sound_volume(sounds_.sound(slot),1.f); //Read-only0083BEC4=1.
            if(!inside)frames_.object_position(sounds_.sound(slot),{{wheel_position[0],wheel_position[1],wheel_position[2]}});
        }
    }
    void suspension(float volume,bool audible,bool inside){ //0051B560.
        if(!audible)return;
        const auto sound=sounds_.sound(inside?0x9e8:0xcb8);if(!sound)return;
        if(frames_.car_sound_state(sound)){
            if(!frames_.car_sound_state(sound))return;
            if(!(static_cast<long double>(volume)+.15000000596046448f>frames_.car_sound_volume(sound)))return;
        }
        frames_.human_sound_enabled(sound,true);frames_.human_sound_volume(sound,volume); //Read-only gain0083BEC4=1.
    }
    template<class Random>void collision(const CarPhysics& physics,float impulse,bool gear,bool inside,Random& random){ //0051A950.
        const auto& fields=physics.scalars();const unsigned threshold=inside?0xc38:0xf38;
        if(impulse<=fields.get(threshold))return;
        unsigned level=0;while(level<3&&!(impulse<fields.get(threshold+(level+1)*4)))++level;
        if(physics.flags().at((inside?0x22ad:0x22bb)+level)&&gear)return;
        const auto first=sounds_.sound((inside?0x9f0:0xcc0)+level*4);
        const auto second=sounds_.sound((inside?0xa00:0xcd0)+level*4);
        Frame selected=first?first:second;if(!selected)return;
        if(first&&second){
            const bool alternate=random()>0x3ffe;selected=alternate?second:first;const auto other=alternate?first:second;
            if(frames_.car_sound_state(selected))return;
            if(frames_.car_sound_state(other)&&frames_.car_sound_elapsed(other)<800)return;
        }else if(frames_.car_sound_state(selected))return;
        frames_.human_sound_enabled(selected,true);frames_.human_sound_volume(selected,1.f);
    }
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
    void reset_skid(){skid_=0;} //004E9DB0's5F0, without stopping/releasing voices.
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
