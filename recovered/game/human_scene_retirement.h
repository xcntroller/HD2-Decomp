#ifndef HD2_GAME_HUMAN_SCENE_RETIREMENT_H
#define HD2_GAME_HUMAN_SCENE_RETIREMENT_H
#include "game/human_joint_math.h"
#include <stdexcept>
namespace hd2::game {
//005A7FE0, AFTER00476680 publishes the SAME owner in Scene2's delayed list.
//This cancels live observations/collisions and flushes one B0 queue. It does not
//deactivate/delete Human, remove its shared input or clear the peer3C borrower.
//Each original field is invalidated only after its actual receiver returns.
template<class Human,class Sight,class Hearing,class Proximity,class Inline,class Mode,class Detach,class Flush>
void retire_scene_human(Human& human,Sight& sight,Hearing& hearing,Proximity& proximity,
        Inline& control,Mode&& mode,Detach&& detach,Flush&& flush){
    if(human.field160)detach(*human.field160,human.field164);
    const auto signed_word=[](std::uint32_t bits){std::int32_t value;std::memcpy(&value,&bits,4);return value;};
    if(human.field258!=0xffffffffu){sight.remove_observer(signed_word(human.field258));human.field258=0xffffffffu;}
    if(human.field25c!=0xffffffffu){sight.remove_target(signed_word(human.field25c));human.field25c=0xffffffffu;}
    if(human.field2a4!=0xffffffffu){hearing.remove_listener(signed_word(human.field2a4));human.field2a4=0xffffffffu;}
    if(human.field188!=-1){proximity.release_human_proximity(human.field188);human.field188=-1;}
    auto* physical=control.control();if(!physical)throw std::logic_error("Scene Human retirement requires actual DD0");
    if(!physical->collision_disabled()){
        auto* look=control.look();if(!look)throw std::logic_error("Scene Human retirement requires actual DD0 posture");
        physical->frame_collision_disabled(true,*look); //DC4 before descriptor rebuild, then actual model visibility(false).
    }
    if(human.loading_state)flush(*human.loading_state,(mode()&1u)!=0); //Actual B0+FC, low BYTE of current mode only.
    human.activation.flag20c=false;
}
}
#endif
