#ifndef HD2_GAME_SESSION_WORLD_INPUT_H
#define HD2_GAME_SESSION_WORLD_INPUT_H

#include "game/human_input_loading_manager.h"

namespace hd2::game {

//0072F500 and0072D2B0's final mode3 writes. All state is borrowed from the
//containing session and process, including the SAME actual input manager.
//A null manager is the genuine native early return, never a replacement one.
template<class Manager,class PacketState>
class SessionWorldInput {
public:
    SessionWorldInput(HumanInputSessionProfiles& profiles,HumanInputShared& shared,
                      Manager* manager,PacketState& packets,std::uint32_t& local_player,std::uint32_t mode)
        :profiles_(profiles),shared_(shared),manager_(manager),packets_(packets),local_player_(local_player),mode_(mode){}
    void bind_world_quantizer(const HumanVector& minimum,const HumanVector& range) {
        if(!manager_)return;
        auto& profile=profiles_.world_profile();
        float y_step;const std::uint32_t bits=0x3aaa64c3;std::memcpy(&y_step,&bits,4);
        profile=HumanInputShared::make_quantizer({{minimum[0],minimum[1],minimum[2]}},
            {{range[0],range[1],range[2]}},{{.01f,y_step,.01f}});
        const typename Manager::Profiles binding{{&profile,nullptr,nullptr}};
        manager_->bind_primary(binding);manager_->bind_secondary(binding);
        shared_.packet_quantizer(0,profile); //0073CCB0→0073CC40 AFTER both history passes.
        manager_->world_quantizer_bound();
    }
    void reset_world_packet_state() {
        packets_.word505c=0;
        if((mode_&7u)==7u)local_player_=0;
        for(auto& buffer:packets_.world_buffers)for(unsigned n=7;n;)buffer[--n]=0; //00724EC0, three actual buffers.
    }
private:
    HumanInputSessionProfiles& profiles_;HumanInputShared& shared_;Manager* manager_;
    PacketState& packets_;std::uint32_t& local_player_;std::uint32_t mode_;
};
} //namespace hd2::game
#endif
