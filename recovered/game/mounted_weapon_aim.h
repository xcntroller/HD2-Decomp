#ifndef HD2_GAME_MOUNTED_WEAPON_AIM_H
#define HD2_GAME_MOUNTED_WEAPON_AIM_H
#include "game/mounted_weapon_control.h"
#include "game/human_ai_shot_direction.h"
#include "game/human_state_frame_timers.h"
#include <cmath>
#include <cstring>
namespace hd2::game {
namespace mounted_aim_detail {
using Vector=std::array<float,4>;
inline float real(std::uint32_t value){float result;std::memcpy(&result,&value,4);return result;}
inline std::uint32_t bits(float value){std::uint32_t result;std::memcpy(&result,&value,4);return result;}
inline float planar_length(const Vector& value){return static_cast<float>(std::sqrt(static_cast<long double>(value[0])*value[0]+static_cast<long double>(value[2])*value[2]));}
inline long double yaw(const Vector& value){return std::atan2(-static_cast<long double>(value[0]),static_cast<long double>(value[2]));}
inline long double pitch(const Vector& value){return std::atan2(static_cast<long double>(value[1]),std::sqrt(static_cast<long double>(value[0])*value[0]+static_cast<long double>(value[2])*value[2]));}
inline void normalize(Vector& value,bool z_y_x=false){const float square=static_cast<float>(z_y_x?
    (static_cast<long double>(value[2])*value[2]+static_cast<long double>(value[1])*value[1])+static_cast<long double>(value[0])*value[0]:
    (static_cast<long double>(value[0])*value[0]+static_cast<long double>(value[2])*value[2])+static_cast<long double>(value[1])*value[1]);normalize_human_ai_shot(value,square);}
inline float horizontal_limit(float value,const StaticWeaponControl& c){
    if(c.field134!=1000000.f&&value>c.field134)return c.field134;
    if(c.field130!=1000000.f&&value<c.field130)return c.field130;
    return value;
}
inline float vertical_limit(float value,const StaticWeaponControl& c){return value>c.field128?c.field128:value<c.field12c?c.field12c:value;}
inline float safe_pitch(float value){return value>1.5676548480987549f?1.5676548480987549f:value< -1.5676548480987549f?-1.5676548480987549f:value;}
inline float wrap(float value){constexpr float circle=6.2831854820251465f;if(value<0)value+=circle;else if(value>circle)value-=circle;return value;}
template<class Sounds>void sound(Sounds& sounds,StaticWeaponControl& c,bool moving){
    if(moving){if(!(c.field14c&0x400u))sounds.begin_motion(c.field14c);}
    else if((c.field14c&0x400u)&&human_frame_signed_word(c.fieldd0)>=301)sounds.release_motion(c.field14c);
}
}
//004C3120, actual player input rates and SAME HORT/VERT state, not camera state.
template<class Frames,class Sounds,class Publish>
void advance_mounted_player_aim(Frames& frames,typename Frames::Frame model,typename Frames::Frame horizontal,
        typename Frames::Frame vertical,StaticWeaponControl& c,Sounds& sounds,std::uint32_t elapsed,Publish&& publish){
    using namespace mounted_aim_detail;bool moving=false;const auto milliseconds=static_cast<long double>(human_frame_signed_word(elapsed));
    if(real(c.fieldcc)!=0){
        frames.update_scene_object(horizontal);auto direction=frames.mounted_local_forward(horizontal);const auto old=yaw(direction);
        const auto angle=horizontal_limit(static_cast<float>(old-milliseconds*real(c.fieldcc)*.0010000000474974513f),c);
        if(static_cast<long double>(angle)!=old){
            const auto length=std::sqrt(static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[2])*direction[2]);
            direction[0]=static_cast<float>(-length*std::sin(static_cast<long double>(angle)));direction[2]=static_cast<float>(length*std::cos(static_cast<long double>(angle)));normalize(direction);
            frames.mounted_local_direction(horizontal,direction);frames.update_scene_object(horizontal);moving=true;c.field11c=bits(angle);
        }
        if(c.flagd4&&real(c.fieldcc)!=0){
            const auto old_rate=real(c.fieldcc);std::int32_t wrapped;const auto product=elapsed*5u;std::memcpy(&wrapped,&product,4);
            const auto change=static_cast<float>(wrapped)*.0010000000474974513f;
            const auto rate=old_rate>0?old_rate-change:old_rate+change;c.fieldcc=bits((old_rate>0&&rate<0)||(old_rate<0&&rate>0)?0.f:rate);
        }
    }
    if(real(c.fieldd8)!=0){
        frames.update_scene_object(vertical);auto direction=frames.mounted_local_forward(vertical);const auto old=pitch(direction),heading=yaw(direction);
        auto angle=vertical_limit(static_cast<float>(old-milliseconds*real(c.fieldd8)*.0010000000474974513f),c);
        if(static_cast<long double>(angle)!=old){
            moving=true;angle=safe_pitch(angle);direction[1]=static_cast<float>(std::sin(static_cast<long double>(angle)));
            const auto cosine=std::cos(static_cast<long double>(angle));direction[0]=static_cast<float>(-cosine*std::sin(heading));direction[2]=static_cast<float>(std::cos(heading)*cosine);
            frames.mounted_local_direction(vertical,direction);c.field118=bits(angle);
        }
        if(c.flagdc&&real(c.fieldd8)!=0){
            const auto old_rate=real(c.fieldd8);const auto change=static_cast<float>(static_cast<long double>(static_cast<float>(milliseconds))*real(c.field164)*.004000000189989805f);
            const auto rate=old_rate>0?old_rate-change:old_rate+change;c.fieldd8=bits((old_rate>0&&rate<0)||(old_rate<0&&rate>0)?0.f:rate);
        }
    }
    sound(sounds,c,moving);frames.update_scene_object(model);
    const auto world=frames.actor_world_matrix(vertical);Vector direction{{world[8],world[9],world[10],world[11]}};normalize(direction);publish(direction); //005A84E0, actual SAME B0+54.
}
//004C2580, actual B0 target direction, world angles and bounded local pivots.
template<class Frames,class Sounds>
void advance_mounted_ai_aim(Frames& frames,typename Frames::Frame model,typename Frames::Frame horizontal,
        typename Frames::Frame vertical,StaticWeaponControl& c,Sounds& sounds,std::uint32_t elapsed,const std::array<float,4>& target){
    using namespace mounted_aim_detail;bool moving=false;
    const auto world=frames.actor_world_matrix(vertical);Vector current{{world[8],world[9],world[10],world[11]}};
    //Native keeps X/Y extended after this normalize; angles are FLOAT snapshots.
    std::array<long double,3> extended{{current[0],current[1],current[2]}};
    const auto length=(extended[2]*extended[2]+extended[1]*extended[1])+extended[0]*extended[0];const float stored=static_cast<float>(length);
    if(!(std::abs(length-1)<9.99999993922529e-09f)){
        if(!(stored<9.99999993922529e-09f)){const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));extended[0]*=inverse;extended[1]*=inverse;extended[2]=static_cast<float>(extended[2]*inverse);}
        else {const unsigned axis=extended[0]!=0?0u:extended[2]!=0?2u:1u;extended[axis]=extended[axis]<0?-1.L:1.L;}
    }
    c.field14c&=~0xfu;
    const auto target_yaw=wrap(static_cast<float>(yaw(target))),current_yaw=wrap(static_cast<float>(std::atan2(-extended[0],extended[2])));
    const auto target_pitch=static_cast<float>(pitch(target)),current_pitch=static_cast<float>(std::atan2(extended[1],std::sqrt(extended[2]*extended[2]+extended[0]*extended[0])));
    const auto milliseconds=static_cast<long double>(human_frame_signed_word(elapsed));
    const float horizontal_difference=std::abs(current_yaw-target_yaw);
    if(horizontal_difference>.001){
        frames.update_scene_object(horizontal);auto direction=frames.mounted_local_forward(horizontal);
        auto step=milliseconds*real(c.field160)*.0010000000474974513f;if(horizontal_difference<step)step=horizontal_difference;
        if(wrap(target_yaw-current_yaw)<3.1415927410125732f)step=-step;
        const auto old=static_cast<float>(yaw(direction));const auto angle=horizontal_limit(static_cast<float>(yaw(direction)-step),c);
        if(std::abs(angle-old)>.001){
            c.field14c|=angle-old<=0?2u:1u;const auto length_xy=planar_length(direction);
            direction[0]=static_cast<float>(-std::sin(static_cast<long double>(angle))*length_xy);direction[2]=static_cast<float>(std::cos(static_cast<long double>(angle))*length_xy);normalize(direction,true);
            frames.mounted_local_direction(horizontal,direction);frames.update_scene_object(horizontal);moving=true;c.field11c=bits(angle);
        }
    }
    const float vertical_difference=std::abs(current_pitch-target_pitch);
    if(vertical_difference>.001){
        frames.update_scene_object(vertical);auto direction=frames.mounted_local_forward(vertical);
        auto step=milliseconds*real(c.field164)*.0010000000474974513f;if(vertical_difference<step)step=vertical_difference;if(current_pitch<target_pitch)step=-step;
        const auto old=static_cast<float>(pitch(direction));auto angle=vertical_limit(static_cast<float>(pitch(direction)-step),c);
        if(std::abs(angle-old)>.001){
            c.field14c|=angle-old<=0?8u:4u;moving=true;angle=safe_pitch(angle);
            direction[0]=0;direction[1]=static_cast<float>(std::sin(static_cast<long double>(angle)));direction[2]=static_cast<float>(std::cos(static_cast<long double>(angle)));normalize(direction,true);
            frames.mounted_local_direction(vertical,direction);frames.update_scene_object(vertical);c.field118=bits(angle);
        }
    }
    c.fieldd0=c.field14c&0xfu?0u:c.fieldd0+elapsed;sound(sounds,c,moving);frames.update_scene_object(model);
}
} //namespace hd2::game
#endif
