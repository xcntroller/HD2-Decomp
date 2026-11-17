#ifndef HD2_GAME_ANIMATION_ACTOR_FRAME_H
#define HD2_GAME_ANIMATION_ACTOR_FRAME_H
#include <cstdint>
#include <vector>
namespace hd2::game {
//008AC488 is BSS zero. Its only native write bracket is the unbound actor
//persistence loader005B3450; this SAME process owner must be used by that loader
//when recovered. It is not another per-World replay/sound default.
struct AnimationActorProcessState {bool restoring=false;};
//006FB660 scans ONLY static shape families0..4 in order, matching the exact
//frame. No triangle, dynamic-bucket, descendant or other-family substitution.
//Native missing-tree return leaves the caller's output untouched.
template<class Tree,class Frame>std::size_t collect_animation_static_flags(Tree* tree,Frame frame,std::vector<std::uint32_t*>& output){
    if(!tree)return 0;
    output.clear();
    for(unsigned family=0;family!=5;++family)for(auto& shape:tree->shapes[family])if(shape.frame==frame)output.push_back(&shape.flags);
    return output.size();
}
template<class Tree,class Frame>void set_animation_interaction_collision(Tree* tree,Frame frame,bool restore){ //005B5550.
    std::vector<std::uint32_t*> flags;collect_animation_static_flags(tree,frame,flags);
    for(auto* word:flags)*word=restore?*word&~0x20u:*word|0x20u;
}
//005B55E0: clear collisions before optional sound/particles, then helper and
//original visibility, script death, and selected non7 publication.
template<class Actor,class Owner>void start_animation_actor_frame(Actor& actor,Owner& owner,bool replicated){
    if(actor.flaga1)return;
    if(!actor.properties.name5.empty()){
        actor.field98=owner.find_interaction_frame(actor.properties.name5);
        if(actor.field98)owner.interaction_collision(actor.field98,false);
    }
    owner.clear_collisions();
    if((owner.mode_flags()&2u)&&!replicated&&!owner.replay_active()){
        if(actor.properties.fielda8!=0xffffffffu)owner.emit_start_sound(actor.properties.fielda8);
        if(actor.properties.fieldac!=0xffffffffu&&owner.particle_frame())owner.emit_start_particle(actor.properties.fieldac);
    }
    if(!actor.base().frame())return;
    if(const auto helper=owner.model()){
        if(owner.clip_zero()){
            const auto clip=owner.clip_zero();owner.clip_start_flag(clip);actor.flaga0=true;
        }
        actor.flaga1=true;owner.model_visibility(helper,true);
    }
    owner.model_visibility(actor.base().frame(),false);
    if((owner.mode_flags()&1u)&&!owner.replay_active())owner.script_death(actor.base().frame());
    if((owner.mode_flags()&7u)!=7u&&(owner.mode_flags()&1u)&&!owner.replay_active())owner.publish_started(replicated);
}
//005B50C0 advances exactly to duration-1 before rebuilding BODYCOLB from the
//HELPER model. A0 clears only after that producer returns.
template<class Actor,class Owner>void finish_animation_actor_frame(Actor& actor,Owner& owner){
    owner.clear_collisions();
    if(owner.clip_zero()){
        const auto clip=owner.clip_zero();const auto current=owner.clip_time();const auto duration=owner.clip_duration(clip);
        owner.tick_model((duration-current)-1u);
    }
    owner.rebuild_collisions(owner.model());actor.flaga0=false;
}
//005B5190. Raw5B5204 writes predicted time at the adjusted stack offset; the
//decompiler's apparently unwritten uStack38 is actually current+elapsed.
template<class Actor,class Owner>void advance_animation_actor_frame(Actor& actor,Owner& owner,std::uint32_t elapsed){
    if(actor.flaga3){actor.flaga3=false;owner.start(false);}
    if(owner.model()&&actor.flaga0&&owner.clip_zero()){
        const auto clip=owner.clip_zero();const std::uint32_t predicted=owner.clip_time()+elapsed;
        if(predicted>=owner.clip_duration(clip))owner.finish_animation();
        else if(!owner.paused())owner.tick_model(elapsed);
    }
    if(actor.properties.flaga2&&((!actor.flaga1&&!actor.flaga0)||owner.collision_countdown())){
        if(owner.collision_countdown())owner.decrement_collision_countdown();
        owner.update_collisions();owner.update_wings();
    }
}
}
#endif
