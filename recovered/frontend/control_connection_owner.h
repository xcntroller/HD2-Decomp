#ifndef HD2_FRONTEND_CONTROL_CONNECTION_OWNER_H
#define HD2_FRONTEND_CONTROL_CONNECTION_OWNER_H

#include "frontend/control_connection_records.h"
#include "frontend/control_connection_stream.h"
#include <new>

namespace hd2::frontend {

// Native lock storage is an OS endpoint. Its lifetime follows the source owner;
// protocol processing must use the SAME lock rather than a new per-call lock.
struct ControlConnectionLock {
    virtual ~ControlConnectionLock()=default;
    virtual void enter()=0;
    virtual void leave() noexcept=0;
};

//00744ED0/00745350/007458D0 cold queue storage. No send, acknowledge, retry,
//decode or packet-pool receiver is supplied here. There is deliberately no
//synthetic producer for populated queues. Sentinel allocations are real owners,
//including empty lists; their allocation failures cannot become empty success.
class ControlConnectionQueue {
public:
    enum class Kind{unreliable,reliable,ordered};
    explicit ControlConnectionQueue(Kind kind):kind_(kind){
        head10_=make_head();head1c_=make_head(); //00744ED0, two12-byte native sentinels.
        if(kind_!=Kind::unreliable){
            head34_=make_head();head40_=make_head(); //00745350,20 then12 bytes.
            sequence24_=sequence28_=0x3fffffu;timeout2c_=200;
        }
        if(kind_==Kind::ordered)sequence48_=0x3fffffu;
    }
    ~ControlConnectionQueue(){
        //00745410→00744FC0. All four lists are necessarily empty until their
        //actual packet producers are reconstructed. No release receiver is
        //invented for a queued packet; this is solely the cold owner boundary.
        head40_.reset();head34_.reset();head1c_.reset();head10_.reset();
    }
    ControlConnectionQueue(const ControlConnectionQueue&)=delete;
    ControlConnectionQueue& operator=(const ControlConnectionQueue&)=delete;
    Kind kind()const{return kind_;}
    std::uint32_t sequence04()const{return sequence04_;}
    std::uint32_t sequence08()const{return sequence08_;}
    std::optional<std::uint32_t> sequence24()const{return sequence24_;}
    std::optional<std::uint32_t> sequence28()const{return sequence28_;}
    std::optional<std::uint32_t> sequence48()const{return sequence48_;}
    std::optional<std::uint32_t> timeout2c()const{return timeout2c_;}
    unsigned sentinel_count()const{return kind_==Kind::unreliable?2u:4u;}
    void maintain_cold(ControlConnectionPump& pump){if(kind_!=Kind::unreliable)(void)pump.connection_clock();} //007456D0 empty pending lists, including fresh clock.
    void write_ack(ControlConnectionStream& stream)const{ //00745650; queue0 virtual14 is native no-op.
        if(kind_!=Kind::unreliable){stream.write(sequence24_.value()&0x7fffffu,23);stream.write(0,1);}
    }
    void read_ack(ControlConnectionStream& stream){ //00745680→007450C0.
        if(kind_==Kind::unreliable)return;
        auto value=stream.read(23);(void)stream.read(1);const auto prior=sequence28_.value();
        constexpr std::uint32_t mask=0x7fffff,q=mask>>2;value&=mask;
        if(q*3<(prior&mask)&&value<q)value=((prior+mask)&~mask)+value;
        else if((prior&mask)<q&&q*3<value)value=((prior-mask)&~mask)+value;
        else value=(prior&~mask)+value;
        const auto signed_word=[](std::uint32_t v){std::int32_t r;std::memcpy(&r,&v,4);return r;};
        if(signed_word(value)>signed_word(prior))sequence28_=value;
    }
    bool incoming_empty()const{ //00745190/00745920 cold branch; no synthetic message producer.
        if(head1c_->next!=head1c_.get())throw std::logic_error("Received queue requires its actual retained message owner");
        return true;
    }
    void complete_cold(){ //007457C0; base queue0 virtual20 is native no-op.
        if(kind_!=Kind::unreliable&&head40_->next!=head40_.get())
            throw std::logic_error("Completion queue requires its actual message retirement");
    }
private:
    struct Head{Head* next=this;Head* previous=this;}; //Sentinel payload/allocator bytes are unproduced and never read.
    static std::unique_ptr<Head> make_head(){return std::make_unique<Head>();}
    Kind kind_;
    std::unique_ptr<Head> head10_,head1c_,head34_,head40_;
    std::uint32_t sequence04_=0x3fffffu,sequence08_=0x3fffffu;
    std::optional<std::uint32_t> sequence24_,sequence28_,sequence48_,timeout2c_;
};

//00745D20's actual scalar/queue/clock/critical-section construction and
//00745E40's cold retirement. Layout, allocator/SEH ABI and populated protocol
//queues are separate. The containing caller still owns publication/deletion;
//the worker registry borrows this SAME object's consumed +08 base.
template<class Platform>
class ControlConnectionOwner final:public ControlConnectionRecord {
public:
    ControlConnectionOwner(Platform& platform,std::uint32_t identity,std::uint32_t address,void* context)
        :identity04_(identity),address0c_(address),context38_(context){
        generation(0); //BEFORE both fresh00748960 reads and queue acquisition.
        clock44_=platform.control_milliseconds();
        clock3c_=clock40_=platform.control_milliseconds();
        queue14_.reset(new(std::nothrow) ControlConnectionQueue(ControlConnectionQueue::Kind::unreliable));
        queue18_.reset(new(std::nothrow) ControlConnectionQueue(ControlConnectionQueue::Kind::reliable));
        queue1c_.reset(new(std::nothrow) ControlConnectionQueue(ControlConnectionQueue::Kind::ordered));
        lock20_=platform.create_connection_lock(); //After all three queues, including NULL queue allocations.
        if(!lock20_)throw std::logic_error("Connection critical section has no actual owner");
    }
    ~ControlConnectionOwner()override{ //00745E40: DeleteCriticalSection BEFORE ascending queue deletions.
        invalidate_lifetime();
        lock20_.reset();queue14_.reset();queue18_.reset();queue1c_.reset();
    }
    ControlConnectionOwner(const ControlConnectionOwner&)=delete;
    ControlConnectionOwner& operator=(const ControlConnectionOwner&)=delete;
    std::uint32_t identity()const override{return identity04_.load();}
    void identity(std::uint32_t value)override{identity04_=value;} //00722180 CONNECT_COMPLETE, lockless native04 publication.
    std::uint32_t address()const{return address0c_;}
    std::uint32_t port()const{return port10_;}
    void port(std::uint32_t value){port10_=value;} //00723D20 writes this AFTER process334 publication.
    void* context()const{return context38_;}
    std::uint32_t clock3c()const{return clock3c_;}
    std::uint32_t clock40()const{return clock40_;}
    std::uint32_t clock44()const{return clock44_;}
    std::uint32_t word48()const{return word48_;}
    std::uint32_t word4c()const{return word4c_;}
    ControlConnectionQueue* queue(unsigned offset)const{
        switch(offset){case 0x14:return queue14_.get();case 0x18:return queue18_.get();case 0x1c:return queue1c_.get();
        default:throw std::out_of_range("Original connection queue offset");}
    }
    void maintain(ControlConnectionPump& pump)override{ //007460F0→00746000, signed wrapping deadline.
        const auto now=pump.connection_clock();
        if(signed_word(clock44_+2000u)>=signed_word(now))return;
        word48_=static_cast<std::uint32_t>((static_cast<std::int64_t>(signed_word(word48_))+1)%256);
        const auto header_identity=identity04_.load(),sequence=word48_; //Native ping copies BOTH before20 enter.
        {Guard lock(*lock20_);auto& stream=pump.connection_scratch();stream.position(0,0);
            stream.write(2,8);stream.write(header_identity,8);stream.write(0,1);stream.write(sequence,8);
            stream.write(pump.connection_clock(),32);pump.connection_send(stream.written_bytes(),address0c_,port10_,context38_);}
        clock44_=now;
    }
    void flush(ControlConnectionPump& pump,bool force)override{ //00746120→007465F0→00746490.
        clock3c_=pump.connection_clock();
        if(port10_!=0xffffffffu){Guard lock(*lock20_);for(unsigned at:{0x14u,0x18u,0x1cu})required_queue(at).maintain_cold(pump);}
        {auto head=std::make_unique<EmptyContainerHead>(); //007470A0's real empty sentinel, BEFORE20 enter.
            Guard lock(*lock20_);
            if(port10_!=0xffffffffu)for(unsigned at:{0x14u,0x18u,0x1cu})(void)required_queue(at);
            if(!force)return;
            auto& stream=pump.connection_scratch();stream.position(0,0);const auto header_identity=identity04_.load();
            stream.write(1,8);stream.write(header_identity,8);
            for(unsigned at:{0x14u,0x18u,0x1cu})required_queue(at).write_ack(stream);
            stream.write(0,8);}
        //00746490 has unlocked20 and retired its container before00746120 sends.
        pump.connection_send(pump.connection_scratch().written_bytes(),address0c_,port10_,context38_);
    }
    void receive(ControlConnectionPump& pump,std::uint32_t kind,ControlConnectionStream& stream,
            std::uint32_t address,std::uint32_t port)override{ //00746A10 endpoint publication then00746170.
        port10_=port;address0c_=address;clock40_=pump.connection_clock();
        if(kind==1){
            {Guard lock(*lock20_);
                for(unsigned at:{0x14u,0x18u,0x1cu})required_queue(at).read_ack(stream);
                if(stream.read(8))throw std::logic_error("Received payload requires actual00747240 message owners");}
            if(pump.connection_callback_bound()){ //SAME process414 == workerD8, read AFTER ACK receive.
                {Guard lock(*lock20_);for(unsigned at:{0x14u,0x18u,0x1cu})(void)required_queue(at).incoming_empty();} //00745F20→00745FB0.
                {Guard lock(*lock20_);for(unsigned at:{0x14u,0x18u,0x1cu})required_queue(at).complete_cold();} //00746630.
            }
        }else if(kind==2){
            const auto reply=stream.read(1),sequence=stream.read(8);auto stamp=stream.read(32);
            if(reply)stamp=pump.connection_clock()-stamp;
            if(!stamp)stamp=1; //007473F0 clamps both request timestamp and reply elapsed zero.
            if(!reply){const auto header_identity=identity04_.load();Guard lock(*lock20_);auto& out=pump.connection_scratch();out.position(0,0);
                out.write(2,8);out.write(header_identity,8);out.write(1,1);out.write(sequence,8);out.write(stamp,32);
                pump.connection_send(out.written_bytes(),address0c_,port10_,context38_);
            }else if(sequence==word48_)word4c_=stamp;
        }
    }
private:
    struct EmptyContainerHead{EmptyContainerHead* next=this;EmptyContainerHead* previous=this;};
    static std::int32_t signed_word(std::uint32_t v){std::int32_t r;std::memcpy(&r,&v,4);return r;}
    struct Guard{ControlConnectionLock& lock;explicit Guard(ControlConnectionLock& value):lock(value){lock.enter();}~Guard(){lock.leave();}};
    ControlConnectionQueue& required_queue(unsigned at){auto* value=queue(at);if(!value)throw std::logic_error("Connection pump reached its native NULL queue dereference");return *value;}
    std::atomic<std::uint32_t> identity04_;
    std::uint32_t address0c_,port10_=0xffffffffu;
    void* context38_;
    std::uint32_t clock3c_=0,clock40_=0,clock44_=0;
    std::uint32_t word48_=0,word4c_=0;
    std::unique_ptr<ControlConnectionQueue> queue14_,queue18_,queue1c_;
    std::unique_ptr<ControlConnectionLock> lock20_;
};

} //namespace hd2::frontend
#endif
