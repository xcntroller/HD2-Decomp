#ifndef HD2_GAME_HUMAN_PHYSICS_LOADING_H
#define HD2_GAME_HUMAN_PHYSICS_LOADING_H

#include "game/human_collision_markers.h"
#include "game/human_look_control.h"
#include "game/human_morph_tables.h"
#include "game/rigid_body.h"
#include "game/collision_tree.h"
#include "game/dynamic_collision_exclusions.h"
#include "game/human_state_ground.h"
#include "game/human_physics_globals.h"
#include "game/human_physical_frame_words.h"
#include "game/human_state_frame_timers.h"
#include "frontend/spatial_sound.h"
#include <deque>
#include <functional>

namespace hd2::game {
template<class>class SceneActor;


//Loading0075B280/0075AE40/0075B870 and the descriptor portion of0075B6C0.
//Uses the containing world's SAME buckets/exclusions and its real face/audio
//owners. This is a DD0 dependency, not an activated actor or simulation service.
//Runtime posture transitions, voice-producing commands and animation teardown
//remain with the containing control and are not exposed as successful stubs.
template<class Frames,class Human,class Collisions,class Face,class Audio>
class HumanPhysicsLoading {
public:
    using Frame=typename Frames::Frame;
    using Descriptor=RigidBodyCollisionDescriptor<Frame>;
    using Vector=std::array<float,4>;
    using SoundOwner=decltype(std::declval<Frames&>().create_human_sound());
    HumanPhysicsLoading(Frames& frames,Collisions& collisions,DynamicCollisionExclusions<Descriptor>& exclusions,
        Face& face,Audio& audio,HumanPhysicsGlobals& globals,std::function<Frame()> shadow)
        :frames_(frames),collisions_(collisions),exclusions_(exclusions),face_(face),audio_(audio),globals_(globals),markers_(frames),shadow_(std::move(shadow)) {
        if(!shadow_)throw std::invalid_argument("Human physics requires the SAME DD0 A9C shadow lookup");
        state_.set(0x1c8,.8f);state_.set(0x1cc,1.f);state_.set(0x1d0,1.f);
        state_.set(0x994,std::int32_t{-1});
    }
    ~HumanPhysicsLoading(){clear_descriptors();}
    HumanPhysicsLoading(const HumanPhysicsLoading&)=delete;
    HumanPhysicsLoading& operator=(const HumanPhysicsLoading&)=delete;
    void bind(Human& human) { //0075AE00; actual geometry borrower is held by DD0.
        if(active_||initializing_)throw std::logic_error("Cannot rebind live Human physical descriptors");
        human_=&human;state_.set(0x22c,std::uint32_t{0x1020});
        for(auto offset:{0x698u,0x69cu,0x984u})state_.set(offset,std::int32_t{0});
        state_.set(0x18c,std::uint8_t{1});
    }
    void initialize(bool collision_disabled) {
        if(!human_||!human_->base().frame())throw std::runtime_error("Human physics requires an actual bound model");
        if(active_||initializing_)throw std::logic_error("Human physics initialized twice");
        initializing_=true;
        try {
            markers_.bind(human_->base().frame());reset();
            for(auto offset:{0x980u,0x984u,0x698u,0x69cu})state_.set(offset,std::int32_t{0});
            for(unsigned index=0;index!=4;++index) {
                primary_[index]=make(0x42,0x101);secondary_[index]=make(0x42,0x101);
            }
            if(!collision_disabled) {
                state_.set(0x698,std::int32_t{3});
                if(markers_.copy_posture(0,posture_)!=3) {
                    state_.set(0x698,std::int32_t{-10000});
                    throw std::runtime_error("Human standing collider markers are incomplete");
                } //Native continues into negative descriptor indexing; reject malformed input.
                //0075BC80(0) registers zero-initialized descriptors FIRST.
                for(int index=0;index<primary_count();++index)collisions_.insert(*primary_[static_cast<std::size_t>(index)]);
                for(int index=0;index<primary_count();++index) {
                    const auto slot=static_cast<std::size_t>(index);
                    primary_[slot]->position=secondary_[slot]->position=posture_[slot];
                    primary_[slot]->radius=static_cast<float>(static_cast<long double>(state_.template get<float>(0x1c8))*posture_[slot][3]);
                    secondary_[slot]->radius=posture_[slot][3];
                }
            }
            shot_spheres();
            for(unsigned index=0;index!=2;++index) {
                shot_[index]=make(0x42000042,0x102);shot_[index]->position=shots_[index];
                shot_[index]->radius=static_cast<float>(static_cast<long double>(state_.template get<float>(index?0x1d0:0x1cc))*shots_[index][3]);
                if(!collision_disabled)collisions_.insert(*shot_[index]);
            }
            for(unsigned index=0;index!=bones_.size();++index) {
                bones_[index]=frames_.actor_find(human_->base().frame(),human_physics_bone_names[index],0xffff);
                bone_flags_[index]=false;
            }
            face_.bind(*human_);face_.schedule(true);
            const auto& position=human_->base().world_position();
            for(unsigned axis=0;axis!=3;++axis)offset740_[axis]=static_cast<float>(static_cast<long double>(posture_[2][axis])-position[axis]);
            constexpr std::array<unsigned,4> moduli{{200,300,400,600}};
            for(unsigned index=0;index!=4;++index)state_.set(0xa80+index*4,audio_.random_source()()%moduli[index]);
            ++globals_.active;active_=true;initializing_=false;
        }catch(...) {clear_descriptors();throw;}
    }
    //Called after DD0's actual sound/face/animation cleanup. Also provides
    //exception rollback for partial loading; it never invents those receivers.
    void clear_descriptors() noexcept {
        for(unsigned index=0;index!=4;++index) {
            retire(primary_[index]);retire(secondary_[index]);
        }
        queue_.clear();ground_contact_.reset();
        for(auto& descriptor:shot_)retire(descriptor);
        state_.set(0x698,std::int32_t{-1});
        state_.set(0x1c8,.8f);state_.set(0x1cc,1.f);state_.set(0x1d0,1.f);
        if(active_){--globals_.active;active_=false;}
        initializing_=false;
    }
    auto query_descriptors() const { //0075C960(false)'s exact primary,shot,secondary order.
        std::vector<Descriptor*> result;
        const auto append=[&](const auto& descriptors,int count) {
            if(count>static_cast<int>(descriptors.size()))throw std::runtime_error("Human descriptor count exceeds original storage");
            for(int index=0;index<count;++index) {
                auto* descriptor=descriptors[static_cast<std::size_t>(index)].get();
                if(!descriptor)throw std::runtime_error("Human query descriptor has no allocation");
                result.push_back(descriptor);
            }
        };
        append(primary_,primary_count());append(shot_,state_.template get<std::int32_t>(0x69c));
        append(secondary_,state_.template get<std::int32_t>(0x984));return result;
    }
    auto& exclusions(){return exclusions_;}
    void exclude_query(bool remove_buckets) { //0075C960, SAME primary/shot/secondary descriptors.
        auto descriptors=query_descriptors();
        if(remove_buckets)for(auto* descriptor:descriptors)collisions_.remove(*descriptor);
        else exclusions_.many(descriptors.data(),descriptors.size(),nullptr);
    }
    void restore_query(bool insert_buckets) { //0075CAC0; false clears the shared exclusion list.
        if(insert_buckets)for(auto* descriptor:query_descriptors())collisions_.insert(*descriptor);
        else exclusions_.clear();
    }
    void falling(bool value){state_.set(0xa90,std::uint8_t(value));}
    template<class Pool>void close_audio(std::uint32_t mode,Pool& pool) {
        //0075EAA0(true): native198's final entry first; mode2 gates returns.
        if(mode&2u)while(!sounds_.empty()) {
            auto sound=std::move(sounds_.back());sounds_.pop_back();pool.give(std::move(sound));
        }
        stop_frame_voice();
    }
    void stop_frame_voice(){ //0075EEB0, SAME actual native994 handle, also called by00777AD0.
        auto handle=state_.template get<std::int32_t>(0x994);
        if(handle!=-1) { //0075EEB0 passes the actual handle by address to SOUND+5C.
            try{audio_.stop_voice(handle);}catch(...){state_.set(0x994,handle);throw;}
            state_.set(0x994,handle);
        }
    }
    template<class Services>void play_frame_climb_voice(Services& services){ //0075EDE0, actual Ladder178 and shared SOUND handle994.
        if(!interaction_actor_)return;
        const auto entry=human_frame_signed_word(services.human_ladder(interaction_actor_).properties.settings170[2]);
        if(entry==-1||state_.template get<std::int32_t>(0x994)!=-1)return;
        if(!human_||!bones_[16])throw std::runtime_error("Human climb voice requires its actual actor and relative bone");
        frontend::SpatialSoundRequest request;
        request.position={{0,-.4f,0}};request.flags=5;request.relative=bones_[16];request.source=human_;
        request.group=0;request.entry=entry;request.voice_group=4;
        state_.set(0x994,services.emit_human_equipment_sound(request));
    }
    template<class Services,class Pool>void play_frame_presentation_sound(unsigned group,Services& services,Pool& pool){ //0075E770.
        if(!(services.mode_flags()&2u))return;
        if(!human_)throw std::logic_error("Human presentation sound requires its actual actor");
        if(services.current_human_frame_player()!=human_||group<4||group>=20)return;
        const auto marker=bones_[7]; //Native864, SAME cached bone/flag records.
        if(services.human_is_player(*human_)&&services.human_view_offset_enabled()){
            if(!marker)throw std::runtime_error("Human presentation sound requires its actual orientation bone");
            (void)frames_.car_forward(marker); //75E7E5..75E865, actual quaternion/matrix cache side effects.
        }
        auto sound=pool.play(group,marker,{{0,0,0}});
        if(sound)sounds_.push_back(std::move(sound)); //Matching0075EAA0 returns this SAME owner to the shared pool.
    }
    template<class CurrentPlayer,class Pool>void retire_frame_audio(std::uint32_t mode,CurrentPlayer&& current_player,Pool& pool){ //0075EAA0(false).
        if(!(mode&2u))return;
        if(!current_player()){
            while(!sounds_.empty()){
                auto sound=std::move(sounds_.back());sounds_.pop_back();pool.give(std::move(sound));
            }
            return;
        }
        for(std::size_t n=0;n<sounds_.size();){
            if(frames_.car_sound_state(sounds_[n].get())){
                ++n;
                continue;
            }
            auto sound=std::move(sounds_[n]);
            if(n!=sounds_.size()-1)sounds_[n]=std::move(sounds_.back());
            sounds_.pop_back();
            pool.give(std::move(sound));
        }
    }
    void enable(bool value){state_.set(0x18c,std::uint8_t(value));} //005AB330's final nonplayer gate.
    void add_control_flags(std::uint32_t mask) { //005AC390, same physical22C.
        const auto old=state_.template get<std::uint32_t>(0x22c);
        if(!(old&8u)&&(mask&8u))suspend_collisions(true,0);
        state_.set(0x22c,old|mask);
    }
    template<class Look,class Sweep>void clear_control_flags(std::uint32_t mask,const Look& look,Sweep& sweep) { //0075C770.
        const auto old=state_.template get<std::uint32_t>(0x22c),next=old&~mask;
        state_.set(0x22c,next);
        if((mask&8u)&&(old&8u))suspend_collisions(false,look.state().template get<std::uint32_t>(0x4c));
        if(!(next&6u)&&(old&6u)&&next!=old)reposition_human_state_on_ground(frames_,*human_,*this,look,sweep);
    }
    const auto& state() const{return frame_words_;}
    auto& frame_globals(){return globals_;}
    template<class Look>void capture_look_frame(const Look& look){ //0077DE30's retained A0-byte770 snapshot.
        for(unsigned offset=0;offset!=0xa0;offset+=4)
            if(offset!=4&&offset!=8)state_.set(0x770+offset,look.state().template get<std::uint32_t>(offset));
        look_snapshot_model_=look.action_model();look_snapshot_actor_=look.interaction_actor();
        look_snapshot_argument_=look.interaction_frame_argument(); //Native804, typed94 frame alternative in the retained snapshot.
    }
    Frame look_snapshot_model()const{return look_snapshot_model_;}
    SceneActor<Frames>* look_snapshot_actor()const{return look_snapshot_actor_;}
    Frame look_snapshot_argument()const{return look_snapshot_argument_;}
    auto& control_frame_words(){return frame_words_;}
    void bind_animation_tracks(HumanJointWords<0x3a4>& tracks){frame_words_.bind(tracks);}
    void unbind_animation_tracks()noexcept{frame_words_.unbind();}
    auto& control_action_queue(){return queue_;} //DD0+A4C, SAME deque retired by0075B6C0.
    void rebind_frame_posture(std::uint32_t posture){rebind_posture(posture);} //0075BC80.
    float control_projection_height(float model_height)const{ //00770100, unscaled marker DD0+6E0.
        const auto count=primary_count();
        if(count<1)return model_height;
        const auto& upper=posture_.at(static_cast<std::size_t>(count-1));
        return upper[1]+upper[3];
    }
    float control_camera_projection(std::uint32_t tick,float field_of_view,float scale){ //00770100.
        if(globals_.camera_projection_tick!=tick){
            const float half_fov=field_of_view*.5f;
            globals_.camera_projection_tick=tick;
            globals_.camera_projection_scale=static_cast<float>(std::tan(static_cast<long double>(half_fov))/scale);
        }
        return globals_.camera_projection_scale;
    }
    HumanVector current_direction_override()const{return state_.template get<HumanVector>(0x240);}
    void invalidate_current_direction(){ //0077F460 invalidates XYZ only; native24C comes from unwritten stack.
        for(unsigned axis=0;axis!=3;++axis)state_.set(0x240+axis*4,-10000.099609375f);
    }
    void target_direction(const HumanVector& value){state_.set(0x940,value);} //0077F690, AFTER geometry receiver.
    void equipment_transfer_mode(std::uint32_t command,std::uint32_t mode) { //00792590/00792720 DD0 stores.
        state_.set(0x7c4,command);state_.set(0xa20,mode);
    }
    void* interaction_actor()const { //005AC2E0. Native990 is an actor identity borrower, never a DWORD address.
        const auto animation=state_.template get<std::int32_t>(0x1d8);
        return (animation>=106&&animation<=108)||animation==8||animation==14?interaction_actor_:nullptr;
    }
    auto* control_action_actor()const{return interaction_actor_;}
    void control_action_actor(void* actor){interaction_actor_=actor;}
    SceneActor<Frames>* control_interaction_target()const{return interaction_target_;} //Native A3C, borrowed actual action target.
    void control_interaction_target(SceneActor<Frames>* actor){interaction_target_=actor;}
    Frame control_action_frame()const{return action_frame_;} //Native A40 frame alternative, borrowed actual model.
    void control_action_frame(Frame frame){action_frame_=frame;}
    void animation_loading_word(unsigned offset,std::uint32_t value) {
        if(offset!=0x750)throw std::out_of_range("Human animation loading scalar");
        state_.set(offset,value);
    }
    struct GroundContact {std::optional<CollisionCellReference> static_identity;const Descriptor* dynamic_identity;};
    const auto& ground_contact()const{return ground_contact_;}
    void clear_ground_contact(){ground_contact_.reset();} //Native754 is a typed identity borrower.
    template<class Hit>void ground_contact(const Hit& hit){ground_contact_=GroundContact{hit.static_identity,hit.dynamic_identity};}
    const auto& primary() const{return primary_;}
    const auto& secondary() const{return secondary_;}
    const auto& shots() const{return shot_;}
    const auto& shot_relative() const{return shot_relative_;}
    const auto& bones() const{return bones_;}
    const auto& offset740() const{return offset740_;}
    const auto& posture_spheres()const{return posture_;}
    void prepare_ladder_secondary(){state_.set(0x984,state_.template get<std::uint32_t>(0x698));} //0074BBE0.
    void activate_step_secondary(std::int32_t duration){ //00779E60, native980 before ascending inserts of the SAME970 descriptors.
        state_.set(0x980,duration);
        const auto count=state_.template get<std::int32_t>(0x984);
        for(std::int32_t index=0;index<count;++index)collisions_.insert(required(secondary_,index));
    }
    void retire_step_secondary(){ //00787270, ascending006EF5E0 calls on the SAME970 descriptors.
        const auto count=state_.template get<std::int32_t>(0x984);
        for(std::int32_t index=0;index<count;++index)collisions_.remove(required(secondary_,index));
    }
    void publish_step_secondary(const Vector& forward,float height,bool shortened){ //0074A690's actual secondary descriptors.
        if(shortened)state_.set(0x984,state_.template get<std::uint32_t>(0x984)-1u);
        const auto count=state_.template get<std::int32_t>(0x984);
        for(int index=0;index<count;++index){
            auto& descriptor=required(secondary_,index);
            const auto& sphere=posture_.at(static_cast<std::size_t>(index));
            descriptor.position[0]=forward[0]+sphere[0];descriptor.position[1]=height+sphere[1];
            descriptor.position[2]=forward[2]+sphere[2];descriptor.radius=sphere[3];
        }
    }
    void publish_ladder_secondary(const Vector& origin,const Vector& step){ //0074BBE0, SAME allocated secondary descriptors.
        const auto count=state_.template get<std::int32_t>(0x984);
        for(int index=0;index<count;++index){
            auto& descriptor=required(secondary_,index);
            for(unsigned axis=0;axis!=3;++axis)
                descriptor.position[axis]=static_cast<float>(static_cast<long double>(step[axis])*index+origin[axis]);
            descriptor.radius=posture_.at(static_cast<std::size_t>(index))[3];
        }
    }
    void refresh_ground_geometry(){markers_.snapshot();shot_spheres();} //0075C4F0→0075C070/0075B870.
    void refresh_shot_geometry(){shot_spheres();} //0075B870, SAME actual shot marker snapshots.
    void publish_air_shots(){ //00769920, scaled positions/radii without changing the native descriptor flags.
        const auto count=state_.template get<std::int32_t>(0x69c);
        for(int index=0;index<count;++index){
            auto& descriptor=required(shot_,index);
            descriptor.position=shots_.at(static_cast<std::size_t>(index));
            descriptor.radius=static_cast<float>(static_cast<long double>(state_.template get<float>(index?0x1d0:0x1cc))*descriptor.position[3]);
            collisions_.move(descriptor);
        }
    }
    void publish_retired_mount_shots(){ //0075E290, current DD0 descriptors after a separate0075B870 call.
        state_.set(0x1cc,1.f);state_.set(0x1d0,1.f);
        const auto count=state_.template get<std::int32_t>(0x69c);
        if(count>2)throw std::runtime_error("Human mount retirement exceeds actual shot descriptors");
        for(int n=0;n<count;++n){
            auto& descriptor=required(shot_,n);descriptor.position=shots_[static_cast<std::size_t>(n)];
            descriptor.radius=static_cast<float>(static_cast<long double>(state_.template get<float>(n?0x1d0:0x1cc))*descriptor.position[3]);
            descriptor.flags=0x42;descriptor.mask=0x10002;collisions_.move(descriptor);
        }
    }
    void translate_ground_geometry(const Vector& position){ //00763EC0 calls0075C250 then0075BBF0.
        markers_.translate(position);state_.set(0x69c,std::int32_t{2});
        for(unsigned index=0;index!=2;++index)for(unsigned axis=0;axis!=3;++axis)
            shots_[index][axis]=shot_relative_[index][axis]+position[axis];
    }
    void publish_ground_geometry(std::uint32_t posture){ //00763EC0 descriptor tail, actual SAME buckets.
        rebind_posture(posture);
        for(int index=0;index<primary_count();++index){
            auto& descriptor=required(primary_,index);descriptor.position=posture_.at(static_cast<std::size_t>(index));
            descriptor.radius=static_cast<float>(static_cast<long double>(descriptor.position[3])*state_.template get<float>(0x1c8));
            collisions_.move(descriptor);
        }
        for(int index=0;index<state_.template get<std::int32_t>(0x69c);++index){
            auto& descriptor=required(shot_,index);descriptor.position=shots_.at(static_cast<std::size_t>(index));
            descriptor.radius=static_cast<float>(static_cast<long double>(state_.template get<float>(index?0x1d0:0x1cc))*descriptor.position[3]);
            collisions_.move(descriptor);
        }
    }
    void suspend_collisions(bool suspended,std::uint32_t posture) { //0075C2D0.
        if(!human_)throw std::logic_error("Human collision transition has no bound Human");
        if(suspended) {
            const auto primary=primary_count(),shots=state_.template get<std::int32_t>(0x69c);
            if(primary>4||shots>2)throw std::runtime_error("Human collision transition exceeds owned allocations");
            for(int n=0;n<primary;++n)collisions_.remove(required(primary_,n));
            state_.set(0x698,std::int32_t{0});
            for(int n=0;n<shots;++n)collisions_.remove(required(shot_,n));
            state_.set(0x69c,std::int32_t{0});
        }else {
            markers_.snapshot();rebind_posture(posture);
            for(int n=0;n<primary_count();++n) {
                auto& descriptor=required(primary_,n);descriptor.position=posture_[static_cast<std::size_t>(n)];
                descriptor.radius=static_cast<float>(static_cast<long double>(state_.template get<float>(0x1c8))*descriptor.position[3]);
                collisions_.move(descriptor);
            }
            shot_spheres();
            for(int n=0;n<state_.template get<std::int32_t>(0x69c);++n) {
                auto& descriptor=required(shot_,n);descriptor.position=shots_[static_cast<std::size_t>(n)];
                descriptor.radius=static_cast<float>(static_cast<long double>(state_.template get<float>(n?0x1d0:0x1cc))*descriptor.position[3]);
                descriptor.flags=0x42;descriptor.mask=2;
                if(static_cast<std::int32_t>(human_->properties.stats_ec[0])<1)descriptor.mask|=0x10000;
                collisions_.insert(descriptor);
            }
        }
        if(const auto shadow=shadow_())frames_.object_visible(shadow,!suspended); //SAME DD0+A9C, not a second zero-only borrower.
    }
    int primary_count() const{return state_.template get<std::int32_t>(0x698);}
    Human** human_slot_address(){return &human_;}
private:
    Frame look_snapshot_model_{},look_snapshot_argument_{};SceneActor<Frames>* look_snapshot_actor_=nullptr; //Native774/778/804 borrowed identities.
    std::vector<SoundOwner> sounds_; //Native198, populated by0075E770 and returned by0075EAA0.
    void rebind_posture(std::uint32_t requested) { //0075BC80 ordinary descriptor branch.
        if(state_.template get<std::uint32_t>(0x22c)&8u)return;
        const int old_primary=std::max(0,primary_count()),old_shots=state_.template get<std::int32_t>(0x69c);
        auto first=requested,second=requested;
        const auto transition=state_.template get<std::int32_t>(0x99c);
        if(!transition) {
            const auto previous=state_.template get<std::uint32_t>(0x9a4),next=state_.template get<std::uint32_t>(0x9a8);
            if(previous!=3){first=previous;second=3;}if(next!=3)second=next;
            state_.set(0x99c,state_.template get<std::int32_t>(0x9a0)!=-10000?std::int32_t{-1}:std::int32_t{-10000});
        }else if(transition==-1) {
            if(!state_.template get<std::int32_t>(0x9a0)){state_.set(0x99c,std::int32_t{-10000});state_.set(0x9a0,std::int32_t{-10000});}
            else {
                const auto previous=state_.template get<std::uint32_t>(0x9a4);
                if(previous!=3)first=previous;
                second=state_.template get<std::uint32_t>(0x9a8);
            }
        }
        std::array<Vector,4> posture=posture_;unsigned count=0;
        constexpr std::array<unsigned,3> counts{{3,2,4}};
        for(unsigned kind=0;kind!=3;++kind)if(first==kind||second==kind) {
            if(markers_.copy_posture(kind,posture,count)!=counts[kind])throw std::runtime_error("Human posture collider markers are incomplete");
            count+=counts[kind];
        }
        const auto motion=state_.template get<std::int32_t>(0x1ac);
        if(motion!=1&&motion!=7&&motion!=8)count=0;
        const auto animation=state_.template get<std::int32_t>(0x1d8),action=state_.template get<std::int32_t>(0x9b8);
        if(animation>112&&animation<155&&(action==0||action==-10000)&&state_.template get<float>(0x9f0)!=-10000.099609375f) {
            state_.set(0x9f0,-10000.099609375f);state_.set(0x9f4,0.f);state_.set(0x9f8,0.f);
            if(motion<7){
                state_.set(0x1ac,0u);state_.set(0x18c,std::uint8_t{1});
                face_.activate_air_pose(); //00781310, after the native contact and motion-state stores.
            }
        }
        if(state_.template get<float>(0x1c8)==0)count=0;
        if(state_.template get<float>(0x1cc)==0)state_.set(0x69c,std::int32_t{0});
        const auto shots=state_.template get<std::int32_t>(0x69c);
        if(old_primary>4||old_shots<0||old_shots>2||shots<0||shots>2)throw std::runtime_error("Human posture descriptor count exceeds its owned allocations");
        posture_=posture;state_.set(0x698,static_cast<std::int32_t>(count));
        for(int n=old_primary;n<static_cast<int>(count);++n)collisions_.insert(required(primary_,n));
        for(int n=static_cast<int>(count);n<old_primary;++n)collisions_.remove(required(primary_,n));
        for(int n=old_shots;n<shots;++n)collisions_.insert(required(shot_,n));
        for(int n=shots;n<old_shots;++n)collisions_.remove(required(shot_,n));
    }
    template<std::size_t Count>static Descriptor& required(std::array<std::unique_ptr<Descriptor>,Count>& descriptors,int index) {
        if(index<0||static_cast<std::size_t>(index)>=Count||!descriptors[static_cast<std::size_t>(index)])
            throw std::runtime_error("Human collision transition has no actual descriptor");
        return *descriptors[static_cast<std::size_t>(index)];
    }
    std::unique_ptr<Descriptor> make(std::uint32_t flags,std::uint32_t mask) {
        auto descriptor=std::make_unique<Descriptor>();descriptor->flags=flags;descriptor->mask=mask;
        descriptor->frame=human_->base().frame();descriptor->actor=human_;return descriptor;
    }
    void retire(std::unique_ptr<Descriptor>& descriptor) noexcept {
        if(descriptor){collisions_.remove_for_cleanup(*descriptor);descriptor.reset();}
    }
    void reset() { //0075AE40. Native scratch/SIMD padding is deliberately excluded.
        markers_.snapshot();state_.set(0x18c,std::uint8_t{1});
        for(auto offset:{0xa7cu,0x1bcu,0x1c0u,0x1c4u,0x230u,0x1ecu,0x1f8u,0x220u,0x228u,0x950u,
            0x250u,0x254u,0x9d0u,0x9d4u,0x9d8u,0xa18u,0x758u,0xa24u,
            0xa20u,0x188u,0xa34u,0xa40u,0xa44u,0xa94u,
            0x190u,0x234u,0x1a4u,0x1a8u,0x944u,0x940u,0x948u})state_.set(offset,std::uint32_t{0});
        ground_contact_.reset();interaction_actor_=nullptr;interaction_target_=nullptr;action_frame_=Frame{}; //Native754/990/A3C/A40 retain typed identity borrowers.
        for(auto offset:{0x988u,0x998u,0x9b8u,0x9bcu,0x9c0u,0x9c4u,0x9ccu,0x9acu,0x9b0u,
            0xa2cu,0xa30u,0xa1cu,0xa0cu,0xa38u,0x99cu,0x9a0u,0x9b4u})state_.set(offset,std::int32_t{-10000});
        for(auto offset:{0x98cu,0xa90u,0x1b4u,0x760u,0x761u,0x762u,0x96cu,0x1d4u,0x9c8u,0x9c9u})state_.set(offset,std::uint8_t{0});
        state_.set(0x1c8,.8f);state_.set(0x1cc,1.f);state_.set(0x1d0,1.f);state_.set(0x22c,std::uint32_t{0x1020});
        state_.set(0x1d8,std::uint32_t{45});state_.set(0x1dc,std::uint32_t{0});state_.set(0x1e0,std::uint32_t{1});
        state_.set(0x1e8,std::uint32_t{91});state_.set(0x1f4,std::uint32_t{75});state_.set(0x1fc,std::uint32_t{206});
        state_.set(0x208,std::uint8_t{1});state_.set(0x20c,std::uint32_t{91});state_.set(0x218,std::uint32_t{75});
        state_.set(0x9e0,std::uint32_t{3});state_.set(0x1ac,std::uint32_t{1});state_.set(0x1b0,std::uint32_t{1});
        for(auto base:{0x9f0u,0x240u})for(unsigned axis=0;axis!=3;++axis)state_.set(base+axis*4,std::uint32_t{0xc61c4066});
        for(auto offset:{0xa14u,0x750u,0x75cu,0x764u,0x994u,0xa98u})state_.set(offset,std::int32_t{-1});
        state_.set(0xa28,std::uint32_t{107}); //trunc(150/float1.4),00810D90/0084E80C.
        state_.set(0x224,std::uint8_t{1});state_.set(0x184,std::uint32_t{5});
        const auto model=human_->base().frame();
        shot_markers_[0]=frames_.actor_find(model,"shot1",0x20);shot_markers_[1]=frames_.actor_find(model,"shot2",0x20);
        state_.set(0x9a4,std::uint32_t{3});state_.set(0x9a8,std::uint32_t{3});
        audio_.configure_physics_voices();reset_human_look_words(state_,0x770);
        look_snapshot_model_=look_snapshot_argument_=Frame{};look_snapshot_actor_=nullptr;queue_.clear();
    }
    void shot_spheres() { //0075B870: relative centres are saved BEFORE the health adjustment.
        state_.set(0x69c,std::int32_t{2});
        for(unsigned index=0;index!=2;++index) {
            if(!shot_markers_[index])throw std::runtime_error("Required Human shot volume is absent");
            const auto world=frames_.actor_world_matrix(shot_markers_[index]);
            for(unsigned axis=0;axis!=3;++axis)shots_[index][axis]=world[12+axis];
            shots_[index][3]=frames_.human_volume_radius(shot_markers_[index]);
            const auto model=frames_.actor_world_matrix(human_->base().frame());
            for(unsigned axis=0;axis!=3;++axis)shot_relative_[index][axis]=static_cast<float>(static_cast<long double>(shots_[index][axis])-model[12+axis]);
        }
        if(static_cast<std::int32_t>(human_->properties.stats_ec[0])<1) {
            //0075BA01..0075BB1C keeps X extended; Y/Z and sqrt input spill.
            long double x=static_cast<long double>(shots_[0][0])-shots_[1][0];
            float y=shots_[0][1]-shots_[1][1],z=shots_[0][2]-shots_[1][2];
            const auto square=(static_cast<long double>(z)*z+static_cast<long double>(y)*y)+x*x;
            const float stored=static_cast<float>(square);constexpr float epsilon=1.e-8f;
            if(!std::isfinite(square)||!std::isfinite(stored))throw std::runtime_error("Non-finite Human shot direction");
            if(std::abs(square-1)>=epsilon) {
                if(stored>=epsilon) {
                    const auto scale=1/std::sqrt(static_cast<long double>(stored));x*=scale;
                    y=static_cast<float>(y*scale);z=static_cast<float>(z*scale);
                }else if(x!=0)x=x<0?-1:1;
                else if(z!=0)z=z<0?-1.f:1.f;
                else y=y<0?-1.f:1.f;
            }
            for(auto& shot:shots_)shot[3]*=.5f;
            shots_[0][0]=static_cast<float>(static_cast<long double>(shots_[0][0])-x*.4f);
            shots_[0][1]=static_cast<float>(static_cast<long double>(shots_[0][1])-static_cast<float>(static_cast<long double>(y)*.4f));
            shots_[0][2]=static_cast<float>(static_cast<long double>(shots_[0][2])-static_cast<float>(static_cast<long double>(z)*.4f));
            shots_[1][0]=static_cast<float>(x*.15f+shots_[1][0]);
            shots_[1][1]=static_cast<float>(static_cast<long double>(y)*.15f+shots_[1][1]);
            shots_[1][2]=static_cast<float>(static_cast<long double>(static_cast<float>(static_cast<long double>(z)*.15f))+shots_[1][2]);
        }
    }
    Frames& frames_;Collisions& collisions_;DynamicCollisionExclusions<Descriptor>& exclusions_;
    Face& face_;Audio& audio_;HumanPhysicsGlobals& globals_;Human* human_=nullptr;
    HumanCollisionMarkers<Frames> markers_;HumanJointWords<0xaa0> state_;
    HumanPhysicalFrameWords frame_words_{state_};
    std::array<std::unique_ptr<Descriptor>,4> primary_{},secondary_{};
    std::array<std::unique_ptr<Descriptor>,2> shot_{};
    std::array<Vector,4> posture_{};std::array<Vector,2> shots_{},shot_relative_{};
    std::array<Frame,2> shot_markers_{};std::array<Frame,25> bones_{};std::array<bool,25> bone_flags_{};
    Vector offset740_{};std::deque<std::array<std::uint32_t,3>> queue_;
    std::function<Frame()> shadow_; //Borrow actual HumanShadowModel's A9C; owner outlives calls, including failure retirement.
    void* interaction_actor_=nullptr; //Identity matches MissionTableReference::object; no virtual actor calls through this slot.
    Frame action_frame_{};
    SceneActor<Frames>* interaction_target_=nullptr; //A3C, SAME action target published by00751480.
    std::optional<GroundContact> ground_contact_;
    bool active_=false,initializing_=false;
};
} //namespace hd2::game
#endif
