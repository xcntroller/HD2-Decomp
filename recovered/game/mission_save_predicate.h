#ifndef HD2_GAME_MISSION_SAVE_PREDICATE_H
#define HD2_GAME_MISSION_SAVE_PREDICATE_H
#include <stdexcept>
namespace hd2::game {
//0067FAA0, thiscall(mission). Two fresh forward scans of actual objective34.
//Empty/all4 admits; any0 admits; other populated states block. This query
//does not mark objectives complete, perform a save or produce saved state.
template<class Mission>bool mission_save_blocked(const Mission& mission){
    bool all_four=true;
    for(const auto& objective:mission.related){
        if(!objective)throw std::logic_error("Save predicate requires its actual objective descriptor");
        if(objective->word10!=4){all_four=false;break;}
    }
    if(all_four)return false;
    for(const auto& objective:mission.related){
        if(!objective)throw std::logic_error("Save predicate requires its actual objective descriptor");
        if(!objective->word10)return false;
    }
    return true;
}
}
#endif
