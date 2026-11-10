#ifndef HD2_GAME_HUMAN_AI_NAVIGATION_PATH_H
#define HD2_GAME_HUMAN_AI_NAVIGATION_PATH_H

#include "game/human_ai_navigation_jobs.h"
#include "game/human_state_frame_timers.h"
#include "game/mission_navigation.h"

namespace hd2::game {
namespace human_ai_navigation {
using Position=std::array<float,3>;
using Jobs=std::vector<HumanAINavigationJobPool::Lease>;

inline Position difference(const Position& a,const Position& b){
    return {{a[0]-b[0],a[1]-b[1],a[2]-b[2]}};
}
inline long double product(float a,float b){return static_cast<long double>(a)*b;}
inline float square_length(const Position& value){
    return static_cast<float>((product(value[0],value[0])+product(value[1],value[1]))+product(value[2],value[2]));
}
inline long double length(float square){
    std::uint32_t bits;
    std::memcpy(&bits,&square,sizeof(bits));
    if(human_frame_signed_word(bits)<0x322bcc77)return 0;
    if(std::abs(static_cast<long double>(square)-1)<1.e-8f)return 1;
    return std::sqrt(static_cast<long double>(square));
}
inline Position unit(Position value,float square){
    if(std::abs(static_cast<long double>(square)-1)>=1.e-8f){
        if(square>=1.e-8f){
            const auto inverse=1.L/std::sqrt(static_cast<long double>(square));
            for(auto& axis:value)axis=static_cast<float>(axis*inverse);
        }else{
            const auto axis=value[0]!=0?0u:value[2]!=0?2u:1u;
            value[axis]=value[axis]<0?-1.f:1.f;
        }
    }
    return value;
}

inline void adjust_tail(HumanAINavigationJob& job,const HumanVector& destination){ //0057A8E0.
    const Position target{{destination[0],destination[1],destination[2]}};
    const auto point=job.words.get<Position>(0);
    auto offset=difference(target,point);
    const bool floor=job.words.get<std::uint8_t>(0x26)==15;
    if(floor)offset[1]=0;
    const auto square=static_cast<float>((product(offset[1],offset[1])+product(offset[0],offset[0]))+product(offset[2],offset[2]));
    const auto radius=job.words.get<float>(0x28);
    if(length(square)<=radius){job.words.set(0,destination);return;}
    offset=unit(offset,square);
    Position adjusted;
    for(unsigned axis=0;axis<3;++axis)adjusted[axis]=static_cast<float>(point[axis]+product(offset[axis],radius));
    if(floor)adjusted[1]=target[1];
    job.words.set(0,adjusted);
}

//0057C760. Runs of kind15 nodes use X/Z to interpolate height. Each adjusted
//node becomes the preceding height and remaining distance for the next node.
inline void smooth_floor(Jobs& jobs,const Position& origin,const Position& destination){
    const auto floor=[](const HumanAINavigationJob& job){
        return job.words.get<std::uint8_t>(0x26)==15&&job.words.get<std::uint16_t>(0x24)!=0x100;
    };
    const auto distance=[](const Position& a,const Position& b){
        const auto x=static_cast<long double>(a[0])-b[0];
        const auto z=static_cast<long double>(a[2])-b[2];
        return length(static_cast<float>(x*x+z*z));
    };
    std::size_t first=jobs.size();
    Position start{};
    for(std::size_t i=0;i<jobs.size();++i){
        if(first==jobs.size()){
            if(floor(*jobs[i])){
                first=i;
                start=i?jobs[i-1]->words.get<Position>(0):origin;
            }
            continue;
        }
        const bool continues=floor(*jobs[i]);
        if(continues&&i+1<jobs.size())continue;
        const auto end=continues?destination:jobs[i]->words.get<Position>(0);
        const auto last=continues?i:i-1;
        const auto span=distance(start,end);
        if(span>.1f){
            auto remaining=static_cast<float>(span);
            auto previous_y=start[1];
            for(auto j=first;j<=last;++j){
                auto& job=jobs[j]->words;
                const auto separation=distance(job.get<Position>(0),end);
                const auto y=static_cast<float>((static_cast<long double>(previous_y)-end[1])/remaining*separation+end[1]);
                const auto minimum=static_cast<float>(static_cast<long double>(job.get<float>(0x14))+1);
                previous_y=minimum<y?y:minimum;
                job.set(4,previous_y);
                remaining=static_cast<float>(separation);
            }
        }
        first=jobs.size();
    }
}

//0057BDC0/0057C2B0. Project each eligible center onto the line from the last
//adjusted job to the next. The table-point route excludes the tail here.
inline void smooth(Jobs& jobs,const HumanVector& origin,const HumanVector& destination,bool point_route){
    const Position start{{origin[0],origin[1],origin[2]}};
    const Position target=point_route?jobs.back()->words.get<Position>(0):
        Position{{destination[0],destination[1],destination[2]}};
    auto previous=start;
    const auto count=jobs.size()-(point_route?1u:0u);
    for(std::size_t i=0;i<count;++i){
        auto& job=jobs[i]->words;
        const auto center=job.get<Position>(0);
        const auto radius=job.get<float>(0x28);
        previous=([&]{
            if(radius<=.3f||job.get<std::uint8_t>(0x26)==1)return center;
            const auto next=i+1<jobs.size()?jobs[i+1]->words.get<Position>(0):target;
            const auto line=difference(next,previous);
            const auto reverse=difference(previous,next);
            const auto x=static_cast<long double>(center[0])-previous[0];
            const auto y=static_cast<long double>(center[1])-previous[1];
            const auto z=static_cast<long double>(center[2])-previous[2];
            const auto divisor=(product(reverse[0],reverse[0])+product(reverse[1],reverse[1]))+product(reverse[2],reverse[2]);
            const float fraction=static_cast<float>(((z*line[2]+y*line[1])+x*line[0])/divisor);
            Position projected;
            for(unsigned axis=0;axis<3;++axis){
                const auto shift=static_cast<float>(product(line[axis],fraction));
                projected[axis]=static_cast<float>(static_cast<long double>(previous[axis])+shift);
            }
            auto offset=difference(projected,center);
            const auto square=square_length(offset);
            if(length(square)<=radius)return projected;
            offset=unit(offset,square);
            const float limit=point_route?static_cast<float>(static_cast<long double>(radius)-.1f):radius;
            for(unsigned axis=0;axis<3;++axis){
                const auto shift=product(offset[axis],limit);
                projected[axis]=static_cast<float>(center[axis]+(point_route?shift:static_cast<float>(shift)));
            }
            return projected;
        })();
        //00404260 initializes XYZ only. Keep the job's existing W instead of
        //copying the original temporary's unwritten stack word.
        job.set(0,previous);
    }
    smooth_floor(jobs,start,target);
}

inline void append_jobs(Jobs& jobs,HumanAINavigationJobPool& pool,const MissionNavigation& navigation,
                        const std::vector<std::uint16_t>& route){
    jobs.reserve(route.size());
    for(auto index:route){
        const auto& point=*navigation.record(index);
        jobs.push_back(pool.acquire(index,navigation.position(index),static_cast<std::uint16_t>(point.flags&0xfb40u),
            point.byte16,static_cast<unsigned char>(point.prefix[0x13])*.0625f));
    }
}

template<class Direction>
int aim_along_job(Direction& direction,const HumanVector& origin,HumanVector target,bool floor_search){ //00571730.
    Position delta{{target[0]-origin[0],floor_search?target[1]-origin[1]:0.f,target[2]-origin[2]}};
    const auto square=static_cast<float>((product(delta[1],delta[1])+product(delta[0],delta[0]))+product(delta[2],delta[2]));
    delta=unit(delta,square);
    std::copy(delta.begin(),delta.end(),target.begin());
    const auto result=direction.set_current(target);
    if(floor_search)direction.set_target(target);
    return result;
}
} //namespace human_ai_navigation

//0057AAF0 and 0057B2E0 share the same table and process job pool. This is the
//ordinary point/coordinate builder underneath 0057BD00, before path activation.
template<class AI>
bool build_human_ai_table_path(AI& ai,HumanAINavigationJobPool& pool,const HumanVector& origin,bool floor_search){
    using namespace human_ai_navigation;
    auto* navigation=ai.navigation();
    if(!navigation)throw std::logic_error("AI route requires the actual navigation table");
    auto& resources=ai.resources();
    auto& query=resources.query().state;
    const auto flags=query.template get<std::uint32_t>(0x50);
    const auto fail=[&](std::uint32_t result){query.set(0x4c,result);return false;};
    const auto index=query.template get<std::uint16_t>(0x20);
    const bool point_route=index!=0xffff;
    if(point_route&&!navigation->record(index))return fail(0xfffffff8u);
    const auto destination=query.template get<HumanVector>(0x10);
    const auto b0_flags=ai.words().template get<std::uint32_t>(0x50);
    const bool interacting=b0_flags&0x100000u;
    auto start=interacting?query.template get<std::uint16_t>(0x4a):std::uint16_t{0xffff};
    const Position from{{origin[0],origin[1],origin[2]}},to{{destination[0],destination[1],destination[2]}};
    if(start==0xffff)start=navigation->nearest_volume(from,5,-1);
    auto end=point_route?index:navigation->nearest_volume(to,5,-1);
    if(floor_search){
        if(start==0xffff)start=navigation->nearest_floor_volume(from,5,-1);
        if(!point_route&&end==0xffff)end=navigation->nearest_floor_volume(to,5,-1);
    }
    resources.release_queued_navigation_jobs();
    auto& jobs=resources.navigation_jobs();
    const auto kind=ai.words().template get<std::uint32_t>(0x48);
    const auto mask=kind==4||kind==8?0x100u:0u;
    std::vector<std::uint16_t> route;
    if(!navigation->find_route(start,end,mask,route)){
        if(!point_route&&(b0_flags&0x40000u)){
            jobs.push_back(pool.acquire(0xffffffffu,destination,0,0,0));
            return true;
        }
        if(start==0xffff)return fail(0xfffffff7u);
        if(end==0xffff)return fail(0xfffffff8u);
        return fail(0xfffffff6u);
    }

    append_jobs(jobs,pool,*navigation,route);
    if(!point_route)adjust_tail(*jobs.back(),destination);
    if(interacting&&jobs.size()>1&&jobs[1]->words.template get<std::uint32_t>(0x20)==query.template get<std::uint16_t>(0x48))
        jobs.erase(jobs.begin());
    if(point_route&&(flags&2u))adjust_tail(*jobs.back(),destination);
    smooth(jobs,origin,destination,point_route);
    if(jobs.size()>1&&(!point_route||!interacting)){
        const auto first=jobs[0]->words.template get<Position>(0);
        const auto second=jobs[1]->words.template get<Position>(0);
        const auto dx=(static_cast<long double>(first[0])-from[0])*(static_cast<long double>(second[0])-from[0]);
        const auto dz=(static_cast<long double>(first[2])-from[2])*(static_cast<long double>(second[2])-from[2]);
        const auto dy=(static_cast<long double>(first[1])-from[1])*(static_cast<long double>(second[1])-from[1]);
        if((point_route?dx+dz:(dx+dy)+dz)<0)jobs.front()->words.set(0,origin);
    }
    if(point_route)query.set(0x50,flags&~2u); //0057BD00 only after point-route success.
    return true;
}

//0057B740 follows the Human's actual hostile perception list at268. It uses
//the escape search's distance penalty and the point-route smoothing.
template<class AI,class Frames>
bool build_human_ai_escape_path(AI& ai,Frames& frames,HumanAINavigationJobPool& pool,const HumanVector& origin){
    using namespace human_ai_navigation;
    const auto kind=ai.words().template get<std::uint32_t>(0x48);
    if(kind!=4&&kind!=8)return false;
    const auto& threats=ai.human()->perception.lists()[0];
    auto* navigation=ai.navigation();
    if(threats.empty()||!navigation)return false;
    auto& resources=ai.resources();
    auto& query=resources.query().state;
    const auto fail=[&](std::uint32_t result){query.set(0x4c,result);return false;};
    const bool interacting=ai.words().template get<std::uint32_t>(0x50)&0x100000u;
    auto start=std::uint16_t{0xffff};
    const Position from{{origin[0],origin[1],origin[2]}};
    if(const auto* point=ai.point()){
        const auto index=navigation->index_of(*point);
        const auto xyz=navigation->position(index);
        const auto radius=static_cast<unsigned char>(point->prefix[0x13])*.0625f;
        const auto x=static_cast<long double>(from[0])-xyz[0];
        const auto y=static_cast<long double>(from[1])-xyz[1];
        const auto z=static_cast<long double>(from[2])-xyz[2];
        if((x*x+z*z)+y*y<product(radius,radius))start=index;
    }
    if(start==0xffff){
        if(interacting)start=query.template get<std::uint16_t>(0x4a);
        if(start==0xffff)start=navigation->nearest_volume(from,5,-1);
    }
    resources.release_queued_navigation_jobs();
    std::vector<Position> targets;
    targets.reserve(threats.size());
    for(const auto* threat:threats){
        if(!threat||!threat->base().frame())throw std::logic_error("AI escape route requires actual threat models");
        const auto world=frames.actor_world_matrix(threat->base().frame());
        targets.push_back({{world[12],world[13],world[14]}});
    }
    if(start==0xffff)return fail(0xfffffff7u);
    std::vector<std::uint16_t> route;
    const auto exclusion=static_cast<float>(human_frame_signed_word(query.template get<std::uint32_t>(0x38)));
    const auto range=static_cast<float>(human_frame_signed_word(query.template get<std::uint32_t>(0x3c)));
    if(!navigation->find_escape_route(start,targets,exclusion,range,0x100u,route))return fail(0xfffffff6u);
    auto& jobs=resources.navigation_jobs();
    append_jobs(jobs,pool,*navigation,route);
    if(interacting&&jobs.size()>1&&jobs[1]->words.template get<std::uint32_t>(0x20)==query.template get<std::uint16_t>(0x48))
        jobs.erase(jobs.begin());
    smooth(jobs,origin,HumanVector{},true);
    if(jobs.size()>1){
        const auto first=jobs[0]->words.template get<Position>(0);
        const auto second=jobs[1]->words.template get<Position>(0);
        const auto x=(static_cast<long double>(first[0])-from[0])*(static_cast<long double>(second[0])-from[0]);
        const auto z=(static_cast<long double>(first[2])-from[2])*(static_cast<long double>(second[2])-from[2]);
        if(x+z<0)jobs.front()->words.set(0,origin);
    }
    return true;
}
} //namespace hd2::game
#endif
