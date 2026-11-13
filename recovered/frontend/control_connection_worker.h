#ifndef HD2_FRONTEND_CONTROL_CONNECTION_WORKER_H
#define HD2_FRONTEND_CONTROL_CONNECTION_WORKER_H
#include "frontend/control_connection_socket.h"
#include "frontend/control_startup.h"
#include <mutex>
#include <unordered_map>

namespace hd2::frontend {
//Actual singleton+33C. Process-static zero fill is explicit; it is NOT a
//constructor default for arbitrary worker allocations. No DirectPlay service,
//application load or DirectPlay admission service is supplied.
template<class Platform>class ControlConnectionWorker final:public ControlConnectionPump {
public:
    using Owner=ControlConnectionOwner<Platform>;
    ControlConnectionWorker(Platform& platform,ControlStartupState& process,
            ControlConnectionStorage storage):os_(platform),process_(process),registry_(storage),socket_(platform,storage){
        os_.initialize_control_worker_lock(); //007466E0: socket constructor BEFORE B4.
        process_.callback414.reset(); //WorkerD8 IS process414, not a second callback field.
    }
    ~ControlConnectionWorker(){
        try{reset_native();}catch(...){os_.record_control_worker_failure(std::current_exception());
            stopac_=true;if(thread_&&!thread_->join(0xffffffffu))std::terminate();socket_.force_stop();}
        thread_.reset(); //Additional source handle ownership; no native ignored-handle leak claim.
        socket_.force_stop(); //Also consume a throwing pre-A4 socket-start prefix before storage retirement.
        if(completion_open_)os_.close_control_event(completionb0_); //Exceptional pre-A4 source acquisition only.
        os_.delete_control_critical_section(2);socket_.close_storage();registry_.release_storage(); //00746750.
    }
    ControlConnectionWorker(const ControlConnectionWorker&)=delete;
    ControlConnectionWorker& operator=(const ControlConnectionWorker&)=delete;
    void start(std::uint32_t port){ //007467B0.
        if(active_||stopping_||completion_open_)throw std::logic_error("Worker restart requires prior retirement");
        completionb0_=os_.create_control_event();completion_open_=true;
        socket_.start(port);registry_.reset_end();active_=true;stopac_=false;registry_.generation(0);
        clockcc_=clockd0_=os_.control_milliseconds();
        thread_=os_.create_control_thread([this]{run();},thread_id_);
    }
    void stop(){ //00746840. A timed wait alone never proves a callback is dead.
        if(!active_&&!stopping_)return;
        active_=false;stopac_=true;stopping_=true;
        (void)os_.wait_control_event(completionb0_,1000);
        if(thread_&&!thread_->join(1000))throw std::runtime_error("Worker callback still owns its storage");
        auto failure=thread_?thread_->failure():std::exception_ptr{};
        const auto socket_failure=socket_.stop();
        os_.close_control_event(completionb0_);completion_open_=false;stopping_=false;thread_.reset();
        if(failure)std::rethrow_exception(failure);
        if(socket_failure)std::rethrow_exception(socket_failure);
    }
    void reset_native(){ //Reached0072E4E0: reverse deletion BEFORE334 clear BEFORE worker stop.
        delete_all();process_.client334=nullptr;stop();
        //An allocation published but never registered remains source-owned for
        //exceptional cleanup. This is RAII, not native successful registration.
        {std::lock_guard<std::mutex> guard(owned_mutex_);owned_.clear();}
    }
    Owner& make(std::uint32_t identity,std::uint32_t address){
        auto value=std::make_unique<Owner>(os_,identity,address,&socket_);auto* p=value.get();
        std::lock_guard<std::mutex> guard(owned_mutex_);owned_.emplace(p,std::move(value));return *p;
    }
    Owner& publish_client(std::uint32_t address,std::uint32_t descriptor_port){ //00723D20, AFTER actual worker start.
        process_.server_address338=address;
        auto& value=make(0,process_.server_address338);process_.client334=&value;value.port(descriptor_port+1u);
        registry_.append(value,os_);flush_all(true);return value;
    }
    void complete_client(std::uint32_t result,const std::vector<unsigned char>& reply,std::int32_t& local_peer){ //00722180 FFFF0005.
        if(result==0){
            if(reply.empty())throw std::runtime_error("Client completion requires its actual reply byte");
            local_peer=reply[0]&0x7f; //No admission/bounds check here; BEFORE334 dereference, no418/B4 lock.
            if(!process_.client334)throw std::logic_error("Client completion lost its actual334 owner");
            process_.client334->identity(static_cast<std::uint32_t>(local_peer));
        }else if(!reply.empty())process_.word5044=reply[0]; //Failure byte is NOT masked to a peer slot.
    }
    void retire_client(){ //00722180 FFFF0016 prefix; current-application status11 belongs to its caller.
        if(auto* value=process_.client334){registry_.erase(*value,os_);retire(value);process_.client334=nullptr;}
        process_.word5044=0;
    }
    Owner& finish_prepared_peer(std::uint32_t slot){ //007260A0 CREATE_PLAYER tail, after admission/name/04 producer.
        Owner* value;
        {Lock records(os_,0);process_.records.at(slot).word(0xa8,os_.peer_human_ticks());process_.records.at(slot).byte(0xa1,0);
            const auto address=process_.records.at(slot).word(4); //00724DC0's prior address producer is REQUIRED.
            value=&make(slot,address);process_.records.at(slot).connection(value);
            for(unsigned at=0x40;at<0x64;at+=4)process_.records.at(slot).word(at,0);
        }
        registry_.append(*value,os_);return *value; //AFTER418 unlock.
    }
    void disconnect_peer(std::uint32_t slot){ //007260A0 DESTROY_PLAYER connection prefix; input70 is a later receiver.
        Lock records(os_,0);auto* value=process_.records.at(slot).connection();
        if(value){registry_.erase(*value,os_);retire(value);process_.records.at(slot).connection(nullptr);}
    }
    void abort_peer(std::uint32_t slot){ //INDICATED_CONNECT_ABORTED: native does NOT clear6C after deletion.
        Lock records(os_,0);process_.records.at(slot).word(0,0);process_.records.at(slot).byte(8,0);
        auto* value=process_.records.at(slot).connection();if(value){registry_.erase(*value,os_);retire(value);}
    }
    void delete_all(){ //00746890, reverse vtable00 deletion THEN fresh end decrement, SAME B4.
        Lock registry(os_,2);
        while(!registry_.entries().empty()){auto* value=registry_.entries().back();if(value)retire(value);registry_.pop_deleted_last();}
    }
    void maintain(){ //00746DA0.
        {Lock registry(os_,2);for(std::size_t n=0;n<registry_.entries().size();++n)registry_.entries().at(n)->maintain(*this);}
        const auto now=os_.control_milliseconds();if(connection_signed(now-clockcc_)>500)flush_all(true);
    }
    void flush_all(bool force){ //00746E00: timestamp BEFORE lock/iteration; inactive branch reads no clock.
        if(!active_)return;
        clockcc_=os_.control_milliseconds();Lock registry(os_,2);
        for(std::size_t n=0;n<registry_.entries().size();++n)registry_.entries().at(n)->flush(*this,force);
    }
    void receive(){ //00746A10.
        socket_.forward_fragment(fragment_clock_);
        for(auto packet=socket_.receive();packet;packet=socket_.receive()){
            clockd0_=os_.control_milliseconds();
            ControlConnectionStream stream;stream.receive(packet->bytes);
            const auto kind=stream.read(8),identity=stream.read(8);
            {Lock registry(os_,2);
                if(auto* value=find(identity))value->receive(*this,kind,stream,packet->address,packet->port);
            }
            socket_.forward_fragment(fragment_clock_);
        }
    }
    ControlConnectionRecord* find(std::uint32_t identity){ //00746900 takes B4 even when its caller already holds it.
        Lock registry(os_,2);
        for(std::size_t n=0;n<registry_.entries().size();++n){auto* value=registry_.entries().at(n);if(value->identity()==identity)return value;}
        return nullptr;
    }
    void run(){ //00746EB0 and native007466D0 wrapper (absent frozen, no new wrapper credit).
        os_.control_thread_priority(2);while(!stopac_){maintain();receive();}os_.signal_control_event(completionb0_);
    }
    bool active()const{return active_;}bool stopping()const{return stopping_;}
    auto& registry(){return registry_;}auto& socket(){return socket_;}
    auto clockcc()const{return clockcc_.load();}auto clockd0()const{return clockd0_.load();}
    std::uint32_t connection_clock()override{return os_.control_milliseconds();}
    bool connection_callback_bound()const override{return process_.callback414.has_value();}
    typename ControlConnectionPump::Receiver connection_receiver()override{
        const auto callback=process_.callback414;if(!callback)return {};
        return [this,callback](const auto& bytes,std::uint32_t channel,std::uint32_t flags){
            if(*callback!=ControlStartupCallback::packed_event_0072c360)throw std::logic_error("Unknown actual process414 callback");
            os_.receive_control_connection_payload(bytes,channel,flags); //0072C360→SAME process FIFO, not World dispatch.
        };
    }
    ControlConnectionStream& connection_scratch()override{return scratch_;}
    void connection_send(const std::vector<unsigned char>& bytes,std::uint32_t address,std::uint32_t port,void* context)override{
        if(context!=&socket_)throw std::logic_error("Connection send lost its actual38 socket owner");
        socket_.send(bytes,address,port);
    }
private:
    struct Lock{Platform& p;unsigned i;Lock(Platform& v,unsigned n):p(v),i(n){p.enter_control_lock(i);}~Lock(){p.leave_control_lock(i);}};
    void retire(ControlConnectionRecord* value){
        std::unique_ptr<ControlConnectionRecord> owner;
        {std::lock_guard<std::mutex> guard(owned_mutex_);auto found=owned_.find(value);
            if(found==owned_.end())throw std::logic_error("Worker deletion requires a live owned connection (including duplicate deletion)");
            owner=std::move(found->second);owned_.erase(found);}
        owner.reset();
    }
    Platform& os_;ControlStartupState& process_;
    ControlConnectionRegistry<> registry_;ControlConnectionSocket<Platform> socket_;
    ControlConnectionStream scratch_;
    std::unordered_map<ControlConnectionRecord*,std::unique_ptr<ControlConnectionRecord>> owned_;
    std::mutex owned_mutex_;
    std::atomic<bool> active_{false},stopac_{false};bool stopping_=false,completion_open_=false;
    std::uintptr_t completionb0_=0;std::uint32_t thread_id_=0,fragment_clock_=0;
    std::atomic<std::uint32_t> clockcc_{0},clockd0_{0}; //Native lockless shared fields, defined C++ publication.
    std::unique_ptr<ControlConnectionThread> thread_;
};
} //namespace hd2::frontend
#endif
