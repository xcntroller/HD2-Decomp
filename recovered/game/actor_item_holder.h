#ifndef HD2_GAME_ACTOR_ITEM_HOLDER_H
#define HD2_GAME_ACTOR_ITEM_HOLDER_H

#include <cstdint>
#include <utility>

namespace hd2::game {

//007D67F0/007D6910: a single-entry owner, distinct from007D7010's slot array.
//Entries use the same matching release endpoint as ActorSlotStorage. This owner
//always releases its remaining entry; field18 is not a borrowed/owned switch.
template<class Backend>
class ActorItemHolder {
public:
    using Entry=typename Backend::Entry;
    ActorItemHolder(Backend& backend,std::uint32_t kind) : backend_(backend),kind_(kind) {
        static_assert(noexcept(std::declval<Backend&>().release_entry(std::declval<Entry>())),
                      "Actor entry release must not throw during cleanup");
    }
    ~ActorItemHolder() {clear_entry();}
    ActorItemHolder(const ActorItemHolder&)=delete;
    ActorItemHolder& operator=(const ActorItemHolder&)=delete;
    std::uint32_t kind() const {return kind_;}
    float first_scalar() const {return first_;}
    float second_scalar() const {return second_;}
    std::uint32_t flags() const {return flags_;}
    void first_scalar(float value) {first_=value;} //007D66C0.
    void second_scalar(float value) {second_=value;} //007D66E0.
    void flags(std::uint32_t value) {flags_=value;} //007D6700.
    Entry entry() const {return entry_;}
    std::uint32_t field18() const {return field18_;}
    void field18(std::uint32_t value) {field18_=value;}
    bool cooling_down(std::uint32_t game_word28) const {return game_word28<field18_;} //007D6890, unsigned.
    float total_scalar() const {return entry_?backend_.entry_scalar(entry_):0.f;} //007D6970.
    long double total_volume() const {return entry_?backend_.entry_volume(entry_):0.L;} //007D6990.
    bool empty() const {return !entry_;} //007D6850.
    //007D69B0: replacing an entry transfers the old handle to the caller. When
    //empty, leave displaced untouched. The original third argument is unused.
    std::int32_t replace(Entry value,Entry& displaced) {
        if(!value)return -2;
        if(entry_)displaced=entry_;
        entry_=value;return 1;
    }
    std::int32_t detach(Entry value) { //007D69E0; null matches an empty holder.
        if(value!=entry_)return -1;
        entry_=Entry{};return 1;
    }
    void clear_entry() noexcept { //007D6A00, including field18 reset when empty.
        if(entry_)backend_.release_entry(entry_);
        field18_=0;entry_=Entry{};
    }
private:
    Backend& backend_;
    std::uint32_t kind_;
    float first_=100,second_=100;
    std::uint32_t flags_=3;
    Entry entry_{};
    std::uint32_t field18_=0;
};

} //namespace hd2::game
#endif
