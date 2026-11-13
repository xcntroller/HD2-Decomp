#ifndef HD2_GAME_HUD_INJURY_MOVIES_LOADING_H
#define HD2_GAME_HUD_INJURY_MOVIES_LOADING_H
#include "frontend/cleanup.h"
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <optional>
#include <stdexcept>
#include <utility>
namespace hd2::game {
//004AD250/004AD310. Actual seven IShow streams, not frame/texture owners.
//Open failures release+04 only; successful teardown is18 stop/10 close/04
//release ascending. Runtime selection/fade/decode uses these SAME owners;
//this remains separate from full-screen opening-video playback.
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
        started_=true;current_=Frame{};
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
    template<class Random>void show(bool enabled,Random& random){ //004AD380.
        if(!visual_)throw std::logic_error("Injury movie requires its actual artwork");
        services_.object_visible(visual_,enabled); //Original first visibility write even on missing optional movie.
        if(!enabled){
            if(current_)services_.stop_mesh_movie(current_);
            current_=Frame{};
            services_.object_visible(visual_,false);services_.update_scene_object(visual_);return;
        }
        if(current_)services_.stop_mesh_movie(current_); //004AD350.
        const auto draw=random();if(draw>0x7fffu)throw std::logic_error("HUD injury selection requires actual CRT random");
        current_=movie(draw%7u); //Exactly ONE draw from the same process CRT.
        if(!current_)return;
        services_.object_visible(visual_,true);
        const auto material=services_.hud_first_material(visual_);
        if(!material)throw std::runtime_error("Injury movie lost its actual first material");
        services_.first_person_material_texture(material,services_.mesh_movie_texture(current_));
        services_.rewind_mesh_movie(current_);services_.play_mesh_movie(current_);services_.update_scene_object(visual_);
        fading_in_=true;time_=0;alpha_=0;
    }
    void update(std::uint32_t elapsed){ //004AD440.
        if(!current_)return;
        std::int32_t signed_elapsed;std::memcpy(&signed_elapsed,&elapsed,4);
        const auto seconds=static_cast<long double>(signed_elapsed)*.001f;time_+=elapsed;
        if(fading_in_){
            const auto value=seconds*4.f+alpha_;alpha_=static_cast<float>(value);
            if(value>=.5f){alpha_=.5f;fading_in_=false;}
        }else{
            const auto value=static_cast<long double>(alpha_)-seconds*.5f;alpha_=static_cast<float>(value);
            if(value<0){
                alpha_=0;
                //004AD380(false) performs no random draw.
                if(!visual_)throw std::logic_error("Injury movie lost its artwork");
                services_.object_visible(visual_,false);services_.stop_mesh_movie(current_);current_=Frame{};
                services_.object_visible(visual_,false);services_.update_scene_object(visual_);return;
            }
        }
        services_.effects_frame_fade_alpha(visual_,alpha_);
        services_.update_mesh_movie_texture(current_,false); //IShow4C, after opacity; no model24.
    }
    void clear() {
        current_=Frame{}; //Retract the borrower before any stream owner can retire.
        frontend::CleanupErrors errors;
        for(auto& movie:movies_)if(movie) {
            errors.attempt([&]{services_.stop_mesh_movie(movie->get());});
            errors.attempt([&]{services_.close_mesh_movie_stream(movie->get());});movie.reset();
        }
        current_=Frame{};started_=false;errors.rethrow();
    }
    Frame movie(unsigned index)const {const auto& owner=movies_.at(index);return owner?owner->get():Frame{};}
    Frame visual()const{return visual_;}Frame current_movie()const{return current_;}
    Frame field04()const{return current_;} //Human damage observes the SAME active movie borrower.
    float alpha()const{return alpha_;}std::uint32_t elapsed()const{return time_;}
    void discard_borrower()noexcept{visual_=Frame{};} //Before HUD scene release; no native frame release here.
private:
    Services& services_;std::array<std::optional<MovieOwner>,7> movies_{};
    Frame visual_{},current_{};std::uint32_t time_=0;float alpha_=0;bool fading_in_=false,started_=false;
};
} //namespace hd2::game
#endif
