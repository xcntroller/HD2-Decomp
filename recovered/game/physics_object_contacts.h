#ifndef HD2_GAME_PHYSICS_OBJECT_CONTACTS_H
#define HD2_GAME_PHYSICS_OBJECT_CONTACTS_H

#include <cstdint>
#include <limits>
#include <stdexcept>

namespace hd2::game {

struct PhysicsObjectHandle {std::int32_t index=-1;bool first_list=false;};

//00410530: a contact moves an actual second-list object to the active first
//list BEFORE its virtual14 receiver, even when that receiver returns false.
//The moved last object's stable handle must be repaired before the callback.
template<class Owners,class Contact>bool contact_physics_object(Owners& first,Owners& second,
        PhysicsObjectHandle* handle,Contact&& contact){
    if(!handle||handle->index==-1)return false;
    if(handle->index<0)throw std::out_of_range("Invalid physics contact handle");
    auto index=static_cast<std::size_t>(handle->index);
    auto& current=handle->first_list?first:second;
    if(index>=current.size())throw std::out_of_range("Physics contact handle exceeds its actual list");
    if(!current[index])return false;
    if(&current[index]->handle()!=handle)throw std::logic_error("Physics contact handle names another owner");
    if(!handle->first_list){
        if(first.size()>static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()))
            throw std::length_error("Physics contact exceeds native active index range");
        first.reserve(first.size()+1); //Allocate before transferring any owner.
        auto owner=std::move(second[index]);
        if(index+1!=second.size()){
            second[index]=std::move(second.back());
            if(second[index])second[index]->handle().index=static_cast<std::int32_t>(index);
        }
        second.pop_back();
        index=first.size();first.push_back(std::move(owner));
        handle->index=static_cast<std::int32_t>(index);handle->first_list=true;
    }
    return contact(*first[index]);
}

} //namespace hd2::game
#endif
