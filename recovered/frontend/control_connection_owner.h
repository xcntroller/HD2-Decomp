#ifndef HD2_FRONTEND_CONTROL_CONNECTION_OWNER_H
#define HD2_FRONTEND_CONTROL_CONNECTION_OWNER_H

#include "frontend/control_connection_records.h"
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
        lock20_.reset();queue14_.reset();queue18_.reset();queue1c_.reset();
    }
    ControlConnectionOwner(const ControlConnectionOwner&)=delete;
    ControlConnectionOwner& operator=(const ControlConnectionOwner&)=delete;
    std::uint32_t identity()const{return identity04_;}
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
private:
    std::uint32_t identity04_,address0c_,port10_=0xffffffffu;
    void* context38_;
    std::uint32_t clock3c_=0,clock40_=0,clock44_=0;
    std::uint32_t word48_=0,word4c_=0;
    std::unique_ptr<ControlConnectionQueue> queue14_,queue18_,queue1c_;
    std::unique_ptr<ControlConnectionLock> lock20_;
};

} //namespace hd2::frontend
#endif
