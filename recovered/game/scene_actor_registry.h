#ifndef HD2_GAME_SCENE_ACTOR_REGISTRY_H
#define HD2_GAME_SCENE_ACTOR_REGISTRY_H

#include <array>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>
#include "frontend/cleanup.h"

namespace hd2::game {

//004759C0's Scene2 construction/identity lists, immediate00475C50 removal and
//00475EC0 active-list changes and00476680/00476860 retirement ownership.
//Forward simulation visits defer activity changes; the separate runtime registry
//uses reverse traversal and reverse draining instead.
//Backend supplies matching concrete ActorOwner factories and atomic frame-data
//binding/nonthrowing unbinding. Actor exposes base() with00610070 state. Factory
//registration deliberately precedes the caller's separate +2C initialization.
template<class Backend>
class SceneActorRegistry {
public:
    using Frame=typename Backend::Frame;
    using ActorOwner=typename Backend::ActorOwner;
    using Actor=typename ActorOwner::element_type;
    explicit SceneActorRegistry(Backend& backend) : backend_(backend),identities_(32,nullptr) {generations_.fill(0xff);}
    ~SceneActorRegistry() {clear();}
    SceneActorRegistry(const SceneActorRegistry&)=delete;
    SceneActorRegistry& operator=(const SceneActorRegistry&)=delete;
    Actor* create(std::uint32_t type,Frame frame,bool active=true,std::int32_t replace=-1) {
        if(clearing_)throw std::logic_error("Actor creation during registry cleanup");
        if(type>=39)return nullptr;
        if(!backend_.has_scene_actor_factory(type))return nullptr;
        if(replace<-1||replace>=32)throw std::runtime_error("Scene actor replacement requires a reserved identity");
        auto owner=backend_.make_scene_actor(type); //Real matching native constructor, or null.
        if(replace>=0&&identities_[std::size_t(replace)])remove(identities_[std::size_t(replace)]);
        if(!owner)return nullptr;
        if(identity_count_>=std::size_t(std::numeric_limits<std::int32_t>::max())||
           active_.size()>=std::size_t(std::numeric_limits<std::int32_t>::max()))
            throw std::runtime_error("Scene actor identity count exceeds bounds");
        //Acquire native vector capacity before publishing an opaque frame handle.
        owners_.reserve(owners_.size()+1);
        if(replace<0&&identity_count_==identities_.size())identities_.reserve(identity_count_+1);
        if(active)active_.reserve(active_.size()+1);
        auto* actor=owner.get();
        if(frame)backend_.bind_scene_actor_frame(frame,actor);
        const auto index=replace<0?identity_count_:std::size_t(replace);
        actor->base().id10(static_cast<std::int32_t>(index));
        if(index==identities_.size())identities_.push_back(actor);else identities_[index]=actor;
        if(replace<0)++identity_count_;else ++generations_[index];
        owners_.push_back(Record{std::move(owner),frame});
        if(active) {
            actor->base().id14(static_cast<std::int32_t>(active_.size()));
            active_.push_back(actor);
        }
        return actor;
    }
    void active(Actor* actor,bool value) {
        if(!actor)return;
        auto& base=actor->base();
        if((value&&base.id14()>=0)||(!value&&base.id14()<0))return;
        if(visiting_){toggles_.push_back(actor);return;} //00475EC0 queues the pointer, NOT the desired state.
        if(value&&base.id14()<0) {
            if(active_.size()>=std::size_t(std::numeric_limits<std::int32_t>::max()))throw std::runtime_error("Scene active count exceeds bounds");
            active_.push_back(actor);base.id14(static_cast<std::int32_t>(active_.size()-1));
        } else if(!value&&base.id14()>=0)remove_active(actor);
    }
    void remove(Actor* actor) noexcept {
        if(!actor)return;
        static_assert(noexcept(std::declval<Backend&>().unbind_scene_actor_frame(std::declval<Frame>())),
                      "Actor frame-data unbinding must not throw");
        for(std::size_t i=0;i!=owners_.size();++i)if(owners_[i].owner.get()==actor) {
            unlink_identity(actor);
            const auto frame=owners_[i].frame;
            if(frame)backend_.unbind_scene_actor_frame(frame);
            auto owner=std::move(owners_[i].owner);
            if(i+1!=owners_.size())owners_[i]=std::move(owners_.back());
            owners_.pop_back();owner.reset();return;
        }
    }
    template<class Retire>void retire(Actor* actor,Retire&& retire_actor,std::uint32_t milliseconds=20000) { //00475C50(false)→00476680.
        if(!actor)return;
        if(clearing_)throw std::logic_error("Actor retirement during registry cleanup");
        for(std::size_t index=0;index!=owners_.size();++index)if(owners_[index].owner.get()==actor) {
            pending_.reserve(pending_.size()+1); //Before removing active/identity publication.
            unlink_identity(actor);
            auto record=std::move(owners_[index]);
            if(index+1!=owners_.size())owners_[index]=std::move(owners_.back());
            owners_.pop_back();pending_.push_back(Pending{std::move(record),milliseconds});
            //005A7FE0 is a real Human/controller/Sight/sound/effect consumer,
            //called AFTER append; a thrown endpoint retains the pending owner.
            retire_actor(*actor);return;
        }
    }
    template<class Mode,class ClearRecord,class Deactivate,class Presentation>
    void flush_retired(std::uint32_t elapsed,Mode&& mode,ClearRecord&& clear_record,
                       Deactivate&& deactivate_actor,Presentation&& presentation) { //00476860.
        frontend::CleanupErrors errors;
        for(std::size_t index=0;index<pending_.size();) {
            pending_[index].milliseconds-=elapsed; //Native32-bit wrapping subtraction, then SIGNED test.
            if(!(pending_[index].milliseconds&0x80000000u)){++index;continue;}
            //Keep the owner published while callbacks find its base. Queue
            //growth moves unique_ptrs, not actors; re-fetch by index afterwards.
            auto* actor=pending_[index].record.owner.get();
            errors.attempt([&]{if(mode()&1)clear_record(*actor);}); //Actual locked recordA2=0.
            errors.attempt([&]{if(mode()&2)deactivate_actor(*actor,false);});
            errors.attempt([&]{if(mode()&1)deactivate_actor(*actor,true);});
            errors.attempt([&]{if(mode()&2)presentation(*actor);}); //0049EF70(1,actor).
            remove_active(actor);
            auto record=std::move(pending_[index].record);
            if(record.frame)backend_.unbind_scene_actor_frame(record.frame); //Native lifetime guard before destruction.
            record.owner.reset();
            if(index+1!=pending_.size())pending_[index]=std::move(pending_.back());
            pending_.pop_back(); //Revisit the swapped record, including its elapsed subtraction.
        }
        errors.rethrow();
    }
    void clear() noexcept {
        if(visiting_) {backend_.record_actor_cleanup_failure(std::make_exception_ptr(
            std::logic_error("Scene actor cleanup during traversal")));return;}
        clearing_=true;
        //00475FB0 visits the original physical identity array in ascending order.
        //Swap/pop writes earlier identities but leaves the old tail pointers in
        //that array until cleanup ends. Keep physical storage distinct from its
        //logical size so no native vector access occurs past a popped end.
        const auto count=identity_count_;
        for(std::size_t i=0;i!=count;++i)if(identities_[i])remove(identities_[i]);
        while(!pending_.empty()) {
            auto record=std::move(pending_.back().record);pending_.pop_back();
            if(record.owner) {
                remove_active(record.owner.get());
                if(record.frame)backend_.unbind_scene_actor_frame(record.frame);
            }
            record.owner.reset();
        }
        identities_.resize(32);for(auto& actor:identities_)actor=nullptr;
        identity_count_=32;active_.clear();toggles_.clear();generations_.fill(0xff);clearing_=false;
    }
    Actor* identity(std::size_t index) const {return index<identity_count_?identities_[index]:nullptr;}
    Actor* command_identity(std::int32_t index)const { //00475D50.
        if(index<0||static_cast<std::size_t>(index)>=identity_count_)return nullptr;
        auto* current=identities_[static_cast<std::size_t>(index)];
        if(!current&&index<32)for(auto n=pending_.size();n;){
            auto* actor=pending_[--n].record.owner.get();
            if(actor&&actor->base().id10()==index)return actor;
        }
        return current;
    }
    Actor* human_identity(std::int32_t index)const { //00475E00; bounds BEFORE pending lookup, unlike ordinary00475DD0.
        if(index<0||static_cast<std::size_t>(index)>=identity_count_)return nullptr;
        if(auto* actor=identities_[static_cast<std::size_t>(index)];actor&&actor->base().type()==3)return actor;
        for(auto n=pending_.size();n;)if(auto* actor=pending_[--n].record.owner.get();actor&&actor->base().id10()==index)return actor;
        return nullptr;
    }
    std::size_t identity_count() const {return identity_count_;}
    const std::vector<Actor*>& active_actors() const {return active_;}
    template<class Visit>void visit_active(Visit&& visit) { //00476080/+18 and00476160/+14, FORWARD.
        if(visiting_||clearing_)throw std::logic_error("Nested Scene2 actor traversal");
        visiting_=true;frontend::CleanupErrors errors;
        errors.attempt([&]{
            const auto count=active_.size();
            for(std::size_t n=0;n!=count;++n) {
                if(n>=active_.size()||!active_[n])throw std::runtime_error("Scene2 active actor removed during traversal");
                visit(*active_[n]); //Reload vector after callbacks; a new actor may reallocate it.
            }
        });
        visiting_=false;
        //Native captures this count separately AFTER visiting. Repeated queued
        //pointers toggle repeatedly; requests are neither deduplicated nor last-wins.
        const auto count=toggles_.size();
        for(std::size_t n=0;n!=count;++n) {
            auto* actor=toggles_[n];
            if(owns(actor))errors.attempt([&]{active(actor,actor->base().id14()==-1);});
        }
        toggles_.clear();errors.rethrow();
    }
    template<class Services>void summarize_humans(Services& services) { //00476A50.
        if((services.mode_flags()&7u)!=7u&&!services.human_remote_ai_enabled())return;
        field4c_=field50_=0;single_human54_=nullptr;
        const auto count=identity_count_;
        for(std::size_t n=0;n!=count;++n)if(auto* actor=identities_[n];actor&&actor->base().type()==3) {
            const auto summary=services.human_frame_summary(*actor);
            if(!summary.eligible)continue;
            if(!summary.suspended)++field4c_;
            if(summary.selected){++field50_;single_human54_=actor;}
        }
        if(field50_!=1)single_human54_=nullptr;
    }
    std::uint32_t available_human_count()const{return field4c_;}
    std::uint32_t selected_human_count()const{return field50_;}
    Actor* single_selected_human()const{return single_human54_;}
    template<class Activate>void activate(Activate&& activate_actor) { //00476210 snapshots count, not a growing traversal.
        field4c_=0;field50_=0;
        const auto count=identity_count_;
        for(std::size_t index=0;index!=count;++index) {
            if(index<32)generations_[index]=0xff;
            if(auto* actor=identities_[index])activate_actor(*actor);
        }
    }
    template<class Deactivate,class RemoveReserved>void deactivate(Deactivate&& deactivate_actor,RemoveReserved&& remove_reserved) { //00476260 after its actual pending-removal receiver.
        frontend::CleanupErrors errors;
        const auto count=identity_count_;
        for(std::size_t index=0;index!=count;++index)if(auto* actor=identities_[index]) {
            errors.attempt([&]{deactivate_actor(*actor);});
            //Original fetches the slot again AFTER the derived callback.
            if(index<32&&remove_reserved())remove(identities_[index]);
        }
        errors.rethrow();
    }
    std::uint8_t generation(std::size_t index) const {return generations_.at(index);}
    void player_generation(std::int32_t identity,std::uint32_t value){if(identity>=0&&identity<32)generations_[static_cast<std::size_t>(identity)]=static_cast<std::uint8_t>(value);} //00475510.
    std::size_t size() const {return owners_.size();}
    std::size_t retired_size() const {return pending_.size();}
    template<class Base>Actor* actor_for_base(const Base& base) const {
        for(const auto& record:owners_)if(&record.owner->base()==&base)return record.owner.get();
        for(const auto& pending:pending_)if(pending.record.owner && &pending.record.owner->base()==&base)return pending.record.owner.get();
        return nullptr;
    }
private:
    bool owns(Actor* actor)const {
        for(const auto& record:owners_)if(record.owner.get()==actor)return true;
        for(const auto& pending:pending_)if(pending.record.owner.get()==actor)return true;
        return false;
    }
    void unlink_identity(Actor* actor) noexcept {
        remove_active(actor);
        const auto identity=std::size_t(actor->base().id10());
        if(identity<32)identities_[identity]=nullptr;
        else {
            identities_[identity]=identities_[identity_count_-1];
            identities_[identity]->base().id10(static_cast<std::int32_t>(identity));
            --identity_count_;
        }
    }
    void remove_active(Actor* actor) noexcept {
        const auto id=actor->base().id14();
        if(id<0)return;
        active_[std::size_t(id)]=active_.back();
        active_[std::size_t(id)]->base().id14(id);
        active_.pop_back();actor->base().id14(-1);
    }
    struct Record {ActorOwner owner;Frame frame;};
    struct Pending {Record record;std::uint32_t milliseconds;};
    Backend& backend_;
    std::vector<Record> owners_;
    std::vector<Pending> pending_;
    std::vector<Actor*> identities_,active_,toggles_;
    std::size_t identity_count_=32;
    std::array<std::uint8_t,32> generations_{};
    bool clearing_=false,visiting_=false;
    std::uint32_t field4c_=0,field50_=0;
    Actor* single_human54_=nullptr;
};

} //namespace hd2::game
#endif
