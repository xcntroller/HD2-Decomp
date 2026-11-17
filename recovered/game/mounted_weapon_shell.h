#ifndef HD2_GAME_MOUNTED_WEAPON_SHELL_H
#define HD2_GAME_MOUNTED_WEAPON_SHELL_H
#include "game/human_ai_shot_direction.h"
#include <cmath>
namespace hd2::game {
//004C5080, S marker casing. No bullet, ammunition or new Scene2 actor owner.
template<class Frames,class Services>
void emit_mounted_weapon_shell(Frames& frames,Services& services,typename Frames::Frame barrel,typename Frames::Frame shells){
    if(!barrel)return;
    using Vector=typename Frames::Vector;
    auto world=frames.actor_world_matrix(barrel);Vector position{{world[12],world[13],world[14],world[15]}};
    world=frames.actor_world_matrix(barrel);Vector direction{{world[8],world[9],world[10],world[11]}};
    const float square=static_cast<float>((static_cast<long double>(direction[1])*direction[1]+static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[0])*direction[0]);
    normalize_human_ai_shot(direction,square);
    world=frames.actor_world_matrix(barrel);Vector velocity{{world[4],world[5],world[6],world[7]}};
    const float x=static_cast<float>((static_cast<long double>(frames.volume_random_unit())*.5f+1.f)*velocity[0]);
    float y=static_cast<float>((static_cast<long double>(frames.volume_random_unit())+1.f)*velocity[1]);
    long double z=(static_cast<long double>(frames.volume_random_unit())*.5f+1.f)*velocity[2],extended_x=x;
    const auto length=(extended_x*extended_x+static_cast<long double>(y)*y)+z*z;const float stored=static_cast<float>(length);
    if(!(std::abs(length-1)<9.99999993922529e-09f)){
        if(!(stored<9.99999993922529e-09f)){const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));extended_x*=inverse;y=static_cast<float>(y*inverse);z*=inverse;}
        else if(extended_x!=0)extended_x=extended_x<0?-1.L:1.L;
        else if(z!=0)z=z<0?-1.L:1.L;
        else y=y<0?-1.f:1.f;
    }
    velocity[0]=static_cast<float>(extended_x*3.f);velocity[1]=y*3.f;velocity[2]=static_cast<float>(z*3.f);
    if(!shells)throw std::runtime_error("Mounted casing requires retained GunShells model");
    const auto source=frames.actor_find(shells,"SHELL",1);
    auto visual=frames.mounted_shell_visual(source);const auto frame=visual.get();
    const auto offset=static_cast<long double>(frames.volume_random_unit());
    //Native X/Y products spill to FLOAT; Z remains extended until addition.
    for(unsigned axis=0;axis!=3;++axis){const auto product=offset*velocity[axis];position[axis]=static_cast<float>((axis==2?product:static_cast<long double>(static_cast<float>(product)))+position[axis]);}
    frames.mounted_shell_position(frame,position);frames.mounted_local_direction(frame,direction);frames.object_visible(frame,true);frames.update_scene_object(frame);
    services.mounted_shell_physics(frame,velocity); //SAME module Physics: type0/material12/list1, impulse at origin, scale.05.
}
} //namespace hd2::game
#endif
