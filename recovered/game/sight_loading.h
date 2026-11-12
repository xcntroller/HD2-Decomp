#ifndef HD2_GAME_SIGHT_LOADING_H
#define HD2_GAME_SIGHT_LOADING_H

#include <array>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>
#include <cmath>
#include <cstring>

namespace hd2::game {

//Module0089B178, registration/lifetime and frame scheduling. The two lists contain
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
    //0046F000 scheduler. Queries must supply actual Human/Dog virtual00;
    //the caller supplies SAME process FLOAT00839390 (initial1), not a World
    //copy. This does not replace positive visibility with a successful probe.
    template<class Queries>void tick(std::uint32_t elapsed,float& accumulator,Queries& queries){
        if(delay_){delay_-=elapsed;if(signed_word(delay_)<=0)delay_=0;return;}
        accumulator=static_cast<float>(static_cast<long double>(accumulator)+signed_word(elapsed));
        const auto pairs=static_cast<std::uint32_t>(targets_.size())*static_cast<std::uint32_t>(observers_.size());
        const auto product=signed_word(pairs);
        const auto quotient=(static_cast<long double>(accumulator)*.0010000000474974513f)*product;
        if(!std::isfinite(quotient)||quotient<-9223372036854775808.L||quotient>=9223372036854775808.L)
            throw std::runtime_error("Sight work budget exceeds original signed conversion range");
        auto budget=signed_word(static_cast<std::uint32_t>(static_cast<std::int64_t>(std::trunc(quotient))));
        if(!budget)return;
        accumulator=static_cast<float>(static_cast<long double>(accumulator)-(1000.L/product)*budget);
        if(target_cursor_>=targets_.size())target_cursor_=0;
        const auto first_target=target_cursor_;
        for(;;){
            auto* current=record_at(targets_,static_cast<std::int32_t>(target_cursor_));if(!current)return;
            const auto original_field10=current->field10;
            const auto paired=current->own_observer;
            if(current->mask&&paired!=-1){
                while(observer_cursor_<observers_.size()){
                    auto* observer=record_at(observers_,static_cast<std::int32_t>(observer_cursor_));if(!observer)return;
                    const auto other_target=observer->own_target;
                    if(static_cast<std::uint32_t>(paired)!=observer_cursor_&&observer->mask){
                        current=record_at(targets_,static_cast<std::int32_t>(target_cursor_));if(!current)return;
                        const auto mask=current->field10&observer->mask;
                        const bool marker=next_marker_requires_update(*observer,target_cursor_); //Called even when mask!=0.
                        current=record_at(targets_,static_cast<std::int32_t>(target_cursor_));if(!current)return;
                        if((mask||marker)&&!current->flag29){
                            observer=record_at(observers_,static_cast<std::int32_t>(observer_cursor_));if(!observer)return;
                            queries.sight_observe(*observer,mask,target_cursor_,elapsed);
                        }
                        if(other_target!=-1){
                            auto* reciprocal=record_at(observers_,paired);if(!reciprocal)return;
                            const auto reciprocal_mask=reciprocal->mask;
                            (void)next_marker_requires_update(*reciprocal,static_cast<std::uint32_t>(other_target)); //Native ignores this result.
                            const auto* target=record_at(targets_,other_target);
                            if(!target)throw std::runtime_error("Sight reciprocal target is absent");
                            if(reciprocal_mask&&!target->flag29){
                                reciprocal=record_at(observers_,paired);if(!reciprocal)return;
                                queries.sight_observe(*reciprocal,reciprocal_mask,static_cast<std::uint32_t>(other_target),elapsed);
                            }
                        }
                    }
                    ++observer_cursor_;
                    if(budget<=0)return; //Native tests BEFORE decrement: budget1 permits a second iteration.
                    budget=signed_word(static_cast<std::uint32_t>(budget)-1);
                }
                observer_cursor_=0;
            }
            ++target_cursor_;if(target_cursor_>=targets_.size())target_cursor_=0;
            if(target_cursor_==first_target)return;
            if(!original_field10){
                const auto spent=observer_cursor_-static_cast<std::uint32_t>(observers_.size());
                observer_cursor_=0;budget=signed_word(static_cast<std::uint32_t>(budget)+spent);
                if(budget<=0)return;
            }
        }
    }
    struct HumanProbe {bool visible,deferred;};
    using Vector=std::array<float,4>;
    struct RayHit {std::uint32_t flags;Actor actor;};
    //0046DF80: advance ONE actual marker. Human deadlines use an unsigned
    //comparison; a future deadline returns the old aggregate and suppresses
    //feedback. Shared exclusions bracket the ray, not the marker/time writes.
    template<class Queries>HumanProbe probe_human(Observer& observer,std::uint32_t index,
            std::uint32_t elapsed,Queries& queries){
        auto* target=live_target(static_cast<std::int32_t>(index));if(!target)return {false,false};
        auto& relation=required_relation(observer,static_cast<std::int32_t>(index));const auto before=relation.words[0];
        const auto marker=advance_marker(relation,*target);
        if(queries.sight_time()<relation.words[16+marker])return {before!=0,true};
        relation.words[16+marker]=0;
        const auto frame=target->markers.at(marker);const auto actor=receivers_.sight_actor(target->frame);
        if(!actor)throw std::runtime_error("Sight Human probe lost its actual target actor");
        auto interval=std::uint32_t{0};const auto score=word_float(relation.words[6+marker]);
        if(score==0)interval=elapsed;
        if(score>0){interval=queries.sight_time()-relation.words[11+marker];relation.words[11+marker]=queries.sight_time();}
        relation.words[16+marker]=queries.sight_time()+interval;
        const auto origin=queries.sight_human_origin(observer.actor); //Head260 is required even when marker is NULL.
        if(!frame)return {false,false};
        const auto displacement=marker_displacement(queries.sight_position(frame),origin);
        auto layers=target->enabled?0u:2u;const auto kind=target->kind;
        Actor match{};
        const auto restore=[&]{if(kind==0)queries.sight_restore_human(actor);else if(kind==1)queries.sight_restore_dog();};
        //The source boundary restores the SAME shared exclusion owner on a
        //query exception, preserving the first failure during partial teardown.
        const auto hit=[&]{try{
                if(kind==0){queries.sight_exclude_human(actor);match=queries.sight_human_attachment(actor);}
                else if(kind==1){queries.sight_exclude_dog(actor);layers|=2u;}
                return queries.sight_ray(origin,displacement,layers|0x2cc100u,0x100000u);}
            catch(...){try{restore();}catch(...){}throw;}}();
        restore();
        if(!hit||(match&&(hit->flags&0x40u)&&hit->actor==match))return {true,false};
        required_relation(observer,static_cast<std::int32_t>(index)).words[6+marker]=0;return {false,false};
    }
    //0046E3B0: Dog deadline subtraction is SIGNED, and visibility requires an
    //actual target-actor hit. The original NULL544 path reads model position
    //then still dereferences544; the real receiver must reject that bad state,
    //not turn the model read into a successful head-marker fallback.
    template<class Queries>bool probe_dog(Observer& observer,std::uint32_t index,
            std::uint32_t elapsed,Queries& queries){
        auto* target=live_target(static_cast<std::int32_t>(index));if(!target)return false;
        auto& relation=required_relation(observer,static_cast<std::int32_t>(index));
        const auto marker=advance_marker(relation,*target);
        if(signed_word(relation.words[16+marker]-queries.sight_time())>0)return false;
        relation.words[16+marker]=0;
        const auto frame=target->markers.at(marker);const auto actor=receivers_.sight_actor(target->frame);
        if(!actor)throw std::runtime_error("Sight Dog probe lost its actual target actor");
        const auto score=word_float(relation.words[6+marker]);std::uint32_t interval;
        if(score==0)interval=elapsed;
        else if(score>0){interval=queries.sight_time()-relation.words[11+marker];relation.words[11+marker]=queries.sight_time();}
        else throw std::runtime_error("Sight Dog interval is unwritten for a negative or unordered cached score");
        if(target->kind>1)return false;
        auto metric=score;queries.sight_dog_marker_metric(observer.actor,actor,interval,metric); //Actual005D64C0.
        auto& fresh=required_relation(observer,static_cast<std::int32_t>(index));
        fresh.words[6+marker]=float_word(metric);fresh.words[16+marker]=queries.sight_time()+interval;
        if(metric!=100.f)return false;
        const auto origin=queries.sight_dog_origin(observer.actor);if(!origin||!frame)return false;
        const auto displacement=marker_displacement(queries.sight_position(frame),*origin);
        target=live_target(static_cast<std::int32_t>(index));if(!target)throw std::runtime_error("Sight Dog probe lost its target after metric callback");
        const auto hit=queries.sight_ray(*origin,displacement,(target->enabled?0u:2u)|0x208000u,0x100000u);
        if(hit&&(hit->flags&0x40u)&&hit->actor==actor)return true;
        required_relation(observer,static_cast<std::int32_t>(index)).words[6+marker]=0;return false;
    }
    //Human0046DCF0 (absent frozen) and Dog0046DEA0 observer virtual00.
    //Queries supplies the actual B0 metric and marker/ray receivers, which
    //may mutate SAME relation fields. No clear-ray or score100 fallback.
    template<class Queries>void observe(Observer& observer,std::uint32_t,std::uint32_t index,
            std::uint32_t elapsed,Queries& queries){
        if(observer.flag24)return;
        auto& relation=required_relation(observer,static_cast<std::int32_t>(index));if(relation.mode()!=2)return;
        const auto before=relation.words[0];relation.words[2]=relation.words[1];
        const auto kind=receivers_.sight_actor_type(observer.actor);
        if(kind==3){
            auto* target=live_target(static_cast<std::int32_t>(index));if(!target)throw std::runtime_error("Sight Human observer lost its actual target");
            const auto actor=receivers_.sight_actor(target->frame);if(!actor)throw std::runtime_error("Sight target has no actual actor");
            const auto target_kind=receivers_.sight_actor_type(actor);std::optional<float> score;
            if(target_kind==3){if(queries.sight_human_health(actor)<=0&&before)return;score=queries.sight_human_metric(observer.actor,actor);}
            else if(target_kind==30)score=queries.sight_dog_metric(observer.actor,actor);
            else return;
            if(!score)throw std::runtime_error("Sight observer would consume an unwritten native metric score");
            if(*score!=100.f){
                if(before)receivers_.human_sight_changed(observer.actor,true,index); //Native notifies BEFORE clearing fields.
                auto& fresh=required_relation(observer,static_cast<std::int32_t>(index));fresh.words[0]=fresh.words[1]=0;fresh.words[3]&=0xffffff00u;return;
            }
            const HumanProbe result=queries.sight_human_probe(observer,index,elapsed);
            auto& fresh=required_relation(observer,static_cast<std::int32_t>(index));const auto bit=1u<<(fresh.words[4]&31u);
            if(result.visible){fresh.words[0]|=bit;if(fresh.words[0]!=before)++fresh.words[1];fresh.words[3]=(fresh.words[3]&0xffffff00u)|1u;}
            else{fresh.words[0]&=~bit;if(fresh.words[0]!=before)--fresh.words[1];fresh.words[3]&=0xffffff00u;}
            if(result.deferred)return;
            if(before&&!fresh.words[0])receivers_.human_sight_changed(observer.actor,true,index);
            else if(!before&&fresh.words[0])receivers_.human_sight_changed(observer.actor,false,index);
        }else if(kind==30){
            const bool visible=queries.sight_dog_probe(observer,index,elapsed);
            auto& fresh=required_relation(observer,static_cast<std::int32_t>(index));const auto bit=1u<<(fresh.words[4]&31u);
            if(visible){fresh.words[0]|=bit;fresh.words[3]=(fresh.words[3]&0xffffff00u)|1u;}
            else{fresh.words[0]&=~bit;fresh.words[3]&=0xffffff00u;}
            if(before&&!fresh.words[0])receivers_.dog_sight_changed(observer.actor,true,index);
            else if(!before&&fresh.words[0])receivers_.dog_sight_changed(observer.actor,false,index);
        }else throw std::runtime_error("Sight observer has an unsupported actual actor family");
    }
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
    std::uint32_t human_relation_mask(std::int32_t observer_index,std::int32_t target_index){ //0046F2A0→0046DC10.
        auto& observer=required_observer(observer_index);
        if(receivers_.sight_actor_type(observer.actor)!=3||!live_target(target_index))
            throw std::runtime_error("Sight mask requires an actual Human observer and live target");
        const auto& relation=required_relation(observer,target_index);
        return relation.mode()==0?0x1fu:relation.state();
    }
    std::uint32_t human_relation_feedback(std::int32_t observer_index,std::int32_t target_index,
            std::uint32_t* previous,bool* visible){ //0046F320→Human virtual0C0046DB60 (latter absent frozen).
        auto& observer=required_observer(observer_index);
        if(receivers_.sight_actor_type(observer.actor)!=3)throw std::runtime_error("Sight feedback query requires actual Human observer virtual0C");
        const auto& relation=required_relation(observer,target_index);
        if(relation.mode()==0){
            const auto* target=live_target(target_index);if(!target)throw std::runtime_error("Sight mode0 feedback requires actual target marker count");
            const auto count=static_cast<std::uint32_t>(target->markers.size());if(previous)*previous=count;if(visible)*visible=true;return count;
        }
        if(previous)*previous=relation.words[2];
        if(visible)*visible=(relation.words[3]&0xffu)!=0;
        return relation.words[1];
    }
    //0046F280→Human observer0046D8C0: immediate all-marker acquisition, no cached successful ray.
    template<class Queries>bool force_human_target(std::int32_t observer_index,std::uint32_t index,Queries& queries){
        auto* target=live_target(static_cast<std::int32_t>(index));if(!target||target->flag29)return false;
        auto& observer=required_observer(observer_index);if(receivers_.sight_actor_type(observer.actor)!=3)throw std::logic_error("Forced Sight requires actual Human observer");
        if(required_relation(observer,static_cast<std::int32_t>(index)).words[0])return true;
        const auto origin=queries.sight_human_origin(observer.actor);const auto actor=receivers_.sight_actor(target->frame);
        if(actor==observer.actor)return false;
        const bool human=target->kind==0;Actor attachment{};
        const auto restore=[&]{if(human)queries.sight_restore_human(actor);};
        try{
            if(human){
                queries.sight_exclude_human(actor);attachment=queries.sight_human_attachment(actor);
                const auto metric=queries.sight_human_metric(observer.actor,actor);
                if(!metric||*metric!=100.f){restore();return false;}
            }
            target=live_target(static_cast<std::int32_t>(index));if(!target)throw std::logic_error("Forced Sight lost actual target after metric");
            for(auto n=target->markers.size();n;){const auto marker=--n;const auto frame=target->markers[marker];
                const auto delta=marker_displacement(queries.sight_position(frame),origin);
                target=live_target(static_cast<std::int32_t>(index));if(!target)throw std::logic_error("Forced Sight lost actual target before ray");
                const auto hit=queries.sight_ray(origin,delta,(target->enabled?0u:2u)|0x2cc100u,0x100000u);
                if(!hit||(attachment&&(hit->flags&0x40u)&&hit->actor==attachment)){
                    auto& relation=required_relation(observer,static_cast<std::int32_t>(index));relation.words[4]=static_cast<std::uint32_t>(marker);relation.words[0]|=1u<<(marker&31u);++relation.words[1];
                    receivers_.human_sight_changed(observer.actor,false,index);restore();return true;
                }
                target=live_target(static_cast<std::int32_t>(index));if(!target)throw std::logic_error("Forced Sight lost actual target after ray");
            }
        }catch(...){try{restore();}catch(...){}throw;}
        restore();return false;
    }
    Target* target(std::int32_t index){return live_target(index);}
    const auto& observers()const{return observers_;}const auto& targets()const{return targets_;}
private:
    static std::int32_t signed_word(std::uint32_t bits){std::int32_t value;std::memcpy(&value,&bits,4);return value;}
    static float word_float(std::uint32_t bits){float value;std::memcpy(&value,&bits,4);return value;}
    static std::uint32_t float_word(float value){std::uint32_t bits;std::memcpy(&bits,&value,4);return bits;}
    static std::uint32_t advance_marker(Relation& relation,const Target& target){
        ++relation.words[4];if(relation.words[4]==target.markers.size())relation.words[4]=0;
        if(relation.words[4]>=target.markers.size()||relation.words[4]>=5)
            throw std::runtime_error("Sight probe marker cursor exceeds its actual relation record");
        return relation.words[4];
    }
    static Vector marker_displacement(const Vector& point,const Vector& origin){
        Vector result{};for(unsigned n=0;n!=3;++n)result[n]=static_cast<float>(static_cast<long double>(point[n])-origin[n]);return result;
    }
    bool next_marker_requires_update(Observer& observer,std::uint32_t index){ //0046D840/0046DC70, both absent frozen.
        const auto* target=live_target(static_cast<std::int32_t>(index));if(!target)return false;
        auto& relation=required_relation(observer,static_cast<std::int32_t>(index));auto next=relation.words[4]+1;
        if(next==target->markers.size())next=0;
        if(next>=target->markers.size()||6u+next>=relation.words.size())throw std::runtime_error("Sight marker cursor exceeds its actual relation record");
        return relation.words[6+next]!=0x42c80000u;
    }
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
    std::uint32_t observer_cursor_=0,target_cursor_=0,delay_=0; //Native2C/30/34;0046EFD0 supplies actual loading state.
};
} //namespace hd2::game
#endif
