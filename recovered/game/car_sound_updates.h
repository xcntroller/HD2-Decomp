#ifndef HD2_GAME_CAR_SOUND_UPDATES_H
#define HD2_GAME_CAR_SOUND_UPDATES_H

#include "game/car_sound_loading.h"
#include "game/car_body_integration.h"
#include "game/car_drivetrain.h"
#include "game/car_definition_tables.h"
#include "game/car_auxiliary_geometry.h"

namespace hd2::game {

//0083BEC4 starts at FLOAT1, but00518AB0 temporarily changes this SAME process
//gain while calling the engine. Other Car sound workers read it too.
inline float car_sound_gain=1.f;

//Required0050B180 sound consumers, on the SAME81 sound slots. Calls below retain
//the actual78(state,0),98(rate),8C(volume) and30(enable) engine boundaries.
template<class Frames>class CarSoundUpdates {
public:
    using Frame=typename Frames::Frame;using Vector=typename Frames::Vector;
    CarSoundUpdates(Frames& frames,CarSoundLoading<Frames>& sounds):frames_(frames),sounds_(sounds){}
    void pause_engine_loops(){ //0051B7C0, AC(0,1), distinct from30(false).
        auto pause=[&](unsigned slot){if(auto sound=sounds_.sound(slot))frames_.car_sound_stop(sound);};
        for(unsigned bank:{0x988u,0x99cu,0x9b0u})for(unsigned n=0;n<5;++n)pause(bank+n*4);
        pause(0x9c4);pause(0xc54);
        for(unsigned bank:{0xc58u,0xc6cu,0xc80u})for(unsigned n=0;n<5;++n)pause(bank+n*4);
        pause(0xc94);
    }
    void engine(const CarPhysics& physics,const CarDrivetrainState& drive,std::uint32_t features,
            bool transition,bool inside,bool& last_inside,float& timeout,float elapsed){ //00518AB0.
        if(!transition)return;
        const auto saved_gain=car_sound_gain;
        timeout+=elapsed;if(timeout>.30000001192092896f)timeout=.30000001192092896f;
        car_sound_gain=static_cast<float>(static_cast<long double>(car_sound_gain)*timeout*3.3333332538604736f);
        //Engine endpoints cannot unwind in the original ABI. Our required
        //endpoints can report failure; do not leak a temporary process gain.
        struct Restore {float saved;~Restore(){car_sound_gain=saved;}} restore{saved_gain};
        const auto& fields=physics.scalars();float speed=drive.engine_speed;
        if(speed<fields.get(0x914))speed=fields.get(0x914);
        else{const float upper=fields.get(0x221c)-.0010000000474974513f;if(upper<speed)speed=upper;}
        auto bank_stop=[&](bool selected_inside,unsigned bank){for(unsigned n=5;n;)stop((selected_inside?0x988u:0xc58u)+bank*20+(--n)*4);};
        if(last_inside!=inside){
            for(unsigned bank=0;bank<3;++bank)bank_stop(last_inside,bank);
            if(last_inside){stop(0x9c4);stop(0x9c8);}
            else{stop(0xc94);stop(0xc98);stop(0xc54);}
            last_inside=inside;
        }
        if(!(features&0x20u)&&!last_inside){
            if(const auto sound=sounds_.sound(0xc54)){
                float volume=fields.get(0xcf8);
                if(speed<fields.get(0xcf4))volume=static_cast<float>((static_cast<long double>(speed)-fields.get(0x914))/
                    (static_cast<long double>(fields.get(0xcf4))-fields.get(0x914))*volume);
                const auto pitch=static_cast<float>((static_cast<long double>(speed)-fields.get(0x914))/
                    ((static_cast<long double>(fields.get(0x221c))-fields.get(0x914))/
                     (static_cast<long double>(fields.get(0xcf0))-fields.get(0xcec)))+fields.get(0xcec));
                if(!frames_.car_sound_state(sound))frames_.human_sound_enabled(sound,true);
                frames_.car_sound_parameter98(sounds_.sound(0xc54),pitch);
                frames_.human_sound_volume(sounds_.sound(0xc54),car_sound_gain*volume);
            }
        }else{
            if(drive.gear<0){bank_stop(last_inside,0);bank_stop(last_inside,1);}
            else bank_stop(last_inside,2);
            const unsigned first=drive.gear<0?2u:0u,last=drive.gear<0?3u:2u;
            for(unsigned bank=first;bank<last;++bank)for(unsigned n=5;n;){const auto row=--n;
                const auto slot=(last_inside?0x988u:0xc58u)+bank*20+row*4;
                const auto sound=sounds_.sound(slot);if(!sound)continue;
                const auto field=(last_inside?0xa1cu:0xcfcu)+bank*0xa0+row*4;
                const auto low=fields.get(field+40),high=fields.get(field+60);
                const bool neutral_disabled=bank<2&&drive.gear==0&&physics.flags().at((last_inside?0x22a3u:0x22b1u)+bank*5+row);
                if(!(low<=speed&&speed<=high)||neutral_disabled){if(frames_.car_sound_state(sound))stop(slot);continue;}
                const auto blend=bank==1?static_cast<long double>(1.f)-drive.torque_amount:static_cast<long double>(drive.torque_amount);
                float volume=static_cast<float>((static_cast<long double>(fields.get(field+120))-fields.get(field+140))*blend+fields.get(field+140));
                if(volume>.009999999776482582f){
                    const auto fade_in=fields.get(field+80),fade_out=fields.get(field+100);
                    if(speed<fade_in&&low!=fade_in)volume=static_cast<float>((static_cast<long double>(speed)-low)/(static_cast<long double>(fade_in)-low)*volume);
                    else if(fade_out<speed&&high!=fade_out)volume=static_cast<float>((1.L-(static_cast<long double>(speed)-fade_out)/(static_cast<long double>(high)-fade_out))*volume);
                }
                const auto pitch=static_cast<float>((static_cast<long double>(speed)-low)/
                    ((static_cast<long double>(high)-low)/(static_cast<long double>(fields.get(field+20))-fields.get(field)))+fields.get(field));
                if(frames_.car_sound_state(sound)){if(volume<.009999999776482582f){stop(slot);continue;}}
                else{if(volume<=.009999999776482582f)continue;frames_.human_sound_enabled(sounds_.sound(slot),true);}
                frames_.car_sound_parameter98(sounds_.sound(slot),pitch);
                frames_.human_sound_volume(sounds_.sound(slot),car_sound_gain*volume);
            }
        }
        const auto slot=last_inside?0x9c4u:0xc94u,base=last_inside?0xbfcu:0xedcu;
        const auto sound=sounds_.sound(slot);if(!sound)return;
        const auto low=fields.get(base+4),high=fields.get(base);
        if(!(low<=speed&&speed<=high)){if(frames_.car_sound_state(sound))stop(slot);return;}
        const float volume=static_cast<float>((static_cast<long double>(high)-speed)/(static_cast<long double>(high)-low)*fields.get(base+8));
        if(frames_.car_sound_state(sound)){if(volume<.009999999776482582f){stop(slot);return;}}
        else{if(volume<=.009999999776482582f)return;frames_.human_sound_enabled(sounds_.sound(slot),true);}
        frames_.human_sound_volume(sounds_.sound(slot),car_sound_gain*volume);
    }
    void audible(CarDrivetrainState& drive,bool enabled){ //004F2710.
        if(!enabled&&drive.audible)sounds_.stop_all();
        drive.audible=enabled;
    }
    void stop_engine_transition(CarDrivetrainState& drive,bool inside,bool& transition,bool& destroyed){ //00518710(false), native22A2/22BF/22A0.
        //These four observations precede even the stop branch's early return.
        if(drive.audible)for(auto offset:{0x97cu,0x980u,0xc48u,0xc4cu})if(auto sound=sounds_.sound(offset))(void)frames_.car_sound_state(sound);
        if(!drive.running)return; //Native22A2 gate, distinct from pending22BF.
        sounds_.stop_all();
        if(auto sound=sounds_.sound(inside?0x980:0xc4c);sound&&drive.audible){frames_.human_sound_enabled(sound,true);frames_.human_sound_volume(sound,car_sound_gain);}
        for(auto offset:{0x97cu,0x980u,0xc48u,0xc4cu})if(auto sound=sounds_.sound(offset))frames_.human_sound_parameter94(sound,0);
        drive.engine_speed=0;drive.running=false;transition=false;destroyed=false;
    }
    void start_engine_transition(CarDrivetrainState& drive,bool inside,bool& transition,bool& last_inside,float& timeout){ //00518710(true).
        bool idle=true;
        if(drive.audible)for(auto offset:{0x97cu,0x980u,0xc48u,0xc4cu})if(auto sound=sounds_.sound(offset))if(frames_.car_sound_state(sound))idle=false;
        if(!drive.running&&!transition&&idle){
            if(auto sound=sounds_.sound(inside?0x97c:0xc48);sound&&drive.audible){
                frames_.human_sound_parameter94(sound,0);frames_.human_sound_enabled(sound,true);frames_.human_sound_volume(sound,car_sound_gain);
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
        if(auto sound=sounds_.sound(last_inside?0x984:0xc50)){frames_.human_sound_enabled(sound,true);frames_.human_sound_volume(sound,car_sound_gain);}
    }
    template<class Random>void clutch_changed(std::uint32_t features,const bool& last_inside,Random& random){ //0051A480.
        if(!(features&0x20u))return;
        bool idle=true;
        if(auto sound=sounds_.sound(0x9d8+clutch_variant_*4))if(frames_.car_sound_state(sound))idle=false;
        if(auto sound=sounds_.sound(0xca8+clutch_variant_*4))if(frames_.car_sound_state(sound))return;
        if(!idle)return;
        const auto draw=random();clutch_variant_=static_cast<std::uint32_t>(draw%100);
        clutch_variant_=clutch_variant_>79u;
        const unsigned slot=(last_inside?0x9d8u:0xca8u)+clutch_variant_*4;
        if(auto sound=sounds_.sound(slot)){
            frames_.human_sound_enabled(sound,true);
            const auto current=sounds_.sound(slot);
            if(!current)throw std::logic_error("Car clutch sound retired before its native volume write");
            frames_.human_sound_volume(current,car_sound_gain);
        }
    }
    void puncture(bool audible,bool inside,const Vector& wheel_position){ //0051A880.
        if(!audible)return;
        const unsigned slot=inside?0x9ec:0xcbc;
        if(const auto sound=sounds_.sound(slot)){
            frames_.human_sound_enabled(sound,true);
            frames_.human_sound_volume(sounds_.sound(slot),car_sound_gain); //SAME0083BEC4.
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
        frames_.human_sound_enabled(sound,true);frames_.human_sound_volume(sound,car_sound_gain*volume); //SAME0083BEC4.
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
        frames_.human_sound_enabled(selected,true);frames_.human_sound_volume(selected,car_sound_gain);
    }
    void signal(bool second,bool enabled,bool inside){ //0051A6C0 /0051A7A0.
        const unsigned first=second?0x9d0:0x9cc,other=second?0xca0:0xc9c;
        if(!enabled){stop(first);stop(other);return;}
        const auto selected=sounds_.sound(inside?first:other);
        if(selected&&!frames_.car_sound_state(selected)){
            frames_.human_sound_enabled(selected,true);frames_.human_sound_volume(selected,car_sound_gain);
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
                frames_.human_sound_volume(selected,car_sound_gain*skid_);}
            stop(inside?0xcb0:0x9e0);
        }else{stop(0x9e0);stop(0xcb0);}
    }
    void door(const CarAuxiliaryGeometry<Frames>& geometry,std::size_t seat,bool opening,bool audible,bool inside){ //0051B660.
        if(!audible)return;
        const auto sound=sounds_.sound(inside?(opening?0xa10:0xa14):(opening?0xce0:0xce4));if(!sound)return;
        frames_.human_sound_enabled(sound,true);frames_.human_sound_volume(sound,car_sound_gain);
        const auto index=geometry.seats().at(seat).door;
        if(index==CarAuxiliaryGeometry<Frames>::no_door)throw std::runtime_error("Car door sound has no actual seat door");
        frames_.car_sound_head(sound,geometry.doors().at(index).position);
    }
    void air_brake(std::uint32_t features,bool inside,bool& requested){ //0051A580; caller supplies actual228E.
        if(!(features&0x20u))return;
        const auto sound=sounds_.sound(inside?0x9d4:0xca4);
        if(sound&&!frames_.car_sound_state(sound)&&!requested){frames_.human_sound_enabled(sound,true);frames_.human_sound_volume(sound,car_sound_gain);}
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
        frames_.car_sound_parameter98(sound,value);frames_.human_sound_volume(sound,car_sound_gain*volume);
    }
    Frames& frames_;CarSoundLoading<Frames>& sounds_;float skid_=0; //004EA780 clears5F0.
    std::uint32_t clutch_variant_=0; //004EA780 clears650;0051A480 retains the selected0/1.
};
} //namespace hd2::game
#endif
