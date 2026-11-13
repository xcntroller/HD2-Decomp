#ifndef HD2_FRONTEND_CONTROL_CONNECTION_RECORDS_H
#define HD2_FRONTEND_CONTROL_CONNECTION_RECORDS_H

#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>
#include <atomic>
#include <functional>

namespace hd2::frontend {
class ControlConnectionStream;
struct ControlConnectionMessage;

enum class ControlConnectionStorage{unproduced,loader_zero_filled_process};

struct ControlConnectionPump {
    using Receiver=std::function<void(const std::vector<unsigned char>&,std::uint32_t,std::uint32_t)>;
    virtual ~ControlConnectionPump()=default;
    virtual std::uint32_t connection_clock()=0;
    virtual bool connection_callback_bound()const=0;
    virtual Receiver connection_receiver(){
        if(!connection_callback_bound())return {};
        return [](const auto&,std::uint32_t,std::uint32_t){throw std::logic_error("Connection requires actual process414 payload receiver");};
    }
    virtual ControlConnectionStream& connection_scratch()=0;
    virtual void connection_send(const std::vector<unsigned char>& bytes,std::uint32_t address,std::uint32_t port,void* context)=0;
};

// Consumed +08 of the actual 0x50-byte00745D20 connection, NOT a replacement
// constructor/transport. ControlConnectionOwner constructs the queue/lock/clock
// boundary; its containing caller must construct the
// containing object before publishing it as peer6C or client334. In particular,
// no default here supplies their missing lifetime or a successful connection.
class ControlConnectionRecord {
public:
    ControlConnectionRecord()=default;
    ControlConnectionRecord(const ControlConnectionRecord&)=delete;
    ControlConnectionRecord& operator=(const ControlConnectionRecord&)=delete;
    struct Lifetime{std::atomic<bool> live{true};}; //Source safety metadata, not an original field or readiness flag.
    virtual ~ControlConnectionRecord(){invalidate_lifetime();}
    auto lifetime()const{return std::weak_ptr<Lifetime>{lifetime_};}
    void invalidate_lifetime()noexcept{lifetime_->live=false;}
    virtual std::uint32_t identity()const{throw std::logic_error("Connection requires its actual containing owner");}
    virtual void identity(std::uint32_t){throw std::logic_error("Connection identity publication requires its actual owner");}
    virtual void maintain(ControlConnectionPump&){throw std::logic_error("Connection maintenance requires its actual owner");}
    virtual void flush(ControlConnectionPump&,bool){throw std::logic_error("Connection flush requires its actual owner");}
    virtual void receive(ControlConnectionPump&,std::uint32_t,ControlConnectionStream&,std::uint32_t,std::uint32_t){
        throw std::logic_error("Connection receive requires its actual owner");
    }
    virtual std::weak_ptr<ControlConnectionMessage> submit(const std::vector<unsigned char>&,std::uint32_t,std::uint32_t,ControlConnectionPump&){
        throw std::logic_error("Connection submission requires its actual owner");
    }
    std::uint32_t generation() const {
        if(!generation_produced08_.load())throw std::logic_error("Connection generation has no original producer");
        return generation08_.load();
    }
    void generation(std::uint32_t value){generation08_=value;generation_produced08_=true;}
private:
    std::atomic<std::uint32_t> generation08_{0}; //Native lockless08 publication versus worker packet reads.
    std::atomic<bool> generation_produced08_{false}; //Source definedness metadata; backing zero is inaccessible.
    std::shared_ptr<Lifetime> lifetime_=std::make_shared<Lifetime>();
};

// Worker+04/+08/+0C registry.007466E0 constructs an EMPTY vector; it does not
// initialize D4. Only007467B0's actual start or a subsequent native writer can
// supply D4. Entries BORROW connections; neither erase nor vector retirement
// deletes them.00746890 and peer-disconnect destruction are separate consumers.
template<class Allocator=std::allocator<ControlConnectionRecord*>>
class ControlConnectionRegistry {
public:
    explicit ControlConnectionRegistry(const Allocator& allocator=Allocator{}):entries_(allocator){}
    explicit ControlConnectionRegistry(ControlConnectionStorage storage,const Allocator& allocator=Allocator{}):entries_(allocator){
        if(storage==ControlConnectionStorage::loader_zero_filled_process)generationd4_=0;
    }
    void generation(std::uint32_t value){generationd4_=value;}
    std::uint32_t generation()const {
        if(!generationd4_)throw std::logic_error("Worker generation has no original producer");
        return *generationd4_;
    }
    const auto& entries()const{return entries_;}
    void reset_end(){entries_.clear();} //007467B0 resets end WITHOUT deleting prior pointers or freeing capacity.
    void pop_deleted_last(){if(entries_.empty())throw std::logic_error("Deleted connection has no registry entry");entries_.pop_back();}
    void release_storage(){decltype(entries_) empty(entries_.get_allocator());entries_.swap(empty);}

    template<class Locks>void append(ControlConnectionRecord& connection,Locks& locks){ //00746B50.
        locks.enter_control_lock(2); //SAME worker B4, not process418/430.
        struct Unlock{Locks& locks;~Unlock(){locks.leave_control_lock(2);}}unlock{locks};
        connection.generation(generation()); //BEFORE any vector allocation/append, even on failure.
        if(entries_.size()==entries_.capacity()){
            const auto count=entries_.size(),extra=count?count:1;
            if(extra>entries_.max_size()-count)throw std::length_error("Original connection registry capacity");
            entries_.reserve(count+extra);
        }
        entries_.push_back(&connection); //Duplicates are native; do not deduplicate.
    }
    template<class Locks>void erase(ControlConnectionRecord& connection,Locks& locks){ //00746D20.
        locks.enter_control_lock(2);
        struct Unlock{Locks& locks;~Unlock(){locks.leave_control_lock(2);}}unlock{locks};
        for(std::size_t n=0;n<entries_.size();++n){
            if(entries_[n]!=&connection)continue;
            if(entries_.size()>1)entries_[n]=entries_.back();
            entries_.pop_back();break; //First match only. No deletion, no connection08 clear.
        }
    }
private:
    std::vector<ControlConnectionRecord*,Allocator> entries_;
    std::optional<std::uint32_t> generationd4_;
};

} //namespace hd2::frontend
#endif
