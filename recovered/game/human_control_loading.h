#ifndef HD2_GAME_HUMAN_CONTROL_LOADING_H
#define HD2_GAME_HUMAN_CONTROL_LOADING_H

#include "game/human_action_control.h"
#include "game/human_control_action_queue.h"
#include "game/human_state_frame_timers.h"
#include "game/human_animation_state.h"
#include "game/human_face_control.h"
#include "game/human_model_geometry.h"
#include "game/human_movement_control.h"
#include "game/human_physics_loading.h"
#include "game/human_motion_state.h"
#include "game/effects_shadow_models.h"
#include "game/human_camera_amount.h"
#include "game/human_ai_shot_direction.h"
#include <cmath>
#include <optional>

namespace hd2::game {

//DD0's ordinary loading composition007855C0/00785B20/00785BE0/00786030.
//Native inline98 still owns E0 and230; bind the actual E0 before this owner's
//bind(). World services supply actual equipment/SOUND consumers and tables.
//No containing-world readiness or runtime action/mark producers are supplied.
template<class Frames,class Human,class Collisions,class Audio,class Random,
    class Callbacks,class Effects,class Presentation,class Services>
class HumanControlLoading {
public:
    using Self=HumanControlLoading;
    using Frame=typename Frames::Frame;
    using SceneInstance=SceneActor<Frames>;
    using Geometry=HumanModelGeometry<Frames,Random>;
    using Face=HumanFaceControl<Frames,Human,Random>;
    using Physical=HumanPhysicsLoading<Frames,Human,Collisions,Face,Audio>;
    using Movement=HumanMovementControl<Human,Physical>;
    struct Action;
    using Links=HumanControlLinks<Self,Movement,Action,HumanMotionState,Human>;
    struct Action : HumanActionControl<Frames,Self,Links,Movement,Human> {};
    using Tracks=HumanAnimationTracks<Frames,Callbacks>;
    using Shadow=HumanShadowModel<Frames,Effects>;
    using Animation=HumanAnimationState<Frames,Tracks,Shadow,Self,Random>;
    using PoolLease=decltype(std::declval<Audio&>().acquire_pool());
    using BoneOwner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    HumanControlLoading(Frames& frames,Collisions& collisions,
            DynamicCollisionExclusions<typename Physical::Descriptor>& exclusions,
            const HumanMorphTables& morph,HumanPhysicsGlobals& physics_globals,Audio& audio,Random& random,
            Callbacks& callbacks,Effects& effects,Presentation& presentation,Services& services)
        :frames_(frames),audio_(audio),random_(random),effects_(effects),presentation_(presentation),services_(services),
         geometry_(frames,random),face_(frames,morph,random),physical_(frames,collisions,exclusions,face_,audio,physics_globals,[this]{return shadow_.frame();}),
         tracks_(frames,callbacks),shadow_(frames,effects),animation_(frames,tracks_,shadow_,*this,opacity_,random) {
        pool_.emplace(audio_.acquire_pool()); //Geometry constructor precedes007855C0's shared pool lease.
    }
    ~HumanControlLoading(){deactivate();geometry_.clear();pool_.reset();} //00785950 ordering.
    HumanControlLoading(const HumanControlLoading&)=delete;
    HumanControlLoading& operator=(const HumanControlLoading&)=delete;
    template<class Resources,class Sweep>void bind(Human& human,Resources& resources,Sweep& sweep,const CollisionTree<Frame>* tree) {
        if(started_)throw std::logic_error("Cannot rebind active DD0");
        if(!human.command)throw std::runtime_error("Human DD0 requires its actual command owner");
        human.command->bind_event_pool(services_.human_ai_event_pool());
        human_=&human;physical_.bind(human);opacity_.reset(random_);
        bind_controls(resources,sweep,tree);
        std::optional<typename Geometry::OfflineBinding> offline;
        if((mode_flags()&7)==7) {
            const bool quality=services_.human_face_quality_enabled();
            geometry_.configure_offline_faces(services_.human_face_resources(),
                quality,services_.human_face_camouflage());
            if(quality) {
                auto* storage=human.base().storage();auto* holder=storage?storage->holder(0):nullptr;
                if(!holder)throw std::runtime_error("Offline Human face requires actual clothing holder0");
                std::optional<float> clothing;
                if(const auto entry=holder->entry())clothing=frames_.entry_scalar(entry);
                offline=typename Geometry::OfflineBinding{human.properties.field19c==1,clothing};
            }
        }
        geometry_.bind_loading(human.base().frame(),&human,this,mode_flags(),offline);
    }
    template<class Resources,class Sweep>void activate(Resources& resources,Sweep& sweep,const CollisionTree<Frame>* tree) {
        if(started_||!human_||!human_->base().frame())throw std::logic_error("DD0 activation has no bound cold Human");
        started_=true;const auto model=human_->base().frame();
        try {
            frames_.register_human_model(model);registered_=true;frames_.flag_human_model(model);
            presentation_.acquire();presentation_acquired_=true;amount_=presentation_.settings().amount;
            physical_.initialize(flag_dc4_); //0075B280 reads SAME DD0+DC4, initialized0 by007855C0.
            motion_.initialize(random_);animation_.activate(model);
            bind_controls(resources,sweep,tree);field_dc8_=0;
            for(unsigned n=0;n!=25;++n) {
                const auto frame=physical_.bones()[n];
                if(!frame)throw std::runtime_error("Human DD0 requires all25 retained loading bones");
                bones_[n].emplace(frames_.retain_actor_frame(frame));
            }
            active_=true;
        }catch(...){deactivate();throw;}
    }
    void deactivate() noexcept {
        if(!started_)return;
        //00786030's last shared close precedes scene unregister and physical close.
        if(presentation_acquired_){presentation_.release();presentation_acquired_=false;}
        if(registered_){attempt([&]{frames_.unregister_human_model(human_->base().frame());});registered_=false;}
        attempt([&]{physical_.close_audio(mode_flags(),pool_->pool());});
        attempt([&]{face_.close_sound(pool_->pool());});
        physical_.clear_descriptors(); //0075B6C0's descriptor/queue portion.
        animation_.deactivate();attempt([&]{movement_.close_loading();});
        for(auto& bone:bones_)bone.reset();
        active_=started_=false;
    }
    Geometry& geometry(){return geometry_;}Face& face(){return face_;}Physical& physical(){return physical_;}
    Tracks& tracks(){return tracks_;}Action& action(){return action_;}
    Shadow& shadow(){return shadow_;}
    const Links& links() const{return links_;}const auto& motion() const{return motion_;}
    const auto& animation() const{return animation_;}const auto& opacity() const{return opacity_;}
    Human** human_slot_address(){return physical_.human_slot_address();}
    std::uint32_t mode_flags() const{return services_.mode_flags();}
    bool human_is_player()const{if(!human_)throw std::logic_error("Human control has no bound actor");return services_.human_is_player(*human_);}
    bool frame_special_movement_item()const{ //005A66B0.
        auto* storage=human_?human_->base().storage():nullptr;
        if(!storage||storage->single()||!storage->holder(5))
            throw std::logic_error("Human movement requires the actual slot-five holder");
        const auto entry=storage->holder(5)->entry();
        return entry&&entry->definition_identifier()==0xe7u;
    }
    void frame_climb_direction(bool backward){movement_.frame_climb_direction(backward);}
    void resolve_model_geometry_camera(){ //007A7470 tail, actual Camera virtual6C.
        const auto camera=services_.human_geometry_camera_target();
        Frame bone{};
        if(camera==geometry_.camera3rd())bone=geometry_.skeleton().bones().at(6).frame;
        else if(camera==geometry_.camera_fpv())bone=geometry_.skeleton().bones().at(8).frame;
        else return;
        if(!camera||!bone)throw std::runtime_error("Human camera collision requires its actual target and skeleton bone");
        physical_.exclude_query(false);
        struct Restore {
            Physical& physical;
            ~Restore(){physical.restore_query(false);}
        } restore{physical_};
        const auto source=frames_.actor_world_matrix(bone),target=frames_.actor_world_matrix(camera);
        const HumanVector origin{{source[12],source[13],source[14],source[15]}};
        const HumanVector delta{{target[12]-source[12],target[13]-source[13],target[14]-source[14],0}};
        if(const auto hit=services_.human_geometry_sweep().sweep(origin,delta,.15f,0xc8002u,0x230000u))
            frames_.car_camera_position(camera,hit->position);
    }
    bool active() const{return active_;}
    bool collision_disabled()const{return flag_dc4_;} //No loading producer changes this DD0 byte.
    float presentation_amount() const{return amount_;}
    std::uint32_t animation_state_word(unsigned offset) const {
        if(offset==0xa9c)throw std::logic_error("DD0 A9C is the actual typed shadow frame, not a scalar word");
        if(offset>=0x260&&offset<0x604)return tracks_.words().template get<std::uint32_t>(offset-0x260);
        if(offset<0xaa0)return physical_.state().template get<std::uint32_t>(offset);
        if(offset>=0xac0&&offset<0xc00)return movement_.state().template get<std::uint32_t>(offset-0xac0);
        if(offset>=0xc00&&offset<0xcb0)return action_.state().template get<std::uint32_t>(offset-0xc00);
        throw std::out_of_range("DD0 loading scalar has no recovered owner");
    }
    void animation_state_word(unsigned offset,std::uint32_t value){
        if(offset==0xa9c){
            if(value||shadow_.frame())throw std::logic_error("CD0 initialization requires its actual A9C shadow already retired");
            return; //00757DC0's cold null write; no pointer is copied into scalar storage.
        }
        physical_.animation_loading_word(offset,value);
    }
    std::uint32_t animation_human_flags() const{return human_->properties.field1a0;}
    int animation_look_word5c() const{return services_.human_look_control(*human_).state().template get<std::int32_t>(0x5c);}
    int animation_property_a0() const{return services_.human_look_control(*human_).state().template get<std::int32_t>(0xa0);}
    int animation_look_stance() const{return services_.human_look_control(*human_).state().template get<std::int32_t>(0x50);}
    int animation_look_word(unsigned offset)const{return services_.human_look_control(*human_).state().template get<std::int32_t>(offset);}
    Frame human_animation_resource(unsigned table,unsigned row){return services_.human_animation_resource(table,row);}
    void refresh_equipment_visibility(){services_.refresh_human_equipment_visibility(*human_,shadow_);}
    void human_opacity_changed(Frame model,float value){(void)effects_.model_opacity(mode_flags(),model,value);}
    template<class EquipmentState>void apply_camera_amount(float amount,EquipmentState& equipment) { //00786650.
        if(!human_)throw std::logic_error("Human camera amount requires bound DD0");
        apply_human_camera_amount(effects_,human_->base().frame(),physical_.state().template get<std::uint32_t>(0xa30),equipment,amount);
    }
    std::uint32_t human_frame_epoch(Frame model){return frames_.human_frame_epoch(model);}
    std::uint32_t human_scene_epoch(){return frames_.human_scene_epoch();}
    int equipment_current_mode() const{return physical_.state().template get<std::int32_t>(0xa20);}
    int equipment_command_identity() const{return action_.state().template get<std::int32_t>(0x38);}
    void reset_equipment_command(){action_.reset_equipment_command();}
    void equipment_transfer_mode(std::uint32_t command,std::uint32_t mode,bool moved) {
        if(moved)field_dc8_=3;
        physical_.equipment_transfer_mode(command,mode);
    }
    bool animation_authority_actor() const{return (mode_flags()&1u)&&human_->properties.field19c!=1;}
    void enable_animation_control(bool enabled){movement_.animation_control_delay(500);animation_.enable_control(enabled);} //007870D0.
    template<class Look,class Sweep,class Runtime>void advance_control_frame(Look& look,Sweep& sweep,std::uint32_t elapsed,Runtime& runtime){ //00787270.
        if(flag_dc4_)return;
        if(!active_||!human_)throw std::logic_error("Human control update requires actual live DD0");
        (void)frames_.actor_world_matrix(human_->base().frame());
        const bool player=human_is_player();
        auto& state=physical_.control_frame_words();
        const auto physical_word=[&](unsigned offset){return state.template get<std::uint32_t>(offset);};
        state.set(0x1c0,0u);state.set(0x760,std::uint8_t{0});
        state.set(0x1d4,std::uint8_t{1});state.set(0x224,std::uint8_t{1});
        state.set(0x1c0,0u-services_.human_control_microseconds());
        const auto finish_clock=[&]{state.set(0x1c0,physical_word(0x1c0)+services_.human_control_microseconds());};
        if(!physical_word(0x1ac)&&!physical_word(0x9c0)){
            finish_clock();state.set(0x1d4,std::uint8_t{0});
            if((geometry_.movement().field6304&0x200u)&&geometry_.joints().field68d0>=1.e-8f&&
               movement_.state().template get<float>(0x114)==-10000.099609375f)
                runtime.update_inactive_human_obstruction(*this); //0074EA80 and the live geometry height gate.
            return;
        }
        if(physical_word(0x22c)&8u){
            finish_clock();state.set(0x1d4,std::uint8_t{0});return;
        }
        const auto look_word=[&](unsigned offset){return look.state().template get<std::uint32_t>(offset);};
        if(!player&&look_word(0x4c)!=2&&look_word(0xb8)!=5)state.set(0x22c,physical_word(0x22c)|1u);
        else physical_.clear_control_flags(1,look,sweep);
        prepare_control_projection(look);
        look.begin_control_frame(physical_word(0x1f4));
        if(human_frame_signed_word(physical_word(0xa24))>1&&physical_word(0x1f4)==0x4b){
            const auto remaining=physical_word(0xa24)-elapsed;
            state.set(0xa24,human_frame_signed_word(remaining)<1?1u:remaining);
        }
        auto publish=[&](std::uint32_t command){publish_control_frame_command(command);};
        look.advance_timed_posture(elapsed,publish);
        auto& equipment_owner=services_.human_presentation_equipment(*human_);
        advance_human_control_action_queue(*this,look,physical_.control_action_queue(),equipment_owner,elapsed,publish,runtime);
        runtime.apply_human_look_frame(*this,look,elapsed); //0077DE30.
        const auto animation=physical_word(0x1d8);
        if(!((animation>=0x29&&animation<=0x41)||(animation>=0x1e&&animation<=0x21)||
             (animation>=0xa2&&animation<=0xa5))||state.template get<std::uint8_t>(0xa90)){
            state.set(0x228,0u);movement_.control_frame_motion_mode(3);
        }
        if(human_frame_signed_word(physical_word(0x9ac))!=-10000){
            const auto remaining=physical_word(0x9ac)-elapsed;
            state.set(0x9ac,remaining);
            if(human_frame_signed_word(remaining)<1){state.set(0x9ac,std::int32_t{-10000});state.set(0x228,std::int32_t{-500});}
        }
        if(human_frame_signed_word(physical_word(0xa28))>0){
            const auto remaining=physical_word(0xa28)-elapsed;
            state.set(0xa28,human_frame_signed_word(remaining)<0?0u:remaining);
        }
        if(physical_word(0x1ac)==1)runtime.advance_human_ground_frame(*this,elapsed); //007637D0.
        runtime.advance_human_control_transitions(*this,elapsed); //007507A0.
        runtime.advance_human_attachment_control(*this); //00779B30.
        if(mode_flags()&2u)runtime.refresh_human_control_sector(*this); //0075C960/00789DB0/0075CAC0.
        look.clear_control_frame_commands();
        if(human_frame_signed_word(physical_word(0x980))>0){
            const auto remaining=physical_word(0x980)-elapsed;
            state.set(0x980,remaining);
            if(human_frame_signed_word(remaining)<1){
                if(human_frame_signed_word(physical_word(0x984))>0)runtime.retire_human_control_contacts(*this);
                state.set(0x984,0u);
            }
        }
        runtime.advance_human_control_motion(*this,elapsed); //00755500.
        physical_.retire_frame_audio(mode_flags(),[&]{return services_.current_human_frame_player()==human_;},pool_->pool());
        if(mode_flags()&1u){
            if(look_word(0xb8)==5&&physical_word(0x764)==0x46)runtime.advance_human_motion_event(*this,0x46,elapsed);
            else if(!state.template get<std::uint8_t>(0x760))runtime.advance_human_motion_event(*this,physical_word(0x75c),elapsed);
            if(human_->base().world_position()[1]<static_cast<long double>(services_.human_control_world_floor())-5.f)
                runtime.apply_human_floor_damage(*human_);
        }
        publish_control_frame_task(elapsed,runtime);
        constexpr std::array<unsigned,4> periods{{200,300,400,600}};
        for(unsigned n=0;n!=periods.size();++n){
            const auto offset=0xa80+4*n,value=physical_word(offset);
            if(value>=periods[n])state.set(offset,value-periods[n]);
        }
        const bool ordinary=!(physical_word(0x22c)&0x200u)||human_is_player()||physical_word(0x1ac)!=1;
        const bool flag761=state.template get<std::uint8_t>(0x761)!=0;
        const bool forced=!ordinary&&!flag761;
        if(state.template get<std::uint8_t>(0x18c)&&
           (static_cast<long double>(human_frame_signed_word(physical_word(0x228)))<=280.L/services_.human_control_animation_scale()||
            (!forced&&flag761))){
            if(forced)animation_.enable_control(true);
            try{synchronize_control_position(human_->base().world_position());} //007599F0.
            catch(...){if(forced)animation_.enable_control(false);throw;}
            if(forced)animation_.enable_control(false);
        }
        finish_clock();
        if(forced){
            animation_.enable_control(true);
            try{present_control_frame(look,elapsed,runtime);}
            catch(...){animation_.enable_control(false);throw;}
            animation_.enable_control(false);
        }
        const auto& tracks=tracks_.words();
        const float speed=tracks.template get<float>(0x4a0-0x260),other=tracks.template get<float>(0x4c0-0x260);
        auto vector=tracks.template get<HumanVector>((other<=speed?0x490u:0x4b0u)-0x260);
        for(unsigned n=0;n!=3;++n)vector[n]*=other<=speed?speed:other;
        movement_.control_frame_track_velocity(vector);
        state.set(0x1d4,std::uint8_t{0});
        if(human_frame_signed_word(field_dc8_)>0)--field_dc8_;
    }
    template<class Look,class Runtime>void present_control_frame(Look& look,std::uint32_t elapsed,Runtime& runtime){ //00787D50.
        if(flag_dc4_)return;
        if(!active_||!human_)throw std::logic_error("Human presentation requires actual live DD0");
        auto& state=physical_.control_frame_words();
        const auto word=[&](unsigned offset){return state.template get<std::uint32_t>(offset);};
        if(!word(0x1ac)&&!word(0x9c0)){
            state.set(0x1c4,0u);
            geometry_.present_frame(*human_,*this,elapsed,runtime);
            return;
        }
        if((mode_flags()&7u)!=7u&&!frames_.human_control_visible(human_->base().frame()))
            runtime.refresh_nonoffline_human_visibility(*this);
        const bool forced=state.template get<std::uint8_t>(0x1d4)!=0; //007881D0's same update without periodic/profiling work.
        if(!forced){
            for(unsigned offset:{0xa80u,0xa84u,0xa88u,0xa8cu})state.set(offset,word(offset)+elapsed);
            if((word(0x22c)&0x200u)&&!human_is_player()&&word(0x1ac)==1)return;
            state.set(0x1c4,0u-services_.human_control_microseconds());
        }
        const auto finish_clock=[&]{if(!forced)state.set(0x1c4,word(0x1c4)+services_.human_control_microseconds());};
        if(word(0x1ac)!=1){
            runtime.prepare_human_presentation_motion(*this,elapsed); //00772A00.
            runtime.advance_human_ground_frame(*this,elapsed); //007637D0.
        }
        if(word(0x22c)&8u){
            finish_clock();
            return;
        }
        //0076DF10 is a genuine RET 4 in this client.
        if(!human_is_player()){
            runtime.advance_human_presentation_direction(*this,look.state().template get<HumanVector>(0x20),elapsed); //0076DB30.
            const auto animation=human_frame_signed_word(word(0x1d8));
            constexpr auto inactive=static_cast<std::uint32_t>(-10000);
            if((animation<=8||animation>14)&&animation!=18&&(word(0x988)==inactive||word(0x998)!=inactive)){
                movement_.advance_presentation_replay([&]{
                    if(word(0x998)!=inactive){
                        const auto remaining=word(0x998)-elapsed;
                        state.set(0x998,human_frame_signed_word(remaining)<1?inactive:remaining);
                    }
                },[&]{return runtime.advance_human_presentation_replay(*this,elapsed);}); //0076CFE0(step,1).
            }else movement_.cancel_presentation_replay();
        }
        if(state.template get<std::uint8_t>(0x18c))synchronize_presentation_direction(look,runtime);
        animation_.present_position(elapsed); //007587F0→007585C0.
        present_control_effects(look,elapsed,runtime);
        if(human_frame_signed_word(word(0x950))>0){
            const auto remaining=word(0x950)-elapsed;
            state.set(0x950,human_frame_signed_word(remaining)<0?0u:remaining);
        }
        action_.advance_presentation_timer(elapsed);
        if(human_frame_signed_word(word(0xa94))<1000)state.set(0xa94,word(0xa94)+elapsed);
        const auto ticks=word(0xa7c)+elapsed;
        state.set(0xa7c,ticks>0xfffffc17u?0u:ticks);
        finish_clock();
        geometry_.present_frame(*human_,*this,elapsed,runtime);
    }
private:
    template<class Look,class Runtime>void present_control_effects(const Look& look,std::uint32_t elapsed,Runtime& runtime){ //00758830.
        auto& equipment_owner=services_.human_presentation_equipment(*human_);
        auto& state=physical_.control_frame_words();
        const auto mode=animation_.present_scene_effects(elapsed,human_is_player(),
            [&](std::int32_t event){ //00758D20.
                switch(event){
                case 1:equipment_owner.transfers().event(1);return true;
                case 2:{
                    const auto command=look.state().template get<std::uint32_t>(0x54);
                    if(geometry_.equipped_weapon().model()&&(geometry_.movement().field62d8&0xfu)==1&&command&&command!=2)
                        equipment_owner.transfers().event(0);
                    return true;
                }
                case 3:return true; //00758D20 has no receiver for this event ID.
                case 10:equipment_owner.set_frame_weapon_pose(false,true);return true;
                case 0x0b:case 0x0c:
                    if(event==0x0c||!animation_.equipment_frame_timer_active())equipment_owner.set_frame_weapon_pose(false,true);
                    animation_.clear_equipment_frame_timer();
                    return true;
                case 0x15:state.set(0x9c8,std::uint8_t{1});return true;
                default:return runtime.apply_human_track_event(*this,look,event);
                }
            },
            [&](Frame previous,Frame next){
                if(!(mode_flags()&2u))return;
                const auto root=frames_.camera_root_sector();
                if((!previous||!next||previous==root||next==root)&&services_.current_human_frame_player()==human_)
                    services_.refresh_human_presentation_camera();
            });
        if(state.template get<std::int32_t>(0xa30)>0)
            runtime.present_human_control_attachment(*this,elapsed,mode); //00793490 and attached opacity.
        if(animation_.advance_equipment_frame_timer(elapsed))
            equipment_owner.set_frame_weapon_pose(true,false); //00791AC0(1,1,0).
        const auto secondary=state.template get<std::uint32_t>(0x1e8);
        const auto equipment=look.state().template get<std::uint32_t>(0x54);
        const auto weapon_flags=geometry_.movement().field62d8;
        if(secondary>=0x1e&&secondary<=0x20&&state.template get<std::uint32_t>(0x20c)==0x5b&&
           state.template get<std::uint32_t>(0x1f4)==0x4b&&state.template get<std::uint32_t>(0x218)==0x4b&&
           geometry_.equipped_weapon().model()&&(weapon_flags&0xfu)==1&&equipment&&equipment!=2&&
           tracks_.transition_progress(1)>=1&&!(equipment_owner.state().words().template get<std::uint32_t>(0x50)&1u)&&
           !(weapon_flags&0x80000000u))
            equipment_owner.transfers().event(0); //007913D0(0).
        face_.present(elapsed,mode_flags(),(state.template get<std::uint32_t>(0x22c)&0x10u)!=0,
            human_is_player(),services_.current_human_frame_player()==human_,services_.human_control_animation_scale(),pool_->pool());
    }
    template<class Look>void prepare_control_projection(const Look& look){ //00770100.
        auto& state=physical_.control_frame_words();
        auto flags=state.template get<std::uint32_t>(0x22c);
        if(!(mode_flags()&2u)){
            state.set(0x22c,flags|0x20u);
            return;
        }
        const auto camera=frames_.active_game_camera();
        if(!camera)throw std::logic_error("Human projection requires the actual active scene camera");
        const auto model=frames_.actor_world_matrix(human_->base().frame());
        const auto view=frames_.actor_world_matrix(camera);
        const float x=model[12]-view[12],y=physical_.control_projection_height(model[13])-view[13],z=model[14]-view[14];
        const auto square=static_cast<float>((static_cast<long double>(y)*y+static_cast<long double>(z)*z)+static_cast<long double>(x)*x);
        constexpr float epsilon=1.e-8f; //0080F520 and the 322BCC77 cut at 00770292.
        const float distance=square<epsilon?0.f:std::abs(static_cast<long double>(square)-1)<epsilon?1.f:std::sqrt(square);
        const auto projection=frames_.vertex_camera_projection(camera);
        const auto scale=physical_.control_camera_projection(services_.game_word28(),projection[0],projection[1]);
        const float width=static_cast<float>(static_cast<long double>(scale)*distance*2.L);
        const float size=1.7f/width; //008100A0. Native zero width compares as positive infinity.
        constexpr float near_size=.15f; //0080FF04.
        const bool visible_epoch=frames_.human_frame_epoch(human_->base().frame())==frames_.human_scene_epoch();
        flags=(flags&0xffff102fu)|0x1020u;
        if(services_.current_human_frame_player()==human_||state.template get<std::uint32_t>(0x1ac)!=1){
            if(size>=near_size&&visible_epoch)flags|=0x10u;
        }else{
            const auto stance=look.state().template get<std::uint32_t>(0x50);
            constexpr std::array<float,6> thresholds{{.035f,.025f,.018f,.03f,.03f,.0135f}};
            const float threshold=stance<thresholds.size()?thresholds[stance]:.03f;
            if(!visible_epoch||size<.0135f)flags=(flags&0xfffff10fu)|0x100u;
            if(size>=near_size)flags|=0x10u;
            else if(size<threshold)flags=(flags&~0x20u)|0x80u;
            else if(size<.048f)flags=(flags&~0x20u)|0x40u;
            if(!visible_epoch)flags|=0x200u;
        }
        if(width>25.f||distance>frames_.hud_camera_clips(camera)[1])flags=(flags&0xffff2fffu)|0x2000u;
        state.set(0x22c,flags);
    }
    void synchronize_control_position(const HumanVector& position){ //007599F0.
        if(!animation_.synchronize_authority_position(position))return;
        physical_.invalidate_current_direction();
        frames_.human_control_set_position(human_->base().frame(),position);
    }
    template<class Look,class Runtime>void synchronize_presentation_direction(const Look& look,Runtime& runtime){ //00759B80.
        auto& state=physical_.control_frame_words();
        const auto mode=state.template get<std::uint32_t>(0x1ac);
        if(mode>2&&mode!=9&&state.template get<std::int32_t>(0x9cc)==-10000)return;
        const auto model=human_->base().frame();
        const auto matrix=frames_.actor_world_matrix(model);
        HumanVector current{{matrix[8],matrix[9],matrix[10],matrix[11]}};
        const float square=static_cast<float>((static_cast<long double>(current[0])*current[0]+
            static_cast<long double>(current[2])*current[2])+static_cast<long double>(current[1])*current[1]);
        normalize_human_ai_shot(current,square); //Same LS3DF zero/near-unit vector rule as the original here.
        const auto& target=human_->base().world_direction();
        const auto x=std::abs(current[0]-target[0]),y=std::abs(current[1]-target[1]),z=std::abs(current[2]-target[2]);
        const auto roll=state.template get<float>(0x250);
        if(state.template get<std::int32_t>(0xa44)>0||
           (state.template get<float>(0x254)==roll&&static_cast<long double>(z)+y+x<.002f))return;
        state.set(0x254,roll);
        frames_.human_control_set_direction(model,target,roll);
        if(look.state().template get<std::uint32_t>(0x4c)==2)runtime.refresh_prone_human_control(*this); //00763EC0(1).
    }
    void publish_control_frame_command(std::uint32_t command){ //005A5C90.
        if(auto* state=services_.human_state_owner(*human_).get())state->publish_frame_command_flags(command);
    }
    template<class Runtime>void publish_control_frame_task(std::uint32_t elapsed,Runtime& runtime){
        auto& state=physical_.control_frame_words();
        const auto animation=state.template get<std::uint32_t>(0x1d8);
        std::uint32_t task=0;
        if((animation>=106&&animation<=108)||animation==8||animation==14)task=0x100000;
        else if(animation>=8&&animation<=14)task=0x20000;
        else if(animation==0x12||animation==0x17||animation==0x18)task=0x80000;
        const auto height=movement_.state().template get<float>(0x114);
        bool obstruction=false;
        if(height!=-10000.099609375f){
            const auto bone=physical_.bones().at(4);
            if(!bone)throw std::logic_error("Human obstruction requires the actual head bone");
            obstruction=static_cast<long double>(frames_.actor_world_matrix(bone)[13])+.08f<=height;
            if(obstruction)task|=0x40000u;
        }
        runtime.update_human_control_obstruction(*this,elapsed,obstruction); //0075EBE0.
        publish_control_frame_command(task?task:0x1e0000u);
    }
    template<class Resources,class Sweep>void bind_controls(Resources& resources,Sweep& sweep,const CollisionTree<Frame>* tree) {
        movement_.bind(physical_,*human_,&links_,&action_,tree,frames_,resources,sweep);
        links_.bind(*this,movement_,action_,motion_,*human_);
        action_.bind(frames_,*this,links_,movement_,*human_);
    }
    template<class Work>void attempt(Work&& work) noexcept {
        try{work();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
    }
    Frames& frames_;Audio& audio_;Random& random_;Effects& effects_;Presentation& presentation_;Services& services_;
    std::optional<PoolLease> pool_;Geometry geometry_;Face face_;Physical physical_;Movement movement_;
    Links links_;Action action_;HumanMotionState motion_;HumanOpacityControl opacity_;
    Tracks tracks_;Shadow shadow_;Animation animation_;Human* human_=nullptr;
    std::array<std::optional<BoneOwner>,25> bones_{};
    float amount_=0;std::uint32_t field_dc8_=0;
    bool started_=false,active_=false,registered_=false,presentation_acquired_=false;
    bool flag_dc4_=false;
};
} //namespace hd2::game
#endif
