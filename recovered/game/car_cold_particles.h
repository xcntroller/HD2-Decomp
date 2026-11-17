#ifndef HD2_GAME_CAR_COLD_PARTICLES_H
#define HD2_GAME_CAR_COLD_PARTICLES_H

#include "game/car_health_reset.h"
#include "game/car_body_contacts.h"
#include "game/car_particles.h"

namespace hd2::game {

struct CarColdParticleSlots {
    std::int32_t damage=-1;std::array<std::int32_t,3> impacts{{-1,-1,-1}};
    std::vector<std::int32_t> exhausts;
    explicit CarColdParticleSlots(std::size_t exhaust_count):exhausts(exhaust_count,-1){} //004EA780.
};

} //namespace hd2::game
#endif
