#ifndef HD2_GAME_DOG_DAMAGE_H
#define HD2_GAME_DOG_DAMAGE_H

#include "game/mission_integrity_tables.h"
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace hd2::game {

inline std::int32_t dog_signed_word(std::uint32_t value){std::int32_t result;std::memcpy(&result,&value,4);return result;}

//The seven borrowed frames produced by005C7D30, stored at Actor478..490.
//Initialization alone005D4D70 does not produce them. Keep the geometry owner
//separate from the cold Dog; its containing activation must retain the model.
template<class Frames>class DogDamageGeometry {
public:
    using Frame=typename Frames::Frame;
    using Vector=std::array<float,4>;
    DogDamageGeometry(Frames& frames,Frame model):frames_(frames){
        constexpr std::array<const char*,7> names{{"f","d","c","h","p","t","l"}};
        for(std::size_t n=0;n!=bones_.size();++n)bones_[n]=frames_.actor_find(model,names[n],0xffff);
    }
    DogDamageGeometry(Frames& frames,const std::array<Frame,7>& actual_bones):frames_(frames),borrowed_bones_(&actual_bones){} //SAME005C7D30 slots.
    std::array<Vector,7> positions(){ //005D73B0 updates/reads every frame before selecting any distance.
        std::array<Vector,7> result{};const auto& bones=borrowed_bones_?*borrowed_bones_:bones_;
        for(std::size_t n=0;n!=bones_.size();++n){
            if(!bones[n])throw std::runtime_error("Dog damage requires its actual skeletal frame");
            const auto matrix=frames_.car_world_matrix(bones[n]);
            for(unsigned axis=0;axis!=3;++axis)result[n][axis]=matrix[12+axis];
        }
        return result;
    }
private:
    Frames& frames_;std::array<Frame,7> bones_{};const std::array<Frame,7>* borrowed_bones_=nullptr;
};

//005D73B0. Raw00404260 is the XYZ constructor;0040E080 writes point-l
//into a local output. Neither helper modifies the caller's impact point.
inline long double dog_scaled_damage(const std::array<std::array<float,4>,7>& bones,
        const std::array<float,4>& point,float damage,MissionIntegrityTables& integrity){
    const auto square=[&](std::size_t n,bool spill_delta,bool xyz){
        std::array<long double,3> delta{};
        for(unsigned axis=0;axis!=3;++axis){
            const auto d=static_cast<long double>(point[axis])-bones[n][axis];
            delta[axis]=spill_delta?static_cast<float>(d):d;
        }
        return xyz?(delta[0]*delta[0]+delta[1]*delta[1])+delta[2]*delta[2]:
            (delta[2]*delta[2]+delta[1]*delta[1])+delta[0]*delta[0];
    };
    float nearest=static_cast<float>(square(2,false,false)),factor=.65f;
    for(const auto n:{0u,1u,3u,4u,5u}){
        const auto distance=square(n,n>=4,false);
        if(distance<nearest){nearest=static_cast<float>(distance);factor=n==0?2.f:n==1?1.5f:.2f;}
    }
    if(square(6,true,true)<nearest)factor=.2f;
    long double adjusted=factor;
    //008CAB7E/008CB27E are ushort index39 in the actual initialized process
    //permutations. A mismatch multiplies, rather than bypassing, native damage.
    if(integrity.expected(39)!=integrity.observed(39))adjusted*=.1f;
    return adjusted*damage; //Original returns ST0 without another FLOAT spill.
}

inline std::int32_t dog_action_category(std::int32_t action){ //005D86F0.
    if(action>6&&action<13)return 3;
    if(action>=0&&action<5)return action==0||action==2?0:1;
    if(action==5||action==6)return 2;
    if(action==18)return 4;
    if(action>12&&action<18)return 5;
    if(action==19||action==20)return 6;
    return 7;
}

//A borrowed view of actual Actor/Body state. None of the reference fields is
//a cold/default producer;005D5290/005D4DB0/005C7D30 must supply the live owner.
template<class Geometry>struct DogDamageView {
    std::uint32_t& health4a0;
    const std::uint32_t& initial_health4b0;
    const std::int32_t& current_action3f4;
    std::int32_t& sight_source53c;
    std::int32_t& sight_listener540;
    std::int32_t& sound548;
    Geometry& geometry;
};

//005D7180, native D4 RET24: kind,amount,point,direction,source,extra.
//The final argument and direction are genuinely unused here. Effects are
//mandatory callbacks to the SAME Dog controller/Body/Sight/SOUND, never noops.
template<class Geometry,class Actor,class Receivers>void apply_dog_damage(DogDamageView<Geometry> dog,
        std::uint32_t mode,std::uint32_t kind,float damage,const std::array<float,4>& point,
        const std::array<float,4>&,Actor* source,std::uint32_t,MissionIntegrityTables& integrity,Receivers& receivers){
    const auto health=dog_signed_word(dog.health4a0);
    if(!(mode&1u)||health<=0)return;
    if(damage==-1.f)dog.health4a0=0;
    else{
        const float old_health=static_cast<float>(health); //005D71CF FLOAT spill before005D73B0.
        const auto next=static_cast<long double>(old_health)-dog_scaled_damage(dog.geometry.positions(),point,damage,integrity);
        if(!std::isfinite(next)||next>=2147483648.L||next<-2147483648.L)
            throw std::runtime_error("Dog damage exceeds native signed health range");
        dog.health4a0=static_cast<std::uint32_t>(static_cast<std::int32_t>(next)); //__ftol, truncate toward zero.
    }
    if(dog_signed_word(dog.health4a0)<=0){
        dog.health4a0=0;
        receivers.dog_control_dead(); //005CE6E0, controller flags20|=800.
        receivers.dog_request_body_action(5,0); //005C8490: queue cancellation/ground/death collision removal remain required.
        if(dog.sight_source53c>=0){receivers.dog_remove_sight_source(dog.sight_source53c);dog.sight_source53c=-1;}
        if(dog.sight_listener540>=0){receivers.dog_remove_sight_listener(dog.sight_listener540);dog.sight_listener540=-1;}
        if(dog.sound548!=-1){receivers.dog_remove_sound(dog.sound548);dog.sound548=-1;}
        return;
    }
    if(dog_action_category(dog.current_action3f4)!=6&&dog_action_category(receivers.dog_next_body_action())!=6)
        receivers.dog_request_body_action(6,2);
    receivers.dog_control_damage(kind,source); //Actual virtual54, source is fifth D4 stack argument.
    if(dog_signed_word(dog.health4a0)<dog_signed_word(dog.initial_health4b0)/8)
        receivers.dog_body_timed_action(2,10000); //005C8540, after control54.
}

} //namespace hd2::game
#endif
