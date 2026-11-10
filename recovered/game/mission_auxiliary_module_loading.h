#ifndef HD2_GAME_MISSION_AUXILIARY_MODULE_LOADING_H
#define HD2_GAME_MISSION_AUXILIARY_MODULE_LOADING_H

#include "frontend/module_registration.h"
#include "game/mission_tracks.h"
#include "frontend/cleanup.h"
#include <optional>

namespace hd2::game {

//Module0 owns real stored definitions. There is NO playback producer/API in
//this loading owner:00401F40 reaches its genuinely empty20/24 playback array.
//FRM_WatchTrack/playback/update remain unsupported by script command dispatch.
class MissionTracksLoading {
public:
    MissionTracksLoading(frontend::ClientModuleRegistry& registry,frontend::ClientModuleRegistration& registration)
        :registration_(registry,registration,0,"Track module"){}
    auto& registration(){return registration_.get();}auto& definitions(){return definitions_;}
    template<class Files>void finish_loading(bool authority,Files& files,const std::string& resource) {
        if(!authority)(void)definitions_.load(files,resource); //00402020, optional file.
    }
    void activate(bool){} //004010B0 genuine RET4.
    void finish(bool){} //00401F40 cold playback only, NOT a claimed RET body.
    void unload(bool authority){if(!authority)definitions_.clear();} //00402030→00402040.
private:
    frontend::ClientModuleBinding registration_;MissionTracks definitions_;
};

//Module8's actual changed-visual20 records. Named group10 parsing/creation and
//save restoration are not supplied; those groups stay cold, with no successful
//group producer. Direct visual changes and their actual finish/unload lifetimes
//are concrete. Engine owners retain/release the same visual, never a copy.
template<class Frames>class MissionLightmapGroupsLoading {
public:
    using Frame=typename Frames::Frame;
    using Owner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    MissionLightmapGroupsLoading(frontend::ClientModuleRegistry& registry,frontend::ClientModuleRegistration& registration,Frames& frames)
        :registration_(registry,registration,8,"LMapsGrp"),frames_(frames){}
    ~MissionLightmapGroupsLoading(){try{finish(false);}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}}
    MissionLightmapGroupsLoading(const MissionLightmapGroupsLoading&)=delete;
    MissionLightmapGroupsLoading& operator=(const MissionLightmapGroupsLoading&)=delete;
    auto& registration(){return registration_.get();}
    std::size_t changed_count()const{return changes_.size();}
    void finish_loading(bool){} //004115E0 genuine RET4.
    void activate(bool){} //00411600 genuine RET4.
    void set_visual_group(Frame frame,std::uint32_t group) { //0048D270, reverse lookup, first original value retained.
        if(!frame)return;
        for(auto n=changes_.size();n;)if(changes_[--n].frame.get()==frame) {
            frames_.set_visual_lightmap_group(frame,group);frames_.update_scene_object(frame);
            if(group==changes_[n].original) {
                changes_[n].frame.reset();
                if(n+1!=changes_.size())changes_[n]=std::move(changes_.back());
                changes_.pop_back();
            }
            return;
        }
        auto owner=frames_.retain_actor_frame(frame);const auto original=frames_.visual_lightmap_group(frame);
        changes_.push_back(Change{std::move(owner),original});
        frames_.set_visual_lightmap_group(frame,group);frames_.update_scene_object(frame);
    }
    void finish(bool) { //0048CB60→virtual2C(false)/0048C790; authority is ignored.
        frontend::CleanupErrors errors;
        for(auto n=changes_.size();n;) {
            auto& change=changes_[--n];
            errors.attempt([&]{frames_.set_visual_lightmap_group(change.frame.get(),change.original);frames_.update_scene_object(change.frame.get());});
        }
        //Native restores ALL visuals before releasing ANY retained reference.
        while(!changes_.empty())changes_.pop_back();
        errors.rethrow();
    }
    void unload(bool authority) { //0048CB70: false destroys records WITHOUT restoring visual state.
        if(!authority)while(!changes_.empty())changes_.pop_back();
    }
private:
    struct Change{Owner frame;std::uint32_t original;};
    frontend::ClientModuleBinding registration_;Frames& frames_;std::vector<Change> changes_;
};

//0048E1F0 constructs an empty10/14/18 array. No texture-shift record producer
//or simulation is supplied in the supported loading path. The native tails,
//activation and finish are RETs;0048E190(false) clears that cold array only.
class MissionTextureShiftLoading {
public:
    MissionTextureShiftLoading(frontend::ClientModuleRegistry& registry,frontend::ClientModuleRegistration& registration)
        :registration_(registry,registration,10,"TShift"){}
    auto& registration(){return registration_.get();}
    void finish_loading(bool){} //004115E0.
    void activate(bool){} //0048E180.
    void finish(bool){} //0048E1D0.
    void unload(bool){} //0048E190, cold array; no positive-state producer/API.
private:
    frontend::ClientModuleBinding registration_;
};
} //namespace hd2::game
#endif
