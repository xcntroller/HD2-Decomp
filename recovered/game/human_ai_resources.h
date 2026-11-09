#ifndef HD2_GAME_HUMAN_AI_RESOURCES_H
#define HD2_GAME_HUMAN_AI_RESOURCES_H

#include "game/human_ai_direction.h"
#include "game/human_ai_action_pool.h"
#include "game/mission_tables.h"

namespace hd2::game {

//005607D0/00560810, separate50-byte state at AI+128. Its00560800
//destructor is RET. Native30 is a typed actor borrower, never a scalar DWORD.
template<class Actor>struct HumanAITargetLoading {
    HumanAITargetLoading() {
        state.set(0x34,std::uint8_t{0});state.set(0x35,std::uint8_t{0});
        for(auto offset:{0u,4u,8u,0x10u,0x14u,0x18u,0x38u})state.set(offset,std::uint32_t{0});
        for(auto offset:{0x3cu,0x40u,0x44u,0x48u})state.set(offset,std::int32_t{-1});
    }
    HumanJointWords<0x50> state;
    Actor* actor=nullptr;
    void bind_actor(Actor* value) { //005607B0, resets both three-component accumulators.
        actor=value;
        for(auto offset:{0x10u,0x14u,0x18u,0u,4u,8u})state.set(offset,std::uint32_t{0});
    }
};

//The inline60-byte allocation in0056EE90 at AI+12C. Its runtime graph/query
//producers remain unexposed; unwritten bytes are not fabricated as recovered.
struct HumanAIQueryLoading {
    HumanAIQueryLoading() {
        state.set(0x38,std::uint32_t{6});state.set(0x3c,std::uint32_t{40});
        for(auto offset:{0x50u,0x54u,0x4cu,0x28u,0x40u,0x24u})state.set(offset,std::uint32_t{0});
        state.set(0x48,std::uint16_t{0xffff});state.set(0x4a,std::uint16_t{0xffff});
        state.set(0x58,std::uint8_t{0});state.set(0x30,std::int32_t{-1});state.set(0x2c,std::uint32_t{0x7f7fffff});
    }
    HumanJointWords<0x60> state;
    void* actor=nullptr; //Native44 actor identity, shared with actual navigation references.
};

//00563A60/00563A70 at AI+134; matching destructor is RET.
struct HumanAIFlagLoading {
    HumanAIFlagLoading(){state.set(0,std::uint32_t{0});state.set(4,std::uint8_t{0});}
    HumanJointWords<8> state;
};

//0055DF90/0055F2C0/0055E130/0055E030. Borrow the actual SECOND MissionTableSet
//(original game+54 dataset+3C), not a private graph or empty successful path.
//No path/query-node producer is exposed here; both real owning buffers start empty.
class HumanAIPathLoading {
public:
    explicit HumanAIPathLoading(MissionTableSet& second):table_(second) {
        state_.set(0x40,std::uint32_t{0});state_.set(0x74,std::uint32_t{0});
        state_.set(0x88,std::uint8_t{0});state_.set(0x8c,-1.f);state_.set(0x90,std::uint32_t{0x2000});
        reset();
    }
    ~HumanAIPathLoading(){while(!nodes_.empty())nodes_.pop_back();route_.clear();} //007E513C reverse nodes, then route.
    HumanAIPathLoading(const HumanAIPathLoading&)=delete;HumanAIPathLoading& operator=(const HumanAIPathLoading&)=delete;
    void reset() { //0055F2C0 clears route end, not its allocation/capacity.
        route_.clear();state_.set(0x10,std::uint16_t{0});state_.set(0x14,std::uint32_t{0});state_.set(0x18,std::uint32_t{0x4cbebc20});
        for(auto offset:{0x20u,0x24u,0x30u,0x34u,0x38u})state_.set(offset,std::uint32_t{0});
        state_.set(0x1d,std::uint8_t{0});state_.set(0x1c,std::uint8_t{0});
    }
    MissionTableSet& table()const{return table_;}const auto& state()const{return state_;}
    void stop_loading() { //0055E250/0055E2B0(4); native74 vehicle has no producer here.
        if(state_.get<std::uint32_t>(0x74))throw std::runtime_error("Human AI path stop requires actual vehicle path ownership");
        state_.set(0x40,state_.get<std::uint32_t>(0x40)|4u);
    }
private:
    MissionTableSet& table_;HumanJointWords<0xa0> state_;
    std::vector<std::uint8_t> route_; //Uninterpreted native04 buffer; no element format or producer inferred.
    std::vector<std::unique_ptr<unsigned char[]>> nodes_; //Native7C owns raw node allocations; producer remains required.
};

//Owned allocation and cold-field portion of0056EE90/0056F0E0. The containing
//B0 derived state still supplies actual base bind, input callbacks, jobs and
//0056F200/0056F630 receivers. This is NOT a successful AI activation service.
template<class Actor>class HumanAIResources {
public:
    using Target=HumanAITargetLoading<Actor>;
    HumanAIResources(std::uint32_t kind,MissionTableSet& second) {
        state_.set(0xf4,static_cast<std::uint8_t>(kind));
        //00576AC0 receiver is embedded at C0.
        state_.set(0xc0,std::uint8_t{0});
        for(auto offset:{0xc4u,0xd8u,0xccu,0xc8u,0xd0u,0xd4u,0x108u,0x10cu,0x110u,0xf0u,0x104u})
            state_.set(offset,std::uint32_t{0});
        state_.set(0x114,std::int32_t{-1});state_.set(0x118,std::int32_t{-1});
        try {
            direction_=std::make_unique<HumanAIDirection>();target_=std::make_unique<Target>();
            flag_=std::make_unique<HumanAIFlagLoading>();query_=std::make_unique<HumanAIQueryLoading>();
            path_=std::make_unique<HumanAIPathLoading>(second);
        }catch(...){clear();throw;}
    }
    ~HumanAIResources(){clear();}
    HumanAIResources(const HumanAIResources&)=delete;HumanAIResources& operator=(const HumanAIResources&)=delete;
    void clear()noexcept { //0056F0E0:124,128,134,130,12C, then F8 vector; base belongs to containing state.
        current_action_.reset(); //Typed nativeF0; explicit state close normally returns it first.
        direction_.reset();target_.reset();flag_.reset();path_.reset();query_.reset();jobs_.clear();
    }
    HumanAIDirection& direction(){return required(direction_);}Target& target(){return required(target_);}
    const HumanAIDirection& direction()const{return required(direction_);}
    HumanAIFlagLoading& flag(){return required(flag_);}HumanAIQueryLoading& query(){return required(query_);}
    HumanAIPathLoading& path(){return required(path_);}auto& state(){return state_;}const auto& state()const{return state_;}
    HumanAIAction1* current_action()const{return current_action_.get();}
    void current_action(HumanAIAction1Pool::Lease value){current_action_=std::move(value);}
    void retire_current_action(){current_action_.reset();} //00575820→00582C50 case1.
private:
    template<class T>static T& required(const std::unique_ptr<T>& owner) {
        if(!owner)throw std::logic_error("Human AI resource has already been retired");
        return *owner;
    }
    HumanJointWords<0x140> state_;
    std::unique_ptr<HumanAIDirection> direction_;std::unique_ptr<Target> target_;
    std::unique_ptr<HumanAIFlagLoading> flag_;std::unique_ptr<HumanAIQueryLoading> query_;std::unique_ptr<HumanAIPathLoading> path_;
    struct Job;std::vector<Job*> jobs_; //NativeF8; no unreconstructed pool/job producer is exposed.
    HumanAIAction1Pool::Lease current_action_{nullptr,HumanAIAction1Pool::Return{}};
};
} //namespace hd2::game
#endif
