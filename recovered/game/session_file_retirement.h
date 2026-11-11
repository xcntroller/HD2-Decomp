#ifndef HD2_GAME_SESSION_FILE_RETIREMENT_H
#define HD2_GAME_SESSION_FILE_RETIREMENT_H

#include "game/session_activation.h"
#include "frontend/cleanup.h"
namespace hd2::game {
struct SessionFileProgress {
    bool dirty=false,frame_data=false,objects=false,collision=false,process=false,tables=false,scene=false;
    std::array<bool,19> authority{},presentation{};
};
//0069A3D0, plus explicit source rollback of an unpublished loaded14 prefix.
//Status1 finish is REQUIRED even if loaded14 is zero (native check order).
template<class Services>
void retire_session_files(SessionActivationState& state,SessionFileProgress& p,Services& services){
    if(state.status==1)services.finish_for_file_unload();
    if(!state.loaded&&!p.dirty)return;
    services.require_file_retirement(); //No live Human/VM/collision consumer may be bypassed.
    if(!p.frame_data){services.reset_file_fields();services.clear_file_frame_data();p.frame_data=true;}
    auto pass=[&](bool authority){
        auto& registry=services.module_registry();auto& done=authority?p.authority:p.presentation;
        if(registry.size()!=done.size())throw std::logic_error("File unload requires actual nineteen-slot registry");
        for(std::size_t n=0;n!=registry.size();++n)if(!done[n]){
            if(auto* module=registry[n])services.unload_mission_module(*module,authority);
            done[n]=true;
        }
    };
    if(services.mode_flags()&1u)pass(true);
    if(services.mode_flags()&2u)pass(false);
    //A failed mandatory unload stops dependent users/owners; completed slots
    //are consumed only once on explicit retry.
    if(!p.objects){services.clear_file_object_references();p.objects=true;}
    if(!p.collision){services.release_file_collision_tree();p.collision=true;}
    if(!p.process){services.retire_file_process_borrowers();p.process=true;}
    if(!p.tables){services.clear_file_tables();p.tables=true;}
    if(!p.scene){services.reset_file_scene();p.scene=true;}
    services.release_file_resource();
    state.resource.reset();state.mission=nullptr;state.loaded=false;p.dirty=false; //Native publication LAST.
}
} //namespace hd2::game
#endif
