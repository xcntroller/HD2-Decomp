#ifndef HD2_GAME_SHOT_BULLET_PRODUCTION_H
#define HD2_GAME_SHOT_BULLET_PRODUCTION_H
#include "game/shot_manager_loading.h"

namespace hd2::game {
//00450D30's offline7 record production, followed by the distinct presentation
//producers. The SAME mark pool and active114 owner survive late effect failures.
template<class State,class Frames,class World,class Item,class Human>
void produce_item_bullet(State& state,Frames& frames,World& world,Item& item,Human& human,
        const HumanVector& position,const HumanVector& direction,typename Frames::Frame muzzle,
        typename Frames::Frame shell,bool trace,bool silent){
    if(!state.world)throw std::logic_error("Bullet production requires actual world-bound ShotMgr");
    if((world.mode_flags()&7u)!=7u)throw std::runtime_error("Bullet requires actual selected nonoffline00450D30 producer");
    const auto& payload=item.definition().payloads[0];const bool flare=item.definition().identifier==0x105u;
    auto record=state.marks.acquire();
    record->initialize(frames,world,human,position,direction,payload,trace?state.trace:typename Frames::Frame{},silent,flare);
    if(human.field160)(void)record->authority_tick(100); //Return ignored BEFORE active publication.
    state.transient[0].push_back(std::move(record));
    if(!(world.mode_flags()&2u))return;
    if(shell)world.emit_item_shell(human,shell,item.definition().fields38[1]);
    if(muzzle){
        if(world.shot_actor_in_scene(human))world.emit_item_muzzle_particle(item,muzzle);
        auto flash=std::make_unique<typename State::MuzzleFlash>(frames);
        flash->create(muzzle,item.flash_frame(),100);state.muzzle_flashes.push_back(std::move(flash));
    }
}
} //namespace hd2::game
#endif
