#ifndef HD2_HOST_GAME_SESSION_LIFECYCLE_H
#define HD2_HOST_GAME_SESSION_LIFECYCLE_H

#include "host/game_blast_waves.h"
#include "host/game_ui_presentation.h"
#include "host/game_session_frames.h"
#include "game/session_lifecycle.h"
#include <cstdio>
#include <exception>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace hd2::host {
//Concrete P22-LIFE seam for P22-OWN. Parent/Startup/catalogue/suffix/controller
//outlive this object. It owns exactly ONE existing loading World and all the
//lifecycle adapters over it, never a duplicate SessionOwner/roster/clock.
//Construction DOES NOT load/activate the application. Caller must retire frame
//coordinators borrowing world() before close(), and close before Parent reset.
template<class Controller>
class GameSessionLifecycle final:public GameSessionFileFinish {
public:
    using Parent=world_loading::World::Parent;
    using Activation=GameSessionActivation<Controller>;
    using Overlay=std::remove_reference_t<decltype(std::declval<StartupSession&>().frame_text_overlay())>;
    GameSessionLifecycle(Parent& parent,GameOwnerBackend& backend,StartupSession& startup,
            MenuDriver& driver,HMODULE module,Graphics& graphics,FrameClockPlatform& clock,
            const frontend::PreMissionItemCatalogue& catalogue,const std::string& suffix,
            game::SessionActivationState& state,Controller* controller)
        :parent_(parent),startup_(startup),boundary_(state,*this){
        if(state.loaded)throw std::logic_error("A new lifecycle cannot adopt another World's loaded14 publication");
        //Unique members unwind the exact constructed prefix on every throw.
        loading_=std::make_unique<GameMissionLoadingSession>(parent,backend,startup,driver,module,graphics,clock,catalogue,suffix,state);
        activation_=std::make_unique<Activation>(*loading_,parent,backend,startup,graphics,module,clock,state,controller);
        blast_=std::make_unique<GameBlastWaves>(loading_->world().owners());
        ui_=std::make_unique<GameUiPresentation>(loading_->world(),parent,backend,graphics);
        loading_->loader().bind_finish(*this); //Last: every mandatory receiver now exists.
    }
    ~GameSessionLifecycle()override{
        try{close();}catch(const std::exception& error){std::fprintf(stderr,"Session lifecycle cleanup failed: %s\n",error.what());}
        catch(...){std::fputs("Session lifecycle cleanup failed with a nonstandard exception\n",stderr);}
        //A destructor cannot propagate. Remaining actual owners perform their
        //existing guarded RAII cleanup while Parent/engine/process still live.
    }
    GameSessionLifecycle(const GameSessionLifecycle&)=delete;
    GameSessionLifecycle& operator=(const GameSessionLifecycle&)=delete;
    bool activate(){
        require_open();return boundary_.activate();
    }
    bool finish(){
        require_open();return boundary_.finish();
    }
    void finish_for_file_unload()override{(void)finish();}
    bool process_finish_complete()const override{
        const auto& p=boundary_.activation_progress();return p.resources.entered&&p.finished;
    }
    void unload(){
        require_open();boundary_.unload();
    }
    void close(){
        if(!loading_)return;
        unload(); //If mandatory retirement fails, retain the graph for explicit retry/diagnosis.
        loading_->loader().unbind_finish(*this);
        ui_.reset();blast_.reset();activation_.reset();loading_.reset();
    }
    void bind_presentation(GameUiRenderStages& render){
        require_open();if(callbacks_)throw std::logic_error("Session presentation is already bound");
        if(boundary_.activation_progress().finished)throw std::logic_error("Cannot bind presentation to a retired session prefix");
        auto callbacks=std::make_unique<GameUiSceneCallbacks>(*ui_,render);
        parent_.scene()->bind_callbacks(*callbacks);
        callbacks_=std::move(callbacks);
    }
    //Pages use the EXISTING process registry/font owner. Retire the real page
    //wrappers before World/HUD fonts; process overlay itself survives sessions.
    game::FrameTextPage& make_overlay_page(std::uint32_t flags,std::optional<std::string> name={}){
        require_open();if(boundary_.activation_progress().finished)throw std::logic_error("Cannot register a page after session finish");
        auto page=startup_.frame_text_overlay().make_page(flags,std::move(name));
        auto& result=page->page();pages_.push_back(std::move(page));return result;
    }
    auto& world(){require_open();return loading_->world();}
    auto& presentation(){require_open();if(boundary_.activation_progress().finished)throw std::logic_error("Presentation prefix is retired");return *ui_;}
    auto& blast_waves(){require_open();return *blast_;}
    const auto& selection()const{require_open();return activation_->selection();}
    template<class Runtime>GameSessionFrames<Runtime>& bind_frames(GameSessionFrameExternal& external,Runtime& runtime){
        require_open();if(frames_)throw std::logic_error("Session frame coordinator is already bound");
        if(frame_retirement_started_||boundary_.activation_progress().finished)throw std::logic_error("Cannot bind frames to a retiring session");
        auto frames=std::make_unique<GameSessionFrames<Runtime>>(loading_->world(),parent_,activation_->selection(),*blast_,external,runtime);
        auto& result=*frames;frames_=std::move(frames);return result; //Publish only a complete same-owner composition.
    }
    void retire_frames()noexcept{if(frames_){frames_->retire();frames_.reset();}}
    void input(const std::uint32_t* first,const std::uint32_t* second){require_frames().input(first,second);}
    void fixed_update(std::uint32_t elapsed){require_frames().fixed_update(elapsed);}
    void update(std::uint32_t elapsed){require_frames().update(elapsed);}
    const auto& activation_progress()const noexcept{return boundary_.activation_progress();}
    std::exception_ptr rollback_failure()const noexcept{return boundary_.rollback_failure();}
    std::exception_ptr activation_flag_failure()const{require_open();return activation_->activation_flag_failure();}
private:
    friend class game::SessionLifecycleBoundary<GameSessionLifecycle>;
    bool activate_prefix(game::SessionActivationProgress& progress){return activation_->activate(&progress);}
    bool finish_prefix(game::SessionActivationProgress& progress){return activation_->finish_prefix(progress,*blast_);}
    void discard_loading_screen(){loading_->discard_loading_screen();}
    void unload_files(){loading_->loader().unload_loaded_mission();}
    void rethrow_cleanup_failure(){loading_->world().scene().frames().rethrow_actor_cleanup_failure();}
    void require_open()const{if(!loading_)throw std::logic_error("Session lifecycle is closed");}
    GameSessionFrameCallbacks& require_frames(){require_open();if(!frames_)throw std::logic_error("Actual session frame coordinator is not bound");return *frames_;}
    void retire_presentation(){
        frame_retirement_started_=true;retire_frames(); //Also on activation rollback, before any World borrower is retired.
        parent_.scene()->suspend_callback(); //Engine can no longer call UI or application during retirement/reset.
        if(callbacks_){parent_.scene()->unbind_callbacks(*callbacks_);callbacks_.reset();}
        pages_.clear(); //Actual unregister006E83B0 before page/row storage dies.
    }
    Parent& parent_;StartupSession& startup_;
    game::SessionLifecycleBoundary<GameSessionLifecycle> boundary_;
    std::unique_ptr<GameMissionLoadingSession> loading_;
    std::unique_ptr<Activation> activation_;std::unique_ptr<GameBlastWaves> blast_;
    std::unique_ptr<GameUiPresentation> ui_;std::unique_ptr<GameUiSceneCallbacks> callbacks_;
    std::unique_ptr<GameSessionFrameCallbacks> frames_;bool frame_retirement_started_=false;
    std::vector<std::unique_ptr<typename Overlay::PageOwner>> pages_;
};
} //namespace hd2::host
#endif
