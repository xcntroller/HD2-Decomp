#ifndef HD2_FRONTEND_SOUND_LISTENERS_H
#define HD2_FRONTEND_SOUND_LISTENERS_H

#include "frontend/sound_runtime.h"

namespace hd2::frontend {

//00433ED0/00433E80, SAME SoundRuntimeState+0C. Borrowed entries; slot reuse
//scans backwards by CALLBACK, not frame identity. Removal preserves list length.
template<class Frame,class Resolve,class Type>
std::int32_t add_sound_listener(SoundRuntimeState& state,std::uint32_t mode,Frame frame,
        void* callback,Resolve&& resolve,Type&& type) {
    if(!(mode&1u))return -1;
    std::int32_t slot=-1;
    for(auto n=state.registrations.size();n;)if(!state.registrations[--n][2]){slot=static_cast<std::int32_t>(n);break;}
    auto* actor=resolve(frame);
    if(!actor)throw std::runtime_error("Sound listener requires its actual frame actor"); //Native null dereference rejected.
    std::array<void*,3> record{{frame,type(actor)==3?static_cast<void*>(actor):nullptr,callback}};
    if(slot!=-1){state.registrations[static_cast<std::size_t>(slot)]=record;return slot;}
    if(state.registrations.size()>=0x7fffffffu)throw std::length_error("Sound listener exceeds original signed slot range");
    state.registrations.push_back(record);return static_cast<std::int32_t>(state.registrations.size()-1);
}
inline bool remove_sound_listener(SoundRuntimeState& state,std::uint32_t mode,std::int32_t slot) { //00433E80.
    if(!(mode&1u)||slot<0||static_cast<std::size_t>(slot)>=state.registrations.size())return false;
    auto& record=state.registrations[static_cast<std::size_t>(slot)];if(!record[2])return false;
    record.fill(nullptr);return true;
}
} //namespace hd2::frontend
#endif
