#ifndef HD2_GAME_FIRST_PERSON_ANIMATION_COMPLETION_H
#define HD2_GAME_FIRST_PERSON_ANIMATION_COMPLETION_H
#include <cstdint>
#include <cstring>
#include <stdexcept>
namespace hd2::game {
//00491C60, independent of00491660 camera motion. All arguments borrow the
//actual FPV composition. State/argument/bob are REFERENCES to its60/98/64;
//models.playback() is SAME70/74/94, not another animation state owner.
template<class Frames,class Overlay,class Models,class Items,class Players,class Cursor,class Playback,class Receiver>
void complete_first_person_animation(std::uint32_t mode,std::uint32_t elapsed,Frames& frames,Overlay& overlay,
        Models& models,Items& items,Players& players,Cursor& cursor,Playback& playback,Receiver& receiver,
        const std::uint32_t& state,std::uint32_t& argument,float& bob){
    if(!(mode&2u)||!overlay.byte3c||!models.playback().current.clip)return;
    const auto clip=models.playback().current.clip; //Captured before querying the model time.
    const auto model=models.model();
    if(!model)throw std::logic_error("FPV completion requires the actual model for its live clip");
    const auto time=frames.human_model_time(model,models.playback().current.slot);
    const auto duration=frames.human_clip_duration(clip);
    const auto bits=std::uint32_t(duration)-std::uint32_t(time);
    std::int32_t remaining,signed_elapsed;std::memcpy(&remaining,&bits,4);std::memcpy(&signed_elapsed,&elapsed,4);
    if(remaining>=signed_elapsed)return; //STRICT signed comparison, including subtraction wrap.
    const auto human=[&]{
        auto* player=receiver.player();
        if(!player)throw std::logic_error("FPV completion requires its actual Human borrower");
        (void)players.state(*player); //Validate SAME Human lifetime before item/inline access.
        return player;
    };
    const auto unbind=[&]{
        models.bind_weapon(mode,typename Frames::Frame{},typename Frames::Frame{},-1);
        models.playback().animation=-1;argument=0xffffffffu;
    };
    if(models.animation()==9){
        receiver.reset_zoom(mode);
        if(argument==8){
            receiver.bind_player(mode);models.reset_tracks(mode);argument=0;
            (void)playback.play(mode,8);return;
        }
        unbind();return;
    }
    if(state==4&&models.animation()==2){
        models.reset_tracks(mode);receiver.refresh_weapon(mode);bob=-.8f;
        if(!players.inline_control(*human()).first_person_weapon_suppressed()){
            receiver.set_state(mode,5);argument=0;
        }else{
            receiver.set_state(mode,players.state(*human()).first_person_state());unbind();
        }
    }
    if(overlay.bytes58[2]){
        if(models.animation()==11&&players.player_mode(*human(),mode)==3)
            cursor.set_mode(players.player_mode(*human(),mode)); //Original rereads action after first query.
        if(argument==0||argument==1){models.playback().animation=0;return;}
    }
    if(argument==0){
        //005A2B10 is deliberately queried twice; do not retain the first entry
        //across a callback. Only the second result supplies native virtual0C.
        if(items.weapon(*human())){
            const auto entry=items.weapon(*human());
            if(!entry)throw std::logic_error("FPV weapon disappeared between actual holder queries");
            if(items.amount(entry)==0.f)argument=1;
        }
    }
    std::int32_t animation;std::memcpy(&animation,&argument,4);
    (void)playback.play(mode,animation);
}
}
#endif
