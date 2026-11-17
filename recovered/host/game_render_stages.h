#ifndef HD2_HOST_GAME_RENDER_STAGES_H
#define HD2_HOST_GAME_RENDER_STAGES_H
#include "host/game_ui_presentation.h"
#include "io/data_operation.h"
namespace hd2::host {
//Concrete RENDER pre-scene service for the existing UI seam. Borrows SAME World;
//retire with its callbacks before lifecycle close. Does not own a second camera,
//texture cache, graphics device, clock, actor registry or presentation scheduler.
class GameRenderStages final:public GameUiRenderStages {
public:
    explicit GameRenderStages(world_loading::World& world):world_(world){}
    const world_loading::World* render_world_key()const noexcept override{return &world_;}
    void retire_command_special_model()override{
        auto& d=world_.owners().dependencies();
        io::observe_data_operation(d.scene.frames().resource_observations(),"004870A0 command modal special-model retirement",[&]{
            d.scene.effects().configure_special_model(false,nullptr,true);
        });
    }
    void render_before_scene()override{
        (void)world_.owners().dependencies();
        io::observe_data_operation(world_.scene().frames().resource_observations(),"00486BF0 pre-scene render",[&]{
            world_.modules().game_effects().render_before_scene();
        });
    }
    void render_debug_geometry()override{
        auto& d=world_.owners().dependencies();
        io::observe_data_operation(d.scene.frames().resource_observations(),"0079B760 debug geometry",[&]{
            d.startup.render_debug_skeleton().render(*this);
        });
    }
    auto debug_actor_count(){return world_.scene().frames().scene_actor_identity_count();}
    template<class Emit>bool visit_debug_geometry(std::size_t index,Emit&& emit){
        auto& d=world_.owners().dependencies();auto& frames=d.scene.frames();
        auto* actor=frames.scene_actor_command_identity(static_cast<std::int32_t>(index)); //00475D50, includes pending reserved IDs.
        if(!actor||actor->base().type()!=3)return false;
        auto& human=static_cast<world_loading::Human&>(*actor);
        if(game::human_script_suspended(human))return false;
        auto* control=d.first_players.inline_control(human).control();
        if(!control)return false; //Original null A0/DD0 gate.
        const auto& geometry=control->geometry();const auto model=geometry.model();
        if(!model)throw std::runtime_error("Debug Human geometry has no actual model08");
        if(frames.vertex_frame_epoch(model)!=frames.vertex_scene_epoch())return false;
        emit(geometry,frames);return true;
    }
    void debug_identity_transform(){
        world_.scene().frames().volume_world_transform({1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1});
    }
    void debug_draw_lines(const void* vertices,std::uint32_t count,const std::uint16_t* indices,
            const std::array<float,3>& rgb,std::uint32_t alpha){
        world_.scene().frames().debug_skeleton_lines(vertices,count,indices,rgb,alpha);
    }
private:
    world_loading::World& world_;
};
}
#endif
