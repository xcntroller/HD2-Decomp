#ifndef HD2_GAME_HUMAN_LOOK_POSE_H
#define HD2_GAME_HUMAN_LOOK_POSE_H

#include "game/human_look_state.h"
#include "game/human_ai_shot_direction.h"
#include "game/human_animation_requests.h"
#include "game/human_state_frame_timers.h"
#include <optional>

namespace hd2::game {

struct HumanLookPose {
    HumanVector direction;
    float influence;
};

//007ACCC0 repeats the actual matrix query for each projected component and
//writes XYZ only. The retained vector's W word keeps its previous value.
template<class Frames>
void project_human_look_direction(Frames& frames,typename Frames::Frame frame,
        const HumanVector& direction,HumanVector& output){
    for(unsigned row=0;row!=3;++row){
        const auto matrix=frames.actor_world_matrix(frame);
        output[row]=static_cast<float>((static_cast<long double>(direction[1])*matrix[row*4+1]+
            static_cast<long double>(direction[2])*matrix[row*4+2])+static_cast<long double>(direction[0])*matrix[row*4]);
    }
}

inline void limit_human_look_forward(HumanVector& direction,float minimum,float radius){
    for(unsigned axis=0;axis!=3;++axis){
        if(direction[axis]>1)direction[axis]=1;
        else if(direction[axis]<-1)direction[axis]=-1;
    }
    if(!(direction[2]<minimum))return;
    const auto length=std::sqrt(1.L-static_cast<long double>(direction[2])*direction[2]);
    if(length>9.99999993922529e-09f){
        const auto scale=static_cast<long double>(radius)/length;
        direction[0]=static_cast<float>(direction[0]*scale);
        direction[1]=static_cast<float>(direction[1]*scale);
    }else{direction[0]=radius;direction[1]=0;}
    direction[2]=minimum;
}

//007ACCC0 uses the SAME6570 command flags and6580..65D4 state as007AC780/
//007ACA30. The result is applied by the containing geometry through007ADA10.
template<class Frames,class Random,class Movement>
std::optional<HumanLookPose> advance_human_look_pose(Frames& frames,Random& random,
        HumanLookState& look,typename Frames::Frame model,typename Frames::Frame head,
        typename Frames::Frame torso,std::uint32_t mode,const Movement& movement,std::uint32_t elapsed){
    if(!(mode&2u))return std::nullopt;
    const auto original_flags=look.flags;
    const bool returning=(original_flags&0x80000000u)!=0;
    const bool target=(original_flags&1u)!=0;
    const auto head_direction=[&](HumanVector& output){
        const auto matrix=frames.actor_world_matrix(head);
        project_human_look_direction(frames,model,HumanVector{{matrix[8],matrix[9],matrix[10],matrix[11]}},output);
    };
    if(target){
        look.time65c0-=elapsed;
        if(returning){
            if(human_frame_signed_word(look.time65c0)<0){look.time65c0=0;look.flags=0;}
            head_direction(look.scan);
        }else{
            if(human_frame_signed_word(look.time65c0)<0){
                look.time65c0=0;look.time65cc-=elapsed;
                if(human_frame_signed_word(look.time65cc)<0){
                    look.flags=original_flags|0x80000000u;look.time65c0=look.time65c8;
                    look.current=look.baseline;
                }
            }
            const auto matrix=frames.actor_world_matrix(head);
            auto direction=look.target;
            for(unsigned axis=0;axis!=3;++axis)direction[axis]-=matrix[12+axis];
            normalize_human_ai_shot(direction,(static_cast<long double>(direction[2])*direction[2]+
                static_cast<long double>(direction[1])*direction[1])+static_cast<long double>(direction[0])*direction[0]);
            project_human_look_direction(frames,model,direction,look.scan);
        }
    }else if(original_flags&2u){
        if(returning){
            head_direction(look.scan);
            look.time65c0-=elapsed;
            if(human_frame_signed_word(look.time65c0)<0){
                look.time65c0=0;
                if(look.flags&0x80u)look.flags=0;
                else{
                    look.time65c4-=elapsed;
                    if(human_frame_signed_word(look.time65c4)<0){
                        look.flags&=0x7fffffffu;look.begin_scan(random);
                    }
                }
            }
        }else{
            head_direction(look.current);
            look.time65c0-=elapsed;
            if(human_frame_signed_word(look.time65c0)<0){
                look.time65c0=0;look.time65cc-=elapsed;
                if(human_frame_signed_word(look.time65cc)<0){
                    look.current=look.scan;look.flags|=0x80000000u;look.time65c0=look.time65c8;
                }
            }
        }
    }else{
        if(!(movement.field6304&0x35u))return std::nullopt;
        auto direction=movement.direction6360;
        project_human_look_direction(frames,torso,movement.direction6360,direction);
        limit_human_look_forward(direction,.1736f,.9848f);
        return HumanLookPose{frames.collision_sweep_rotate(direction,frames.actor_world_matrix(torso)),
            human_animation_word_float(movement.field6318)};
    }
    //007E8CE0 is the CRT power helper. Its x87 base is stored DOUBLE, and the
    //exponent is the retained FLOAT. The delegated CRT receives no body credit.
    const double ratio=static_cast<double>(static_cast<long double>(human_frame_signed_word(look.time65c0))/
        human_frame_signed_word(look.time65c8));
    const auto progress=std::pow(ratio,static_cast<double>(look.speed65d0));
    look.progress65d4=static_cast<float>(progress);
    if(target)look.baseline=interpolate_human_directions(look.scan,look.current,look.progress65d4);
    else look.baseline=interpolate_human_directions(look.current,look.scan,static_cast<float>(1.L-progress));
    const auto world=frames.collision_sweep_rotate(look.baseline,frames.actor_world_matrix(model));
    auto direction=world;
    project_human_look_direction(frames,torso,world,direction);
    limit_human_look_forward(direction,target?.707f:.1736f,target?.707f:.9848f);
    if(target&&direction[1]<-.342f){
        const auto length=std::sqrt(1.L-static_cast<long double>(direction[1])*direction[1]);
        if(length>9.99999993922529e-09f){
            const auto scale=.9396f/length;
            direction[0]=static_cast<float>(direction[0]*scale);
            direction[2]=static_cast<float>(direction[2]*scale);
        }else{direction[0]=0;direction[2]=.9396f;}
        direction[1]=-.342f;
    }
    return HumanLookPose{frames.collision_sweep_rotate(direction,frames.actor_world_matrix(torso)),1.f};
}

} //namespace hd2::game
#endif
