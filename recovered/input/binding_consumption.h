#ifndef HD2_INPUT_BINDING_CONSUMPTION_H
#define HD2_INPUT_BINDING_CONSUMPTION_H
#include "input/binding_producer.h"
#include <algorithm>
namespace hd2::input {
//006C70A0 compares ONLY kind/code, including aliases on other devices and
//directions.006C71C0 compares ONLY the16-bit code against the full scan DWORD.
//Both visit backwards and clear BOTH output snapshots. No polling/device owner.
template<class Clear>void consume_binding_action(const BindingInputState& state,std::size_t action,Clear clear){
    if(state.bindings.empty())return;
    const auto selected=state.bindings.at(action);
    for(auto n=state.bindings.size();n;){--n;const auto& value=state.bindings[n];if(value.device_kind==selected.device_kind&&value.code==selected.code)clear(n);}
}
template<class Clear>void consume_binding_scan(const BindingInputState& state,std::uint32_t scan,Clear clear){
    for(auto n=state.bindings.size();n;){--n;if(state.bindings[n].code==scan)clear(n);}
}
//In-call views of original first/second buffers. Either absent argument selects
//the manager's own pair, matching native OR gate. Buffers are never retained.
class BindingInputViews {
public:
    BindingInputViews(BindingInputState& state,std::uint32_t* first=nullptr,std::uint32_t* second=nullptr,std::size_t count=0):state_(state){
        if(!first||!second){first_=state.first.data();second_=state.second.data();count_=std::min(state.first.size(),state.second.size());}
        else{first_=first;second_=second;count_=count;}
        if(count_<state.bindings.size())throw std::invalid_argument("Input views do not cover actual binding records");
    }
    float first(std::size_t index)const{check(index);float value;std::memcpy(&value,first_+index,4);return value;}
    void first(std::size_t index,float value){check(index);std::memcpy(first_+index,&value,4);}
    float second(std::size_t index)const{check(index);float value;std::memcpy(&value,second_+index,4);return value;}
    void second(std::size_t index,float value){check(index);std::memcpy(second_+index,&value,4);}
    void consume_action(std::size_t action){consume_binding_action(state_,action,[&](std::size_t index){clear(index);});}
    void consume_scan(std::uint32_t scan){consume_binding_scan(state_,scan,[&](std::size_t index){clear(index);});}
private:
    void check(std::size_t index)const{if(index>=count_)throw std::out_of_range("Input view exceeds actual output buffer");}
    void clear(std::size_t index){check(index);first_[index]=second_[index]=0;}
    BindingInputState& state_;std::uint32_t* first_=nullptr;std::uint32_t* second_=nullptr;std::size_t count_=0;
};
} //namespace hd2::input
#endif
