#ifndef HD2_GAME_HUMAN_EQUIPMENT_MOUNT_PRESENTATION_H
#define HD2_GAME_HUMAN_EQUIPMENT_MOUNT_PRESENTATION_H
#include "game/human_ai_shot_direction.h"
#include "game/human_animation_requests.h"
#include "game/human_state_frame_timers.h"
namespace hd2::game {
//00793490. Borrow the SAME230 action model/HW1 and DD0 sector. Each callback-
//visible model/marker read is fresh, matching native reads through this+1C/B4.
template<class Frames,class State,class Services>
void present_human_equipment_mount(Frames& frames,State& state,Services& services,std::uint32_t elapsed){
    if(!state.model(1))return;
    const auto model=[&]{const auto value=state.model(1);if(!value)throw std::logic_error("Mounted presentation lost actual action model");return value;};
    const auto marker=[&]{const auto value=state.first_volume();if(!value)throw std::logic_error("Mounted presentation requires actual HW1");return value;};
    auto matrix=frames.actor_world_matrix(marker());
    frames.human_mounted_model_position(model(),HumanVector{{matrix[12],matrix[13],matrix[14],matrix[15]}});
    matrix=frames.actor_world_matrix(marker()); //Native reread/cache side effects AFTER frame08.
    HumanVector direction{{matrix[8],matrix[9],matrix[10],matrix[11]}};
    const auto square=(static_cast<long double>(direction[1])*direction[1]+
        static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[0])*direction[0];
    normalize_human_ai_shot(direction,square);
    frames.human_reaction_model_direction(model(),direction); //Native0C(direction,0).
    frames.update_scene_object(model());frames.refresh_dynamic_human_model(model()); //24 BEFORE Scene144.
    frames.human_mounted_model_sector(model(),services.equipment_presentation_sector()); //38(sector,1), status ignored.
    frames.human_mounted_model_visibility(model(),services.equipment_human_model()); //Original raw native argument.
    const auto rate=state.words().template get<float>(0xd0);
    const auto step=human_frame_signed_word(elapsed);
    if(rate>0&&step>0){
        const auto scaled=static_cast<long double>(step)*rate;
        frames.human_equipment_tick_model(model(),human_animation_time_integer(scaled+scaled));
    }
}
}
#endif
