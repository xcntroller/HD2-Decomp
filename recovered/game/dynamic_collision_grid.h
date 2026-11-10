#ifndef HD2_GAME_DYNAMIC_COLLISION_GRID_H
#define HD2_GAME_DYNAMIC_COLLISION_GRID_H

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

namespace hd2::game {

template<class Object>struct DynamicCollisionBucket {
    std::uint32_t x=0,z=0;
    std::vector<Object*> objects;
};

//006EF3E0/006EF5E0/006EF7F0/006EF330. The grid is the SAME dynamic head array
//allocated by006EE570. Objects own their descriptors and outlive registration;
//this owner releases bucket bindings before freeing any buckets. Object supplies
//position (descriptor+30) and bucket (descriptor+0C), with native pointer widths.
template<class Object,class Grid>
class DynamicCollisionBuckets {
public:
    using Bucket=DynamicCollisionBucket<Object>;
    //Source containing owner exists before006EE570. Queries/registration still
    //require the actual loaded grid; this does not manufacture empty geometry.
    DynamicCollisionBuckets()=default;
    explicit DynamicCollisionBuckets(Grid& grid){bind(grid);}
    void bind(Grid& grid) {
        if(grid_)throw std::logic_error("Dynamic collision buckets already bind their actual grid");
        if(!grid.count_x||!grid.count_z||std::uint64_t(grid.count_x)*grid.count_z!=grid.heads.size())
            throw std::runtime_error("Dynamic collision grid dimensions disagree");
        for(unsigned axis=0;axis!=2;++axis)
            if(!std::isfinite(grid.origin[axis])||!std::isfinite(grid.step[axis])||grid.step[axis]<=0)
                throw std::runtime_error("Invalid dynamic collision grid scale");
        for(auto* head:grid.heads)if(head)throw std::logic_error("Dynamic collision grid already has an owner");
        grid_=&grid;
    }
    bool empty()const noexcept{
        return std::none_of(owned_.begin(),owned_.end(),[](const auto& bucket){return !bucket->objects.empty();});
    }
    bool bound()const noexcept{return grid_!=nullptr;}
    const Grid& grid_identity()const{return grid();}
    void detach()noexcept{clear();grid_=nullptr;} //Containing tree may now retire.
    ~DynamicCollisionBuckets() {clear();}
    DynamicCollisionBuckets(const DynamicCollisionBuckets&)=delete;
    DynamicCollisionBuckets& operator=(const DynamicCollisionBuckets&)=delete;
    void insert(Object& object) {
        if(object.bucket)return;
        const auto index=cell(object);
        auto* bucket=acquire(index);
        bucket->objects.push_back(&object);
        object.bucket=bucket;
    }
    void remove(Object& object) {
        auto* bucket=object.bucket;
        if(!bucket)return;
        require_owned(bucket);
        const auto found=std::find(bucket->objects.begin(),bucket->objects.end(),&object);
        if(found!=bucket->objects.end()) {
            //Allocate before mutation so a native allocation failure cannot
            //leave a descriptor pointing into a half-removed bucket.
            if(bucket->objects.size()==1)deferred_.reserve(deferred_.size()+1);
            bucket->objects.erase(found); //Original preserves the remaining order.
            if(bucket->objects.empty())deferred_.push_back(bucket);
        }
        object.bucket=nullptr;
    }
    //Native lifetime guard for a descriptor whose owner is being destroyed.
    //Bucket retirement is an optimization; an allocation failure may leave an
    //empty bucket in its actual cell, but never a pointer to a freed descriptor.
    void remove_for_cleanup(Object& object) noexcept {
        auto* bucket=object.bucket;if(!bucket)return;
        if(std::none_of(owned_.begin(),owned_.end(),[&](const auto& owner){return owner.get()==bucket;}))return;
        const auto found=std::find(bucket->objects.begin(),bucket->objects.end(),&object);
        if(found!=bucket->objects.end()) {
            bucket->objects.erase(found);
            if(bucket->objects.empty())try {deferred_.push_back(bucket);}catch(...) {}
        }
        object.bucket=nullptr;
    }
    void move(Object& object) {
        const auto index=cell(object);
        auto* previous=object.bucket;
        if(previous==head(index))return;
        if(previous)require_owned(previous);
        auto* next=acquire(index);
        next->objects.reserve(next->objects.size()+1);
        remove(object);
        next->objects.push_back(&object);object.bucket=next;
    }
    void recycle_empty() { //006EF330: reverse deferred traversal, then LIFO reuse.
        free_.reserve(free_.size()+deferred_.size());
        while(!deferred_.empty()) {
            auto* bucket=deferred_.back();
            if(bucket->objects.empty()) {
                auto& slot=grid().heads[std::size_t(bucket->z)*grid().count_x+bucket->x];
                //A bucket may have been emptied more than once before this
                //pass. Avoid publishing duplicate native free-list pointers.
                if(slot==bucket) {slot=nullptr;free_.push_back(bucket);}
            }
            deferred_.pop_back();
        }
    }
    void clear() noexcept {
        for(auto& bucket:owned_)
            for(auto* object:bucket->objects)if(object->bucket==bucket.get())object->bucket=nullptr;
        if(grid_)for(auto& head:grid_->heads)head=nullptr;
        deferred_.clear();free_.clear();owned_.clear();
    }
    const Bucket* bucket(std::uint32_t x,std::uint32_t z) const {
        if(x>=grid().count_x||z>=grid().count_z)throw std::out_of_range("Dynamic collision grid cell");
        return head(std::size_t(z)*grid().count_x+x);
    }
    std::size_t pooled_count() const {return free_.size();}
private:
    Bucket* head(std::size_t index) const {return static_cast<Bucket*>(grid().heads[index]);}
    std::size_t cell(const Object& object) const {
        //Original x87 keeps subtraction/division extended until __ftol,
        //which truncates toward zero BEFORE the integer clamp.
        auto coordinate=[&](unsigned axis,std::uint32_t count) {
            const auto value=(static_cast<long double>(object.position[axis?2:0])-grid().origin[axis])/grid().step[axis];
            if(!std::isfinite(value)||value<std::numeric_limits<std::int32_t>::min()||
               value>std::numeric_limits<std::int32_t>::max())
                throw std::runtime_error("Dynamic collision position exceeds integer bounds");
            const auto integer=static_cast<std::int32_t>(value);
            return std::min<std::uint32_t>(count-1,integer<0?0:std::uint32_t(integer));
        };
        return std::size_t(coordinate(1,grid().count_z))*grid().count_x+coordinate(0,grid().count_x);
    }
    Bucket* acquire(std::size_t index) {
        if(auto* current=head(index))return current;
        Bucket* result;
        if(free_.empty()) {
            auto next=std::make_unique<Bucket>();next->objects.reserve(8);
            owned_.push_back(std::move(next));result=owned_.back().get();
        } else {result=free_.back();free_.pop_back();}
        result->x=std::uint32_t(index%grid().count_x);result->z=std::uint32_t(index/grid().count_x);
        grid().heads[index]=result;return result;
    }
    void require_owned(const Bucket* bucket) const {
        if(std::none_of(owned_.begin(),owned_.end(),[&](const auto& owner){return owner.get()==bucket;}))
            throw std::logic_error("Dynamic collision descriptor belongs to another grid");
    }
    Grid& grid()const{if(!grid_)throw std::logic_error("Dynamic collision requires its actual loaded grid");return *grid_;}
    Grid* grid_=nullptr;
    std::vector<std::unique_ptr<Bucket>> owned_;
    std::vector<Bucket*> free_,deferred_;
};

} //namespace hd2::game
#endif
