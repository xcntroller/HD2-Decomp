#ifndef HD2_GAME_HUMAN_DAMAGE_POLICY_H
#define HD2_GAME_HUMAN_DAMAGE_POLICY_H
#include "game/human_state_frame_timers.h"

namespace hd2::game {
//005A1C90's nonoffline policy, with real application24 configuration words.
//Team/specialization getters borrow the SAME startup records; no local roster.
template<class Human,class Configuration,class Teams>bool allow_human_network_damage(Human& human,std::uint32_t kind,
        Human* source,std::int32_t& reflected,const Configuration& configuration,Teams&& team){
    if(human_frame_signed_word(human.properties.stats_ec[0])>0&& &human!=source&&configuration.style224!=0&&source){
        const auto source_team=team(*source);const auto target_team=team(human); //Native SOURCE before target.
        if(source_team==target_team){
            if(source->properties.field19c==1&&configuration.rules228[7]!=0)reflected=configuration.rules228[11];
            else if(kind!=13&&kind!=5)return false;
        }
    }
    if(human.properties.field19c==1&&human_frame_signed_word(human.field208)<
            human_frame_signed_word(static_cast<std::uint32_t>(configuration.rules228[10])*1000u)&&kind!=12)return false;
    return true;
}
} //namespace hd2::game
#endif
