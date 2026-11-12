#ifndef HD2_FRONTEND_CONTROL_CONNECTION_QUEUE_H
#define HD2_FRONTEND_CONTROL_CONNECTION_QUEUE_H

#include "frontend/control_connection_message.h"
#include "frontend/control_connection_records.h"
#include <algorithm>

namespace hd2::frontend {
//00744ED0/00745350/007458D0, one actual queue owner for outgoing10, incoming1C,
//reliable pending34 and completion40. Queue2 additionally orders takes by48.
class ControlConnectionQueue {
    using Message=ControlConnectionMessage;using Pointer=Message::Pointer;
    struct Pending{std::uint32_t deadline,interval;Pointer message;};
public:
    enum class Kind{unreliable,reliable,ordered};
    explicit ControlConnectionQueue(Kind kind):kind_(kind){
        outgoing_=std::make_unique<ControlConnectionMessages>();incoming_=std::make_unique<ControlConnectionMessages>();
        if(kind_!=Kind::unreliable){
            pending_=std::make_unique<ControlConnectionList<Pending>>();
            completions_=std::make_unique<ControlConnectionMessages>();
            sequence24_=sequence28_=0x3fffffu;timeout2c_=200;
        }
        if(kind_==Kind::ordered)sequence48_=0x3fffffu;
    }
    ~ControlConnectionQueue(){ //00745410→00744FC0, distinguish node release from head retirement.
        if(pending_)pending_->clear_back(); //Native releases pending records from tail BEFORE40 head retirement.
        if(completions_){completions_->clear_front();completions_.reset();}
        pending_.reset(); //Empty34 head AFTER40.
        outgoing_->clear_back();incoming_->clear_front();incoming_.reset();outgoing_.reset(); //1C head BEFORE10.
        //Native completion/incoming teardown frees nodes without virtual08;
        //typed owners additionally reclaim those undelivered record/payload leaks.
    }
    ControlConnectionQueue(const ControlConnectionQueue&)=delete;
    ControlConnectionQueue& operator=(const ControlConnectionQueue&)=delete;
    Kind kind()const{return kind_;}
    std::uint32_t sequence04()const{return sequence04_;}std::uint32_t sequence08()const{return sequence08_;}
    auto sequence24()const{return sequence24_;}auto sequence28()const{return sequence28_;}
    auto sequence48()const{return sequence48_;}auto timeout2c()const{return timeout2c_;}
    unsigned sentinel_count()const{return kind_==Kind::unreliable?2u:4u;}
    auto outgoing_count()const{return outgoing_->size();}auto incoming_count()const{return incoming_->size();}
    auto pending_count()const{return pending_?pending_->size():0;}auto completion_count()const{return completions_?completions_->size():0;}
    void enqueue(Pointer message,ControlConnectionPump& pump){ //00745130, then00745520 for queues1/2.
        message->sequence=++sequence04_;outgoing_->push_back(message);
        if(kind_==Kind::unreliable)return;
        const auto interval=timeout2c_.value(),now=pump.connection_clock();
        pending_->push_back(Pending{now+interval,interval,std::move(message)});
    }
    void maintain(ControlConnectionPump& pump){ //007456D0; queue0 virtual1C is native no-op.
        if(kind_==Kind::unreliable)return;
        while(!pending_->empty()){
            if(control_queue_signed(pending_->begin()->message->required_sequence())>control_queue_signed(sequence28_.value()))break;
            pending_->discard(pending_->begin()); //Release message BEFORE freeing pending node.
        }
        const auto now=pump.connection_clock();unsigned remaining=7;
        for(auto at=pending_->begin();at!=pending_->end()&&remaining;++at){
            if(control_queue_signed(now)<=control_queue_signed(at->deadline))continue;
            at->deadline=now+at->interval;at->interval=std::min(at->interval*2u,5000u);
            outgoing_->push_back(at->message);--remaining;
        }
    }
    Pointer take_outgoing(){return outgoing_->empty()?Pointer{}:outgoing_->take(outgoing_->begin());} //007451D0.
    void receive(Pointer message){ //007455A0→00745210, or base00745210 directly.
        if(kind_!=Kind::unreliable){
            const auto lifted=lift_control_sequence(message->required_sequence(),sequence08_);
            if(control_queue_signed(lifted)<=control_queue_signed(sequence24_.value()))return;
            message->sequence=lifted;
            for(auto at=completions_->begin();at!=completions_->end();++at)
                if((*at)->required_sequence()==lifted)return;
            completions_->push_back(message); //BEFORE base incoming append/duplicate bookkeeping.
        }
        const auto lifted=lift_control_sequence(message->required_sequence(),sequence08_);
        if(control_queue_signed(sequence08_)<control_queue_signed(lifted)||control_queue_signed(sequence08_-lifted)>9)
            sequence08_=lifted;
        else message->flags|=4;
        incoming_->push_back(std::move(message));
    }
    Pointer take_incoming(){ //00745190 or ordered00745920.
        if(kind_!=Kind::ordered)return incoming_->empty()?Pointer{}:incoming_->take(incoming_->begin());
        const auto next=sequence48_.value()+1u;
        for(auto at=incoming_->begin();at!=incoming_->end();++at){
            if((*at)->required_sequence()!=next)continue;
            sequence48_=next;auto result=incoming_->take(at);result->flags&=~4u;return result;
        }
        return {};
    }
    bool incoming_empty()const{return incoming_->empty();}
    void complete(){ //007457C0; AFTER callback delivery, separate20 scope.
        if(kind_==Kind::unreliable)return;
        for(;;){
            const auto next=sequence24_.value()+1u;auto at=completions_->begin();
            while(at!=completions_->end()&&(*at)->required_sequence()!=next)++at;
            if(at==completions_->end())return;
            sequence24_=next;auto value=completions_->take(at);value.reset();
        }
    }
    void write_ack(ControlConnectionStream& stream)const{ //00745650.
        if(kind_!=Kind::unreliable){stream.write(sequence24_.value()&0x7fffffu,23);stream.write(0,1);}
    }
    void read_ack(ControlConnectionStream& stream){ //00745680→007450C0.
        if(kind_==Kind::unreliable)return;
        const auto value=stream.read(23);(void)stream.read(1);
        const auto lifted=lift_control_sequence(value,sequence28_.value());
        if(control_queue_signed(lifted)>control_queue_signed(sequence28_.value()))sequence28_=lifted;
    }
private:
    Kind kind_;
    std::unique_ptr<ControlConnectionMessages> outgoing_,incoming_;
    std::unique_ptr<ControlConnectionList<Pending>> pending_;
    std::unique_ptr<ControlConnectionMessages> completions_;
    std::uint32_t sequence04_=0x3fffffu,sequence08_=0x3fffffu;
    std::optional<std::uint32_t> sequence24_,sequence28_,sequence48_,timeout2c_;
};
} //namespace hd2::frontend
#endif
