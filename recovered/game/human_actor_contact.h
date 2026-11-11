#ifndef HD2_GAME_HUMAN_ACTOR_CONTACT_H
#define HD2_GAME_HUMAN_ACTOR_CONTACT_H

#include "game/human_joint_owners.h"

namespace hd2::game {

//005A07C0, absent from the frozen inventory. Its B0 receiver is reread after
//moving-Car damage. Contact vectors unused by the three actual C8 bodies
//remain with their callers and earn no additional reconstruction credit.
template<class Human,class Actor,class Services,class Runtime>
bool receive_human_actor_contact(Human& human,Actor& source,const HumanVector& point,
        Services& services,Runtime&){
    if(!human.loading_state)return false;
    const auto mode=services.mode_flags();
    if((mode&1u)&&source.base().type()==4&&services.human_collision_car_speed(source)>2.f){
        if((mode&7u)==7u){
            const auto& occupants=services.human_collision_car_occupants(source);
            for(std::size_t reverse=occupants.size();reverse!=0;){
                auto* occupant=occupants[--reverse];
                if(occupant)services.human_collision_alignment(*occupant,-30.f);
            }
        }
        services.apply_human_damage(human,13,-1.f,point,human.base().world_direction(),services.human_collision_car_driver(source),0); //Actual virtualD4.
    }
    auto* state=human.loading_state;
    if(!state)throw std::runtime_error("Human contact lost its actual B0 receiver after damage");
    return state->frame_actor_contact(source.base().type(),source.base().id10(),mode);
}

} //namespace hd2::game
#endif
