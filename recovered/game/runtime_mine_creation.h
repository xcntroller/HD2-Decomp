#ifndef HD2_GAME_RUNTIME_MINE_CREATION_H
#define HD2_GAME_RUNTIME_MINE_CREATION_H
#include "game/human_joint_owners.h"
#include <stdexcept>
#include <utility>
namespace hd2::game {
//00478B90/00478D30 borrow the actual resource catalogue and runtime registry.
//A failed resource load releases only the temporary factory reference. Once
//created, the Mine remains registry-owned across later bind/inventory failures.
template<class Frames,class Catalogue>auto prepare_runtime_mine_model(Frames& frames,const Catalogue& catalogue,
        std::int32_t definition,const HumanVector& position,const HumanVector& direction){
    auto model=frames.create_item_template_model(); //Driver9 BEFORE definition lookup.
    const auto* item=catalogue.item(definition);
    if(!item)throw std::logic_error("Runtime Mine model requires actual item definition");
    if(frames.load_item_template_model(model.get(),item->resources[2])!=0){model.reset();return model;}
    frames.car_suspension_position(model.get(),position); //Direct B0/E0 write, including position-lock gate.
    frames.prepared_human_direction(model.get(),direction); //Model1C(direction,0), not quaternion rotation.
    frames.update_scene_object(model.get());
    return model;
}
template<class Frames,class Environment,class Catalogue,class Actor>
auto create_runtime_mine(Frames& frames,Environment& environment,const Catalogue& catalogue,Actor* human,
        std::int32_t definition,const HumanVector& position,const HumanVector& direction,bool publish){ //00478B90.
    using MinePointer=decltype(environment.create_mine(typename Frames::Frame{},-1));
    auto model=prepare_runtime_mine_model(frames,catalogue,definition,position,direction);
    if(!model)return MinePointer{};
    auto* mine=environment.create_mine(model.get(),-1);
    if(!mine)throw std::logic_error("Runtime Mine factory returned no actual owner");
    mine->bind(human,definition,0xffffffffu);
    model.reset(); //Native factory reference release precedes Human inventory lookup.
    if(human&&!environment.consume_mine_inventory(human,definition))return mine;
    if((environment.mode_flags()&7u)!=7u&&publish){
        if(!human)throw std::logic_error("Runtime Mine publication requires actual source Human identity");
        environment.publish_mine(mine->base().id10(),human->base().id10(),definition,position,direction);
    }
    return mine;
}
template<class Frames,class Environment,class Catalogue>
auto present_runtime_mine(Frames& frames,Environment& environment,const Catalogue& catalogue,
        std::int32_t identity,std::int32_t human_identity,std::int32_t definition,
        const HumanVector& position,const HumanVector& direction){ //00478D30.
    //On authority this is only a backwards runtime identity lookup: no model,
    //Scene2 lookup, inventory change or type check is performed by this body.
    if(environment.mode_flags()&1u)return environment.identity(identity);
    auto model=prepare_runtime_mine_model(frames,catalogue,definition,position,direction);
    using ActorPointer=decltype(environment.identity(identity));
    if(!model)return ActorPointer{};
    auto* human=human_identity<0?nullptr:environment.mine_scene_actor(human_identity);
    auto* mine=environment.create_mine(model.get(),identity);
    if(!mine)throw std::logic_error("Presented Mine factory returned no actual owner");
    mine->bind(human,definition,0xffffffffu);model.reset();
    if(human)(void)environment.consume_mine_inventory(human,definition);
    return static_cast<ActorPointer>(mine);
}
//00477660's kind10/flag1D branch. A missing collision owner is a required
//failure in mine_ground_ray, never a synthetic miss or flat ground result.
template<class Environment>auto placed_mine_pose(Environment& environment,const HumanVector& position){
    auto start=position;start[1]=static_cast<float>(static_cast<long double>(start[1])+.5f); //0080F518 FLOAT.
    const auto hit=environment.mine_ground_ray(start,HumanVector{{0,-2,0,0}});
    return hit?std::make_pair(hit->position,hit->normal):std::make_pair(position,HumanVector{{0,1,0,0}});
}
}
#endif
