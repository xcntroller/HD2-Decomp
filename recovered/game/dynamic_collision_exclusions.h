#ifndef HD2_GAME_DYNAMIC_COLLISION_EXCLUSIONS_H
#define HD2_GAME_DYNAMIC_COLLISION_EXCLUSIONS_H

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vector>

namespace hd2::game {

//006EFA70/006EFB10 shared query exclusion state. Bit8 belongs to descriptor+08
//layers, not descriptor+00 flags. Descriptors must outlive this query window.
//Snapshotting the borrowed array protects native vector relocations; valid
//callers do not alter membership until the matching reset.
template<class Object>
class DynamicCollisionExclusions {
public:
    ~DynamicCollisionExclusions(){clear();}
    DynamicCollisionExclusions()=default;
    DynamicCollisionExclusions(const DynamicCollisionExclusions&)=delete;
    DynamicCollisionExclusions& operator=(const DynamicCollisionExclusions&)=delete;
    void single(Object* object,const void* receiver=nullptr) { //006EFA70.
        std::vector<Object*> next;if(object)next.push_back(object);replace(std::move(next),receiver);
    }
    void many(Object* const* objects,std::size_t count,const void* receiver=nullptr) { //006EFB10.
        std::vector<Object*> next;
        if(objects) {
            next.reserve(count);
            for(std::size_t index=0;index!=count;++index) {
                if(!objects[index])throw std::runtime_error("Null dynamic collision exclusion");
                next.push_back(objects[index]);
            }
        }
        replace(std::move(next),receiver);
    }
    void clear() noexcept {for(auto* object:objects_)object->mask&=0xfffffff7u;objects_.clear();receiver_=nullptr;}
    std::size_t count() const {return objects_.size();}
    const void* receiver() const {return receiver_;}
private:
    void replace(std::vector<Object*> next,const void* receiver) {
        clear();objects_=std::move(next);receiver_=receiver;
        for(auto* object:objects_)object->mask|=8;
    }
    std::vector<Object*> objects_;
    const void* receiver_=nullptr;
};
} //namespace hd2::game
#endif
