#ifndef HD2_GAME_DYNAMIC_ITEM_FRAMES_H
#define HD2_GAME_DYNAMIC_ITEM_FRAMES_H
#include <cstdint>
#include <cstring>
namespace hd2::game {
//A view of the actual loose Item's6A0..6B0, never a second timer owner.
struct LooseItemFrameState {
    std::int32_t& lifetime;std::int32_t& sound_cooldown;bool& fading;
    std::int32_t& thrown_lifetime;std::uint32_t& age;
};
inline std::int32_t loose_item_subtract_time(std::int32_t time,std::uint32_t elapsed){
    const auto bits=static_cast<std::uint32_t>(time)-elapsed;std::int32_t result;std::memcpy(&result,&bits,4);return result;
}
template<class Item,class World>void advance_loose_item_frame(Item& item,LooseItemFrameState state,
        bool authority,std::uint32_t elapsed,World& world){
    if(!authority){ //005B7480: Body runs even when item90 is NULL.
        world.advance_loose_item_body(item.body(),elapsed);
        if(state.sound_cooldown){state.sound_cooldown=loose_item_subtract_time(state.sound_cooldown,elapsed);if(state.sound_cooldown<0)state.sound_cooldown=0;}
        return;
    }
    if(!item.entry())return; //005B7320 authority has this distinct early gate.
    state.age+=elapsed;
    if(!(world.mode_flags()&2u))world.advance_loose_item_body(item.body(),elapsed);
    if(state.thrown_lifetime!=-1){state.thrown_lifetime=loose_item_subtract_time(state.thrown_lifetime,elapsed);
        if(state.thrown_lifetime<1){world.loose_item_timed_effect(item);state.thrown_lifetime=-1;}}
    if(state.lifetime==-1)return;
    state.lifetime=loose_item_subtract_time(state.lifetime,elapsed);
    if(state.lifetime<1){
        world.restore_loose_item_opacity(item.base().frame());world.hide_loose_item(item.base().frame());
        world.publish_loose_item_expiry(item.base().id10());state.lifetime=-1;
        if(!(world.mode_flags()&2u)){world.remove_loose_item(item);return;}
    }else if(static_cast<float>(state.lifetime)<=500.f&&!state.fading){
        state.fading=true;world.fade_loose_item(item.base().frame(),state.lifetime);
    }
}
}
#endif
