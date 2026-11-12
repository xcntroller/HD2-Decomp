#ifndef HD2_FRONTEND_CONTROL_CONNECTION_SOCKET_H
#define HD2_FRONTEND_CONTROL_CONNECTION_SOCKET_H
#include "frontend/control_connection_owner.h"
#include <functional>
#include <list>
#include <optional>
#include <exception>
#include <cstring>
#include <algorithm>

namespace hd2::frontend {
struct ControlConnectionThread {
    virtual ~ControlConnectionThread()=default;
    virtual bool join(std::uint32_t milliseconds)=0;
    virtual std::exception_ptr failure()const=0;
};
struct ControlDatagram{std::vector<unsigned char> bytes;std::uint32_t address=0,port=0;};
inline std::int32_t connection_signed(std::uint32_t v){std::int32_t r;std::memcpy(&r,&v,4);return r;}
inline std::uint32_t connection_load32(const unsigned char* p){return std::uint32_t{p[0]}|(std::uint32_t{p[1]}<<8)|(std::uint32_t{p[2]}<<16)|(std::uint32_t{p[3]}<<24);}
inline void connection_store32(unsigned char* p,std::uint32_t v){for(unsigned n=0;n<4;++n)p[n]=static_cast<unsigned char>(v>>(n*8));}

//Actual singleton process+34C, NOT a generic freshly allocated socket's missing
//defaults. Untouched fields/events/counters are produced by PE loader zero-fill.
//The packet-pool allocator/layout is adapted to owned byte vectors.
template<class Platform>class ControlConnectionSocket {
public:
    using Event=std::uintptr_t;
    explicit ControlConnectionSocket(Platform& platform,ControlConnectionStorage storage):os_(platform){
        if(storage!=ControlConnectionStorage::loader_zero_filled_process)
            throw std::logic_error("Socket requires its demonstrated process storage producer");
        deferred_=std::make_unique<Queue>();immediate_=std::make_unique<Queue>(); //007474E0, heads40/4C.
        receive_buffer_.reset(new(std::nothrow) unsigned char[32768]);
        fragment_buffer_.reset(new(std::nothrow) unsigned char[800]);
        lock_=os_.create_connection_lock(); //54, AFTER both buffers.
        if(!lock_)throw std::logic_error("Socket critical section has no owner");
    }
    ~ControlConnectionSocket(){force_stop();close_storage();}
    ControlConnectionSocket(const ControlConnectionSocket&)=delete;
    ControlConnectionSocket& operator=(const ControlConnectionSocket&)=delete;
    void start(std::uint32_t port){ //00747690, RET4. Return values remain native OS results, not readiness.
        if(socket_!=invalid_socket)(void)stop();
        if(thread_)throw std::logic_error("Socket restart still owns a callback");
        os_.control_wsa_startup();wsa_=true;socket_=os_.control_udp_create();
        bound_address_=0;bound_port_=port==0xffffffffu?0u:static_cast<std::uint16_t>(port);
        os_.control_udp_bind(socket_,bound_port_);stop70_=false;
        thread_=os_.create_control_thread([this]{send_loop();},thread_id_); //BEFORE74/78 publication, as native.
        completion74_=os_.create_control_event();completion_open_=true;
        wake78_=os_.create_control_event();wake_open_=true;
    }
    std::exception_ptr stop(){ //00747750; thread-handle joining is additional source lifetime protection.
        if(socket_==invalid_socket){
            if(thread_)throw std::logic_error("Invalid socket leaves its native callback unretired");
            return {};
        }
        stop70_=true;os_.signal_control_event(wake78_);
        (void)os_.wait_control_event(completion74_,1000);
        if(thread_&&!thread_->join(1000))throw std::runtime_error("Socket callback has not retired; its owners remain live");
        auto failure=thread_?thread_->failure():std::exception_ptr{};
        os_.close_control_event(completion74_);completion_open_=false;
        os_.close_control_event(wake78_);wake_open_=false;
        os_.control_udp_close(socket_);socket_=invalid_socket;
        os_.control_wsa_cleanup();wsa_=false;thread_.reset();return failure;
    }
    //Separate portable exceptional fallback, not00747750 success or a native
    //1000ms completion claim. Never free callbacks' storage while they can run.
    void force_stop()noexcept{
        try{
            stop70_=true;
            if(thread_){os_.signal_control_event(wake78_);if(!thread_->join(0xffffffffu))std::terminate();thread_.reset();}
            if(completion_open_){os_.close_control_event(completion74_);completion_open_=false;}
            if(wake_open_){os_.close_control_event(wake78_);wake_open_=false;}
            if(socket_!=invalid_socket){os_.control_udp_close(socket_);socket_=invalid_socket;}
            if(wsa_){os_.control_wsa_cleanup();wsa_=false;}
        }catch(...){std::terminate();}
    }
    void close_storage(){ //007475B0:54, buffers14/18, list4C then40, vector30.
        if(closed_)return;
        if(thread_)throw std::logic_error("Socket storage still owns a callback");
        lock_.reset();receive_buffer_.reset();fragment_buffer_.reset();
        immediate_.reset();deferred_.reset();closed_=true;
    }
    void send(std::vector<unsigned char> payload,std::uint32_t address,std::uint32_t port){ //00747860.
        if(payload.size()<500){enqueue({encode(payload,key28_),address,port});return;}
        ++fragment_id24_; //Before the first required18 buffer write.
        if(!fragment_buffer_)throw std::logic_error("Fragmentation requires actual socket18 storage");
        unsigned char index=0;
        for(std::size_t at=0;at<payload.size();){
            const auto count=std::min<std::size_t>(500,payload.size()-at);
            std::vector<unsigned char> part(count+5);part[0]=index++|0x80u;
            connection_store32(part.data()+1,fragment_id24_|(count==payload.size()-at?0x80000000u:0u));
            std::copy_n(payload.data()+at,count,part.data()+5);at+=count;
            deferred_->push_back({encode(part,key28_),address,port});
        }
    }
    void forward_fragment(std::uint32_t& shared_clock){ //00748070, SAME process00983100, strict signed comparison.
        if(deferred_->empty())return;
        const auto now=os_.control_milliseconds();
        if(connection_signed(shared_clock)>=connection_signed(now))return;
        auto value=std::move(deferred_->front());shared_clock=now;deferred_->pop_front();enqueue(std::move(value));
    }
    std::optional<ControlDatagram> receive(){ //00747F50→00747A40, reached unfragmented receiver.
        if(!receive_buffer_)throw std::logic_error("UDP receive requires its actual14 buffer");
        auto packet=os_.control_udp_receive(socket_,receive_buffer_.get(),32768,20000);
        if(!packet||packet->bytes.empty())return {};
        received1c_+=static_cast<std::uint32_t>(packet->bytes.size());
        if(packet->bytes.size()<4)throw std::runtime_error("Datagram checksum reads unproduced/truncated header bytes");
        const auto length=packet->bytes.size()-3;auto& data=packet->bytes;
        const auto mask=static_cast<std::uint32_t>(length)*0x800u+key28_;std::uint32_t sum=0;
        for(std::size_t n=0;n<length/4;++n){auto* p=data.data()+3+n*4;const auto word=connection_load32(p)^mask;connection_store32(p,word);sum+=word;}
        if(((connection_load32(data.data())^sum)&0xffffffu)!=0)return {};
        if(data[3]&0x80u)throw std::logic_error("Received fragment requires actual00747A40 reassembly owner");
        data.erase(data.begin(),data.begin()+3);return packet;
    }
    void drain(){ //00748180: pop under54, unlock for send/free, relock, preserving FIFO.
        lock_->enter();bool held=true;
        try{
            while(!immediate_->empty()){
                auto value=std::move(immediate_->front());immediate_->pop_front();
                lock_->leave();held=false;
                bound_address_=value.address;bound_port_=static_cast<std::uint16_t>(value.port); //00747A00 mutates SAME sockaddr before sendto.
                os_.control_udp_send(socket_,value.bytes,value.address,static_cast<std::uint16_t>(value.port));
                sent20_+=static_cast<std::uint32_t>(value.bytes.size());
                value=ControlDatagram{}; //007496C0 before reacquiring54.
                lock_->enter();held=true;
            }
        }catch(...){if(held)lock_->leave();throw;}
        lock_->leave();
    }
    void send_loop(){ //00748220, no synthetic completion on exceptions.
        os_.control_thread_priority(2);
        while(!stop70_){(void)os_.wait_control_event(wake78_,0xffffffffu);drain();os_.reset_control_event(wake78_);}
        os_.signal_control_event(completion74_);
    }
    static std::vector<unsigned char> encode(const std::vector<unsigned char>& payload,std::uint32_t key=0x1742bdcdu){ //007477A0.
        std::vector<unsigned char> result(payload.size()+3);std::copy(payload.begin(),payload.end(),result.begin()+3);
        std::uint32_t sum=0;for(std::size_t n=0;n<payload.size()/4;++n)sum+=connection_load32(payload.data()+n*4);
        for(unsigned n=0;n<3;++n)result[n]=static_cast<unsigned char>(sum>>(n*8));
        const auto mask=static_cast<std::uint32_t>(payload.size())*0x800u+key;
        for(std::size_t n=0;n<payload.size()/4;++n){auto* p=result.data()+3+n*4;connection_store32(p,connection_load32(p)^mask);}
        return result;
    }
    auto native_socket()const{return socket_;}
    auto received_bytes()const{return received1c_;}auto sent_bytes()const{return sent20_;}
    auto fragment_id()const{return fragment_id24_;}
    bool has_buffers()const{return receive_buffer_&&fragment_buffer_;}
    std::size_t immediate_count()const{return immediate_->size();}std::size_t deferred_count()const{return deferred_->size();}
private:
    using Queue=std::list<ControlDatagram>;
    void enqueue(ControlDatagram value){ //007480F0.
        lock_->enter();try{immediate_->push_back(std::move(value));}catch(...){lock_->leave();throw;}
        lock_->leave();os_.signal_control_event(wake78_);
    }
    static constexpr std::uintptr_t invalid_socket=0xffffffffu;
    Platform& os_;std::unique_ptr<Queue> deferred_,immediate_;
    std::unique_ptr<unsigned char[]> receive_buffer_,fragment_buffer_;
    std::unique_ptr<ControlConnectionLock> lock_;
    std::uintptr_t socket_=invalid_socket;
    std::uint32_t bound_address_=0,bound_port_=0,received1c_=0,sent20_=0,fragment_id24_=0,key28_=0x1742bdcdu,thread_id_=0;
    std::atomic<bool> stop70_{false};std::atomic<Event> completion74_{0},wake78_{0};
    std::unique_ptr<ControlConnectionThread> thread_;bool completion_open_=false,wake_open_=false,wsa_=false,closed_=false;
};
} //namespace hd2::frontend
#endif
