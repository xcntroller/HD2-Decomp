#ifndef HD2_GAME_ACTOR_REFERENCES_H
#define HD2_GAME_ACTOR_REFERENCES_H

#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace hd2::game {

//004CE8F0's references within a script context. Record's remaining fields are
//owned by the actual script producer; no serialized/raw-object ABI is implied.
//A record exposes .actor, and moving/destroying it must not throw in cleanup.
template<class Actor,class Record>
struct ActorContextReferences {
    std::vector<Record> records;
    std::array<Actor*,14> actors{};
    std::uint32_t field5a0=0,field564=0;
    bool flag4e=false;
    void remove_actor(Actor* actor) noexcept {
        static_assert(std::is_nothrow_move_assignable<Record>::value&&std::is_nothrow_destructible<Record>::value,
                      "Script reference records must not throw during cleanup");
        auto i=records.size();
        while(i) {
            --i;
            if(records[i].actor==actor) {
                if(i+1!=records.size())records[i]=std::move(records.back());
                records.pop_back();field5a0=0;field564=0;flag4e=true;
            }
        }
        for(auto i=actors.size();i;--i)if(actors[i-1]==actor)actors[i-1]=nullptr;
    }
};

//004CFF70: actual script owners register their borrowed contexts here and must
//unregister before destruction. Empty at cold construction; it creates no
//successful script/context/module substitutes.
template<class Context>
class ActorContextRegistry {
public:
    void add(Context& context) {
        for(const auto* entry:contexts_)if(entry==&context)throw std::logic_error("Script context registered twice");
        contexts_.push_back(&context);
    }
    void remove(Context& context) noexcept {
        for(auto i=contexts_.begin();i!=contexts_.end();++i)if(*i==&context) {contexts_.erase(i);return;}
    }
    template<class Actor> void clear_actor_references(Actor* actor) noexcept {
        static_assert(noexcept(std::declval<Context&>().remove_actor(actor)),"Script cleanup must not throw");
        for(auto i=contexts_.size();i;--i)contexts_[i-1]->remove_actor(actor);
    }
private:
    std::vector<Context*> contexts_;
};

//Entry deletion used by007D6A00/007D75B0:007DD3F0→007DC8A0. A native producer
//transfers each actual matching entry owner into this registry. Release ignores
//unknown handles, swap/pops a known handle, then destroys it through that owner.
template<class EntryOwner>
class ActorEntryRegistry {
public:
    using Entry=decltype(std::declval<EntryOwner&>().get());
    ~ActorEntryRegistry() {clear();}
    void clear() noexcept {
        //A pack destructor can release other registered entries. Remove each
        //owner before invoking it, so recursive releases cannot mutate a vector
        //whose element destructor is still being run by vector destruction.
        while(!entries_.empty()) {
            auto owner=std::move(entries_.back());entries_.pop_back();owner.reset();
        }
    }
    Entry add(EntryOwner owner) {
        if(!owner)throw std::logic_error("Cannot register an empty actor entry");
        const auto entry=owner.get();entries_.push_back(std::move(owner));return entry;
    }
    void release_entry(Entry entry) noexcept {
        static_assert(std::is_nothrow_move_constructible<EntryOwner>::value&&
                      std::is_nothrow_move_assignable<EntryOwner>::value&&
                      noexcept(std::declval<EntryOwner&>().reset()),
                      "Actor entry ownership must not throw during cleanup");
        if(!entry)return;
        for(std::size_t i=0;i!=entries_.size();++i)if(entries_[i].get()==entry) {
            auto owner=std::move(entries_[i]);
            if(i+1!=entries_.size())entries_[i]=std::move(entries_.back());
            entries_.pop_back();owner.reset();return;
        }
    }
    std::size_t size() const {return entries_.size();}
private:
    std::vector<EntryOwner> entries_;
};

} //namespace hd2::game
#endif
