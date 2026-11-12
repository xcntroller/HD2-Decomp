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
    void write(std::uint32_t value,unsigned count){
        if(received_bits_)throw std::logic_error("Write to received session packet");
        bits_.write(value,count);
    }
    std::uint32_t read(unsigned count){
        if(received_bits_&&(count>*received_bits_||bit_count()>*received_bits_-count))
            throw std::runtime_error("Truncated received session packet");
        return bits_.read(count);
    }
    void position(std::size_t word,unsigned bit){bits_.position(word,bit);}
    std::size_t bit_count()const{return bits_.word_position()*32u+bits_.bit_position();}
    const auto& words()const{return storage_.words;}
    std::uint32_t capacity_bits()const{return storage_.capacity_bits;}
    void receive(const std::vector<unsigned char>& bytes) { //0072C3B0's temporary0073D370 receiver, native borrowed payload.
        if(bytes.size()>std::numeric_limits<std::uint32_t>::max()/8u)throw std::length_error("Received packet exceeds native bit length");
        storage_.words.assign((bytes.size()+3u)/4u,0);
        for(std::size_t n=0;n<bytes.size();++n)storage_.words[n/4]|=std::uint32_t{bytes[n]}<<((n%4)*8u);
        storage_.capacity_bits=static_cast<std::uint32_t>(bytes.size()*8u);received_bits_=storage_.capacity_bits;
        bits_.position(0,0); //Typed copy keeps the queued payload's lifetime explicit; no raw borrowed DWORD ABI.
    }
    std::vector<unsigned char> written_bytes()const {
        std::vector<unsigned char> bytes((bit_count()+7u)/8u);
        for(std::size_t n=0;n<bytes.size();++n)bytes[n]=static_cast<unsigned char>(storage_.words.at(n/4)>>((n%4)*8u));
        return bytes;
    }
    std::size_t received_bit_count()const {
        if(!received_bits_)throw std::logic_error("Session stream is not a received packet");
        return *received_bits_;
    }
    void write_string(const std::string& value) { //0073D070, signed CHAR promotion is deliberately unmasked.
        for(const auto c:value){if(!c){write(0,8);return;}write(static_cast<std::uint32_t>(static_cast<std::int8_t>(c)),8);}
        write(0,8);
    }
    std::string read_string(std::size_t limit) { //0073D0A0: consume at most limit bytes, force the final destination byte NUL.
        if(!limit)throw std::out_of_range("Zero native packet string capacity");
        std::string result;
        for(std::size_t n=0;n<limit;++n){const auto c=read(8);if(!c)return result;result.push_back(static_cast<char>(c));}
        result.resize(limit-1);return result;
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
    void write_yaw(const HumanVector& value,std::int32_t index){ //0073D760, atan2(Z,X), secondary quantizer, mask1 ONLY.
        const HumanVector angle{{static_cast<float>(std::atan2(static_cast<long double>(value[2]),static_cast<long double>(value[0]))),0,0,0}};
        (void)write_vector(angle,secondary_quantizer(index),1);
    }
    HumanVector read_vector(const Quantizer* quantizer,std::uint8_t mask) { //0073D530, signed FILD and strict dead-zone comparison.
        if(!quantizer)throw std::runtime_error("Received vector has no actual quantizer");
        const auto& q=*quantizer;HumanVector value{};
        const auto component=[&](unsigned axis){
            const auto word=read(q.bits[axis]);std::int32_t signed_value;std::memcpy(&signed_value,&word,4);
            return static_cast<long double>(signed_value)/q.scale[axis]+q.minimum[axis];
        };
        if(mask&1u)value[0]=static_cast<float>(component(0));
        if((mask&2u)&&read(1))value[1]=static_cast<float>(component(1));
        //Original retains extended Z after its FLOAT store for the final comparison.
        const auto z=(mask&4u)?component(2):0.L;value[2]=static_cast<float>(z);
        for(unsigned n=0;n!=2;++n)if(std::abs(static_cast<long double>(value[n]))<q.step[n])value[n]=0;
        if(std::abs(z)<q.step[2])value[2]=0;
        return value;
    }
    HumanVector read_direction(std::int32_t index) { //0073D710, pitch/yaw FLOAT values followed by x87 sin/cos.
        const auto angles=read_vector(secondary_quantizer(index),3);
        const auto pitch=static_cast<long double>(angles[1]),yaw=static_cast<long double>(angles[0]);
        const auto c=std::cos(pitch);
        return {{static_cast<float>(std::cos(yaw)*c),static_cast<float>(std::sin(pitch)),static_cast<float>(std::sin(yaw)*c),0}};
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
    HumanInputShared& shared_;Storage storage_;HumanInputBits<Storage> bits_;std::optional<std::size_t> received_bits_;
};
} //namespace hd2::game
#endif
