#ifndef HD2_GAME_HUD_ARTWORK_LOADING_H
#define HD2_GAME_HUD_ARTWORK_LOADING_H
#include <array>
#include <cstdio>
#include <stdexcept>
namespace hd2::game {
//0049B730/0049B620 and004A8D70 artwork borrowers. HUD scene owns visuals
//and textures. No texture reference is added; drop these tables before scene
//release. Full0049A940 loading, remaining panel binders and render stay separate.
template<class Frames>
class HudArtworkLoading {
public:
    using Frame=typename Frames::Frame;
    explicit HudArtworkLoading(Frames& frames):frames_(frames) {}
    void bind_actions(Frame scene) { //0049B730, native6B7C0.
        require_scene(scene);
        for(unsigned i=0;i!=actions_.size();++i) {
            const auto visual=numbered(scene,"action%02u",i);
            actions_[i]=visual?frames_.hud_first_texture(visual):Frame{};
            if(visual)frames_.object_visible(visual,false);
        }
    }
    void bind_player_states(Frame scene) { //0049B620, native6B6D0/6B720.
        require_scene(scene);
        for(unsigned i=0;i!=first_.size();++i) {
            const auto visual=numbered(scene,"playerstate1_%02u",i);
            first_[i]=visual?frames_.hud_first_texture(visual):Frame{};
            if(visual)frames_.object_visible(visual,false);
        }
        for(unsigned i=0;i!=second_.size();++i) {
            const auto visual=numbered(scene,"playerstate2_%02u",i);
            if(visual)frames_.object_visible(visual,false); //Second family hides BEFORE lazy texture resolution.
            second_[i]=visual?frames_.hud_first_texture(visual):Frame{};
        }
    }
    void bind_speed(Frame scene) { //004A8D70, root literal is spee, not speed.
        require_scene(scene);speed_[0]=frames_.hud_find(scene,"spee",0xffff);
        if(!speed_[0])throw std::runtime_error("HUD speed artwork root is missing");
        constexpr std::array<const char*,3> names{{"speed01","speed02","speed03"}};
        for(unsigned n=0;n!=3;++n)speed_[n+1]=frames_.actor_find(speed_[0],names[n],0xffff);
        frames_.object_visible(speed_[0],false);
    }
    void show_vehicle(bool enabled){require_scene(speed_[0]);frames_.object_visible(speed_[0],enabled);}
    void update_vehicle(float speed,float rpm,float fuel){ //004A8DF0, upper clamps only, each needle publishes immediately.
        constexpr float radians=.01745329238474369f;
        if(speed>180.f)speed=180.f;
        require_scene(speed_[1]);frames_.static_weapon_restore_axis(speed_[1],{{0,0,1,0}},static_cast<float>(static_cast<long double>(speed)*radians));
        if(rpm>9000.f)rpm=9000.f;
        require_scene(speed_[2]);frames_.static_weapon_restore_axis(speed_[2],{{0,0,1,0}},static_cast<float>(static_cast<long double>(rpm)*.019999999552965164f*radians));
        require_scene(speed_[3]);frames_.static_weapon_restore_axis(speed_[3],{{0,0,1,0}},static_cast<float>(static_cast<long double>(fuel)*180.f*radians));
    }
    const auto& actions()const{return actions_;}const auto& first_states()const{return first_;}
    const auto& second_states()const{return second_;}const auto& speed()const{return speed_;}
    void clear()noexcept{actions_.fill(Frame{});first_.fill(Frame{});second_.fill(Frame{});speed_.fill(Frame{});}
private:
    static void require_scene(Frame scene){if(!scene)throw std::runtime_error("HUD artwork requires actual scene");}
    Frame numbered(Frame scene,const char* format,unsigned value) {
        std::array<char,32> name{};const auto count=std::snprintf(name.data(),name.size(),format,value);
        if(count<0||static_cast<unsigned>(count)>=name.size())throw std::runtime_error("HUD artwork name exceeds native buffer");
        return frames_.hud_find(scene,name.data(),0xffff);
    }
    Frames& frames_;std::array<Frame,39> actions_{};std::array<Frame,20> first_{};
    std::array<Frame,40> second_{};std::array<Frame,4> speed_{};
};
} //namespace hd2::game
#endif
