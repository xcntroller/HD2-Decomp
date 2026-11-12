#ifndef HD2_FRONTEND_CONTROL_CONNECTION_OWNER_H
#define HD2_FRONTEND_CONTROL_CONNECTION_OWNER_H

#include "frontend/control_connection_records.h"
#include "frontend/control_connection_queue.h"
#include <new>

namespace hd2::frontend {

// Native lock storage is an OS endpoint. Its lifetime follows the source owner;
// protocol processing must use the SAME lock rather than a new per-call lock.
struct ControlConnectionLock {
    virtual ~ControlConnectionLock()=default;
    virtual void enter()=0;
    virtual void leave() noexcept=0;
};

//00745D20's actual scalar/queue/clock/critical-section construction and
//00745E40's retirement. Layout and allocator/SEH ABI are separate.
//The containing caller still owns publication/deletion;
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
    std::weak_ptr<ControlConnectionMessage> submit(const std::vector<unsigned char>& bytes,
            std::uint32_t channel,std::uint32_t index,ControlConnectionPump& pump)override{ //00745EA0.
        auto message=std::make_shared<ControlConnectionMessage>();message->bytes=bytes;
        message->channel=channel;message->flags=index;std::weak_ptr<ControlConnectionMessage> borrowed=message;
        {Guard lock(*lock20_);required_queue_index(index).enqueue(std::move(message),pump);}
        return borrowed;
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
        if(port10_!=0xffffffffu){Guard lock(*lock20_);for(unsigned at:{0x14u,0x18u,0x1cu})required_queue(at).maintain(pump);}
        {ControlConnectionContainer container; //007470A0's real empty sentinel, BEFORE20 enter.
            Guard lock(*lock20_);
            if(port10_!=0xffffffffu)for(unsigned at:{0x14u,0x18u,0x1cu}){
                auto& queue=required_queue(at);
                while(auto message=queue.take_outgoing()){
                    message->generation=generation();container.push_back(std::move(message)); //007472C0.
                }
            }
            if(container.empty()&&!force)return;
            auto& stream=pump.connection_scratch();stream.position(0,0);const auto header_identity=identity04_.load();
            stream.write(1,8);stream.write(header_identity,8);
            for(unsigned at:{0x14u,0x18u,0x1cu})required_queue(at).write_ack(stream);
            stream.write(static_cast<std::uint32_t>(container.size()),8); //00747200, count truncation is native.
            for(auto at=container.begin();at!=container.end();++at)(*at)->write(stream);}
        //00746490 has unlocked20 and retired its container before00746120 sends.
        pump.connection_send(pump.connection_scratch().written_bytes(),address0c_,port10_,context38_);
    }
    void receive(ControlConnectionPump& pump,std::uint32_t kind,ControlConnectionStream& stream,
            std::uint32_t address,std::uint32_t port)override{ //00746A10 endpoint publication then00746170.
        port10_=port;address0c_=address;clock40_=pump.connection_clock();
        if(kind==1){
            {ControlConnectionContainer container;Guard lock(*lock20_);
                for(unsigned at:{0x14u,0x18u,0x1cu})required_queue(at).read_ack(stream);
                auto count=stream.read(8);
                while(count--){auto message=ControlConnectionMessage::read(stream);container.push_back(std::move(message));}
                //Decode the ENTIRE container before any queue admission. A late
                //malformed message keeps ACK mutations, without admitting earlier ones.
                while(!container.empty()){
                    auto message=container.take(container.begin());
                    if(message->generation==generation())required_queue_index(message->flags).receive(std::move(message));
                }
            }
            if(auto receiver=pump.connection_receiver()){ //Snapshot SAME414 once, AFTER ACK receive.
                for(;;){
                    ControlConnectionMessage::Pointer message;
                    {Guard lock(*lock20_);for(unsigned at:{0x14u,0x18u,0x1cu}){
                        message=required_queue(at).take_incoming();if(message)break;
                    }} //00745FB0 unlocks BEFORE payload extraction/record release and callback.
                    if(!message)break;
                    auto bytes=std::move(message->bytes);message->bytes.clear();const auto flags=message->flags,channel=message->channel;
                    message.reset(); //00745F20 clears1C/0C BEFORE virtual08; completion may retain record.
                    if((flags&3u)||!(flags&4u))receiver(bytes,channel,flags);
                }
                {Guard lock(*lock20_);for(unsigned at:{0x14u,0x18u,0x1cu})required_queue(at).complete();} //00746630.
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
    static std::int32_t signed_word(std::uint32_t v){std::int32_t r;std::memcpy(&r,&v,4);return r;}
    struct Guard{ControlConnectionLock& lock;explicit Guard(ControlConnectionLock& value):lock(value){lock.enter();}~Guard(){lock.leave();}};
    ControlConnectionQueue& required_queue(unsigned at){auto* value=queue(at);if(!value)throw std::logic_error("Connection pump reached its native NULL queue dereference");return *value;}
    ControlConnectionQueue& required_queue_index(std::uint32_t index){
        if(index>2)throw std::logic_error("Connection queue index reaches native20 lock storage instead of queue");
        return required_queue(0x14u+index*4u);
    }
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
