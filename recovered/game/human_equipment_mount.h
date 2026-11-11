#ifndef HD2_GAME_HUMAN_EQUIPMENT_MOUNT_H
#define HD2_GAME_HUMAN_EQUIPMENT_MOUNT_H

#include "game/human_joint_math.h"
#include "game/human_state_frame_timers.h"

namespace hd2::game {

//00790520 case9. The230 producer owns the retained action-model reference.
//Its animation, placement sweep and final release precede clearing1C/B0.
template<class Frames,class State,class Services>
void retire_human_equipment_mount(Frames& frames,State& state,Services& services){
    const auto model=state.model(1);if(!model)return;
    state.words().set(0x54,std::int32_t{-10000});
    frames.human_model_animation(model,services.equipment_animation_resource(0,291),6,1.f,2);
    frames.human_equipment_tick_model(model,1000);
    const auto human_model=services.equipment_human_model();
    const auto normalized=[](const auto& matrix){ //Native keeps X in x87 while Y/Z normalization spills.
        long double x=matrix[8];float y=matrix[9],z=matrix[10];
        const auto square=(x*x+static_cast<long double>(y)*y)+static_cast<long double>(z)*z;
        const float stored=static_cast<float>(square);
        if(std::abs(square-1)>=1.e-8f){
            if(stored>=1.e-8f){
                const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));
                x*=inverse;y=static_cast<float>(y*inverse);z=static_cast<float>(z*inverse);
            }else if(x!=0)x=x<0?-1.L:1.L;
            else if(z!=0)z=z<0?-1.f:1.f;
            else y=y<0?-1.f:1.f;
        }
        return std::array<long double,3>{{x,y,z}};
    };
    const auto direction=normalized(frames.actor_world_matrix(human_model));
    const float offset_x=static_cast<float>(direction[0]*.4f),offset_z=static_cast<float>(direction[2]*.4f);
    const auto matrix=frames.actor_world_matrix(human_model);
    HumanVector destination{{static_cast<float>(static_cast<long double>(matrix[12])+offset_x),
        static_cast<float>(static_cast<long double>(matrix[13])+.5f),
        static_cast<float>(static_cast<long double>(matrix[14])+offset_z),matrix[15]}};
    const HumanVector origin{{matrix[12],static_cast<float>(static_cast<long double>(matrix[13])+.5f),matrix[14],matrix[15]}};
    const HumanVector delta{{destination[0]-matrix[12],static_cast<float>((static_cast<long double>(destination[1])-matrix[13])+.5f),destination[2]-matrix[14],0}};
    services.exclude_equipment_query();
    struct Restore {
        Services& services;bool armed=true;
        void finish(){armed=false;services.restore_equipment_query();}
        ~Restore(){if(armed)services.restore_equipment_query();}
    } restore{services};
    const auto hit=services.equipment_sweep().sweep(origin,delta,.33f,0xc8002u,0x230000u,false);
    if(hit){
        const auto x=static_cast<long double>(hit->position[0])-matrix[12];
        const auto z=static_cast<long double>(hit->position[2])-matrix[14];
        const float squared=static_cast<float>(z*z+x*x);
        std::uint32_t bits;std::memcpy(&bits,&squared,4);
        long double distance=0;
        if(human_frame_signed_word(bits)>=0x322bcc77)
            distance=std::abs(static_cast<long double>(squared)-1)<1.e-8f?1.L:std::sqrt(static_cast<long double>(squared));
        if(distance>=.5f)destination=origin;
        else {
            const float remaining=static_cast<float>(.5L-distance);
            const auto current=frames.actor_world_matrix(human_model);
            const auto forward=normalized(current);
            for(unsigned axis=0;axis!=2;++axis)
                destination[axis]=static_cast<float>(static_cast<long double>(origin[axis])-static_cast<long double>(forward[axis])*remaining);
            const float z_offset=static_cast<float>(forward[2]*remaining);
            destination[2]=static_cast<float>(static_cast<long double>(origin[2])-z_offset);
            destination[3]=current[11]; //Native copies the reread direction's padding word.
        }
    }
    restore.finish();
    frames.object_position(model,{destination[0],destination[1],destination[2]});
    frames.refresh_dynamic_human_model(human_model);
    frames.update_scene_object(model);frames.release_item_frame(model);
    state.model(1,typename Frames::Frame{});state.words().set(0xb0,11u);
}

} //namespace hd2::game
#endif
