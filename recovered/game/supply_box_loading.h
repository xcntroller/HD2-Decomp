#ifndef HD2_GAME_SUPPLY_BOX_LOADING_H
#define HD2_GAME_SUPPLY_BOX_LOADING_H
#include "game/model_scene_actors.h"
#include "game/mission_vertex_instances.h"
#include "game/rigid_body.h"
#include "frontend/spatial_sound.h"
#include "frontend/cleanup.h"
namespace hd2::game {
//005FAC80/005F9A60/005FB220 actual cold SupplyBox loading composition.
//Same shared vertex registry/definitions, frame fades and dynamic buckets.
//Cold command6→0/animation update is required now; respawn/falling/pickup remain
//distinct receivers that reject when reached, never successful substitutes.
template<class Frames,class Services,class Collisions,class Vertices,class Fades>
class SupplyBoxLoading final:public SupplyBoxLoadingLifetime<typename Frames::Frame> {
public:
    using Actor=SupplyBoxSceneActor<Frames>;using Frame=typename Frames::Frame;using Descriptor=RigidBodyCollisionDescriptor<Frame>;
    using FrameOwner=decltype(std::declval<Frames&>().create_supply_helper(std::uint32_t{}));
    using ClipOwner=decltype(std::declval<Frames&>().load_supply_animation(std::declval<const std::string&>()));
    using Vertex=typename Vertices::Instance;
    SupplyBoxLoading(Frames& frames,Services& services,Collisions& collisions,Vertices& vertices,Fades& fades,Actor& actor)
        :frames_(frames),services_(services),collisions_(collisions),vertices_(vertices),fades_(fades),actor_(actor){}
    ~SupplyBoxLoading()override{try{retire();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}}
    Frame helper(unsigned index)const override{return helpers_.at(index)?helpers_[index]->get():Frame{};}
    const Descriptor* collision_descriptor()const{return descriptor_.get();}std::uint32_t state()const{return flags124_;}
    void activate(bool authority)override { //005FAC80, genuine once guard BEFORE common activation.
        if(actor_.flag90){if(!ready_)throw std::logic_error("Retire partial SupplyBox activation before retry");return;}
        if(descriptor_)throw std::logic_error("Retire actual SupplyBox resources before retry");
        actor_.flag90=true;ready_=false;
        if(!actor_.geometry)throw std::runtime_error("SupplyBox requires its actual initialized geometry");
        actor_.field138=6000;actor_.field130=actor_.properties.field12c;
        actor_.field244=0;actor_.field248=0xffffffffu;actor_.field24c=5;actor_.field250=actor_.field254=0;actor_.flag224=false;
        actor_.geometry->refresh(frames_);const auto frame=actor_.base().frame();const auto parent=frames_.actor_parent(frame);
        actor_.base().activate_frame(authority,services_);frames_.actor_link_preserving_world(frame,parent);
        helpers_[0].emplace(frames_.create_supply_helper(9));require_helper(0);
        if(frames_.load_supply_model(helper(0),"cratedummy",0x10)!=0)throw std::runtime_error("Required SupplyBox cratedummy model is unavailable");
        falling_crate_=frames_.actor_find(helper(0),"letibedna",0x10020);
        helpers_[2].emplace(frames_.create_supply_helper(6));require_helper(2);
        helpers_[1].emplace(frames_.create_supply_helper(9));require_helper(1);
        if(frames_.load_supply_model(helper(1),"parachute",0)!=0)throw std::runtime_error("Required SupplyBox parachute model is unavailable");
        const char* names[]={"_cratefly","_crateopen","_crateclosed","_cratedrop"};
        for(unsigned i=0;i!=4;++i){clips_[i].emplace(frames_.load_supply_animation(names[i]));if(!clips_[i]->get())throw std::runtime_error("Required SupplyBox animation is unavailable");}
        initialize_vertex_and_collision();actor_.base().name(services_.actor_localized(actor_.properties.field210));ready_=true;
    }
    void deactivate(bool authority)override { //005FB220.
        if(!actor_.flag90)return;
        frontend::CleanupErrors errors;errors.attempt([&]{actor_.base().deactivate_frame(authority,services_);});actor_.flag90=false;
        errors.attempt([&]{retire();});errors.rethrow();
    }
    void update(std::uint32_t elapsed)override { //005FB380 reached cold state6→0; authority005FBD20 skips mode2 in caller.
        if(flags124_&2u)return;
        if(!ready_)throw std::logic_error("SupplyBox update requires completed actual loading");
        if(actor_.field250!=6&&actor_.field250!=0)throw std::runtime_error("SupplyBox update requires actual respawn/falling receivers");
        if(signed_word(actor_.field254)>0) {
            actor_.field254-=elapsed;
            if(signed_word(actor_.field254)<=0){actor_.field254=0xffffffffu;actor_.field250=0;} //005FCA60→005FCD80(0).
        }
        update_animation(elapsed);
        if(actor_.flag224)throw std::runtime_error("SupplyBox moving dummy requires actual sound/respawn receiver");
    }
private:
    static std::int32_t signed_word(std::uint32_t bits){std::int32_t value;std::memcpy(&value,&bits,4);return value;}
    static float stored(long double value){return static_cast<float>(value);}
    void require_helper(unsigned index)const{if(!helper(index))throw std::runtime_error("Cannot create required SupplyBox helper frame");}
    void initialize_vertex_and_collision() { //005F9A60.
        const auto* definition=vertices_.definition("ammobox");
        const auto morph=frames_.actor_find(helper(1),"morph",0x10001);
        if(!morph||frames_.visual_type(morph)!=0x5f4a424f)throw std::runtime_error("SupplyBox parachute requires actual morph OBJ_ geometry");
        vertex_=std::make_unique<Vertex>();
        if(definition) {
            frames_.prepare_supply_morph(morph);vertices_.bind(*vertex_,morph,definition);vertex_->finished=true;vertices_.reset_geometry(*vertex_);
            vertices_.register_borrowed(*vertex_);vertex_->ping_pong=true;
            if(definition->sizes.empty()||definition->sizes.size()>0x80000000u)throw std::runtime_error("SupplyBox vertex LOD count leaves native maximum undefined");
            vertex_->maximum_lod=static_cast<std::int32_t>(definition->sizes.size()-1);vertex_->duration=0x82;vertex_->start=0;vertex_->repeat=false;
        }
        const auto frame=actor_.base().frame();const auto bounds=frames_.car_bounds(frame);const auto scale=frames_.actor_local_scale(frame);
        descriptor_=std::make_unique<Descriptor>();auto& d=*descriptor_;d.flags=0x40;d.mask=0x800;d.frame=frame;d.actor=&actor_;
        for(unsigned a=0;a!=3;++a) {
            if(!std::isfinite(scale[a])||scale[a]==0)throw std::runtime_error("SupplyBox requires finite nonzero local collision scales");
            d.minimum[a]=stored(static_cast<long double>(bounds.minimum[a])*scale[a]);d.maximum[a]=stored(static_cast<long double>(bounds.maximum[a])*scale[a]);
        }
        const auto half_x=(static_cast<long double>(d.maximum[0])-d.minimum[0])*.5f,half_z=(static_cast<long double>(d.maximum[2])-d.minimum[2])*.5f;
        d.radius=stored(std::sqrt(half_z*half_z+half_x*half_x)*1.2000000476837158); //Native horizontal-only radius and DOUBLE factor.
        refresh_collision(false);refresh_ammo();flags124_=2;set_flags(actor_.properties.field120|0x18u);
        if(flags124_&1u)initial_open();
    }
    void refresh_collision(bool move=true) { //005FA530, stable owned normalized matrices; frame origin sphere centre.
        if(!descriptor_)throw std::logic_error("SupplyBox collision update requires its actual descriptor");
        const auto frame=actor_.base().frame();const auto scale=frames_.actor_local_scale(frame);world_=frames_.actor_world_matrix(frame);
        for(unsigned a=0;a!=3;++a) {
            if(!std::isfinite(scale[a])||scale[a]==0)throw std::runtime_error("SupplyBox collision matrix has invalid scale");
            const auto inverse=1.L/scale[a];for(unsigned c=0;c!=3;++c)world_[a*4+c]=stored(static_cast<long double>(world_[a*4+c])*inverse);
        }
        inverse_=frames_.car_inverse_matrix(world_);descriptor_->matrix=&world_;descriptor_->inverse=&inverse_;
        const auto world=frames_.actor_world_matrix(frame);descriptor_->position={{world[12],world[13],world[14],world[15]}};
        if(move)collisions_.move(*descriptor_);
    }
    void refresh_ammo() { //005FD760;14 actual authored borrowers, final-case13 rule.
        if(signed_word(actor_.properties.field12c)==0)throw std::runtime_error("SupplyBox ammo visibility divides by native zero capacity");
        const auto amount=stored(14.L-static_cast<long double>(signed_word(actor_.field130))/signed_word(actor_.properties.field12c)*14.f); //Actual x87 FLOAT spill before every comparison.
        unsigned index=0;
        for(const auto frame:actor_.geometry->ammo_frames()) {
            if(frame){const bool visible=amount<=index||(index>12&&amount<14.f&&amount>13.f);frames_.object_visible(frame,visible);}
            ++index;
        }
    }
    void set_flags(std::uint32_t flags) { //005FBD90, immediate fade-in initial branch; same existing fade-out queue.
        if(!(flags&1u)) {
            if(helper(1))frames_.object_visible(helper(1),false);
            fades_.fade_out_tree(actor_.base().frame(),(flags&0x10u)?0:1000,true);
            if(flags124_&1u)collisions_.remove(*descriptor_);
            flags124_=flags&7u;return;
        }
        if(!(flags&0x10u))throw std::runtime_error("SupplyBox positive fade-in requires actual 210 queue/cancellation receivers");
        if(flags124_&6u)collisions_.insert(*descriptor_);
        fades_.fade_in_immediate_tree(actor_.base().frame(),true);
        if(flags&8u) {
            const auto& position=actor_.geometry->position();frames_.object_position(actor_.base().frame(),{{position[0],position[1],position[2]}});
            frames_.update_scene_object(actor_.base().frame());frames_.auto_parent_object(actor_.base().frame());
        }
        refresh_collision();flags124_=flags&7u;
    }
    void initial_open() { //005FCD80(6)→005FC860(1), actual crate-open clip and group1/entry10.
        if(actor_.field248==0xffffffffu)actor_.field248=0;
        frames_.human_model_animation(actor_.base().frame(),clips_[1]->get(),actor_.field248,1.f,1);actor_.field244=0;actor_.field24c=1;
        vertex_->finished=true;
        const auto duration=frames_.human_clip_duration(clips_[1]->get());actor_.field254=static_cast<std::uint32_t>(duration<1000?1000:duration);actor_.field250=6;
        frontend::SpatialSoundRequest request;const auto world=frames_.actor_world_matrix(actor_.base().frame());
        request.position={{world[12],world[13],world[14],world[15]}};request.flags=1;request.group=1;request.entry=10;services_.emit_supply_sound(request);
    }
    void stop_animation() { //005FC860(5), failure-prefix guards only omit genuinely unacquired resources.
        if(actor_.field248==0xffffffffu)actor_.field248=0;
        for(unsigned index:{2u,1u})if(clips_[index]&&clips_[index]->get()) {
            frames_.human_model_animation(actor_.base().frame(),clips_[index]->get(),actor_.field248,1.f,1);frames_.supply_model_tick(actor_.base().frame(),0);
        }
        frames_.human_model_remove_animation(actor_.base().frame(),actor_.field248);actor_.field24c=5;
    }
    void update_animation(std::uint32_t elapsed) { //005FC9A0 cold unblended path.
        if(signed_word(actor_.field244)>0)throw std::runtime_error("SupplyBox animation crossfade requires its actual weight receiver");
        if(actor_.field248!=0xffffffffu&&!frames_.human_model_animation_enabled(actor_.base().frame(),actor_.field248))actor_.field24c=5;
        frames_.supply_model_tick(actor_.base().frame(),elapsed);
    }
    void retire() { //005FB220 derived cleanup; common deactivation remains with caller.
        frontend::CleanupErrors errors;ready_=false;
        if(vertex_){vertex_->finished=true;errors.attempt([&]{vertices_.reset_geometry(*vertex_);});}
        if(helper(0))errors.attempt([&]{frames_.object_visible(helper(0),false);});
        if(helper(1))errors.attempt([&]{frames_.actor_link_preserving_world(helper(1),Frame{});});
        if(actor_.flag90||descriptor_)errors.attempt([&]{frames_.actor_link_preserving_world(actor_.base().frame(),Frame{});});
        if(actor_.field248!=0xffffffffu||clips_[1]||clips_[2]) {
            errors.attempt([&]{stop_animation();});errors.attempt([&]{update_animation(1);});
        }
        if(vertex_){errors.attempt([&]{vertices_.unregister_borrowed(*vertex_);});vertex_.reset();}
        helpers_[0].reset();falling_crate_=Frame{};helpers_[2].reset();helpers_[1].reset();for(auto& clip:clips_)clip.reset();
        if(descriptor_) {
            errors.attempt([&]{set_flags(0x19);});
            collisions_.remove_for_cleanup(*descriptor_);descriptor_.reset();
        }
        actor_.flag90=false;errors.rethrow();
    }
    Frames& frames_;Services& services_;Collisions& collisions_;Vertices& vertices_;Fades& fades_;Actor& actor_;
    std::array<std::optional<FrameOwner>,3> helpers_;std::array<std::optional<ClipOwner>,6> clips_;Frame falling_crate_{};
    std::unique_ptr<Vertex> vertex_;std::unique_ptr<Descriptor> descriptor_;typename Frames::Matrix world_{},inverse_{};
    std::uint32_t flags124_=2;bool ready_=false;
};
} //namespace hd2::game
#endif
