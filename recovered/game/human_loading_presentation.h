#ifndef HD2_GAME_HUMAN_LOADING_PRESENTATION_H
#define HD2_GAME_HUMAN_LOADING_PRESENTATION_H

#include <cstdint>
#include <stdexcept>

namespace hd2::game {

//Source presentation bridge shared by SAME Human DD0/state/inventory and
//Players control. Dependencies borrows mode/local_player/difficulty, shared
//HumanPresentationShared, player_modes, FPV composition, first_person_players,
//camera, Players module and HUD scene owners. It may be forward-declared while
//their mutually borrowing template types are formed; construct it only with
//the completed actual dependency owner. No default endpoint or readiness flag.
//Required inventory_changed remains the real0049FBF0 receiver, never success.
template<class Dependencies>class HumanLoadingPresentation {
public:
    explicit HumanLoadingPresentation(Dependencies& dependencies):dependencies_(dependencies){}
    HumanLoadingPresentation(const HumanLoadingPresentation&)=delete;
    HumanLoadingPresentation& operator=(const HumanLoadingPresentation&)=delete;
    void acquire(){dependencies_.shared.acquire();}
    void release()noexcept{dependencies_.shared.release();}
    const auto& settings()const{return dependencies_.shared.settings();}
    template<class Player,class Human>void set_player_mode(Player& state,Human& human,std::int32_t value,std::uint32_t mode){
        dependencies_.player_modes.set_player_mode(state,human,value,mode);
    }
    template<class Human>bool human_selected_suppressed(Human& human){ //005A63F0; native AL only.
        if((dependencies_.mode&7u)==7u||!human.loading_state||human.properties.field19c!=1)return false;
        if(human.loading_state->words().template get<std::uint32_t>(0x48)!=1)
            throw std::runtime_error("Selected nonoffline Human requires its actual state virtual64 receiver");
        return human.loading_state->words().template get<std::uint8_t>(0x70)!=0; //005A4240 Player virtual64.
    }
    void reset_player_camera(std::uint32_t){ //0046C370 virtual40; ordinary argument0 has no producer side effect.
        auto& view=dependencies_.fpv.view();
        dependencies_.camera.refresh_current_loading_target(dependencies_.players.control(),view.enabled(),view.secondary_enabled(),
            [&](auto& human){return dependencies_.first_person_players.control_sector(human);},dependencies_.first_person_players.camera_retirement());
    }
    void bind_first_person_player(){dependencies_.fpv.view().bind_player(dependencies_.mode);} //004922A0.
    void refresh_first_person_visibility(){dependencies_.fpv.view().update_player_visibility();} //0045DAC0.
    template<class Human>void hud_add_human(Human* human,std::int32_t index){
        (void)dependencies_.hud.add_human(human,index,dependencies_.mode,dependencies_.difficulty,dependencies_.local_player);
    }
    template<class Human>void hud_remove_human(Human* human,std::int32_t index){(void)dependencies_.hud.remove_human(human,index);}
    template<class Human>void offline_player_close(Human& human){dependencies_.players.offline_player_close(human);} //004593E0.
    template<class Human>void leave_player_controlled_car(Human& human,std::uint32_t){ //0053BC50's actual kind4/driver0 gate.
        if(human.field160&&human.field160->base().type()==4&&human.field164==0)
            throw std::runtime_error("Player close requires actual occupied Car004D9280 receiver");
    }
    void first_person_request(std::uint32_t mode,std::int32_t animation,std::uint32_t argument){dependencies_.fpv.view().request(mode,animation,argument);}
    void hud_inventory_changed(){dependencies_.inventory_changed();} //Required actual0049FBF0, supplied by containing HUD consumer.
private:
    Dependencies& dependencies_;
};

} //namespace hd2::game
#endif
