#ifndef HD2_GAME_HUMAN_FRAME_RUNTIME_H
#define HD2_GAME_HUMAN_FRAME_RUNTIME_H
#include "game/human_ground_motion.h"

namespace hd2::game {
//Containing Human-frame borrower, not another Human/state/physics owner.
//Known receivers use the same World as activation and damage. Unrecovered
//positive leaves fail at their call site instead of supplying an idle success.
template<class World>class HumanFrameRuntime {
public:
    explicit HumanFrameRuntime(World& world):world_(world){}
    template<class Item,class Human>auto fire_human_item(Item& item,Human& human,const HumanVector& point,
            const HumanVector& direction,std::uint32_t argument,bool primary){
        return world_.fire_human_item(item,human,point,direction,argument,primary);
    }
    template<class Formation>void advance_human_formation(Formation& formation,std::uint32_t elapsed){world_.advance_human_formation(formation,elapsed);}
    template<class State,class Queue>void dispatch_script_ai_event(State& state,Queue& queue,std::size_t index){world_.dispatch_script_ai_event(state,queue,index);}
    template<class State>bool advance_script_formation_path(State& state){return world_.advance_script_formation_path(state);}
    template<class State,class Human>void finish_human_action_drop(State& state,Human& human){world_.finish_human_action_drop(state,human);}
    template<class Actor,class State>void finish_human_actor_motion(Actor& actor,State& state){world_.finish_human_actor_motion(actor,state);}
    template<class Actor,class Human>void finish_human_timed_type15(Actor& actor,Human& human){world_.finish_human_timed_type15(actor,human);}
    template<class Human>void notify_human_ladder_probe(Human& human,std::uint32_t kind){world_.notify_human_ladder_probe(human,kind);}
    template<class State>void apply_player_attachment_inputs(State& state){world_.human_attachment_frame_inputs(state);}
    template<class Human>bool player_attachment_blocks_item(Human& human){return world_.player_attachment_blocks_item(human);}
    template<class Human>std::uint32_t player_attachment_control_flags(Human& human){return world_.player_attachment_control_flags(human);}
    template<class State>void notify_player_selected_item(State& state){world_.notify_player_selected_item(state);}
    template<class Control>bool advance_human_walk_motion(Control& control,std::int32_t elapsed){ //007646F0, actual boolean.
        return control.advance_walk_frame(elapsed,*this);
    }
    template<class Control>void advance_human_motion_event(Control& control,std::uint32_t material,std::uint32_t elapsed){
        game::apply_human_ground_material(control,bound_human(control),world_,*this,
            human_frame_signed_word(material),human_frame_signed_word(elapsed)); //0075EEE0, not a separate event queue.
    }
    template<class Human>void retire_human_water_attachment(Human& human){world_.retire_human_water_attachment(human);}
    template<class Human>void human_ground_surface_effect(Human& human,std::int32_t material){world_.human_ground_surface_effect(human,material);}
    void human_walk_debug_beep(){world_.human_walk_debug_beep();}
    bool human_model_debug_enabled(){return world_.human_model_debug_enabled();}
    template<class Geometry>void present_human_model_debug(Geometry& geometry){world_.present_human_model_debug(geometry);}
    //These named requirements are actual remaining source boundaries. Their
    //conditional callers retain earlier writes and do not skip the receiver.
    template<class Human>void advance_human_state(Human&,bool,std::uint32_t){throw std::runtime_error("Human frame requires actual Active/Remote B0 frame receiver");}
    template<class State>void choose_script_alarm_action(State&){throw std::runtime_error("Script alarm requires actual00591550 decision receiver");}
    template<class Control,class... Args>void apply_human_look_interaction(Control&,Args&&...){throw std::runtime_error("Human interaction requires remaining00751480 action receivers");}
    template<class Control>void advance_human_control_motion(Control&,std::uint32_t){throw std::runtime_error("Human interaction requires remaining00755500 action frame");}
    template<class State,class Human>void refresh_suppressed_player_camera(State&,Human&){throw std::runtime_error("Suppressed Player requires actual00540090 World B8/B4 camera receiver");}
    template<class State>void dispatch_player_inline_command(State&){throw std::runtime_error("Player delayed command requires actual0053D5B0 Players virtual0");}
    template<class State>void advance_player_remote_inputs(State&,std::uint32_t){throw std::runtime_error("Remote Player inputs require actual0053F520");}
    template<class State>void advance_player_input_transport(State&,std::uint32_t){throw std::runtime_error("Player input transport requires actual0053F9B0 non7 history publication");}
    template<class State>void advance_network_ai_presentation(State&,std::uint32_t){throw std::runtime_error("Network AI presentation requires actual0056F710");}
    template<class Control>void refresh_nonoffline_human_visibility(Control&){throw std::runtime_error("Nonoffline Human visibility requires actual00787D50 receiver");}
    void request_human_respawn(){throw std::runtime_error("Human respawn requires actual006DC940/0072B1C0 request");}
    template<class Control>bool human_fall_damage_authority(Control&){throw std::runtime_error("Nonoffline Human fall requires actual authority predicate");}
    template<class Human>void send_human_fall_damage(Human&,float){throw std::runtime_error("Nonauthority fall requires actual005A9BF0 publication");}
    template<class... Args>void human_contact_actor(Args&&...){throw std::runtime_error("Human motion requires actual contacted actor virtual58");}
private:
    template<class Control>static auto& bound_human(Control& control){
        auto** slot=control.human_slot_address();if(!slot||!*slot)throw std::logic_error("Human frame lost actual DD0 Human258");return **slot;
    }
    World& world_;
};
}
#endif
