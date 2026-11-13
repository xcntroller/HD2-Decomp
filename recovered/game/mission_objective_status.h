#ifndef HD2_GAME_MISSION_OBJECTIVE_STATUS_H
#define HD2_GAME_MISSION_OBJECTIVE_STATUS_H

#include "game/mission_integrity_tables.h"
#include "game/mission_script_context.h"
#include "game/session_activation.h"
#include "game/human_death_objective.h"

namespace hd2::game {

inline void retail_objective_status_output()noexcept{} //0069BFD0, genuine RET.

//006B6510→0069BA60/0069BC40. Uses SAME missionA8.related/native34
//descriptors and session clock/result words. Actual process guard, CRT random,
//Players, experience and HUD hint owners remain mandatory; no healthy guard
//or successful presentation stand-in. Non7 uses the SAME World score/FIFO/HUD
//owners; normal metadata admission and nonoffline transport remain required.
template<class Random,class Players,class Experience,class Hint,class World>class MissionObjectiveStatus {
public:
    MissionObjectiveStatus(SessionActivationState& session,const std::uint32_t& mode,MissionIntegrityTables& tables,
            Random& random,Players& players,Experience& experience,Hint& hint,World& world)
        :session_(session),mode_(mode),tables_(tables),random_(random),players_(players),experience_(experience),hint_(hint),world_(world){}
    void set_status(std::int32_t identity,std::int32_t status) {
        if((mode_&7u)!=7u){
            if(world_.human_score_style()==2&&status==1){
                const auto fields=human_death_objective_fields(find(identity)); //Native invalid dereference rejects explicitly.
                world_.award_human_objective_score(fields.score24,fields.side18);
            }
            apply(identity,status);
            world_.queue_human_objective_status(identity,status); //AFTER score, descriptor and HUD; owns scalar values only.
            return;
        }
        if(tables_.objective_failure_trial(random_)){apply(identity,3);return;}
        if(status==1){
            auto* objective=find(identity);if(session_.mission&&!objective)return;
            if(objective&&objective->flag15){apply(identity,status);return;}
            const auto amount=objective&&objective->flag14?1500:1000;
            auto& roster=players_.human_roster();
            for(auto n=roster.size();n;){
                auto* human=roster.at(static_cast<std::int32_t>(--n));
                if(!human)throw std::runtime_error("Objective experience requires the actual Players Human");
                if(signed_word(human->properties.stats_ec[0])>0)experience_.add_experience(*human,amount);
            }
        }
        apply(identity,status);
    }
    void receive_status(std::int32_t identity,std::int32_t status){apply(identity,status);} //00728450→0069BC40; no0069BA60 score/FIFO producer.
    void fail_all(bool special) { //0069BED0; hint notification precedes descriptor mutation.
        session_.words.set(0x6f,std::uint8_t{1});result(special?0x1cf0u:0x1ce9u);
        const auto* mission=session_.mission;
        if(!mission)throw std::runtime_error("Objective failure requires its actual mission descriptor");
        for(const auto& owner:mission->related){
            if(!owner)throw std::runtime_error("Objective vector lost its actual descriptor");
            if(owner->word10==0)owner->word10=2;
        }
    }
private:
    frontend::GameDataNode* find(std::int32_t identity)const {
        const auto* mission=session_.mission;if(!mission)return nullptr;
        const auto index=static_cast<std::uint32_t>(identity)-1u;
        return index<mission->related.size()?mission->related[index].get():nullptr;
    }
    void apply(std::int32_t identity,std::int32_t status) { //0069BC40/0069BEB0.
        if(auto* objective=find(identity)){
            objective->word10=static_cast<std::uint32_t>(status);
            if(status==1)objective->flag15=1; //Latch; other statuses do not clear it.
        }else if(!session_.mission)retail_objective_status_output();
        switch(status){
        case 0:case 1:case 2:retail_objective_status_output();break;
        case 3:fail_all(false);retail_objective_status_output();return;
        case 4:return; //Set descriptor, then genuinely skip completion evaluation.
        default:break;
        }
        if((mode_&7u)!=7u){
            if(mode_&2u)if(auto* objective=find(identity)){
                const auto side=human_death_objective_fields(objective).side18;
                //00724EA0 is a register-argument signed (side+1)%2 helper,
                //not the selected/local team getter. ADD wraps before remainder.
                const auto kind=status==1?signed_word(side):signed_word(side+1u)%2;
                hint_.select_objective(static_cast<std::uint32_t>(identity)-1u,kind);
            }
            return; //Mission completion/result/clock scan belongs ONLY to offline7.
        }
        const auto* mission=session_.mission;if(!mission||mission->related.empty())return;
        bool all_ignored=true;
        for(const auto& objective:mission->related){
            if(!objective)throw std::runtime_error("Objective completion requires its actual descriptor");
            if(objective->word10!=4){all_ignored=false;break;}
        }
        if(all_ignored)return;
        for(const auto& objective:mission->related){
            if(!objective)throw std::runtime_error("Objective completion requires its actual descriptor");
            if(objective->word10==0)return;
        }
        result(status==1&&!session_.words.template get<std::uint8_t>(0x6f)?0x1ce8u:0x1ce9u);
    }
    void result(std::uint32_t text) {
        session_.words.set(0x88,std::uint32_t{1});session_.words.set(0x8c,session_.words.template get<std::uint32_t>(0x28));
        hint_.set_hint(true,text); //0049DE30→actual004AD880, not a new message queue.
    }
    static std::int32_t signed_word(std::uint32_t value){std::int32_t result;std::memcpy(&result,&value,4);return result;}
    SessionActivationState& session_;const std::uint32_t& mode_;MissionIntegrityTables& tables_;
    Random& random_;Players& players_;Experience& experience_;Hint& hint_;World& world_;
};

template<class Frames,class Objectives,class Runtime>std::optional<bool> execute_mission_objective_command(
        MissionScriptContext<Frames>& context,const MissionScriptInstruction& instruction,Objectives& objectives,Runtime& runtime){
    if(instruction.opcode!=0x88u)return std::nullopt;
    if(!runtime.script_authority())return false;
    const auto first=context.program().integers.at(instruction.operands.at(0)).value;
    const auto second=context.program().integers.at(instruction.operands.at(1)).value;
    std::int32_t identity,status;std::memcpy(&identity,&first,4);std::memcpy(&status,&second,4);
    objectives.set_status(identity,status);context.advance(runtime);return true;
}

} //namespace hd2::game
#endif
