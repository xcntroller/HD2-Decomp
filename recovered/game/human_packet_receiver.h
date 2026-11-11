#ifndef HD2_GAME_HUMAN_PACKET_RECEIVER_H
#define HD2_GAME_HUMAN_PACKET_RECEIVER_H
#include "game/human_packet_commands.h"
namespace hd2::game {
//005A5F80/005A5FB0 and B0 C4/C0. Resolve rechecks the actual Scene2 slot,
//including actor retirement; no decoded value retains a borrower. Source guard
//rereads B0 before each command in case an earlier effect replaced it.
template<class Services,class Resolve>class HumanPacketReceiver {
public:
    HumanPacketReceiver(Services& services,Resolve& resolve,bool authority,bool reserved)
        :services_(services),resolve_(resolve),authority_(authority),reserved_(reserved){}
    HumanPacketGrammar grammar(){
        auto* human=resolve_();const auto kind=human&&human->loading_state?
            human->loading_state->words().template get<std::uint32_t>(0x48):0u;
        if(authority_){
            if(kind==1)return HumanPacketGrammar::authority_player;
            if(reserved_)return HumanPacketGrammar::authority_discard;
        }else{
            if(kind==1)return HumanPacketGrammar::presentation_player;
            if(kind==7)return HumanPacketGrammar::presentation_remote;
            if(reserved_)return HumanPacketGrammar::presentation_discard;
        }
        throw std::runtime_error("Scene2 Human packet requires its actual nonreserved B0 receiver");
    }
    void receive(const HumanPacketCommand& command,HumanPacketGrammar grammar){
        //005B2A90 has an independent referenced-Human effect, even in discard.
        if(command.kind==20){services_.receive_human_equipment_packet(command);return;}
        if(grammar==HumanPacketGrammar::authority_discard||grammar==HumanPacketGrammar::presentation_discard)return;
        auto* human=resolve_();if(!human)throw std::logic_error("Human retired between command decode and effect");
        services_.apply_human_packet_command(*human,command,grammar);
    }
private:
    Services& services_;Resolve& resolve_;bool authority_,reserved_;
};
template<class Services,class Resolve>void receive_human_packet(SessionPacketStream& packet,std::uint32_t mode,
        bool authority,bool reserved,Services& services,Resolve&& resolve){
    HumanPacketReceiver<Services,std::remove_reference_t<Resolve>> receiver(services,resolve,authority,reserved);
    (void)receiver.grammar(); //Native actor/B0 receiver is selected even for an empty list.
    receive_human_packet_commands(packet,mode,receiver);
}
} //namespace hd2::game
#endif
