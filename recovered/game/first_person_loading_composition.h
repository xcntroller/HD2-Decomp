#ifndef HD2_GAME_FIRST_PERSON_LOADING_COMPOSITION_H
#define HD2_GAME_FIRST_PERSON_LOADING_COMPOSITION_H

#include "game/first_person_model_loading.h"
#include "game/first_person_human_items.h"
#include "game/first_person_animation_sound.h"
#include "game/first_person_animation_playback.h"
#include "game/first_person_view_loading.h"

namespace hd2::game {

//One source lifetime for the actual Game+5C OverlayOwner's model/items/
//animation sound/playback/view receivers. All use the SAME overlay and frame
//registry, Items cache, Players lookup, camera, category3 cache and SOUND.
//The containing session must close this BEFORE Humans/Items/scene retirement;
//the borrowed overlay and all dependencies outlive it. No world-readiness claim.
template<class Frames,class Overlay,class Items,class Players,class Camera,
         class Resources,class Sound,class Cursor,class Effects>
class FirstPersonLoadingComposition {
public:
    using Models=FirstPersonModelLoading<Frames,Overlay>;
    using HumanItems=FirstPersonHumanItems<Items>;
    using Notifications=FirstPersonAnimationSound<Overlay,Models,Camera,Sound>;
    using Playback=FirstPersonAnimationPlayback<Frames,Overlay,Models,Resources,Notifications>;
    using View=FirstPersonViewLoading<Frames,Overlay,Models,HumanItems,Players,Camera,Playback,Cursor,Effects>;
    FirstPersonLoadingComposition(Frames& frames,Overlay& overlay,Items& items,Players& players,Camera& camera,
            Resources& resources,Sound& sound,Cursor& cursor,Effects& effects,
            const frontend::PreMissionItemCatalogue& catalogue,const float& game_fov)
        :models_(frames,overlay),items_(items),notifications_(overlay,models_,catalogue,camera,sound),
         playback_(frames,overlay,models_,resources,notifications_),
         view_(frames,overlay,models_,items_,players,camera,playback_,cursor,effects,game_fov){}
    FirstPersonLoadingComposition(const FirstPersonLoadingComposition&)=delete;
    FirstPersonLoadingComposition& operator=(const FirstPersonLoadingComposition&)=delete;
    template<class Files,class Clock>void activate(std::uint32_t mode,Files& files,Clock& clock){view_.activate(mode,files,clock);}
    void advance(std::uint32_t mode,std::uint32_t elapsed){view_.advance(mode,elapsed);}
    void complete_animation(std::uint32_t mode,std::uint32_t elapsed){view_.complete_animation(mode,elapsed);}
    void update_opacity(std::uint32_t mode){view_.update_opacity(mode);} //00491E30, not the complete00491660 frame.
    void close(std::uint32_t mode){models_.close(mode);} //00490B50; native mode gate retained.
    template<class Files,class Clock>void reset(std::uint32_t mode,Files& files,Clock& clock,const std::uint8_t& preference){ //00493360.
        if(!(mode&2u))return;
        close(mode);activate(mode,files,clock);view_.enable(mode,preference!=0);
    }
    auto& view(){return view_;}auto& models(){return models_;}auto& playback(){return playback_;}
private:
    Models models_;HumanItems items_;Notifications notifications_;Playback playback_;View view_;
};

} //namespace hd2::game
#endif
