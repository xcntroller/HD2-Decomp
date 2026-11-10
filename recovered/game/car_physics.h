#ifndef HD2_GAME_CAR_PHYSICS_H
#define HD2_GAME_CAR_PHYSICS_H

#include "game/car_parameters.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <map>
#include <optional>
#include <vector>

namespace hd2::game {

// Scalar simulation fields retain original offsets as research identifiers, not
// native object layout. No engine/client addresses or uninitialized padding are
// stored here. A read of a coefficient not yet produced is an integration error.
class CarPhysicsScalars {
public:
    float get(std::size_t field) const { return values_.at(field); }
    void set(std::size_t field, float value) { values_[field] = value; }
    const std::map<std::size_t,float>& values() const { return values_; }
private:
    std::map<std::size_t,float> values_;
};

struct CarWheelPhysics {
    float radius;                     //004E2E30 -> wheel1D8.
    std::uint32_t flags;               //Real geometry flags, including dummy400.
    std::int32_t material_id = -1, material_index = -1;
    CarPhysicsScalars scalars;
};
struct CarPartPhysics {
    std::uint8_t type;                 //Actual constructed part's byte3A.
    CarPhysicsScalars scalars;
};
struct CarPhysicsGeometry {
    std::array<float,3> minimum, maximum;
    std::size_t axles;
    std::uint32_t flags;               //Geometry/car variant flags at F74.
    std::vector<CarWheelPhysics> wheels;
    std::vector<std::uint16_t> first_connections, second_connections;
    std::vector<CarPartPhysics> parts;
};

//004ED270's runtime branch (third argument zero). Geometry must come from real
//004E2E30 construction. Material IDs must come from the native registry owner;
//an empty registry genuinely maps every requested ID to -1, as original.
//Editor repair/remap branches are deliberately not exposed by this entry point.
//Mutations remain visible after rejection; the caller must discard or handle
//the failed application before publishing readiness. Native guards are stricter
//for malformed non-finite input and do not reproduce undefined padding copies.
class CarPhysics {
public:
    explicit CarPhysics(CarPhysicsGeometry geometry) : geometry_(std::move(geometry)) {
        if (geometry_.axles > 12 || geometry_.axles * 2 != geometry_.wheels.size())
            throw std::runtime_error("Car geometry has inconsistent axle/wheel counts");
    }
    const CarPhysicsGeometry& geometry() const { return geometry_; }
    const CarPhysicsScalars& scalars() const { return body_; }
    void contact_mass(float value){ //004D3EF0 temporarily scales SAME core918 during one contact.
        if(!std::isfinite(value)||value<=0)throw std::runtime_error("Invalid Car contact mass");
        body_.set(0x918,value);
    }
    //Native1E0/220..228 are live centre/inertia fields as well as initial
    //004ED270 outputs.004F3950 updates this SAME storage before the solver.
    void load_mass_result(const std::array<float,3>& centre,const std::array<float,3>& inertia) {
        for(unsigned axis=0;axis!=3;++axis){body_.set(0x1e0+axis*4,centre[axis]);body_.set(0x220+axis*4,inertia[axis]);}
    }
    const std::map<std::size_t,std::uint8_t>& flags() const { return flags_; }
    const std::optional<CarParameters>& applied_parameters() const { return applied_; }
    std::int32_t gear_count() const { return gear_count_; }
    std::int32_t field_754() const { return field_754_; }

    bool apply(const CarParameters& p, const std::vector<std::int32_t>& material_ids) {
        auto r = [&](std::size_t index) {
            const auto value = real(p,index);
            if (!std::isfinite(value)) throw std::runtime_error("Non-finite Car parameter");
            return value;
        };
        auto set = [&](std::size_t field, std::size_t index, float scale = 1.f) {
            body_.set(field, r(index) * scale);
        };
        auto put = [&](std::size_t field, float value) { body_.set(field, value); };
        auto get = [&](std::size_t field) { return body_.get(field); };
        auto nonnegative = [&](std::size_t field, std::size_t index, float scale = 1.f) {
            if (!(r(index) >= 0.f)) return false;
            set(field,index,scale); return true;
        };
        auto positive = [&](std::size_t field, std::size_t index, float scale = 1.f) {
            if (!(r(index) > 0.f)) return false;
            set(field,index,scale); return true;
        };
        auto range = [&](std::size_t field, std::size_t index, float low, float high, float scale = 1.f) {
            if (r(index) < low || r(index) > high) return false;
            set(field,index,scale); return true;
        };
        auto unit = [&](std::size_t field, std::size_t index) {
            if (!range(field,index,0.f,1.0010000467300415f)) return false;
            if (get(field) > 1.f) put(field,1.f);
            return true;
        };
        put(0xf7c,0);
        for (std::size_t i=0;i!=3;++i) set(0x140 + i*4,0x1ae + i);
        set(0x67c,0x1b1);
        flags_[0x2289]=p.flag(0x698); flags_[0x228c]=p.flag(0x121c);
        if (!nonnegative(0x6f4,0x1a2) || !range(0x6f8,0x1a3,1.f,INFINITY)) return false;
        set(0x674,0x1a4);
        flags_[0x228f]=p.flag(0x121d);flags_[0x2290]=p.flag(0x121e);
        put(0x550,p.flag(0x121f)?2.f:0.f);
        set(0x518,0x1a5);set(0x510,0x1a7);set(0x514,0x1a8);
        field_754_=signed_word(p.word(0x1a9));
        if (field_754_<0 || !nonnegative(0x758,0x1aa,kph_to_mps) || !nonnegative(0x75c,0x1ab)) return false;
        if (!(r(0x1ac)>=r(0x1ab))) return false;
        set(0x760,0x1ac);set(0x504,0x1ad,0.017453279346227646f);
        for (const auto& item : {std::pair<std::size_t,std::size_t>{0x234,0x19b},{0x238,0x19c},{0x230,0x19d}}) {
            if (!(r(item.second)>0.f)) return false;
            put(item.first,1.f/r(item.second));
        }
        for (std::size_t i=0;i!=3;++i) put(0x220 + i*4,get(0x230 + i*4));
        if (!unit(0x928,0x53)||!unit(0x924,0x52)||!unit(0x920,0x51)) return false;
        for (std::size_t i=0;i!=3;++i) {
            const auto value=(geometry_.maximum[i]-geometry_.minimum[i])*get(0x920 + i*4)+geometry_.minimum[i];
            put(0x1f0 + i*4,value);put(0x1e0 + i*4,value);
        }
        if (!positive(0x788,0x54)||!positive(0x91c,0)) return false;
        put(0x918,get(0x91c));put(0x200,0);put(0x204,-get(0x91c)*get(0x788));put(0x208,0);
        if (!positive(0x720,0x19f)||!nonnegative(0x71c,0x19e)||
            !positive(0x70c,0x1a1)||!nonnegative(0x708,0x1a0)) return false;
        put(0x8dc,0.0001f);put(0x8d8,.01f);
        for (std::size_t axle=0;axle!=geometry_.axles;++axle) {
            for (std::size_t index=axle*2;index!=axle*2+2;++index) {
                auto& wheel=geometry_.wheels[index];auto& fields=wheel.scalars;
                auto assign = [&](std::size_t field,std::size_t parameter,float low,float high,float scale=1.f,bool strict=false) {
                    const auto value=r(parameter+axle);
                    if (!std::isfinite(value)) throw std::runtime_error("Non-finite Car wheel parameter");
                    if ((strict?value<=low:value<low)||value>high) return false;
                    fields.set(field,value*scale);return true;
                };
                if (!assign(0x160,0x57,0,INFINITY,1000,true)||!assign(0x164,0x63,.008999999612569809f,INFINITY,1,true)||
                    !assign(0x170,0x6f,0,INFINITY,1,true)||!assign(0x174,0x7b,0,INFINITY,1,true)||
                    !assign(0x178,0x87,-10,10,radians)||!assign(0x150,0x93,-10,10,radians)||
                    !assign(0x154,0x9f,0,INFINITY,radians)||!assign(0x158,0xab,0,INFINITY,radians)) return false;
                wheel.material_id=signed_word(p.word(0xb7 + axle));
                if (wheel.material_id<0) return false;
                wheel.material_index=-1;
                for (std::size_t i=material_ids.size();i;--i)
                    if (material_ids[i-1]==wheel.material_id) {wheel.material_index=static_cast<std::int32_t>(i-1);break;}
                if (wheel.material_index<0) wheel.material_id=-1;
                fields.set(0x188,0);
                if (!assign(0x18c,0xc3,0,INFINITY,1000,true)||!assign(0x194,0xcf,1,INFINITY,radians)||
                    !assign(0x198,0xdb,1,INFINITY,radians)||!assign(0x19c,0xe7,0,1)||!assign(0x190,0xf3,0,1)||
                    !assign(0x1b8,0xff,0,INFINITY,1,true)||!assign(0x1bc,0x10b,0,INFINITY,1,true)||
                    !assign(0x1c0,0x117,0,INFINITY)||!assign(0x1c4,0x123,0,INFINITY,1000,true)||
                    !assign(0x1c8,0x12f,0,INFINITY,1,true)||!assign(0x1cc,0x13b,0,INFINITY,1,true)||
                    !assign(0x1d0,0x147,0,INFINITY)||!assign(0x1f8,0x153,0,100,.01f)) return false;
                const std::array<std::pair<std::size_t,std::uint32_t>,4> flag_fields{{{0x11ec,2},{0x11f8,1},{0x1204,4},{0x1210,0x800}}};
                for (const auto& flag:flag_fields) wheel.flags=p.flag(flag.first+axle)?wheel.flags|flag.second:wheel.flags&~flag.second;
                wheel.flags &= ~0x100u;
                if (std::find(geometry_.first_connections.begin(),geometry_.first_connections.end(),index)!=geometry_.first_connections.end() ||
                    std::find(geometry_.second_connections.begin(),geometry_.second_connections.end(),index)!=geometry_.second_connections.end()) wheel.flags|=0x100u;
                const auto brake=r((index%2?0x16b:0x15f)+axle);
                if (!(brake>=0)) return false;
                fields.set(0x1f4,get(0x71c)*brake*.01f);
                if (!assign(0x21c,0x177,1,90,radians) || r(0x18f + axle)<r(0x183 + axle) ||
                    !assign(0x220,0x183,1,90,radians)||!assign(0x224,0x18f,1,90,radians)) return false;
                if ((wheel.flags&2)&&!(wheel.flags&0x400)&&get(0x8d8)<wheel.radius*tau) {
                    put(0x8d8,wheel.radius*tau);put(0x8dc,wheel.radius);
                }
                constexpr std::array<std::size_t,5> targets{{0x24c,0x248,0x254,0x250,0x258}};
                for (std::size_t i=0;i!=targets.size();++i)fields.set(targets[i],r(0x1b2 + i));
            }
        }
        flags_[0x228d]=p.flag(0x1220);put(0x5dc,0);put(0x73c,0);
        for (const auto& wheel:geometry_.wheels) {
            if (!(wheel.flags&0x400)) {
                put(0x5dc,get(0x5dc)+wheel.scalars.get(0x1f4)/wheel.radius);
                if (wheel.flags&2) put(0x73c,get(0x73c)+1.f);
            }
        }
        if (!(geometry_.flags&2)&&get(0x73c)==0) return false;
        if (!positive(0x92c,0x4a,radians)||r(0x4a)>90||
            !positive(0x930,0x4b,radians)||r(0x4b)>90||
            !range(0x954,0x4c,0,50,radians)||r(0x4a)<r(0x4c)) return false;
        for (std::size_t i=0;i!=4;++i) if (!positive(0x938 + i*4,0x4d + i,radians)) return false;
        constexpr std::array<std::size_t,3> drag_fields{{0x854,0x728,0x730}};
        for (std::size_t i=0;i!=3;++i)
            if (!range(drag_fields[i],0x44 + i*2,.028999999165534973f,1.3309999704360962f)||
                !nonnegative(drag_fields[i]+4,0x45 + i*2)) return false;
        put(0x1c0,get(0x1f0));put(0x1c4,get(0x1f4));put(0x1c8,geometry_.maximum[2]);
        put(0x1d0,get(0x1f0));put(0x1d4,get(0x1f4));put(0x1d8,geometry_.minimum[2]);
        if (!nonnegative(0x87c,3,rpm_to_rps)||!(r(4)>r(3))) return false;
        set(0x221c,4,rpm_to_rps);
        if (!(r(5)>0)||r(3)<r(5)) return false;
        set(0x914,5,rpm_to_rps);put(0x870,get(0x914)*.5f);
        if (!nonnegative(0x8f8,0xd)||!nonnegative(0x900,0xe)||!nonnegative(0x8fc,0xf)) return false;
        put(0x900,1.f/r(0xe));put(0x8fc,1.f/r(0xf));
        if (!range(0x8ec,0x10,0,r(0x12),rpm_to_rps)||!(r(0x11)<=r(0x12))||!(r(0x12)<=r(3))) return false;
        set(0x8f0,0x11,rpm_to_rps);set(0x8f4,0x12,rpm_to_rps);
        if (!(r(0x13)>=1&&r(0x13)<=10)) return false;
        gear_count_=static_cast<std::int32_t>(r(0x13));
        for (std::size_t i=0;i!=15;++i) if (!nonnegative(0x88c + i*4,0x15 + i)) return false;
        if (!nonnegative(0x884,0x14)) return false;
        put(0x888,get(0x88c));
        put(0x6f0,(get(0x8d8)/get(0x888 + std::size_t(gear_count_)*4))*get(0x221c));
        for (std::size_t i=0;i!=15;++i) {
            if (!range(0x7d0 + i*4,0x25 + i,0,100.0000991821289f)||!range(0x814 + i*4,0x35 + i,0,100.0000991821289f)) return false;
            put(0x7d0 + i*4,std::min(r(0x25 + i)*.01f*(get(0x221c)-get(0x914))+get(0x914),get(0x221c)));
            put(0x814 + i*4,std::max(r(0x35 + i)*.01f*(get(0x221c)-get(0x914))+get(0x914),get(0x914)));
        }
        put(0x814,0);
        if (!positive(0x850,1,1000)||!positive(0x704,2,.01f)) return false;
        put(0x850,get(0x704)*get(0x850));set(0x77c,3,rpm_to_rps);put(0x76c,get(0x850)/(get(0x77c)*tau));
        if (!(r(9)>r(5))||r(10)<r(9)||r(3)<=r(10)) return false;
        set(0x774,9,rpm_to_rps);set(0x778,10,rpm_to_rps);
        if (!positive(0x768,6)||!positive(0x764,7)||!positive(0x770,8)||!positive(0x8e8,0xb)) return false;
        const auto torque=(get(0x914)/get(0x221c))*get(0x8e8);put(0x524,torque/(torque+get(0x764)));
        if (!positive(0x738,0xc)||!range(0x2220,0x55,1,INFINITY)||!(r(0x56)>0)) return false;
        auto factor=r(4)*rpm_to_rps*.4166666567325592f;
        factor=factor/get(0x888 + std::size_t(gear_count_)*4);factor=100000.f/(factor*get(0x8d8));
        put(0x784,(r(0x56)/factor)*1.7000000476837158f);
        for (std::size_t i=0;i!=4;++i)set(0x95c + i*4,0x21a + i);
        if (r(0x21f)<r(0x21e)) return false;
        set(0x5e4,0x21e,kph_to_mps);set(0x5e8,0x21f,kph_to_mps);set(0x5ec,0x220);
        put(0x96c,r(0x221)*get(0x8e8)*-.009999999776482582f);
        put(0x970,r(0x222)*get(0x768)*.01f);set(0x974,0x223);set(0x978,0x224);
        for (std::size_t i=0;i!=4;++i) {
            //004EF4xx's reverse loop ends at C38/22AD. C34 belongs to
            //puncture volume and is subsequently set to1, not impact level0.
            put(0xc38 + i*4,r(0x344 + i)*kph_to_mps*get(0x918));flags_[0x22ad + i]=p.flag(0x122b + i);
            put(0xf38 + i*4,r(0x477 + i)*kph_to_mps*get(0x918));flags_[0x22bb + i]=p.flag(0x1239 + i);
        }
        if (!sound_rpm(p,0x26d,0x33d,0xbfc)||!sound_rpm(p,0x394,0x468,0xedc)) return false;
        for (std::size_t i=0;i!=4;++i)set(0xee8 + i*4,0x46a + i);
        if (!sound_speed(p,0x271,0x33f,0xc08)||!sound_speed(p,0x398,0x46e,0xef8)) return false;
        for (std::size_t i=0;i!=4;++i)set(0xf10 + i*4,0x473 + i);
        for (const auto base:{0xc20u,0xf20u}) {
            put(base,.55f);put(base+4,base==0xc20u?1.3f:1.4f);
            put(base+8,get(0x6f0)*.01f);put(base+12,get(0x6f0)*.6000000238418579f);
            put(base+16,get(0x6f0)*.30000001192092896f);put(base+20,1);
        }
        if (!sound_fixed(p,0x354,0x3ec,0xcec)) return false;
        for (std::size_t group=0;group!=3;++group)for (std::size_t row=0;row!=5;++row) {
            if (!sound_group(p,0x231 + group*20+row*4,0x2c5 + group*40+row,0xa1c + group*0xa0 + row*4))return false;
            if (!sound_group(p,0x358 + group*20+row*4,0x3f0 + group*40+row,0xcfc + group*0xa0 + row*4))return false;
            if (group<2) {
                if (!p.name(0x231 + group*20+row*4).empty())flags_[0x22a3 + group*5+row]=p.flag(0x1221 + group*5+row);
                if (!p.name(0x358 + group*20+row*4).empty())flags_[0x22b1 + group*5+row]=p.flag(0x122f + group*5+row);
            }
        }
        constexpr std::array<std::pair<std::size_t,std::size_t>,10> tail{{{0x61c,0x1b7},{0x620,0x1b8},{0x614,0x1b9},{0x618,0x1ba},{0x608,0x1bb},{0x624,0x1bc},{0x628,0x1bd},{0x62c,0x1be},{0x630,0x1bf},{0x634,0x1c0}}};
        for (const auto& field:tail)set(field.first,field.second,field.first==0x61c||field.first==0x620?kph_to_mps*get(0x918):field.first==0x62c?kph_to_mps:1.f);
        put(0x648,r(0x1c1)*kph_to_mps*get(0x918));put(0x64c,r(0x1c2)*kph_to_mps*get(0x918));set(0x640,0x1c3);set(0x644,0x1c4);
        for (auto& wheel:geometry_.wheels) {
            wheel.scalars.set(0x228,r(0x1c9)*radians);wheel.scalars.set(0x234,r(0x1c7));wheel.scalars.set(0x244,r(0x1ca));
            wheel.scalars.set(0x23c,r(0x1c5)*kph_to_mps*get(0x918));wheel.scalars.set(0x240,r(0x1c6)*kph_to_mps*get(0x918));wheel.scalars.set(0x238,r(0x1c8));
        }
        put(0x558,r(0x1e8)*kph_to_mps*get(0x918));put(0x55c,r(0x1e9)*kph_to_mps*get(0x918));set(0x560,0x1ea);set(0x564,0x1eb);set(0x568,0x1ec);
        for (auto& part:geometry_.parts) apply_part(part,p);
        applied_=p;return true;
    }
private:
    static constexpr float radians=.01745329238474369f, rpm_to_rps=.01666666753590107f, kph_to_mps=.2777777910232544f, tau=6.283185005187988f;
    static float real(const CarParameters& p, std::size_t index) {
        const auto value=p.real(index);
        if (!std::isfinite(value)) throw std::runtime_error("Non-finite Car parameter");
        return value;
    }
    static std::int32_t signed_word(std::uint32_t value) {std::int32_t result;std::memcpy(&result,&value,4);return result;}
    bool sound_rpm(const CarParameters& p,std::size_t name,std::size_t index,std::size_t field) {
        if (p.name(name).empty())return true;
        const auto percentage=real(p,index), volume=real(p,index+1);
        if (percentage<0||percentage>100.0009994506836f||volume<0||volume>1.0010000467300415f)return false;
        body_.set(field,std::min(percentage*.01f*(body_.get(0x221c)-body_.get(0x914))+body_.get(0x914),body_.get(0x221c)));
        body_.set(field+4,real(p,5)*rpm_to_rps);body_.set(field+8,std::min(volume,1.f));return true;
    }
    bool sound_speed(const CarParameters& p,std::size_t name,std::size_t index,std::size_t field) {
        if (p.name(name).empty())return true;
        if (real(p,index)<.5f||real(p,index+1)<=real(p,index)||real(p,index+1)>2.009999990463257f||real(p,index+1)<real(p,index))return false;
        body_.set(field,real(p,index));body_.set(field+4,std::min(real(p,index+1),2.f));
        for(std::size_t i=2;i!=4;++i){if(real(p,index+i)<0||real(p,index+i)>100.0009994506836f)return false;body_.set(field+i*4+(i==3?4:0),std::min(real(p,index+i)*.01f*body_.get(0x6f0),body_.get(0x6f0)));}
        body_.set(field+12,body_.get(0x6f0));
        if(real(p,index+4)<0||real(p,index+4)>1.0010000467300415f)return false;
        body_.set(field+20,std::min(real(p,index+4),1.f));return true;
    }
    bool sound_fixed(const CarParameters& p,std::size_t name,std::size_t index,std::size_t field) {
        if(p.name(name).empty())return true;
        if(real(p,index)<.5f||real(p,index+1)<=real(p,index)||real(p,index+1)>2.009999990463257f||real(p,index+1)<real(p,index)||real(p,index+2)<0||real(p,index+2)>100.0009994506836f||real(p,index+3)<0||real(p,index+3)>1.0010000467300415f)return false;
        body_.set(field,real(p,index));body_.set(field+4,std::min(real(p,index+1),2.f));
        body_.set(field+8,std::min(real(p,index+2)*.01f*(body_.get(0x221c)-body_.get(0x914))+body_.get(0x914),body_.get(0x221c)));
        body_.set(field+12,std::min(real(p,index+3),1.f));return true;
    }
    bool sound_group(const CarParameters& p,std::size_t name,std::size_t index,std::size_t field) {
        if(p.name(name).empty())return true;
        if(real(p,index)<.5f||real(p,index+5)<=real(p,index)||real(p,index+5)>2.009999990463257f||real(p,index+5)<real(p,index))return false;
        body_.set(field,real(p,index));body_.set(field+20,std::min(real(p,index+5),2.f));
        for(std::size_t i=2;i!=6;++i){const auto value=real(p,index+i*5);if(value<0||value>100.0009994506836f||((i==3||i==5)&&value<p.real(index+(i-1)*5)))return false;body_.set(field+i*20,std::min(value*.01f*(body_.get(0x221c)-body_.get(0x914))+body_.get(0x914),body_.get(0x221c)));}
        for(std::size_t i=6;i!=8;++i){const auto value=real(p,index+i*5);if(value<0||value>1.0010000467300415f)return false;body_.set(field+i*20,std::min(value,1.f));}
        return true;
    }
    void apply_part(CarPartPhysics& part,const CarParameters& p) {
        constexpr std::array<std::size_t,11> index{{0,0x1cb,0x1d1,0x1d8,0x1f7,0x1e8,0x1fe,0x1f0,0x205,0x20c,0x213}};
        if(!part.type||part.type>=index.size())return;
        const auto source=index[part.type];auto& fields=part.scalars;
        fields.set(0x14,real(p,source)*kph_to_mps*body_.get(0x918));fields.set(0x18,real(p,source+1)*kph_to_mps*body_.get(0x918));
        fields.set(0x1c,part.type==1?100.f:real(p,source+2));
        const bool extra=part.type==3||part.type==5;
        const auto tail=part.type==1?2u:extra?4u:3u;
        fields.set(0x28,real(p,source+tail));fields.set(0x2c,real(p,source+tail+1)*.01f);fields.set(0x30,real(p,source+tail+2)*.01f);fields.set(0x34,real(p,source+tail+3)*.01f);
        if(extra)fields.set(0x20,real(p,source+3));
    }
    CarPhysicsGeometry geometry_;
    CarPhysicsScalars body_;
    std::map<std::size_t,std::uint8_t> flags_;
    std::optional<CarParameters> applied_;
    std::int32_t gear_count_=0, field_754_=0;
};

} //namespace hd2::game
#endif
