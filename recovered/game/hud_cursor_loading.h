#ifndef HD2_GAME_HUD_CURSOR_LOADING_H
#define HD2_GAME_HUD_CURSOR_LOADING_H
#include <array>
#include <cstdint>
#include <stdexcept>
#include "frontend/pre_mission_data.h"
#include "game/first_person_human_items.h"
namespace hd2::game {
//004A8C10 and004A8CC0, HUD+2E60. These are borrowed artwork frames from
//the actual HUD scene; that scene must outlive the receiver. No empty artwork
//or constructor defaults can serve as initialized HUD/rendering readiness.
template<class Frames,class Players,class Items>
class HudCursorLoading {
public:
    using Frame=typename Frames::Frame;
    HudCursorLoading(Frames& frames,Players& players,Items& items):frames_(frames),players_(players),items_(items) {}
    void bind(Frame scene) {
        if(!scene)throw std::runtime_error("HUD cursor requires the actual artwork scene");
        frames0_[0]=frames_.hud_find(scene,"sniper",1);
        frames0_[5]=frames_.hud_find(scene,"scope",1);
        if(frames0_[5])show(5,false);
        constexpr std::array<const char*,4> names{{"arisaka","k98","springfield","lee_enfield"}};
        for(unsigned n=0;n!=4;++n)frames0_[n+1]=frames_.hud_find(scene,names[n],0x20);
        show(0,false);show(5,false);
        for(unsigned n=5;--n;)if(frames0_[n])show(n,false);
    }
    void set_kind(std::int32_t kind) { //004A8CC0; other kinds intentionally make no calls.
        if(kind>=0&&kind<=3){show(0,true);show(static_cast<unsigned>(kind)+1,true);}
        else if(kind==4)show(5,true);
        else if(kind==6){show(0,false);show(5,false);for(unsigned n=5;--n;)show(n,false);}
    }
    void set_mode(std::int32_t mode) { //0049DC60; mode3 reads actual current weapon's second payload.
        switch(mode) {
        case 0:case 2:set_kind(6);break;
        case 1:set_kind(4);break;
        case 3:{
            auto* human=players_.current_player();
            if(!human)throw std::runtime_error("HUD weapon cursor requires an actual current player");
            FirstPersonHumanItems<Items> human_items(items_);
            const auto weapon=human_items.weapon(*human);
            if(!weapon)throw std::runtime_error("HUD weapon cursor requires the actual hand weapon");
            const auto& payload=human_items.definition(weapon).payloads[1];
            if(!payload.type||payload.type>=10||payload.bytes.size()<12)
                throw std::runtime_error("HUD weapon cursor payload has no native0C field");
            const auto bits=frontend::pre_mission_detail::Bytes::word(payload.bytes.data()+8);
            set_kind(frontend::pre_mission_detail::Bytes::integer(bits));break;
        }
        default:break;
        }
    }
    const auto& frames()const{return frames0_;}
    void clear()noexcept{frames0_.fill(Frame{});} //Discard borrowers before destroying HUD scene.
private:
    void show(unsigned index,bool enabled) {
        const auto frame=frames0_[index];
        if(!frame)throw std::runtime_error("Required HUD cursor artwork is missing");
        frames_.object_visible(frame,enabled);
    }
    Frames& frames_;Players& players_;Items& items_;std::array<Frame,6> frames0_{};
};
} //namespace hd2::game
#endif
