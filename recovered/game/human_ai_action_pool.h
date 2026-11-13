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

//0058A660/0058A790 grow the distinct manager+78/+90 families. The free-list
//record remains allocated when returned by00582C50; the next acquire only
//overwrites the words that00582890/00582990 actually assign.
template<std::uint32_t Kind,std::size_t Size>struct alignas(16) HumanAIPooledAction {
    HumanJointWords<Size> state;
};
//Native8+14 /12+30 are actor borrowers, never pointers in host DWORD words.
//Their cold constructors do not initialize these slots; acquire supplies them.
template<>struct alignas(16) HumanAIPooledAction<8,0x20> {
    HumanJointWords<0x20> state;void* actor=nullptr;
};
template<>struct alignas(16) HumanAIPooledAction<12,0x40> {
    HumanJointWords<0x40> state;void* actor=nullptr;
};
template<std::uint32_t Kind,std::size_t Size>class HumanAIActionPool {
public:
    using Action=HumanAIPooledAction<Kind,Size>;
    struct Return {
        HumanAIActionPool* pool=nullptr;
        void operator()(Action* value)const noexcept{if(value)pool->give(value);}
    };
    using Lease=std::unique_ptr<Action,Return>;
    HumanAIActionPool(){grow();}
    HumanAIActionPool(const HumanAIActionPool&)=delete;
    HumanAIActionPool& operator=(const HumanAIActionPool&)=delete;
    std::size_t allocated()const{return records_.size();}
    std::size_t available()const{return free_.size();}
    std::int32_t growth_batch()const{return batch_;}
    void growth_batch(std::int32_t value){batch_=value;}
protected:
    Lease acquire_record(){
        if(free_.empty())grow();
        auto* result=free_.back();free_.pop_back();
        return Lease(result,Return{this});
    }
private:
    void give(Action* value)noexcept{free_.push_back(value);}
    void grow(){
        if(batch_<=0)throw std::runtime_error("Human AI action pool has a nonpositive growth batch");
        const auto count=records_.size()+static_cast<std::uint32_t>(batch_);
        records_.reserve(count);free_.reserve(count);
        for(std::int32_t n=0;n!=batch_;++n){
            auto value=std::make_unique<Action>();value->state.set(4,Kind);
            for(auto offset:{8u,12u,16u})value->state.set(offset,std::uint32_t{0});
            free_.push_back(value.get());records_.push_back(std::move(value));
        }
    }
    std::vector<std::unique_ptr<Action>> records_;std::vector<Action*> free_;
    std::int32_t batch_=16;
};
class HumanAIAction2Pool : public HumanAIActionPool<2,0x50> {
public:
    template<std::size_t N>
    Lease acquire(const std::array<float,N>& first,const std::array<float,N>& second,std::uint32_t mode){ //00582890.
        static_assert(N==3||N==4);
        auto result=acquire_record();auto& state=result->state;
        state.set(0x44,std::uint32_t{0});state.set(0x20,first);state.set(0x30,second);
        state.set(0xc,std::uint32_t{0});state.set(8,mode);return result;
    }
};
class HumanAIAction3Pool : public HumanAIActionPool<3,0x40> {
public:
    template<std::size_t N>
    Lease acquire(std::uint32_t mode,std::uint32_t secondary,const std::array<float,N>& position){ //00582990.
        static_assert(N==3||N==4);
        auto result=acquire_record();auto& state=result->state;
        state.set(0x30,secondary);state.set(8,mode);state.set(0x20,position);
        return result;
    }
};
class HumanAIAction5Pool : public HumanAIActionPool<5,0x14> { //008AB048+C0, 0058A9F0.
public:
    Lease acquire(std::uint32_t mode){ //00582A40 leaves age and lifetime intact.
        auto result=acquire_record();
        result->state.set(8,mode);
        return result;
    }
};
static_assert(sizeof(HumanAIPooledAction<2,0x50>)==0x50);
static_assert(sizeof(HumanAIPooledAction<3,0x40>)==0x40);

//0058AB40 allocates native14 bytes aligned16; known fields stop at lifetime10.
//00582A90 only transfers the existing record;0054CB70 writes mode8 afterwards.
class HumanAIAction6Pool:public HumanAIActionPool<6,0x20> {
public:Lease acquire(){return acquire_record();}
};
class HumanAIAction7Pool:public HumanAIActionPool<7,0x50> { //0058AC90, manager+F0.
public:
    Lease acquire(const HumanVector& position,const HumanVector& direction){ //00582AD0.
        auto result=acquire_record();result->state.set(0x20,position);result->state.set(0x30,direction);return result;
    }
};
class HumanAIAction8Pool:public HumanAIActionPool<8,0x20> { //0058ADE0, manager+108.
public:
    Lease acquire(void* actor){ //00582B50; native18/1C cleared, age/mode/lifetime retained.
        auto result=acquire_record();result->actor=actor;
        result->state.set(0x18,0u);result->state.set(0x1c,0u);return result;
    }
};
class HumanAIAction12Pool:public HumanAIActionPool<12,0x40> { //0058B2C0, manager+168, NOT action4.
public:
    Lease acquire(const HumanVector& direction,void* actor){ //00582920; age/mode/lifetime retained.
        auto result=acquire_record();result->actor=actor;result->state.set(0x20,direction);return result;
    }
};
struct HumanAIActionPools {
    HumanAIAction1Pool kind1;
    HumanAIAction2Pool kind2;
    HumanAIAction3Pool kind3;
    HumanAIAction5Pool kind5;
    HumanAIAction6Pool kind6;
    HumanAIAction7Pool kind7;
    HumanAIAction8Pool kind8;
    HumanAIAction12Pool kind12;
};
} //namespace hd2::game
#endif
