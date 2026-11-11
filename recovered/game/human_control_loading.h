#ifndef HD2_GAME_HUMAN_CONTROL_LOADING_H
#define HD2_GAME_HUMAN_CONTROL_LOADING_H

#include "game/human_action_control.h"
#include "game/human_animation_state.h"
#include "game/human_face_control.h"
#include "game/human_model_geometry.h"
#include "game/human_movement_control.h"
#include "game/human_physics_loading.h"
#include "game/human_motion_state.h"
#include "game/effects_shadow_models.h"
#include "game/human_camera_amount.h"
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
private:
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
