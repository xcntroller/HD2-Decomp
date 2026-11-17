#ifndef HD2_GAME_HUMAN_FRAME_INPUTS_H
#define HD2_GAME_HUMAN_FRAME_INPUTS_H
#include "game/human_joint_owners.h"

namespace hd2::game {
//Owned packets/AI/local snapshots keep value semantics. Player74/78 may instead
//borrow the application manager's actual mutable256-byte arrays. Copying a view
//captures its current bytes into an OWNED value, never another persistent loan.
struct HumanFrameInputBuffer:HumanJointWords<0x100>{
    HumanFrameInputBuffer()=default;
    HumanFrameInputBuffer(const HumanFrameInputBuffer& other){*this=other;}
    HumanFrameInputBuffer& operator=(const HumanFrameInputBuffer& other){
        if(this!=&other){std::memmove(bytes.data(),other.data(),bytes.size());borrowed_=nullptr;}return *this;
    }
    void borrow(std::uint32_t* words)noexcept{borrowed_=words;}
    std::uint32_t* borrowed_words()const noexcept{return borrowed_;}
    template<class Value>Value get(std::size_t offset)const{
        static_assert(std::is_trivially_copyable_v<Value>);
        if(offset>bytes.size()||sizeof(Value)>bytes.size()-offset)throw std::out_of_range("Human input scalar offset");
        Value value;std::memcpy(&value,data()+offset,sizeof value);return value;
    }
    template<class Value>void set(std::size_t offset,const Value& value){
        static_assert(std::is_trivially_copyable_v<Value>);
        if(offset>bytes.size()||sizeof(Value)>bytes.size()-offset)throw std::out_of_range("Human input scalar offset");
        auto* out=borrowed_?reinterpret_cast<std::uint8_t*>(borrowed_):bytes.data();std::memcpy(out+offset,&value,sizeof value);
    }
private:
    const std::uint8_t* data()const noexcept{return borrowed_?reinterpret_cast<const std::uint8_t*>(borrowed_):bytes.data();}
    std::uint32_t* borrowed_=nullptr;
};
//Process BSS008AAF48/008AAE48, shared sequentially by AI frame callbacks.
//These are not network input7C or per-Human replacement buffers.
struct HumanAIFrameInputs {
    HumanFrameInputBuffer first,second;
    void clear(){first.bytes.fill(0);second.bytes.fill(0);}
};
} //namespace hd2::game
#endif
