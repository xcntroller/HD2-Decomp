#ifndef HD2_GAME_HUMAN_REACTION_SHAPE_H
#define HD2_GAME_HUMAN_REACTION_SHAPE_H

#include "game/human_retained_clip_motion.h"

namespace hd2::game {
//0074C6E0, AA0 action receiver. Probe actual posture spheres in reverse,
//retaining the native blocker index0 sentinel and lateral rejection policy.
template<class Control,class Look,class Human,class Services,class Frames>
bool check_human_reaction_shape(Control& control,const Look& look,Human& human,Services& services,
        Frames& frames,const HumanVector& requested,unsigned side){
    using namespace human_retained_clip;
    auto& physical=control.physical();auto& state=physical.control_frame_words();
    const auto word=[&](unsigned offset){return state.template get<std::int32_t>(offset);};
    const auto motion=[&](std::uint32_t value){state.set(0x1ac,value);state.set(0x18c,std::uint8_t{1});};
    const auto direction=normalize(requested);motion(2);
    const auto active=word(0x1d8);
    if((active>=8&&active<=14)||(active>=106&&active<=108)){
        auto velocity=normalize(state.template get<Vector>(0x9d0));velocity[1]=.4000000059604645f;
        motion(3);state.set(0x9d0,velocity);return true;
    }
    std::int32_t blocker=0;float retained_angle=0;
    const Vector up{{0,1,0,0}};
    const auto angle=[&](const auto& hit){return frames.car_vector_angle(hit.normal,up);};
    try{
        physical.exclude_query(false);
        const auto count=physical.primary_count();
        for(auto n=count;n>0;){
            const auto index=--n;
            const auto start=physical.posture_spheres().at(static_cast<std::size_t>(index));
            const auto crouched=look.state().template get<std::uint32_t>(0x4c)==1;
            Vector target;
            if(!crouched&&index==count-1)target=add(scale(direction,1.399999976158142f),start);
            else{
                const auto origin=human.base().world_position();const auto delta=subtract(start,origin);
                const float square=static_cast<float>(dot(delta,delta));
                std::int32_t bits;std::memcpy(&bits,&square,4);
                const auto distance=bits<0x322bcc77?0.L:std::abs(static_cast<long double>(square)-1)<9.99999993922529e-09f?1.L:std::sqrt(static_cast<long double>(square));
                const auto factor=crouched?2.0999999046325684f:1.2999999523162842f;
                target=origin;
                for(unsigned axis=0;axis!=3;++axis){
                    const auto product=static_cast<long double>(direction[axis])*factor;
                    target[axis]=static_cast<float>(origin[axis]+product*distance);
                }
            }
            auto hit=services.human_geometry_sweep().sweep(start,subtract(target,start),start[3],0x88002u,0x230000u);
            decltype(hit) extra;
            if(index==1&&count>2){
                if(hit)retained_angle=angle(*hit);
                if(!hit||retained_angle<.7853981852531433f)
                    extra=services.human_geometry_sweep().sweep(start,scale(direction,1.399999976158142f),physical.posture_spheres()[1][3],0x88002u,0x230000u);
            }
            if(hit){
                if(index==1&&crouched)retained_angle=angle(*hit);
                if(retained_angle>=.7853981852531433f&&index)blocker=index;
                else if(extra&&angle(*extra)>=.7853981852531433f)blocker=index;
            }else if(index>0){
                const bool last=index==count-1;
                const auto floor=services.human_geometry_sweep().sweep(target,Vector{{0,last?-2.4000000953674316f:-1.f,0,0}},
                    last?start[3]:start[3]*.949999988079071f,0x88002u,0x230000u);
                if(!floor){if(word(0x1ac)==2)motion(3);}
                else if(last&&((static_cast<long double>(floor->normal[2])+floor->normal[0])*0.f+floor->normal[1])<.7070000171661377f&&
                        floor->position[1]<human.base().world_position()[1])motion(3);
                if(extra&&angle(*extra)>=.7853981852531433f)blocker=index;
            }
        }
    }catch(...){
        const auto primary=std::current_exception();
        try{physical.restore_query(false);}catch(...){frames.record_actor_cleanup_failure(std::current_exception());}
        std::rethrow_exception(primary);
    }
    physical.restore_query(false);
    if(word(0x1ac)==5){
        auto velocity=state.template get<Vector>(0x9d0);velocity[1]=0;
        state.set(0x9d0,scale(normalize(velocity),.5f));
    }else if(word(0x1ac)==3){
        auto velocity=requested;velocity[1]=0;velocity=normalize(velocity);velocity[1]=.4000000059604645f;state.set(0x9d0,velocity);
    }
    if(side&&blocker==0)blocker=1; //Original lateral probes reject even with no blocking sphere.
    if(blocker){auto velocity=state.template get<Vector>(0x9d0);velocity[0]=velocity[1]=velocity[2]=0;state.set(0x9d0,velocity);motion(2);}
    return blocker==0;
}
} //namespace hd2::game
#endif
