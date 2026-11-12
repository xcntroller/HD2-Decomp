#ifndef HD2_GAME_HUMAN_INPUT_RECORDS_H
#define HD2_GAME_HUMAN_INPUT_RECORDS_H

#include "game/human_joint_owners.h"
#include <functional>
#include <list>
#include <optional>

namespace hd2::game {

//Process-shared loading state00982DF8/00982E00/00982F90. Runtime interpolation
//and packet producers are not exposed by this loading owner.
struct HumanInputShared {
    struct Quantizer {
        std::array<float,3> minimum{},range{},step{},scale{};
        std::array<std::uint32_t,3> bits{};std::uint32_t word70=100;
    };
    std::uint64_t frequency=0;bool quantizers_initialized=false;float interpolation_step=0;
    bool relative_positions=false; //00982F94; actual process flag, initially zero.
    std::array<Quantizer,3> quantizers{};
    //00982DE8 is a separate process table from the angle-history profiles
    //00982E00 above.0073CBD0 allocates ten slots and0073CCF0 seeds only0..3;
    //reading another unproduced slot must not invent a zero profile.
    std::vector<std::optional<Quantizer>> packet_quantizers;
    void initialize_packet_quantizers() {
        if(!packet_quantizers.empty())return;
        packet_quantizers.resize(10);
        constexpr std::array<std::uint32_t,4> minima{{0xc4bb8000,0xc04947ae,0xc0000000,0xc04947ae}};
        constexpr std::array<std::uint32_t,4> ranges{{0x453b8000,0x40c947ae,0x40000000,0x40c947ae}};
        constexpr std::array<std::uint32_t,4> steps{{0x3c23d70a,0x3ccccccd,0x3ca3d70a,0x3e4ccccd}};
        for(unsigned n=0;n!=4;++n)packet_quantizers[n]=make_quantizer(
            uniform(minima[n]),uniform(ranges[n]),uniform(steps[n]));
    }
    void packet_quantizer(std::int32_t index,const Quantizer& value) { //0073CC40, one doubling, then copy.
        initialize_packet_quantizers();
        if(index<0)throw std::out_of_range("Negative process packet quantizer");
        if(static_cast<std::size_t>(index)>=packet_quantizers.size()) {
            if(static_cast<std::size_t>(index)>=packet_quantizers.size()*2)
                throw std::out_of_range("Packet quantizer exceeds native single growth");
            packet_quantizers.resize(packet_quantizers.size()*2);
        }
        packet_quantizers[static_cast<std::size_t>(index)]=value;
    }
    const Quantizer* packet_quantizer(std::int32_t index)const {
        if(index<0||static_cast<std::size_t>(index)>=packet_quantizers.size())return nullptr;
        const auto& value=packet_quantizers[static_cast<std::size_t>(index)];
        if(!value)throw std::runtime_error("Packet quantizer slot has no original producer");
        return &*value;
    }
    void initialize_quantizers() { //00740410→0073D7E0, actual three uniform vectors.
        if(quantizers_initialized)return;
        constexpr std::array<std::uint32_t,3> minima{{0xc0490fdb,0xbe20d97c,0xbda0d97c}};
        constexpr std::array<std::uint32_t,3> ranges{{0x40c90fdb,0x3ea0d97c,0x3e20d97c}};
        for(unsigned n=0;n!=3;++n) {
            const auto minimum=real(minima[n]),range=real(ranges[n]),step=real(0x3c47e282);
            quantizers[n]=make_quantizer({{minimum,minimum,minimum}},{{range,range,range}},{{step,step,step}});
        }
        quantizers_initialized=true;
    }
    static Quantizer make_quantizer(const std::array<float,3>& minimum,const std::array<float,3>& range,
                                     const std::array<float,3>& step) { //0073D7E0, including nonuniform step vectors.
        Quantizer result;result.minimum=minimum;result.range=range;result.step=step;
        const double denominator=static_cast<double>(std::log(2.L));
        for(unsigned axis=0;axis!=3;++axis) {
            if(!std::isfinite(minimum[axis])||!std::isfinite(range[axis])||!std::isfinite(step[axis])||range[axis]<=0||step[axis]<=0)
                throw std::runtime_error("Invalid Human input quantizer range");
            const double quotient=static_cast<double>(std::log(static_cast<long double>(range[axis])/step[axis])/denominator);
            const auto count=std::floor(quotient)+1;
            if(count<0||count>32)throw std::runtime_error("Human input quantizer exceeds original mask table");
            const auto bits=static_cast<std::uint32_t>(count);result.bits[axis]=bits;
            const auto mask=bits==32?0xffffffffu:(std::uint32_t{1}<<bits)-1u;
            result.scale[axis]=static_cast<float>(static_cast<long double>(mask)/range[axis]);
        }
        return result;
    }
private:
    static std::array<float,3> uniform(std::uint32_t bits){const auto value=real(bits);return {{value,value,value}};}
    static float real(std::uint32_t bits){float result;std::memcpy(&result,&bits,4);return result;}
};

class HumanInputHistory {
public:
    using Vector=std::array<float,3>;
    using Profiles=std::array<const HumanInputShared::Quantizer*,3>;
    struct Value { //0073F810/0073FA80 and angle00740410/007406F0.
        explicit Value(HumanInputShared& shared,bool angles):angles(angles),shared_(shared) {
            for(auto offset:{0x20u,0x24u,0x28u,
                0x30u,0x34u,0x38u,0x40u,0x44u,0x48u,0x50u,0x54u,0x58u,
                0x60u,0x64u,0x68u,0x70u,0x74u,0x78u})words.set(offset,std::uint32_t{0});
            words.set(0x80,40.f);words.set(0x84,.035f);words.set(0x88,std::uint32_t{7});words.set(0x8c,3.f);
            shared.interpolation_step=.1f;
            if(angles) {
                words.set(0x88,std::uint32_t{3});
                for(auto offset:{0x90u,0x94u,0x98u,0xa0u,0xa4u,0xa8u})words.set(offset,std::uint32_t{0});
                shared.initialize_quantizers();
                for(unsigned n=0;n!=3;++n)primary[n]=secondary[n]=&shared.quantizers[n];
            }
        }
        void bind_primary(const Profiles& value){if(!angles)for(unsigned n=0;n!=3;++n)if(value[n])primary[n]=value[n];} //004C1360; angle007405D0 is RET.
        void bind_secondary(const Profiles& value){if(!angles)for(unsigned n=0;n!=3;++n)if(value[n])secondary[n]=value[n];} //004C1390; angle007405E0 is RET.
        void flags(std::uint32_t flags){words.set(0x88,angles?flags&0xfffc3fffu:flags);} //0073F840/007410F0.
        void interval(float interval){words.set(0x84,interval);} //0073F850.
        void seed(const HumanVector& value) { //0073F860, actual position-value virtual+1C.
            if(shared_.relative_positions) {
                for(auto offset:{0x40u,0x44u,0x48u})words.set(offset,0.f); //Preserve unwritten fourth lane.
            }else words.set(0x40,value);
            words.set(0x20,value);words.set(0x30,value);
        }
        const bool angles;HumanJointWords<0xb0> words;
        Profiles primary{},secondary{}; //Native04/08/0C and10/14/18 borrowed tables.
    private:
        HumanInputShared& shared_;
    };
    explicit HumanInputHistory(HumanInputShared& shared,std::uint32_t id)
        :id(id),position_value(shared,false),angle_value(shared,true) {} //0073DA50, rings180/380 reset32×3 words.
    void bind_primary(const Profiles& value){primary_=value;position_value.bind_primary(value);angle_value.bind_primary(value);} //0073DC60.
    void bind_secondary(const Profiles& value){secondary_=value;position_value.bind_secondary(value);angle_value.bind_secondary(value);} //0073DCA0.
    void interval(float value){position_value.interval(value);angle_value.interval(value);} //0073DCE0.
    void flags(unsigned group,std::uint32_t value) { //0073DD40, native six-bit mask, original unmasked value.
        const auto shift=(group*6u)&31u;
        flags_=(flags_&~(0x3fu<<shift))|(value<<shift);
        position_value.flags(flags_);angle_value.flags(flags_);
    }
    const auto& primary_profiles()const{return primary_;}const auto& secondary_profiles()const{return secondary_;}
    std::uint32_t flags()const{return flags_;}
    const std::uint32_t id;Value position_value,angle_value;
    std::array<Vector,32> position_ring{},angle_ring{}; //Unwritten fourth lanes omitted.
private:
    Profiles primary_{},secondary_{};std::uint32_t flags_=199;
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

//0073DDC0/0073DF70 and0073F2B0/0073F5F0/0073F530 ordinary loading ownership.
//The containing manager supplies actual defaults/receivers and real QPC clock.
//No runtime packet/interpolation command or network transport is supplied here.
template<class Clock,class Receiver>
class HumanInputRecords {
public:
    using Profiles=HumanInputHistory::Profiles;
    struct Defaults {
        Profiles primary{},secondary{};float interval=0;std::uint32_t field2=0;
        std::function<Receiver*(std::int32_t)> first_receiver,second_receiver; //Native E4/E0 receive record ID in ECX.
    };
    struct Record {
        Record(Clock& clock,HumanInputShared& shared,std::int32_t id)
            :id(id) {
            if(!shared.frequency) {
                shared.frequency=clock.performance_frequency();
                if(!shared.frequency)throw std::runtime_error("Human input clock has no frequency");
            }
            timestamp=clock.performance_counter();
            children.push_back(std::make_unique<HumanInputHistory>(shared,1)); //0073E090(1), actual child owner.
        }
        ~Record(){while(!children.empty())children.pop_back();second_pending_.clear();first_pending_.clear();}
        Record(const Record&)=delete;Record& operator=(const Record&)=delete;
        void bind_primary(const Profiles& value){primary_=value;for(auto& child:children)child->bind_primary(value);} //0073E920.
        void bind_secondary(const Profiles& value){secondary_=value;for(auto& child:children)child->bind_secondary(value);} //0073E8D0.
        HumanInputHistory* history(std::uint32_t id)const { //0073E2C0 first forward child ID match.
            for(const auto& child:children)if(child->id==id)return child.get();
            return nullptr;
        }
        HumanInputMessage* adopt_pending(std::unique_ptr<HumanInputMessage> message){
            if(!message)throw std::invalid_argument("Human input pending requires its actual message");
            if(!next_pending_identity_)throw std::length_error("Human input pending lifetime identity exhausted");
            auto* result=message.get();pending_owners_.push_back({std::move(message),next_pending_identity_++});return result;
        }
        void append_pending(HumanInputMessage& message,std::uint32_t stamp,bool second){ //0073E5A0: payload stores BEFORE list allocation.
            auto& owner=live_pending(&message);message.words().set(0x20,stamp);
            message.words().set(0x24,message.words().template get<std::uint32_t>(0x24)+1u);
            (second?second_pending_:first_pending_).push_back({&message,owner.identity});
        }
        HumanInputMessage* take_pending(bool second){ //0073E600 decrements before unlink and returns the SAME payload.
            auto& pending=second?second_pending_:first_pending_;if(pending.empty())return nullptr;
            const auto node=pending.front();auto& message=checked_pending(node);
            message.words().set(0x24,message.words().template get<std::uint32_t>(0x24)-1u);
            pending.pop_front();return &message;
        }
        void retire_pending_message(HumanInputMessage* message){ //Actual three Human message virtual10 deleting bodies have no resource tail.
            for(auto it=pending_owners_.begin();it!=pending_owners_.end();++it)if(it->message.get()==message){
                it->message.reset();pending_owners_.erase(it);return; //Payload BEFORE caller's pending-node unlink.
            }
            throw std::logic_error("Human input pending retirement requires its actual live message");
        }
        void reset_pending(bool decrement,bool second,bool force){ //0073E660/0073E6E0; retained node repeats while its count remains positive.
            auto& pending=second?second_pending_:first_pending_;
            for(auto it=pending.begin();it!=pending.end();){
                auto& message=checked_pending(*it);
                if(decrement)message.words().set(0x24,message.words().template get<std::uint32_t>(0x24)-1u);
                if(force||message.words().template get<std::int32_t>(0x24)<1){
                    retire_pending_message(&message);it=pending.erase(it);
                }else if(!decrement)throw std::logic_error("Human input pending reset would repeat an unchanged native node");
            }
        }
        std::size_t pending_count(bool second)const{return (second?second_pending_:first_pending_).size();}
        const auto& primary_profiles()const{return primary_;}const auto& secondary_profiles()const{return secondary_;}
        std::int32_t id; //Native1C; containing manager assigns its embedded record's player ID after construction.
        std::uint64_t timestamp=0;
        std::uint32_t word30=1,word34=0x40000000,word38=0,word3c=7,word50=0;
        bool flag40=true;float elapsed44=0;
        Receiver* first_receiver=nullptr;Receiver* second_receiver=nullptr;Receiver* auxiliary_receiver=nullptr;
        void* first_owner=nullptr;void* second_owner=nullptr; //Native60/5C; native58 is the auxiliary receiver.
        std::vector<std::unique_ptr<HumanInputHistory>> children;
    private:
        struct Pending {HumanInputMessage* message;std::uint64_t identity;};
        struct OwnedPending {std::unique_ptr<HumanInputMessage> message;std::uint64_t identity;};
        std::list<Pending> first_pending_,second_pending_; //Native6C/78 borrowed payload nodes.
        std::list<OwnedPending> pending_owners_;std::uint64_t next_pending_identity_=1; //Typed allocation ownership, not another native packet queue.
        OwnedPending& live_pending(HumanInputMessage* message){
            for(auto& owner:pending_owners_)if(owner.message.get()==message)return owner;
            throw std::logic_error("Human input pending payload has no actual live owner");
        }
        HumanInputMessage& checked_pending(const Pending& node){
            auto& owner=live_pending(node.message);
            if(owner.identity!=node.identity)throw std::logic_error("Human input pending node refers to a retired/reused payload");
            return *owner.message;
        }
        Profiles primary_{},secondary_{};
    };
    HumanInputRecords(Clock& clock,HumanInputShared& shared,Defaults& defaults)
        :clock_(clock),shared_(shared),defaults_(defaults) {}
    ~HumanInputRecords(){clear();}
    HumanInputRecords(const HumanInputRecords&)=delete;HumanInputRecords& operator=(const HumanInputRecords&)=delete;
    Record* find(std::int32_t id,bool create) { //0073F5F0, first forward ID match.
        for(auto& record:records_)if(record->id==id)return record.get();
        return create?make(id):nullptr;
    }
    Record* make(std::int32_t id) { //0073F2B0 may append duplicate IDs; find() does not.
        auto record=std::make_unique<Record>(clock_,shared_,id);
        apply_defaults(*record);auto* result=record.get();records_.push_back(std::move(record));return result;
    }
    void apply_defaults(Record& record) { //0073F4B0, preserve call order.
        if(!defaults_.first_receiver||!defaults_.second_receiver)throw std::runtime_error("Human input manager has no actual default receivers");
        for(auto& child:record.children)child->interval(defaults_.interval); //0073E9B0.
        record.bind_primary(defaults_.primary);record.bind_secondary(defaults_.secondary);
        for(auto& child:record.children)child->flags(2,defaults_.field2); //0073E970.
        record.first_receiver=defaults_.first_receiver(record.id);record.first_owner=nullptr;
        record.second_receiver=defaults_.second_receiver(record.id);record.auxiliary_receiver=nullptr;record.second_owner=nullptr;
    }
    void remove(Record* record) { //0073F530, order-preserving erase before destruction.
        for(std::size_t index=0;index<records_.size();++index)if(records_[index].get()==record) {
            auto removed=std::move(records_[index]);records_.erase(records_.begin()+static_cast<std::ptrdiff_t>(index));
            removed.reset();
        }
    }
    void clear()noexcept{while(!records_.empty())records_.pop_back();}
    bool contains(const Record* record)const noexcept {
        for(const auto& owner:records_)if(owner.get()==record)return true;
        return false;
    }
    const auto& records()const{return records_;}
private:
    Clock& clock_;HumanInputShared& shared_;Defaults& defaults_;
    std::vector<std::unique_ptr<Record>> records_;
};
} //namespace hd2::game
#endif
