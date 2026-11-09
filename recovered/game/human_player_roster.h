#ifndef HD2_GAME_HUMAN_PLAYER_ROSTER_H
#define HD2_GAME_HUMAN_PLAYER_ROSTER_H
#include "game/text_glyphs.h"
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <stdexcept>
namespace hd2::game {
//004617F0/004619C0/004619F0/00461A20/00461A60/00461AA0/00461C70/
//00462300: one actual list08 of BORROWED Humans. Destruction never deletes them.
template<class Human>class HumanRoster {
public:
    explicit HumanRoster(std::int32_t identity=-1):identity_(identity){}
    void clear()noexcept{entries_.clear();} //Retains allocation/name/identity.
    std::size_t size()const{return entries_.size();}
    std::uint32_t live_count()const { //004622C0, ascending; NULL/dead do not count.
        std::uint32_t count=0;
        for(auto* human:entries_)if(human) {
            std::int32_t health;const auto word=human->properties.stats_ec[0];std::memcpy(&health,&word,4);
            if(health>0)++count;
        }
        return count;
    }
    Human* at(std::int32_t index)const{ //004619F0 unsigned comparison, invalid returns NULL.
        return static_cast<std::uint32_t>(index)<entries_.size()?entries_[static_cast<std::uint32_t>(index)]:nullptr;
    }
    std::int32_t index(Human* human)const{ //00461A60 reverse identity lookup.
        for(std::size_t n=entries_.size();n;)if(entries_[--n]==human)return static_cast<std::int32_t>(n);
        return -1;
    }
    bool contains(Human* human)const{return index(human)!=-1;}
    bool append(Human* human){ //00461AA0 rejects NULL/duplicates before publishing.
        if(!human||contains(human))return false;
        entries_.push_back(human);return true;
    }
    bool remove(Human* human,std::int32_t* removed_index=nullptr){ //00461C70 swap/pop; leave output untouched on failure.
        if(!human)return false;
        const auto n=index(human);if(n<0)return false;
        entries_[static_cast<std::size_t>(n)]=entries_.back();entries_.pop_back();
        if(removed_index)*removed_index=n;
        return true;
    }
    const auto& entries()const{return entries_;}
    std::int32_t identity()const{return identity_;}const auto& name()const{return name_;}
private:
    std::vector<Human*> entries_;std::string name_;std::int32_t identity_;
};

//The actual control38 subobject's owned16-byte records. Feedback must supply
//0049B980/005A8740/005A86F0 and actual Human150/005A6380 when the native gates
//call them. There are no successful default event, HUD or Human receivers.
template<class Human>class HumanSelectionRecords {
public:
    struct Record {Human* human=nullptr;std::int32_t references=1;bool enabled=true;std::uint32_t timer=0;};
    ~HumanSelectionRecords(){clear();}
    void clear()noexcept{ //0045D3A0: reverse retirement, retain list capacity; do NOT reset changed10.
        while(!records_.empty())records_.pop_back();
        active_count_=inactive_count_=0;notify_=false;
    }
    const auto& records()const{return records_;}
    std::int32_t active_count()const{return active_count_;}std::int32_t inactive_count()const{return inactive_count_;}
    bool changed()const{return changed_;}bool& notification_flag(){return notify_;} //Native control54, reset by0045D3A0.
    template<class Feedback>void acquire(Human& human,std::uint32_t mode,Feedback& feedback){ //0045E410.
        if(mode&2u)feedback.hud_selected(human,true); //Before any record lookup/allocation.
        if(notify_&&feedback.human_hidden(human))feedback.show_human(human,200);
        for(auto& owner:records_)if(owner->human==&human){
            owner->timer=0;owner->references=text_add(owner->references,1);
            if(health(human)<=0)deactivate(human);
            return; //Existing positive-health record does NOT set changed10.
        }
        auto owner=std::make_unique<Record>();owner->human=&human;
        records_.insert(records_.begin(),std::move(owner));active_count_=text_add(active_count_,1);
        if(health(human)<=0)deactivate(human);
        if(notify_)feedback.show_human(human,200);
        changed_=true;
    }
    template<class Feedback>void release(Human& human,std::uint32_t mode,Feedback& feedback){ //0045E540.
        if(mode&2u)feedback.hud_selected(human,false);
        if(health(human)<=0||feedback.human_unavailable(human))return;
        for(auto& owner:records_)if(owner->human==&human){
            if(owner->references>0)--owner->references;
            if(!owner->references){owner->timer=10000;if(notify_)feedback.hide_human(human,200);}
            return;
        }
    }
    void deactivate(Human& human){ //0045E3B0, first ascending identity match and native partition swap.
        for(std::size_t n=0;n!=records_.size();++n)if(records_[n]->human==&human){
            records_[n]->enabled=false;active_count_=text_sub(active_count_,1);inactive_count_=text_add(inactive_count_,1);
            if(static_cast<std::int64_t>(n)<active_count_)std::swap(records_[n],records_.at(static_cast<std::size_t>(active_count_)));
            return;
        }
    }
    void tick(std::uint32_t elapsed){ //0045E2F0/0045E2C0, reverse erase with ordered shift, zero timer is immortal.
        changed_=false;
        for(std::size_t n=records_.size();n;){--n;auto& timer=records_[n]->timer;if(!timer)continue;
            timer-=elapsed;std::int32_t remaining;std::memcpy(&remaining,&timer,4);
            if(remaining<=0){timer=0;records_.erase(records_.begin()+static_cast<std::ptrdiff_t>(n));changed_=true;active_count_=text_sub(active_count_,1);}
        }
    }
    void selected(std::vector<Human*>& output)const{ //0045EB30, reverse records; refcount!=0, not enabled byte.
        output.clear();for(std::size_t n=records_.size();n;)if(const auto& record=*records_[--n];record.references!=0)output.push_back(record.human);
    }
private:
    static std::int32_t health(const Human& human){const auto word=human.properties.stats_ec[0];std::int32_t value;std::memcpy(&value,&word,4);return value;}
    std::vector<std::unique_ptr<Record>> records_;std::int32_t active_count_=0,inactive_count_=0;
    bool changed_=false,notify_=false; //Defined cold guards for native unwritten bytes; no ABI constructor credit.
};
//0045D160/0045D1B0 are DISTINCT from first_person_current_player0045D1D0.
//Suppressed queries actual005A63F0; it may change index/list, hence reload them.
template<class Human,class Suppressed>Human* selected_control_player(const HumanRoster<Human>& roster,std::int32_t& index28,
                                                                  std::uint32_t mode,Suppressed&& suppressed){
    const auto required=[&]{
        if(index28==-1)return static_cast<Human*>(nullptr);
        auto* human=roster.at(index28);
        if(!human)throw std::runtime_error("Selected control player exceeds actual roster");
        return human;
    };
    auto* human=required();if(!human)return nullptr;
    if((mode&7u)!=7u&&suppressed(*human))return nullptr;
    return required();
}
template<class Human>Human* previous_control_player(const HumanRoster<Human>& roster,std::int32_t index30){
    if(index30==-1)return nullptr;
    auto* human=roster.at(index30);
    if(!human)throw std::runtime_error("Previous control player exceeds actual roster");
    return human;
}
} //namespace hd2::game
#endif
