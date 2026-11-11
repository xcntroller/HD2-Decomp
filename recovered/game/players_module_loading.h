#ifndef HD2_GAME_PLAYERS_MODULE_LOADING_H
#define HD2_GAME_PLAYERS_MODULE_LOADING_H

#include "frontend/module_registration.h"
#include "game/human_player_control_loading.h"
#include "game/players_process.h"
#include <new>

namespace hd2::game {

//Required native subgroup virtual14/04 interface. The loading module has no
//append producer and creates no subgroup. A later real producer must supply
//both bodies; an empty successful tick/destroy is not a loading dependency.
class PlayersSubgroup {
public:
    virtual ~PlayersSubgroup()=default;
    virtual void tick(std::uint32_t)=0;
    virtual void destroy(bool)=0;
};

//Actual module+E8/0045FB90, separate roster and selection28/buffer2C.
//0045FE10 passes THIS+28 to the same0045E540 selection body as control38;
//005A0220 also invokes0045E410 on this actual auxiliary subobject.
template<class Human>class PlayersAuxiliaryLoading {
public:
    PlayersAuxiliaryLoading(){reset();}
    ~PlayersAuxiliaryLoading(){reset();} //0045FC10, records before roster/base destruction.
    void reset(){roster_.clear();clear_records();} //0045FC80.
    void post_loading(){clear_records();} //0045FD80, unlike reset it preserves roster.
    auto& human_roster(){return roster_;}const auto& human_roster()const{return roster_;}
    auto& selection(){return selection_;}const auto& selection()const{return selection_;}
    auto words()const{ //Read the actual selection counters; no second scalar copy.
        HumanJointWords<0x50> result;result.set(0x3c,static_cast<std::uint32_t>(selection_.active_count()));
        result.set(0x40,static_cast<std::uint32_t>(selection_.inactive_count()));result.set(0x44,std::uint8_t(selection_.notification_flag()));result.set(0x48,field48_);return result;
    }
private:
    void clear_records() {
        selection_.clear();field48_=0; //Reverse release; retain allocation and changed28.
    }
    HumanRoster<Human> roster_;HumanSelectionRecords<Human> selection_;std::uint32_t field48_=0;
};

//00461430/004614E0: module11 registration, vector10, control1C, auxiliaryE8.
//Group is the real subgroup receiver supplied by its producer, not a successful
//fallback. Runtime supplies mode/config; Speech supplies actual005383D0.
//Loading lifecycle is supplied here; subgroup/save/remote producers are separate.
template<class Frames,class Human,class Presentation,class Active,class Runtime,class Group>
class PlayersModuleLoading {
public:
    using Control=HumanPlayerControlLoading<Frames,Human,Presentation>;
    using Process=PlayersProcess<Human>;
    PlayersModuleLoading(frontend::ClientModuleRegistry& registry,Frames& frames,Presentation& presentation,
                         Process& process,Runtime& runtime,const char* name="TeamMgr")
        :registration_(registry,11,name),control_(frames,presentation),frames_(frames),process_(process),runtime_(runtime){}
    PlayersModuleLoading(frontend::ClientModuleRegistry& registry,Frames& frames,Presentation& presentation,
                         Process& process,Runtime& runtime,frontend::ClientModuleRegistration& shared_registration,const char* name="TeamMgr")
        :registration_(registry,shared_registration,11,name),control_(frames,presentation),frames_(frames),process_(process),runtime_(runtime){}
    PlayersModuleLoading(const PlayersModuleLoading&)=delete;PlayersModuleLoading& operator=(const PlayersModuleLoading&)=delete;
    auto& registration(){return registration_.get();}
    auto& control(){return control_;}const auto& control()const{return control_;} //00461410.
    auto& auxiliary(){return auxiliary_;} //00461420.
    bool active()const{return active134_;}bool flag135()const{return flag135_;}
    void activate(bool authority) { //00460AA0, module virtual1C.
        if(!authority)active134_=true;
        if((runtime_.mode_flags()&7u)==7u&&authority)process_.reset();
    }
    void finish(bool authority) { //00460B30, module virtual20.
        if(!authority) {
            if(runtime_.mode_flags()&2u)control_.post_loading(); //Actual control virtual18/0045DDE0, NOT its clear.
            active134_=false;
        }else {
            flag135_=false;groups_.clear(); //Native drops pointers WITHOUT subgroup destruction here.
            if((runtime_.mode_flags()&7u)!=7u&&runtime_.human_remote_ai_enabled())auxiliary_.reset();
        }
    }
    void reset(bool parameter) { //00460C20, module virtual2C.
        (void)parameter; //Both actual post-loading receivers ignore their native argument.
        if(runtime_.mode_flags()&2u)control_.post_loading();
        if((runtime_.mode_flags()&7u)!=7u&&(runtime_.mode_flags()&1u)&&runtime_.human_remote_ai_enabled())auxiliary_.post_loading();
        for(auto n=groups_.size();n;)if(auto* group=groups_[--n])group->destroy(true); //Native virtual04(1), fresh buffer each callback.
        groups_.clear();process_.reset();
    }
    template<class Speech>void tick(std::uint32_t elapsed,std::uint32_t game_time,
                                  std::vector<typename Process::DelayedSpeech>& delayed,Speech& speech) { //00460AE0.
        if(!(runtime_.mode_flags()&2u))return;
        process_.template tick<Active>(elapsed,runtime_.mode_flags(),frames_,control_,game_time,delayed,speech);
        for(auto n=groups_.size();n;) {
            auto* group=groups_[--n];if(!group)throw std::runtime_error("Players tick has a null native subgroup");
            group->tick(elapsed); //Native virtual14, no null suppression or dedup.
        }
    }
    void offline_player_close(Human& human){process_.template offline_player_close<Active>(human,control_);}
    //No subgroup append API until00461290/its real producer is reconstructed.
private:
    //Declaration order preserves004614E0: auxiliary, control, vector allocation,
    //then base registration. The original destructor does not destroy subgroups.
    frontend::ClientModuleBinding registration_;std::vector<Group*> groups_;Control control_;PlayersAuxiliaryLoading<Human> auxiliary_;
    Frames& frames_;Process& process_;Runtime& runtime_;bool active134_=false,flag135_=false;
};
} //namespace hd2::game
#endif
