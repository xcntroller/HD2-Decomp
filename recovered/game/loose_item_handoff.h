#ifndef HD2_GAME_LOOSE_ITEM_HANDOFF_H
#define HD2_GAME_LOOSE_ITEM_HANDOFF_H
namespace hd2::game {
//005B6980: erase frame EC allocation BEFORE clearing90 and releasing20.
//The body remains live until00477190 performs actual actor deactivation. The
//item now belongs to its inventory/module and must survive loose-actor cleanup.
template<class Frames,class Base,class Entry>void release_loose_item_to_inventory(Frames& frames,Base& base,Entry& entry){
    frames.erase_actor_frame_data(base.frame());entry=Entry{};base.release_runtime_frame();
}
}
#endif
