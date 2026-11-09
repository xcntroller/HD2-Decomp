#ifndef HD2_GAME_HUMAN_SOUND_POOL_H
#define HD2_GAME_HUMAN_SOUND_POOL_H

#include "game/human_sound_definitions.h"
#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace hd2::game {

//00784C40/0098A698 process state. Its randomized windows survive pool deletion
//and repeated worlds. Borrow the SAME client CRT random source as speech/items.
class HumanSoundVariants {
public:
    template<class Random>std::size_t count(std::size_t group,Random& random) {
        if(group>=orders_.size())throw std::out_of_range("Human sound group");
        if(!initialized_) {
            std::array<std::vector<std::uint32_t>,20> next;
            for(unsigned g=0;g!=next.size();++g)for(unsigned index=0;index!=6;++index) {
                if(!human_sound_definitions[g][index].name)break;
                const auto value=random();if(value>0x7fffu)throw std::runtime_error("Human sound pool requires shared CRT random");
                auto& order=next[g];const auto position=value%(order.size()+2);
                if(position<order.size())order.insert(order.begin()+static_cast<std::ptrdiff_t>(position),index);
                else order.push_back(index);
            }
            orders_.swap(next);initialized_=true;
        }
        return orders_[group].size();
    }
    bool initialized() const {return initialized_;}
    const auto& orders() const {return orders_;}
private:
    std::array<std::vector<std::uint32_t>,20> orders_;
    bool initialized_=false;
};

//00784B70/00784F60/00784ED0 actual kind4 sound-frame pool. Each variant has two
//native cache slots; construction takes one real frame and stops/caches it.
//take() transfers its retained owner to the actual Human audio consumer. Return
//producers/selection/playback remain separate until recovered.
template<class Frames,class Random>
class HumanSoundPool {
public:
    using Frame=typename Frames::Frame;
    using Owner=decltype(std::declval<Frames&>().create_human_sound());
    HumanSoundPool(Frames& frames,HumanSoundVariants& variants,Random& random)
        :frames_(frames),variants_(variants),random_(random) {
        for(std::size_t group=0;group!=cached_.size();++group) {
            cached_[group].resize(variants_.count(group,random_));
            for(std::size_t index=0;index!=cached_[group].size();++index) {
                auto sound=take(group,index);
                if(sound) {
                    frames_.human_sound_enabled(sound.get(),false);
                    cached_[group][index].push_back(std::move(sound));
                }
            }
        }
    }
    ~HumanSoundPool(){clear();}
    HumanSoundPool(const HumanSoundPool&)=delete;
    HumanSoundPool& operator=(const HumanSoundPool&)=delete;
    Owner take(std::size_t group,std::size_t index) {
        if(!frames_.human_sound_available())return Owner(nullptr,nullptr);
        if(group>=cached_.size()||index>=cached_[group].size())throw std::out_of_range("Human sound variant");
        const auto& definition=human_sound_definitions[group][index];
        auto& cache=cached_[group][index];
        if(!cache.empty()) {
            auto sound=std::move(cache.back());cache.pop_back();
            frames_.human_sound_parameter94(sound.get(),0);
            frames_.human_sound_enabled(sound.get(),true);
            frames_.human_sound_volume(sound.get(),real(definition.volume_bits));
            return sound;
        }
        auto sound=frames_.create_human_sound();
        if(!sound)throw std::runtime_error("Cannot create Human sound frame");
        const auto status=frames_.load_human_sound(sound.get(),definition.name);
        if(status) {
            if(std::uint32_t(status)!=0x80000009u)frames_.human_sound_missing(definition.name,status);
            return Owner(nullptr,nullptr); //Local real frame releases on load failure.
        }
        frames_.human_sound_parameter7c(sound.get(),1);
        frames_.human_sound_volume(sound.get(),real(definition.volume_bits));
        frames_.human_sound_range(sound.get(),real(definition.near_bits),real(definition.far_bits),.5f,.5f);
        frames_.human_sound_parameter90(sound.get(),0);
        frames_.human_sound_parameter94(sound.get(),0);
        frames_.human_sound_enabled(sound.get(),true);
        frames_.human_sound_tag(sound.get(),std::uint32_t(group<<8)|std::uint32_t(index));
        return sound;
    }
    void give(Owner sound) { //00785400; caller's0075EA30 already holds this shared pool.
        if(!sound)return;
        const auto tag=frames_.human_sound_tag(sound.get());
        const auto group=tag>>8,index=tag&0xffu;
        if(group>=cached_.size()||index>=variants_.count(group,random_)) {
            frames_.human_sound_detach(sound.get());return;
        }
        frames_.human_sound_detach(sound.get()); //Frame+38(sound,0,0), not a playback stop.
        frames_.human_sound_enabled(sound.get(),false);
        auto& cache=cached_[group][index];
        if(cache.size()<2)cache.push_back(std::move(sound));
    }
    void clear() noexcept { //Original group/variant/cached-slot ascending destruction.
        for(auto& group:cached_) {
            for(auto& cache:group){for(auto& sound:cache)sound.reset();cache.clear();}
            group.clear();
        }
    }
    std::size_t cached_count(std::size_t group,std::size_t index) const {return cached_.at(group).at(index).size();}
private:
    static float real(std::uint32_t bits) {float value;std::memcpy(&value,&bits,4);return value;}
    Frames& frames_;HumanSoundVariants& variants_;Random& random_;
    std::array<std::vector<std::vector<Owner>>,20> cached_;
};

//0098A690/0098A694 ownership portion of007855C0/00785950. The containing
//world owns this manager and destroys Human control leases before it. Variant
//windows are separately process-owned and survive a zero-use pool destruction.
template<class Frames,class Random>
class HumanSoundPoolManager {
public:
    using Pool=HumanSoundPool<Frames,Random>;
    class Lease {
    public:
        ~Lease(){if(manager_)manager_->release(pool_);}
        Lease(const Lease&)=delete;
        Lease& operator=(const Lease&)=delete;
        Lease(Lease&& other) noexcept:manager_(std::exchange(other.manager_,nullptr)),pool_(other.pool_) {}
        Pool& pool() const {return *pool_;}
    private:
        friend class HumanSoundPoolManager;
        Lease(HumanSoundPoolManager& manager,Pool& pool):manager_(&manager),pool_(&pool) {}
        HumanSoundPoolManager* manager_;Pool* pool_;
    };
    HumanSoundPoolManager(Frames& frames,HumanSoundVariants& variants,Random& random)
        :frames_(frames),variants_(variants),random_(random) {}
    HumanSoundPoolManager(const HumanSoundPoolManager&)=delete;
    HumanSoundPoolManager& operator=(const HumanSoundPoolManager&)=delete;
    Lease acquire() {
        if(users_==std::uint32_t(std::numeric_limits<std::int32_t>::max()))throw std::runtime_error("Human sound pool reference overflow");
        if(!pool_)pool_=std::make_unique<Pool>(frames_,variants_,random_);
        ++users_;return Lease(*this,*pool_);
    }
    std::uint32_t use_count() const {return users_;}
private:
    void release(Pool* pool) noexcept {if(pool_.get()==pool&&users_&&!--users_)pool_.reset();}
    Frames& frames_;HumanSoundVariants& variants_;Random& random_;
    std::unique_ptr<Pool> pool_;
    std::uint32_t users_=0;
};

} //namespace hd2::game
#endif
