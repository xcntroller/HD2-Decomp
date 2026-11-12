#ifndef HD2_GAME_FIRST_PERSON_PLAYERS_LOADING_H
#define HD2_GAME_FIRST_PERSON_PLAYERS_LOADING_H

#include "game/human_camera_state.h"
#include "game/human_camera_amount.h"
#include "game/session_activation.h"
#include "game/actor_seat_presentation.h"
#include <functional>
#include <optional>
#include <type_traits>

namespace hd2::game {

//Concrete source lookups for FirstPersonViewLoading: SAME Players control,
//per-Human world composition and Scene2/frame registry. No second roster, B0 or
//inline owner. World supplies its checked human_loading_owner<Composition>.
//Clear camera targets BEFORE world/actor retirement; its callback borrows them.
//The containing World binds the actual occupied-Actor facade before activation
//and keeps it bound through FPV and Actor retirement.
template<class HumanActor,class Control,class World,class Composition,class Actors>
class FirstPersonPlayersLoading {
public:
    using Human=HumanActor;using Frame=typename Actors::Frame;
    using AttachedActor=std::remove_pointer_t<decltype(std::declval<Human&>().field160)>;
    using SeatPresentation=ActorSeatPresentation<AttachedActor>;
    FirstPersonPlayersLoading(Control& control,World& world,Actors& actors,SessionActivationState& session,
            const std::int32_t& local_player):control_(control),world_(world),actors_(actors),session_(session),local_player_(local_player){}
    Human* selected_player(std::uint32_t mode){return control_.selected_player(mode);}
    Human* current_player()const{return control_.current_player();} //0045D1D0, never selected28.
    auto& inline_control(Human& human){return world_.template human_loading_owner<Composition>(human).control();}
    auto& state(Human& human){
        auto* state=world_.template human_loading_owner<Composition>(human).states().get();
        if(!state||human.loading_state!=state)throw std::runtime_error("First-person player lost its actual published Human B0");
        return *state;
    }
    const auto& velocity(Human& human){(void)world_.template human_loading_owner<Composition>(human);return human.activation.vector1b0;}
    std::int32_t health(Human& human){(void)world_.template human_loading_owner<Composition>(human);const auto word=human.properties.stats_ec[0];std::int32_t value;std::memcpy(&value,&word,4);return value;}
    std::uint32_t time()const{return session_.words.template get<std::uint32_t>(0x28);}
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
    bool attached_driver(Human& human)const{return human.field160&&seat_presentation().kind(*human.field160)==4&&human.field164==0;}
    void release_driver_input(Human& human){
        auto* car=dynamic_cast<typename World::Car*>(human.field160);
        if(!car||!car->loading)throw std::logic_error("Player close requires actual occupied Car lifetime");
        car->loading->driver_input(0,0);
    }
    void bind_seat_presentation(SeatPresentation& source){
        if(seats_)throw std::logic_error("First-person seat presentation already bound");seats_=&source;
    }
    void unbind_seat_presentation(SeatPresentation& source)noexcept{if(seats_==&source)seats_=nullptr;}
    SeatPresentation& seat_presentation()const{
        if(!seats_)throw std::logic_error("First-person seat presentation has no containing World binding");return *seats_;
    }
    void attached_first_person(Human& human,bool enabled){
        if(human.field160)seat_presentation().view(*human.field160,human.field164,enabled);
    }
    bool attachment_hides_human(Human& human)const { //0045DAC0's exact kind gate before virtual8C/bit10.
        if(human.field160)switch(seat_presentation().kind(*human.field160)){
        case 4:case 20:case 21:case 34:return (seat_presentation().flags(*human.field160,human.field164)&0x10u)!=0;
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
    SeatPresentation* seats_=nullptr; //Borrowed only from the SAME containing World.
};

} //namespace hd2::game
#endif
