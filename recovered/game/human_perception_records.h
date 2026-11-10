#ifndef HD2_GAME_HUMAN_PERCEPTION_RECORDS_H
#define HD2_GAME_HUMAN_PERCEPTION_RECORDS_H

#include "game/human_joint_math.h"
#include <memory>
#include <vector>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace hd2::game {

//Human2AC sound-perception records, process008AB048+48 (pool008AB090).
//00582740/0058A510: aligned20-byte native records, borrowed ACTOR pointer14.
//Host64 keeps that pointer typed; this is not a serialized/native ABI overlay.
template<class Actor>struct alignas(16) HumanPerceptionRecord {
    HumanVector position{};std::uint32_t kind=0;Actor actor{};
    std::uint32_t timestamp=0;std::uint8_t refreshed=0;
};
#if UINTPTR_MAX == 0xffffffffu
static_assert(sizeof(HumanPerceptionRecord<void*>)==0x20);
static_assert(offsetof(HumanPerceptionRecord<void*>,actor)==0x14);
#endif
template<class Actor>class HumanPerceptionRecordPool {
public:
    using Record=HumanPerceptionRecord<Actor>;
    struct Return {HumanPerceptionRecordPool* pool=nullptr;void operator()(Record* record)const noexcept{if(record)pool->give(record);}};
    using Lease=std::unique_ptr<Record,Return>;
    HumanPerceptionRecordPool(){grow();} //00582030's family+48 eagerly allocates16.
    HumanPerceptionRecordPool(const HumanPerceptionRecordPool&)=delete;
    HumanPerceptionRecordPool& operator=(const HumanPerceptionRecordPool&)=delete;
    Lease acquire(std::uint32_t kind,const HumanVector& position,Actor actor,std::uint32_t timestamp) {
        if(free_.empty())grow();
        auto* record=free_.back();free_.pop_back();
        record->kind=kind;record->position=position;record->actor=actor;
        record->timestamp=timestamp;record->refreshed=0;
        return Lease(record,Return{this});
    }
    std::size_t available()const{return free_.size();}std::size_t allocated()const{return records_.size();}
    std::int32_t growth_batch()const{return batch_;}void growth_batch(std::int32_t value){batch_=value;}
private:
    void give(Record* record)noexcept{free_.push_back(record);} //0059FA60 reverse list→pool094, no reset.
    void grow() {
        if(batch_<=0)throw std::runtime_error("Human perception pool has a nonpositive growth batch");
        const auto count=records_.size()+static_cast<std::uint32_t>(batch_);
        records_.reserve(count);free_.reserve(count);
        for(std::int32_t n=0;n!=batch_;++n){auto record=std::make_unique<Record>();free_.push_back(record.get());records_.push_back(std::move(record));}
    }
    std::vector<std::unique_ptr<Record>> records_;std::vector<Record*> free_;std::int32_t batch_=16;
};

template<class Actor>class HumanPerceptionRecords {
public:
    using Pool=HumanPerceptionRecordPool<Actor>;using Record=typename Pool::Record;
    void bind(Pool& pool){if(!records_.empty()&&pool_!=&pool)throw std::logic_error("Cannot replace a populated Human perception pool");pool_=&pool;}
    bool empty()const{return records_.empty();}std::size_t size()const{return records_.size();}
    Record* at(std::size_t index)const{return records_.at(index).get();}
    template<class Clock,class Notify>void append_or_refresh(std::uint32_t kind,const HumanVector& position,Actor actor,
            Clock&& ticks,Notify&& notify) { //005A5430 record tail AFTER its actual sound gates.
        for(auto n=records_.size();n;) {
            auto* record=records_[--n].get();
            if(record->actor==actor&&record->kind==kind) {
                record->position=position;record->timestamp=ticks();record->refreshed=1;notify(*record);return;
            }
        }
        if(!pool_)throw std::runtime_error("Human perception records require the actual shared process pool");
        const auto timestamp=ticks(); //New record: timeGetTime BEFORE pool acquisition, refresh: AFTER position write.
        auto record=pool_->acquire(kind,position,actor,timestamp);auto* result=record.get();records_.push_back(std::move(record));
        notify(*result); //Native B0+B4 sees the newly appended record; never replaced by a no-op callback.
    }
    void clear()noexcept{while(!records_.empty())records_.pop_back();} //0059FA60, retain list allocation.
    void expire(std::uint32_t now){ //005A5380, SAME timeGetTime basis as append/refresh.
        for(auto n=records_.size();n;){
            auto& record=records_[--n];if(!record)throw std::logic_error("Human perception expiry encountered a missing actual record");
            const auto bits=now-record->timestamp;std::int32_t elapsed;std::memcpy(&elapsed,&bits,4);
            if(elapsed<=3000)continue;
            record.reset(); //Return SAME record to process pool BEFORE compacting the list; strict>3000.
            records_.erase(records_.begin()+static_cast<std::ptrdiff_t>(n));
        }
    }
    ~HumanPerceptionRecords(){clear();}
private:
    Pool* pool_=nullptr;std::vector<typename Pool::Lease> records_;
};
} //namespace hd2::game
#endif
