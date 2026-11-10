#ifndef HD2_GAME_HUMAN_HUD_LOADING_H
#define HD2_GAME_HUMAN_HUD_LOADING_H
#include "game/human_camera_state.h"
#include "frontend/human_store.h"
namespace hd2::game {
//005A8380 offline7: native profile24, distinct from profile rank44/stat array.
template<class Human>std::uint32_t offline_human_hud_rank(const Human& human){return human.profile_record?human.profile_record->field24:0u;}
//005A81C0 offline7: user14 else LAST04, unlike005A82C0's short10 fallback.
//Without B8 use the actual model's E8 name rather than the Actor name override.
template<class Frames,class Human>std::string offline_human_hud_name(Frames& frames,const Human& human){
    if(const auto* profile=human.profile_record)return profile->user_name.empty()?profile->last:profile->user_name;
    return frames.frame_name(human.base().frame());
}
template<class Human>std::int32_t human_hud_first_mode(const Human& human){ //005A88B0.
    const auto* state=human.loading_state;
    return !state||state->words().template get<std::uint32_t>(0x48)!=3?-1:state->hud_first_mode();
}
template<class Human>std::int32_t human_hud_second_mode(const Human& human){ //005A88D0.
    const auto* state=human.loading_state;
    return !state||state->words().template get<std::uint32_t>(0x48)!=3?-1:state->hud_second_mode();
}
} //namespace hd2::game
#endif
