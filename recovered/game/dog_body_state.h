#ifndef HD2_GAME_DOG_BODY_STATE_H
#define HD2_GAME_DOG_BODY_STATE_H
#include "game/dog_body_transform.h"
#include "game/dog_body_collisions.h"
namespace hd2::game {
//Defined005C7D30 Body fields not already owned by actions/orders/transform/clip.
//The seven damage frames are the actual478..490 actor slots, not a second lookup.
template<class Frame>struct DogBodyState {
    using Vector=std::array<float,4>;
    Frame animation2_164{},ground404{};std::array<Frame,7> damage_bones{};
    Vector direction180{},surface210{{0,-1,0,0}},surface220{{0,-1,0,0}};
    std::uint32_t field24c=0,field240=0,field250=0,field254=0,field264=0,field1fc=0,
        field23c=0,field234=0,field238=0,field388=0;
    float speed244=1,field258=0,field1f8=0,field200=1;
    std::int32_t field25c=-10000,material1f0=-1,field1f4=-420;
    bool flag230=false,flag231=false,flag232=false,flag268=false,flag3e4=false;
    bool initialized=false; //Source provenance only, never substitutes for a reached receiver.
};
//005C7D30 following the actual005C6E10 clip publication/initialization. The
//collision owner retains an inserted prefix on a later marker/priority failure.
//Engine callbacks borrow the same Actor/model and each lookup reloads frame20.
template<class Actor,class Frames,class Collisions,class Receivers>
void initialize_dog_body(Actor& actor,Frames& frames,Collisions& collisions,Receivers& receivers){
    auto& b=actor.body;auto& a=actor.body_actions;auto& t=actor.body_transform;
    b.initialized=false;b.field24c=0;
    if(actor.collisions)throw std::logic_error("Dog Body initialization requires actual prior descriptor retirement");
    actor.collisions=std::make_unique<DogBodyCollisions<Frames,Collisions,Actor>>(frames,collisions,actor);
    b.animation2_164=frames.actor_find(actor.base().frame(),"anim2",0xffffu);
    a.animation2_168=0;a.animation2_position170.fill(0);b.direction180=actor.base().world_direction();
    actor.body_orders.initialize();a.initialize_action_prefix();
    t.requested3c0=false;t.interval38c=-1;for(unsigned n=0;n!=3;++n)t.displacement3d0[n]=0;
    b.field388=0;b.flag3e4=false;
    const auto matrix=frames.actor_world_matrix(actor.base().frame());
    t.initialize_bootstrap_position({{matrix[12],matrix[13],matrix[14],matrix[15]}});
    actor.base().world_position(*t.position3b0);
    b.speed244=a.speed248=1;b.field240=b.field250=b.field254=b.field264=0;
    b.field258=-10000.099609375f;b.field25c=-10000;b.field1f4=-420;b.field200=1;b.field1f8=-10000.099609375f;
    b.material1f0=-1;b.field1fc=b.field23c=b.field234=b.field238=0;
    b.surface210=b.surface220=typename DogBodyState<typename Frames::Frame>::Vector{{0,-1,0,0}};b.flag230=false;
    a.timer370=a.death374=0;a.death_timer378=-10000;
    //Native order d,w,f,c,h,p,t,l differs from the contiguous damage slots.
    b.damage_bones[1]=frames.actor_find(actor.base().frame(),"d",0xffffu);
    b.ground404=frames.actor_find(actor.base().frame(),"w",0xffffu);
    b.damage_bones[0]=frames.actor_find(actor.base().frame(),"f",0xffffu);
    constexpr std::array<const char*,5> names{{"c","h","p","t","l"}};
    for(unsigned n=0;n!=5;++n)b.damage_bones[n+2]=frames.actor_find(actor.base().frame(),names[n],0xffffu);
    b.flag268=false;a.initialize_voice_tail();b.flag231=b.flag232=false;
    //278/27C/280 are NOT initialized here. Their existing provenance is kept;
    //if unproduced, fail at the first native read after the entire prior prefix.
    a.request_action(3,0,receivers);a.request_posture(0,0);a.request_stance(0,0);
    a.animation2_168=0;frames.dog_clear_animation2_position(b.animation2_164);
    a.animation2_position170.fill(0);b.initialized=true;
}
//005C8370 uses Body+10 and SAME actual Actor. Callback lifetime must outlive clip.
template<class Mixer,class Actor,class Frames,class Callbacks,class Collisions,class Receivers>
void bind_dog_body(Actor& actor,Frames& frames,Callbacks& callbacks,Collisions& collisions,Receivers& receivers){
    if(actor.clip)throw std::logic_error("Dog Body bind requires actual prior clip retirement");
    actor.clip=std::make_unique<Mixer>(frames,callbacks,actor.clip_forced_motion);
    actor.clip->initialize(actor.base().frame());
    initialize_dog_body(actor,frames,collisions,receivers);
}
}
#endif
