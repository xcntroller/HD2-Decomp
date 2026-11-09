#ifndef HD2_GAME_HUD_EXPLOSIVE_TIMERS_LOADING_H
#define HD2_GAME_HUD_EXPLOSIVE_TIMERS_LOADING_H
#include "game/hud_text_loading.h"
#include <array>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
namespace hd2::game {
//004AF630/004AF8B0: borrow ACTUAL0089B3B0 actor registry. Type22 references
//append in reverse registry order, without retain/deduplication or ten-item cap.
//Ten visible slots and the separate8-byte layout capacity are not actor limits.
template<class Frames,class Factory,class ActorType=typename Frames::Actor>
class HudExplosiveTimersLoading {
public:
    using Frame=typename Frames::Frame;using Actor=ActorType;using Text=typename Factory::DisplayText;
    HudExplosiveTimersLoading(Frames& frames,Factory& factory):frames_(frames),factory_(factory) {}
    ~HudExplosiveTimersLoading(){close();}
    template<class Identities>void bind(Frame scene,const Identities& actors) {
        if(started_)throw std::logic_error("HUD explosive timers rebound before teardown");
        if(!scene)throw std::runtime_error("HUD explosive timers require actual scene");
        started_=true;root_=frames_.hud_find(scene,"explosivetimer",0xffff);scene_=scene;
        for(auto count=actors.size();count;)if(auto* actor=actors[--count])if(actor->type()==22)actors_.push_back(actor); //004B04A0 is vector insertion, not an explosive virtual call.
        for(unsigned n=0;n!=10;++n) {
            require(root_);std::array<char,32> name{};std::snprintf(name.data(),name.size(),"timer_explosive%02u",n);
            frames4_[n]=frames_.actor_find(root_,name.data(),1);
            texts2c_[n]=factory_.make_hud_text();if(!texts2c_[n])throw std::runtime_error("HUD explosive timer text factory returned null");
            auto& text=*texts2c_[n];bind_hud_text_geometry(frames_,text,scene,frames4_[n]);
            text.text.inset=0;text.text.append(std::make_shared<const std::string>(),0x48); //Inset AFTER geometry, preserving default10 geometry.
            require(frames4_[n]);frames_.object_visible(frames4_[n],false);
        }
        if(layout_.capacity()<10)layout_.reserve(10); //Native54/58 reserve0x50 without constructing records.
    }
    void close()noexcept { //004AF8B0: clear actor borrowers retaining capacity, then slots0..9, then5C=0.
        actors_.clear();for(unsigned n=0;n!=10;++n){frames4_[n]=Frame{};texts2c_[n].reset();}field5c_=0;started_=false;
    }
    void draw(){ //004AFBD0, slot order0..9; no containing-root visibility gate.
        for(unsigned n=0;n!=10;++n){require(frames4_[n]);if(frames_.hud_frame_visible(frames4_[n])){
            if(!texts2c_[n])throw std::logic_error("HUD timer draw before text initialization");
            factory_.hud_draw_text(*texts2c_[n]);
        }}
    }
    void discard_borrowers()noexcept{scene_=root_=Frame{};}
    Frame root()const{return root_;}Frame scene()const{return scene_;}const auto& actors()const{return actors_;}
    const auto& frames()const{return frames4_;}const auto& texts()const{return texts2c_;}std::size_t layout_capacity()const{return layout_.capacity();}
private:
    static void require(Frame frame){if(!frame)throw std::runtime_error("Required HUD explosive timer artwork is missing");}
    Frames& frames_;Factory& factory_;Frame root_{},scene_{};bool started_=false;
    std::array<Frame,10> frames4_{};std::array<std::unique_ptr<Text>,10> texts2c_{};std::vector<Actor*> actors_;
    std::vector<std::array<std::uint32_t,2>> layout_;std::uint32_t field5c_=0; //Unwritten/cold guard, no fabricated timer records/producer.
};
} //namespace hd2::game
#endif
