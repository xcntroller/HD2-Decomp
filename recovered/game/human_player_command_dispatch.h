#ifndef HD2_GAME_HUMAN_PLAYER_COMMAND_DISPATCH_H
#define HD2_GAME_HUMAN_PLAYER_COMMAND_DISPATCH_H
#include "game/human_active_ai_retreat.h"
#include "game/human_player_cover_move.h"
#include "game/human_active_ai_task_completion.h"
#include "game/human_packet_commands.h"
#include <functional>
namespace hd2::game {
//005A45E0→0053D910 copies the stack kind3 request into owned first80 command12.
//Native offline pointer payloads use the actual i386 source actor addresses;
//the World encoder must reject an unrepresentable host address, never use an ID.
template<class State,class Request,class Frames,class World>void queue_player_signal_command(State& state,const Request& request,Frames& frames,World& world){
    auto* human=state.human();if(!human||!human->head_marker)throw std::runtime_error("Player signal requires actual Human260 head frame");
    const auto matrix=frames.actor_world_matrix(human->head_marker);
    HumanPacketCommand command;command.kind=12;auto& fields=command.fields;
    fields.set(0x10,HumanVector{{matrix[12],matrix[13],matrix[14],matrix[15]}});fields.set(0x24,3u);
    fields.set(0x30,request.position);fields.set(0x40,request.command);fields.set(0x44,request.action);
    fields.set(0x48,world.player_command_pointer_word(request.target));fields.set(0x4c,request.parameter);fields.set(0x50,world.player_command_pointer_word(request.sender));
    state.queue_frame_command(std::move(command),false);
}
//0053D5B0 passes the actual embedded4E0 object, not a DD0 common command.
//Borrow scalar words so recipient callbacks observe fresh inline mutations.
template<class State>class HumanDelayedPlayerCommand {
public:
    explicit HumanDelayedPlayerCommand(State& state):state_(state){}
    std::uint32_t word(unsigned offset)const{return state_.player_words().template get<std::uint32_t>(0x4e0+offset);}
    HumanVector position()const{return state_.player_words().template get<HumanVector>(0x4f0);}
    std::uint32_t action()const{return word(0x20);}
private:State& state_;
};
//004568D0: modes0..3 copy their actual process group;4 reverse-filters group0
//and5 reverse-filters the actual roster. These are NOT private empty groups.
template<class Process,class Control,class World>auto collect_player_command_recipients(Process& process,Control& control,std::uint32_t mode,World& world){
    using Human=std::remove_pointer_t<decltype(control.human_roster().at(0))>;std::vector<Human*> output;
    if(mode<4u){output=process.groups().at(mode);return output;}
    if(mode!=4u&&mode!=5u)throw std::runtime_error("Players recipient mode exceeds native group storage");
    auto* selected=control.selected_player(world.mode_flags());
    const auto accept=[&](Human* human){
        if(!human)throw std::runtime_error("Players recipient query reaches native NULL Human");
        if(human_frame_signed_word(human->properties.stats_ec[0])>0&&human!=selected&&(mode==5u||!world.player_command_pending_count(*human)))output.push_back(human);
    };
    if(mode==4u){const auto& group=process.groups()[0];for(auto n=group.size();n;)accept(group[--n]);}
    else for(auto n=control.human_roster().size();n;)accept(control.human_roster().at(static_cast<std::int32_t>(--n)));
    return output;
}
//00537C60, actual three-attempt retreat producer.
template<class AI,class Frames,class World>void start_active_player_retreat(AI& ai,const HumanVector& direction,bool immediate,Frames& frames,World& world){
    auto& state=ai.state();state.set(0x190,3u);state.set(0x150,direction);state.set(0x202,std::uint8_t{1});
    const auto model=frames.actor_world_matrix(ai.human()->base().frame());
    ai.initialize_task(nullptr,40u,HumanVector{{model[12],model[13],model[14],model[15]}},direction,true,1.5f);
    for(auto offset:{0x201u,0x200u,0x204u})state.set(offset,std::uint8_t{1});
    state.set(0x188,std::uint8_t(immediate));state.set(0x1c0,3.f);ai.pending_first_mode(immediate?5u:4u);
    notify_human_active_ai_empty_orders(ai,world);select_active_group_move_posture(ai,frames,world);
}
//0045B7D0 CRT writes XYZ; all six fourth lanes retain their real BSS zero.
inline constexpr std::array<HumanVector,6> player_position_move_offsets{{
    {{0,0,0,0}},{{-4,0,-2,0}},{{4,0,-2,0}},{{-2,0,-8,0}},{{2,0,-8,0}},{{0,0,0,0}}
}};
template<class AI,class Frames,class World>void start_active_player_position_move(AI& ai,const HumanVector& position,bool immediate,Frames& frames,World& world){ //00535440.
    auto& state=ai.state();state.set(0x190,0u);
    ai.initialize_task(nullptr,42u,position,ai.human()->base().world_direction(),true,2.5f);
    state.set(0x204,std::uint8_t{1});state.set(0x1c0,2.5f);state.set(0x188,std::uint8_t(immediate));state.set(0x214,200u);
    ai.pending_first_mode(immediate?5u:4u);notify_human_active_ai_empty_orders(ai,world);select_active_group_move_posture(ai,frames,world);
}
//0045B890: attachment effects run for ALL recipients before ANY position
//task. A passenger skipped in the second pass does not consume an offset.
template<class Command,class Frames,class World>void dispatch_player_position_move(Command& command,bool immediate,Frames& frames,World& world){
    auto* selected=world.active_selected_player();auto recipients=world.player_command_recipients(command,4u);if(recipients.empty())return;
    if(!selected)throw std::runtime_error("Players position move reaches native NULL selected Human");
    HumanMatrix matrix{};frames.formation_matrix_direction(matrix,selected->base().world_direction());
    const auto position=command.position();for(unsigned n=0;n!=3;++n)matrix[12+n]=position[n];
    std::vector<std::function<void()>> loans;for(auto* human:recipients){if(!human)throw std::runtime_error("Players position move reaches native NULL recipient");loans.emplace_back(world.active_event_actor_guard(*human));}
    for(std::size_t n=0;n<recipients.size();++n){loans[n]();auto* human=recipients[n];if(auto* attachment=human->field160){
        if(attachment->base().type()==21u)world.player_command_common_action(*human,5u,0,0);
        else if(!world.active_command_seat_occupied(*attachment,0u))world.player_command_common_action(*human,3u,attachment->base().id10(),0u);
        loans[n]();
    }}
    std::size_t offset=0;
    for(std::size_t n=0;n<recipients.size();++n){loans[n]();auto* human=recipients[n];if(human->field160&&human->field164)continue;
        if(offset>=player_position_move_offsets.size())throw std::runtime_error("Players position move exceeds native six-offset storage");
        const auto destination=frames.formation_transform(player_position_move_offsets[offset],matrix);
        //004D0430(position,colour,.5,1,5000,0) is the previously verified RET16.
        start_active_player_position_move(world.active_command_state(*human),destination,immediate,frames,world);loans[n]();++offset;
    }
}
//00538430/00538460/00538490: current order completion and pause/resume.
template<class AI>void change_active_player_orders(AI& ai,std::uint32_t action){
    if(action==27u||action==28u){
        if(ai.has_pending_orders_loading()){auto& first=ai.first_pending_order().state;if(first.template get<std::uint32_t>(0xc)==action-21u)first.set(0x48,std::uint8_t{1});}
        return;
    }
    const bool paused=action==29u;auto& state=ai.state();state.set(0x18d,std::uint8_t(paused));
    if(!ai.has_pending_orders_loading()||state.template get<std::uint8_t>(0x202)||state.template get<std::uint8_t>(0x200))return;
    auto& query=ai.resources().query();
    if(!paused){query.request_position(state.template get<HumanVector>(0x1e0));return;}
    const auto flags=query.state.template get<std::uint32_t>(0x50);if(flags&0x10000000u)query.state.set(0x50,flags|0x2000u);
}
//00455D40 obtains the actual Active134 count BEFORE its action switch. Only
//27..32 bypass that count's zero gate; they still use the same B0 borrower.
template<class AI,class Command,class Human,class Frames,class World>void receive_active_player_command(AI& ai,Command& command,Human* selected,Frames& frames,World& world){
    const bool available=!ai.has_pending_orders_loading();const auto action=command.action();
    if(!available&&action<27u)return;
    switch(action){
    case 0:case 1:world.set_active_order_formation(ai,selected);ai.enable_follow(action==1u);return;
    case 2:case 3:ai.cancel_task();ai.state().set(0x188,std::uint8_t(action==3u));ai.complete_pending_task(32u);return;
    case 4:case 5:start_active_player_retreat(ai,command.position(),action==5u,frames,world);return;
    case 6:case 7:hold_human_active_task(ai,action==7u,true,world);return;
    case 12:case 13:case 14:case 15:case 16:case 17:ai.active_event_action_mode(action);notify_human_active_ai_empty_orders(ai,world);return;
    case 18:ai.saved_frame_position(true,command.position());notify_human_active_ai_empty_orders(ai,world);return;
    case 19:start_active_group_grenade(ai,command.position(),world);return;
    case 20:world.start_active_player_support(ai,command.position());return;
    case 26:ai.regroup(false);return;
    case 27:case 28:case 29:case 30:change_active_player_orders(ai,action);return;
    case 31:ai.reset_orders_loading(true);ai.cancel_task();ai.disable_follow_loading(false);return;
    case 32:ai.regroup(true);if(ai.human()->formation_leader)ai.enable_follow(ai.state().template get<std::uint8_t>(0x188)!=0);return;
    default:return;
    }
}
//00456020. Selected Player is read once before switch; per-recipient00455D40
//rereads inline command words. Sender/B0 and recipient loans validate around callbacks.
template<class State,class World>void dispatch_delayed_player_command(State& state,World& world){
    HumanDelayedPlayerCommand<State> command(state);const auto validate=world.human_packet_action_validator(state);
    auto* selected=world.active_selected_player();const auto selected_live=selected?std::function<void()>(world.active_event_actor_guard(*selected)):std::function<void()>{};
    const auto action=command.action();
    if(action==8u||action==9u){world.player_command_cover_move(command,action==9u);validate();return;}
    if(action==10u||action==11u){world.player_command_position_move(command,action==11u);validate();return;}
    if((action>=21u&&action<=25u)){if(selected_live)selected_live();world.player_command_formation(selected,action);validate();return;}
    if(action==53u||action==56u){world.player_command_target_dispatch(command,action==56u);validate();return;}
    if(!command.word(0x30)&&action>11u&&action<18u&&selected){if(selected_live)selected_live();world.player_command_selected_mode(*selected,command.action());validate();}
    const auto mode=action>=27u&&action<=32u?0u:4u;
    auto recipients=world.player_command_recipients(command,mode);validate();
    std::vector<std::function<void()>> loans;for(auto* human:recipients){if(!human)throw std::runtime_error("Players dispatch reaches native NULL recipient");loans.emplace_back(world.active_event_actor_guard(*human));}
    for(auto n=recipients.size();n;){const auto index=--n;loans[index]();
        const auto pending=world.player_command_pending_count(*recipients[index]);
        if((mode==0u?pending!=0u:pending==0u)){world.receive_player_command(*recipients[index],command);validate();}
    }
}
}
#endif
