#ifndef HD2_GAME_GEOMETRY_SCENE_ACTORS_H
#define HD2_GAME_GEOMETRY_SCENE_ACTORS_H

#include "game/scene_actor.h"
#include "game/actor_initial_properties.h"
#include "game/ladder_geometry.h"
#include "game/door_portal_binding.h"
#include "game/door_loading_packets.h"
#include "game/door_rotation_loading.h"
#include "game/dynamic_collision_exclusions.h"
#include "frontend/spatial_sound.h"
#include "game/sound_emission.h"
#include "game/switch_actor_state.h"
#include "game/rigid_body.h"
#include "frontend/cleanup.h"
#include <functional>
#include <optional>

namespace hd2::game {

//Concrete loading and bounded activation owners. Interaction/simulation and
//the remaining Door activation receivers are separate required consumers.
template<class Backend>
class LadderSceneActor final : public SceneActor<Backend> {
public:
    using Frame=typename Backend::Frame;
    using Descriptor=RigidBodyCollisionDescriptor<Frame>;
    LadderSceneActor(Backend& backend,std::string localized_name) : SceneActor<Backend>(backend,8),
        properties(initial_ladder_properties()) {this->base_.name(std::move(localized_name));} //005EA3D0.
    bool initialize(Frame frame) override { //005EA5D0, geometry precedes common retain.
        if(!frame)return false;
        if(geometry)throw std::logic_error("Ladder frame initialization repeated");
        geometry.emplace(this->backend_,frame);
        return this->base_.initialize_frame(frame);
    }
    void load(SerializedBytes& bytes) override {properties.load(bytes);} //005EBBD0.
    ~LadderSceneActor() override {if(remove_collisions_)remove_collisions_();}
    template<class Activation,class Collisions>void activate(bool authority,Activation& activation,Collisions& collisions) { //005EAD20.
        const auto frame=this->base_.frame();
        if(!frame)throw std::logic_error("Ladder activation requires its initialized frame");
        const auto parent=this->backend_.actor_parent(frame); //Snapshot BEFORE common auto-parenting.
        this->base_.activate_frame(authority,activation);
        this->backend_.actor_link_preserving_world(frame,parent);
        if(flag90)return;
        if(!geometry)throw std::logic_error("Ladder activation requires its actual marker geometry");
        if(collision_owner_&&collision_owner_!=&collisions)throw std::logic_error("Ladder collision owner changed");
        static_assert(noexcept(collisions.remove_for_cleanup(std::declval<Descriptor&>())),"Ladder collision teardown must not throw");
        if(!collision_owner_) {
            remove_collisions_=[this,&collisions]() noexcept {
                for(auto& descriptor:descriptors_)if(descriptor)collisions.remove_for_cleanup(*descriptor);
            };
            collision_owner_=&collisions;
        }
        flag90=true;
        auto bounds=this->backend_.actor_bounds(frame);
        const auto scale=this->backend_.actor_local_scale(frame);
        for(unsigned axis=0;axis!=3;++axis) {
            if(!std::isfinite(scale[axis])||scale[axis]==0)throw std::runtime_error("Ladder collision requires a finite nonzero local scale");
            bounds.minimum[axis]=static_cast<float>(static_cast<long double>(bounds.minimum[axis])*scale[axis]);
            bounds.maximum[axis]=static_cast<float>(static_cast<long double>(bounds.maximum[axis])*scale[axis]);
        }
        world_=this->backend_.actor_world_matrix(frame);
        for(unsigned axis=0;axis!=3;++axis) {
            const auto reciprocal=1.L/scale[axis];
            for(unsigned component=0;component!=3;++component)
                world_[axis*4+component]=static_cast<float>(reciprocal*world_[axis*4+component]);
        }
        inverse_=this->backend_.car_inverse_matrix(world_);
        typename Backend::Vector upper{{0,bounds.maximum[1],0,0}};
        upper=this->backend_.car_transform(upper,world_);
        bounds.maximum[1]=static_cast<float>(((static_cast<long double>(geometry->up_position()[1])-upper[1])+bounds.maximum[1])+.300000011920928955078125f);
        descriptors_[0]=std::make_unique<Descriptor>();
        auto& primary=*descriptors_[0];
        prepare(primary,bounds,frame);
        primary.radius=static_cast<float>((static_cast<long double>(bounds.maximum[1])-bounds.minimum[1])+.300000011920928955078125f);
        collisions.insert(primary); //Original inserts both, without006EF7F0.
        const auto upper_local=this->backend_.car_transform(geometry->up_position(),inverse_);
        bounds.minimum[1]=static_cast<float>(static_cast<long double>(upper_local[1])-.0074999998323619365692138671875f);
        bounds.minimum[2]=static_cast<float>(static_cast<long double>(bounds.minimum[2])-.100000001490116119384765625f);
        bounds.maximum[2]=static_cast<float>(static_cast<long double>(bounds.minimum[2])+.25f);
        descriptors_[1]=std::make_unique<Descriptor>();prepare(*descriptors_[1],bounds,frame);
        descriptors_[1]->radius=primary.radius;collisions.insert(*descriptors_[1]);
    }
    template<class Activation,class Collisions>void deactivate(bool authority,Activation& activation,Collisions& collisions) { //005EB120.
        frontend::CleanupErrors errors;
        errors.attempt([&]{this->base_.deactivate_frame(authority,activation);});
        if(flag90) {
            flag90=false;
            for(auto& descriptor:descriptors_)if(descriptor) {
                errors.attempt([&]{collisions.remove(*descriptor);});
                //Even a failed deferred-bucket allocation must not leave its
                //descriptor published after matching native destruction.
                if(descriptor->bucket)remove_collisions_();
                descriptor.reset();
            }
            field164.clear();
        }
        errors.rethrow();
    }
    const Descriptor* collision_descriptor(unsigned index) const {return descriptors_.at(index).get();}
    LadderActorProperties properties;
    std::optional<LadderGeometry<Backend>> geometry;
    std::vector<std::uint32_t> field164;
    bool flag90=false;
private:
    template<class Bounds>void prepare(Descriptor& descriptor,const Bounds& bounds,Frame frame) {
        descriptor.flags=0x40;descriptor.mask=0x4000;descriptor.actor=this;descriptor.frame=frame;
        const auto world=this->backend_.actor_world_matrix(frame);
        descriptor.position={{world[12],world[13],world[14],world[15]}};
        descriptor.matrix=&world_;descriptor.inverse=&inverse_;
        descriptor.minimum=bounds.minimum;descriptor.maximum=bounds.maximum;
    }
    typename Backend::Matrix world_{},inverse_{};
    std::array<std::unique_ptr<Descriptor>,2> descriptors_;
    std::function<void()> remove_collisions_;
    void* collision_owner_=nullptr;
};

template<class Backend>
class DoorSceneActor final : public SceneActor<Backend> {
public:
    using Frame=typename Backend::Frame;
    using Descriptor=RigidBodyCollisionDescriptor<Frame>;
    explicit DoorSceneActor(Backend& backend) : SceneActor<Backend>(backend,6),
        properties(initial_door_properties()) {} //005FF210.
    bool initialize(Frame frame) override {return this->base_.initialize_frame(frame);} //005FF3D0.
    void load(SerializedBytes& bytes) override { //00601AC0, including real00600A50.
        properties.load(bytes,[&] {portal.refresh(this->backend_,this->base_.frame(),properties.flaga0!=0);});
    }
    ~DoorSceneActor()override{if(remove_collision_)remove_collision_();}
    template<class Activation,class Collisions>void activate(bool authority,Activation& activation,Collisions& collisions) { //00600CC0.
        this->base_.activate_frame(authority,activation);const auto frame=this->base_.frame();
        if(!properties.flaga0)this->backend_.actor_link_preserving_world(frame,this->base_.field70());
        if(!flag90)portal.clear();
        if((activation.mode_flags()&2u)&&!authority&&properties.flaga0) {
            portal.refresh(this->backend_,frame,true);
            this->backend_.actor_link_preserving_world(frame,this->backend_.door_presentation_sector());
            this->backend_.update_scene_object(frame);save_pose();
            if(properties.fields220[1]==0)set_portals(true);
        }
        if(flag90){if(!collision_ready_)throw std::logic_error("Retire partial Door activation before retry");return;}
        if(!properties.name120.empty()) {
            if(const auto linked=this->backend_.find_scene_frame(properties.name120,0xc0001)) {
                const auto* data=this->backend_.find_actor_frame_data(linked);
                linked_door=data?reinterpret_cast<SceneActor<Backend>*>(data->words[0]):nullptr;
            }
        }
        this->base_.name(activation.actor_localized(properties.field114));
        voice_f0_=-1;field_f4_=0;
        handle104_=find_handle(frame,"dhandle",0);handle108_=find_handle(frame,"d2handle",0);
        flag10d_=true;flag10c_=false;field110_=0;field264=0;save_pose();
        initial_state_=properties.fields220[1];initial_lock_=properties.flag234;
        if((activation.mode_flags()&1u)&&properties.fields220[1]==1)change_state(2,true,false,false,activation);
        flag90=true;field9c=0;
        if(collision_owner_&&collision_owner_!=&collisions)throw std::logic_error("Door collision owner changed");
        static_assert(noexcept(collisions.remove_for_cleanup(descriptor_)),"Door collision teardown must not throw");
        if(!collision_owner_) {
            remove_collision_=[this,&collisions]()noexcept{collisions.remove_for_cleanup(descriptor_);};collision_owner_=&collisions;
        }
        const auto bounds=this->backend_.physics_object_bounds(frame,false);
        if(!(bounds.minimum[0]<bounds.maximum[0]))throw std::runtime_error("Door collision centre is unwritten for invalid aggregate bounds");
        if(descriptor_.bucket)collisions.remove(descriptor_);
        descriptor_=Descriptor{};descriptor_.flags=0x40;descriptor_.mask=0x1000;descriptor_.frame=frame;descriptor_.actor=this;
        descriptor_.minimum=bounds.minimum;descriptor_.maximum=bounds.maximum;
        descriptor_.matrix=this->backend_.actor_world_matrix_pointer(frame);
        inverse_=this->backend_.car_inverse_matrix(this->backend_.actor_world_matrix(frame));descriptor_.inverse=&inverse_;
        const auto scale=this->backend_.car_scale(frame);const auto yz=scale[1]>scale[2]?scale[1]:scale[2];
        descriptor_.radius=static_cast<float>(static_cast<long double>(scale[0]>yz?scale[0]:yz)*bounds.radius);
        descriptor_.position=this->backend_.car_transform(bounds.centre,this->backend_.actor_world_matrix(frame));
        if(properties.flag190)descriptor_.flags|=0x100000;
        collisions.insert(descriptor_); //Native does NOT follow this with006EF7F0.
        sound.position(saved_position_);sound.words[7]=1;sound.words[4]=1;sound.words[5]=sound.words[6]=0;
        collision_ready_=true;
    }
    template<class Activation,class Collisions>void deactivate(bool authority,Activation& activation,Collisions& collisions) { //00601350.
        frontend::CleanupErrors errors;errors.attempt([&]{set_portals(false);});linked_door=nullptr;flag90=false;collision_ready_=false;
        errors.attempt([&]{collisions.remove(descriptor_);});
        if(initial_state_)properties.fields220[1]=*initial_state_; //Unwritten124 on failed initialization is not fabricated.
        field94=0;errors.attempt([&]{this->base_.deactivate_frame(authority,activation);});errors.rethrow();
    }
    template<class Activation>void change_state(std::uint32_t command,bool auto_close,bool suppress_link,bool force,Activation& activation,unsigned depth=0) { //005FF4B0.
        if(depth>1024)throw std::runtime_error("Door linked-state recursion exceeds native bounded traversal");
        if((activation.mode_flags()&7u)==7u&&command==2&&!force&&activation.session_mission_ordinal()>12)
            throw std::runtime_error("Campaign Door opening requires its actual script-value protection guard");
        if(linked_door&&!suppress_link&&flag90) {
            auto* linked=dynamic_cast<DoorSceneActor*>(linked_door);
            if(!linked)throw std::runtime_error("Linked Door state requires an actual Door actor");
            linked->change_state(command,auto_close,true,force,activation,depth+1);
        }
        const auto* data=this->backend_.find_actor_frame_data(this->base_.frame());
        if(data&&data->words[1])activation.door_script_use(this->base_.frame(),command);
        if(command==2) {
            if(!force&&properties.flag234){write_door_loading_command(activation.session_broadcast_packets(),this->base_.id10(),2);return;}
            field268=field238_;field9c=auto_close&&properties.flag98?field238_+15000u:0;
            write_door_loading_command(activation.session_broadcast_packets(),this->base_.id10(),0);
            properties.fields220[1]=2;flag10c_=true;set_portals(false);
        }else if(command==3) {
            write_door_loading_command(activation.session_broadcast_packets(),this->base_.id10(),1);properties.fields220[1]=3;
        }
    }
    template<class Sounds>void receive_loading_packet(SessionPacketStream& packet,Sounds& sounds) { //00601F90.
        const auto command=packet.read(3);
        if(command==0||command==1) {
            if(command==0&&properties.fields240[0]!=0xffffffffu)emit_sound(properties.fields240[0],sounds);
            if(properties.fields240[2]==0xffffffffu)voice_f0_=-1;
            else if(voice_f0_==-1) {
                sound.words[8]=properties.fields240[2];sound.words[4]|=4u;
                voice_f0_=sounds.emit_door_sound(sound_request());flagf8_=false;sound.words[4]&=~4u;
            }else {sounds.resume_door_sound(voice_f0_);flagf8_=false;}
            properties.fields220[1]=command==0?2u:3u;
            if(command==0){flag10c_=flag10d_=true;set_portals(false);}return;
        }
        std::uint32_t sound=0xffffffffu;
        switch(command) {
        case 2:sound=properties.fields240[8];break;
        case 3:
            properties.flag234=0;
            if(linked_door) {
                auto* linked=dynamic_cast<DoorSceneActor*>(linked_door);
                if(!linked)throw std::runtime_error("Door unlock requires an actual linked Door");
                linked->properties.flag234=0;
            }
            sound=properties.fields240[4];break;
        case 4:properties.flag234=1;sound=properties.fields240[3];break;
        case 5:sound=properties.fields240[5];break;
        default:return; //Native6/7 default consumes the command without another effect.
        }
        if(sound!=0xffffffffu)emit_sound(sound,sounds);
    }
    template<class Runtime,class Collisions,class Queries>void frame_update(std::uint32_t elapsed,Runtime& runtime,Collisions& collisions,
                                                                          Queries& queries,DynamicCollisionExclusions<Descriptor>& exclusions) { //005FFFE0, ordinary rotating Door.
        if(!flag90||!collision_ready_)throw std::logic_error("Door update requires completed activation");
        if((handle104_||handle108_)&&flag10c_) {
            auto angle=static_cast<long double>(real_word(field110_))+(flag10d_?1.L:-1.L)*signed_word(elapsed)*.004999999888241291f;
            field110_=float_word(static_cast<float>(angle));
            if(flag10d_&&angle>.7853981852531433f){field110_=0x3f490fdbu;flag10d_=false;}
            else if(!flag10d_&&angle<0){field110_=0;flag10c_=false;}
            if(handle104_){this->backend_.door_roll_handle(handle104_,real_word(field110_));this->backend_.update_scene_object(handle104_);}
            if(handle108_){this->backend_.door_roll_handle(handle108_,real_word(field110_));this->backend_.update_scene_object(handle108_);}
        }
        if(field9c) {
            field9c-=elapsed;
            if(signed_word(field9c)<=0) {
                field9c=0;
                if(properties.fields220[1]==1) {
                    bool defer=false;
                    if((runtime.mode_flags()&7u)==7u)if(const auto player=runtime.door_current_player_frame()) {
                        const auto position=this->backend_.actor_world_matrix(player);const auto& origin=this->base_.world_position();
                        const auto x=static_cast<long double>(origin[0])-position[12],y=static_cast<long double>(origin[1])-position[13],z=static_cast<long double>(origin[2])-position[14];
                        defer=(x*x+y*y)+z*z<100.f||this->backend_.door_recently_visible(this->base_.frame());
                    }
                    if(defer)field9c=2000;else change_state(3,true,false,false,runtime);
                }
            }
        }
        auto state=properties.fields220[1];if(state==0||state==1)return;
        if(properties.fields220[0]!=0)throw std::runtime_error("Sliding Door first update requires its actual translation/collision receiver");
        const auto previous=field264;
        if(field268){field268-=elapsed;if(signed_word(field268)>0)return;field268=0;}
        if(state==2) {
            const auto next=static_cast<long double>(elapsed)*.0010000000474974513f*real_word(properties.fields220[3])+real_word(field264);
            field264=float_word(static_cast<float>(next));
            if(real_word(properties.fields220[2])<next) {
                properties.fields220[1]=1;field264=properties.fields220[2];
                if(voice_f0_!=-1)runtime.pause_door_sound(voice_f0_);
            }
        }else if(state==3) {
            const auto next=static_cast<long double>(real_word(field264))-static_cast<long double>(elapsed)*.0010000000474974513f*real_word(properties.fields220[3]);
            field264=float_word(static_cast<float>(next));
            if(next<0) {
                if(voice_f0_!=-1)runtime.pause_door_sound(voice_f0_);
                properties.fields220[1]=0;field264=0;
                if(properties.fields240[1]!=0xffffffffu)emit_sound(properties.fields240[1],runtime);
                bool linked_closed=true;
                if(linked_door) {
                    const auto* linked=dynamic_cast<const DoorSceneActor*>(linked_door);
                    if(!linked)throw std::runtime_error("Door close requires an actual linked Door");
                    linked_closed=linked->properties.fields220[1]==0;
                }
                if(linked_closed)set_portals(true);
            }
        }
        if(rotate_and_probe(queries,exclusions)) {
            if(voice_f0_!=-1){field_f4_=runtime.door_sound_cursor(voice_f0_);runtime.pause_door_sound(voice_f0_);}
            flagf8_=true;field264=previous;field268=200;refresh_collision(collisions);return;
        }
        if(voice_f0_!=-1&&flagf8_){runtime.door_sound_cursor(voice_f0_,field_f4_);runtime.resume_door_sound(voice_f0_);}
        flagf8_=false;refresh_collision(collisions);
    }
    const Descriptor& collision_descriptor()const{return descriptor_;}
    Frame first_handle()const{return handle104_;}Frame second_handle()const{return handle108_;}
    const auto& saved_position()const{return saved_position_;}const auto& saved_rotation()const{return saved_rotation_;}
    std::uint8_t initial_lock()const{return initial_lock_;}
    DoorActorProperties properties;
    DoorPortalBinding<Backend> portal;
    SoundEmission sound;
    std::array<std::uint32_t,24> fields130{};
    std::string field180;
    bool flag90=false;
    std::uint32_t field94=0,field9c=0,field18c=0,field1a0=0,field264=0,field268=0;
    SceneActor<Backend>* linked_door=nullptr; //Actual native11C borrower, never a pointer in a DWORD.
private:
    static float real_word(std::uint32_t word){return frontend::sound_float_word(word);}
    static std::uint32_t float_word(float value){std::uint32_t result;std::memcpy(&result,&value,4);return result;}
    static std::int32_t signed_word(std::uint32_t value){return frontend::sound_signed_word(value);}
    template<class Collisions>void refresh_collision(Collisions& collisions) { //006007F0.
        const auto frame=this->base_.frame();const auto centre=this->backend_.door_sphere_centre(frame);
        inverse_=this->backend_.car_inverse_matrix(this->backend_.actor_world_matrix(frame));
        const bool along_x=(properties.vector210[0]==0x3f800000u||properties.vector210[0]==0xbf800000u)&&real_word(properties.vector210[1])==0&&real_word(properties.vector210[2])==0;
        descriptor_.mask=properties.fields220[0]==0&&properties.fields220[1]==2&&!along_x?0x1002u:0x1000u;
        descriptor_.position=centre;collisions.move(descriptor_);
    }
    template<class Queries>bool rotate_and_probe(Queries& queries,DynamicCollisionExclusions<Descriptor>& exclusions) { //005FF700.
        const auto frame=this->base_.frame();const auto previous=this->backend_.actor_rotation(frame);
        typename Backend::Vector axis{};for(unsigned a=0;a!=3;++a)axis[a]=real_word(properties.vector210[a]);
        const auto rotation=this->backend_.door_axis_rotation(axis,real_word(field264));
        this->backend_.object_rotation(frame,door_rotation_product(rotation,saved_rotation_));this->backend_.update_scene_object(frame);
        if(properties.fields220[1]==2)return false;
        if((properties.vector210[0]==0x3f800000u||properties.vector210[0]==0xbf800000u)&&axis[1]==0&&axis[2]==0)return false;
        exclusions.single(&descriptor_);
        struct Clear {DynamicCollisionExclusions<Descriptor>& owner;~Clear(){owner.clear();}}clear{exclusions};
        const auto bounds=this->backend_.physics_object_bounds(frame,false);
        if(!(bounds.minimum[0]<bounds.maximum[0]))throw std::runtime_error("Door edge probes require valid aggregate bounds");
        std::array<typename Backend::Vector,8> points{};const auto matrix=this->backend_.actor_world_matrix(frame);
        for(unsigned x=0;x!=2;++x)for(unsigned y=0;y!=2;++y)for(unsigned z=0;z!=2;++z) {
            typename Backend::Vector point{{
                static_cast<float>(x?static_cast<long double>(bounds.maximum[0])+.009999999776482582f:static_cast<long double>(bounds.minimum[0])-.009999999776482582f),
                static_cast<float>(y?static_cast<long double>(bounds.maximum[1])+.009999999776482582f:static_cast<long double>(bounds.minimum[1])-.009999999776482582f),
                static_cast<float>(z?static_cast<long double>(bounds.minimum[2])-.009999999776482582f:static_cast<long double>(bounds.maximum[2])+.009999999776482582f),0}};
            points[x*4+y*2+z]=this->backend_.car_transform(point,matrix);
        }
        constexpr std::array<std::array<unsigned,2>,14> edges{{{{2,3}},{{3,7}},{{7,6}},{{6,2}},{{0,2}},{{1,3}},{{5,7}},{{4,6}},{{0,1}},{{1,5}},{{5,4}},{{4,0}},{{1,6}},{{2,5}}}};
        for(const auto edge:edges) {
            typename Backend::Vector displacement{};
            for(unsigned a=0;a!=3;++a)displacement[a]=static_cast<float>(static_cast<long double>(points[edge[1]][a])-points[edge[0]][a]);
            if(queries.blocked(points[edge[0]],displacement,2,0)) {
                exclusions.clear();this->backend_.object_rotation(frame,previous);this->backend_.update_scene_object(frame);return true;
            }
        }
        return false;
    }
    void save_pose(){
        const auto world=this->backend_.actor_world_matrix(this->base_.frame());
        saved_position_={{world[12],world[13],world[14],world[15]}};saved_rotation_=this->backend_.actor_rotation(this->base_.frame());
    }
    void set_portals(bool enabled) { //006007B0: virtual00(closed) on BOTH borrowed portals.
        if(const auto value=portal.portal())this->backend_.door_portal_closed(value,!enabled);
        if(const auto value=portal.attachment())this->backend_.door_portal_attachment_closed(value,!enabled);
    }
    static std::string lower_name(std::string value) {
        if(value.size()>=32)throw std::length_error("Door short name exceeds native32-byte search buffer");
        for(auto& c:value)if(c>='A'&&c<='Z')c=static_cast<char>(c+('a'-'A'));
        return value;
    }
    Frame find_handle(Frame frame,const char* pattern,unsigned depth) { //00600950, reverse child then subtree, own name last.
        if(depth>1024)throw std::runtime_error("Door handle hierarchy exceeds traversal bounds");
        for(auto n=this->backend_.actor_child_count(frame);n;) {
            const auto child=this->backend_.actor_child(frame,--n);
            if(!child)throw std::runtime_error("Door handle hierarchy contains a null child");
            if(lower_name(this->backend_.actor_short_name(child)).find(pattern)!=std::string::npos)return child;
            if(const auto result=find_handle(child,pattern,depth+1))return result;
        }
        return lower_name(this->backend_.actor_short_name(frame)).find(pattern)!=std::string::npos?frame:Frame{};
    }
    frontend::SpatialSoundRequest sound_request()const {
        frontend::SpatialSoundRequest request;
        for(unsigned n=0;n!=4;++n)request.position[n]=frontend::sound_float_word(sound.words[n]);
        if(sound.words[5]||sound.words[6]||sound.words[9])throw std::runtime_error("Door sound requires its actual typed relative/source/filename producer");
        request.flags=sound.words[4];request.group=frontend::sound_signed_word(sound.words[7]);request.entry=frontend::sound_signed_word(sound.words[8]);
        request.near_range=frontend::sound_float_word(sound.words[10]);request.far_range=frontend::sound_float_word(sound.words[11]);
        request.voice_group=sound.words[12];request.near_scale=frontend::sound_float_word(sound.words[13]);
        request.far_scale=frontend::sound_float_word(sound.words[14]);request.volume=frontend::sound_float_word(sound.words[15]);return request;
    }
    template<class Sounds>void emit_sound(std::uint32_t id,Sounds& sounds){sound.words[8]=id;(void)sounds.emit_door_sound(sound_request());}
    Descriptor descriptor_{};typename Backend::Matrix inverse_{};std::function<void()> remove_collision_;void* collision_owner_=nullptr;
    Frame handle104_{},handle108_{};typename Backend::Vector saved_position_{};std::array<float,4> saved_rotation_{};
    std::optional<std::uint32_t> initial_state_;std::uint8_t initial_lock_=0;
    std::int32_t voice_f0_=-1;std::uint32_t field_f4_=0,field110_=0,field238_=0;
    bool flag10c_=false,flag10d_=true,flagf8_=false,collision_ready_=false;
};

template<class Backend,class Runtime>
class SwitchSceneActor final : public SceneActor<Backend> {
public:
    using Frame=typename Backend::Frame;
    using Descriptor=RigidBodyCollisionDescriptor<Frame>;
    SwitchSceneActor(Backend& backend,Runtime& runtime,std::string localized_name)
        : SceneActor<Backend>(backend,13),state(backend),runtime_(runtime) {
        this->base_.name(std::move(localized_name)); //00613450, name1BF3.
    }
    bool initialize(Frame frame) override { //00613550.
        return this->base_.initialize_frame(frame)&&state.initialize(frame,runtime_);
    }
    void load(SerializedBytes& bytes) override {state.load(bytes,runtime_);} //00613E10.
    ~SwitchSceneActor() override {if(remove_collision_)remove_collision_();}
    template<class Activation,class Collisions>void activate(bool authority,Activation& activation,Collisions& collisions) { //00613920.
        this->base_.activate_frame(authority,activation);
        if(!authority&&(activation.mode_flags()&1))return;
        const auto frame=this->base_.frame();if(!frame)return;
        if(collision_owner_&&collision_owner_!=&collisions)throw std::logic_error("Switch collision owner changed");
        static_assert(noexcept(collisions.remove_for_cleanup(descriptor_)),"Switch collision teardown must not throw");
        if(!collision_owner_) {
            remove_collision_=[this,&collisions]() noexcept {collisions.remove_for_cleanup(descriptor_);};
            collision_owner_=&collisions;
        }
        //Original overwritesF0; retract a repeated/partially registered native
        //descriptor first so an exception never strands its borrowed address.
        if(descriptor_.bucket)collisions.remove(descriptor_);
        const auto bounds=this->backend_.physics_object_bounds(frame,false);
        if(!(bounds.minimum[0]<bounds.maximum[0]))throw std::runtime_error("Switch collision centre is unwritten for invalid bounds");
        descriptor_=Descriptor{};descriptor_.flags=0x40;descriptor_.mask=1;descriptor_.frame=frame;
        descriptor_.minimum=bounds.minimum;descriptor_.maximum=bounds.maximum;
        descriptor_.matrix=this->backend_.actor_world_matrix_pointer(frame);
        inverse_=this->backend_.car_inverse_matrix(this->backend_.actor_world_matrix(frame));descriptor_.inverse=&inverse_;
        const auto scale=this->backend_.car_scale(frame);
        const auto yz=scale[1]>scale[2]?scale[1]:scale[2];
        descriptor_.radius=static_cast<float>(static_cast<long double>(scale[0]>yz?scale[0]:yz)*bounds.radius);
        descriptor_.position=this->backend_.car_transform(bounds.centre,this->backend_.actor_world_matrix(frame));
        descriptor_.actor=this;
        collisions.insert(descriptor_);collisions.move(descriptor_);state.begin_activation();
    }
    template<class Activation,class Collisions>void deactivate(bool authority,Activation& activation,Collisions& collisions) { //00613BD0.
        frontend::CleanupErrors errors;
        errors.attempt([&]{this->base_.deactivate_frame(authority,activation);});
        errors.attempt([&]{collisions.remove(descriptor_);});
        if(authority)errors.attempt([&]{state.restore_activation(runtime_);});
        errors.rethrow();
    }
    const Descriptor& collision_descriptor() const {return descriptor_;}
    SwitchActorState<Backend> state;
private:
    Runtime& runtime_;
    typename Backend::Matrix inverse_{};
    Descriptor descriptor_{};
    std::function<void()> remove_collision_;
    void* collision_owner_=nullptr;
};

} //namespace hd2::game
#endif
