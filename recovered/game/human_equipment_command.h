#ifndef HD2_GAME_HUMAN_EQUIPMENT_COMMAND_H
#define HD2_GAME_HUMAN_EQUIPMENT_COMMAND_H

#include <cstdint>

namespace hd2::game {
//0077F2F0's E0 scalar receiver. Context reads the actual DD0+A20/C38;
//native pointer words A4/A8 must not be read through this scalar record.
template<class Words,class Context>
void human_equipment_command(Words& words,std::int32_t command,bool changed,Context& context) {
    bool accepted=false;
    if(!(words.template get<std::uint32_t>(0xd0)&0x100u)&&!(words.template get<std::uint32_t>(0xd4)&0x100u)) {
        const auto mode=context.equipment_current_mode();
        accepted=((mode==0||mode==3||mode==4)&&context.equipment_command_identity()==-10000)||command==100;
    }
    if(accepted||words.template get<std::uint8_t>(0xb0)) {
        if(words.template get<std::int32_t>(0x54)!=command) {
            words.set(0x54,command);
            if(changed)words.set(0x40,words.template get<std::uint32_t>(0x40)|0x100u);
        }
    }else words.set(0xc8,words.template get<std::uint32_t>(0xc8)|0x100u);
}
} //namespace hd2::game
#endif
