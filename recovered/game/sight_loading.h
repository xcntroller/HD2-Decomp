#ifndef HD2_GAME_SIGHT_LOADING_H
#define HD2_GAME_SIGHT_LOADING_H

#include <array>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace hd2::game {

//Module0089B178, loading registration/lifetime only. The two lists contain
//owned records and BORROWED actors/frames. Actual relationship and Human/Dog
//feedback receivers are required; no simulation/visibility test is supplied.
template<class Frames,class Receivers,
    class ActorType=decltype(std::declval<Receivers&>().sight_actor(std::declval<typename Frames::Frame>()))>
class SightLoading {
public:
    using Frame=typename Frames::Frame;
    using Actor=ActorType;
    struct Relation { //Native104 bytes, only known reset fields are interpreted.
        std::array<std::uint32_t,26> words{};
        void reset(std::uint32_t mode){words[0]=words[1]=words[2]=words[4]=0;words[3]&=0xffffff00u;words[5]=mode;}
        std::uint32_t state()const{return words[0];}
        std::uint32_t mode()const{return words[5];}
    };
    struct Observer {
        Frame frame{};Actor actor{};std::uint32_t mask=0,index=0;
        std::int32_t own_target=-1;std::vector<Relation> relations;bool flag24=false;
        bool reset_relation(std::uint32_t target,std::uint32_t mode) { //0046D600; target==size appends.
            if(target>0x7fffffffu||target>relations.size())return false;
            if(target==relations.size()){Relation value;value.reset(mode);relations.push_back(value);}
            else relations[target].reset(mode);
            return true;
        }
    };
    struct Target {
        Frame frame{};Actor actor{};std::uint32_t mask=0,index=0,field10=0;
        std::int32_t own_observer=-1;std::vector<Frame> markers;
        bool enabled=false,flag29=false;unsigned kind=0;
    };
    SightLoading(Frames& frames,Receivers& receivers):frames_(frames),receivers_(receivers){}
    ~SightLoading(){clear();}
    SightLoading(const SightLoading&)=delete;SightLoading& operator=(const SightLoading&)=delete;
    void initialize_loading(bool authority) { //0046EFD0.
        if(authority){observer_cursor_=target_cursor_=0;delay_=4000;}
    }
    void close_loading(bool authority){if(authority)clear();} //0046EFF0.
    void clear() noexcept { //0046FFB0: observers reverse, then targets reverse, retaining list capacity.
        while(!observers_.empty())observers_.pop_back();
        while(!targets_.empty())targets_.pop_back();
    }
    std::int32_t add_observer(Frame frame,std::uint32_t mask) { //0046F350.
        const auto slot=reuse_slot(observers_,frame);if(slot<0||!frame)return -1;
        const auto actor=receivers_.sight_actor(frame);if(!actor)return -1;
        const auto type=receivers_.sight_actor_type(actor);if(type!=3&&type!=30)return -1;
        auto record=std::make_unique<Observer>();record->frame=receivers_.sight_actor_model(actor);
        record->actor=actor;record->mask=mask;record->index=static_cast<std::uint32_t>(slot);
        publish(observers_,slot,std::move(record));
        for(std::size_t n=0;n<targets_.size();++n) {
            auto* target=targets_[n].get();if(!target)continue;
            auto& observer=*observers_.at(static_cast<std::size_t>(slot));
            if(!target->frame){observer.reset_relation(static_cast<std::uint32_t>(n),1);continue;}
            if(target->frame==frame){target->own_observer=slot;observer.own_target=static_cast<std::int32_t>(n);}
            const auto other=receivers_.sight_actor(target->frame);
            if(!other){observer.reset_relation(static_cast<std::uint32_t>(n),2);continue;}
            seed(observer,actor,other,static_cast<std::uint32_t>(n));
        }
        return slot;
    }
    std::int32_t add_target(Frame frame,std::uint32_t mask) { //0046F9D0 accepts1..15 only.
        if(!mask||mask>15)return -1;
        const auto slot=reuse_slot(targets_,frame);if(slot<0)return -1;
        const auto actor=frame?receivers_.sight_actor(frame):Actor{};
        std::unique_ptr<Target> record;
        if(actor) {
            const auto type=receivers_.sight_actor_type(actor);
            if(type==3||type==30) {
                record=std::make_unique<Target>();record->frame=receivers_.sight_actor_model(actor);
                record->actor=actor;record->mask=mask;record->index=static_cast<std::uint32_t>(slot);record->kind=type==3?0:1;
                bind_markers(*record);
            }
        }
        //Native publishes a NULL target for missing/unsupported actor data and
        //still returns its index. Such records are not silently active targets.
        publish(targets_,slot,std::move(record));
        for(auto n=observers_.size();n;) {
            auto* observer=observers_[--n].get();if(!observer||!observer->mask)continue;
            auto* target=targets_[static_cast<std::size_t>(slot)].get();
            if(observer->frame==frame) {
                if(target)target->own_observer=static_cast<std::int32_t>(n);
                observer->own_target=slot;
            }
            if(!target)continue;
            const auto owner=observer->frame?receivers_.sight_actor(observer->frame):Actor{};
            if(!owner)throw std::runtime_error("Live Sight observer lost its actual actor");
            seed(*observer,owner,actor,static_cast<std::uint32_t>(slot));
        }
        return slot;
    }
    void target_mode(std::int32_t index,unsigned operation) { //0046F700; incidental EAX is not status.
        auto* target=live_target(index);if(!target)return;
        switch(operation) {
        case 0:target->enabled=true;break;
        case 1:target->enabled=false;break;
        case 2:
            for(auto n=observers_.size();n;) {
                auto* observer=observers_[--n].get();if(!observer||!observer->mask)continue;
                auto& relation=required_relation(*observer,index);
                if(relation.mode()==0) {
                    relation.words[5]=2;
                    if(observer->frame&&observer->actor&&receivers_.sight_actor_type(observer->actor)==3&&index!=observer->own_target)
                        receivers_.human_sight_changed(observer->actor,true,static_cast<std::uint32_t>(index));
                }
            }
            break;
        case 3:
            target->flag29=true;
            for(auto n=observers_.size();n;) {
                auto* observer=observers_[--n].get();if(!observer||!observer->mask||!observer->frame)continue;
                auto& relation=required_relation(*observer,index);
                if(relation.state()&&observer->actor&&receivers_.sight_actor_type(observer->actor)==3)
                    receivers_.human_sight_changed(observer->actor,true,static_cast<std::uint32_t>(index));
                relation.words[0]=relation.words[1]=0;
            }
            break;
        case 4:target->flag29=false;break;
        default:break;
        }
    }
    void observer_enabled(std::int32_t index,bool enabled) { //0046F8C0; incidental EAX is not status.
        auto* value=observer(index);if(!value)return;
        value->flag24=!enabled;
        if(enabled)return;
        for(auto n=value->relations.size();n;) {
            const auto target=static_cast<std::uint32_t>(--n);
            if(value->relations[n].state()&&value->frame) {
                //Native resolves model EC again for every callback. A callback
                //can change perception; clear this relation AFTER notification.
                const auto actor=receivers_.sight_actor(value->frame);
                if(actor&&receivers_.sight_actor_type(actor)==3)
                    receivers_.human_sight_changed(actor,true,target);
            }
            value->relations[n].words[0]=value->relations[n].words[1]=0;
        }
    }
    bool remove_observer(std::int32_t index) { //0046FD50 retains the slot and relation allocation.
        auto* observer=record_at(observers_,index);if(!observer)return false;
        observer->mask=0;observer->frame=Frame{};
        if(observer->own_target!=-1){required_target(observer->own_target).own_observer=-1;observer->own_target=-1;}
        observer->relations.clear();return true;
    }
    bool remove_target(std::int32_t index) { //0046FDF0 callbacks precede target record invalidation.
        if(index<0||static_cast<std::size_t>(index)>=targets_.size())return false;
        for(auto n=observers_.size();n;) {
            auto* observer=observers_[--n].get();if(!observer||!observer->mask||!observer->frame)continue;
            auto& relation=required_relation(*observer,index);
            if(relation.state()) {
                relation.words[0]=relation.words[1]=0;
                if(observer->actor&&receivers_.sight_actor_type(observer->actor)==3)
                    receivers_.human_sight_changed(observer->actor,true,static_cast<std::uint32_t>(index));
            }
        }
        auto* target=record_at(targets_,index);if(!target)return false;
        target->mask=target->field10=0;target->frame=Frame{};
        if(target->own_observer!=-1){required_observer(target->own_observer).own_target=-1;target->own_observer=-1;}
        target->markers.clear();return true;
    }
    Observer* observer(std::int32_t index){auto* value=record_at(observers_,index);return value&&value->mask?value:nullptr;} //0046EFA0.
    Target* target(std::int32_t index){return live_target(index);}
    const auto& observers()const{return observers_;}const auto& targets()const{return targets_;}
private:
    template<class Record>static Record* record_at(const std::vector<std::unique_ptr<Record>>& records,std::int32_t index) {
        return index>=0&&static_cast<std::size_t>(index)<records.size()?records[static_cast<std::size_t>(index)].get():nullptr;
    }
    template<class Record>static std::int32_t reuse_slot(const std::vector<std::unique_ptr<Record>>& records,Frame frame) {
        for(auto n=records.size();n;)if(const auto* record=records[--n].get()) {
            if(record->frame==frame)return -1;
            if(!record->mask)return static_cast<std::int32_t>(n); //Stops BEFORE looking for duplicates below this free record.
        }
        if(records.size()>=static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()))throw std::length_error("Sight index exceeds native signed range");
        return static_cast<std::int32_t>(records.size());
    }
    template<class Record>static void publish(std::vector<std::unique_ptr<Record>>& records,std::int32_t slot,std::unique_ptr<Record> record) {
        if(static_cast<std::size_t>(slot)==records.size())records.push_back(std::move(record));
        else records[static_cast<std::size_t>(slot)]=std::move(record);
    }
    Observer& required_observer(std::int32_t index){auto* record=record_at(observers_,index);if(!record)throw std::runtime_error("Sight observer cross-link is absent");return *record;}
    Target& required_target(std::int32_t index){auto* record=record_at(targets_,index);if(!record)throw std::runtime_error("Sight target cross-link is absent");return *record;}
    Target* live_target(std::int32_t index){auto* record=record_at(targets_,index);return record&&record->mask?record:nullptr;}
    static Relation& required_relation(Observer& observer,std::int32_t target) {
        if(target<0||static_cast<std::size_t>(target)>=observer.relations.size())throw std::runtime_error("Sight relation index has no native record");
        return observer.relations[static_cast<std::size_t>(target)];
    }
    void bind_markers(Target& target) { //0046EB20/0046ECF0; actual short-name enumeration, mask400.
        constexpr std::array<const char*,5> human{{"d","z","i","m","q"}};
        constexpr std::array<const char*,4> dog{{"c","f","k","o"}};
        const auto count=target.kind?dog.size():human.size();target.markers.reserve(count);
        for(std::size_t n=0;n!=count;++n)target.markers.push_back(frames_.human_sight_marker(target.frame,target.kind?dog[n]:human[n]));
        if(!target.kind&&!target.markers[1])target.markers[1]=frames_.actor_find(target.frame,"f",0x400);
    }
    void seed(Observer& observer,Actor actor,Actor target,std::uint32_t index) {
        if(actor==target){observer.reset_relation(index,0);return;}
        const auto first=receivers_.sight_actor_type(actor),second=receivers_.sight_actor_type(target);
        bool friendly=false,hostile=false;
        if(first==3&&(second==3||second==30)) {
            friendly=receivers_.human_sight_friendly(actor,target);
            hostile=receivers_.human_sight_hostile(actor,target); //005A0980 issues a SECOND friendship query.
            if(second==3) {
                const auto a=receivers_.sight_human_kind(actor),b=receivers_.sight_human_kind(target);
                if((a==1||b==1)&&(a==2||b==2)){friendly=false;hostile=true;}
            }
        }else if(first==30) {
            if(second==30)friendly=true;
            else if(second==3){const auto kind=receivers_.sight_human_kind(target);friendly=kind==2||kind==3;hostile=kind==1||kind==4||kind==5;}
        }
        observer.reset_relation(index,friendly?0:hostile?2:1);
        if(friendly) {
            if(first==3)receivers_.human_sight_changed(actor,false,index);
            else if(first==30)receivers_.dog_sight_changed(actor,false,index);
        }
    }
    Frames& frames_;Receivers& receivers_;
    std::vector<std::unique_ptr<Target>> targets_;std::vector<std::unique_ptr<Observer>> observers_;
    std::uint32_t observer_cursor_=0,target_cursor_=0,delay_=0; //Cold guards;0046EFD0 supplies actual loading state.
};
} //namespace hd2::game
#endif
