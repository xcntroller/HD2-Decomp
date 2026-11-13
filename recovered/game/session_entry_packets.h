#ifndef HD2_GAME_SESSION_ENTRY_PACKETS_H
#define HD2_GAME_SESSION_ENTRY_PACKETS_H
#include <cstddef>
namespace hd2::game {
//00731610 is distinct from cold0072E4E0: no flush and no implicit other-FIFO
//clear. Require actual manager14 only when asked to reset registered records.
template<class Inputs,class Packets,class Mode>void reset_session_entry_packets(Inputs& inputs,Packets& packets,bool reset_inputs,Mode&& mode){
    if(reset_inputs){
        for(std::size_t n=0;n<inputs.records().size();++n){
            inputs.records().at(n)->reset_pending(true,false,true);
            inputs.records().at(n)->reset_pending(true,true,true);
        }
    }
    packets.clear_selected((mode()&1u)!=0);
}
} //namespace hd2::game
#endif
