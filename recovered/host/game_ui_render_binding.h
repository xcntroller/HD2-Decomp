#ifndef HD2_HOST_GAME_UI_RENDER_BINDING_H
#define HD2_HOST_GAME_UI_RENDER_BINDING_H

#include "game/world_loading_dependency_link.h"

namespace hd2::host {
namespace world_loading {class World;}

//Required RENDER services over the UI's SAME World. These are borrowers, not
//another scene/device/camera/effects owner. No successful unbound receiver.
class GameUiRenderStages {
public:
    virtual ~GameUiRenderStages()=default;
    virtual const world_loading::World* render_world_key()const noexcept=0;
    virtual void render_before_scene()=0;
    virtual void render_debug_geometry()=0;
    virtual void retire_command_special_model()=0; //004B5E40 -> GEffects44(false,NULL,true).
};

//Source lifetime guard shared by actual UI and its Scene dispatcher. The owner
//token is the dispatcher, so destruction of an older borrower cannot retract a
//new binding to the same render object. Native guard, not original game policy.
class GameUiRenderBinding {
public:
    explicit GameUiRenderBinding(const world_loading::World& world):world_(&world){}
    void bind(const void* owner,GameUiRenderStages& stages){
        if(!owner)throw std::invalid_argument("UI render binding needs its callback owner");
        if(stages_.bound())throw std::logic_error("UI render stages already bound");
        if(stages.render_world_key()!=world_)throw std::logic_error("UI/render stages require the SAME World");
        stages_.bind(stages);owner_=owner; //Publish only after all checks.
    }
    void unbind(const void* owner)noexcept{
        if(!owner||owner_!=owner)return;
        if(stages_.bound())stages_.unbind(stages_.get());
        owner_=nullptr;
    }
    GameUiRenderStages& get()const{return stages_.get();}
    GameUiRenderStages& get(const void* owner)const{
        if(!owner||owner_!=owner)throw std::logic_error("UI Scene callback render binding is retired");
        return stages_.get();
    }
    bool bound()const noexcept{return stages_.bound();}
private:
    const world_loading::World* world_;
    const void* owner_=nullptr;
    game::WorldLoadingDependencyLink<GameUiRenderStages> stages_;
};
} //namespace hd2::host
#endif
