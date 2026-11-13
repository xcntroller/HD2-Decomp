#ifndef HD2_GAME_HUMAN_LOADING_RETIREMENT_H
#define HD2_GAME_HUMAN_LOADING_RETIREMENT_H
#include "frontend/cleanup.h"
#include <stdexcept>
namespace hd2::game {
//Mode gates shared with00476860/Scene2 destruction: presentation bit2 first,
//authority bit1 second. Query again after a callback; no invented opposite pass.
template<class Mode,class Retire>void retire_human_mode_passes(Mode&& mode,Retire&& retire,frontend::CleanupErrors& errors){
    errors.attempt([&]{if(mode()&2u)retire(false);});
    errors.attempt([&]{if(mode()&1u)retire(true);});
}
//Typed native-lifetime adaptation: the actual external composition must retire
//before its Scene2 Human. Client-only mode has no later authority call. Keep the
//SAME owner published through its receiver; erase at its native last pass.
template<class Owners,class Human,class Mode>void deactivate_human_loading_owner(Owners& owners,
        Human& human,bool authority,Mode&& mode){
    const auto found=owners.find(&human);
    if(found==owners.end()){
        if(human.loading_lifetime)throw std::runtime_error("Human retirement lost its published containing owner");
        return; //Source partial-activation guard: this Human never acquired an external composition.
    }
    if(!found->second)throw std::logic_error("Human containing owner retirement re-entered");
    frontend::CleanupErrors errors;errors.attempt([&]{found->second->deactivate(authority);});
    if(authority||!(mode()&1u)){
        //The map key remains published during destruction. Its actual Human
        //loading_lifetime borrower still resolves the same retiring composition
        //while unique_ptr::reset has already cleared the owning slot.
        found->second.reset();owners.erase(found);
    }
    errors.rethrow();
}
}
#endif
