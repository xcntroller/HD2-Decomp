#ifndef HD2_GAME_HUMAN_OBJECTIVE_EXPERIENCE_H
#define HD2_GAME_HUMAN_OBJECTIVE_EXPERIENCE_H

#include "game/mission_script_output_commands.h"
#include "frontend/human_store.h"

namespace hd2::game {

//005A82C0 offline7: profile user14, falling back to short-name10; absent B8
//uses the SAME actor virtual04/name. No borrowed-string lifetime is extended.
template<class Human>const std::string& offline_human_objective_name(const Human& human){
    if(const auto* profile=human.profile_record)return profile->user_name.empty()?profile->short_name:profile->user_name;
    return human.base().name();
}

//005A9640: offline experience is HumanE0 (the final DWORD of C0..E0), not
//the profile's rank/stat array. Arithmetic wraps as a DWORD, formatting is
//signed;0069BFE0 is the genuine empty retail receiver.
class HumanObjectiveExperience {
public:
    explicit HumanObjectiveExperience(const std::uint32_t& mode):mode_(mode){}
    template<class Human>void add_experience(Human& human,std::int32_t amount){
        if((mode_&7u)!=7u)return;
        auto& value=human.fieldc0.at(8);value+=static_cast<std::uint32_t>(amount);
        std::int32_t total;std::memcpy(&total,&value,4);
        auto text=offline_human_objective_name(human)+" Update Experience by "+std::to_string(amount)+" to "+std::to_string(total);
        retail_mission_script_output(text);
    }
private:
    const std::uint32_t& mode_;
};

} //namespace hd2::game
#endif
