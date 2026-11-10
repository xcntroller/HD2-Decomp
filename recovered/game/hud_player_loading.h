#ifndef HD2_GAME_HUD_PLAYER_LOADING_H
#define HD2_GAME_HUD_PLAYER_LOADING_H
#include <cstddef>
#include <cstdint>
#include <stdexcept>
namespace hd2::game {

//0049B7E0 actual HUD flag0C/index gates and ordered soldier/marker publication.
//Refresh is actual0049F1C0 over the SAME panels/current player, never a default.
template<class Frames,class Hud,class Soldiers,class Compass,class Players,class Items,class Human,class Refresh>
bool add_hud_loading_human(Frames& frames,Hud& hud,Soldiers& soldiers,Compass& compass,Players& players,Items& items,
        Human* human,std::int32_t index,std::uint32_t mode,std::uint32_t difficulty,std::int32_t local_player,Refresh&& refresh){
    if(!hud.flags||index<0||index>5)return false;
    if(!human)throw std::runtime_error("HUD soldier publication requires its actual Human");
    auto& soldier=soldiers.at(static_cast<std::size_t>(index));soldier.bind_human(*human,items,mode,local_player);
    const auto root=soldier.artwork().root;if(!root)throw std::runtime_error("HUD soldier publication lost its actual root");
    frames.object_visible(root,true);hud.flags|=4u;
    compass.add_human(human,mode,difficulty);
    if(human==players.current_player())refresh();
    return true;
}

//0049B870 hides before fresh reverse six-root scan. A visible root returns
//WITHOUT clearing4 or removing a marker. The last removal marks reverse-first
//matching marker amount0; it does not erase/free the record or clear Human74.
template<class Frames,class Hud,class Soldiers,class Compass,class Human>
bool remove_hud_loading_human(Frames& frames,Hud& hud,Soldiers& soldiers,Compass& compass,Human* human,std::int32_t index){
    if(!hud.flags||index<0||index>5)return false;
    const auto root=soldiers.at(static_cast<std::size_t>(index)).artwork().root;
    if(!root)throw std::runtime_error("HUD soldier removal lost its actual root");
    frames.object_visible(root,false);
    for(unsigned n=6;n;){const auto candidate=soldiers.at(--n).artwork().root;
        if(!candidate)throw std::runtime_error("HUD soldier removal scan lost its actual root");
        if(frames.hud_frame_visible(candidate))return true;
    }
    hud.flags&=~4u;compass.remove_human(human);return true;
}

//0049B900, including populated Human borrowers; native FLAGS clear only after
//every marker notification and hide succeeds. Records and borrowers persist.
template<class Frames,class Hud,class Soldiers,class Compass>
bool hide_hud_loading_humans(Frames& frames,Hud& hud,Soldiers& soldiers,Compass& compass){
    if(!hud.flags)return false;
    for(unsigned n=6;n;){auto& soldier=soldiers.at(--n);compass.remove_human(soldier.human_borrower());
        const auto root=soldier.artwork().root;if(!root)throw std::runtime_error("HUD soldier hide lost its actual root");
        frames.object_visible(root,false);
    }
    hud.flags&=~6u;return true;
}

} //namespace hd2::game
#endif
