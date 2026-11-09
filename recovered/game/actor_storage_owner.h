#ifndef HD2_GAME_ACTOR_STORAGE_OWNER_H
#define HD2_GAME_ACTOR_STORAGE_OWNER_H

#include "game/actor_item_holder.h"
#include "game/actor_slot_storage.h"
#include <array>
#include <memory>
#include <variant>
#include <stdexcept>

namespace hd2::game {

//Cold values of mutable client globals00863758/0086375C. The live Human owner
//must supply their current values if a configuration consumer changes them.
struct HumanStorageScalars {float slot_second=12,holder_second=8;};

//Storage portion of00610200's virtual+08 initialization. Most actors invoke
//00610F80; Human overrides it with005A2740 and cleans up via005A2A40. These
//native owners are not laid out as the original client objects or vtables.
template<class Backend>
class ActorStorageOwner {
public:
    using Holder=ActorItemHolder<Backend>;
    using Slots=ActorSlotStorage<Backend>;
    using Storage=std::variant<std::monostate,std::unique_ptr<Holder>,std::unique_ptr<Slots>>;
    ActorStorageOwner(Backend& backend,std::uint32_t actor_type,HumanStorageScalars scalars={}) {
        try {
            if(actor_type==3) {
                single_=false;count_=6;
                make_holder(backend,0,4,2);
                auto slots=std::make_unique<Slots>(backend,1);
                slots->second_scalar(scalars.slot_second);slots->flags(1);
                owners_[1]=std::move(slots);
                make_holder(backend,2,scalars.holder_second,3);
                make_holder(backend,3,8,3);
                make_holder(backend,4,8,3);
                make_holder(backend,5,32,5);
            } else if(actor_type==4||actor_type==0x15||actor_type==0x14||actor_type==0x22) {
                count_=1;
                auto slots=std::make_unique<Slots>(backend,2);
                slots->first_scalar(1000);owners_[0]=std::move(slots);
            }
        } catch(...) {clear();throw;}
    }
    ~ActorStorageOwner() {clear();}
    ActorStorageOwner(const ActorStorageOwner&)=delete;
    ActorStorageOwner& operator=(const ActorStorageOwner&)=delete;
    bool single() const {return single_;} //Original flag78, not number of owners.
    std::size_t size() const {return count_;}
    Holder* holder(std::size_t index) {
        if(index==6)return nullptr;
        auto* owner=std::get_if<std::unique_ptr<Holder>>(&owners_.at(index));
        return owner?owner->get():nullptr;
    }
    Slots* slots(std::size_t index) {
        if(index==6)return pack_slots_; //Borrowed from the equipped pack's actual RuntimeItem.
        auto* owner=std::get_if<std::unique_ptr<Slots>>(&owners_.at(index));
        return owner?owner->get():nullptr;
    }
    void pack_slots(Slots* value) {pack_slots_=value;} //007D8B40 publishes Human array index6.
    void reset_spawn_entries() { //007DDEF0; remove borrowed pack6 BEFORE ascending0..5 retirement.
        if(!single_)pack_slots_=nullptr;
        const auto last=single_?0u:6u;
        for(unsigned index=0;index<=last;++index) {
            if(auto* array=slots(index))array->clear_entries(); //Array+4C is the genuine base RET4.
            else if(auto* value=holder(index)){value->clear_entry();value->field18(0);}
        }
    }
    void clear_human_entries() { //005A29C0; pack contents6 BEFORE releasing the pack holder5.
        if(single_||count_!=6)throw std::logic_error("Human inventory retirement requires its actual seven-pointer storage");
        for(auto index:{0u,1u,2u,3u,4u,6u,5u}) {
            if(auto* array=slots(index))array->clear_entries();
            else if(auto* value=holder(index))value->clear_entry();
        }
    }
    void clear() noexcept { //00611020/005A2A40: ascending owner order.
        pack_slots_=nullptr;
        for(auto& owner:owners_)owner.template emplace<0>();
        count_=0;
    }
private:
    void make_holder(Backend& backend,std::size_t index,float second,std::uint32_t flags) {
        auto holder=std::make_unique<Holder>(backend,0);
        holder->second_scalar(second);holder->flags(flags);owners_[index]=std::move(holder);
    }
    std::array<Storage,6> owners_{}; //Human's seventh pointer separately borrows equipped pack slots.
    std::size_t count_=0;
    bool single_=true;
    Slots* pack_slots_=nullptr;
};

} //namespace hd2::game
#endif
