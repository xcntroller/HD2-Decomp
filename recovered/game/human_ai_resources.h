#ifndef HD2_GAME_HUMAN_AI_RESOURCES_H
#define HD2_GAME_HUMAN_AI_RESOURCES_H

#include "game/human_ai_direction.h"
#include "game/human_ai_action_pool.h"
#include "game/human_ai_navigation_jobs.h"
#include "game/mission_tables.h"
#include "game/human_state_frame_timers.h"
#include <variant>

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

//The inline60-byte allocation in0056EE90 at AI+12C. Requests and path
//publication share this owner with the original typed actor and Door borrowers.
struct HumanAIQueryLoading {
    HumanAIQueryLoading() {
        state.set(0x38,std::uint32_t{6});state.set(0x3c,std::uint32_t{40});
        for(auto offset:{0x50u,0x54u,0x4cu,0x28u,0x24u})state.set(offset,std::uint32_t{0});
        state.set(0x48,std::uint16_t{0xffff});state.set(0x4a,std::uint16_t{0xffff});
        state.set(0x58,std::uint8_t{0});state.set(0x30,std::int32_t{-1});state.set(0x2c,std::uint32_t{0x7f7fffff});
    }
    HumanJointWords<0x60> state;
    void* actor=nullptr; //Native44 actor identity, shared with actual navigation references.
    void* door=nullptr; //Native40, populated by the actual path job worker.
    void request_position(const HumanVector& position){ //00560EA0.
        if(position[1]<-100000.f||position[1]>100000.f)return; //Both native bounds tests accept unordered Y.
        state.set(0x10,position);
        state.set(0x20,std::uint16_t{0xffff});
        state.set(0x50,(state.get<std::uint32_t>(0x50)&0x7ff7fffcu)|1u);
        state.set(0x58,std::uint8_t{0});
        state.set(0x24,0u);
    }
    void request_point(std::uint16_t index){ //00560F00.
        if(index==0xffff)return;
        state.set(0x20,index);
        state.set(0x50,(state.get<std::uint32_t>(0x50)&0x7ff7fffcu)|1u);
        state.set(0x58,std::uint8_t{0});
        state.set(0x24,0u);
    }
    void request_mode(std::uint32_t mode,std::uint32_t range){ //00560E20.
        state.set(0x38,mode);state.set(0x3c,range);state.set(0x20,std::uint16_t{0xffff});
        state.set(0x50,(state.get<std::uint32_t>(0x50)&0x7ff7fff8u)|5u);
        state.set(0x58,std::uint8_t{0});state.set(0x24,0u);
    }
    void finish(){ //00560F30.
        door=nullptr;
        state.set(0x50,(state.get<std::uint32_t>(0x50)&0xcfff57ffu)|0x80000000u);
    }
    void begin_path(){ //00560F50.
        state.set(0x2c,0x7f7fffffu);
        door=nullptr;
        state.set(0x28,0u);
        state.set(0x34,0u);
        state.set(0x50,(state.get<std::uint32_t>(0x50)&0x6fffeff8u)|0x10001000u);
    }
};

//00563A60/00563A70 at AI+134; matching destructor is RET.
struct HumanAIFlagLoading {
    HumanAIFlagLoading(){state.set(0,std::uint32_t{0});state.set(4,std::uint8_t{0});}
    HumanJointWords<8> state;
    void request(std::uint32_t mode){ //00563A20.
        if(state.get<std::uint32_t>(0)==mode)return;
        state.set(0,mode);
        state.set(4,std::uint8_t{1});
    }
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
    using Frame=decltype(std::declval<Actor&>().base().frame());
    using Direction=HumanAIDirection<Frame>;
    HumanAIResources(std::uint32_t kind,MissionTableSet& second) {
        state_.set(0xf4,static_cast<std::uint8_t>(kind));
        //00576AC0 receiver is embedded at C0.
        state_.set(0xc0,std::uint8_t{0});
        for(auto offset:{0xc4u,0xd8u,0xccu,0xc8u,0x108u,0x10cu,0x110u,0xf0u})
            state_.set(offset,std::uint32_t{0});
        state_.set(0x114,std::int32_t{-1});state_.set(0x118,std::int32_t{-1});
        try {
            direction_=std::make_unique<Direction>();target_=std::make_unique<Target>();
            flag_=std::make_unique<HumanAIFlagLoading>();query_=std::make_unique<HumanAIQueryLoading>();
            path_=std::make_unique<HumanAIPathLoading>(second);
        }catch(...){clear();throw;}
    }
    ~HumanAIResources(){clear();}
    HumanAIResources(const HumanAIResources&)=delete;HumanAIResources& operator=(const HumanAIResources&)=delete;
    void clear()noexcept { //0056F0E0:124,128,134,130,12C, then F8 vector; base belongs to containing state.
        retire_current_action(); //Typed nativeF0; explicit state close normally returns it first.
        direction_.reset();target_.reset();flag_.reset();path_.reset();query_.reset();finish_navigation_jobs();
    }
    std::uint32_t current_action_mode()const{
        return std::visit([](const auto& lease)->std::uint32_t{using T=std::decay_t<decltype(lease)>;if constexpr(std::is_same_v<T,std::monostate>)return 0;else return lease?lease->state.template get<std::uint32_t>(8):0;},current_action_);
    }
    Direction& direction(){return required(direction_);}Target& target(){return required(target_);}
    const Direction& direction()const{return required(direction_);}
    HumanAIFlagLoading& flag(){return required(flag_);}HumanAIQueryLoading& query(){return required(query_);}
    const HumanAIQueryLoading& query()const{return required(query_);}
    HumanAIPathLoading& path(){return required(path_);}auto& state(){return state_;}const auto& state()const{return state_;}
    Actor* aimed_actor()const{return aimed_actor_;}
    Actor* selected_actor()const{return selected_actor_;}
    void publish_aimed_actor(Actor* actor){ //00576350 clears D0 for a vector while D4 retains its previous selection.
        aimed_actor_=actor;
        if(actor)selected_actor_=actor;
    }
    auto& navigation_jobs(){return jobs_;}
    const HumanAINavigationJob* current_navigation_job()const{return current_job_.get();}
    void advance_navigation_job(){
        current_job_=std::move(jobs_.front());
        jobs_.erase(jobs_.begin());
    }
    void release_queued_navigation_jobs()noexcept{
        while(!jobs_.empty())jobs_.pop_back();
    }
    void finish_navigation_jobs()noexcept{ //0057F6F0/0056F630 reverse F8, then104.
        release_queued_navigation_jobs();
        current_job_.reset();
    }
    void current_action(HumanAIAction1Pool::Lease value){current_action_=std::move(value);}
    void current_action(HumanAIAction2Pool::Lease value){current_action_=std::move(value);}
    void current_action(HumanAIAction3Pool::Lease value){current_action_=std::move(value);}
    void current_action(HumanAIAction5Pool::Lease value){current_action_=std::move(value);}
    void current_action(HumanAIAction6Pool::Lease value){current_action_=std::move(value);}
    template<class Pool>auto* current_action_as()const{
        const auto* lease=std::get_if<typename Pool::Lease>(&current_action_);
        return lease?lease->get():nullptr;
    }
    void retire_current_action()noexcept{current_action_.template emplace<std::monostate>();} //00575820→00582C50.
    std::uint32_t current_action_kind()const{
        if(state_.template get<std::uint32_t>(0xf0))throw std::runtime_error("AI action ageing requires actual typed pooled action owner");
        return std::visit([](const auto& lease)->std::uint32_t{
            if constexpr(std::is_same_v<std::decay_t<decltype(lease)>,std::monostate>)return 0;
            else return lease?lease->state.template get<std::uint32_t>(4):0u;
        },current_action_);
    }
    void advance_current_action(std::uint32_t elapsed){ //00547E70/00559C60.
        if(state_.template get<std::uint32_t>(0xf0))throw std::runtime_error("AI action ageing requires actual typed pooled action owner");
        std::visit([elapsed](auto& lease){
            if constexpr(!std::is_same_v<std::decay_t<decltype(lease)>,std::monostate>){
                if(!lease)return;
                auto& state=lease->state;
                const auto remaining=state.template get<std::uint32_t>(0x10)-elapsed;
                state.set(0xc,state.template get<std::uint32_t>(0xc)+elapsed);
                state.set(0x10,human_frame_signed_word(remaining)<0?0u:remaining);
            }
        },current_action_);
    }
    std::uint32_t reconcile_direction_flags(){ //00575850, actual134 flag/12C query/124 direction owners.
        auto& flag=required(flag_).state;if(!flag.template get<std::uint8_t>(4))return 1;
        const auto kind=flag.template get<std::uint32_t>(0);
        auto& direction=required(direction_);
        if(kind==1){flag.set(4,std::uint8_t{0});direction.unlock_current();direction.lock_target();return 2;}
        if(required(query_).state.template get<std::uint32_t>(0x50)&0xf30u)return 0;
        flag.set(4,std::uint8_t{0});direction.unlock_target();
        if(kind==2||kind==4)direction.lock_current();else direction.unlock_current();
        return 2;
    }
private:
    template<class T>static T& required(const std::unique_ptr<T>& owner) {
        if(!owner)throw std::logic_error("Human AI resource has already been retired");
        return *owner;
    }
    HumanJointWords<0x140> state_;
    std::unique_ptr<Direction> direction_;std::unique_ptr<Target> target_;
    Actor* aimed_actor_=nullptr;
    Actor* selected_actor_=nullptr;
    std::unique_ptr<HumanAIFlagLoading> flag_;std::unique_ptr<HumanAIQueryLoading> query_;std::unique_ptr<HumanAIPathLoading> path_;
    std::vector<HumanAINavigationJobPool::Lease> jobs_; //NativeF8.
    HumanAINavigationJobPool::Lease current_job_{nullptr,HumanAINavigationJobPool::Return{}}; //Native104.
    std::variant<std::monostate,HumanAIAction1Pool::Lease,HumanAIAction2Pool::Lease,
        HumanAIAction3Pool::Lease,HumanAIAction5Pool::Lease,HumanAIAction6Pool::Lease> current_action_;
};
} //namespace hd2::game
#endif
