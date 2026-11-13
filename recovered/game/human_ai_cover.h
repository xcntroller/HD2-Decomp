#ifndef HD2_GAME_HUMAN_AI_COVER_H
#define HD2_GAME_HUMAN_AI_COVER_H
#include "game/mission_navigation.h"
#include "game/human_joint_owners.h"
#include "game/human_ai_shot_direction.h"
#include <cmath>
namespace hd2::game {
namespace human_ai_cover {
inline HumanVector difference(const HumanVector& a,const HumanVector& b){auto result=a;for(unsigned n=0;n<3;++n)result[n]=static_cast<float>(static_cast<long double>(a[n])-b[n]);return result;}
inline HumanVector position(const MissionTableRecord& p){HumanVector r{};std::memcpy(r.data(),p.prefix.data(),12);return r;}
inline long double square(const HumanVector& v){return (static_cast<long double>(v[2])*v[2]+static_cast<long double>(v[1])*v[1])+static_cast<long double>(v[0])*v[0];}
inline long double length(float square){std::int32_t bits;std::memcpy(&bits,&square,4);if(bits<0x322bcc77)return 0;return std::abs(static_cast<long double>(square)-1)<9.99999993922529e-09f?1:std::sqrt(static_cast<long double>(square));}
//00524A30: QWORD cosine constants are rounded to FLOAT by the original comparisons.
inline std::uint8_t direction(const HumanVector& value){
    float x=value[0],z=value[2];const float magnitude=static_cast<float>(std::sqrt(static_cast<long double>(z)*z+static_cast<long double>(x)*x));
    if(magnitude>9.99999993922529e-09f){const auto scale=1.L/magnitude;x=static_cast<float>(scale*x);z=static_cast<float>(scale*z);}
    if(z>static_cast<float>(.9238795325112867))return 1;
    if(z>static_cast<float>(.3826834323650898))return x<=0?0x80:2;
    if(z<=static_cast<float>(-.3826834323650898)){if(z<=static_cast<float>(-.9238795325112867))return 0x10;return x<=0?0x20:8;}
    return x<=0?0x40:4;
}
inline std::uint8_t blocked(const MissionTableRecord& point,std::uint8_t octant,std::uint8_t posture=7){ //0045ACF0.
    std::uint8_t result=0;if((posture&1)&&(point.reserved17[2]&octant))result|=1;if((posture&2)&&(point.reserved17[0]&octant))result|=2;if((posture&4)&&(point.reserved17[1]&octant))result|=4;return result;
}
inline std::uint8_t blocked_toward(const MissionTableRecord& point,const HumanVector& target,std::uint8_t posture=7){return blocked(point,direction(difference(target,position(point))),posture);} //005534B0.
inline long double square_xyz(const HumanVector& v){return (static_cast<long double>(v[0])*v[0]+static_cast<long double>(v[1])*v[1])+static_cast<long double>(v[2])*v[2];}
inline long double distance_xyz(const HumanVector& a,const HumanVector& b){return length(static_cast<float>(square_xyz(difference(a,b))));}
//00520A30: reverse mutable candidate vector, actual point masks and strict
//nearest/farthest selection. Unmarked/absent points are removed regardless of
//the caller's40000000 flag; the signed distance cap also removes failures.
inline std::uint16_t filter_points(MissionNavigation& navigation,const HumanVector& origin,
        const HumanVector& target,std::vector<std::uint16_t>& points,std::uint32_t flags){
    const float range=static_cast<float>(distance_xyz(origin,target));
    float closest=10000.f,farthest=0.f;std::uint16_t selected=0xffff;
    for(auto n=points.size();n;){const auto i=--n;const auto* point=navigation.record(points[i]);
        const auto remove=[&]{if(i>=points.size())throw std::logic_error("Navigation filter lost its native candidate");points[i]=points.back();points.pop_back();};
        if(!point||!(point->reserved17[0]|point->reserved17[1]|point->reserved17[2])){remove();continue;}
        const auto mask=blocked_toward(*point,target);const auto required=flags&0x700u;
        const bool rejected=(required==0x100u&&!(mask&1u))||(required==0x200u&&!(mask&2u))||
            (required==0x400u&&!(mask&4u))||((flags&0x6000u)==0x2000u&&(mask&2u))||
            ((flags&0x6000u)==0x4000u&&(mask&4u));
        if(rejected){if(flags&0x40000000u)remove();continue;}
        const auto p=position(*point);const auto distance=distance_xyz(flags&1u?origin:target,p);
        if((flags&0x80000000u)&&!(distance<=range)){remove();continue;}
        if(flags&4u){if(distance>farthest){farthest=static_cast<float>(distance);selected=points[i];}}
        else if(distance<closest){closest=static_cast<float>(distance);selected=points[i];}
    }
    return selected;
}
template<class Frames,class Actor>HumanVector actor_position(Frames& frames,Actor& actor){const auto m=frames.actor_world_matrix(actor.base().frame());return {{m[12],m[13],m[14],m[15]}};}
//0055BD40: reverse scoring, strict positive best, integer reciprocal distance.
template<class AI,class Frames>std::uint16_t defensive_point(AI& ai,Frames& frames,const std::vector<std::uint16_t>& points){
    auto* navigation=ai.navigation();if(points.empty()||!navigation)return 0xffff;
    std::vector<std::uint32_t> scores(points.size());
    for(auto n=points.size();n;){const auto i=--n;const auto* point=navigation->record(points[i]);if(!point)throw std::logic_error("Area defensive point requires actual navigation record");
        const auto delta=difference(position(*point),actor_position(frames,*ai.human()));const auto distance=square(delta);
        const auto inverse=distance>0?30.L/distance:30.L;
        if(!std::isfinite(inverse)||inverse>=9223372036854775808.L)throw std::runtime_error("Area defensive score exceeds native integer conversion");
        const auto multiplier=static_cast<std::uint32_t>(static_cast<std::int64_t>(inverse));
        const auto& hostiles=ai.human()->perception.lists()[0];
        for(auto h=hostiles.size();h;){auto* actor=hostiles[--h];const auto bits=blocked_toward(*point,actor_position(frames,*actor));scores[i]+=(std::uint32_t(bits&4u)*multiplier)!=0?3u:bits&2u?2u:bits&1u;}
    }
    std::uint16_t selected=0xffff;std::int32_t best=0;
    for(auto n=points.size();n;){const auto i=--n;std::int32_t score;std::memcpy(&score,&scores[i],4);if(score>best){best=score;selected=points[i];}}
    return selected;
}
//0055C0B0/0055C450: three FLOAT spills, original unusual score weights retained.
template<class AI,class Frames,class Actor>std::uint16_t attack_point(AI& ai,Frames& frames,const std::vector<std::uint16_t>& points,Actor* target,bool hostile){
    auto* navigation=ai.navigation();if(points.empty()||!navigation||!target)return 0xffff;
    std::vector<std::uint32_t> scores(points.size());
    for(auto n=points.size();n;){const auto i=--n;const auto* point=navigation->record(points[i]);if(!point)throw std::logic_error("Area attack point requires actual navigation record");
        const float self_square=static_cast<float>(square(difference(position(*point),actor_position(frames,*ai.human()))));
        const float reciprocal=self_square>1?static_cast<float>(30.L/self_square):30.f;
        const auto target_square=square(difference(position(*point),actor_position(frames,*target)));
        const float weighted=static_cast<float>(target_square*.5f);
        const float total=hostile?static_cast<float>(target_square*.5f+reciprocal):static_cast<float>(static_cast<long double>(weighted)+reciprocal);
        const auto score=[&](auto& actor){const auto mask=blocked_toward(*point,actor_position(frames,actor));return static_cast<long double>(mask&4u)*total!=0?1u:mask&2u?3u:(mask&1u)*2u;};
        if(hostile){const auto& enemies=ai.human()->perception.lists()[0];for(auto h=enemies.size();h;)scores[i]+=score(*enemies[--h]);}
        else scores[i]+=score(*target);
    }
    std::uint16_t selected=0xffff;std::int32_t best=-1;
    for(auto n=points.size();n;){const auto i=--n;std::int32_t score;std::memcpy(&score,&scores[i],4);if(score>best){best=score;selected=points[i];}}
    return selected;
}
} //namespace human_ai_cover
} //namespace hd2::game
#endif
