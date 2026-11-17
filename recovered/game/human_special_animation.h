#ifndef HD2_GAME_HUMAN_SPECIAL_ANIMATION_H
#define HD2_GAME_HUMAN_SPECIAL_ANIMATION_H
#include "game/human_joint_owners.h"
#include <string>
namespace hd2::game {
//007757E0, four stack arguments(clip,blendMS,fadeOutMS,boolLoop).90 is the
//fourth lane selector;94/98 are reset before publication. Model getter is fresh
//after clip duration lookup. Preserve byteB8/C8 neighbors and other lane metadata.
template<class Words,class Frames,class Model>void setup_human_fourth_track(Words& words,Frames& frames,Model&& model,
        typename Frames::Frame clip,std::int32_t blend,std::int32_t fade,bool loop){
    words.set(0x98,0u);words.set(0x94,0u);
    const auto previous=words.template get<std::int32_t>(0x90);const unsigned lane=previous==-1||previous==0?1u:0u;
    words.set(0x90,lane);words.set(0xb4,frames.human_clip_duration(clip));words.set(0xb8,std::uint8_t{0});
    const unsigned slot=6u+lane,flags=1u+unsigned(loop);
    frames.human_model_animation(model(),clip,slot,lane?0.f:1.f,flags);frames.human_clip_word8(clip,flags);
    if(lane){
        words.set(0x1c4,std::uint8_t(previous==-1));words.set(0x1c0,0.f);words.set(0x1b8,1.f);words.set(0x1c8,0u);
        words.set(0x1bc,static_cast<float>(blend));words.set(0x1cc,1.f);words.set(0x1d0,1.f);
    }else{
        words.set(0x19c,0.f);words.set(0x198,static_cast<float>(blend));words.set(0x194,1.f);words.set(0x1a0,std::uint8_t{0});
        words.set(0x1a4,0u);words.set(0x1a8,1.f);words.set(0x1ac,1.f);
    }
    words.set(0x224,loop?std::int32_t{-1}:fade);
}
//0077CE10 on actual C00/DD0. Native missing/empty clip or DD01AC!=1 fades
//fourth rather than substituting animation. The shared009834A0 request array is
//the SAME animation process outgoing state; callbacks observe its live writes.
template<class Control,class World>void play_human_special_animation(Control& control,const char* name,
        std::int32_t blend,std::int32_t fade,bool loop,World& world){
    auto& tracks=control.tracks();
    if(control.animation_state_word(0x1ac)!=1u||!name||!*name){tracks.fade_fourth(blend);return;}
    auto clip=world.load_human_special_animation(std::string(name)+world.human_animation_suffix());
    if(!clip){tracks.fade_fourth(blend);return;}
    tracks.setup_fourth(clip.get(),blend,fade,loop);
    auto& requests=world.human_animation_arbitration().outgoing;
    for(unsigned n=0;n!=3;++n)requests[n].animation={};
    requests[3].animation=clip.get();
    tracks.update(1,true,&requests); //007724E0(0,1,1,&009834A0): first DWORD unused, elapsed1.
} //Owned cache loan retires AFTER update, including failure unwinding.
} //namespace hd2::game
#endif
