#ifndef HD2_GAME_HUMAN_AI_EVENT_QUEUE_H
#define HD2_GAME_HUMAN_AI_EVENT_QUEUE_H

#include <array>
#include <cstdint>
#include <cstring>
#include <deque>
#include <memory>
#include <stdexcept>
#include <vector>

namespace hd2::game {

//Native008AC320 pool,005AC960/0054CAD0. Records are aligned64-byte values;
//unknown initial words are defined guards, never claimed as recovered fields.
//Return does not reset the record: subsequent producers preserve unwritten data.
struct alignas(16) HumanAIEvent {std::array<std::uint32_t,16> words{};};
static_assert(sizeof(HumanAIEvent)==64&&alignof(HumanAIEvent)==16);
class HumanAIEventPool {
public:
    struct Return {
        HumanAIEventPool* pool=nullptr;
        void operator()(HumanAIEvent* event)const noexcept{if(event)pool->give(event);}
    };
    using Lease=std::unique_ptr<HumanAIEvent,Return>;
    HumanAIEventPool(){grow();} //Original static initializer eagerly allocates50 records.
    HumanAIEventPool(const HumanAIEventPool&)=delete;HumanAIEventPool& operator=(const HumanAIEventPool&)=delete;
    Lease acquire() {
        if(free_.empty())grow();
        auto* event=free_.back();free_.pop_back();return Lease(event,Return{this});
    }
    std::size_t available()const{return free_.size();}
    std::size_t allocated()const{return records_.size();}
    std::int32_t growth_batch()const{return batch_;}
    void growth_batch(std::int32_t value){batch_=value;} //Native0C has additional real writers outside these producers.
private:
    void give(HumanAIEvent* event)noexcept{free_.push_back(event);} //005AE450 pushes onto the SAME free list.
    void grow() {
        if(batch_<=0)throw std::runtime_error("Human AI event pool cannot grow with its actual nonpositive batch");
        const auto count=records_.size()+static_cast<std::uint32_t>(batch_);
        records_.reserve(count);free_.reserve(count);
        for(std::int32_t n=0;n!=batch_;++n) {
            auto event=std::make_unique<HumanAIEvent>();event->words[1]=0;
            free_.push_back(event.get());records_.push_back(std::move(event));
        }
    }
    //Containing process outlives all command queues/leases, as does the native pool.
    std::vector<std::unique_ptr<HumanAIEvent>> records_;std::vector<HumanAIEvent*> free_;
    std::int32_t batch_=50;
};

//005ADAC0 enqueue and005AE450 retirement. Native uses1024-pointer deque blocks;
//internal STL allocation layout is not exposed as a recovered ABI. Lazy storage
//preserves allocation-free005AA890 cold queues. Reverse clear returns LIFO.
class HumanAIEventQueue {
public:
    using Lease=HumanAIEventPool::Lease;
    void bind(HumanAIEventPool& pool) {
        if(size()&&pool_!=&pool)throw std::logic_error("Cannot replace the pool of live Human AI events");
        pool_=&pool;
    }
    HumanAIEventPool& pool()const {if(!pool_)throw std::runtime_error("Human AI event queue requires the actual shared pool");return *pool_;}
    void push(Lease event) {
        if(!event||event.get_deleter().pool!=&pool())throw std::logic_error("Human AI event belongs to a different pool");
        if(!events_)events_=std::make_unique<std::deque<Lease>>();
        events_->push_back(std::move(event));
    }
    Lease pop_front() { //005ADC50; the caller's earlier pushed1 belongs to the FOLLOWING005ADEE0 call.
        if(!size())return Lease(nullptr,HumanAIEventPool::Return{});
        auto result=std::move(events_->front());events_->pop_front();
        if(events_->empty())events_.reset();
        return result;
    }
    void merge(Lease event,std::uint32_t lifetime,bool keep_priority) { //005ADEE0, SECOND queue.
        if(!event||event.get_deleter().pool!=&pool())throw std::logic_error("Processed Human AI event belongs to a different pool");
        event->words[1]=lifetime;
        const auto key=!(event->words[3]&0x80000000u)?3u:!(event->words[4]&0x80000000u)?4u:0u;
        if(key)for(auto n=size();n;) {
            auto& old=events_->at(--n);
            if(old->words[key]!=event->words[key])continue;
            if(signed_word(lifetime)<signed_word(old->words[1]))event->words[1]=old->words[1];
            if(key==4&&keep_priority&&(event->words[0]&0xf0000000u)<(old->words[0]&0xf0000000u))
                event->words[0]=old->words[0]; //Native copies the ENTIRE kind word, only in this target-ID branch.
            old=std::move(event);return;
        }
        push(std::move(event));
    }
    const HumanAIEvent* at(std::size_t index)const{return events_?events_->at(index).get():nullptr;}
    std::size_t size()const{return events_?events_->size():0;}
    void clear()noexcept{if(events_){while(!events_->empty())events_->pop_back();events_.reset();}}
    ~HumanAIEventQueue(){clear();}
private:
    static std::int64_t signed_word(std::uint32_t value){return value&0x80000000u?static_cast<std::int64_t>(value)-0x100000000ll:value;}
    HumanAIEventPool* pool_=nullptr;std::unique_ptr<std::deque<Lease>> events_;
};

//00574070/00574130/005741F0. Actor handles005ACA30 are signed actor10 values,
//not a new reference registry. Queue is actual Human command's first deque.
template<class Frames,class Actor>
void queue_human_ai_sight_event(Frames& frames,HumanAIEventQueue& queue,Actor& actor,unsigned kind) {
    if(kind>2)throw std::out_of_range("Human AI sight event kind");
    std::array<float,4> position{};
    if(kind!=2) {
        const auto matrix=frames.actor_world_matrix(actor.base().frame());
        position={{matrix[12],matrix[13],matrix[14],matrix[15]}};
    }
    auto event=queue.pool().acquire();
    event->words[0]=kind==0?0x30000001u:kind==1?0x10000002u:0x20000004u;
    event->words[1]=10000;event->words[2]=0;event->words[12]=0;
    const auto handle=static_cast<std::uint32_t>(actor.base().id10());
    event->words[3]=kind==2?handle:0xffffffffu;event->words[4]=kind==2?0xffffffffu:handle;
    if(kind==2)event->words[8]=event->words[9]=event->words[10]=0; //Dead producer leaves W2C untouched.
    else std::memcpy(event->words.data()+8,position.data(),16);
    queue.push(std::move(event));
}
} //namespace hd2::game
#endif
