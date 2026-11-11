#ifndef HD2_HOST_GAME_UI_PRESENTATION_H
#define HD2_HOST_GAME_UI_PRESENTATION_H

#include "host/game_world_loading.h"
#include "host/game_effects_fade_render.h"

namespace hd2::host {
//Source presentation endpoints over ONE containing World. This does not claim
//the still-missing full HUD/FPV frame receivers or supply a frame coordinator.
//The caller must retire this borrower before Owners::close or World retirement.
class GameUiPresentation {
public:
    GameUiPresentation(world_loading::World& world,world_loading::World::Parent& parent,
                       GameOwnerBackend& backend,Graphics& graphics)
        :world_(world),d_(world.owners().dependencies()),parent_(parent),backend_(backend),graphics_(graphics),fade_(world.owners(),graphics) {
        if(&backend_!=&parent_.backend()||&backend_.presentation_graphics()!=&graphics_||
           &backend_.presentation_startup()!=&d_.startup||!parent_.constructed()||
           d_.scene.frames().world_scene_frame()!=parent_.scene()->frame()||
           &d_.request!=&parent_.request())throw std::logic_error("UI requires its SAME World/session owners");
    }
    GameUiPresentation(const GameUiPresentation&)=delete;
    GameUiPresentation& operator=(const GameUiPresentation&)=delete;
    void before_hud(){require_open();d_.hud.render_prepass(d_.camera);} //0049B9B0.
    void render_hud(){require_open();d_.hud.render(d_.camera,d_.startup.application_flags(),*this);} //0049B9F0.
    void advance_mission_text(std::uint32_t elapsed){require_open();d_.subtitles.tick(elapsed);} //0048F2D0.
    void render_mission_text(){require_open();d_.subtitles.draw(d_.startup.profile_settings().bytes[0x3a]!=0,backend_);} //0048F390.
    void render_effects_overlay(){require_open();d_.startup.frame_text_overlay().render();} //006E8490.
    bool has_game()const{return true;}
    std::uint32_t game_status()const{return d_.session.status;}
    std::uint32_t game_word24()const{return d_.session.words.get<std::uint32_t>(0x24);}
    bool suppress_hud()const{return d_.startup.scene_presentation_flags().suppress_hud;}
    bool full_viewport()const{return d_.startup.scene_presentation_flags().full_viewport;}
    bool debug_enabled()const{return d_.startup.scene_presentation_flags().debug;}
    bool hud_enabled(){require_open();return parent_.hud()->enabled();}
    CursorViewport graphics_viewport(){
        const auto* viewport=graphics_.methods->viewport(&graphics_);
        if(!viewport)throw std::runtime_error("UI callback requires actual graphics viewport");
        return *viewport;
    }
    void graphics_viewport(const CursorViewport& value){
        if(graphics_.methods->set_viewport(&graphics_,&value)<0)throw std::runtime_error("Cannot set UI callback viewport");
    }
    std::uint32_t graphics_width(){return graphics_.methods->width(&graphics_);}
    std::uint32_t graphics_height(){return graphics_.methods->height(&graphics_);}
    void render_game_fade(){require_open();game::render_session_fade(d_.session,fade_);} //0069AE10.
    void control_after_scene(){require_open();world_.modules().game_effects().render_after_scene(fade_);} //00486C10.
    void update_first_person_opacity(){require_open();d_.first_person.update_opacity(d_.startup.application_flags());}
    void advance_injury_message(std::uint32_t elapsed){require_open();world_.owners().hud_panels().small().update_injury_message(elapsed);}
    void update_carnage(){require_open();world_.owners().hud_panels().small().update_carnage(d_.scene.frames().selected_human_count());}
    void update_hud_timers(){
        require_open();auto& panels=world_.owners().hud_panels();
        panels.timers().update(full_viewport(),[&](game::ActorBase<world_loading::Frames>* actor){
            //Membership must precede RTTI or any borrowed actor dereference.
            GameSceneActors::Instance* found=nullptr;
            for(std::size_t n=0;n<d_.scene.frames().scene_actor_identity_count();++n){
                auto* candidate=d_.scene.frames().scene_actor_identity(static_cast<std::int32_t>(n));
                if(candidate && &candidate->base()==actor){found=candidate;break;}
            }
            if(!found)throw std::logic_error("HUD explosive borrower is no longer in the actual World");
            auto* explosive=dynamic_cast<game::ExplosivePlaceSceneActor<world_loading::Frames>*>(found);
            if(!explosive)throw std::logic_error("HUD timer borrower is not an actual ExplosivePlace");
            return std::make_pair(explosive->field94,explosive->countdown());
        });
    }
    //004B6720 remains UI-owned work. Explicit rejection prevents command-mode
    //render from pretending the positive route/selection geometry is supplied.
    [[noreturn]]void command_before_scene(){throw std::runtime_error("P22-UI requires actual004B6720 command geometry receivers");}
private:
    void require_open(){
        if(!parent_.constructed())throw std::logic_error("UI parent SessionOwner is retired");
        (void)world_.owners().dependencies();
    }
    world_loading::World& world_;world_loading::Dependencies& d_;
    world_loading::World::Parent& parent_;GameOwnerBackend& backend_;Graphics& graphics_;GameEffectsFadeRender fade_;
};
//The pre-scene volume/procedural pipeline and debug geometry remain P22-RENDER.
//A caller must supply the actual receiver; no empty successful service is made.
class GameUiRenderStages {
public:
    virtual ~GameUiRenderStages()=default;
    virtual void render_before_scene()=0;
    virtual void render_debug_geometry()=0;
};
class GameUiSceneCallbacks final:public GameSceneCallbacks {
public:
    GameUiSceneCallbacks(GameUiPresentation& ui,GameUiRenderStages& render)
        :ui_(ui),render_(render){}
    std::uint32_t dispatch_game_scene(std::uint32_t stage)override{return game::scene_callback(stage,*this);}
    bool has_game()const{return ui_.has_game();}auto game_status()const{return ui_.game_status();}
    auto game_word24()const{return ui_.game_word24();}
    bool suppress_hud()const{return ui_.suppress_hud();}bool full_viewport()const{return ui_.full_viewport();}
    bool debug_enabled()const{return ui_.debug_enabled();}bool hud_enabled(){return ui_.hud_enabled();}
    void before_hud(){ui_.before_hud();}void render_hud(){ui_.render_hud();}
    auto graphics_viewport(){return ui_.graphics_viewport();}void graphics_viewport(const CursorViewport& value){ui_.graphics_viewport(value);}
    auto graphics_width(){return ui_.graphics_width();}auto graphics_height(){return ui_.graphics_height();}
    void render_mission_text(){ui_.render_mission_text();}void render_game_fade(){ui_.render_game_fade();}
    void render_effects_overlay(){ui_.render_effects_overlay();}
    void control_before_scene(){render_.render_before_scene();}
    void control_after_scene(){ui_.control_after_scene();}void render_debug(){render_.render_debug_geometry();}
    void render_network_overlay()noexcept {} //00724CB0 is an actual RET in the frozen client.
private:
    GameUiPresentation& ui_;GameUiRenderStages& render_;
};
}
#endif
