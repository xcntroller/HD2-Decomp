#ifndef HD2_GAME_ACTOR_OCCUPANTS_PRESENTATION_H
#define HD2_GAME_ACTOR_OCCUPANTS_PRESENTATION_H
#include <cstddef>
#include <cstdint>
#include <stdexcept>
namespace hd2::game {
//Read-only virtual78/88/94 of the actual occupied Actor. Car/StaticWeapon
//already have concrete owners; absent families20/34 implement this on their
//OWN Actor. Every returned Human is an in-call loan checked by the World.
template<class Actor>class ActorOccupantsPresentation {
public:
    virtual ~ActorOccupantsPresentation()=default;
    virtual std::size_t hud_occupant_count()const=0;
    virtual Actor* hud_occupant(std::size_t seat)const=0;
    virtual std::int32_t hud_seat_caption(std::int32_t seat)const=0;
};
template<class Actor>const ActorOccupantsPresentation<Actor>& actor_occupants_presentation(const Actor& actor){
    const auto* view=dynamic_cast<const ActorOccupantsPresentation<Actor>*>(&actor);
    if(!view)throw std::runtime_error("Occupied UI requires the actual Actor family virtual78/88/94 producer");
    return *view;
}
} //namespace hd2::game
#endif
