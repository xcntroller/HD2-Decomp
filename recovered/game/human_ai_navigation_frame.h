#ifndef HD2_GAME_HUMAN_AI_NAVIGATION_FRAME_H
#define HD2_GAME_HUMAN_AI_NAVIGATION_FRAME_H

#include "game/human_ai_navigation_path.h"
#include "game/human_ai_scene_steering.h"

namespace hd2::game {
namespace human_ai_navigation {
inline long double distance_squared(const Position& a,const Position& b){
    const auto x=static_cast<long double>(a[0])-b[0];
    const auto y=static_cast<long double>(a[1])-b[1];
    const auto z=static_cast<long double>(a[2])-b[2];
    return (x*x+z*z)+y*y;
}
inline Position position_xyz(const HumanVector& value){return {{value[0],value[1],value[2]}};}

inline bool within_job_link(const HumanAINavigationJob& from,const HumanAINavigationJob& to,const Position& position){ //005802A0.
    const auto a=from.words.get<Position>(0x10);
    const auto b=to.words.get<Position>(0x10);
    const auto axis=difference(b,a);
    const auto offset=difference(position,a);
    const auto square=(product(axis[0],axis[0])+product(axis[1],axis[1]))+product(axis[2],axis[2]);
    const float fraction=static_cast<float>(((product(offset[0],axis[0])+product(offset[1],axis[1]))+
        product(offset[2],axis[2]))/square);
    if(!(fraction>0&&fraction<1))return false;
    const float start_radius=from.words.get<float>(0x28);
    const float end_radius=to.words.get<float>(0x28);
    const float radius=static_cast<float>((static_cast<long double>(end_radius)-start_radius)*fraction+start_radius);
    Position delta;
    for(unsigned i=0;i<3;++i)delta[i]=static_cast<float>((a[i]+product(axis[i],fraction))-position[i]);
    return length(square_length(delta))<radius;
}

template<class AI>
MissionTableRecord* path_navigation_point(AI& ai,const HumanVector& origin,bool floor_search){ //0057FF10.
    auto* navigation=ai.navigation();
    if(!navigation)throw std::logic_error("AI path point requires the actual navigation table");
    const auto position=position_xyz(origin);
    auto* point=ai.point();
    if(!point){
        point=navigation->record(navigation->nearest_volume(position,5,-1));
        if(!point&&floor_search)point=navigation->record(navigation->nearest_floor_volume(position,5,-1));
        return point;
    }
    const auto center=navigation->position(navigation->index_of(*point));
    const auto radius=static_cast<unsigned char>(point->prefix[0x13])*.0625f;
    if(distance_squared(position,center)<product(radius,radius))return point;
    const auto& jobs=ai.resources().navigation_jobs();
    if(!jobs.empty()){
        const auto& head=*jobs.front();
        const auto next_center=head.words.template get<Position>(0x10);
        const auto next_radius=head.words.template get<float>(0x28);
        const HumanAINavigationJob* selected=nullptr;
        if(distance_squared(position,next_center)<product(next_radius,next_radius))selected=&head;
        else if(const auto* previous=ai.resources().current_navigation_job();previous&&within_job_link(*previous,head,position)){
            const auto old_metric=distance_squared(position,previous->words.template get<Position>(0x10))-
                previous->words.template get<float>(0x28);
            const auto new_metric=distance_squared(position,next_center)-next_radius;
            selected=old_metric<new_metric?previous:&head; //00580460 subtracts linear radius.
        }
        if(!selected)return point;
        if(auto* record=navigation->record(static_cast<std::uint16_t>(selected->words.get<std::uint32_t>(0x20))))
            return record;
    }
    return navigation->record(navigation->nearest_volume(position,5,-1));
}

template<class AI>
void finish_path(AI& ai,std::uint32_t result){ //0057F6F0.
    auto& resources=ai.resources();
    resources.finish_navigation_jobs();
    auto& query=resources.query();
    query.state.set(0x4c,result);
    query.finish();
}

//0057D5A0. The same progress counter controls the close radius and the
//no-progress failure. B0+30 velocity may complete a point on overshoot.
template<class AI>
bool reach_path_job(AI& ai,const HumanVector& origin){
    auto& resources=ai.resources();
    const auto& jobs=resources.navigation_jobs();
    if(jobs.empty())return false;
    const auto center=jobs.front()->words.template get<Position>(0);
    const auto x=static_cast<long double>(center[0])-origin[0];
    const auto z=static_cast<long double>(center[2])-origin[2];
    const auto distance=length(static_cast<float>(x*x+z*z));
    auto& query=resources.query().state;
    auto stalled=query.template get<std::uint32_t>(0x28);
    if(distance<query.template get<float>(0x2c))query.set(0x2c,static_cast<float>(distance));
    else query.set(0x28,++stalled);
    const auto frames=human_frame_signed_word(stalled);
    if(distance<1.f){
        if(jobs.size()==1)ai.words().set(0x58,static_cast<float>(distance));
        const auto reached=[&]{
            if(frames<=4){
                constexpr std::array<float,5> close{{.1f,.1f,.2f,.3f,.5f}};
                if(distance<close[frames<0?0:frames])return true;
            }
            const auto velocity=ai.words().template get<Position>(0x30);
            if(!(((product(velocity[2],velocity[2])+product(velocity[1],velocity[1]))+
                product(velocity[0],velocity[0]))>=1.e-16f))return false;
            const auto forward=unit(velocity,square_length(velocity));
            auto delta=difference(center,position_xyz(origin));
            delta=unit(delta,square_length(delta));
            return (product(forward[0],delta[0])+product(forward[1],delta[1]))+product(forward[2],delta[2])<=0;
        };
        if(reached()){
            query.set(0x28,0u);
            query.set(0x2c,0x7f7fffffu);
            return true;
        }
    }
    if(frames>=30)finish_path(ai,0xfffffffbu);
    return false;
}

template<class AI,class Movement>
void output_path_inputs(AI& ai,const Movement& movement,const HumanMatrix& matrix,bool floor_search){ //0057E030.
    auto& words=ai.words();
    auto* input=ai.first_frame_inputs();
    if(!input)throw std::logic_error("AI path output requires the actual first frame input buffer");
    const auto flags=words.template get<std::uint32_t>(0x50);
    if(flags&0x80000u){
        input->set(0,1.f);
        words.set(0x5c,0x4000u);
        return;
    }
    auto& resources=ai.resources();
    auto& direction=resources.direction();
    auto& query=resources.query().state;
    const auto origin=HumanVector{{matrix[12],matrix[13],matrix[14],matrix[15]}};
    const auto& jobs=resources.navigation_jobs();
    if(jobs.empty())throw std::logic_error("AI path output requires the actual next path job");
    const auto& job=*jobs.front();
    const auto target=job.words.template get<HumanVector>(0);
    auto aim=target;
    if(movement.flag10){
        for(unsigned i=0;i<3;++i)aim[i]=static_cast<float>(static_cast<long double>(origin[i])+movement.steering[i]);
        aim[3]=direction.state().template get<float>(0x0c); //The original constructed temporary leaves W unwritten.
    }
    const auto result=aim_along_job(direction,origin,aim,floor_search);
    if(movement.flag10){
        const auto& v=movement.steering;
        words.set(0x58,static_cast<float>(length(static_cast<float>((product(v[2],v[2])+product(v[1],v[1]))+
            product(v[0],v[0])))));
    }
    bool slow_strafe=false;
    if(result==0){
        const auto path_flags=query.template get<std::uint32_t>(0x50);
        if(path_flags&0x1000u){
            auto forward=Position{{matrix[8],matrix[9],matrix[10]}};
            forward=unit(forward,square_length(forward));
            forward=unit(forward,square_length(forward));
            const auto actual=direction.state().template get<HumanVector>(0);
            const auto flat=normalize_ai_scene_horizontal(actual[0],actual[2]);
            if((forward[2]*flat[2]+forward[1]*flat[1])+forward[0]*flat[0]<.5f)return;
            query.set(0x50,path_flags&~0x1000u);
        }
        input->set(0,1.f);
    }else{
        input->set(0x30,0.f);
        const auto move=normalize_ai_scene_horizontal(target[0]-origin[0],target[2]-origin[2]);
        const float mx=static_cast<float>(move[0]),my=static_cast<float>(move[1]),mz=static_cast<float>(move[2]);
        const auto look=direction.state().template get<HumanVector>(0x10);
        const auto aim_flat=normalize_ai_scene_horizontal(look[0],look[2]);
        const auto dot=(aim_flat[2]*mz+aim_flat[0]*mx)+aim_flat[1]*my;
        const auto magnitude=static_cast<float>(std::abs(dot));
        const bool right=aim_flat[0]*mz-aim_flat[2]*mx>0;
        const bool forward=static_cast<float>(dot)>0;
        const auto cosine=[](float angle){return std::cos(static_cast<long double>(angle)*.01745329238474369f);}; //0057E020.
        const bool stance=flags&0x18u;
        const bool movement_mode=flags&0x60u;
        const float straight=stance?(movement_mode?12.5f:32.5f):22.5f;
        const float side=stance&&movement_mode?77.5f:!stance&&movement_mode?57.5f:67.5f;
        const bool strafe=magnitude<=cosine(straight);
        const bool walk=!strafe||magnitude>cosine(side);
        HumanVector facing{{0,0,0,look[3]}};
        if(strafe){
            input->set(0xc,right?0.f:1.f);
            input->set(8,right?1.f:0.f);
            facing[0]=right?mz:-mz;
            facing[2]=right?-mx:mx;
            slow_strafe=!walk&&(flags&1u);
        }
        if(walk){
            input->set(0,forward?1.f:0.f);
            input->set(4,forward?0.f:1.f);
            facing[0]+=forward?mx:-mx;
            facing[2]+=forward?mz:-mz;
            if(!strafe&&forward)facing[3]=direction.state().template get<float>(0x0c);
        }
        const auto heading=normalize_ai_scene_horizontal(facing[0],facing[2]);
        for(unsigned i=0;i<3;++i)facing[i]=static_cast<float>(heading[i]);
        direction.set_current(facing);
    }
    apply_human_ai_navigation_stance(ai,movement,*input,slow_strafe);
}
} //namespace human_ai_navigation
} //namespace hd2::game
#endif
