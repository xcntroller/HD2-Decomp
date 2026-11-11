#ifndef HD2_GAME_HUMAN_CONTROL_ACTION_QUEUE_H
#define HD2_GAME_HUMAN_CONTROL_ACTION_QUEUE_H

#include "game/human_state_frame_timers.h"
#include <array>

namespace hd2::game {

//0075F230 consumes the existing DD0+A4C deque of twelve-byte records.
//The ordinary return clears E0+B0. The native early completion of commands
//19..22 retains that override until a later frame.
template<class Control,class Look,class Queue,class Equipment,class Publish,class Runtime>
void advance_human_control_action_queue(Control& control,Look& look,Queue& queue,Equipment& equipment,
        std::uint32_t elapsed,Publish& publish,Runtime& runtime){
    if(queue.empty())return;
    auto& record=queue.front();
    record[0]-=elapsed;
    const auto remaining=human_frame_signed_word(record[0]);
    const auto command=record[1],mode=record[2];
    if(mode==3){
        if(remaining<=-16000)queue.pop_front();
        return;
    }
    look.force_frame_commands(true);
    const auto physical=[&](unsigned offset){return human_frame_signed_word(control.animation_state_word(offset));};
    const auto look_word=[&](unsigned offset){return look.state().template get<std::uint32_t>(offset);};
    const auto satisfied=[&]{
        if(mode<2)queue.pop_front();
        else if(mode==2){record[2]=3;record[0]=0;}
    };
    const auto animation=[&](std::int32_t value){look.frame_movement_animation(value,publish);};
    const bool waiting=mode==1&&remaining>0;
    if(command<=2){
        if(!waiting){
            if(look_word(0x4c)==command)satisfied();
            else if(remaining<=(command? -2000:-4000))queue.pop_front();
            else if(physical(0x9ac)==-10000)look.frame_posture(command,publish);
        }
    }else if(command==3){
        if(!waiting){
            if(control.request_frame_ladder(static_cast<typename Control::SceneInstance*>(control.physical().control_action_actor()),look,runtime)>0)satisfied();
            else if(remaining<=-4000)queue.pop_front();
            else animation(0x6a);
        }
    }else if(command>=4&&command<=11){
        if(!waiting){
            constexpr std::array<std::int32_t,8> desired{{0x6a,0x19,4,5,5,6,7,0x27}};
            auto requested=desired[command-4];
            if((command==7||command==8)&&look_word(0xb8)==5)requested=0x59;
            const auto current=physical(0x1d8);
            const bool reached=command==4?(current>=8&&current<=14):current==requested;
            if(reached){
                if(command==11)equipment.transfers().event(9);
                satisfied();
            }else if(remaining<=(command==4? -800:-4000))queue.pop_front();
            else animation(requested);
        }
    }else if(command==12){
        if(mode==0||(mode==1&&remaining<=0)){
            equipment.set_hidden(false); //00791C60(0).
            queue.pop_front();
        }else if(mode==2){
            equipment.set_hidden(false);
            record[2]=3; //Native retains the countdown on this branch.
        }
    }else if(command>=13&&command<=15){
        if(mode==1&&remaining<=0){
            queue.pop_front();look.control_frame_animation_lock(false);
        }else{
            if(physical(0xa34)==0||physical(0xa34)==1)animation(0x2d);
            look.control_frame_animation_lock(true);
        }
    }else if(command==16){
        if(!waiting){
            if(physical(0x1e8)==0x47)satisfied();
            else if(remaining<=-4000)queue.pop_front();
            else control.physical().control_frame_words().set(0x20c,std::uint32_t{0x47});
        }
    }else if(command==17){
        if(!waiting)satisfied();
    }else if(command==18){
        if(!waiting){
            if(physical(0x1d8)==0x28)satisfied();
            else if(remaining<=-4000)queue.pop_front();
            else animation(0x28);
        }
    }else if(command>=19&&command<=22){
        if(physical(0x9ac)==-10000){
            const auto current=physical(0x1d8);
            if(current>=8&&current<=14){
                queue.pop_front();
                return;
            }
            constexpr std::array<std::int32_t,4> desired{{11,12,13,10}};
            animation(desired[command-19]);
        }
        if(remaining<=0)queue.pop_front();
    }
    look.force_frame_commands(false);
}

} //namespace hd2::game
#endif
