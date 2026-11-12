#ifndef HD2_GAME_HUD_COMMAND_FRAME_H
#define HD2_GAME_HUD_COMMAND_FRAME_H
#include "game/hud_command_routes.h"
#include <array>
#include <cstdint>
#include <cstring>
#include <stdexcept>
namespace hd2::game {
struct HudCommandPresentation {
    const void* previous14=nullptr;const void* current18=nullptr;const void* target1c=nullptr;
    std::array<float,4> position20{};bool project_pointer=false;
};
//Scalar state belongs to the actual command panel. feedback_tag identifies one
//of its18 inline buttons rather than retaining a pointer into another owner.
struct HudCommandFrameState {
    std::uint32_t rotation54=0,cooldown58=0,feedback740=0,feedback_tag=0;
    bool mode5c=false,selection_changed5a7b1=false,point_gate5a7b8=false,point_gate5a7b9=false;
};
inline std::int32_t hud_command_signed(std::uint32_t value){std::int32_t result;std::memcpy(&result,&value,4);return result;}
//004B6420. The camera and Human calls below are required producers, never
//successful empty substitutes. Roster/selection lists are borrowed in-call.
template<class Command,class Queries,class Pointer>
void advance_hud_command_frame(Command& command,HudCommandFrameState& state,HudCommandPresentation& presentation,
                               std::uint32_t elapsed,Queries& queries,Pointer& pointer){
    if(!command.visible())return;
    command.update_hover_caption(pointer);
    if(state.cooldown58){state.cooldown58-=elapsed;if(hud_command_signed(state.cooldown58)<0)state.cooldown58=0;}
    presentation.current18=queries.current_identity();const auto previous=presentation.previous14;
    presentation.previous14=queries.previous_identity();
    if(presentation.previous14!=previous){
        state.selection_changed5a7b1=true;command.update_policies(queries);
        if(command.nested_roll_visible())command.show_nested_roll(false,queries.null_actor(),queries,pointer,queries.paused());
    }
    std::uint32_t kind=0;
    if(presentation.previous14&&queries.order_count(presentation.previous14)!=0){
        const auto count=queries.order_count(presentation.previous14);
        if(count<=0)throw std::logic_error("Command selected order disappeared before its last-order query");
        kind=queries.order_kind(presentation.previous14,static_cast<std::uint32_t>(count-1));
    }
    command.update_order_buttons(kind);
    for(std::int32_t index=0;index<queries.roster_count();++index)if(auto* human=queries.roster_human(index)){
        command.update_routes(*human,static_cast<std::size_t>(index),queries);
        if(!state.mode5c)queries.present_human(*human,static_cast<std::uint32_t>(index)); //Index, NOT elapsed;004B653E.
    }
    state.selection_changed5a7b1=false;
    if(command.nested_roll_visible()){
        if(!presentation.previous14)command.show_nested_roll(false,queries.null_actor(),queries,pointer,queries.paused());
        else command.advance_nested_roll(elapsed,queries,pointer);
    }
    if(state.feedback_tag){
        state.feedback740-=elapsed;
        if(hud_command_signed(state.feedback740)<=0){command.toggle_feedback_button(state.feedback_tag);state.feedback_tag=0;}
    }
    if(!state.point_gate5a7b8&&!state.point_gate5a7b9)command.update_point_target(queries,pointer);
    queries.advance_detached_camera(elapsed);
    if(!state.mode5c){
        queries.advance_effects(elapsed);
        auto selected=queries.selected_humans();
        for(auto index=selected.size();index;){--index;auto* human=selected[index];
            if(!human)throw std::logic_error("Command selection contains null Human");
            queries.present_human(*human,static_cast<std::uint32_t>(index));
        }
    }
    queries.publish_soldier_selection();state.rotation54+=elapsed;
    if(pointer.pointer_kind()==2){
        const auto angle=static_cast<float>(static_cast<long double>(hud_command_signed(0u-state.rotation54))*.003000000026077032f);
        pointer.rotate_command_pointer(angle);
    }
}
//004B9E80. prepare_route reaches the SAME Human order74/84 producer0058CCE0;
//the UI copies only vertices into its original six buffers. No model is retained.
template<class Frames,class Human,class Queries>
void update_hud_command_routes(HudCommandRoutes& routes,Frames& frames,const HudCommandFrameState& state,
                               const HudCommandPresentation& presentation,Human& human,std::size_t index,Queries& queries){
    if(!state.mode5c&&!queries.routes_changed(human)&&!state.selection_changed5a7b1)return;
    auto& route=routes.routes().at(index);
    if(queries.route_count(human)==0){route.path_count=route.auxiliary_count=0;return;}
    route.path_count=0; //Positive rebuild deliberately retains the auxiliary strip.
    auto from=queries.actor_position(human);
    for(std::uint32_t order=0;static_cast<std::int64_t>(order)<queries.route_count(human);++order){
        const auto value=queries.prepare_route(human,order,from);
        const bool selected=queries.identity(human)==presentation.previous14;
        routes.append(index,selected?std::array<float,3>{{0,0,1}}:std::array<float,3>{{0,0,0}},value.positions);
        if(!value.model)throw std::runtime_error("Command route requires actual Human order model84");
        frames.object_visible(value.model,true);
        const auto selector=frames.actor_find(value.model,"selector_commands",1);
        if(!selector)throw std::runtime_error("Command route model is missing selector_commands");
        const bool show=presentation.previous14&&queries.identity(human)==presentation.previous14&&queries.selected_order_index()==static_cast<std::int32_t>(order);
        frames.object_visible(selector,show);
        from=value.target?queries.actor_position(*value.target):value.position;
    }
}
} //namespace hd2::game
#endif
