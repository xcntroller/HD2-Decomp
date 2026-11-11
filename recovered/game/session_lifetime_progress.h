#ifndef HD2_GAME_SESSION_LIFETIME_PROGRESS_H
#define HD2_GAME_SESSION_LIFETIME_PROGRESS_H

#include <array>
#include <cstddef>
#include <stdexcept>

namespace hd2::game {
//Source exception/lifetime guards, NOT additional native readiness/status/clock
//or roster state. Enter before a potentially mutating call, complete afterwards.
struct SessionLifetimeStep {
    bool entered=false,completed=false,retired=false,required=false;
    template<class Work>void run(Work&& work){entered=true;work();completed=true;}
    template<class Work>void retire(Work&& work){if((entered||required)&&!retired){work();retired=true;}}
};
struct SessionActivationProgress {
    SessionLifetimeStep resources,lights,faces,hud,first_person,spawn,dubbing,text;
    std::array<SessionLifetimeStep,19> authority{},presentation{};
    bool snapshot_taken=false,finished=false;
    std::array<bool,9> tail{};
};
template<class Registry,class Services>
void activate_session_module_prefix(Registry& registry,Services& services,bool authority,
                                   SessionActivationProgress* progress){
    if(progress&&registry.size()!=progress->authority.size())
        throw std::logic_error("Session lifetime requires the actual nineteen-slot module registry");
    for(std::size_t n=0;n!=registry.size();++n)if(auto* module=registry[n]){
        //GEffects authority activation loads birds; only its presentation
        //finish clears them. This is a cleanup dependency, not a claim that
        //presentation activation was entered/completed.
        if(progress&&authority&&n==1)progress->presentation[n].required=true;
        if(progress)(authority?progress->authority:progress->presentation)[n].run([&]{services.activate_mission_module(*module,authority);});
        else services.activate_mission_module(*module,authority);
    }
}
} //namespace hd2::game
#endif
