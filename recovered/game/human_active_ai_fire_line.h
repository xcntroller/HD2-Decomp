#ifndef HD2_GAME_HUMAN_ACTIVE_AI_FIRE_LINE_H
#define HD2_GAME_HUMAN_ACTIVE_AI_FIRE_LINE_H
#include "game/human_ai_shot_direction.h"
#include "game/human_state_frame_timers.h"
#include <limits>
namespace hd2::game {
//0052F1F0, final Active008109F8+114. Unlike Guard00558E50, use the SAME
//Players roster FORWARD with fresh count reads and DOUBLE1.15 sphere margin.
//The native geometry is an infinite line;200 is its scale, not a range clamp.
template<class State,class Frames,class World>bool human_active_ai_line_clear(State& state,const HumanVector& input,Frames& frames,World& world){
    constexpr float epsilon=9.99999993922529e-09f;
    auto unit=input;HumanVector line{};
    for(unsigned n=0;n<3;++n)line[n]=static_cast<float>(static_cast<long double>(input[n])*200.f);
    normalize_human_ai_shot(unit,(static_cast<long double>(unit[1])*unit[1]+static_cast<long double>(unit[2])*unit[2])+static_cast<long double>(unit[0])*unit[0]);
    auto* human=state.human();if(!human||!human->head_marker)throw std::runtime_error("Active fire line requires actual Human260 head frame");
    const auto matrix=frames.actor_world_matrix(human->head_marker);const HumanVector origin{{matrix[12],matrix[13],matrix[14],matrix[15]}};
    for(std::size_t n=0;n<world.human_roster().size();++n){
        if(n>static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()))throw std::runtime_error("Active fire roster exceeds native signed index");
        auto* other=world.human_roster().at(static_cast<std::int32_t>(n));if(!other)throw std::runtime_error("Active fire line reaches native NULL roster Human");
        if(human_frame_signed_word(other->properties.stats_ec[0])<=0||other==state.human())continue;
        const auto current=frames.actor_world_matrix(other->base().frame());
        long double x=static_cast<float>(static_cast<long double>(current[12])-origin[0]),z=static_cast<float>(static_cast<long double>(current[14])-origin[2]);float y=0;
        const auto square=z*z+x*x;const float stored=static_cast<float>(square);
        if(!(std::abs(square-1)<epsilon)){
            if(!(stored<epsilon)){const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));x*=inverse;z*=inverse;y=static_cast<float>(0.L*inverse);}
            else if(x!=0)x=x<0?-1:1;
            else if(z!=0)z=z<0?-1:1;
            else y=1;
        }
        if(!((static_cast<long double>(unit[2])*z+static_cast<long double>(unit[1])*y)+static_cast<long double>(unit[0])*x>0))continue;
        std::int32_t count=0;const auto* spheres=world.human_ai_friendly_spheres(*other,count);
        if(count<0||count>4||(!spheres&&count))throw std::runtime_error("Active fire line requires actual DD0 primary spheres");
        const float length=static_cast<float>((static_cast<long double>(line[2])*line[2]+static_cast<long double>(line[1])*line[1])+static_cast<long double>(line[0])*line[0]);
        while(count){const auto& sphere=spheres[--count];long double distance=-1.0000000272564224e16f;
            if(!(length<epsilon)){
                const auto dx=static_cast<long double>(origin[0])-sphere[0],dy=static_cast<long double>(origin[1])-sphere[1],dz=static_cast<long double>(origin[2])-sphere[2];const float saved_z=static_cast<float>(dz);
                const auto parameter=((dz*line[2]+dy*line[1])+dx*line[0])/length;
                const float projected_y=static_cast<float>(static_cast<long double>(line[1])*parameter),projected_z=static_cast<float>(static_cast<long double>(line[2])*parameter);
                const float rx=static_cast<float>(dx-static_cast<long double>(line[0])*parameter);const auto ry=dy-projected_y,rz=static_cast<long double>(saved_z)-projected_z;
                const float residual=static_cast<float>((rz*rz+ry*ry)+static_cast<long double>(rx)*rx);std::uint32_t bits;std::memcpy(&bits,&residual,4);
                distance=human_frame_signed_word(bits)<0x322bcc77?0.L:std::abs(static_cast<long double>(residual)-1)<epsilon?1.L:std::sqrt(static_cast<long double>(residual));
            }
            if(distance<static_cast<long double>(sphere[3])*1.15)return false; //QWORD00810BA0, no FLOAT margin spill.
        }
    }
    return true;
}
}
#endif
