#ifndef HD2_GAME_FIRST_PERSON_PLAYERS_LOADING_H
#define HD2_GAME_FIRST_PERSON_PLAYERS_LOADING_H

#include "game/human_camera_state.h"
#include "game/human_camera_amount.h"
#include "game/session_activation.h"
#include <functional>
#include <optional>

namespace hd2::game {

//Concrete source lookups for FirstPersonViewLoading: SAME Players control,
//per-Human world composition and Scene2/frame registry. No second roster, B0 or
//inline owner. World supplies its checked human_loading_owner<Composition>.
//Clear camera targets BEFORE world/actor retirement; its callback borrows them.
//The cold first-frame path has no attachments. Reached attachment virtualC4/
//8C remains explicit until the actual occupied actor consumers are available.
template<class HumanActor,class Control,class World,class Composition,class Actors>
class FirstPersonPlayersLoading {
public:
    using Human=HumanActor;using Frame=typename Actors::Frame;
    FirstPersonPlayersLoading(Control& control,World& world,Actors& actors,SessionActivationState& session,
            const std::int32_t& local_player):control_(control),world_(world),actors_(actors),session_(session),local_player_(local_player){}
    Human* current_player()const{return control_.current_player();} //0045D1D0, never selected28.
    auto& inline_control(Human& human){return world_.template human_loading_owner<Composition>(human).control();}
    auto& state(Human& human){
        auto* state=world_.template human_loading_owner<Composition>(human).states().get();
        if(!state||human.loading_state!=state)throw std::runtime_error("First-person player lost its actual published Human B0");
        return *state;
    }
    float opacity(Human& human){ //005AC360→00789C80, actual checked DD0 owner.
        auto* control=inline_control(human).control();
        if(!control)throw std::logic_error("First-person opacity requires the actual Human DD0");
        return control->opacity().opacity();
    }
    std::uint32_t player_mode(Human& human,std::uint32_t mode){return human_camera_action(human,mode,local_player_);} //005A8170.
    std::uint32_t game_phase()const{return session_.words.template get<std::uint32_t>(0xb8);}
    std::optional<Frame> control_sector(Human& human){
        auto& control=inline_control(human);if(!control.control())return std::nullopt;
        return control.scene_sector(); //005AC4C0; an actual null sector remains engaged.
    }
    bool attached_driver(Human& human)const{return human.field160&&human.field160->base().type()==4&&human.field164==0;}
    void attached_first_person(Human& human,bool){
        if(human.field160)throw std::runtime_error("Attached first-person view requires actual actor virtualC4 seat receiver");
    }
    bool attachment_hides_human(Human& human)const { //0045DAC0's exact kind gate before virtual8C/bit10.
        if(human.field160)switch(human.field160->base().type()){
        case 4:case 20:case 21:case 34:throw std::runtime_error("Attached visibility requires actual actor virtual8C seat flags");
        default:break;
        }
        return false;
    }
    std::function<void(Frame)> camera_retirement(){
        //0046C370/00465610 retire old target amount BEFORE releasing the frame.
        //Capture actual owners rather than this facade's lifetime.
        return [actors=&actors_,world=&world_](Frame target){
            if(auto* human=actors->camera_target_human(target)){
                auto& control=world->template human_loading_owner<Composition>(*human).control();
                set_human_camera_amount(*human,0.f,&control);
            }
        };
    }
private:
    Control& control_;World& world_;Actors& actors_;SessionActivationState& session_;const std::int32_t& local_player_;
};

} //namespace hd2::game
#endif
