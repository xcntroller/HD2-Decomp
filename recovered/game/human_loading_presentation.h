#ifndef HD2_GAME_HUMAN_LOADING_PRESENTATION_H
#define HD2_GAME_HUMAN_LOADING_PRESENTATION_H

#include "game/world_loading_dependency_link.h"
#include <cstdint>
#include <stdexcept>

namespace hd2::game {

//Source presentation bridge shared by SAME Human DD0/state/inventory and
//Players control. Dependencies borrows mode/local_player/difficulty, shared
//HumanPresentationShared, player_modes, FPV composition, first_person_players,
//camera, Players module and HUD scene owners. It may be forward-declared while
//their mutually borrowing template types are formed. Bind the completed actual
//dependency owner before use; unbound calls reject. No default endpoint or readiness flag.
//Inventory change uses the SAME HUD's actual0049FBF0 visibility gate.
template<class Dependencies>class HumanLoadingPresentation {
public:
    HumanLoadingPresentation()=default;
    explicit HumanLoadingPresentation(Dependencies& dependencies):dependencies_(dependencies){}
    void bind_dependencies(Dependencies& dependencies){dependencies_.bind(dependencies);}
    void unbind_dependencies(Dependencies& dependencies)noexcept{dependencies_.unbind(dependencies);}
    bool dependencies_bound()const noexcept{return dependencies_.bound();}
    HumanLoadingPresentation(const HumanLoadingPresentation&)=delete;
    HumanLoadingPresentation& operator=(const HumanLoadingPresentation&)=delete;
    void acquire(){dependencies().shared.acquire();}
    void release()noexcept{dependencies().shared.release();}
    const auto& settings()const{return dependencies().shared.settings();}
    template<class Player,class Human>void set_player_mode(Player& state,Human& human,std::int32_t value,std::uint32_t mode){
        dependencies().player_modes.set_player_mode(state,human,value,mode);
    }
    template<class Human>bool human_selected_suppressed(Human& human){ //005A63F0; native AL only.
        if((dependencies().mode&7u)==7u||!human.loading_state||human.properties.field19c!=1)return false;
        return human.loading_state->selection_suppressed();
    }
    void reset_player_camera(std::uint32_t){ //0046C370 virtual40; ordinary argument0 has no producer side effect.
        auto& view=dependencies().fpv.view();
        dependencies().camera.refresh_current_loading_target(dependencies().players.control(),view.enabled(),view.secondary_enabled(),
            [&](auto& human){return dependencies().first_person_players.control_sector(human);},dependencies().first_person_players.camera_retirement());
    }
    void bind_first_person_player(){dependencies().fpv.view().bind_player(dependencies().mode);} //004922A0.
    void refresh_first_person_visibility(){dependencies().fpv.view().update_player_visibility();} //0045DAC0.
    template<class Human>void hud_add_human(Human* human,std::int32_t index){
        (void)dependencies().hud.add_human(human,index,dependencies().mode,dependencies().human_difficulty(),dependencies().local_player);
    }
    template<class Human>void hud_remove_human(Human* human,std::int32_t index){(void)dependencies().hud.remove_human(human,index);}
    template<class Human>void offline_player_close(Human& human){dependencies().players.offline_player_close(human);} //004593E0.
    template<class Human>void leave_player_controlled_car(Human& human,std::uint32_t){ //0053BC50's actual kind4/driver0 gate.
        if(human.field160&&human.field160->base().type()==4&&human.field164==0)
            throw std::runtime_error("Player close requires actual occupied Car004D9280 receiver");
    }
    void first_person_request(std::uint32_t mode,std::int32_t animation,std::uint32_t argument){dependencies().fpv.view().request(mode,animation,argument);}
    void hud_inventory_changed(){dependencies().hud.inventory_changed();} //Actual0049FBF0 over the SAME loaded HUD inventory root.
private:
    Dependencies& dependencies()const{return dependencies_.get();}
    WorldLoadingDependencyLink<Dependencies> dependencies_;
};

} //namespace hd2::game
#endif
