#ifndef HD2_GAME_HUMAN_PEER_REMOVAL_H
#define HD2_GAME_HUMAN_PEER_REMOVAL_H
#include "frontend/control_startup.h"
#include "game/human_damage_messages.h"
namespace hd2::game {
//0072F2F0 is scalar publication followed by a localized application log. It
//does NOT itself delete the peer/transport/Human. Received application2 reaches
//0069E720;17 publishes the local result/hint and this scalar log.
template<class Services>void remove_human_peer(frontend::ControlStartupState& state,std::uint32_t identity,std::uint32_t reason,Services& services){
    if(identity>=state.records.size())throw std::out_of_range("Peer removal requires actual process record");
    if(!state.records[identity].word(0))return;
    if(services.mode_flags()&1u)services.queue_human_peer_removal(reason,identity);
    constexpr std::array<std::uint32_t,4> labels{{0x22ab,0x22aa,0x22ad,0x22ac}};
    const auto format=reason<labels.size()?services.human_damage_localize(labels[reason]):std::string("%s kicked because unknow(new?) reason");
    const auto text=format_human_damage_arguments(format,{{state.records.at(identity).name(),std::string{}}},1);
    if(text.size()>=512)throw std::length_error("Peer removal message exceeds native512-byte buffer");
    services.emit_human_peer_removal_log(text);
}
}
#endif
