#ifndef HD2_GAME_HUD_INJURY_MOVIES_LOADING_H
#define HD2_GAME_HUD_INJURY_MOVIES_LOADING_H
#include "frontend/cleanup.h"
#include <array>
#include <cstdint>
#include <cstdio>
#include <optional>
#include <stdexcept>
#include <utility>
namespace hd2::game {
//004AD250/004AD310. Actual seven IShow streams, not frame/texture owners.
//Open failures release+04 only; successful teardown is18 stop/10 close/04
//release ascending. No decoding/playback or full-screen opening-video path.
template<class Services>
class HudInjuryMoviesLoading {
public:
    using Frame=typename Services::Frame;
    using MovieOwner=decltype(std::declval<Services&>().create_mesh_movie());
    explicit HudInjuryMoviesLoading(Services& services):services_(services) {}
    ~HudInjuryMoviesLoading(){try{clear();}catch(...){services_.record_actor_cleanup_failure(std::current_exception());}}
    HudInjuryMoviesLoading(const HudInjuryMoviesLoading&)=delete;
    HudInjuryMoviesLoading& operator=(const HudInjuryMoviesLoading&)=delete;
    void bind(Frame scene) {
        if(started_)throw std::logic_error("HUD injury movies rebound before teardown");
        if(!scene)throw std::runtime_error("HUD injury movies require actual HUD scene");
        started_=true;active_=Frame{};
        try {
            for(unsigned index=0;index!=movies_.size();++index) {
                auto owner=services_.create_mesh_movie();
                if(!owner)throw std::runtime_error("HUD injury IShow factory returned null");
                movies_[index].emplace(std::move(owner));
                std::array<char,260> path{};std::snprintf(path.data(),path.size(),"GameMovie\\AnimHit%u.bik",index+1);
                if(services_.open_mesh_movie(movies_[index]->get(),path.data(),0)!=0)movies_[index].reset();
            }
            visual_=services_.hud_find(scene,"injury",0xffff);
            if(!visual_)throw std::runtime_error("Required HUD injury artwork is missing");
            services_.hud_injury_prepare(visual_);services_.hud_group18(visual_);
            services_.object_visible(visual_,false);services_.update_scene_object(visual_);
        }catch(...) {
            frontend::CleanupErrors errors(std::current_exception());errors.attempt([&]{clear();});errors.rethrow();
        }
    }
    void clear() {
        frontend::CleanupErrors errors;
        for(auto& movie:movies_)if(movie) {
            errors.attempt([&]{services_.stop_mesh_movie(movie->get());});
            errors.attempt([&]{services_.close_mesh_movie_stream(movie->get());});movie.reset();
        }
        active_=Frame{};started_=false;errors.rethrow();
    }
    template<class Random>void show(bool enabled,Random& random){ //004AD380 and004AD350, raw ABI reads the stack bool.
        if(!visual_)throw std::logic_error("HUD injury playback requires actual artwork");
        services_.object_visible(visual_,enabled);
        if(!enabled){
            if(active_)services_.stop_mesh_movie(active_);
            active_=Frame{};services_.object_visible(visual_,false);services_.update_scene_object(visual_);return;
        }
        if(active_)services_.stop_mesh_movie(active_);
        const auto draw=random();if(draw>0x7fffu)throw std::logic_error("HUD injury selection requires actual CRT random");
        active_=movie(draw%7u);
        if(!active_)return; //Failed-open movie slot is genuine; initial visibility remains enabled.
        services_.object_visible(visual_,true);
        services_.hud_injury_movie_texture(visual_,services_.mesh_movie_texture(active_));
        services_.loop_mesh_movie(active_,false);services_.play_mesh_movie(active_);
        services_.update_scene_object(visual_);playing_=true;time24_=time28_=0;
    }
    Frame movie(unsigned index)const {const auto& owner=movies_.at(index);return owner?owner->get():Frame{};}
    Frame visual()const{return visual_;}Frame field04()const{return active_;}
    void discard_borrower()noexcept{visual_=Frame{};} //Before HUD scene release; no native frame release here.
private:
    Services& services_;std::array<std::optional<MovieOwner>,7> movies_{};
    Frame visual_{},active_{};std::uint32_t time24_=0,time28_=0;bool playing_=false,started_=false;
};
} //namespace hd2::game
#endif
