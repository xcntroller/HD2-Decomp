#ifndef HD2_GAME_HUMAN_FRAME_FEEDBACK_H
#define HD2_GAME_HUMAN_FRAME_FEEDBACK_H

#include "game/human_joint_owners.h"
#include "game/human_holster_lookup.h"

namespace hd2::game {
//005839D0 base/player and005783D0 AI inline feedback (B0+80). Required
//item execution0058B6C0 and command08 receivers are supplied by the containing
//frame services. An offline command RET does NOT replace item execution.
template<class State,class Queries>void apply_human_frame_feedback(State& state,std::uint32_t flags,
        bool ai,Queries& queries){
    auto& words=state.words();
    words.set(0xad,static_cast<std::uint8_t>((flags>>6)&1u));
    words.set(0xac,static_cast<std::uint8_t>((flags>>8)&1u));
    const auto old=words.template get<std::uint32_t>(0x50);
    words.set(0x50,(flags&0x10000u)?old|0x8000000u:old&0xf7ffffffu);
    state.refresh_item();
    const auto item=state.item();
    if(!item||!queries.human_feedback_item_ready(item))words.set(0xac,std::uint8_t{0});
    if(!ai&&(queries.mode_flags()&7u)!=7u){
        auto* human=state.human();if(!human)throw std::logic_error("Human feedback requires bound B0+44");
        if(!queries.human_is_player(*human))return; //Flags/item refresh precede this base-only gate.
    }
    if(flags&0x1000u){
        if(item){queries.execute_human_feedback_item(state,item,6);state.clear_cached_item();}
    }else if(flags&0x2000u){
        if(item){queries.execute_human_feedback_item(state,item,7);state.clear_cached_item();}
    }else if(flags&0x4000u){
        auto* human=state.human();if(!human)throw std::logic_error("Human kind9 feedback requires bound B0+44");
        if(const auto selected=queries.human_feedback_kind9(*human))queries.execute_human_feedback_item(state,selected,4);
    }else if(flags&0x20000u){
        queries.execute_human_feedback_item(state,item,8); //Native passes current item even NULL; no successful fallback.
    }
}
//Containing-world bridge, retaining the required real item virtual24/28 fire
//receiver throughout the complete0058B6C0→B0+08 sequence.
template<class World,class Fire>struct HumanFrameFeedbackReceivers{
    World& world;Fire& fire;
    auto mode_flags(){return world.mode_flags();}
    template<class Human>bool human_is_player(Human& human){return world.human_is_player(human);}
    template<class Entry>bool human_feedback_item_ready(Entry entry){return world.human_feedback_item_ready(entry);}
    template<class Human>auto human_feedback_kind9(Human& human){return world.human_feedback_kind9(human);}
    template<class State,class Entry>void execute_human_feedback_item(State& state,Entry entry,std::uint32_t command){world.execute_human_feedback_item(state,entry,command,fire);}
};
} //namespace hd2::game
#endif
