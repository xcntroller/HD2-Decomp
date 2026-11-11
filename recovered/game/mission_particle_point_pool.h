#ifndef HD2_GAME_MISSION_PARTICLE_POINT_POOL_H
#define HD2_GAME_MISSION_PARTICLE_POINT_POOL_H
#include "game/mission_particle_point.h"
#include <memory>
#include <cassert>

namespace hd2::game {
//SAME008998C4 process point pool.00440452 sets refill count0x400.004437C0
//allocates alignedB0 points and prepends each;00440DC0 pops a point and returns
//it on failed initialization.004412D0 and004406A0 return without deleting.
//The process owner must outlive every instance and all of its point leases.
class MissionParticlePointPool {
public:
    struct Return {
        MissionParticlePointPool* pool=nullptr;
        void operator()(MissionParticlePoint* point)const noexcept{
            if(!point)return;
            assert(pool&&pool->active_&&pool->free_.size()<pool->free_.capacity());
            pool->free_.push_back(point);--pool->active_; //Capacity covers every allocated point; no allocation here.
        }
    };
    using Lease=std::unique_ptr<MissionParticlePoint,Return>;
    MissionParticlePointPool()=default;
    ~MissionParticlePointPool(){assert(!active_);}
    MissionParticlePointPool(const MissionParticlePointPool&)=delete;
    MissionParticlePointPool& operator=(const MissionParticlePointPool&)=delete;
    Lease acquire(){
        if(free_.empty()){
            constexpr std::size_t batch=0x400;
            free_.reserve(owned_.size()+batch);owned_.reserve(owned_.size()+batch);
            for(std::size_t n=0;n!=batch;++n){auto point=std::make_unique<MissionParticlePoint>();
                auto* value=point.get();owned_.push_back(std::move(point));free_.push_back(value);}
        }
        auto* point=free_.back();free_.pop_back();++active_;return Lease(point,Return{this});
    }
    std::size_t available()const{return free_.size();}std::size_t allocated()const{return owned_.size();}
    std::size_t active()const{return active_;}
private:
    std::vector<std::unique_ptr<MissionParticlePoint>> owned_;std::vector<MissionParticlePoint*> free_;
    std::size_t active_=0;
};
} //namespace hd2::game
#endif
