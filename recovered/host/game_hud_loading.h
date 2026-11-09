#ifndef HD2_HOST_GAME_HUD_LOADING_H
#define HD2_HOST_GAME_HUD_LOADING_H
#include "host/game_owner.h"
namespace hd2::host {
//Concrete source composition using ONE scene-actor/frame/movie backend and
//StartupSession's actual process owners. Containing world must bind its real
//Players/Items/Camera/World and retire this BEFORE those owners/HUD/scene.
//No automatic activation or successful missing World endpoint is supplied.
template<class Players,class Items,class Camera,class World>
class GameHudLoading {
public:
    using Panels=game::HudPanelsLoading<GameSceneActors,GameOwnerBackend,GameOwnerBackend::Hud,
        Players,Items,frontend::OpeningTextStyles,Camera,World>;
    using Scene=game::HudSceneLoading<GameSceneActors,GameOwnerBackend,GameOwnerBackend::Hud,Panels>;
    using Borrowers=typename Panels::Borrowers;
    GameHudLoading(GameSceneActors& frames,GameOwnerBackend& factory,StartupSession& startup,
                   GameOwnerBackend::Hud& hud,Players& players,Items& items,Camera& camera,World& world)
        :panels_(frames,factory,hud,players,items,startup.opening_text_styles(),camera,world,
                 startup.hud_signal_tables(),startup.hud_compass_state()),
         scene_(frames,factory,hud,panels_,startup.hud_click_sound()){}
    bool bind(const std::string& mission,std::uint32_t mode,bool editor_kind3,const Borrowers& borrowers){
        return scene_.bind(mission,mode,editor_kind3,borrowers);
    }
    bool close(){return scene_.close();}
    Panels& panels(){return panels_;}Scene& scene(){return scene_;}
private:
    Panels panels_; //Must outlive scene_, including its failed-prefix cleanup.
    Scene scene_;
};
} //namespace hd2::host
#endif
