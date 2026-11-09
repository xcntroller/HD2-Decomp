#ifndef HD2_GAME_ACTOR_SLOT_STORAGE_H
#define HD2_GAME_ACTOR_SLOT_STORAGE_H

#include <cstdint>
#include <algorithm>
#include <type_traits>
#include <utility>
#include <vector>

namespace hd2::game {

//007D6690/007D7010 and007D7140/007D75B0. Entries are matching module handles;
//native1C counts occupied stacks, as established by007D7390/007D7470.
//Backend::release_entry must call the actual matching entry owner and not throw.
//The two100-valued fields and flags retain neutral names until their wider
//consumers are recovered. This is not a clock/timer implementation.
template<class Backend>
class ActorSlotStorage {
public:
    using Entry=typename Backend::Entry;
    struct Slot {std::uint8_t kind;std::vector<Entry> entries;};
    ActorSlotStorage(Backend& backend,std::uint32_t kind) : backend_(backend),kind_(kind) {
        static_assert(noexcept(std::declval<Backend&>().release_entry(std::declval<Entry>())),
                      "Actor entry release must not throw during cleanup");
        slots_.resize(kind==1?14:32,Slot{static_cast<std::uint8_t>(kind),{}});
    }
    ~ActorSlotStorage() {if(occupied_)clear_entries();}
    ActorSlotStorage(const ActorSlotStorage&)=delete;
    ActorSlotStorage& operator=(const ActorSlotStorage&)=delete;
    std::uint32_t kind() const {return kind_;}
    float first_scalar() const {return first_;}
    float second_scalar() const {return second_;}
    std::uint32_t flags() const {return flags_;}
    std::size_t occupied_stacks() const {return occupied_;}
    const std::vector<Slot>& slots() const {return slots_;}
    void first_scalar(float value) {first_=value;} //007D66C0.
    void second_scalar(float value) {second_=value;} //007D66E0.
    void flags(std::uint32_t value) {flags_=value;} //007D6700.
    //Raw access is for matching serialized/runtime producers; normal insertion
    //uses insert() so native1C remains synchronized with the actual slot list.
    Slot& slot(std::size_t index) {return slots_.at(index);}
    std::int32_t insert(Entry entry) { //007D7390, index/out arguments are unused.
        if(!entry)return -2;
        Slot* empty=nullptr;
        for(auto& slot:slots_) {
            if(slot.entries.empty()) {if(!empty)empty=&slot;}
            else if(backend_.entry_definition(entry)==backend_.entry_definition(slot.entries.front())&&
                    backend_.entry_family(entry)==backend_.entry_family(slot.entries.front())) {
                slot.entries.push_back(entry);return 1;
            }
        }
        if(!empty)return -1;
        empty->entries.push_back(entry);++occupied_;return 1;
    }
    bool empty() const {return occupied_==0;} //007D7630.
    std::int32_t detach(Entry entry) { //007D7470; consume no item owner.
        if(!entry)return -1;
        for(auto it=slots_.begin();it!=slots_.end();++it)if(!it->entries.empty()&&
            backend_.entry_definition(entry)==backend_.entry_definition(it->entries.front())&&
            backend_.entry_family(entry)==backend_.entry_family(it->entries.front())) {
            for(auto& value:it->entries)if(value==entry) {
                value=it->entries.back();it->entries.pop_back();
                if(it->entries.empty()) {
                    std::rotate(it,it+1,slots_.end());--occupied_;
                }
                return 1;
            }
            break;
        }
        return -1;
    }
    float total_volume() const { //007D72A0: first entry times stack size; store after each group.
        float result=0;
        for(const auto& slot:slots_)if(!slot.entries.empty())
            result=static_cast<float>(backend_.entry_volume(slot.entries.front())*slot.entries.size()+result);
        return result;
    }
    void clear_entries() noexcept { //007D75B0: ascending slots, reverse entries.
        for(auto& slot:slots_)while(!slot.entries.empty()) {
            backend_.release_entry(slot.entries.back());slot.entries.pop_back();
        }
        occupied_=0;
    }
    float total_scalar() const { //007D7220 uses the FIRST entry times stack size.
        float result=0;
        for(const auto& slot:slots_)if(!slot.entries.empty())
            result=static_cast<float>(static_cast<long double>(backend_.entry_scalar(slot.entries.front()))*slot.entries.size()+result);
        return result;
    }
private:
    Backend& backend_;
    std::uint32_t kind_;
    float first_=100,second_=100;
    std::uint32_t flags_=3;
    std::size_t occupied_=0;
    std::vector<Slot> slots_;
};

} //namespace hd2::game
#endif
