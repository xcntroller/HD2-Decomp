#ifndef HD2_GAME_HUMAN_STATE_GROUND_H
#define HD2_GAME_HUMAN_STATE_GROUND_H

#include "game/human_joint_owners.h"

namespace hd2::game {

//0075C4F0, reached when physical22C's last bit2/4 clears. Uses the actual
//Human/base, E0, physical descriptors/exclusions and loaded-tree sweep owner.
//No default no-hit or model/position service is supplied.
template<class Frames,class Human,class Physical,class Look,class Sweep>
void reposition_human_state_on_ground(Frames& frames,Human& human,Physical& physical,const Look& look,Sweep& sweep) {
    const auto model=human.base().frame();
    if(!model)throw std::runtime_error("Human state ground query requires its actual model");
    const auto parent=frames.object_parent(model);
    if(parent&&frames.frame_kind(parent)!=5)return;
    const auto& state=physical.state();
    const auto kind=look.state().template get<std::int32_t>(0xb8);
    const auto animation=state.template get<std::int32_t>(0x1d8);
    std::int32_t life;std::memcpy(&life,&human.properties.stats_ec[0],4);
    if(kind==5||kind==2||kind==3||state.template get<std::uint8_t>(0xa90)||life<1||
        animation==159||animation==158||animation==155||animation==111||animation==34)return;
    const auto posture=look.state().template get<std::int32_t>(0x4c);
    const float height=posture==0?1.f:posture==2?.5f:.75f;
    auto& exclusions=physical.exclusions();const auto descriptors=physical.query_descriptors();
    using Exclusions=std::remove_reference_t<decltype(exclusions)>;
    struct Clear {Exclusions& exclusions;~Clear(){exclusions.clear();}} clear{exclusions};
    exclusions.many(descriptors.data(),descriptors.size()); //0075C960(false); clear even if native adapter throws.
    for(unsigned attempt=0;attempt!=3;++attempt) {
        const auto position=human.base().world_position();
        HumanVector start=position;start[1]=static_cast<float>(static_cast<long double>(position[1])+(attempt+static_cast<long double>(height)));
        const auto hit=sweep.sweep(start,HumanVector{{0,-5,0,0}},.3f,0xc8102,0x230000);
        if(!hit)continue;
        const float y=static_cast<float>(static_cast<long double>(hit->position[1])-.300000011920928955078125f);
        const auto current=human.base().world_position();
        if(current[1]<y||std::abs(static_cast<long double>(current[1])-y)>.5) {
            //Native copies an unwritten hit padding lane. Preserve the known
            //base fourth lane rather than fabricate that original stack word.
            auto next=current;next[0]=hit->position[0];next[1]=y;next[2]=hit->position[2];
            human.base().world_position(next);
            frames.object_position(model,{next[0],next[1],next[2]});frames.update_scene_object(model);
            physical.refresh_ground_geometry();
        }
        return;
    }
}
} //namespace hd2::game
#endif
