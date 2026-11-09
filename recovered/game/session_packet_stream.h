#ifndef HD2_GAME_SESSION_PACKET_STREAM_H
#define HD2_GAME_SESSION_PACKET_STREAM_H

#include "game/human_input_callbacks.h"
#include <limits>

namespace hd2::game {

//0073D280/0073D2F0/0073D370: actual growing bit-buffer owner. Native unused
//allocation/padding bytes are indeterminate; this source initializes that
//padding to zero and only gives meaning to the written bit count.
class SessionPacketStream {
    class Storage {
    public:
        void reserve_bits(std::size_t word,unsigned bit,unsigned count) {
            if(word>(std::numeric_limits<std::uint32_t>::max()-bit-count)/32u)
                throw std::length_error("Session bit buffer cursor exceeds native DWORD");
            const auto required=static_cast<std::uint32_t>(word*32u+bit+count);
            if(capacity_bits<=required) { //Native equality also grows.
                if(words.empty())capacity_bits=count<801u?800u:count+32u;
                else {
                    const auto growth=capacity_bits<count?count+32u:capacity_bits;
                    if(growth>std::numeric_limits<std::uint32_t>::max()-capacity_bits)
                        throw std::length_error("Session bit buffer capacity overflow");
                    capacity_bits+=growth;
                }
                words.resize((static_cast<std::size_t>(capacity_bits)+31u)/32u);
            }
        }
        std::size_t word_count()const{return words.size();}
        std::uint32_t& word(std::size_t n){return words.at(n);}
        std::vector<std::uint32_t> words;std::uint32_t capacity_bits=0;
    };
public:
    using Quantizer=HumanInputShared::Quantizer;
    explicit SessionPacketStream(HumanInputShared& shared):shared_(shared),bits_(storage_) {
        shared_.initialize_packet_quantizers();
    }
    SessionPacketStream(const SessionPacketStream&)=delete;
    SessionPacketStream& operator=(const SessionPacketStream&)=delete;
    void write(std::uint32_t value,unsigned count){bits_.write(value,count);}
    std::uint32_t read(unsigned count){return bits_.read(count);}
    void position(std::size_t word,unsigned bit){bits_.position(word,bit);}
    std::size_t bit_count()const{return bits_.word_position()*32u+bits_.bit_position();}
    const auto& words()const{return storage_.words;}
    std::uint32_t capacity_bits()const{return storage_.capacity_bits;}
    void write_string(const std::string& value) { //0073D070, signed CHAR promotion is deliberately unmasked.
        for(const auto c:value){if(!c){write(0,8);return;}write(static_cast<std::uint32_t>(static_cast<std::int8_t>(c)),8);}
        write(0,8);
    }
    const Quantizer* primary_quantizer(std::int32_t index)const{return select(index,primary,primary_default);} //0073D3A0.
    const Quantizer* secondary_quantizer(std::int32_t index)const{return select(index,secondary,secondary_default);} //0073D3F0.
    bool write_vector(const HumanVector& value,const Quantizer* quantizer,std::uint8_t mask) { //0073D440, x87 math.
        if(!quantizer)throw std::runtime_error("Session vector has no actual quantizer");
        const auto& q=*quantizer;
        if(mask&1u)write(quantize(value[0],q,0),q.bits[0]);
        if(mask&2u) {
            //Only Y is clamped, to the original duplicated minimum at+14.
            const auto y=std::max(value[1],q.minimum[1]);
            if(std::abs(static_cast<long double>(y))<=q.step[1])write(0,1);
            else {write(1,1);write(quantize(y,q,1),q.bits[1]);}
        }
        if(mask&4u)write(quantize(value[2],q,2),q.bits[2]);
        return std::abs(static_cast<long double>(value[0]))>q.step[0]||
            std::abs(static_cast<long double>(value[1]))>q.step[1]||
            std::abs(static_cast<long double>(value[2]))>q.step[2];
    }
    void write_direction(const HumanVector& value,std::int32_t index) { //0073D6C0, CRT007E6260 is asin, not acos.
        if(!std::isfinite(value[1])||std::abs(value[1])>1.f)
            throw std::runtime_error("Session direction exceeds native asin domain");
        HumanVector angles{{static_cast<float>(std::atan2(static_cast<long double>(value[2]),static_cast<long double>(value[0]))),
            static_cast<float>(std::asin(static_cast<long double>(value[1]))),0,0}};
        (void)write_vector(angles,secondary_quantizer(index),3);
    }
    //0072BEF0 selected-stream writes. The containing session chooses a stream
    //BEFORE this method. Messages append;0072BF90(target0) only clears active.
    void begin(std::uint32_t kind,std::int32_t priority,std::uint32_t target,bool local) {
        if(priority<0)throw std::out_of_range("Negative native packet priority");
        word20=static_cast<std::uint32_t>(std::min(priority,2));word1c=1;word24=target;flag18=!local;
        write(kind,5); //00816370 double20; trunc(log(20)/log(2)+1)=5.
        word28=kind;
    }
    void finish_broadcast() { //0072BF90 target0 branch; transport target branches require real receivers.
        word1c=0;
        if(word24)throw std::runtime_error("Session targeted packet requires its actual local/remote transport");
    }
    const Quantizer* primary=nullptr;const Quantizer* secondary=nullptr; //Native2C/30, borrowed overrides.
    std::int32_t primary_default=0,secondary_default=1; //Native34/38.
    std::uint32_t word1c=0,word20=0,word24=0,word28=0;bool flag18=false;
private:
    const Quantizer* select(std::int32_t index,const Quantizer* override_profile,std::int32_t fallback)const {
        if(const auto* value=shared_.packet_quantizer(index))return value;
        return override_profile?override_profile:shared_.packet_quantizer(fallback);
    }
    static std::uint32_t quantize(float value,const Quantizer& q,unsigned axis) {
        const auto number=(static_cast<long double>(value)-q.minimum[axis])*q.scale[axis];
        if(!std::isfinite(number)||number<-9223372036854775808.L||number>=9223372036854775808.L)
            throw std::runtime_error("Session quantized component exceeds native __ftol");
        return static_cast<std::uint32_t>(static_cast<std::int64_t>(std::trunc(number))); //Native keeps EAX, no clamp or mask.
    }
    HumanInputShared& shared_;Storage storage_;HumanInputBits<Storage> bits_;
};
} //namespace hd2::game
#endif
