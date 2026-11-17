#ifndef HD2_GAME_DOG_BODY_ORDERS_H
#define HD2_GAME_DOG_BODY_ORDERS_H
#include <array>
#include <cstdint>
#include <cstring>
#include <optional>
#include <stdexcept>
namespace hd2::game {
inline std::int32_t dog_order_signed(std::uint32_t value){std::int32_t result;std::memcpy(&result,&value,4);return result;}
//005D4C70 initializes only1C/20. Commands0..0C, final action10, posture14
//and stance18 share the real nine-word record; unwritten cold words are typed
//zero without claiming native values. Source storage is not an i386 ABI overlay.
struct DogBodyOrderRecord {
    std::array<std::uint32_t,7> unknown_words{};
    std::uint32_t field1c=0xffffffffu,field20=0xffffffffu;
    //Source provenance only: cold native10/14/18 are unwritten. Queued
    //metadata may retain an earlier producer across cursor reset/reuse.
    std::array<bool,3> metadata_produced{};
    void reset_cursor(){field1c=field20=0xffffffffu;}
    void publish_metadata(std::uint32_t action,std::uint32_t posture,std::uint32_t stance){unknown_words[4]=action;unknown_words[5]=posture;unknown_words[6]=stance;metadata_produced.fill(true);}
    std::uint32_t metadata(unsigned index)const{if(index>=3||!metadata_produced[index])throw std::runtime_error("Dog queue metadata requires its actual005C5030 producer");return unknown_words[4+index];}
    void append(std::uint32_t action){ //Inlined in005C5030; at most four commands.
        if(field1c==0xffffffffu){field1c=3;field20=4;}
        if(dog_order_signed(field20)>0){--field20;if(field20>=4)throw std::runtime_error("Dog order append reaches native invalid action slot");unknown_words[field20]=action;}
    }
    bool exhausted()const{return field1c==0xffffffffu||dog_order_signed(field1c)<dog_order_signed(field20);}
    std::uint32_t action()const{if(exhausted()||field1c>=4)throw std::runtime_error("Dog order reaches native invalid current action");return unknown_words[field1c];}
    void consume(){if(field1c!=0xffffffffu&&dog_order_signed(field1c)>=dog_order_signed(field20))--field1c;}
};
//One owned current314 record and five338 records on the actual Dog actor.
//Body activation005C7D30 supplies head/tail3EC/3F0; cold construction does not.
class DogBodyOrders {
public:
    DogBodyOrderRecord current;
    std::array<DogBodyOrderRecord,5> records;
    void initialize(){ //The order portion of005C7D30, not complete Body readiness.
        current.reset_cursor();for(auto& record:records)record.reset_cursor();
        current.publish_metadata(23,0,0);
        first_=last_=-1;
    }
    bool produced()const noexcept{return first_.has_value()&&last_.has_value();}
    bool empty()const{return first()==-1;}
    std::int32_t first()const{require_producer();return *first_;}
    std::int32_t last()const{require_producer();return *last_;}
    DogBodyOrderRecord& enqueue(){ //005C4B90: bounded overwrite-oldest queue.
        require_producer();
        if(*first_==-1){first_=last_=0;records[0].reset_cursor();return records[0];}
        //Tail=-1 after native005C4C70 is intentionally not silently wrapped.
        if(*last_<-1||*last_>4)throw std::runtime_error("Dog queue append reaches native invalid tail");
        ++*last_;if(*last_>4)last_=0;
        if(*first_==*last_){++*first_;if(*first_>4)first_=0;}
        auto& result=record(*last_);result.reset_cursor();return result;
    }
    void discard_first(){ //005C4C30, leaves the old tail on final removal.
        require_producer();if(*first_==-1)return;
        if(*first_==*last_){first_=-1;return;}
        ++*first_;if(*first_>4)first_=0;
    }
    void discard_last(){ //005C4C70's unusual HEAD store is proven by raw5C4CA2.
        require_producer();if(*first_==-1)return;
        if(*first_==*last_)first_=-1;else --*last_;
        if(*last_<0)first_=4; //Native stores35C, not360. Preserve, do not repair.
    }
    void discard_all(){require_producer();unsigned count=0;while(*first_!=-1){if(++count>5)throw std::runtime_error("Dog queue clear reaches native invalid head/tail");discard_first();}}
    void take_first(){ //005C4CB0 copies all nine words BEFORE005C4C30.
        require_producer();if(*first_==-1)throw std::runtime_error("Dog queue copy reaches native invalid first record");
        current=record(*first_);discard_first();
    }
    //005C85A0/005C8600/005C8660 read LAST queued metadata, not first.
    std::uint32_t next_action()const{return metadata().metadata(0);}
    std::uint32_t next_posture()const{return metadata().metadata(1);}
    std::uint32_t next_stance()const{return metadata().metadata(2);}
private:
    void require_producer()const{if(!produced())throw std::runtime_error("Dog order queue requires actual005C7D30 activation producer");}
    DogBodyOrderRecord& record(std::int32_t index){if(index<0||index>4)throw std::runtime_error("Dog order queue reaches native invalid record index");return records[static_cast<unsigned>(index)];}
    const DogBodyOrderRecord& metadata()const{require_producer();if(*first_==-1)return current;if(*last_<0||*last_>4)throw std::runtime_error("Dog order query reaches native invalid last record");return records[static_cast<unsigned>(*last_)];}
    std::optional<std::int32_t> first_,last_;
};
}
#endif
