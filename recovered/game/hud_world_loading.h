#ifndef HD2_GAME_HUD_WORLD_LOADING_H
#define HD2_GAME_HUD_WORLD_LOADING_H

#include "game/hud_actor_identity_view.h"
#include "game/session_activation.h"

namespace hd2::game {

//Borrow missionA8/native34 objective records directly. Capture the descriptor
//once per map-binding pass, then observe its actual vector on every count/read.
//No descriptor copy or separate objective ownership/index is created here.
class HudMissionObjectiveView {
public:
    explicit HudMissionObjectiveView(const frontend::GameDataNode* mission):mission_(mission){}
    std::size_t size()const{return mission_?mission_->related.size():0;} //004A7520/004A7680 nullA8 gate.
    frontend::GameDataNode* operator[](std::size_t index)const{
        if(!mission_)throw std::logic_error("HUD objective access without its actual mission descriptor");
        return mission_->related.at(index).get();
    }
private:
    const frontend::GameDataNode* mission_;
};

//Actual HUD scene/map/timer borrowers. Identity slots are SAME Scene2 globals
//0089B3B0/B4 (including reserved32/holes), NOT HUD's two selection pointers.
//Zone37 construction/groupA4 remains outside the supported offline loading path.
template<class SceneActors,class Tables>class HudWorldLoading {
public:
    using Frame=typename SceneActors::Frame;using Objective=frontend::GameDataNode;
    HudWorldLoading(SceneActors& actors,SessionActivationState& session,Tables& tables):actors_(actors),session_(session),tables_(tables){}
    Frame scene_frame()const{return actors_.world_scene_frame();}
    auto hud_actor_identities(){return HudActorIdentityView<SceneActors>(actors_);}
    auto map_objectives()const{return HudMissionObjectiveView(session_.mission);}
    void* navigation_context()const{return &tables_;} //004B4980 stores game54:00523AB0's SAME two datasets, not collision tree/grid.
    template<class Actor>std::int32_t map_zone_group(const Actor&)const{
        throw std::runtime_error("HUD map zone requires its actual type37 groupA4 owner");
    }
private:
    SceneActors& actors_;SessionActivationState& session_;Tables& tables_;
};

} //namespace hd2::game
#endif
