#ifndef HD2_GAME_HUMAN_PLAYER_SELECTION_H
#define HD2_GAME_HUMAN_PLAYER_SELECTION_H
#include "game/human_state_frame_timers.h"
#include <cstddef>
#include <stdexcept>
namespace hd2::game {
//0045DA40/0045DC20/0045DCC0. Use the existing borrowed roster and B0 owners;
//current2C, selected28 and previous30 are distinct publications.
template<class Control>auto* required_player_roster_human(Control& control,std::int32_t index){
    auto* human=control.human_roster().at(index);
    if(!human)throw std::logic_error("Player selection reached native missing roster Human");
    return human;
}
template<class Control,class World>void resume_loading_player_control(Control& control,World& world){
    const auto current=control.current_index();if(current!=-1)world.select_loading_player(*required_player_roster_human(control,current),true);
}
template<class Control,class World>bool select_loading_player_index(Control& control,std::int32_t index,World& world){
    if(index<0||static_cast<std::size_t>(index)>=control.human_roster().size())return false;
    auto* target=required_player_roster_human(control,index);
    if(human_frame_signed_word(target->properties.stats_ec[0])<=0||target->properties.field19c!=1)return false;
    world.restore_loading_player_model(required_player_roster_human(control,control.current_index())); //0045DB90 BEFORE the fresh old2C lookup.
    world.select_loading_player(*required_player_roster_human(control,control.current_index()),false);
    control.current_index()=index;control.previous_index()=index;control.selected_index()=-1;
    world.bind_loading_first_person_player();world.refresh_loading_first_person_visibility();
    world.loading_selected_player_log(*required_player_roster_human(control,index)); //Fresh roster;005A8320 evaluated even for retail RET0069BFE0.
    return true;
}
template<class Control,class World>bool select_loading_command_player_index(Control& control,std::int32_t index,World& world){
    if(control.previous_index()==index)return true;
    if(index==-1){control.previous_index()=-1;return true;}
    if(index<0||static_cast<std::size_t>(index)>=control.human_roster().size())return false;
    auto* human=required_player_roster_human(control,index);
    if(human_frame_signed_word(human->properties.stats_ec[0])<=0||human->base().type()!=3u)return false;
    control.previous_index()=index;
    if(control.selected_index()==index){
        //0045DD12 reloads current2C, not the validated command target.
        world.select_loading_player(*required_player_roster_human(control,control.current_index()),false);
        control.selected_index()=-1;
    }
    return true;
}
//0045D660/0045D9D0 intentionally return0 for empty/wrapped-to-current. With
//current=-1 and no eligible candidate they may return an invalid final index;
//the later selector rejects it. Do not replace this with a guessed sentinel.
template<class Control>std::int32_t next_loading_player_index(Control& control,bool reverse){
    const auto count=[&]{const auto n=control.human_roster().size();if(n>0x7fffffffu)throw std::length_error("Player roster exceeds native signed count");return static_cast<std::int32_t>(n);};
    if(!count())return 0;
    const auto step=[&](std::int32_t value){return human_frame_signed_word(static_cast<std::uint32_t>(value)+(reverse?0xffffffffu:1u));};
    auto index=step(control.current_index());std::int32_t visited=0;
    if(count()<=0)return index;
    do{
        if(reverse){if(index<0)index=count()-1;}
        else if(index>=count())index=0;
        if(index==control.current_index())return 0;
        auto* human=required_player_roster_human(control,index);
        if(human_frame_signed_word(human->properties.stats_ec[0])>0&&human->properties.field19c==1)return index;
        index=step(index);++visited;
    }while(visited<count());
    return index;
}
//0045DB90 restores the OLD current model before changing its B0. These are
//actual Actor8C flags, not the distinct has-control predicate.
template<class Human,class World>void restore_loading_player_model(Human* human,World& world){
    if(!human)return;
    auto* actor=human->field160;bool visible=true;
    if(actor){const auto kind=actor->base().type();
        if(kind==4u||kind==20u||kind==21u||kind==34u)visible=!(world.player_attachment_control_flags(*human)&0x10u);}
    world.loading_player_model_visible(*human,visible);
}
}
#endif
