#ifndef HD2_FRONTEND_CONTROL_CONNECTION_RECORDS_H
#define HD2_FRONTEND_CONTROL_CONNECTION_RECORDS_H

#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>

namespace hd2::frontend {

// Consumed +08 of the actual 0x50-byte00745D20 connection, NOT a replacement
// constructor/transport. ControlConnectionOwner constructs the queue/lock/clock
// boundary; its containing caller must construct the
// containing object before publishing it as peer6C or client334. In particular,
// no default here supplies their missing lifetime or a successful connection.
class ControlConnectionRecord {
public:
    virtual ~ControlConnectionRecord()=default; //Native vtable00 deletes the containing owner, not this consumed-field view alone.
    std::uint32_t generation() const {
        if(!generation08_)throw std::logic_error("Connection generation has no original producer");
        return *generation08_;
    }
    void generation(std::uint32_t value){generation08_=value;}
private:
    std::optional<std::uint32_t> generation08_;
};

// Worker+04/+08/+0C registry.007466E0 constructs an EMPTY vector; it does not
// initialize D4. Only007467B0's actual start or a subsequent native writer can
// supply D4. Entries BORROW connections; neither erase nor vector retirement
// deletes them.00746890 and peer-disconnect destruction are separate consumers.
template<class Allocator=std::allocator<ControlConnectionRecord*>>
class ControlConnectionRegistry {
public:
    explicit ControlConnectionRegistry(const Allocator& allocator=Allocator{}):entries_(allocator){}
    void generation(std::uint32_t value){generationd4_=value;}
    std::uint32_t generation()const {
        if(!generationd4_)throw std::logic_error("Worker generation has no original producer");
        return *generationd4_;
    }
    const auto& entries()const{return entries_;}

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
