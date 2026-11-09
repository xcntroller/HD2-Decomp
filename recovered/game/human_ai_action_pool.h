#ifndef HD2_GAME_HUMAN_AI_ACTION_POOL_H
#define HD2_GAME_HUMAN_AI_ACTION_POOL_H

#include "game/human_joint_owners.h"

namespace hd2::game {

//Kind1 family ONLY of process008AB048, native manager+60.00582030 eagerly
//allocates16;0058A580 grows by mutable0C. Other action families remain separate.
//Native vtable/pointers are never stored in host DWORDs. Unknown words are guards.
struct alignas(16) HumanAIAction1 {HumanJointWords<0x50> state;};
static_assert(sizeof(HumanAIAction1)==0x50);
class HumanAIAction1Pool {
public:
    struct Return {
        HumanAIAction1Pool* pool=nullptr;
        void operator()(HumanAIAction1* value)const noexcept{if(value)pool->give(value);}
    };
    using Lease=std::unique_ptr<HumanAIAction1,Return>;
    HumanAIAction1Pool(){grow();}
    HumanAIAction1Pool(const HumanAIAction1Pool&)=delete;
    HumanAIAction1Pool& operator=(const HumanAIAction1Pool&)=delete;
    Lease acquire(const HumanVector& position,std::uint16_t point,float radius,const HumanVector& direction) { //005827B0.
        if(free_.empty())grow();
        auto* result=free_.back();free_.pop_back();
        result->state.set(0x20,position);result->state.set(0x30,direction);
        result->state.set(0x40,radius);result->state.set(0x44,std::int32_t{-1});
        result->state.set(0x48,static_cast<std::uint32_t>(point));
        return Lease(result,Return{this});
    }
    std::size_t allocated()const{return records_.size();}std::size_t available()const{return free_.size();}
    std::int32_t growth_batch()const{return batch_;}void growth_batch(std::int32_t value){batch_=value;}
private:
    void give(HumanAIAction1* value)noexcept{free_.push_back(value);} //00582C50 case1; retain record words on reuse.
    void grow() {
        if(batch_<=0)throw std::runtime_error("Human AI action pool has a nonpositive growth batch");
        const auto count=records_.size()+static_cast<std::uint32_t>(batch_);
        records_.reserve(count);free_.reserve(count);
        for(std::int32_t n=0;n!=batch_;++n) {
            auto value=std::make_unique<HumanAIAction1>();value->state.set(4,std::uint32_t{1});
            for(auto offset:{8u,12u,16u})value->state.set(offset,std::uint32_t{0});
            free_.push_back(value.get());records_.push_back(std::move(value));
        }
    }
    //The process owner must outlive all AI current-action leases.
    std::vector<std::unique_ptr<HumanAIAction1>> records_;std::vector<HumanAIAction1*> free_;
    std::int32_t batch_=16;
};
} //namespace hd2::game
#endif
