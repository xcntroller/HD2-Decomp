#ifndef HD2_GAME_HUMAN_AI_ORDER_POOL_H
#define HD2_GAME_HUMAN_AI_ORDER_POOL_H
#include "game/human_joint_owners.h"
#include <cstddef>

namespace hd2::game {
template<class Frame>class HumanAIOrderMarkerLifetime {
public:
    virtual ~HumanAIOrderMarkerLifetime()=default;
    virtual Frame frame()const=0;
    virtual void update()=0;
    virtual void retire()noexcept=0;
};
//0058A3F0's DISTINCT manager+18 record (native kind4, allocation130).
//Issuer8/target40/model84 and descriptor pointers never occupy host DWORDs.
//Native74 route allocation has no producer here; its positive byte layout
//remains required. The cold state and return free the allocation as one owner.
template<class Frames,class Actor>struct HumanAIOrder {
    HumanJointWords<0x90> state;
    Actor* issuer=nullptr;Actor* target=nullptr;
    std::unique_ptr<std::byte[]> route_allocation;
    std::unique_ptr<HumanAIOrderMarkerLifetime<typename Frames::Frame>> marker;
    HumanAIOrder(){state.set(4,4u);state.set(0x80,8u);}
};
//005825C0 acquisition;00582640/0058C8E0 return. SAME Startup process owner
//outlives Human order leases. Marker destruction owns descriptor/EC/model
//retirement BEFORE route allocation release and free-list publication.
template<class Frames,class Actor>class HumanAIOrderPool {
public:
    using Order=HumanAIOrder<Frames,Actor>;
    struct Return {HumanAIOrderPool* pool=nullptr;void operator()(Order* value)const noexcept{if(value)pool->give(value);}};
    using Lease=std::unique_ptr<Order,Return>;
    HumanAIOrderPool(){grow();}
    HumanAIOrderPool(const HumanAIOrderPool&)=delete;HumanAIOrderPool& operator=(const HumanAIOrderPool&)=delete;
    template<class Runtime>Lease acquire(Runtime& runtime,Actor* issuer,std::uint32_t kind,std::int32_t action,
            const HumanVector& position,const HumanVector& direction,std::uint32_t movement,std::uint32_t posture,
            std::uint32_t mind,Actor* target,std::uint32_t secondary){
        if(free_.empty())grow();
        auto* value=free_.back();free_.pop_back();Lease result(value,Return{this});
        auto& state=value->state;state.set(0xc,kind);value->issuer=issuer;state.set(0x10,action);
        state.set(0x20,direction);state.set(0x30,position);value->target=target;state.set(0x70,secondary);
        state.set(0x44,movement|posture|mind);
        for(auto offset:{0x50u,0x54u,0x58u,0x60u,0x64u,0x68u})state.set(offset,100000.f);
        //5C/6C are uninitialized temporary W stores in native0058C5A0; unclaimed.
        for(auto offset:{0x49u,0x48u,0x4au})state.set(offset,std::uint8_t{0});
        if(value->marker)value->marker->retire();
        value->marker.reset(); //Native retires an existing84 BEFORE creating its replacement.
        runtime.initialize_human_ai_order_marker(*value); //Publishes marker84 before EC/position/link/descriptor work.
        if(!value->marker||!value->marker->frame())throw std::logic_error("AI order requires actual collision-visible marker");
        return result;
    }
    std::size_t allocated()const{return records_.size();}std::size_t available()const{return free_.size();}
    std::int32_t growth_batch()const{return batch_;}void growth_batch(std::int32_t value){batch_=value;}
private:
    void give(Order* value)noexcept{
        if(value->marker)value->marker->retire(); //84 remains observable through EC/model-return callbacks until retirement finishes.
        value->marker.reset();value->route_allocation.reset(); //0058C8E0, BEFORE native00582640 free-list publication.
        free_.push_back(value);
    }
    void grow(){
        if(batch_<=0)throw std::runtime_error("AI order pool has a nonpositive growth batch");
        const auto count=records_.size()+static_cast<std::uint32_t>(batch_);
        records_.reserve(count);free_.reserve(count);
        for(std::int32_t n=0;n<batch_;++n){auto value=std::make_unique<Order>();free_.push_back(value.get());records_.push_back(std::move(value));}
    }
    std::vector<std::unique_ptr<Order>> records_;std::vector<Order*> free_;std::int32_t batch_=16;
};
}
#endif
