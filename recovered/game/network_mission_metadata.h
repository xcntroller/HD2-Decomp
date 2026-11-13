#ifndef HD2_GAME_NETWORK_MISSION_METADATA_H
#define HD2_GAME_NETWORK_MISSION_METADATA_H
#include "frontend/network_map_packets.h"
#include "game/human_death_objective.h"
namespace hd2::game {
inline void clear_network_mission(frontend::GameDataNode& mission){ //006B5890, retains allocations/strings/scalars.
    while(!mission.related.empty())mission.related.pop_back();
    mission.network_mission.value().table74.clear();
    while(!mission.children.empty())mission.children.pop_back();
}
inline void append_network_mission_objective(frontend::GameDataNode& mission,
        std::unique_ptr<frontend::GameDataNode> objective){mission.related.push_back(std::move(objective));} //006B5960.
inline void reset_network_mission_objectives(frontend::GameDataNode& mission){ //006B83C0.
    for(const auto& objective:mission.related){if(!objective)throw std::logic_error("Inline objective lost its actual owner");objective->word10=4;}
}
inline void finalize_network_mission(frontend::GameDataNode& mission){mission.word20=0xffffffffu;reset_network_mission_objectives(mission);} //006B83B0.
//006B8700 changes the SAME application-owned inline mission13, not another
//request/World descriptor. Clear/append/finalize are006B5890/006B5960/006B83B0.
inline bool populate_network_mission(frontend::GameData& data,const frontend::GameConfiguration& configuration){
    const auto& map=frontend::selected_network_map(configuration);
    auto& mission=data.multiplayer_mission();
    if(mission.kind!=13||!mission.network_mission)throw std::logic_error("Application lost its actual inline MP mission owner");
    auto& fields=*mission.network_mission;
    clear_network_mission(mission);
    mission.word8=2002;fields.name6c=map.text1;mission.text24=map.text5?*map.text5:std::string{};
    for(const auto& row:map.table28)append_network_mission_objective(mission,make_network_mission_objective(row));
    for(const auto& row:map.table38)fields.table74.push_back({row,std::nullopt}); //Native copies an UNWRITTEN fourth stack DWORD.
    const auto first=frontend::required_network_map_word(map,0),second=frontend::required_network_map_word(map,1);
    fields.words50[1]=second;fields.words50[0]=first; //Native68 store before64.
    const auto third=frontend::required_network_map_word(map,2),fourth=frontend::required_network_map_word(map,3);
    fields.words50[2]=fourth;fields.words50[3]=third;
    fields.words50[4]=frontend::required_network_map_word(map,4);
    fields.metadata68=map.metadata44; //Copy retains new COW owner before releasing prior, including self-alias.
    fields.words50[5]=frontend::required_network_map_word(map,5);
    finalize_network_mission(mission);
    return true; //Native AL1 after actual clear/copies/append/finalization.
}
} //namespace hd2::game
#endif
