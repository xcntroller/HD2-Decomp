#ifndef HD2_GAME_HUD_PROCESS_DATA_H
#define HD2_GAME_HUD_PROCESS_DATA_H
#include "frontend/control_startup.h"
#include "game/hud_scoreboard_loading.h"
#include <type_traits>
namespace hd2::game {
//004AED20: local peerB0 and configuration270 are both read BEFORE releasing
//0072EC70/0072ED30's lock0, including limit-1. No copied peer registry.
template<class Locks,class Configuration>
HudMissionScore hud_local_mission_score(const frontend::ControlStartupState& state,Locks& locks,std::int32_t identity,const Configuration& configuration){
    locks.enter_control_lock(0);
    struct Unlock {Locks& locks;~Unlock(){locks.leave_control_lock(0);}} unlock{locks};
    if(identity<0)throw std::out_of_range("HUD mission score requires the actual local peer");
    const auto bits=state.records.at(static_cast<std::size_t>(identity)).word(0xb0);
    std::int32_t losses;std::memcpy(&losses,&bits,4);
    return {losses,configuration.rules228[18]};
}
//Reject an expired/foreign key BEFORE dereferencing it. Info sort keys are
//actual record addresses, held only under the containing process lock.
inline const frontend::ControlStartupRecord& hud_info_peer(const frontend::ControlStartupState& state,const void* identity){
    for(const auto& record:state.records)if(&record==identity)return record;
    throw std::logic_error("HUD info peer is absent from the SAME locked process records");
}
inline std::int32_t hud_info_peer_stat(const frontend::ControlStartupRecord& peer,std::uint32_t offset){
    if(offset<0x40||offset>0x60||(offset&3))throw std::out_of_range("HUD info statistic is not a peer40..60 word");
    const auto bits=peer.word(offset);std::int32_t value;std::memcpy(&value,&bits,4);return value;
}
//00725AC0: NULL3C is unavailable; only Player1/Remote7 consult B0 virtual64.
//005A4240/005A4400 read the SAME byte70, already recovered on master. This
//predicate is distinct from health,005A6380 and mode-gated005A63F0.
template<class Human>bool hud_info_peer_unavailable(const Human* human){
    if(!human)return true;
    if(!human->loading_state)throw std::logic_error("HUD peer Human lost its actual B0 owner");
    const auto kind=human->loading_state->words().template get<std::uint32_t>(0x48);
    return (kind==1||kind==7)&&human->loading_state->words().template get<std::uint8_t>(0x70)!=0;
}
//The latest master owns kind12/13 application metadata. Older implementation
//checkpoints require an explicit owner adapter; never substitute offline fields.
template<class Node,class=void>struct HasHudNetworkMetadata:std::false_type{};
template<class Node>struct HasHudNetworkMetadata<Node,std::void_t<decltype(std::declval<Node>().network_objective),decltype(std::declval<Node>().network_mission)>>:std::true_type{};
template<class Node,class Required>std::uint32_t hud_objective_team_caption(const Node& node,std::uint32_t team,Required&& required){
    if(team>1)throw std::out_of_range("HUD objective caption requires team0/1");
    if constexpr(HasHudNetworkMetadata<Node>::value){
        if(node.kind!=12||!node.network_objective)throw std::logic_error("HUD objective lacks its actual kind12 metadata");
        return team?node.network_objective->field20:node.network_objective->field1c;
    }else return required(node,team);
}
template<class Node,class Required>std::int32_t hud_objective_team(const Node& node,Required&& required){
    if constexpr(HasHudNetworkMetadata<Node>::value){
        if(node.kind!=12||!node.network_objective)throw std::logic_error("HUD objective lacks its actual kind12 metadata");
        const auto bits=node.network_objective->side18;std::int32_t value;std::memcpy(&value,&bits,4);return value;
    }else return required(node);
}
template<class Node,class Required>std::shared_ptr<const std::string> hud_network_mission_title(const Node* node,Required&& required){
    if(!node)return {}; //00499770's genuine NULLgameA8 gate.
    if constexpr(HasHudNetworkMetadata<Node>::value){
        if(node->kind!=13||!node->network_mission)throw std::logic_error("HUD mission title lacks its actual kind13 descriptor");
        return node->network_mission->name6c;
    }else return required();
}
} //namespace hd2::game
#endif
