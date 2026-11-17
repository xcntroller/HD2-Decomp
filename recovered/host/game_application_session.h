#ifndef HD2_HOST_GAME_APPLICATION_SESSION_H
#define HD2_HOST_GAME_APPLICATION_SESSION_H
#include "host/game_session_lifecycle.h"
#include "game/application_mission.h"
#include "game/session_warning.h"

namespace hd2::host {
//Actual kind7 factory product. All graph members borrow the application's
//existing parent/status and the containing controller/process/device/clock.
//FRAMEc36bba8 is not in this implementation base: its coordinator must be
//imported before completing the four explicitly guarded forwarding operations.
//These guards are failures, not installed successful callback receivers.
template<class Controller>
class GameApplicationSession final:public game::ApplicationMissionSession {
public:
    GameApplicationSession(GameApplicationState& application,StartupSession& startup,
            MenuDriver& driver,HMODULE module,Graphics& graphics,
            FrameClockPlatform& clock,Controller& controller)
        :startup_(startup),observations_(std::make_shared<io::ResourceObservations>()),
         lifecycle_(checked_parent(application,controller),application.session_owner().backend(),startup,
             driver,module,graphics,clock,startup.pre_mission().resources().catalogue,
             startup.item_model_suffix(),application.activation_state(),&controller){}
    bool activate()override{
        auto files=startup_.pre_mission_files().observe_scope(observations_);
        auto caches=startup_.loaded_resources().observe_scope(observations_);
        return io::observe_data_operation(observations_,"P22-OWN actual006989B0 composition",[&]{
            try{
                lifecycle_.bind_presentation(); //Real same-World UI/RENDER, including loading callbacks.
                const auto result=lifecycle_.activate(); //Actual006989B0 ->006CA780, not load_observed in isolation.
                snapshot();
                if(result)missing_frames(); //Do not let the application schedule an unbound session.
                return false;
            }catch(...){snapshot();throw;}
        });
    }
    void close()override{
        auto files=startup_.pre_mission_files().observe_scope(observations_);
        auto caches=startup_.loaded_resources().observe_scope(observations_);
        lifecycle_.close(); //FRAME retirement must be inserted BEFORE this on import.
    }
    void input(std::uint32_t*,std::uint32_t*)override{missing_frames();}
    void fixed_update(std::uint32_t)override{missing_frames();}
    void update(std::uint32_t)override{missing_frames();}
    void render()override{
        auto files=startup_.pre_mission_files().observe_scope(observations_);
        auto caches=startup_.loaded_resources().observe_scope(observations_);
        lifecycle_.render();
    }
    std::uint32_t dispatch_scene(std::uint32_t stage)override{return lifecycle_.dispatch_scene(stage);}
    const auto& observations()const noexcept{return observations_;}
    auto& lifecycle()noexcept{return lifecycle_;}
    void show_session_warning(std::uint32_t caption,bool zero_argument)override{
        const auto message=game::format_session_warning(startup_.application_localize(caption),zero_argument);
        if(startup_.application_flags()&2u)
            lifecycle_.world().owners().dependencies().hud.append_message(message.c_str(),{{255,255,255}});
        else startup_.append_game_console(message); //Actual0069BF70 route, no configuration170 log gate.
    }
private:
    static auto& checked_parent(GameApplicationState& application,Controller& controller){
        if(controller.active()!=&application)
            throw std::logic_error("Mission factory requires its actual installed controller state");
        return application.session_owner();
    }
    [[noreturn]] static void missing_frames(){
        throw std::runtime_error("P22-FRAME import required: actual GameSessionFrames input/fixed/update and Runtime producers are not bound");
    }
    void snapshot()noexcept{
        try{observe_world_data(lifecycle_.world());}catch(...){observations_->note_drop();}
    }
    StartupSession& startup_;
    std::shared_ptr<io::ResourceObservations> observations_;
    GameSessionLifecycle<Controller> lifecycle_;
};
}
#endif
