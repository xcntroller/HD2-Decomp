#ifndef HD2_GAME_EFFECTS_MODULE_LOADING_H
#define HD2_GAME_EFFECTS_MODULE_LOADING_H

#include "frontend/module_registration.h"
#include "frontend/cleanup.h"
#include "game/effects_loading_process.h"
#include "game/effects_birds_loading.h"
#include "game/effects_loading_update.h"
#include "game/volumetric_lights.h"
#include "game/mesh_movie.h"

namespace hd2::game {

//GEffects1 loading/activation/finish over the actual shared process state,
//scene/frame store, files, shadow/fade queues, camera, FPV/profile and CRT random.
//Cold304 pose records,210 positive fades,2E0 special model and water-camera1AC
//have no source producer; no successful positive runtime/render handler exists.
template<class Frames,class Files,class Random,class Camera,class View,class Effects,class Startup>
class EffectsModuleLoading {
public:
    using Frame=typename Frames::Frame;using Process=EffectsLoadingProcess<Frame>;
    EffectsModuleLoading(frontend::ClientModuleRegistry& registry,frontend::ClientModuleRegistration& registration,
            Frames& frames,Files& files,Effects& effects,Process& process,Random& random,Camera& camera,View& view,Startup& startup,
            const std::uint32_t& mode,std::string resource,const float& far_clip)
        :registration_(registry,registration,1,"GEffects"),frames_(frames),files_(files),effects_(effects),process_(process),
         birds_(frames,random,far_clip),volumes_(frames),movie_(frames),camera_(camera),view_(view),startup_(startup),mode_(mode),resource_(std::move(resource)){
        if(process_.world||process_.water_camera1ac)throw std::logic_error("Retire actual GEffects world/camera borrowers first");
        if(&effects_.frames()!=static_cast<typename Effects::Services*>(&frames_)||&effects_.fade()!=&process_.fade||
           &effects_.environment()!=&process_.environment||&startup_.effects_loading_process()!=&process_)
            throw std::logic_error("GEffects requires its SAME frame registry/process environment/fade owners");
        process_.world=this;
    }
    ~EffectsModuleLoading(){close();}
    EffectsModuleLoading(const EffectsModuleLoading&)=delete;EffectsModuleLoading& operator=(const EffectsModuleLoading&)=delete;
    auto& registration(){return registration_.get();}auto& birds(){return birds_;}auto& volumes(){return volumes_;}auto& movie(){return movie_;}
    template<class Draw>void render_after_scene(Draw& draw){require_open();render_loading_effects_fade(process_,draw);} //00486C10.
    void reset(){ //00488B80's reached queues/scalars; unproduced210/304 genuinely empty.
        require_open();
        process_.colour2f4.fill(1);effects_.frame_fades().reset_loading();effects_.human_fades().clear();
        process_.flag220=false;process_.sun1b0=Frame{};process_.fields25c.fill(0);
    }
    void finish_loading(bool authority){ //00488560, presentation only.
        require_open();if(authority)return;
        unloaded_=false;reset();(void)volumes_.load(files_,resource_);volumes_.refresh(true);(void)movie_.load(files_,resource_);
    }
    void unload(bool authority){ //004885A0, presentation only, reset BEFORE volume/movie retirement.
        require_open();if(authority)return;
        reset();volumes_.clear();movie_.clear();unloaded_=true;
    }
    void activate(bool authority){ //004885D0.
        require_open();finished_=false;if(authority){birds_.load();return;}
        process_.field224=0;frames_.effects_enable_loading_sectors();reset();
        process_.sun1b0=frames_.find_scene_frame("mirasun",0xffff);
        if(process_.sun1b0){frames_.effects_link_root(process_.sun1b0);frames_.update_scene_object(process_.sun1b0);process_.sun_position10=frames_.actor_position(process_.sun1b0);}
        if(!(mode_&1u))birds_.load();
        (void)process_.water.load(files_,resource_);
        if(process_.water.enabled()){
            process_.water_camera1ac=Frame{};process_.water.initial_camera(frames_.effects_active_camera_parameters());
            if(process_.water.use_scene_water())frames_.effects_scene_water();
            constexpr std::array<const char*,4> names{{"a_nsea_","a_undw_","amb","sea_amb"}};
            for(unsigned pair=0;pair!=2;++pair){
                for(unsigned n=pair*2;n!=pair*2+2;++n){auto& frame=process_.water_frames238[n];frame=frames_.find_scene_frame(names[n],n<2?4u:2u);if(!frame)frame=frames_.resolve_scene_path(names[n]);}
                set_water_pair(pair); //Native sea visibility precedes the two light lookups.
            }
        }
        (void)process_.decorations.load(files_,resource_);
        process_.decorations.initialize(startup_.profile_settings().quality_byte(4)!=0,view_.enabled()||view_.secondary_enabled());
        process_.fade.reset_loading(false);process_.active248=true;
    }
    void finish(bool authority){ //004888C0; detached disable precedes the authority gate.
        require_open();camera_.detached().disable_for_loading();if(authority)return;
        frontend::CleanupErrors errors;
        //304 pose vector and special-frame2E0 are cold empty/null; native still
        //shows the actual lensflare. No positive camera/model receiver is supplied.
        errors.attempt([&]{effects_.lensflare_visible(true);});
        if(process_.sun1b0){
            errors.attempt([&]{const auto& p=process_.sun_position10;frames_.object_position(process_.sun1b0,{{p[0],p[1],p[2]}});});
            errors.attempt([&]{frames_.update_scene_object(process_.sun1b0);});
        }
        reset();errors.attempt([&]{birds_.clear();});
        if(process_.water.enabled()){
            errors.attempt([&]{if(process_.water_camera1ac)throw std::runtime_error("Water camera retirement requires its actual populated reflection-camera owner");});
            if(process_.water.use_scene_water())errors.attempt([&]{frames_.effects_scene_water();});
            errors.attempt([&]{water_name_visibility(process_.water.above_water_name(),true);});
            errors.attempt([&]{water_name_visibility(process_.water.below_water_name(),false);});
            errors.attempt([&]{set_water_frames();});process_.water_frames238.fill(Frame{});
            if(process_.water.restore_child_visibility())errors.attempt([&]{frames_.effects_restore_water_children(true);});
        }
        process_.decorations.clear();process_.fade.reset_loading(true);process_.active248=false;
        errors.attempt([&]{frames_.effects_scene_water();});process_.flag24c=false; //004863E0(false), actual scalar24C.
        errors.rethrow();finished_=true;
    }
    template<class Session,class Players>void update_loading(std::uint32_t elapsed,Session& session,Players& players){ //00487240, actual loading-screen+10 receiver.
        require_open();
        update_loading_effects(elapsed,frames_,process_,effects_,birds_,movie_,session,players,startup_);
    }
    void close()noexcept{
        if(process_.world!=this)return;
        frontend::CleanupErrors errors;if(!finished_)errors.attempt([&]{finish(false);});if(!unloaded_)errors.attempt([&]{unload(false);});
        process_.sun1b0=Frame{};process_.water_frames238.fill(Frame{});process_.world=nullptr;
        try{errors.rethrow();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
    }
    void finish_file_borrowers(){if(!finished_)finish(false);} //Authority birds are retired by the presentation receiver, even on a partial authority pass.
private:
    void require_open()const{if(process_.world!=this)throw std::logic_error("GEffects world has been retired");}
    void water_name_visibility(const std::string& name,bool show){if(!name.empty())if(const auto frame=frames_.find_scene_frame(name,0xffff))effects_.frame_visibility(frame,show,0xffff);}
    void set_water_pair(unsigned pair){for(unsigned n=pair*2;n!=pair*2+2;++n)if(const auto frame=process_.water_frames238[n])effects_.frame_visibility(frame,n%2==0,0xffff);}
    void set_water_frames(){set_water_pair(0);set_water_pair(1);}
    frontend::ClientModuleBinding registration_;Frames& frames_;Files& files_;Effects& effects_;Process& process_;
    EffectsBirdsLoading<Frames,Random> birds_;VolumetricLights<Frames> volumes_;MeshMovie<Frames> movie_;
    Camera& camera_;View& view_;Startup& startup_;const std::uint32_t& mode_;std::string resource_;bool finished_=true,unloaded_=true; //Cold construction has no activation prefix. Source destructor guards, not native readiness.
};
} //namespace hd2::game
#endif
