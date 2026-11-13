#ifndef HD2_GAME_HUMAN_ACTIVE_AI_ORDER_ACTION_H
#define HD2_GAME_HUMAN_ACTIVE_AI_ORDER_ACTION_H
#include "game/human_ai_action_pool.h"

namespace hd2::game {
//0052A1E0. Borrow the actual pending-order sequence; this helper does NOT own
//or create orders. Order supplies scalar state and a separately typed target40.
//The native order-marker/collision producer and return lifecycle are separate
//from these current-action leases. Do not substitute an action1 for an order.
template<class State,class Order,class Orders,class Frames,class World>
void activate_human_ai_order(State& ai,const Order& order,const Orders& pending,
        HumanAIActionPools& actions,Frames& frames,World& world){
    ai.reset_frame_action(); //Actual virtual10C BEFORE flags/target/frame reads.
    auto& state=ai.state();state.set(0x174,state.template get<std::uint32_t>(0x174)&~2u);
    const auto position=[&](const auto& record){
        if(!record.target)return record.state.template get<HumanVector>(0x30);
        const auto model=record.target->base().frame();
        if(!model)throw std::logic_error("Active order target requires actual model");
        const auto matrix=frames.actor_world_matrix(model);
        return HumanVector{{matrix[12],matrix[13],matrix[14],matrix[15]}};
    };
    const auto kind=order.state.template get<std::uint32_t>(0xc);
    if(kind==1){
        const auto destination=position(order);
        ai.resources().current_action(actions.kind7.acquire(destination,order.state.template get<HumanVector>(0x20)));
        auto* human=ai.human();if(!human)throw std::logic_error("Active order requires bound Human44");
        if(human->field160&&human->field160->base().type()==4&&human->field164==0){
            auto& path=ai.resources().path();
            path.bind_vehicle(human->field160,[&world](void* car){world.human_vehicle_governor(car,world.human_vehicle_maximum(car));});
            for(std::size_t n=1;n<pending.size();++n){
                const auto* next=pending[n].get();
                if(!next)throw std::logic_error("Active pending order lost actual owner");
                if(next->state.template get<std::uint32_t>(0xc)!=1)continue;
                //Native looks up NEXT target first, then freshly resolves CURRENT.
                const auto tail=position(*next);const auto head=position(order);
                path.request_vehicle_segment(head,tail,1.f);return;
            }
            path.request_vehicle_position(position(order),1.f);
        }
    }else if(kind==2){
        ai.resources().current_action(actions.kind12.acquire(order.state.template get<HumanVector>(0x20),order.target));
    }else if(kind==4){
        ai.resources().current_action(actions.kind8.acquire(order.target));
    }
}
}
#endif
