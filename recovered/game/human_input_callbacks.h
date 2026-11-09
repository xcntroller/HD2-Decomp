#ifndef HD2_GAME_HUMAN_INPUT_CALLBACKS_H
#define HD2_GAME_HUMAN_INPUT_CALLBACKS_H

#include "game/human_input_records.h"

namespace hd2::game {

//0073D000/0073CF50 cursor operations. Storage supplies the actual buffer and
//the native virtual+0 capacity receiver before each write. This layer neither
//creates a network transport nor substitutes for the containing stream owner.
template<class Storage>
class HumanInputBits {
public:
    explicit HumanInputBits(Storage& storage):storage_(storage) {}
    void position(std::size_t word,unsigned bit) {
        if(bit>=32)throw std::out_of_range("Human input bit cursor");
        word_=word;bit_=bit;
    }
    std::size_t word_position()const{return word_;}unsigned bit_position()const{return bit_;}
    std::uint32_t read(unsigned count) { //0073D000, low bits first across DWORDs.
        const auto mask=field_mask(count);const auto total=bit_+count;
        const auto current=checked_word(word_);
        const auto result=(current>>bit_)&mask;
        if(total<32){bit_=total;return result;}
        const auto remaining=total&31u;
        //Check malformed input before advancing the cursor.
        const auto next=remaining?checked_word(word_+1):0u;
        ++word_;bit_=remaining;
        return remaining?result|((next<<((count-remaining)&31u))&mask):result;
    }
    void write(std::uint32_t value,unsigned count) { //0073CF50(value,count), ret8.
        const auto mask=field_mask(count);const auto total=bit_+count;
        storage_.reserve_bits(word_,bit_,count); //Actual virtual+0 precedes buffer lookup.
        checked_word(word_);
        if(total>32)checked_word(word_+1);
        auto& current=storage_.word(word_);
        current=(current&~(mask<<bit_))|(value<<bit_); //Original value is deliberately unmasked.
        if(total<32){bit_=total;return;}
        const auto remaining=total&31u;++word_;bit_=remaining;
        if(remaining) {
            const auto shift=(count-remaining)&31u;
            auto& next=storage_.word(word_);
            next=(next&~(mask>>shift))|(value>>shift);
        }
    }
private:
    static std::uint32_t field_mask(unsigned count) {
        if(count>32)throw std::out_of_range("Human input field exceeds native mask table");
        return count==32?0xffffffffu:((std::uint32_t{1}<<count)-1u);
    }
    std::uint32_t checked_word(std::size_t index)const {
        if(index>=storage_.word_count())throw std::runtime_error("Truncated Human input bit buffer");
        return storage_.word(index);
    }
    Storage& storage_;std::size_t word_=0;unsigned bit_=0;
};

//0073F770 and the three allocation branches in0053B760. Common profile
//pointers are typed borrowers; unknown derived bytes remain unclaimed. Value
//serialization/dispatch is not exposed until its concrete consumers are ready.
class HumanInputMessage {
public:
    explicit HumanInputMessage(std::uint32_t kind):kind_(kind) {
        if(kind>2)throw std::out_of_range("Unknown Human input message kind");
        for(auto offset:{0x24u,0x20u,0x28u})words_.set(offset,std::uint32_t{0});
        words_.set(0x1c,kind);words_.set(0x2c,std::uint8_t{0});
        if(kind==0)words_.set(0x54,0.f);
        if(kind==2)words_.set(0x30,std::uint8_t{0});
    }
    std::uint32_t kind()const{return kind_;}
    bool expired()const{return words_.get<std::int32_t>(0x20)==-1;} //0073F7B0.
    bool elapsed()const{return words_.get<float>(0x28)>1.f;} //0073F7C0.
    auto& words(){return words_;}const auto& words()const{return words_;}
    HumanInputHistory::Profiles primary{},secondary{};
private:
    std::uint32_t kind_;HumanJointWords<0x70> words_;
};

//The actual three callbacks installed by0056F200: factory64, read54 and
//write58. The second pair reads/writes B0+54 through its owner5C; these are
//not history coordinates, and the factory does not use owner60.
struct HumanInputCallbacks {
    template<class Bits>
    std::unique_ptr<HumanInputMessage> first(Bits& bits,bool write) const { //0053B760.
        //Original log(3)/log(2)+1→ftol gives two bits (008108A8=3).
        if(write){bits.write(3,2);return nullptr;}
        const auto kind=bits.read(2);
        return kind<3?std::make_unique<HumanInputMessage>(kind):nullptr;
    }
    template<class Bits,class State>
    void second(Bits& bits,State* owner) const { //0053B970; null owner still consumes nine bits.
        const auto value=bits.read(9);
        if(owner)owner->words().set(0x54,value);
    }
    template<class Bits,class State>
    void auxiliary(Bits& bits,const State& owner) const { //0053B990.
        bits.write(owner.words().template get<std::uint32_t>(0x54),9);
    }
};
} //namespace hd2::game
#endif
