#ifndef HD2_GAME_HUMAN_PROJECTION_PROCESS_H
#define HD2_GAME_HUMAN_PROJECTION_PROCESS_H

#include <array>
#include <cstdint>
#include <cstring>
#include <list>
#include <iterator>
#include <stdexcept>
#include <vector>
#include <cmath>

namespace hd2::game {
template<class Frames>class HumanProjectionModels;

//0098AD28/38/44: two lists of SAME record payloads and the process free pool.
//No projection-frame/footprint producer is fabricated by this storage owner.
//The world facade clears active frames before its Scene retires; the process
//pool retains already-detached frame references until Items shutdown0078A980.
template<class Owner>class HumanProjectionProcess {
public:
    struct Record {std::uint32_t time=0;std::int32_t human_id=-1;Owner frame{nullptr,nullptr};std::uint32_t texture_index=0;bool left=false;};
    using Records=std::list<Record>;
    HumanProjectionProcess()=default;
    HumanProjectionProcess(const HumanProjectionProcess&)=delete;
    HumanProjectionProcess& operator=(const HumanProjectionProcess&)=delete;
    void retain_surface_position(const std::array<float,4>& position,std::uint32_t mode){ //0078AC30, SAME0098AD18/1C/20/24.
        if(!(mode&2u))return;
        if(surface_positions_.size()==surface_positions_.capacity())surface_positions_.reserve(surface_positions_.capacity()+8u);
        surface_positions_.push_back(position);
    }
    bool near_surface_position(const std::array<float,4>& position,std::uint32_t mode)const{ //0078AD00, AL only;0075D210 consumer.
        if(!(mode&2u))return false;
        for(const auto& prior:surface_positions_)
            if(std::abs(static_cast<long double>(position[0])-prior[0])<.41999998688697815&&
               std::abs(static_cast<long double>(position[1])-prior[1])<.20000000298023224&&
               std::abs(static_cast<long double>(position[2])-prior[2])<.41999998688697815)return true;
        return false;
    }
    void clear_surface_positions(){std::vector<std::array<float,4>>{}.swap(surface_positions_);} //0078A5F0/0078A7D0 free allocation, reset count/capacity.
    void shutdown(std::uint32_t mode){ //0078A980 releases frame BEFORE record, in LIFO free-pool order.
        if(!(mode&2u))return;
        if(world||!primary.empty()||!fading.empty())throw std::logic_error("Retire actual Human projection world before Items process shutdown");
        while(!pool.empty()){pool.front().frame.reset();pool.pop_front();}
    }
private:
    template<class Frames>friend class HumanProjectionModels;
    template<class State,class Services>friend void update_human_projection_process(State&,std::uint32_t,Services&);
    template<class State,class Services>friend void tick_item_projection_process(State&,bool,std::uint32_t,Services&);
    Records primary,fading,pool;
    std::vector<std::array<float,4>> surface_positions_;
    std::array<std::uint32_t,400> per_human{}; //0098A6D8, reset only at actual shared retirement.
    std::uint32_t scan_timer=0,fade_timer=0,presentation_timer=0; //0098AD64/68/0098B874, process BSS; not last-user resets.
    std::uint32_t enabled=0,maximum_per_human=40,lifetime_seconds=40; //0098AD60,0084F5A4/A8.
    const void* world=nullptr; //Source borrower guard, not an original readiness or success flag.
};

inline std::int32_t projection_signed(std::uint32_t word){std::int32_t value;std::memcpy(&value,&word,4);return value;}

//0078AD70. Services are actual Scene/selected profile/CRT/visual endpoints.
//Payloads retain their engine reference when returned to the free pool.
template<class State,class Services>void update_human_projection_process(State& state,std::uint32_t elapsed,Services& services){
    if(!(services.mode_flags()&2u))return;
    const auto now=services.projection_game_time();
    const auto scene_epoch=services.projection_scene_epoch();
    const auto previous_epoch=scene_epoch<2u?0u:scene_epoch-1u;
    const auto decrement=[&](const auto& record){
        if(record.human_id<0||static_cast<std::size_t>(record.human_id)>=state.per_human.size())
            throw std::runtime_error("Projection Human identity exceeds original400-word table");
        --state.per_human[static_cast<std::size_t>(record.human_id)];
    };
    const auto retire=[&](auto& list,auto position,bool remove_before_mode){
        services.detach_projection(position->frame.get()); //Native frame38(NULL,0), then Scene84.
        typename State::Records pending;
        auto next=std::next(position);
        if(remove_before_mode)pending.splice(pending.end(),list,position);
        services.projection_mode(position->frame.get(),3);
        if(!remove_before_mode)pending.splice(pending.end(),list,position);
        state.pool.splice(state.pool.begin(),pending); //0078C810 retains frame, LIFO payload return.
        return next;
    };
    const auto expire=[&](auto position,bool remove_before_mode){
        if(services.projection_frame_epoch(position->frame.get())<previous_epoch)
            return retire(state.primary,position,remove_before_mode);
        position->time=0;auto next=std::next(position);
        state.fading.splice(state.fading.end(),state.primary,position); //0078C840 appends, never prepends.
        return next;
    };
    state.scan_timer+=elapsed;
    if(projection_signed(state.scan_timer)>1400){ //Strict SIGNED >1400, one random draw even for an empty list.
        const auto random=services.projection_random_word();
        state.scan_timer-=1400u+static_cast<std::uint32_t>(random%200);
        state.enabled=services.projection_quality_byte()&0xffu;
        for(auto it=state.primary.begin();it!=state.primary.end();){
            if(it->human_id<0||static_cast<std::size_t>(it->human_id)>=state.per_human.size())
                throw std::runtime_error("Projection Human identity exceeds original400-word table");
            if(state.per_human[static_cast<std::size_t>(it->human_id)]>state.maximum_per_human){decrement(*it);it=expire(it,true);}
            else ++it;
        }
    }
    if(state.lifetime_seconds){
        while(!state.primary.empty()&&state.primary.front().time+state.lifetime_seconds*1000u<=now){ //UNSIGNED wrap, oldest prefix only.
            auto it=state.primary.begin();decrement(*it);expire(it,false);
        }
    }
    state.fade_timer+=elapsed;
    if(projection_signed(state.fade_timer)<300)return;
    state.fade_timer-=300; //Exactly one300ms step per call, no catch-up loop.
    for(auto it=state.fading.begin();it!=state.fading.end();){
        it->time+=300;
        if(it->time>=3000u){it=retire(state.fading,it,true);continue;}
        const float alpha=static_cast<float>(1.L-static_cast<long double>(it->time)*.0003333333297632634639739990234375f);
        if(alpha<services.projection_alpha(it->frame.get())){ //Ordered comparison: NaN stays untouched.
            if(it->time<=300u){services.projection_mode(it->frame.get(),0);services.projection_blend(it->frame.get(),5);}
            services.projection_alpha(it->frame.get(),alpha);services.update_projection(it->frame.get());
        }
        ++it;
    }
}

//007DD390/007DD3B0. Offline authority drives the process once; presentation
//without mode1 accumulates SIGNED >=100 and advances exactly100 once. Native
//subtracts the presentation accumulator AFTER the process callback succeeds.
template<class State,class Services>void tick_item_projection_process(State& state,bool authority,
        std::uint32_t elapsed,Services& services){
    if(authority){if(services.mode_flags()&1u)update_human_projection_process(state,elapsed,services);return;}
    if(services.mode_flags()&1u)return;
    state.presentation_timer+=elapsed;
    if(projection_signed(state.presentation_timer)>=100){
        update_human_projection_process(state,100,services);state.presentation_timer-=100;
    }
}
} //namespace hd2::game
#endif
