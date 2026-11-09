#ifndef HD2_GAME_HUD_COMPASS_LOADING_H
#define HD2_GAME_HUD_COMPASS_LOADING_H
#include "game/hud_sound_direction.h"
#include <array>
#include <cstdio>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>
namespace hd2::game {
//0089BB48/4C are process state, shared across HUD lifetimes. Bind publishes0/1
//before the final compass update; close leaves these process values intact.
struct HudCompassShared {float field48=0,field4c=0;};
//00493F80/00494140, ring00493B30/00493BC0 and marker retirement00493B10.
//Runtime compass/marker producers004936C0/00493D10 and simulation remain separate.
template<class Frames>
class HudCompassLoading {
public:
    using Frame=typename Frames::Frame;
    using FrameOwner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    struct Marker {FrameOwner frame;};
    struct Ring {
        Frame us{},en{};std::uint8_t allocator8=0;
        ~Ring(){while(!markers.empty())markers.pop_back();} //Each00493B10 releases its actual frame then frees record, reverse order.
    private:
        std::vector<std::unique_ptr<Marker>> markers; //Native constructor starts empty; no fabricated producer is exposed.
    };
    HudCompassLoading(Frames& frames,HudCompassShared& shared):frames_(frames),shared_(shared) {}
    ~HudCompassLoading(){close();}
    void bind(Frame scene) {
        if(ring_)throw std::logic_error("HUD compass rebound before teardown");
        if(!scene)throw std::runtime_error("HUD compass requires actual scene");
        compass_=frames_.hud_find(scene,"kompas",0xffff);cover_=frames_.hud_find(scene,"kmpscvr",0xffff);arrow_=frames_.hud_find(scene,"strelka",0xffff);
        ring_=std::make_unique<Ring>(); //00493B30: null allocator/owner state and two optional artwork borrowers.
        ring_->allocator8=static_cast<std::uint8_t>(reinterpret_cast<std::uintptr_t>(scene)); //Native copies low BYTE of scene stack argument into dummy allocator storage.
        ring_->us=frames_.hud_find(scene,"kul_us",0xffff);ring_->en=frames_.hud_find(scene,"kul_en",0xffff);
        if(ring_->us)frames_.object_visible(ring_->us,false);
        if(ring_->en)frames_.object_visible(ring_->en,false);
        fields60_.fill(0);
        for(unsigned index=0;index!=8;++index)for(unsigned kind=0;kind!=2;++kind) {
            require(compass_);std::array<char,32> name{};std::snprintf(name.data(),name.size(),kind?"kmps_rd%02u":"kmps_bl%02u",index);
            auto& frame=markers_[index*2+kind];frame=frames_.actor_find(compass_,name.data(),0xffff);require(frame);frames_.object_visible(frame,false);
        }
        fields0_.fill(0);shared_.field4c=1;shared_.field48=0;frames_.update_scene_object(compass_);
    }
    void close()noexcept {ring_.reset();compass_=Frame{};} //00494140 clears ring then root only.
    void show(bool visible){if(compass_)frames_.object_visible(compass_,visible);} //00494170, null root tolerated, no update.
    template<class Direction,class Players>void sound_direction(const Direction& direction,std::uint32_t kind,Players& players) { //004947E0.
        if(!players.current_player())return; //0045D1D0, checked BEFORE reading input.
        fields60_[hud_sound_direction_sector(direction[0],direction[2])*2+(kind!=0)]=500;
    }
    void discard_borrowers()noexcept{compass_=cover_=arrow_=Frame{};markers_.fill(Frame{});}
    Frame compass()const{return compass_;}Frame cover()const{return cover_;}Frame arrow()const{return arrow_;}
    const auto& ring()const{return ring_;}const auto& markers()const{return markers_;}const auto& fields60()const{return fields60_;}const auto& fields0()const{return fields0_;}
private:
    static void require(Frame value){if(!value)throw std::runtime_error("Required HUD compass artwork is missing");}
    Frames& frames_;HudCompassShared& shared_;Frame compass_{},cover_{},arrow_{};
    std::unique_ptr<Ring> ring_;std::array<Frame,16> markers_{};std::array<std::uint32_t,16> fields60_{};std::array<std::uint32_t,4> fields0_{};
};
} //namespace hd2::game
#endif
