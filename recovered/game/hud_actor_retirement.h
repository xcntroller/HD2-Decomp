#ifndef HD2_GAME_HUD_ACTOR_RETIREMENT_H
#define HD2_GAME_HUD_ACTOR_RETIREMENT_H

#include <cstdint>
#include <stdexcept>

namespace hd2::game {

//0049EF70, borrowing SAME HUD flags,6B8F8/FC pair and loaded dialog owners.
//Positive roll/signal rebuilding remains a required receiver, never a null
//callback/default success. The actual loading dialogs have no target producer.
template<class Hud,class Pair,class Dialogs,class Command,class Actor>
void retire_hud_actor(Hud& hud,Pair& selection,Dialogs& dialogs,Command& command,std::uint32_t mode,unsigned kind,Actor* actor) {
    if(!hud.flags||!(mode&2u))return;
    if(!actor)throw std::logic_error("HUD actor retirement requires an actual actor");
    if(kind==1) {
        if(selection[1]==actor)selection[1]=nullptr;
        if(selection[0]==actor)selection[0]=nullptr;
        dialogs.retire_roll_actor(actor,true);
    }else if(kind==2)dialogs.retire_roll_actor(actor,false);
    else return;
    command.retire_actor_reference(actor); //HUD10B3C = command10B20+1C, separate from signals2E80.
}
} //namespace hd2::game
#endif
