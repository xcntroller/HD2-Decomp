#ifndef HD2_FRONTEND_CONTROL_CONNECTION_STREAM_H
#define HD2_FRONTEND_CONTROL_CONNECTION_STREAM_H
#include "game/human_input_callbacks.h"
#include <limits>
namespace hd2::frontend {
//Raw0073CEC0/0073CF50/0073D000 storage only. The worker's process9830E8
//scratch is NOT an additional Human input/quantizer owner. Reuse the actual bit
//kernel; retain unwritten scratch bits between messages, with unclaimed initial
//allocation padding zeroed by the portable source storage.
class ControlConnectionStream {
    struct Storage{
        std::vector<std::uint32_t> words;std::uint32_t capacity_bits=0;
        void reserve_bits(std::size_t word,unsigned bit,unsigned count){
            if(word>(std::numeric_limits<std::uint32_t>::max()-bit-count)/32u)throw std::length_error("Control bit cursor overflow");
            const auto required=static_cast<std::uint32_t>(word*32u+bit+count);
            if(capacity_bits<=required){
                if(words.empty())capacity_bits=count<801?800:count+32;
                else{const auto growth=capacity_bits<count?count+32:capacity_bits;
                    if(growth>std::numeric_limits<std::uint32_t>::max()-capacity_bits)throw std::length_error("Control bit capacity overflow");
                    capacity_bits+=growth;}
                words.resize((static_cast<std::size_t>(capacity_bits)+31)/32);
            }
        }
        auto& word(std::size_t n){return words.at(n);}std::size_t word_count()const{return words.size();}
    } storage_;
public:
    ControlConnectionStream():bits_(storage_){}
    ControlConnectionStream(const ControlConnectionStream&)=delete;
    void position(std::size_t word,unsigned bit){bits_.position(word,bit);}
    std::size_t bit_count()const{return bits_.word_position()*32u+bits_.bit_position();}
    void write(std::uint32_t value,unsigned count){if(received_)throw std::logic_error("Write to received control packet");bits_.write(value,count);}
    std::uint32_t read(unsigned count){if(!received_||count>*received_||bit_count()>*received_-count)throw std::runtime_error("Truncated control packet");return bits_.read(count);}
    void write_bytes(const std::vector<unsigned char>& bytes){ //00745B40's byte copy, then bit cursor advance.
        if(received_)throw std::logic_error("Write to received control packet");
        const auto start=bit_count();
        for(std::size_t n=0;n<bytes.size();++n){
            const auto at=start/8+n;const auto shift=static_cast<unsigned>(at%4)*8;
            storage_.reserve_bits(at/4,shift,8);auto& word=storage_.word(at/4);
            word=(word&~(0xffu<<shift))|(std::uint32_t{bytes[n]}<<shift);
        }
        const auto end=start+bytes.size()*8;position(end/32,static_cast<unsigned>(end%32));
    }
    std::vector<unsigned char> read_bytes(std::size_t count){ //00745BF0 allocates BEFORE checked native copy.
        std::vector<unsigned char> bytes(count);const auto start=bit_count();
        if(!received_||start>*received_||count>(*received_-start)/8)throw std::runtime_error("Truncated message payload");
        for(std::size_t n=0;n<count;++n){const auto at=start/8+n;bytes[n]=static_cast<unsigned char>(storage_.word(at/4)>>((at%4)*8));}
        const auto end=start+count*8;position(end/32,static_cast<unsigned>(end%32));return bytes;
    }
    void receive(const std::vector<unsigned char>& bytes){
        if(bytes.size()>std::numeric_limits<std::uint32_t>::max()/8u)throw std::length_error("Control packet size overflow");
        storage_.words.assign((bytes.size()+3)/4,0);
        for(std::size_t n=0;n<bytes.size();++n)storage_.words[n/4]|=std::uint32_t{bytes[n]}<<((n%4)*8);
        received_=storage_.capacity_bits=static_cast<std::uint32_t>(bytes.size()*8);position(0,0);
    }
    std::vector<unsigned char> written_bytes()const{
        std::vector<unsigned char> bytes((bit_count()+7)/8);
        for(std::size_t n=0;n<bytes.size();++n)bytes[n]=static_cast<unsigned char>(storage_.words.at(n/4)>>((n%4)*8));
        return bytes;
    }
private:
    game::HumanInputBits<Storage> bits_;std::optional<std::size_t> received_;
};
} //namespace hd2::frontend
#endif
