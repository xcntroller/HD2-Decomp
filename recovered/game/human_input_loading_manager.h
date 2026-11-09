#ifndef HD2_GAME_HUMAN_INPUT_LOADING_MANAGER_H
#define HD2_GAME_HUMAN_INPUT_LOADING_MANAGER_H

#include "game/human_input_session_profiles.h"

namespace hd2::game {

//0073EC50/0073ED00 cold containing manager, including its actual embedded30
//record, B0 limiter and20 record registry.00723000 supplies shared session
//profiles and genuine ID-dependent getter functions before records are made.
//No packet/interpolation/transport producers are exposed by this loading owner.
template<class Clock,class Receiver>
class HumanInputLoadingManager {
public:
    using Registry=HumanInputRecords<Clock,Receiver>;
    using Record=typename Registry::Record;
    using Defaults=typename Registry::Defaults;
    using Profiles=typename Registry::Profiles;
    using Getter=std::function<Receiver*(std::int32_t)>;
    HumanInputLoadingManager(Clock& clock,HumanInputShared& shared)
        :shared_(shared),registry_(clock,shared,defaults_),embedded_(clock,shared,-1) {
        defaults_.interval=real(0x3da3d70a); //0073ED80 reset. Session later writes.035.
        state_.set(0xbc,std::uint8_t{0});state_.set(0xd4,std::uint8_t{0});
        for(auto offset:{0xc0u,0xc8u,0xccu,0xd0u})state_.set(offset,std::uint32_t{0});
        state_.set(0xc4,std::uint32_t{1});state_.set(0xd8,defaults_.interval);state_.set(0xdc,std::int32_t{-2});
    }
    ~HumanInputLoadingManager(){registry_.clear();} //Entries, limiter, embedded, then vector allocation.
    HumanInputLoadingManager(const HumanInputLoadingManager&)=delete;
    HumanInputLoadingManager& operator=(const HumanInputLoadingManager&)=delete;
    void configure(const HumanInputSessionProfiles& profiles,Getter first,Getter second,
                   std::int32_t player,std::uint32_t mode) {
        if(configured_||!registry_.records().empty())throw std::logic_error("Human input session configured after record publication");
        if(!first||!second)throw std::runtime_error("Human input session requires both actual ID-dependent getters");
        defaults_.first_receiver=std::move(first);defaults_.second_receiver=std::move(second);
        profiles.configure(defaults_,shared_,mode); //Native session20/120/A0 profiles,3B, interval.035.
        state_.set(0xc0,std::uint32_t{0x3b});state_.set(0xd8,defaults_.interval);
        for(auto& history:embedded_.children)history->flags(2,0x3b); //00723000 explicitly updates embedded30 flags.
        embedded_.id=player;configured_=true;
    }
    Record* find(std::int32_t id,bool create) {if(create)require_configuration();return registry_.find(id,create);}
    Record* make(std::int32_t id){require_configuration();return registry_.make(id);}
    void remove(Record* record){registry_.remove(record);}
    bool contains(const Record* record)const noexcept{return registry_.contains(record);}
    void clear()noexcept{registry_.clear();} //0073F5A0: registered records only; embedded30 survives.
    const auto& records()const{return registry_.records();}
    const auto& embedded()const{return embedded_;}const auto& defaults()const{return defaults_;}
    void bind_primary(const Profiles& profiles) { //0073F250: outer skips nulls, records receive the ORIGINAL triple.
        for(unsigned n=0;n!=3;++n)if(profiles[n])defaults_.primary[n]=profiles[n];
        for(const auto& record:registry_.records())record->bind_primary(profiles);
    }
    void bind_secondary(const Profiles& profiles) { //0073F1F0; embedded30 is intentionally not updated.
        for(unsigned n=0;n!=3;++n)if(profiles[n])defaults_.secondary[n]=profiles[n];
        for(const auto& record:registry_.records())record->bind_secondary(profiles);
    }
    void interval(float interval) { //0073F660, no change to embedded30.
        defaults_.interval=interval;
        state_.set(0xd8,interval);
        for(const auto& record:registry_.records())for(auto& history:record->children)history->interval(interval);
    }
    void player_authority_mode(){state_.set(0xc4,std::uint32_t{0});state_.set(0xc8,std::uint32_t{1});} //0053B9A0 authority branch.
    const auto& state()const{return state_;}
private:
    static float real(std::uint32_t bits){float value;std::memcpy(&value,&bits,4);return value;}
    void require_configuration()const {if(!configured_)throw std::runtime_error("Human input manager has no actual session defaults");}
    struct Limiter { //00741140/00741180; destructor rewrites only its native vtable, with no owning cleanup.
        std::uint32_t word4=0;float threshold8=real(0x3f3cac08);
    };
    HumanInputShared& shared_;Defaults defaults_;Registry registry_;Record embedded_;Limiter limiter_;
    HumanJointWords<0xe8> state_; //Only BC..DC scalar fields; pointers/defaults/embedded record live above.
    bool configured_=false;
};
} //namespace hd2::game
#endif
