#ifndef HD2_GAME_HUMAN_ACTIVE_AI_ORDERS_H
#define HD2_GAME_HUMAN_ACTIVE_AI_ORDERS_H
#include "game/human_ai_order_pool.h"
#include "game/human_state_frame_timers.h"

namespace hd2::game {
//0058D720 accepts unordered vector lengths as well as >=0080F948. It updates
//the stored order, while0052EE60 separately publishes the REQUEST direction.
template<class Order,class Delta>void merge_human_ai_order(Order& order,const Delta& delta){
    for(auto offset:{0x20u,0x30u}){
        const auto value=delta.state.template get<HumanVector>(offset);
        const auto square=(static_cast<long double>(value[0])*value[0]+static_cast<long double>(value[1])*value[1])+
            static_cast<long double>(value[2])*value[2];
        if(!(square<1.0000000168623835e-16f))order.state.set(offset,value);
    }
    const auto flags=delta.state.template get<std::uint32_t>(0x44);if(flags)order.state.set(0x44,flags);
    if(!order.marker)throw std::logic_error("AI order update requires actual marker84");
    order.marker->update();
}
//Actual Active250 sequence. Slots preserve their borrowed native pointer during
//pool/engine cleanup callbacks, before stable erase; only the lease is retired.
//retained_first_ represents the stale first vector slot after native erase,
//whose scalar order remains allocated by the process pool (00531F60).
template<class Pool>class HumanActiveAIOrders {
public:
    using Order=typename Pool::Order;using Lease=typename Pool::Lease;
    struct Slot {
        Lease owner;Order* borrower;
        explicit Slot(Lease value):owner(std::move(value)),borrower(owner.get()){}
        Slot(Slot&&)=default;Slot& operator=(Slot&&)=default;
        Order* get()const{return borrower;}
    };
    ~HumanActiveAIOrders(){while(!records_.empty())records_.pop_back();} //Defined rollback guard; normal0052EF20 retires with callbacks first.
    auto& records(){return records_;}const auto& records()const{return records_;}
    Order& retained_first()const{ //005282E0 rereads begin[0] after00529F60 can reset the list.
        if(!retained_first_)throw std::logic_error("Active pending processor requires its native retained first slot");
        return *retained_first_;
    }
    Order& at(std::size_t index)const{auto* value=records_.at(index).get();if(!value)throw std::logic_error("Active order slot lost its borrower");return *value;}
    template<class State,class World>bool append(State& ai,Lease value,World& world){return insert(ai,std::move(value),0,true,world);}
    template<class State,class World>bool insert_after(State& ai,Lease value,std::int32_t after,World& world){return insert(ai,std::move(value),after,false,world);}
    template<class State,class Delta,class World>void update(State& ai,std::size_t index,const Delta& delta,World& world){ //0052EE60.
        merge_human_ai_order(at(index),delta);
        if(!index){
            ai.state().set(0x18e,std::uint8_t{1});
            if(delta.state.template get<std::uint32_t>(0xc)==2){
                const auto direction=delta.state.template get<HumanVector>(0x20);
                ai.state().set(0x160,direction);ai.words().set(0x10,direction);ai.words().set(0x20,direction);
                (void)ai.resources().direction().set_target(ai.words().template get<HumanVector>(0x10));
                (void)ai.resources().direction().set_current(ai.words().template get<HumanVector>(0x10));
            }
        }
        serial(ai,world);
    }
    template<class State,class World>void retire(State& ai,std::size_t index,bool append_hold,World& world){ //0052EFA0.
        serial(ai,world);auto& order=at(index);
        if(!index){
            auto& query=ai.resources().query().state;query.set(0x50,query.template get<std::uint32_t>(0x50)|0x2000u);
            ai.resources().path().stop_loading();ai.state().set(0x202,std::uint8_t{1});
            ai.state().set(0x174,ai.state().template get<std::uint32_t>(0x174)|1u);ai.state().set(0x206,std::uint8_t{1});
            if(order.state.template get<std::uint32_t>(0x10)==32)world.restore_active_order_formation(ai);
            if(order.state.template get<std::uint32_t>(0xc)==4)append_hold=false;
        }
        records_[index].owner.reset(); //00583980/00582640 BEFORE stable pointer erase.
        records_.erase(records_.begin()+static_cast<std::ptrdiff_t>(index));refresh_first();
        if(records_.empty()&&append_hold){
            const auto direction=ai.current_direction();ai.state().set(0x160,direction);
            auto* human=ai.human();if(!human)throw std::logic_error("Active hold order requires actual Human44");
            const auto matrix=world.human_command_frames().actor_world_matrix(human->base().frame());
            hold(ai,{{matrix[12],matrix[13],matrix[14],matrix[15]}},ai.state().template get<HumanVector>(0x160),world);
        }
    }
    template<class State,class World>void reset(State& ai,bool append_hold,World& world){ //0052EF20.
        for(auto n=records_.size();n;)retire(ai,--n,append_hold,world);
        serial(ai,world);auto& query=ai.resources().query().state;query.set(0x50,query.template get<std::uint32_t>(0x50)|0x2000u);
        ai.resources().path().stop_loading();ai.state().set(0x202,std::uint8_t{1});ai.state().set(0x206,std::uint8_t{1});ai.state().set(0x18d,std::uint8_t{0});
    }
    template<class State,class World>void hold(State& ai,const HumanVector& position,const HumanVector& direction,World& world){ //00531F60.
        //Native dereferences begin[0] even after an erase. Its process record
        //is still allocated. A cold vector has no defined first slot to read.
        if(!retained_first_)throw std::logic_error("Active hold order requires its native retained first slot");
        auto& first=retained_first_->state;auto flags=first.template get<std::uint32_t>(0x44);
        for(const auto pair:{std::pair<std::uint32_t,std::uint32_t>{0x700u,0x200u},{7u,4u},{0x1f000u,0x4000u}})
            if(!(flags&pair.first)){flags|=pair.second;first.set(0x44,flags);}
        auto* human=ai.human();if(!human)throw std::logic_error("Active hold order requires actual Human44");
        auto value=world.human_ai_order_pool().acquire(world,human,2,first.template get<std::int32_t>(0x10),
            position,direction,flags&0x1f000u,flags&7u,flags&0x700u,nullptr,0);
        records_.emplace_back(std::move(value));refresh_first();
    }
    template<class State,class World>bool recently_changed(const State& ai,World& world)const{ //0052F110, unsigned comparison.
        return world.game_frame_serial()<=ai.state().template get<std::uint32_t>(0x1bc);
    }
private:
    template<class State,class World>static void serial(State& ai,World& world){ai.state().set(0x1bc,world.game_frame_serial()+1u);}
    void refresh_first(){if(!records_.empty())retained_first_=records_.front().get();} //Empty erase does not clear old pointer storage.
    template<class State,class World>bool insert(State& ai,Lease value,std::int32_t after,bool append,World& world){ //0052ED00/0052EB20, absent frozen starts.
        if(!value)throw std::logic_error("Active order insertion requires actual process lease");
        if(records_.size()==10||!world.human_ai_order_navigation(*value)){
            value.reset();world.queue_active_ai_speech(ai,0x68u,1,0,1);return false;
        }
        if(!records_.empty()){
            const auto& last=at(records_.size()-1);const auto kind=last.state.template get<std::uint32_t>(0xc);
            if(kind==2||last.state.template get<std::uint32_t>(0x10)==32){
                if(kind!=2)world.restore_active_order_formation(ai);
                records_.back().owner.reset();records_.pop_back();refresh_first();
                if(!append){const auto bits=static_cast<std::uint32_t>(after)-1u;after=human_frame_signed_word(bits);}
            }
        }
        const auto where=append?static_cast<std::int64_t>(records_.size()):static_cast<std::int64_t>(after)+1;
        if(where<0||static_cast<std::uint64_t>(where)>records_.size())throw std::out_of_range("Active order insertion exceeds native defined index");
        records_.emplace(records_.begin()+static_cast<std::ptrdiff_t>(where),std::move(value));refresh_first();serial(ai,world);return true;
    }
    std::vector<Slot> records_;Order* retained_first_=nullptr;
};
}
#endif
