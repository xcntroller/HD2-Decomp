#ifndef HD2_GAME_HUMAN_AI_FLOOR_PATH_H
#define HD2_GAME_HUMAN_AI_FLOOR_PATH_H
#include "game/detached_camera.h"
#include "game/human_ai_navigation_jobs.h"
#include <cmath>
namespace hd2::game {
namespace human_ai_floor {
using Position=std::array<float,3>;
inline Position xyz(const HumanVector& p){return {{p[0],p[1],p[2]}};}
inline HumanVector vector(const Position& p){return {{p[0],p[1],p[2],0}};} //Only XYZ crosses the native vector arithmetic boundary.
inline Position subtract(const Position& a,const Position& b){Position r;for(unsigned n=0;n<3;++n)r[n]=static_cast<float>(static_cast<long double>(a[n])-b[n]);return r;}
inline Position add(const Position& a,const Position& b){Position r;for(unsigned n=0;n<3;++n)r[n]=static_cast<float>(static_cast<long double>(a[n])+b[n]);return r;}
inline Position scale(const Position& a,long double value){Position r;for(unsigned n=0;n<3;++n)r[n]=static_cast<float>(a[n]*value);return r;}
inline long double dot(const Position& a,const Position& b){return (static_cast<long double>(a[0])*b[0]+static_cast<long double>(a[2])*b[2])+static_cast<long double>(a[1])*b[1];}
template<class Frames,class Actor>Position position(Frames& frames,Actor& actor){const auto m=frames.actor_world_matrix(actor.base().frame());return {{m[12],m[13],m[14]}};}
//005814D0: reverse ties, actual actor model position, verified00581210 segment math.
template<class Frames,class Actors>auto nearest(Frames& frames,const Actors& actors,const Position& from,const Position& to)->typename Actors::value_type{
    typename Actors::value_type selected=nullptr;float earliest=10;
    for(auto i=actors.size();i;){auto* actor=actors[--i];float fraction;
        const auto square=camera_segment_square(vector(position(frames,*actor)),vector(from),vector(to),fraction);
        if(fraction>=0&&square<.6399999856948853f&&fraction<earliest){selected=actor;earliest=fraction;}
    }
    return selected;
}
template<class Frames,class Actor>bool within_actor_radius(Frames& frames,Actor& actor,const Position& point){ //00581AA0, ordered inclusive test, Z²+X²+Y².
    const auto p=position(frames,actor);const auto x=static_cast<long double>(p[0])-point[0],y=static_cast<long double>(p[1])-point[1],z=static_cast<long double>(p[2])-point[2];
    return (z*z+x*x)+y*y<=.8100000023841858f;
}
//00581580, two geometric candidates; order allows00581B00 to try the nearer root last-to-first.
//Native vector padding is not interpreted or synthesized as a geometric input.
template<class Frames,class Actor,class Ground>std::vector<Position> candidates(Frames& frames,Actor& actor,
        const Position& from,const Position& to,Ground&& ground){
    std::vector<Position> result;float fraction;
    (void)camera_segment_square(vector(position(frames,actor)),vector(from),vector(to),fraction);
    const auto centre=add(scale(subtract(to,from),fraction),from);
    const auto offset=subtract(centre,position(frames,actor));const auto back=subtract(from,centre);
    Position lateral{{static_cast<float>(static_cast<long double>(back[2])-static_cast<long double>(back[1])*0.f),
        static_cast<float>(static_cast<long double>(back[0])*0.f-static_cast<long double>(back[2])*0.f),
        static_cast<float>(static_cast<long double>(back[1])*0.f-back[0])}};
    const float a=static_cast<float>(dot(back,back)),b=static_cast<float>(dot(back,offset));
    const float c=static_cast<float>(dot(offset,offset)-.8100000023841858f);
    const auto discriminant=static_cast<long double>(b)*b-static_cast<long double>(c)*a;
    if(!(discriminant>=0))return result;
    const auto root=(std::sqrt(discriminant)+b)/a;if(!(root>=0))return result;
    //Native computes this point before reusing its temporary; preserve FLOAT operations.
    (void)add(scale(back,root),centre);
    const float lateral_square=static_cast<float>((static_cast<long double>(lateral[2])*lateral[2]+static_cast<long double>(lateral[1])*lateral[1])+static_cast<long double>(lateral[0])*lateral[0]);
    const float along=static_cast<float>(dot(lateral,offset));
    const auto second_discriminant=static_cast<long double>(along)*along-static_cast<long double>(c)*lateral_square;
    if(!(second_discriminant>=0))return result;
    const auto square_root=std::sqrt(second_discriminant);
    const float positive=static_cast<float>((square_root-along)/lateral_square);
    if(!(positive>=0))return result;
    const float negative=static_cast<float>((along+square_root)/-static_cast<long double>(lateral_square));
    const bool shorter_positive=std::abs(positive)<=std::abs(negative);
    const auto first=add(centre,scale(lateral,shorter_positive?negative:positive));
    const auto second=add(centre,scale(lateral,shorter_positive?positive:negative));
    if(ground(first))result.push_back(first);
    if(ground(second))result.push_back(second);
    return result;
}
//00580E30: destructive reverse swap-last filtering of the local actor vector.
template<class AI,class Frames,class Actors,class World>void filter(AI& ai,Frames& frames,Actors& actors,bool reached,World& world){
    auto& jobs=ai.resources().navigation_jobs();if(jobs.empty())return;
    for(auto i=actors.size();i;){const auto index=--i;auto* actor=actors[index];
        bool remove=world.human_floor_actor_suspended(*actor)||world.human_floor_actor_attached(*actor)||!world.human_floor_actor_state(*actor)||world.human_floor_actor_health(*actor)<1;
        if(!remove){
            const auto destination=jobs[reached&&jobs.size()>1?1u:0u]->words.template get<Position>(0);
            const auto origin=position(frames,*ai.human());const auto refreshed=position(frames,*ai.human());const auto neighbour=position(frames,*actor);const auto delta=subtract(neighbour,refreshed);
            const Position route{{static_cast<float>(static_cast<long double>(destination[0])-origin[0]),static_cast<float>(static_cast<long double>(destination[1])-origin[1]),static_cast<float>(static_cast<long double>(destination[2])-origin[2])}};
            const auto facing=(static_cast<long double>(delta[0])*route[0]+static_cast<long double>(delta[1])*route[1])+static_cast<long double>(delta[2])*route[2];
            if(!(facing>=0))remove=true;
            else{
                const float square=static_cast<float>((static_cast<long double>(delta[0])*delta[0]+static_cast<long double>(delta[1])*delta[1])+static_cast<long double>(delta[2])*delta[2]);
                if(std::isnan(square)||square>4.f){
                    const auto velocity=xyz(world.human_floor_actor_velocity(*actor));
                    const auto speed=(static_cast<long double>(velocity[0])*velocity[0]+static_cast<long double>(velocity[1])*velocity[1])+static_cast<long double>(velocity[2])*velocity[2];
                    const auto away=(static_cast<long double>(delta[0])*velocity[0]+static_cast<long double>(velocity[1])*delta[1])+static_cast<long double>(velocity[2])*delta[2];
                    if(speed>=1.0000000168623835e-16f&&away>0)remove=true;
                }
            }
        }
        if(remove){actors[index]=actors.back();actors.pop_back();}
    }
}
//00581B00. A failed later detour retains earlier insertions/position changes.
template<class AI,class Frames,class Actors,class Pool,class Ground>bool detour(AI& ai,Frames& frames,const Actors& actors,Pool& pool,Ground&& ground){
    auto& jobs=ai.resources().navigation_jobs();if(actors.empty()||jobs.empty())return true;
    auto from=position(frames,*ai.human());
    for(std::size_t i=0;i<jobs.size();++i){
        const auto to=jobs[i]->words.template get<Position>(0);auto* actor=nearest(frames,actors,from,to);
        if(actor&&!within_actor_radius(frames,*actor,from)){
            const bool endpoint=within_actor_radius(frames,*actor,to);auto options=candidates(frames,*actor,from,to,ground);
            bool found=false;
            while(!options.empty()){
                const auto selected=options.back();if(nearest(frames,actors,from,selected)){options.pop_back();continue;}
                if(endpoint)jobs[i]->words.set(0,selected);
                else{auto job=pool.acquire(0xffffffffu,selected,0,0,0);jobs.insert(jobs.begin()+i,std::move(job));}
                found=true;break;
            }
            if(!found)return false;
        }
        from=jobs[i]->words.template get<Position>(0);
    }
    return true;
}
} //namespace human_ai_floor
//00581E30, actual B4 command-neighbour identity lookup and SAME F8 leases.
template<class AI,class Frames,class World>void advance_human_ai_floor_path(AI& ai,Frames& frames,World& world,bool reached){
    const auto& identities=ai.command()->lists[0];using Actor=std::remove_pointer_t<decltype(world.human_floor_actor(std::uint32_t{}))>;
    std::vector<Actor*> actors;for(auto i=identities.size();i;)actors.push_back(world.human_floor_actor(identities[--i]));
    if(actors.empty())return;
    human_ai_floor::filter(ai,frames,actors,reached,world);if(actors.empty())return;
    (void)human_ai_floor::detour(ai,frames,actors,world.human_ai_navigation_jobs(),[&](const auto& point){return world.human_floor_detour_ground(human_ai_floor::vector(point));});
}
} //namespace hd2::game
#endif
