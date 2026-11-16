#ifndef HD2_GAME_HUMAN_GROUND_FRAME_RECEIVERS_H
#define HD2_GAME_HUMAN_GROUND_FRAME_RECEIVERS_H
#include "game/human_joint_owners.h"
namespace hd2::game {
//00451900: initial blast100 occurs before packet acquisition. The centre is
//copied before the blast, but the packet rereads the caller's model position.
template<class Position,class Blast,class Packets>void emit_human_ground_hazard(Position&& position,
        std::int32_t material,Blast&& blast,Packets& packets){
    const auto centre=position();blast(centre,5.f,1000.f);
    auto& packet=packets.begin(14u,0); //00452CA0(0,true), actual priority0.
    packet.write(0u,1u); //log2(1)/log2(2)+1, native subtype width1.
    const auto* quantizer=packet.primary_quantizer(-1);
    (void)packet.write_vector(position(),quantizer,7u);packet.write(static_cast<std::uint32_t>(material),8u);
    packet.finish_broadcast(); //00452D00→0072BF90, even offline.
}
//005A97C0: field174 is captured BEFORE B0 D8. The final call is HUD0049EF70,
//kind2, not a packet. The old focus remains its argument after command mutation.
template<class Human,class Common,class Hud>void retire_human_water_focus(Human& human,std::uint32_t mode,
        Common&& common,Hud&& hud){
    if((mode&7u)!=7u)return;
    auto* focus=human.field174;if(!focus)return;
    common(human,5u,-1,0u);hud(focus,2u);
}
} //namespace hd2::game
#endif
