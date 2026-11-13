#ifndef HD2_GAME_SESSION_HUMAN_INPUTS_H
#define HD2_GAME_SESSION_HUMAN_INPUTS_H
#include "game/human_input_loading_manager.h"
namespace hd2::game {
//ONE process owner for native14 and inline20/120/A0 profiles. World/B0 borrow
//this stable source handle, not a particular manager allocation.00723000 may
//delete/replace that allocation BEFORE the old game/B0 retires; old Record*
//borrowers are checked against the actual current registry before any use.
template<class Clock,class Receiver>class SessionHumanInputs {
public:
    using Manager=HumanInputLoadingManager<Clock,Receiver>;using Record=typename Manager::Record;
    using Profiles=typename Manager::Profiles;using Getter=typename Manager::Getter;
    SessionHumanInputs(Clock& clock,HumanInputShared& shared,void*& published)
        :clock_(clock),shared_(shared),published_(published){}
    ~SessionHumanInputs(){retire();}
    SessionHumanInputs(const SessionHumanInputs&)=delete;SessionHumanInputs& operator=(const SessionHumanInputs&)=delete;
    template<class Mode,class Local>void replace(Mode&& mode,Local&& local,Getter first,Getter second){ //00723000→00723220, destructive prefix.
        retire();
        owner_=std::make_unique<Manager>(clock_,shared_);published_=owner_.get(); //Before actual session defaults/getters.
        profiles_.reset();
        owner_->configure(profiles_,std::move(first),std::move(second),local(),mode());configured_=true;
    }
    void retire()noexcept{ //00723220; native pointer stays published until deleting receiver returns.
        owner_.reset();published_=nullptr;configured_=false;
    }
    bool present()const noexcept{return owner_!=nullptr;}
    bool owns(const void* pointer)const noexcept{return pointer&&pointer==owner_.get()&&published_==pointer;}
    bool configured()const noexcept{return configured_;}
    Record* find(std::int32_t id,bool create){return manager().find(id,create);}
    Record* make(std::int32_t id){return manager().make(id);}
    void remove(Record* record){if(owner_)owner_->remove(record);} //Old B0 teardown can outlive native manager allocation.
    bool contains(const Record* record)const noexcept{return owner_&&owner_->contains(record);}
    void clear()noexcept{if(owner_)owner_->clear();} //0073F5A0 preserves manager/embedded30/defaults.
    const auto& records()const{return manager().records();}
    const auto& embedded()const{return manager().embedded();}const auto& defaults()const{return manager().defaults();}
    void bind_primary(const Profiles& profiles){manager().bind_primary(profiles);}
    void bind_secondary(const Profiles& profiles){manager().bind_secondary(profiles);}
    void interval(float value){manager().interval(value);}
    void player_authority_mode(){manager().player_authority_mode();}
    void world_quantizer_bound(){manager().world_quantizer_bound();}
    const auto& state()const{return manager().state();}
    HumanInputSessionProfiles& profiles(){return profiles_;}
private:
    Manager& manager()const{
        if(!owner_||!configured_||published_!=owner_.get())throw std::logic_error("Human input requires actual configured process14 owner");
        return *owner_;
    }
    Clock& clock_;HumanInputShared& shared_;void*& published_;HumanInputSessionProfiles profiles_;
    std::unique_ptr<Manager> owner_;bool configured_=false;
};
}
#endif
