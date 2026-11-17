#ifndef HD2_GAME_STATIC_WEAPON_SEAT_FRAME_H
#define HD2_GAME_STATIC_WEAPON_SEAT_FRAME_H
#include <stdexcept>
#include <exception>
#include <cstdint>
#include <cstddef>
namespace hd2::game {
//00604B70: native caches the initial occupant (unlike Car004D9E10).
//Seat marker and the cached occupant's MODEL are re-read after callbacks.
template<class Actor,class Seats,class Frames,class View,class Cursor>
auto static_weapon_seat_camera_frame(const Seats& seats,std::int32_t index,Frames& frames,View&& view,Cursor&& cursor){
    using Frame=typename Frames::Frame;
    if(index<0||static_cast<std::size_t>(index)>=seats.size())return Frame{};
    const auto at=static_cast<std::size_t>(index);
    auto* occupant=static_cast<Actor*>(seats.at(at).occupant);
    const auto marker=[&]{const auto f=seats.at(at).frame;if(!f)throw std::logic_error("StaticWeapon camera requires actual seat marker");return f;};
    if(occupant)frames.human_seat_link(occupant->base().frame(),Frame{});
    struct Restore {
        Actor* occupant;Frames& frames;const decltype(marker)& current_marker;bool active=true;
        void finish(){if(occupant){frames.human_seat_link(occupant->base().frame(),current_marker());frames.update_scene_object(occupant->base().frame());}active=false;}
        ~Restore(){if(active)try{finish();}catch(...){frames.record_actor_cleanup_failure(std::current_exception());}}
    } restore{occupant,frames,marker};
    const auto key=[&]{return view()||cursor(index)?"cameraFPV*":"camera3RD*";};
    const auto* name=key();auto result=frames.actor_find(marker(),name,0x10020);
    if(!result){
        name=key();const auto parent=frames.actor_parent(marker());
        if(!parent)throw std::logic_error("StaticWeapon camera fallback requires actual marker parent");
        result=frames.actor_find(parent,name,0x10020);
    }
    restore.finish();return result;
}
} //namespace hd2::game
#endif
