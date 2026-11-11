#ifndef HD2_GAME_HUMAN_HUD_LOADING_H
#define HD2_GAME_HUMAN_HUD_LOADING_H
#include "game/human_camera_state.h"
#include "frontend/human_store.h"
#include "frontend/control_startup.h"
#include "frontend/game_configuration.h"
namespace hd2::game {
//005A8380 offline7: native profile24, distinct from profile rank44/stat array.
template<class Human>std::uint32_t offline_human_hud_rank(const Human& human){return human.profile_record?human.profile_record->field24:0u;}
//005A81C0 offline7: user14 else LAST04, unlike005A82C0's short10 fallback.
//Without B8 use the actual model's E8 name rather than the Actor name override.
template<class Frames,class Human>std::string offline_human_hud_name(Frames& frames,const Human& human){
    if(const auto* profile=human.profile_record)return profile->user_name.empty()?profile->last:profile->user_name;
    return frames.frame_name(human.base().frame());
}
template<class Human,class Locks>std::uint32_t network_human_hud_rank(const Human& human,
        frontend::ControlStartupState& peers,Locks& locks){ //005A8380, copy ALL nine words under418, return copied54 AFTER unlock.
    std::array<std::uint32_t,9> values;
    locks.enter_control_lock(0);
    try{const auto& peer=peers.records.at(static_cast<std::uint32_t>(human.base().id10()));
        for(unsigned n=0;n!=9;++n)values[n]=peer.word(0x40+4*n);
    }catch(...){locks.leave_control_lock(0);throw;}
    locks.leave_control_lock(0);return values[5];
}
template<class Human>std::string network_human_hud_short_name(const Human& human,const frontend::ControlStartupState& peers){ //005A82C0.
    if(human.properties.field19c==1){const auto id=static_cast<std::uint32_t>(human.base().id10());return id<peers.records.size()?peers.records[id].name():std::string{};}
    if(const auto* profile=human.profile_record)return profile->user_name.empty()?profile->short_name:profile->user_name;
    return human.base().name();
}
template<class Human>std::uint32_t network_human_hud_side(const Human& human,const frontend::GameConfiguration& configuration,
        const frontend::ControlStartupState& peers){ //005A8820(false), SAME configuration predicate/peer64 as damage.
    if(configuration.remote_ai_enabled()){
        const auto kind=human.properties.field19c;if(kind==2||kind==5)return 0;
        return kind==1||kind==4?1u:2u;
    }
    const auto id=static_cast<std::uint32_t>(human.base().id10());return id<peers.records.size()?peers.records[id].word(0x64):2u;
}
template<class Human,class Items,class Factory,class Text>void bind_network_hud_human_name(Human& human,Items& items,
        Factory& factory,Text& text,std::uint32_t& cached_rank){ //004B3890 reached non7 body, SAME cache/TextState.
    cached_rank=factory.hud_network_human_rank(human);
    bool first=false;
    if(factory.hud_style224()==0){
        auto* storage=human.base().storage();auto* holder=storage?storage->holder(4):nullptr;
        if(!holder)throw std::logic_error("Network HUD rank requires actual Human clothing holder");
        const auto entry=holder->entry();first=entry&&items.runtime_item(entry).definition().fields38[3]==0;
    }else first=factory.hud_network_human_side(human)==1;
    const auto rank=factory.hud_localized((first?0x27a6u:0x27b0u)+cached_rank);
    const auto name=factory.hud_network_human_short_name(human);
    text.text.set(std::make_shared<const std::string>((rank?*rank:std::string{})+" "+name),0x3a,0);
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
