#ifndef HD2_GAME_HUMAN_ACTIVE_AI_PRESENTATION_H
#define HD2_GAME_HUMAN_ACTIVE_AI_PRESENTATION_H
#include "game/human_ai_shot_direction.h"
#include "game/human_state_frame_timers.h"

namespace hd2::game {
//00525DA0. On-foot and non-Car states only age194; bit800 returns BEFORE
//that store. Controlled and uncontrolled Car seats have different direction
//sources and raw10/160 publication. Do not collapse the two fresh frame reads.
template<class State,class Frames,class World>
void present_human_active_ai(State& ai,std::uint32_t elapsed,Frames& frames,World& world){
    auto& words=ai.words();auto& state=ai.state();
    if(words.template get<std::uint32_t>(0x50)&0x800u)return;
    auto* human=ai.human();if(!human)throw std::logic_error("Active AI presentation requires actual Human44");
    const auto direction=[&](auto frame){
        const auto matrix=frames.actor_world_matrix(frame);
        HumanVector result{{matrix[8],matrix[9],matrix[10],matrix[11]}};
        const auto length=(static_cast<long double>(result[1])*result[1]+static_cast<long double>(result[2])*result[2])+
            static_cast<long double>(result[0])*result[0];
        normalize_human_ai_shot(result,length);return result;
    };
    if(human->field160&&human->field160->base().type()==4){
        if(!world.human_pose_actor_has_control(human->field160,human->field164)){
            if(!(words.template get<std::uint32_t>(0x50)&0x800u)&&
                    (world.player_attachment_control_flags(*human)&0x10u)){
                if(const auto frame=world.human_car_seat_frame(human->field160,human->field164)){
                    (void)ai.resources().direction().set_target(direction(frame));
                    (void)ai.resources().direction().set_current(direction(frame));
                    state.set(0x160,words.template get<HumanVector>(0x10));
                }
            }
        }else{
            const bool align=state.template get<std::uint8_t>(0x230)||[&]{
                const auto& target=ai.resources().target();if(target.actor)return false;
                const auto velocity=target.state.template get<HumanVector>(0x10);
                const auto square=(static_cast<long double>(velocity[0])*velocity[0]+static_cast<long double>(velocity[1])*velocity[1])+
                    static_cast<long double>(velocity[2])*velocity[2];
                return square<1.0000000168623835e-16f&&!state.template get<std::uint32_t>(0x198);
            }();
            if(align){
                const auto value=direction(human->base().frame());words.set(0x10,value);
                (void)ai.resources().direction().set_target(words.template get<HumanVector>(0x10));
                (void)ai.resources().direction().set_current(words.template get<HumanVector>(0x10));
            }
        }
    }
    const auto remaining=state.template get<std::uint32_t>(0x194)-elapsed;
    state.set(0x194,human_frame_signed_word(remaining)<0?0u:remaining);
}
}
#endif
