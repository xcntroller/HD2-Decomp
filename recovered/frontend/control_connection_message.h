#ifndef HD2_FRONTEND_CONTROL_CONNECTION_MESSAGE_H
#define HD2_FRONTEND_CONTROL_CONNECTION_MESSAGE_H

#include "frontend/control_connection_stream.h"
#include <memory>
#include <optional>
#include <utility>

namespace hd2::frontend {
inline std::int32_t control_queue_signed(std::uint32_t value){std::int32_t result;std::memcpy(&result,&value,4);return result;}
inline std::uint32_t lift_control_sequence(std::uint32_t value,std::uint32_t prior){ //007450C0,23-bit sequence.
    constexpr std::uint32_t mask=0x7fffff,q=mask>>2;value&=mask;
    if(q*3<(prior&mask)&&value<q)return ((prior+mask)&~mask)+value;
    if((prior&mask)<q&&q*3<value)return ((prior-mask)&~mask)+value;
    return (prior&~mask)+value;
}

//007459E0/00745AD0 consumed record fields. Byte-vector/shared ownership adapts
//the native packet-pool allocation and virtual04/08 retain/release boundary;
//all queue/container handles refer to ONE record, not cloned message payloads.
//This is not another packet pool or a native allocator/refcount/layout claim.
struct ControlConnectionMessage {
    using Pointer=std::shared_ptr<ControlConnectionMessage>;
    std::uint32_t flags=0,channel=0,generation=0;
    std::optional<std::uint32_t> sequence; //Native reset does NOT produce14.
    std::vector<unsigned char> bytes;
    std::uint32_t required_sequence()const{
        if(!sequence)throw std::logic_error("Connection message14 requires enqueue or decode producer");
        return *sequence;
    }
    void write(ControlConnectionStream& stream)const{ //00745B40; raw byte copy follows48 bits, no new alignment.
        stream.write(flags,2);stream.write(static_cast<std::uint32_t>(bytes.size()),16);stream.write(channel,6);
        stream.write(required_sequence(),23);stream.write(generation,1);stream.write_bytes(bytes);
    }
    static Pointer read(ControlConnectionStream& stream){ //00745BF0, native signed16 length.
        auto result=std::make_shared<ControlConnectionMessage>();
        result->flags=stream.read(2);const auto size=stream.read(16);result->channel=stream.read(6);
        result->sequence=stream.read(23);result->generation=stream.read(1);
        if(size&0x8000u)throw std::runtime_error("Message payload allocator reached native negative signed16 length");
        result->bytes=stream.read_bytes(size);return result;
    }
};

//Native12/20-byte list nodes and allocated empty heads, adapted to typed payload
//ownership. Head payload is never read. Erase frees a node BEFORE returning its
//moved value. Default destruction releases payload then unlinks, reverse order.
template<class Value>class ControlConnectionList {
    struct Node{
        Node* next=this;Node* previous=this;std::optional<Value> value;
        Node()=default;explicit Node(Value v):value(std::move(v)){}
    };
public:
    class Iterator{
        Node* node_;friend class ControlConnectionList;
        explicit Iterator(Node* value):node_(value){}
    public:
        Value& operator*()const{return node_->value.value();}
        Value* operator->()const{return &node_->value.value();}
        Iterator& operator++(){node_=node_->next;return *this;}
        bool operator==(Iterator other)const{return node_==other.node_;}
        bool operator!=(Iterator other)const{return !(*this==other);}
    };
    ControlConnectionList():head_(std::make_unique<Node>()){}
    ~ControlConnectionList(){while(!empty())discard(Iterator(head_->previous));}
    ControlConnectionList(const ControlConnectionList&)=delete;
    ControlConnectionList& operator=(const ControlConnectionList&)=delete;
    Iterator begin()const{return Iterator(head_->next);}Iterator end()const{return Iterator(head_.get());}
    bool empty()const{return !count_;}std::size_t size()const{return count_;}
    void push_back(Value value){
        auto node=std::make_unique<Node>(std::move(value));node->next=head_.get();node->previous=head_->previous;
        head_->previous->next=node.get();head_->previous=node.release();++count_;
    }
    Value take(Iterator at){
        if(at==end())throw std::logic_error("Connection list reached its sentinel payload");
        auto value=std::move(at.node_->value.value());unlink(at.node_);return value;
    }
    void discard(Iterator at){
        if(at==end())throw std::logic_error("Connection list retirement reached its sentinel");
        at.node_->value.reset();unlink(at.node_);
    }
    void clear_front(){while(!empty())discard(begin());}
    void clear_back(){while(!empty())discard(Iterator(head_->previous));}
private:
    void unlink(Node* node){node->previous->next=node->next;node->next->previous=node->previous;--count_;delete node;}
    std::unique_ptr<Node> head_;std::size_t count_=0;
};
using ControlConnectionMessages=ControlConnectionList<ControlConnectionMessage::Pointer>;
struct ControlConnectionContainer:ControlConnectionMessages{
    ~ControlConnectionContainer(){clear_front();} //00747130, payload release BEFORE each front node unlink.
};
} //namespace hd2::frontend
#endif
