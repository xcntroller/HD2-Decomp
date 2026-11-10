#ifndef HD2_GAME_HUMAN_AI_NAVIGATION_JOBS_H
#define HD2_GAME_HUMAN_AI_NAVIGATION_JOBS_H

#include "game/human_joint_owners.h"

namespace hd2::game {
struct alignas(16) HumanAINavigationJob {
    HumanJointWords<0x30> words;
};
static_assert(sizeof(HumanAINavigationJob)==0x30);

//The navigation job family at process008AB048+30. 0058A4A0 adds aligned
//records without native constructor stores. 005638E0 returns the same record.
class HumanAINavigationJobPool {
public:
    struct Return {
        HumanAINavigationJobPool* pool=nullptr;
        void operator()(HumanAINavigationJob* job)const noexcept{if(job)pool->free_.push_back(job);}
    };
    using Lease=std::unique_ptr<HumanAINavigationJob,Return>;
    HumanAINavigationJobPool(){grow();}
    HumanAINavigationJobPool(const HumanAINavigationJobPool&)=delete;
    HumanAINavigationJobPool& operator=(const HumanAINavigationJobPool&)=delete;
    template<std::size_t N>Lease acquire(std::uint32_t index,const std::array<float,N>& position,
            std::uint16_t flags,std::uint8_t kind,float radius){ //00582670.
        static_assert(N==3||N==4);
        if(free_.empty())grow();
        auto* job=free_.back();
        free_.pop_back();
        auto& words=job->words;
        words.set(0x20,index);
        words.set(0,position);
        words.set(0x10,position);
        words.set(0x24,flags);
        words.set(0x26,kind);
        words.set(0x28,radius);
        return Lease(job,Return{this});
    }
    std::size_t available()const{return free_.size();}
    std::size_t allocated()const{return records_.size();}
private:
    void grow(){
        constexpr unsigned batch=16;
        const auto capacity=records_.size()+batch;
        records_.reserve(capacity);
        free_.reserve(capacity);
        for(unsigned n=0;n<batch;++n){
            auto job=std::make_unique<HumanAINavigationJob>();
            free_.push_back(job.get());
            records_.push_back(std::move(job));
        }
    }
    std::vector<std::unique_ptr<HumanAINavigationJob>> records_;
    std::vector<HumanAINavigationJob*> free_;
};
} //namespace hd2::game
#endif
