#ifndef HD2_GAME_HUMAN_MINE_CONTACT_H
#define HD2_GAME_HUMAN_MINE_CONTACT_H
#include "game/human_joint_owners.h"
namespace hd2::game {
//0059FFD0's positive moving-Human branch. Native consumes ONLY the first
//006FCB60 result, even if another nearby Mine would accept this Human family.
template<class Human,class Frames,class Query,class Contact>
void dispatch_human_mine_contact(Human& human,Frames& frames,Query& query,Contact&& contact){
    const auto matrix=frames.actor_world_matrix(human.base().frame());
    const auto candidates=query.radius(HumanVector{{matrix[12],matrix[13],matrix[14],matrix[15]}},.5f,0xfffbffffu);
    if(!candidates.empty()&&(candidates.front()->mask&0x40000u)&&candidates.front()->actor)
        contact(candidates.front()->actor,&human,0xffffffffu);
}
}
#endif
