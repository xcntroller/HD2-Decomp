#ifndef HD2_GAME_HUMAN_INPUT_SESSION_PROFILES_H
#define HD2_GAME_HUMAN_INPUT_SESSION_PROFILES_H

#include "game/human_input_records.h"

namespace hd2::game {

//Quantizer/default portion of00723000. The containing session still owns the
//actual0073EC50 manager, its embedded30 record and actor-dependent E4/E0
//receiver getters. These profiles never replace those required receivers.
class HumanInputSessionProfiles {
public:
    HumanInputSessionProfiles() {
        using Shared=HumanInputShared;
        profiles_[0]=Shared::make_quantizer(uniform(0xc47a0000),uniform(0x453b8000),uniform(0x3c23d70a));
        profiles_[1]=Shared::make_quantizer(uniform(0xbe99999a),uniform(0x3f19999a),
            {{real(0x3ba3d70a),real(0x3be56042),real(0x3ba3d70a)}});
        profiles_[2]=Shared::make_quantizer(uniform(0xc0f00000),uniform(0x41700000),uniform(0x3dcccccd));
    }
    HumanInputSessionProfiles(const HumanInputSessionProfiles&)=delete;
    HumanInputSessionProfiles& operator=(const HumanInputSessionProfiles&)=delete;
    template<class Defaults>
    void configure(Defaults& defaults,HumanInputShared& shared,std::uint32_t mode) const {
        //Native construction AND pointer order20/120/A0: large, small, medium.
        defaults.primary=defaults.secondary={{&profiles_[0],&profiles_[1],&profiles_[2]}};
        defaults.field2=0x3b;defaults.interval=real(0x3d0f5c29); //Final00723000 defaults for later records.
        shared.relative_positions=(mode&1u)!=0; //00982F94.
    }
    const auto& profiles()const{return profiles_;}
    auto& world_profile(){return profiles_[0];} //Same session20 address retained by existing records.
private:
    static float real(std::uint32_t word){float value;std::memcpy(&value,&word,4);return value;}
    static std::array<float,3> uniform(std::uint32_t word){const auto value=real(word);return {{value,value,value}};}
    std::array<HumanInputShared::Quantizer,3> profiles_;
};
} //namespace hd2::game
#endif
