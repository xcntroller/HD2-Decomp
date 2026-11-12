#ifndef HD2_GAME_ACTOR_ZONE_PRESENTATION_H
#define HD2_GAME_ACTOR_ZONE_PRESENTATION_H
#include "game/human_hud_loading.h"
namespace hd2::game {
//The actual kind37 Actor implements this read-only UI face over its OWN
//A4/98/F1/A0/A8/EC fields. No Zone actor or progress state is allocated here.
//A value snapshot is consumed in-call; no borrowed field/frame/page is cached.
class ActorZonePresentation {
public:
    virtual ~ActorZonePresentation()=default;
    virtual std::int32_t hud_zone_group()const=0;
    virtual std::int32_t hud_zone_state()const=0;
    virtual bool hud_zone_progress_active()const=0;
    virtual HudZoneProgressData hud_zone_progress()const=0;
};
template<class Actor>const ActorZonePresentation* actor_zone_presentation(const Actor& actor){
    if(actor.base().type()!=37)throw std::logic_error("Zone UI requires an actual kind37 Actor");
    return dynamic_cast<const ActorZonePresentation*>(&actor);
}
} //namespace hd2::game
#endif
