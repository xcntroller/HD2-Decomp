#ifndef HD2_GAME_HUMAN_LOADING_STATS_H
#define HD2_GAME_HUMAN_LOADING_STATS_H

#include "game/actor_properties.h"
#include "frontend/human_store.h"
#include <cmath>
#include <cstring>
#include <limits>

namespace hd2::game {

template<class Human>long double human_loading_stat(const Human& human,unsigned index) { //005A6530, x87 return.
    if(index>=9)return 0;
    std::int32_t value;std::memcpy(&value,&human.properties.stats_ec[index],4);
    return static_cast<long double>(value)*(index==0?0x1.47ae14p-10f:0x1.47ae14p-7f);
}

//007E5E00 FISTP signed64 under truncate mode; these callers keep EAX ONLY.
//Invalid/out-of-range x87 integer-indefinite has low DWORD0, not INT32_MIN.
inline std::uint32_t human_loading_stat_word(long double value) {
    constexpr auto minimum=static_cast<long double>(std::numeric_limits<std::int64_t>::min());
    if(!std::isfinite(value)||value<minimum||value>=-minimum)return 0;
    return static_cast<std::uint32_t>(static_cast<std::int64_t>(value));
}
inline void apply_human_difficulty_stats(HumanActorProperties& properties,unsigned difficulty) { //005A8990.
    auto& stats=properties.stats_11c;
    if(difficulty==4){for(unsigned index=2;index!=9;++index)stats[index]=100;return;}
    if(difficulty!=1&&difficulty!=2)return;
    //Exact single-precision constants0080F5C8/0080F7A0/00810D38/00810544.
    const float general=difficulty==1?.7f:.85f,skill=difficulty==1?.9f:.95f;
    for(auto index:{0u,3u,4u,5u,6u,7u,8u}) {
        std::int32_t value;std::memcpy(&value,&stats[index],4);
        stats[index]=human_loading_stat_word(static_cast<long double>(value)*(index==3?skill:general));
    }
}

//0059EA90/005A8790/005A87C0. Store is actual shared006A1C80, NOT a profile
//copy. Negative index retains any prior B8 borrower; nonnegative missing clears
//it. Native1C is the EYE texture string, not HumanStore's display/user name.
template<class Human,class Store,class Effects>
void bind_human_loading_profile(Human& human,std::int32_t index,Store& store,Effects& effects) {
    human.properties.field170=static_cast<std::uint32_t>(index);
    if(index>=0)human.profile_record=store.record(index);
    if(const auto* profile=human.profile_record) {
        for(unsigned n=0;n!=9;++n) {
            float value;const auto bits=profile->stats[n+1];std::memcpy(&value,&bits,4);
            human.properties.stats_ec[n]=human_loading_stat_word(value);
        }
    }
    human.properties.stats_11c=human.properties.stats_ec;
    if(human.profile_record) {
        human.fieldbc=human.profile_record->eye;
        effects.human_eye_texture(human.base().frame(),human.fieldbc); //GEffects+AC,0048A5C0.
    }
}
template<class Human>void update_human_loading_profile_mortality(Human& human,std::uint32_t mode) { //0059FA60 BEFORE Players removal.
    if((mode&7u)==7u&&(human.properties.field19c==1||human.properties.field19c==4)&&human.profile_record) {
        const auto bits=human.properties.stats_ec[0];std::int32_t health;std::memcpy(&health,&bits,4);
        auto& flag=human.profile_record->tail[1];flag=(flag&0xffffff00u)|std::uint32_t(health<1); //BYTE70 only.
    }
}
template<class Human>void retire_human_loading_profile(Human& human,std::uint32_t mode) {
    update_human_loading_profile_mortality(human,mode);human.profile_record=nullptr;
}
} //namespace hd2::game
#endif
