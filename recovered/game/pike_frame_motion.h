#ifndef HD2_GAME_PIKE_FRAME_MOTION_H
#define HD2_GAME_PIKE_FRAME_MOTION_H
#include "game/human_joint_math.h"
#include "frontend/spatial_sound.h"
#include <array>
namespace hd2::game {
//005B8F00's client quaternion product;00454660 consumes Z/Y/X extended dot.
inline HumanVector pike_rotation_product(const HumanVector& rotation,const HumanVector& saved){
    HumanVector result{};
    for(unsigned a=0;a!=3;++a){const auto b=(a+1)%3,c=(a+2)%3;
        const float cross=static_cast<float>(static_cast<long double>(rotation[b])*saved[c]-static_cast<long double>(rotation[c])*saved[b]);
        const float left=static_cast<float>(static_cast<long double>(saved[a])*rotation[3]);
        const float right=static_cast<float>(static_cast<long double>(rotation[a])*saved[3]);
        const float sum=left+right;result[a]=sum+cross;
    }
    const float scalar=rotation[3]*saved[3];result[3]=static_cast<float>(static_cast<long double>(scalar)-
        ((static_cast<long double>(rotation[2])*saved[2]+static_cast<long double>(rotation[1])*saved[1])+static_cast<long double>(rotation[0])*saved[0]));
    return result;
}
//005B8F00: apply tentative pose, exclude SAME collider, test12 actual box edges
//plus diagonals1→6/2→5, clear exclusion BEFORE restoring rejected quaternion.
template<class Frames,class Descriptor,class World>
bool rotate_pike_model(Frames& frames,typename Frames::Frame frame,const HumanVector& initial,
        const HumanVector& axis,float angle,Descriptor& descriptor,World& world){
    const auto previous=frames.actor_rotation(frame);
    const auto rotation=frames.car_axis_quaternion(axis,angle);
    frames.car_steering_rotation(frame,pike_rotation_product(rotation,initial));frames.update_scene_object(frame);
    auto& exclusions=world.rigid_exclusions();exclusions.single(&descriptor);
    bool blocked=false;
    try{
        const auto bounds=frames.pike_driver_bounds(frame);std::array<HumanVector,8> points{};
        constexpr float inset=.009999999776482582f;
        for(unsigned x=0;x!=2;++x)for(unsigned y=0;y!=2;++y)for(unsigned z=0;z!=2;++z){auto& p=points[x*4+y*2+z];
            p[0]=x?bounds.minimum[0]+inset:bounds.maximum[0]-inset;
            p[1]=y?bounds.minimum[1]+inset:bounds.maximum[1]-inset;
            p[2]=z?bounds.maximum[2]-inset:bounds.minimum[2]+inset;
        }
        const auto matrix=frames.actor_world_matrix(frame);for(auto n=points.size();n;){--n;points[n]=frames.car_transform(points[n],matrix);}
        constexpr std::array<std::array<unsigned,2>,14> edges{{{{2,3}},{{3,7}},{{7,6}},{{6,2}},{{0,2}},{{1,3}},{{5,7}},{{4,6}},{{0,1}},{{1,5}},{{5,4}},{{4,0}},{{1,6}},{{2,5}}}};
        for(const auto& edge:edges){HumanVector displacement{};
            for(unsigned a=0;a!=3;++a)displacement[a]=points[edge[1]][a]-points[edge[0]][a];
            if(world.rigid_ray(points[edge[0]],displacement,2,0)){blocked=true;break;}
        }
    }catch(...){exclusions.clear();throw;}
    exclusions.clear();
    if(blocked){frames.car_steering_rotation(frame,previous);frames.update_scene_object(frame);}
    return blocked;
}
//005B9340/005B9470, unsigned elapsed FILD64, extended endpoint comparison.
//Authority skips mode2; presentation performs timer subtraction even stationary.
template<class Pike,class World>void advance_pike_frame(Pike& pike,bool authority,std::uint32_t elapsed,World& world){
    auto& state=pike.properties.field184;
    if(authority&&(state==0||state==1||(world.mode_flags()&2u)))return;
    if(pike.field188){pike.field188-=elapsed;std::int32_t remaining;std::memcpy(&remaining,&pike.field188,4);if(remaining>0)return;pike.field188=0;}
    const auto previous=pike.field1a0;
    if(!authority&&state!=2&&state!=3)return;
    const auto speed=frontend::sound_float_word(pike.properties.field190),limit=frontend::sound_float_word(pike.properties.field18c);
    const auto delta=static_cast<long double>(elapsed)*.0010000000474974513f*speed;
    if(state==2){const auto next=previous+delta;pike.field1a0=static_cast<float>(next);
        if(next>limit){state=1;pike.field1a0=limit;if(!authority)pike.finish_motion_sound(1,world);}
    }else if(state==3){const auto next=previous-delta;pike.field1a0=static_cast<float>(next);
        if(next<0){state=0;pike.field1a0=0;if(!authority)pike.finish_motion_sound(0,world);}
    }
    if(pike.rotate_model(world)){pike.field188=200;pike.field1a0=previous;}
    if(!authority)pike.refresh_motion_history();
    pike.refresh_collision(world.pike_collisions(),false);
}
} //namespace hd2::game
#endif
