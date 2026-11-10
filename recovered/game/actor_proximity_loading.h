#ifndef HD2_GAME_ACTOR_PROXIMITY_LOADING_H
#define HD2_GAME_ACTOR_PROXIMITY_LOADING_H

#include "game/human_joint_math.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

namespace hd2::game {

//008AED54/006E00B0 process owner, distinct from GEffects. Actors are borrowed;
//pairs own only three BYTEs. Native pointer/vector layout is not emulated.
template<class Actor>struct ActorProximityState {
    struct Entry {Actor actor{};float speed=0;};
    struct Pair {std::uint8_t first=0,second=0,inside=0;};
    struct Bucket {std::uint32_t timer=0,interval=0;std::vector<std::unique_ptr<Pair>> pairs;};
    ActorProximityState(){for(unsigned n=0;n!=10;++n){buckets[n].interval=100u<<n;buckets[n].timer=buckets[n].interval+100u*n;}}
    ~ActorProximityState(){clear();}
    ActorProximityState(const ActorProximityState&)=delete;ActorProximityState& operator=(const ActorProximityState&)=delete;
    void clear()noexcept { //006E07C0 keeps all ten buckets, timers and container allocations.
        while(!entries.empty())entries.pop_back();
        for(auto n=buckets.size();n;){auto& pairs=buckets[--n].pairs;while(!pairs.empty())pairs.pop_back();}
    }
    std::vector<std::unique_ptr<Entry>> entries;std::array<Bucket,10> buckets;
};

//006DFCB0/006DFE50/006DFFD0/006E0380/006E0760. Receivers implements actual
//005A83F0 on the SAME Human command/B0 owners; source initial registration
//already evaluates pairs from model matrices and cannot be an empty service.
template<class Frames,class Receivers,class Actor>class ActorProximityLoading {
public:
    using State=ActorProximityState<Actor>;using Pair=typename State::Pair;
    ActorProximityLoading(State& state,Frames& frames,Receivers& receivers):state_(state),frames_(frames),receivers_(receivers){}
    std::int32_t add(Actor actor,unsigned speed_kind,std::uint32_t mode) { //006DFCB0, first ascending hole.
        if(!(mode&1u))return -1;
        if(!actor)throw std::runtime_error("Actor proximity requires an actual actor");
        const auto speed=speed_value(speed_kind);std::size_t slot=0;
        while(slot<state_.entries.size()&&state_.entries[slot])++slot;
        if(slot>0x7fffffffu)throw std::length_error("Actor proximity exceeds original signed index range");
        auto record=std::make_unique<typename State::Entry>();record->actor=actor;record->speed=speed;
        if(slot==state_.entries.size())state_.entries.push_back(std::move(record));else state_.entries[slot]=std::move(record);
        try{for(std::size_t other=0;other<state_.entries.size();++other) {
            if(other==slot)continue;
            const auto kind=actor->base().type();
            if(kind==4||kind==37) {
                if(!state_.entries[other])throw std::runtime_error("Original Car proximity dereferences a vacant actor slot");
                if(state_.entries[other]->actor->base().type()!=3)continue;
            }
            auto pair=std::make_unique<Pair>();
            pair->first=static_cast<std::uint8_t>(std::min(slot,other));pair->second=static_cast<std::uint8_t>(std::max(slot,other));
            auto& first=state_.buckets[0].pairs;first.push_back(std::move(pair));update(0,first.size()-1,true);
        }}catch(...){remove(static_cast<std::int32_t>(slot),mode);throw;}
        return static_cast<std::int32_t>(slot);
    }
    bool remove(std::int32_t index,std::uint32_t mode) { //006DFE50, no exit callbacks.
        if(!(mode&1u)||!entry(index))return false;
        state_.entries[static_cast<std::size_t>(index)].reset();
        for(auto n=state_.buckets.size();n;){auto& pairs=state_.buckets[--n].pairs;
            for(auto m=pairs.size();m;){--m;if(pairs[m]->first==index||pairs[m]->second==index){
                auto old=std::move(pairs[m]);old.reset();if(m+1!=pairs.size())pairs[m]=std::move(pairs.back());pairs.pop_back();
            }}
        }
        while(!state_.entries.empty()&&!state_.entries.back())state_.entries.pop_back();
        return true;
    }
    bool speed_kind(std::int32_t index,unsigned kind,std::uint32_t mode) { //006DFFD0, reevaluate only an ORDERED speed increase.
        if(!(mode&1u))return false;
        auto* record=entry(index);if(!record)return false;
        const auto old=record->speed;record->speed=speed_value(kind);
        if(old<record->speed)for(auto n=state_.buckets.size();n;){--n;
            for(auto m=state_.buckets[n].pairs.size();m;){--m;const auto& pair=*state_.buckets[n].pairs[m];
                if(pair.first==index||pair.second==index)update(n,m,false);
            }
        }
        return true;
    }
    void tick100() { //006E0760 subtracts EXACT100, updates only when timer becomes zero, not <=0.
        for(std::size_t n=0;n<state_.buckets.size();++n) {
            auto& bucket=state_.buckets[n];bucket.timer-=100;
            if(bucket.timer)continue;
            bucket.timer=bucket.interval;for(auto m=bucket.pairs.size();m;)update(n,--m,false);
        }
    }
    void clear()noexcept{state_.clear();}
    State& state(){return state_;}
private:
    static float speed_value(unsigned kind) {
        constexpr std::array<float,5> values{{5,0,0,20,0}}; //Exact00843504 table; not five radii.
        if(kind>=values.size())throw std::out_of_range("Actor proximity speed kind exceeds native table");
        return values[kind];
    }
    auto* entry(std::int32_t index) {
        return index>=0&&static_cast<std::size_t>(index)<state_.entries.size()?state_.entries[static_cast<std::size_t>(index)].get():nullptr;
    }
    void update(std::size_t bucket_index,std::size_t pair_index,bool world) { //006E0380, native callback arguments are bucket transitions.
        auto* pair=state_.buckets.at(bucket_index).pairs.at(pair_index).get();
        auto* first=entry(pair->first);auto* second=entry(pair->second);if(!first||!second)return;
        const auto a=first->actor,b=second->actor;
        HumanVector difference{};
        if(world) {
            const auto bm=frames_.actor_world_matrix(b->base().frame()),am=frames_.actor_world_matrix(a->base().frame());
            for(unsigned n=0;n!=3;++n)difference[n]=static_cast<float>(static_cast<long double>(am[12+n])-bm[12+n]);
        }else {
            const auto bp=b->base().world_position(),ap=a->base().world_position();
            for(unsigned n=0;n!=3;++n)difference[n]=static_cast<float>(static_cast<long double>(ap[n])-bp[n]);
        }
        const auto first_kind=a->base().type(),second_kind=b->base().type();
        const float radius=first_kind==4||second_kind==4?7.5f:first_kind==37||second_kind==37?10.f:5.f;
        const auto x=static_cast<long double>(difference[0]),y=static_cast<long double>(difference[1]),z=static_cast<long double>(difference[2]);
        const float square=static_cast<float>((y*y+z*z)+x*x);std::int32_t square_bits;std::memcpy(&square_bits,&square,4);
        const auto distance=square_bits<0x322bcc77?0.L:std::abs(static_cast<long double>(square)-1)<9.99999993922529e-09f?
            1.L:std::sqrt(static_cast<long double>(square));
        const auto signed_gap=distance-radius;const bool inside=signed_gap<=0;
        const float gap=static_cast<float>(std::abs(signed_gap));
        auto speed=static_cast<long double>(second->speed)+first->speed;if(speed==0)speed=.009999999776482582f;
        const float time=static_cast<float>((1000.L/speed)*gap);
        std::size_t next=state_.buckets.size()-1;
        for(std::size_t n=0;n+1<state_.buckets.size();++n)if(time<state_.buckets[n+1].interval){next=n;break;}
        const auto actual_next=next;auto previous=bucket_index;bool notify=true;
        const bool changed=(inside!=(pair->inside!=0))||world;
        if(changed) {
            pair->inside=static_cast<std::uint8_t>(inside);
            if(next==previous){previous=inside?1:0;next=inside?0:1;}
        }else if(inside||next==previous||!previous||!next)notify=false;
        if(notify&&world&&!previous) {
            if(!pair->inside)notify=false;
            else {previous=1;next=0;}
        }
        if(notify&&radius==5) {
            receivers_.human_proximity_changed(a,b,gap,static_cast<std::uint32_t>(next),static_cast<std::uint32_t>(previous));
            receivers_.human_proximity_changed(b,a,gap,static_cast<std::uint32_t>(next),static_cast<std::uint32_t>(previous));
        }
        if(actual_next!=bucket_index) {
            auto& source=state_.buckets[bucket_index].pairs;auto& target=state_.buckets[actual_next].pairs;
            target.reserve(target.size()+1);target.push_back(std::move(source[pair_index]));
            if(pair_index+1!=source.size())source[pair_index]=std::move(source.back());
            source.pop_back();
        }
    }
    State& state_;Frames& frames_;Receivers& receivers_;
};

//005A83F0 with005AE640/005AE810 command ID lists. All seven supported B0
//kinds have genuine RET8 at+3C/+40 (5830D0/E0 or57A370/380); no fake AI event.
template<class Human,class Actor>void notify_human_proximity(Human& human,Actor actor,float distance,std::uint32_t next,std::uint32_t previous) {
    (void)distance;if(next!=0&&previous!=0)return;
    if(human.command) {
        if(!actor)return;
        auto& list=human.command->lists[0];const auto id=static_cast<std::uint32_t>(actor->base().id10());
        std::size_t index=list.size();for(auto n=list.size();n;)if(list[--n]==id){index=n;break;}
        if(next==0){if(index!=list.size())return;list.push_back(id);}
        else {if(index==list.size())return;list[index]=list.back();list.pop_back();}
    }
    if(auto* state=human.loading_state) {
        switch(state->words().template get<std::uint32_t>(0x48)) {
        case 1:case 3:case 4:case 5:case 6:case 7:case 8:return; //Actual derived virtual RET8 bodies.
        default:throw std::runtime_error("Human proximity callback requires an actual supported B0 receiver");
        }
    }
}
} //namespace hd2::game
#endif
