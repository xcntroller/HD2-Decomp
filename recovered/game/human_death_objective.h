#ifndef HD2_GAME_HUMAN_DEATH_OBJECTIVE_H
#define HD2_GAME_HUMAN_DEATH_OBJECTIVE_H
#include "frontend/game_data.h"

namespace hd2::game {
//006B8700's one-objective producer. Native006B8910 initializes base kind4/
//status4, then the containing producer changes kind12 and defines all six words.
//Full selected-map parsing/publication remains a separate required owner.
inline std::unique_ptr<frontend::GameDataNode> make_network_mission_objective(const std::array<std::uint32_t,6>& row){
    auto objective=std::make_unique<frontend::GameDataNode>(12);
    objective->network_objective=frontend::GameDataNode::NetworkObjectiveFields{row[4],row[1],row[0],row[2],row[3],row[5]};
    return objective;
}
inline const frontend::GameDataNode::NetworkObjectiveFields& human_death_objective_fields(const frontend::GameDataNode* objective){
    if(!objective||objective->kind!=12||!objective->network_objective)
        throw std::runtime_error("Player death requires actual kind12 selected-map objective metadata");
    return *objective->network_objective;
}
//0053BD00 positive550 tail. Select the first28==1, but compare the FIRST
//objective's18 side, deliberately preserving the native first-row quirk.
template<class World>void finish_human_player_death_objective(const frontend::GameDataNode* mission,World& world){
    if(!mission)throw std::logic_error("Player death objective lost its actual mission descriptor");
    std::size_t index=0;
    while(index<mission->related.size()&&human_death_objective_fields(mission->related[index].get()).selector28!=1)++index;
    if(index==mission->related.size())return;
    const auto side=human_death_objective_fields(mission->related.front().get()).side18;
    const auto team=world.human_player_objective_team();
    if(index>=0x7fffffffu)throw std::out_of_range("Player objective exceeds native signed identity");
    world.human_player_objective_status(static_cast<std::int32_t>(index+1),side==team?2:1);
}
} //namespace hd2::game
#endif
