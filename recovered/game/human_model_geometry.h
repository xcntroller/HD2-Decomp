#ifndef HD2_GAME_HUMAN_MODEL_GEOMETRY_H
#define HD2_GAME_HUMAN_MODEL_GEOMETRY_H

#include "game/human_loading_joints.h"
#include "game/human_look_state.h"
#include "game/human_weapon_binding.h"
#include "game/human_face_model.h"
#include "game/human_state_frame_timers.h"
#include "game/human_clip_pose.h"
#include "game/human_bone_direction.h"
#include "game/human_ai_shot_direction.h"
#include "game/human_two_bone_ik.h"
#include "game/human_look_pose.h"
#include "game/human_attachment_pose.h"
#include "game/human_prone_pose.h"
#include "game/human_physics_globals.h"
#include "game/human_retained_clip_motion.h"
#include "frontend/spatial_sound.h"
#include "frontend/cleanup.h"
#include <functional>
#include <limits>
#include <exception>
#include <optional>

namespace hd2::game {

//The6950 owner0079B910/0079BAC0 and ordinary loading path through0079BD30,
//0079C320,007B85C0 and007C0D90. This does not implement model replacement
//with preserve-pose or any simulation producer. Offline62B0 requires the
//actual shared face resources and current profile-quality/mission settings.
//Unknown native scalar/padding bytes are not represented as recovered fields.
template<class Frames,class Random>
class HumanModelGeometry {
public:
    using Frame=typename Frames::Frame;
    using Owner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    using Matrix=HumanMatrix;
    using Vector=HumanVector;
    using JointPoint=typename HumanLoadingJoints<Frames>::Point;
    using JointBody=typename HumanLoadingJoints<Frames>::Body;
    using OfflineFaces=HumanFaceResources<Frames>;
    struct OfflineBinding {bool player;std::optional<float> clothing;};
    HumanModelGeometry(Frames& frames,Random& random)
        :frames_(frames),random_(random),joints_(frames,skeleton_) {reset_loading();}
    template<class Audio>HumanModelGeometry(Frames& frames,Random& random,Audio& audio)
        :HumanModelGeometry(frames,random){stop_voice_=[&audio](std::int32_t& handle){audio.stop_voice(handle);};}
    template<class Audio>HumanModelGeometry(Frames& frames,Random& random,Audio& audio,HumanPhysicsGlobals& globals)
        :HumanModelGeometry(frames,random,audio){physics_globals_=&globals;}
    ~HumanModelGeometry(){clear();}
    HumanModelGeometry(const HumanModelGeometry&)=delete;
    HumanModelGeometry& operator=(const HumanModelGeometry&)=delete;
    void configure_offline_faces(OfflineFaces& shared,bool quality_enabled,std::int32_t camouflage) {
        if(model_&&offline_faces_!=&shared)throw std::logic_error("Cannot replace offline face dependencies on bound geometry");
        offline_faces_=&shared;offline_quality_=quality_enabled;offline_camouflage_=camouflage;
    }
    void bind_loading(Frame model,const void* human,const void* control,std::uint32_t mode,
            std::optional<OfflineBinding> offline=std::nullopt) {
        if((mode&7)==7&&(!offline_faces_||(offline_quality_&&!offline)))
            throw std::runtime_error("Offline Human geometry requires actual shared face/profile/Human bindings");
        clear();model_=model;human_=human;control_=human?control:nullptr;reset_loading();
        if(!model)return;
        try {
            skeleton_.bind_graph(frames_,model);
            bind_frames();
            for(const auto& bone:skeleton_.bones()) {
                if(!bone.frame)throw std::runtime_error("Required Human skeleton marker is absent");
                frames_.human_unit_pose(bone.frame);
            }
            frames_.human_unit_pose(model);
            if((mode&7)==7&&offline_quality_) {
                face_model_=std::make_unique<HumanFaceModel<Frames>>(frames_,*offline_faces_);
                face_model_->bind(model,offline->player,offline_camouflage_,offline->clothing,0);
            }
            reset_movement();
            if(human)joints_.build();
        }catch(...) {clear();throw;}
    }
    void clear() noexcept {
        joints_.clear(); //0079BAC0 retires body then point trees before camera references.
        attachment_pose_.clear();
        skeleton_.clear();
        equipped_weapon_.clear();
        geometry_effect_.reset(); //0079BAC0 retires6840 before face/camera owners.
        face_model_.reset(); //0079BAC0 retires62B0 before retained camera frames62F8/FC.
        camera3rd_.reset();camera_fpv_.reset();model_=Frame{};human_=control_=nullptr;
        weapon1_=weapon2_=marker_z_=Frame{};feet_={};
        //0079BAC0 retires6914/6918 after both camera references. The audio
        //adapter borrows the SAME control audio lifetime and process SOUND owner.
        if(joints_.field6914!=-1)stop_contact_voice(joints_.field6914);
        joints_.field6914=-1;
        if(joints_.field6918>=0)stop_contact_voice(joints_.field6918);
        joints_.field6918=-1;
    }
    const auto& skeleton() const {return skeleton_;}
    const auto& joints() const {return joints_;}
    Frame model() const {return model_;}
    Frame camera3rd() const {return camera3rd_?camera3rd_->get():Frame{};}
    Frame camera_fpv() const {return camera_fpv_?camera_fpv_->get():Frame{};}
    Frame weapon1() const {return weapon1_;}
    Frame weapon2() const {return weapon2_;}
    const auto& feet() const {return feet_;}
    std::uint32_t random_seed() const {return seed_;}
    std::uint32_t frame_elapsed()const{return frame_elapsed_;}
    bool geometry_detail_enabled()const{return geometry_detail_;}
    struct GeometryEffect {
        std::uint16_t reason=0;
        const void* source=nullptr;
        Frame clip{};
        Vector root_position{},velocity{},camera_position{};
        std::uint32_t clip_time=0;
        std::array<std::uint32_t,23> position_cursors{},rotation_cursors{};
        std::array<Matrix,23> saved_local{},saved_world{}; //Native bone13C/17C.
        std::vector<Vector> contact_positions; //Actual6840 allocation, three or seven entries.
    };
    const auto& geometry_effect()const{return geometry_effect_;}
    template<class Human,class Control,class Services>void present_retained_clip(Human& human,Control& control,Services& services){
        if(!geometry_effect_)throw std::logic_error("Human retained clip requires its actual effect entry");
        present_human_retained_clip(frames_,skeleton_,model_,*geometry_effect_,movement_,seed_,frame_elapsed_,
            attachment_pose_.actor(),rest_surface_emitted_,human,control,services);
    }
    template<class Human,class Control,class Services>void present_ragdoll_frame(Human& human,Control& control,Services& services){ //007A7470's actual6304&200 branch.
        if(!geometry_effect_)throw std::logic_error("Human ragdoll frame requires its actual reaction entry");
        if(geometry_effect_->clip){
            present_retained_clip(human,control,services);
            const auto root=human_retained_clip::position(frames_,skeleton_.bones()[9].frame);
            const auto delta=human_retained_clip::subtract(geometry_effect_->root_position,root);
            const auto square=human_retained_clip::dot(delta,delta);
            if(square>1.e-6)frames_.refresh_dynamic_human_model(model_);
            const bool moved=square>1;
            if(moved)geometry_effect_->root_position=human_retained_clip::position(frames_,skeleton_.bones()[9].frame);
            control.refresh_ragdoll_geometry(true,moved);
            return;
        }
        if(!(joints_.field68d0>9.99999993922529e-09f)){
            if(joints_.field690c)joints_.update_contact_frames(frame_elapsed_,static_cast<std::uint8_t>(control.mode_flags()));
            if(joints_.field6914!=-1)retire_contact_sound(false,services);
            if(joints_.field6918>=0){services.stop_human_contact_voice(joints_.field6918);joints_.field6918=-2;}
            return;
        }
        auto& owner=services.human_inline(human);
        auto* source=static_cast<Human*>(const_cast<void*>(geometry_effect_->source));
        bool source_entered=false;
        const auto restore=[&]{
            frontend::CleanupErrors errors;errors.attempt([&]{owner.restore_query(false);});
            if(source_entered)errors.attempt([&]{services.human_inline(*source).restore_query(false);});
            errors.rethrow();
        };
        try{
            owner.exclude_query(false);
            if(source){source_entered=true;services.human_inline(*source).exclude_query(false);}
            joints_.field68e8-=frame_elapsed_;
            if(human_frame_signed_word(joints_.field68e8)<1){
                joints_.field68e8+=100u;
                joints_.snapshot_body_interpolation();joints_.update_contact_frames(100,static_cast<std::uint8_t>(control.mode_flags()));
                auto surface=[&]{emit_rest_surface(control);};
                advance_joint_physics(100,control.animation_state_word(0x22c),services,surface);
                joints_.capture_body_pose();
                if(!joints_.points()[0])throw std::logic_error("Human ragdoll frame requires its actual root point");
                const auto root=joints_.points()[0]->template get<Vector>(0x30);
                const auto delta=human_retained_clip::subtract(geometry_effect_->root_position,root);
                const bool moved=human_retained_clip::dot(delta,delta)>1;
                if(moved)geometry_effect_->root_position=root;
                control.refresh_ragdoll_geometry(true,moved);
            }
            const float influence=static_cast<float>(1.L-static_cast<long double>(human_frame_signed_word(joints_.field68e8))*.009999999776482582f);
            joints_.interpolate_body_pose(influence);
            const auto previous=human_retained_clip::position(frames_,model_);
            const auto current=joints_.publish_body_pose(model_);
            human.base().world_position(current); //007C58B0's final actual Human30 publication.
            const auto delta=human_retained_clip::subtract(previous,current);
            if(human_retained_clip::dot(delta,delta)>1.e-6)frames_.refresh_dynamic_human_model(model_);
            if(control.human_is_player()&&services.human_geometry_camera_target()==camera_fpv()){
                if(!joints_.points()[3])throw std::logic_error("Human ragdoll camera requires its actual point3");
                const auto target=joints_.points()[3]->template get<Vector>(0x30);
                auto& position=geometry_effect_->camera_position;
                const auto hit=services.human_geometry_sweep().sweep(position,human_retained_clip::subtract(target,position),
                    .17000000178813934f,0xc8002u,0x230000u);
                position=hit?hit->position:target;frames_.car_suspension_position(camera_fpv(),position);
            }
        }catch(...){
            const auto primary=std::current_exception();
            try{restore();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
            std::rethrow_exception(primary);
        }
        restore();
    }
    template<class Services>void begin_contact_sound(bool secondary,Services& services){ //007CDE00, native Point argument is unused.
        auto& handle=secondary?joints_.field6918:joints_.field6914;
        if(handle!=-1)return;
        if(secondary&&(seed_&1u)){handle=-2;return;}
        if(!stop_voice_)throw std::logic_error("Human contact sound requires its actual audio retirement binding");
        const auto marker=skeleton_.bones()[secondary?4u:22u].frame;
        if(!marker||!human_)throw std::logic_error("Human contact sound requires its actual relative bone and Human");
        frontend::SpatialSoundRequest request;
        request.flags=5;request.relative=reinterpret_cast<void*>(marker);request.source=const_cast<void*>(human_);
        request.group=secondary?1:0;request.entry=secondary?55:44;request.voice_group=4;
        handle=services.emit_human_equipment_sound(request);
    }
    template<class Services>void retire_contact_sound(bool secondary,Services& services){ //007CDF10.
        auto& handle=secondary?joints_.field6918:joints_.field6914;
        if(secondary?handle<0:handle==-1)return;
        const auto active=services.human_contact_voice_active(handle);
        if(!active||*active)return;
        services.stop_human_contact_voice(handle);
        handle=secondary?-2:-1;
    }
    template<class Services>void solve_point_contact(JointPoint& point,Vector& correction,Services& services){ //007CBF90.
        float radius=0;
        for(auto* body:point.bodies)if(body&&(body->points[0]==&point||body->points[1]==&point)){
            const auto candidate=body->template get<float>(0x18c);
            if(radius<candidate)radius=candidate;
        }
        radius*=.5f;
        auto start=point.template get<Vector>(0x100);
        const auto position=point.template get<Vector>(0x30),motion=point.template get<Vector>(0x70);
        Vector displacement=position;
        for(unsigned axis=0;axis!=3;++axis)displacement[axis]-=start[axis];
        const auto index=point.template get<std::uint32_t>(0);
        auto hit=services.human_geometry_sweep().sweep(start,displacement,radius,0xcc002u,index==0?0x220000u:0x230000u);
        if(index==0){
            if(!hit)return;
            if(services.human_geometry_flags(*hit)&0x10000u){
                const float projection=static_cast<float>((static_cast<long double>(hit->normal[0])*motion[0]+
                    static_cast<long double>(hit->normal[2])*motion[2])+static_cast<long double>(hit->normal[1])*motion[1]);
                joints_.flag6912=std::signbit(projection);
                joints_.gravity[0]=joints_.gravity[2]=0;joints_.gravity[1]=joints_.flag6912?-2.25f:-9.f;
                start=hit->position;
                for(unsigned axis=0;axis!=3;++axis)displacement[axis]=position[axis]-start[axis];
                hit=services.human_geometry_sweep().sweep(start,displacement,radius,0xcc002u,0x230000u);
            }
        }
        if(!hit)return;
        const auto flags=services.human_geometry_flags(*hit);
        if(!(flags&0x40u)){
            if(point.contact_frame){frames_.release_item_frame(point.contact_frame);point.contact_frame=Frame{};}
        }else{
            if(!hit->dynamic_identity||!hit->dynamic_identity->frame)
                throw std::runtime_error("Human ragdoll dynamic contact requires its actual frame");
            const auto frame=hit->dynamic_identity->frame;
            if(frame!=point.contact_frame){
                if(point.contact_frame){frames_.release_item_frame(point.contact_frame);point.contact_frame=Frame{};}
                auto* actor=services.human_ragdoll_contact_actor(*hit);
                if((movement_.field6304&0x10u)&&actor==attachment_pose_.actor())return;
                point.contact_frame=frames_.retain_actor_frame(frame).release();
                const auto world=frames_.actor_world_matrix(point.contact_frame);
                point.set(0x140,Vector{{world[12],world[13],world[14],world[15]}});
                if(actor&&actor->base().type()==4&&services.human_ragdoll_car_authority(*actor)){
                    auto impulse=motion;
                    for(unsigned axis=0;axis!=3;++axis)impulse[axis]*=100.f;
                    const float square=static_cast<float>((static_cast<long double>(impulse[1])*impulse[1]+
                        static_cast<long double>(impulse[2])*impulse[2])+static_cast<long double>(impulse[0])*impulse[0]);
                    if(square>640000.f){
                        const auto scale=human_joint_inverse_root(square)*800.f;
                        for(unsigned axis=0;axis!=3;++axis)impulse[axis]=static_cast<float>(impulse[axis]*scale);
                    }
                    services.human_ragdoll_car_impulse(*actor,impulse,hit->position);
                }
            }
        }
        auto contact=hit->position;
        const auto normal=hit->normal;
        for(unsigned axis=0;axis!=3;++axis){
            const float offset=normal[axis]*.0010000000474974513f;
            contact[axis]+=offset;
        }
        switch(index){
        case 2:case 3:case 4:case 5:case 6:case 7:case 8:case 9:case 11:case 12:case 14:case 15:
            if(!point.parent)throw std::runtime_error("Human ragdoll contact requires its actual parent point");
            if(point.parent->template get<float>(0x34)<position[1]){
                auto direction=position;
                for(unsigned axis=0;axis!=3;++axis)direction[axis]=point.parent->template get<float>(0x30+axis*4)-position[axis];
                const float square=static_cast<float>((static_cast<long double>(direction[1])*direction[1]+
                    static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[0])*direction[0]);
                if(square>9.99999993922529e-09f){
                    const auto inverse=human_joint_inverse_root(square);
                    const auto alignment=(static_cast<long double>(direction[2])*inverse*normal[2]+
                        static_cast<long double>(direction[1])*inverse*normal[1])+static_cast<long double>(direction[0])*inverse*normal[0];
                    if(alignment<-.7070000171661377f){
                        const auto projection=(static_cast<long double>(direction[2])*normal[2]+
                            static_cast<long double>(direction[1])*normal[1])+static_cast<long double>(direction[0])*normal[0];
                        for(unsigned axis=0;axis!=3;++axis){
                            auto product=projection*normal[axis];
                            if(axis)product=static_cast<float>(product);
                            direction[axis]=static_cast<float>((direction[axis]-product)*.75f);
                        }
                        const float lateral=static_cast<float>((static_cast<long double>(direction[1])*direction[1]+
                            static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[0])*direction[0]);
                        if(static_cast<long double>(radius)*radius*.9980000257492065f<lateral){
                            const auto scale=human_joint_inverse_root(lateral)*radius*.9990000128746033f;
                            for(unsigned axis=0;axis!=3;++axis)direction[axis]=static_cast<float>(direction[axis]*scale);
                        }
                        for(unsigned axis=0;axis!=3;++axis)contact[axis]+=direction[axis];
                    }
                }
            }
            break;
        default:break;
        }
        point.set(0x158,static_cast<std::uint8_t>(point.template get<std::uint8_t>(0x158)|1u));
        auto stored_normal=point.template get<Vector>(0x120);
        for(unsigned axis=0;axis!=3;++axis)stored_normal[axis]=normal[axis];
        point.set(0x120,stored_normal);point.set(0x154,flags>>24);
        for(unsigned axis=0;axis!=3;++axis)correction[axis]=contact[axis]-position[axis];
        switch(index){
        case 0:case 1:case 2:case 3:case 4:case 7:case 10:case 13:
            if(joints_.field6914==-1&&
                (static_cast<long double>(motion[2])*motion[2]+static_cast<long double>(motion[1])*motion[1])+
                    static_cast<long double>(motion[0])*motion[0]>7.2900004386901855f){
                begin_contact_sound(false,services);begin_contact_sound(true,services);
            }
            break;
        default:break;
        }
    }
    template<class Services>void solve_body_contact(JointBody& body,Vector& first_correction,Vector& last_correction,Services& services){ //007CC740.
        auto* first=body.points[0];auto* last=body.points[1];
        if(!first||!last)throw std::runtime_error("Human body contact requires its actual endpoint points");
        constexpr float epsilon=9.99999993922529e-09f;
        const auto first_previous=first->template get<Vector>(0x100),last_previous=last->template get<Vector>(0x100);
        const auto first_position=first->template get<Vector>(0x30),last_position=last->template get<Vector>(0x30);
        Vector previous{},position{},displacement{};
        for(unsigned axis=0;axis!=3;++axis){
            previous[axis]=static_cast<float>((static_cast<long double>(last_previous[axis])+first_previous[axis])*.5f);
            position[axis]=static_cast<float>((static_cast<long double>(last_position[axis])+first_position[axis])*.5f);
            displacement[axis]=position[axis]-previous[axis];
        }
        const float radius=static_cast<float>((static_cast<long double>(body.template get<float>(0x270))+body.template get<float>(0x18c))*.5f);
        const auto hit=services.human_geometry_sweep().sweep(previous,displacement,radius,0xcc002u,0x230000u);
        if(!hit)return;
        const auto normal=hit->normal;
        auto surface=hit->position;
        Vector movement{},segment{};
        for(unsigned axis=0;axis!=3;++axis){
            const float offset=normal[axis]*radius;
            surface[axis]-=offset;
            movement[axis]=first_position[axis]-first_previous[axis];
            segment[axis]=last_previous[axis]-first_previous[axis];
        }
        std::array<long double,3> plane{{
            static_cast<long double>(segment[1])*movement[2]-static_cast<long double>(segment[2])*movement[1],
            static_cast<long double>(segment[2])*movement[0]-static_cast<long double>(segment[0])*movement[2],
            static_cast<long double>(segment[0])*movement[1]-static_cast<long double>(segment[1])*movement[0]
        }};
        //Native Y/Z cross products spill before normalization. X and normalized
        //Y remain extended through the plane-distance test.
        plane[1]=static_cast<float>(plane[1]);plane[2]=static_cast<float>(plane[2]);
        const auto plane_square=(plane[1]*plane[1]+plane[2]*plane[2])+plane[0]*plane[0];
        if(std::abs(plane_square-1)>=epsilon){
            const float stored=static_cast<float>(plane_square);
            if(stored>=epsilon){
                const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));
                plane[0]*=inverse;plane[1]*=inverse;plane[2]=static_cast<float>(plane[2]*inverse);
            }else{
                const auto axis=plane[0]!=0?0u:plane[2]!=0?2u:1u;
                plane[axis]=plane[axis]<0?-1.L:1.L;
            }
        }
        const auto plane_distance=std::abs(((static_cast<long double>(surface[0])-first_previous[0])*plane[0]+
            (static_cast<long double>(surface[2])-first_previous[2])*plane[2])+
            (static_cast<long double>(surface[1])-first_previous[1])*plane[1]);
        if(!(plane_distance<static_cast<long double>(body.template get<float>(0x18c))*.5f))return;
        Vector contact_motion=surface;
        for(unsigned axis=0;axis!=3;++axis)contact_motion[axis]-=previous[axis];
        const float projection=static_cast<float>((static_cast<long double>(contact_motion[0])*displacement[0]+
            static_cast<long double>(contact_motion[2])*displacement[2])+static_cast<long double>(contact_motion[1])*displacement[1]);
        if(std::signbit(projection))return;
        const float square=static_cast<float>((static_cast<long double>(displacement[2])*displacement[2]+
            static_cast<long double>(displacement[1])*displacement[1])+static_cast<long double>(displacement[0])*displacement[0]);
        std::int32_t bits;std::memcpy(&bits,&square,sizeof bits);
        if(bits<=0x322bcc76)return;
        const auto length=std::abs(static_cast<long double>(square)-1)<epsilon?1.L:std::sqrt(static_cast<long double>(square));
        if(length<=epsilon)return;
        const auto inverse=1.L/static_cast<float>(length); //007CCBEF reloads the stored FLOAT length.
        for(unsigned axis=0;axis!=3;++axis)displacement[axis]=static_cast<float>(displacement[axis]*inverse);
        const auto fraction=((static_cast<long double>(displacement[1])*contact_motion[1]+
            static_cast<long double>(displacement[2])*contact_motion[2])+static_cast<long double>(contact_motion[0])*displacement[0])/
            (static_cast<long double>(body.template get<float>(0x18c))*.5f+static_cast<float>(length));
        const float factor=static_cast<float>(fraction);
        if(!(fraction<1))return; //Native compares extended fraction after storing the interpolation FLOAT.
        const auto flags=services.human_geometry_flags(*hit);
        using ActorPointer=decltype(services.human_ragdoll_contact_actor(*hit));
        ActorPointer actor=nullptr;
        if(!(flags&0x40u)){
            if(first->contact_frame){frames_.release_item_frame(first->contact_frame);first->contact_frame=Frame{};}
            if(last->contact_frame){frames_.release_item_frame(last->contact_frame);last->contact_frame=Frame{};}
        }else{
            if(!hit->dynamic_identity||!hit->dynamic_identity->frame)
                throw std::runtime_error("Human body contact requires its actual dynamic frame");
            const auto frame=hit->dynamic_identity->frame;
            if(frame!=first->contact_frame||frame!=last->contact_frame){
                if(first->contact_frame){frames_.release_item_frame(first->contact_frame);first->contact_frame=Frame{};}
                if(last->contact_frame){frames_.release_item_frame(last->contact_frame);last->contact_frame=Frame{};}
                actor=services.human_ragdoll_contact_actor(*hit);
                if((movement_.field6304&0x10u)&&actor==attachment_pose_.actor())return;
                last->contact_frame=frames_.retain_actor_frame(frame).release();
                first->contact_frame=frames_.retain_actor_frame(frame).release();
                const auto world=frames_.actor_world_matrix(first->contact_frame);
                const Vector origin{{world[12],world[13],world[14],world[15]}};
                last->set(0x140,origin);first->set(0x140,origin);
            }
        }
        const float complement=1.f-factor;
        const std::array<JointPoint*,2> endpoints{{first,last}};
        const std::array<Vector,2> previous_positions{{first_previous,last_previous}},positions{{first_position,last_position}};
        const std::array<Vector*,2> corrections{{&first_correction,&last_correction}};
        for(unsigned endpoint=0;endpoint!=2;++endpoint){
            auto* point=endpoints[endpoint];
            auto target=positions[endpoint];
            for(unsigned axis=0;axis!=3;++axis){
                const float old=complement*previous_positions[endpoint][axis],current=factor*positions[endpoint][axis];
                target[axis]=old+current;
            }
            point->set(0x158,static_cast<std::uint8_t>(point->template get<std::uint8_t>(0x158)|1u));
            auto stored_normal=point->template get<Vector>(0x120);
            for(unsigned axis=0;axis!=3;++axis)stored_normal[axis]=normal[axis];
            point->set(0x120,stored_normal);point->set(0x154,flags>>24);
            for(unsigned axis=0;axis!=3;++axis)(*corrections[endpoint])[axis]=target[axis]-positions[endpoint][axis];
        }
        if(actor&&actor->base().type()==4&&services.human_ragdoll_car_authority(*actor)){
            const auto first_motion=first->template get<Vector>(0x70),last_motion=last->template get<Vector>(0x70);
            const float first_square=static_cast<float>((static_cast<long double>(first_motion[2])*first_motion[2]+
                static_cast<long double>(first_motion[1])*first_motion[1])+static_cast<long double>(first_motion[0])*first_motion[0]);
            const float last_square=static_cast<float>((static_cast<long double>(last_motion[2])*last_motion[2]+
                static_cast<long double>(last_motion[1])*last_motion[1])+static_cast<long double>(last_motion[0])*last_motion[0]);
            const float maximum=static_cast<float>(first_square>last_square?first_square:last_square);
            auto impulse=first_square<=last_square?last_motion:first_motion;
            for(unsigned axis=0;axis!=3;++axis)impulse[axis]*=100.f;
            const float scaled_square=maximum*10000.f;
            if(scaled_square>640000.f){
                const auto scale=human_joint_inverse_root(scaled_square)*800.f;
                for(unsigned axis=0;axis!=3;++axis)impulse[axis]=static_cast<float>(impulse[axis]*scale);
            }
            services.human_ragdoll_car_impulse(*actor,impulse,surface);
        }
    }
    template<class Services>void solve_joint_contacts(float seconds,Services& services){ //007CBD50, all points before all bodies.
        for(auto* point:joints_.points())if(point)
            point->set(0x158,static_cast<std::uint8_t>(point->template get<std::uint8_t>(0x158)&0xfeu));
        for(auto* point:joints_.points())if(point){
            Vector correction{};
            solve_point_contact(*point,correction,services);
            for(unsigned axis=0;axis!=3;++axis)point->set(0x30+axis*4,point->template get<float>(0x30+axis*4)+correction[axis]);
        }
        for(auto* body:joints_.bodies())if(body){
            Vector first{},last{};
            solve_body_contact(*body,first,last,services);
            for(unsigned endpoint=0;endpoint!=2;++endpoint){
                auto* point=body->points[endpoint];
                if(!point)throw std::runtime_error("Human contact publication requires actual body endpoints");
                const auto& correction=endpoint?last:first;
                for(unsigned axis=0;axis!=3;++axis)point->set(0x30+axis*4,point->template get<float>(0x30+axis*4)+correction[axis]);
            }
        }
        for(auto* point:joints_.points())if(point){
            const auto position=point->template get<Vector>(0x30),previous=point->template get<Vector>(0x40);
            point->set(0x100,position);
            const float inverse=static_cast<float>(1.L/seconds);
            auto motion=point->template get<Vector>(0xe0);
            for(unsigned axis=0;axis!=3;++axis){
                auto difference=static_cast<long double>(position[axis])-previous[axis];
                if(axis==2)difference=static_cast<float>(difference);
                auto velocity=difference*inverse;
                if(axis==0)velocity=static_cast<float>(velocity);
                auto sum=velocity+motion[axis];
                if(axis<2)sum=static_cast<float>(sum);
                motion[axis]=static_cast<float>(sum*.5f);
            }
            point->set(0xe0,motion);
        }
    }
    template<class RestSurface>void advance_joint_sleep(JointPoint& point,RestSurface& rest_surface){ //007CD230.
        if(!physics_globals_)throw std::logic_error("Human ragdoll sleep requires its actual process physics globals");
        auto& globals=*physics_globals_;
        const bool root=point.parent==nullptr;
        if(root){
            joints_.field68d8=joints_.field68d4=0;joints_.flag6910=0;joints_.field690c=0;
            globals.ragdoll_contact_normal=0;globals.ragdoll_contacts=0;
        }
        float vertical=0;
        if(std::signbit(joints_.field68ec)){
            vertical=point.template get<float>(0xe4);
            if(!(std::abs(static_cast<long double>(vertical))<.47999998927116394)){
                const auto amount=static_cast<long double>(point.template get<float>(0x24))+.009999999776482582f;
                point.set(0x24,amount>1?1.f:static_cast<float>(amount));
            }else{
                const auto motion=point.template get<Vector>(0x70);
                const auto square=(static_cast<long double>(motion[0])*motion[0]+static_cast<long double>(motion[1])*motion[1])+
                    static_cast<long double>(motion[2])*motion[2];
                if((point.template get<std::uint8_t>(0x158)&3u)&&square>9.99999993922529e-09f){
                    const auto inverse=human_joint_inverse_root(square);
                    const auto normal=point.template get<Vector>(0x120);
                    const auto projection=(inverse*motion[2]*normal[2]+inverse*motion[1]*normal[1])+inverse*motion[0]*normal[0];
                    const auto squared=projection*projection;
                    auto amount=point.template get<float>(0x24)-squared*squared*.009999999776482582f;
                    point.set(0x24,static_cast<float>(amount));
                    if(joints_.flag6912){amount-=.009999999776482582f;point.set(0x24,static_cast<float>(amount));}
                }
                if(std::signbit(point.template get<float>(0x24)))point.set(0x24,0.f);
            }
        }
        if(point.template get<std::uint8_t>(0x158)&3u){
            const auto up=point.template get<float>(0x124);
            globals.ragdoll_contact_normal+=up;++globals.ragdoll_contacts;
            if(up>.5f)++joints_.flag6910;
        }
        if(point.contact_frame)++joints_.field690c;
        joints_.field68d8+=point.template get<float>(0x24);joints_.field68d4+=1;
        for(auto* child:point.children)if(child)advance_joint_sleep(*child,rest_surface);
        if(!root)return;
        if(!std::signbit(joints_.field68ec)){
            joints_.field68ec-=joints_.field68f8;return;
        }
        if(vertical<-400.f)joints_.field68d0=0;
        const auto average=static_cast<long double>(joints_.field68d8)/joints_.field68d4;
        joints_.field68d4=static_cast<float>(average);
        const auto decay=[&]{
            if(!globals.ragdoll_contacts)return static_cast<long double>(.019999999552965164f);
            auto amount=static_cast<long double>(globals.ragdoll_contact_normal)/globals.ragdoll_contacts;
            globals.ragdoll_contact_normal=static_cast<float>(amount);
            if(std::signbit(globals.ragdoll_contact_normal))amount=0;
            globals.ragdoll_contact_normal=static_cast<float>(amount*amount);
            return static_cast<long double>(globals.ragdoll_contact_normal)*.019999999552965164f+.0010000000474974513f;
        };
        if(!(average<.8500000238418579f)){
            const auto amount=static_cast<long double>(joints_.field68d0)+.009999999776482582f;
            joints_.field68d0=amount>1?1.f:static_cast<float>(amount);
        }else{
            const auto amount=joints_.field68d0-decay();
            joints_.field68d0=amount<9.99999993922529e-09f?0.f:static_cast<float>(amount);
            if(!rest_surface_emitted_)rest_surface(); //007B37C0 publishes6740 only after its actual surface request.
        }
        if(joints_.flag6910){
            joints_.field68f0-=joints_.field68f8;
            if(std::signbit(joints_.field68f0)){
                const auto amount=joints_.field68d0-decay();
                joints_.field68d0=amount<9.99999993922529e-09f?0.f:static_cast<float>(amount);
                if(!rest_surface_emitted_)rest_surface();
            }
        }
    }
    template<class Services,class RestSurface>void advance_joint_physics(std::uint32_t elapsed,std::uint32_t frame_flags,Services& services,RestSurface& rest_surface){
        auto contacts=[&](float seconds){solve_joint_contacts(seconds,services);};
        auto sleep=[&](JointPoint& point){advance_joint_sleep(point,rest_surface);};
        joints_.advance_physics(elapsed,frame_flags,contacts,sleep); //007C77E0, SAME geometry collision/sleep receivers.
    }
    template<class Control>void emit_rest_surface(Control& control){ //007B37C0, native6740.
        if(rest_surface_emitted_)return;
        const auto& points=joints_.points();const auto& bodies=joints_.bodies();
        if(!points[0]||!points[1]||!points[2]||!bodies[0]||!bodies[1])
            throw std::logic_error("Human rest surface requires its actual torso points and radii");
        const auto root=points[0]->template get<Vector>(0x30),middle=points[1]->template get<Vector>(0x30),end=points[2]->template get<Vector>(0x30);
        auto direction=end;
        for(unsigned axis=0;axis!=3;++axis)direction[axis]-=root[axis];
        const auto square=(static_cast<long double>(direction[1])*direction[1]+static_cast<long double>(direction[0])*direction[0])+
            static_cast<long double>(direction[2])*direction[2];
        normalize_human_ai_shot(direction,square);
        if(!(direction[1]<.5f))return;
        Vector normal{{0,1,0,0}};
        for(auto index:{1u,0u,2u})if(points[index]->template get<std::uint8_t>(0x158)&1u){normal=points[index]->template get<Vector>(0x120);break;}
        const float projection=static_cast<float>((static_cast<long double>(normal[1])*direction[1]+
            static_cast<long double>(direction[0])*normal[0])+static_cast<long double>(direction[2])*normal[2]);
        for(unsigned axis=0;axis!=3;++axis){
            const float amount=direction[axis]*projection;normal[axis]-=amount;
        }
        const auto normal_square=(static_cast<long double>(normal[2])*normal[2]+static_cast<long double>(normal[0])*normal[0])+
            static_cast<long double>(normal[1])*normal[1];
        normalize_human_ai_shot(normal,normal_square);
        const float draw=static_cast<float>(static_cast<long double>(random_())*3.0518509447574615e-05f);
        const float squared=draw*draw;
        const float end_weight=static_cast<float>((1.L-(static_cast<long double>(draw)+draw))+squared);
        const float middle_weight=static_cast<float>((static_cast<long double>(draw)+draw)-(static_cast<long double>(squared)+squared));
        Vector position{};
        for(unsigned axis=0;axis!=3;++axis){
            const float a=end_weight*end[axis],b=middle_weight*middle[axis];
            auto product=static_cast<long double>(squared)*root[axis];
            if(axis<2)product=static_cast<float>(product);
            position[axis]=static_cast<float>((product+b)+a);
        }
        const auto first=bodies[0]->template get<float>(0x18c),second=bodies[1]->template get<float>(0x18c);
        const float radius=(first<=second?second:first)*.5f;
        for(unsigned axis=0;axis!=3;++axis)position[axis]-=radius*normal[axis]; //004CE5C0 is vector -=, ECX=position.
        control.request_geometry_surface(position,normal);
        rest_surface_emitted_=true; //Publish only after the real DD0 surface chain returns.
    }
    void select_secondary_pose(Frame animation,std::uint32_t mode){ //007AE320, borrowed6630 and actual23-bone scratch.
        if(!(mode&2u))return;
        secondary_pose_=animation;
        if(!animation){movement_.field6304&=~0x80u;return;}
        secondary_pose_current_.fill(0);secondary_pose_previous_.fill(0);secondary_pose_time_=0;
        movement_.field6304|=0x80u;
    }
    void select_equipment_overlay(std::uint32_t mode){ //007AE390, native stack clip argument is unused.
        if(!(mode&2u))return;
        equipment_overlay_time_=0;movement_.field6304|=0x100u;
    }
    void present_secondary_clip(){ //007AFC20, actual6630/6634 and per-bone6638/6694.
        if(secondary_pose_){
            const auto time=human_frame_signed_word(secondary_pose_time_);
            auto influence=static_cast<long double>(time)*.0033333334140479565f;
            float weight=static_cast<float>(influence);
            if(influence>1){
                const auto remaining=human_frame_signed_word(static_cast<std::uint32_t>(frames_.human_clip_duration(secondary_pose_))-secondary_pose_time_);
                influence=static_cast<long double>(remaining)*.0033333334140479565f;
                weight=influence>1?1.f:static_cast<float>(influence);
            }
            auto bone_weight=[](int bone){return static_cast<long double>(human_secondary_clip_bone_weight(bone));};
            apply_human_clip_pose(frames_,skeleton_,secondary_pose_,secondary_pose_current_,secondary_pose_previous_,
                time,22,weight,bone_weight);
            frames_.update_scene_object(model_);
            const auto next=static_cast<long double>(human_frame_signed_word(frame_elapsed_))*1.399999976158142f+time;
            secondary_pose_time_=static_cast<std::uint32_t>(human_animation_time_integer(next));
            if(human_frame_signed_word(secondary_pose_time_)<=frames_.human_clip_duration(secondary_pose_))return;
        }
        movement_.field6304&=~0x80u;
    }
    void present_equipment_overlay(){ //007AFD70.
        if(equipped_weapon_.model()){
            equipment_overlay_time_+=frame_elapsed_;
            const float influence=static_cast<float>(1.L-static_cast<long double>(human_frame_signed_word(equipment_overlay_time_))*.0033333334140479565f);
            if(!std::signbit(influence)){
                const float factor=static_cast<float>(static_cast<long double>(influence)*.5f);
                const Vector weights{{factor,factor,factor,0}};
                if(movement_.field62cc!=4){
                    present_weapon_overlay(weights);
                    return;
                }
                auto desired=movement_.field6350;
                desired[1]-=.1736000031232834f;
                const auto square=(static_cast<long double>(desired[2])*desired[2]+
                    static_cast<long double>(desired[1])*desired[1])+static_cast<long double>(desired[0])*desired[0];
                normalize_human_ai_shot(desired,square);
                for(unsigned bone:{18u,16u,8u})
                    correct_human_bone_direction(frames_,skeleton_,model_,bone,desired,movement_.field6350,weights);
                return;
            }
        }
        movement_.field6304&=~0x100u;
    }
    void pose_equipment_weapon(const Vector& direction,bool align_hands,bool align_weapon,float influence){ //007C02F0.
        auto orientation=frames_.human_direction_matrix(direction,Vector{{0,1,0,0}});
        const auto left=equipped_weapon_.left(),right=equipped_weapon_.right();
        if(align_hands&&left&&right){
            const auto align=[&](Frame marker,unsigned index){
                auto target=frames_.rigid_matrix_product(frames_.car_local_matrix(marker),orientation);
                target[12]=target[13]=target[14]=0;
                const auto hand=skeleton_.bones()[index].frame;
                const auto parent=frames_.object_parent(hand);
                if(!parent)throw std::logic_error("Human hand pose requires its actual frame parent");
                auto inverse=human_transpose(frames_.actor_world_matrix(parent));
                inverse[3]=inverse[7]=inverse[11]=0;
                if(influence<1){
                    const auto desired=frames_.human_matrix_rotation(target);
                    const auto current=frames_.human_matrix_rotation(frames_.actor_world_matrix(hand));
                    frames_.human_set_matrix_rotation(target,frames_.physics_slerp(current,desired,influence,false));
                }
                target=frames_.car_matrix_product(target,inverse);
                frames_.human_publish_bone_rotation(hand,target);
            };
            if(movement_.field62d8&0x80000000u)align(left,21);
            align(right,20);
        }
        if(!align_weapon)return;
        auto marker=right,hand=weapon1_;
        if((movement_.field62d8&0xfu)!=1){marker=left?left:right;hand=weapon2_;}
        if(!marker||!hand)throw std::logic_error("Human weapon pose requires its actual selected hand and grip");
        const auto hand_world=frames_.actor_world_matrix(hand);
        const auto grip=frames_.collision_sweep_rotate(frames_.car_position(marker),orientation);
        Vector position{{hand_world[12]-grip[0],hand_world[13]-grip[1],hand_world[14]-grip[2],hand_world[15]}};
        if(equipped_weapon_.butt()&&(movement_.field6304&1u)){
            const Vector offset=movement_.field62dc==3&&movement_.field62d0==0?
                Vector{{.03f,-.051f,.11f,0}}:Vector{{.033f,.036f,.08f,0}};
            const auto butt=frames_.collision_sweep_rotate(frames_.car_position(equipped_weapon_.butt()),orientation);
            const auto shoulder=frames_.car_transform(offset,frames_.actor_world_matrix(skeleton_.bones()[14].frame));
            const auto amount=human_animation_word_float(movement_.field6318);
            const auto complement=1.L-amount;
            for(unsigned axis=0;axis!=3;++axis){
                const auto difference=static_cast<long double>(shoulder[axis])-butt[axis];
                const float target=static_cast<float>((axis==2?static_cast<float>(difference):difference)*amount);
                const auto previous=static_cast<long double>(position[axis])*complement;
                position[axis]=static_cast<float>(target+(axis?static_cast<float>(previous):previous));
            }
        }
        const auto weapon=equipped_weapon_.model();
        if(!weapon)throw std::logic_error("Human weapon pose requires its actual retained weapon model");
        for(unsigned axis=0;axis!=4;++axis)orientation[12+axis]=position[axis];
        frames_.human_equipment_local_matrix(weapon,orientation);
        frames_.update_scene_object(weapon);
    }
    void pose_hand_chain(unsigned first,unsigned second,const Vector& desired,const Vector& offset,
            Frame grip,float influence,const Vector* preferred){ //007C09C0, same skeleton lengths and actual hand frame.
        auto grip_rotation=frames_.actor_world_matrix(grip);
        grip_rotation[12]=grip_rotation[13]=grip_rotation[14]=0;
        const auto& root_bone=skeleton_.bones().at(first);
        const auto& end_bone=skeleton_.bones().at(second);
        const auto root_world=frames_.actor_world_matrix(root_bone.frame);
        const Vector root{{root_world[12],root_world[13],root_world[14],root_world[15]}};
        const auto grip_world=frames_.actor_world_matrix(grip);
        const auto complement=1.L-influence;
        Vector target=offset;
        for(unsigned axis=0;axis!=3;++axis){
            const auto first_product=static_cast<long double>(desired[axis])*influence;
            const auto second_product=static_cast<long double>(offset[axis])*complement;
            auto mixed=axis?static_cast<float>(first_product):first_product;
            mixed+=axis?static_cast<float>(second_product):second_product;
            if(axis==2)mixed=static_cast<float>(mixed);
            const auto world_position=mixed+grip_world[12+axis];
            target[axis]=static_cast<float>((axis?world_position:static_cast<float>(world_position))-offset[axis]);
        }
        const auto middle_world=frames_.actor_world_matrix(end_bone.frame);
        Vector middle{{middle_world[12],middle_world[13],middle_world[14],middle_world[15]}};
        //0079BDC5/0079BDCF initialize these private weights. No other direct
        //producer occurs in the original image's normal constructed lifetime.
        const bool solved=solve_human_two_bone_positions(root,root_bone.lengths[0],target,end_bone.lengths[0],middle,preferred,.5f,.9f);
        Vector direction=middle;
        for(unsigned axis=0;axis!=3;++axis)direction[axis]=(solved?middle[axis]:target[axis])-root[axis];
        orient_human_bone(frames_,skeleton_,model_,first,direction,-1.f);
        if(solved)for(unsigned axis=0;axis!=3;++axis)direction[axis]=target[axis]-middle[axis];
        orient_human_bone(frames_,skeleton_,model_,second,direction,-1.f);
        auto inverse=human_transpose(frames_.actor_world_matrix(end_bone.frame));
        inverse[3]=inverse[7]=inverse[11]=0;
        grip_rotation=frames_.car_matrix_product(grip_rotation,inverse);
        frames_.human_publish_bone_rotation(grip,grip_rotation);
    }
    void present_stowed_weapon(std::uint32_t mode){ //007B9750.
        if(!(mode&2u)||movement_.field62cc==4||!(movement_.field62d8&0x80000000u))return;
        const auto weapon=equipped_weapon_.model();
        if(!weapon)throw std::logic_error("Human stowed pose requires its actual retained weapon model");
        const auto parent=frames_.object_parent(weapon);
        frames_.human_geometry_weapon_link(weapon,Frame{});
        try{
            const auto hand=frames_.actor_world_matrix(weapon1_);
            const Vector reference{{hand[0],hand[1],hand[2],hand[3]}};
            const auto blend=human_animation_word_float(movement_.field631c);
            const auto prepare_direction=[&]{
                refresh_weapon_hand_offsets();
                auto direction=movement_.left_hand_offset6330;
                for(unsigned axis=0;axis!=3;++axis)direction[axis]-=movement_.right_hand_offset6340[axis];
                const auto square=(static_cast<long double>(direction[2])*direction[2]+
                    static_cast<long double>(direction[1])*direction[1])+static_cast<long double>(direction[0])*direction[0];
                normalize_human_ai_shot(direction,square);
                movement_.field6350=interpolate_human_directions(reference,direction,blend);
            };
            prepare_direction();
            pose_equipment_weapon(movement_.field6350,true,false,blend);
            prepare_direction();
            pose_equipment_weapon(movement_.field6350,false,true,1.f);
            if(geometry_detail_){
                const auto left_hand=skeleton_.bones()[21].frame;
                movement_.left_hand_offset6330=frames_.car_transform(movement_.offset63a0,frames_.actor_world_matrix(left_hand));
                const auto point=frames_.actor_world_matrix(equipped_weapon_.left());
                const Vector target{{point[12],point[13],point[14],point[15]}};
                pose_hand_chain(17,19,target,movement_.left_hand_offset6330,left_hand,blend,nullptr);
            }
        }catch(...){
            const auto failure=std::current_exception();
            try{frames_.human_geometry_weapon_link(weapon,parent);}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
            std::rethrow_exception(failure);
        }
        frames_.human_geometry_weapon_link(weapon,parent);
    }
    void present_standing_weapon(const Vector* velocity){ //007BCFC0.
        limit_aim_vertical();
        limit_aim_forward(.34f,.9404f);
        if(movement_.field62cc==4){
            present_kind_four_aim(velocity&&aim_motion_square(*velocity)>9.99999993922529e-09f);
            return;
        }
        const auto first=frames_.actor_world_matrix(skeleton_.bones()[9].frame);
        const auto second=frames_.actor_world_matrix(skeleton_.bones()[8].frame);
        Vector up{{second[12]-first[12],second[13]-first[13],second[14]-first[14],second[15]}};
        normalize_human_ai_shot(up,(static_cast<long double>(up[0])*up[0]+
            static_cast<long double>(up[1])*up[1])+static_cast<long double>(up[2])*up[2]);
        auto basis=frames_.actor_world_matrix(model_);
        align_human_matrix_up(frames_,basis,up);
        float motion=velocity?static_cast<float>(aim_motion_square(*velocity)*.0625f):0.f;
        if(motion>1)motion=1;
        const auto complement=1.L-motion;
        const float first_x=static_cast<float>(static_cast<long double>(motion)*.15f+.35f*complement);
        const float weight_y=static_cast<float>(static_cast<long double>(motion)*.7f+.95f*complement);
        const float second_x=static_cast<float>(static_cast<long double>(motion)*.25f+complement*.45f);
        auto direction=aim_hand_direction();
        const auto saved_right=movement_.right_hand_offset6340;
        const auto blend=human_animation_word_float(movement_.field6318);
        Vector weights{{first_x*blend,weight_y*blend,blend*0.f,0}};
        correct_human_bone_reference(frames_,skeleton_,model_,12,direction,movement_.direction6360,basis,weights);
        direction=aim_hand_direction();
        weights={{second_x*blend,weight_y*blend,blend*0.f,0}};
        correct_human_bone_reference(frames_,skeleton_,model_,7,direction,movement_.direction6360,basis,weights);
        if(motion<9.99999993922529e-09f&&!equipped_weapon_.butt()&&geometry_detail_)
            restore_aim_right_hand(saved_right);
        if(movement_.field62d8&0x80000000u)direction=aim_hand_direction();
        finish_aimed_weapon(direction);
        present_aimed_arm_chains();
    }
    void present_crouched_weapon(const Vector* velocity){ //007BB540.
        limit_aim_vertical();
        limit_aim_forward(.34f,.9404f);
        const bool moving=velocity&&aim_motion_square(*velocity)>9.99999993922529e-09f;
        if(movement_.field62cc==4){present_kind_four_aim(moving);return;}
        const auto first=frames_.actor_world_matrix(skeleton_.bones()[9].frame);
        const auto second=frames_.actor_world_matrix(skeleton_.bones()[8].frame);
        Vector up{{second[12]-first[12],second[13]-first[13],second[14]-first[14],second[15]}};
        normalize_human_ai_shot(up,(static_cast<long double>(up[0])*up[0]+
            static_cast<long double>(up[1])*up[1])+static_cast<long double>(up[2])*up[2]);
        auto basis=frames_.actor_world_matrix(model_);
        align_human_matrix_up(frames_,basis,up);
        auto direction=aim_hand_direction<true>();
        const auto saved_right=movement_.right_hand_offset6340;
        const auto blend=human_animation_word_float(movement_.field6318);
        Vector weights{{(moving?.15f:.25f)*blend,(moving?.7f:.75f)*blend,blend*0.f,0}};
        correct_human_bone_reference(frames_,skeleton_,model_,12,direction,movement_.direction6360,basis,weights);
        direction=aim_hand_direction<true>();
        weights[0]=.25f*blend;
        correct_human_bone_reference(frames_,skeleton_,model_,7,direction,movement_.direction6360,basis,weights);
        if(!moving&&!equipped_weapon_.butt()&&geometry_detail_)restore_aim_right_hand(saved_right);
        if(movement_.field62d8&0x80000000u)direction=aim_hand_direction<true>();
        finish_aimed_weapon(direction);
        present_aimed_arm_chains();
    }
    void present_special_weapon(){ //007BE6F0.
        limit_aim_vertical();
        limit_special_aim_forward();
        if(movement_.field62cc==4){present_kind_four_aim(false);return;}
        auto direction=aim_hand_direction<true>();
        const auto saved_right=movement_.right_hand_offset6340;
        const auto blend=human_animation_word_float(movement_.field6318);
        const Vector first_weights{{.35f*blend,.95f*blend,blend*0.f,0}};
        correct_human_bone_direction(frames_,skeleton_,model_,12,direction,movement_.direction6360,first_weights);
        direction=aim_hand_direction<true>();
        const Vector second_weights{{.45f*blend,.95f*blend,blend*0.f,0}};
        correct_human_bone_direction(frames_,skeleton_,model_,7,direction,movement_.direction6360,second_weights);
        if(!equipped_weapon_.butt()&&geometry_detail_)restore_aim_right_hand(saved_right);
        if(movement_.field62d8&0x80000000u)direction=aim_hand_direction<true>();
        finish_aimed_weapon(direction);
        present_aimed_arm_chains();
    }
    template<class Queries>void present_prone_weapon(Queries& queries){ //007BA830.
        //660C borrows a retained direction-work allocation, separate from the
        //body/point trees at6848/6874. Its second vector is initialized by0079BD30.
        const auto up=prone_pose_.vectors()[4][1];
        const auto right=frames_.actor_world_matrix(skeleton_.bones()[20].frame);
        const auto left=frames_.actor_world_matrix(skeleton_.bones()[21].frame);
        Vector start=up,displacement=up;
        for(unsigned axis=0;axis!=3;++axis){
            const float first=static_cast<float>(static_cast<long double>(right[12+axis])*.25f);
            const float second=static_cast<float>(static_cast<long double>(left[12+axis])*.75f);
            const float hands=first+second;
            const float offset=static_cast<float>(static_cast<long double>(up[axis])*.5f);
            start[axis]=offset+hands;
            displacement[axis]=static_cast<float>(static_cast<long double>(up[axis])*-2.f);
        }
        auto normal=up;
        if(const auto hit=queries.human_geometry_ray(start,displacement,0xc8102,0x230000,true))normal=hit->normal;
        auto& direction=movement_.direction6360;
        const auto dot=(static_cast<long double>(normal[2])*direction[2]+
            static_cast<long double>(normal[1])*direction[1])+static_cast<long double>(normal[0])*direction[0];
        const float stored=static_cast<float>(dot);
        if(dot<-.5f||stored>.3226f){
            const bool below=dot<-.5f;
            Vector planar=direction;
            for(unsigned axis=0;axis!=3;++axis){
                const float projection=static_cast<float>(static_cast<long double>(normal[axis])*stored);
                planar[axis]=direction[axis]-projection;
            }
            const auto length=std::sqrt(1.L-static_cast<long double>(stored)*stored);
            if(!(length>9.99999993922529e-09f)){
                direction=normal;
                if(below)for(unsigned axis=0;axis!=3;++axis)direction[axis]=-normal[axis];
            }else{
                const auto scale=(below?.866f:.9465f)/length;
                for(unsigned axis=0;axis!=3;++axis)planar[axis]=static_cast<float>(planar[axis]*scale);
                for(unsigned axis=0;axis!=3;++axis){
                    const float projection=static_cast<float>(static_cast<long double>(normal[axis])*(below?-.5f:.3226f));
                    direction[axis]=planar[axis]+projection;
                }
            }
        }
        limit_aim_forward(.866f,.5f);
        if(movement_.field62cc==4){
            movement_.field6350=direction;
            correct_aimed_bone(14,.25f,.5f);
            pose_equipment_weapon(movement_.field6350,true,true,1.f);
            if(movement_.field62d8&0x80000000u){
                movement_.left_hand_offset6330=frames_.car_transform(movement_.offset63a0,frames_.actor_world_matrix(skeleton_.bones()[21].frame));
                const auto grip=frames_.actor_world_matrix(equipped_weapon_.left());
                pose_hand_chain(17,19,Vector{{grip[12],grip[13],grip[14],grip[15]}},movement_.left_hand_offset6330,
                    skeleton_.bones()[21].frame,human_animation_word_float(movement_.field631c),nullptr);
            }
            return;
        }
        Vector current=direction;
        if(movement_.field62d8&0x80000000u)current=aim_hand_direction();
        finish_aimed_weapon(current);
        if(!geometry_detail_)return;
        refresh_weapon_hand_offsets();
        const Vector down{{0,-1,0,0}};
        if(movement_.field62d8&0x80000000u){
            //The original reads both RW and LW here, even when BUTT is absent.
            const auto right_grip=frames_.actor_world_matrix(equipped_weapon_.right());
            const auto left_grip=frames_.actor_world_matrix(equipped_weapon_.left());
            Vector target=movement_.left_hand_offset6330;
            for(unsigned axis=0;axis!=3;++axis){
                const float first=static_cast<float>(static_cast<long double>(right_grip[12+axis])*.25f);
                const auto second=static_cast<long double>(left_grip[12+axis])*.75f;
                target[axis]=static_cast<float>((axis==2?static_cast<float>(second):second)+first);
            }
            pose_hand_chain(17,19,target,movement_.left_hand_offset6330,skeleton_.bones()[21].frame,
                human_animation_word_float(movement_.field631c),equipped_weapon_.butt()?&down:nullptr);
        }
        if(equipped_weapon_.butt()){
            const auto grip=frames_.actor_world_matrix(equipped_weapon_.right());
            pose_hand_chain(16,18,Vector{{grip[12],grip[13],grip[14],grip[15]}},movement_.right_hand_offset6340,
                skeleton_.bones()[20].frame,1.f,nullptr);
        }
    }
    template<class Human,class Control>void present_aimed_weapon(Human& human,Control& control){ //007B9CB0.
        advance_aim_direction();
        if(!(control.mode_flags()&2u))return;
        const auto weapon=equipped_weapon_.model();
        if(!weapon)throw std::logic_error("Human aimed pose requires its actual retained weapon model");
        const auto parent=frames_.object_parent(weapon);
        frames_.human_geometry_weapon_link(weapon,Frame{});
        try{
            switch(movement_.field62d0){
            case 0:present_standing_weapon(&human.activation.vector1b0);break;
            case 1:present_crouched_weapon(&human.activation.vector1b0);break;
            case 2:present_prone_weapon(control);break;
            case 3:present_special_weapon();break;
            }
            if(control.human_is_player())adjust_aimed_camera();
        }catch(...){
            const auto failure=std::current_exception();
            try{frames_.human_geometry_weapon_link(weapon,parent);}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
            std::rethrow_exception(failure);
        }
        frames_.human_geometry_weapon_link(weapon,parent);
    }
    void present_weapon_mount(std::uint32_t mode){ //007BA0C0.
        movement_.time62c0-=frame_elapsed_; //Always subtracts, including an initially zero timer.
        const auto remaining=human_frame_signed_word(movement_.time62c0);
        if(remaining>=0)interpolate_aim_direction(remaining);
        if(!(mode&2u)||movement_.field62d0==2)return;
        auto& direction=movement_.direction6360;
        auto world=frames_.actor_world_matrix(model_);
        const auto vertical=(static_cast<long double>(world[6])*direction[2]+
            static_cast<long double>(world[5])*direction[1])+static_cast<long double>(world[4])*direction[0];
        if(std::abs(vertical)>.699999988079071){ //Original DOUBLE00810898.
            const float stored=static_cast<float>(vertical);
            world=frames_.actor_world_matrix(model_);
            const auto excess=static_cast<long double>(stored)-(std::signbit(stored)?-1.L:1.L)*.7f;
            for(unsigned axis=0;axis!=3;++axis){
                const float projection=static_cast<float>(excess*world[4+axis]);
                direction[axis]-=projection;
            }
            //This body stores the square before the unit-length comparison.
            normalize_human_ai_shot(direction,static_cast<float>(aim_motion_square(direction)));
        }
        world=frames_.actor_world_matrix(model_);
        const auto forward=(static_cast<long double>(direction[2])*world[10]+
            static_cast<long double>(direction[1])*world[9])+static_cast<long double>(direction[0])*world[8];
        if(forward<.34f){
            const float stored=static_cast<float>(forward);
            world=frames_.actor_world_matrix(model_);
            const auto excess=static_cast<long double>(stored)-.34f;
            direction[0]=static_cast<float>(direction[0]-excess*world[8]);
            const float y=static_cast<float>(excess*world[9]),z=static_cast<float>(excess*world[10]);
            direction[1]-=y;direction[2]-=z;
            normalize_human_ai_shot(direction,static_cast<float>(aim_motion_square(direction)));
        }
        const Vector weights{{.85f,.85f,0,0}};
        for(unsigned bone:{12u,7u}){
            const auto body=frames_.actor_world_matrix(skeleton_.bones()[7].frame);
            correct_human_bone_direction(frames_,skeleton_,model_,bone,
                Vector{{body[8],body[9],body[10],body[11]}},direction,weights);
        }
    }
    void pose_head_direction(const Vector& desired,float influence){ //007ADA10.
        const auto& head=skeleton_.bones()[6];
        const auto first=frames_.actor_world_matrix(head.frame);
        Vector forward{{first[8],first[9],first[10],first[11]}};
        normalize_human_ai_shot(forward,(static_cast<long double>(forward[1])*forward[1]+
            static_cast<long double>(forward[2])*forward[2])+static_cast<long double>(forward[0])*forward[0]);
        auto alignment=human_identity();
        human_align_vectors(forward,desired,alignment);
        alignment[15]=1;
        const auto current=frames_.human_matrix_rotation(frames_.actor_world_matrix(head.frame));
        const auto target=frames_.human_matrix_rotation(frames_.rigid_matrix_product(frames_.actor_world_matrix(head.frame),alignment));
        frames_.human_set_matrix_rotation(alignment,frames_.physics_slerp(current,target,influence,false));
        if(head.parent<0)throw std::logic_error("Human head direction requires its actual skeleton parent");
        auto inverse=human_transpose(frames_.actor_world_matrix(skeleton_.bones()[static_cast<std::size_t>(head.parent)].frame));
        inverse[3]=inverse[7]=inverse[11]=0;
        alignment=frames_.car_matrix_product(alignment,inverse);
        frames_.human_publish_bone_rotation(head.frame,alignment);
    }
    void present_look_geometry(std::uint32_t mode,std::uint32_t elapsed){ //007ACCC0.
        const auto pose=advance_human_look_pose(frames_,random_,look_,model_,skeleton_.bones()[6].frame,
            skeleton_.bones()[8].frame,mode,movement_,elapsed);
        if(pose)pose_head_direction(pose->direction,pose->influence);
    }
    template<class Human,class Control,class Services>
    void begin_mounted_effect(Human& human,Control& control,Human& source,Services& services){
        begin_geometry_effect(human,control,8,Vector{},Frame{},4,&source,services);
    }
    template<class Human,class Control,class Services>
    void begin_geometry_effect(Human& human,Control& control,std::uint32_t flags,const Vector& impulse,
            Frame clip,std::uint16_t reason,Human* source,Services& services){ //007B0910.
        if(!(control.mode_flags()&2u)){joints_.field68d0=0;return;}
        if(human_!=&human||control_!=&control||!model_)
            throw std::logic_error("Human geometry effect requires its SAME bound DD0 and model");
        geometry_effect_.emplace();
        auto& effect=*geometry_effect_;
        effect.reason=reason;effect.source=source;
        bool full_joints=reason==4;
        if((control.mode_flags()&7u)==7u){
            const bool quality=services.human_geometry_effect_quality();
            if(quality&&!full_joints){
                const auto camera=frames_.active_game_camera();
                if(!camera)throw std::runtime_error("Human reaction requires its actual active scene camera");
                const auto camera_world=frames_.actor_world_matrix(camera);
                const auto root=frames_.actor_world_matrix(skeleton_.bones()[9].frame);
                Vector displacement{};
                for(unsigned axis=0;axis!=3;++axis)displacement[axis]=root[12+axis]-camera_world[12+axis];
                const auto direction_world=frames_.actor_world_matrix(frames_.active_game_camera());
                long double x=direction_world[8],y=direction_world[9];float z=direction_world[10];
                const auto square=(y*y+static_cast<long double>(z)*z)+x*x;
                if(std::abs(square-1)>=9.99999993922529e-09f){
                    const float stored=static_cast<float>(square);
                    if(stored>=9.99999993922529e-09f){
                        const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));
                        x*=inverse;y*=inverse;z=static_cast<float>(z*inverse);
                    }else if(x!=0)x=x<0?-1.L:1.L;
                    else if(z!=0)z=z<0?-1.f:1.f;
                    else y=y<0?-1.L:1.L;
                }
                const float projection=static_cast<float>((static_cast<long double>(z)*displacement[2]+
                    y*displacement[1])+x*displacement[0]);
                const auto distance=(static_cast<long double>(displacement[2])*displacement[2]+
                    static_cast<long double>(displacement[1])*displacement[1])+static_cast<long double>(displacement[0])*displacement[0];
                full_joints=!std::signbit(projection)&&distance<100.f&&reason!=1&&reason!=2&&reason!=3;
            }
        }
        const auto animation=control.animation_state_word(0x22c)&0xff0u;
        if(!full_joints&&reason!=1&&reason!=2&&reason!=3&&!(animation&0x380u)&&
           !(movement_.field6304&0x68u))full_joints=services.human_geometry_effect_quality();
        if(!full_joints){
            effect.clip=clip;
            if(clip){
                for(unsigned bone=0;bone!=skeleton_.bones().size();++bone){
                    const auto frame=skeleton_.bones()[bone].frame;
                    effect.saved_local[bone]=frames_.car_local_matrix(frame);
                    effect.saved_world[bone]=frames_.actor_world_matrix(frame);
                }
                const auto root=frames_.actor_world_matrix(skeleton_.bones()[9].frame);
                effect.root_position={{root[12],root[13],root[14],root[15]}};
                if(reason==3){
                    effect.contact_positions.resize(7);
                    constexpr std::array<unsigned,7> bones{{6,4,8,13,9,2,3}};
                    for(unsigned index=0;index!=bones.size();++index){
                        const auto world=frames_.actor_world_matrix(skeleton_.bones()[bones[index]].frame);
                        effect.contact_positions[index]={{world[12],world[13],world[14],world[15]}};
                    }
                }else if(reason!=2){
                    effect.contact_positions.resize(3);
                    const auto up_world=frames_.actor_world_matrix(model_);
                    Vector up{{up_world[4],up_world[5],up_world[6],up_world[7]}};
                    Vector offset=up;
                    for(unsigned axis=0;axis!=3;++axis)offset[axis]*=.25f;
                    for(unsigned index=0;index!=3;++index){
                        const auto origin=frames_.actor_world_matrix(model_);
                        Vector position{};
                        if(index<2){
                            const auto world=frames_.actor_world_matrix(skeleton_.bones()[index?22:4].frame);
                            for(unsigned axis=0;axis!=3;++axis)position[axis]=world[12+axis]-origin[12+axis];
                        }else{
                            const auto right=frames_.actor_world_matrix(skeleton_.bones()[3].frame);
                            const auto left=frames_.actor_world_matrix(skeleton_.bones()[2].frame);
                            for(unsigned axis=0;axis!=3;++axis)
                                position[axis]=static_cast<float>((static_cast<long double>(right[12+axis])+left[12+axis])*.5f-origin[12+axis]);
                        }
                        const auto current=frames_.actor_world_matrix(model_);
                        const auto dot=(static_cast<long double>(position[2])*up[2]+
                            static_cast<long double>(position[1])*up[1])+static_cast<long double>(position[0])*up[0];
                        for(unsigned axis=0;axis!=3;++axis){
                            const auto projected=up[axis]*dot;
                            const auto flat=axis?position[axis]-static_cast<float>(projected):position[axis]-projected;
                            effect.contact_positions[index][axis]=static_cast<float>(flat+current[12+axis]+offset[axis]);
                        }
                    }
                }
                movement_.field6304|=0x200u;
                capture_effect_camera();
                return;
            }
        }
        if(services.human_unavailable(human)&&source){
            joints_.build(false);
            for(unsigned index=0;index!=saved_body_angles_.size();++index)
                if(auto* body=joints_.bodies()[index])body->set(0x180,saved_body_angles_[index]);
        }
        joints_.gravity[0]=0;joints_.gravity[1]=-9;joints_.gravity[2]=0;
        joints_.field68d0=1;
        for(auto* point:joints_.points())if(point)point->set(0x24,1.f);
        joints_.field6900=6;joints_.field68e8=0;joints_.field68ec=1;joints_.field68f0=4;
        effect.clip=Frame{};
        if(joints_.field690c)joints_.retire_contact_frames();
        joints_.flag6912=false;
        constexpr std::array<unsigned,9> bones{{9,12,13,7,15,14,4,5,6}};
        constexpr std::array<unsigned,9> saved{{0,1,2,3,5,6,7,8,9}};
        for(unsigned index=0;index!=bones.size();++index){
            if(index==4)(void)services.human_unavailable(human); //Native repeated query before restoring both arms.
            const auto frame=skeleton_.bones()[bones[index]].frame;
            if(!frame)throw std::runtime_error("Human geometry effect lost its actual saved-pose bone");
            frames_.human_equipment_local_matrix(frame,joints_.saved_local()[saved[index]]);
        }
        const auto root=skeleton_.bones()[9].frame;
        if(!root)throw std::runtime_error("Human geometry effect requires its actual pose root");
        frames_.update_scene_object(root);
        joints_.rebind_effect_bodies([&]{return services.human_unavailable(human);});
        auto velocity=movement_.field63d0;velocity[1]=0;
        const float square=static_cast<float>(static_cast<long double>(velocity[0])*velocity[0]+
            static_cast<long double>(velocity[2])*velocity[2]);
        if(square>25.f){
            const auto scale=human_joint_inverse_root(square)*5.f;
            for(unsigned axis=0;axis!=3;++axis)velocity[axis]=static_cast<float>(velocity[axis]*scale);
        }
        const auto interval=static_cast<long double>(human_frame_signed_word(frame_elapsed_))*.0005500000552274287f;
        for(unsigned axis=0;axis!=3;++axis)velocity[axis]=static_cast<float>(velocity[axis]*interval);
        if((flags&0x1000u)||source){
            joints_.gravity[0]=0;joints_.gravity[1]=-9.81f;joints_.gravity[2]=0;
            if(source)velocity[0]=velocity[1]=velocity[2]=0;
        }else if(flags&0x8400u){
            joints_.gravity[0]=0;joints_.gravity[1]=-2.25f;joints_.gravity[2]=0;joints_.flag6912=true;
        }
        joints_.initialize_effect_points(velocity);
        if(services.human_unavailable(human)&&!source)joints_.retire_unavailable_effect_limbs();
        joints_.capture_body_pose();joints_.snapshot_body_endpoints();
        auto* point=joints_.points()[0];
        if(!point)throw std::runtime_error("Human geometry effect requires its actual point root");
        joints_.reset_point(point,point->template get<Vector>(0x30),false);
        frames_.detach_human_effect_model(model_);
        effect.root_position=point->template get<Vector>(0x30);
        frames_.register_human_effect_model(model_,marker_z_,skeleton_.bones()[1].frame);
        frames_.refresh_dynamic_human_model(model_);
        if(!skeleton_.bones()[1].frame&&!source)initialize_effect_impulse(flags,impulse,services);
        movement_.field6304|=0x200u;
        capture_effect_camera();
        auto& inline_owner=services.human_inline(human);
        try{
            inline_owner.exclude_query(false);
            for(auto* joint:joints_.points())if(joint){
                auto* actor=services.human_geometry_containing_actor(joint->template get<Vector>(0x30),0xd8102u);
                if(actor){attachment_pose_.actor(actor);movement_.field6304|=0x10u;break;}
            }
        }catch(...){
            const auto primary=std::current_exception();
            try{inline_owner.restore_query(false);}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
            std::rethrow_exception(primary);
        }
        inline_owner.restore_query(false);
    }
    void command_look(std::uint32_t flags,const Vector* target) {
        look_.command(frames_,random_,model_,skeleton_.bones()[6].frame,flags,target);
    }
    const HumanLookState& look() const {return look_;}
    void equipment_face_flag(bool enabled) const { //007B85A0 dispatches only when62B0 exists.
        if(!model_)throw std::logic_error("Equipment face flag requires bound Human geometry");
        if(face_model_)face_model_->set_shadow(enabled);
    }
    const auto* offline_face_model() const{return face_model_.get();}
    void set_equipment_weapon(Frame model,std::int32_t kind){equipped_weapon_.select(model,kind);}
    void equipment_weapon_pose(unsigned command){equipped_weapon_.pose(command);}
    void stow_equipment_weapon(bool enabled){equipped_weapon_.stow(enabled);}
    const auto& equipped_weapon() const{return equipped_weapon_;}
    struct Movement {
        bool flag62c8=false,flag65f0=false,flag6624=false;
        std::uint32_t field62cc=0,field62d0=0,field62d4=0,field65f4=0,field62d8=1,
            field6304=0,field6308=0,field630c=300,field6310=0,field6314=0,field6318=0,field631c=0,field6410=100;
        std::int32_t field62dc=0;
        std::uint32_t time62c0=0,time62c4=0;
        float angle63c0=0,sine63c4=0;
        Vector direction6360{},direction6370{},direction6380{},offset6390{},
            position6400{},field6350{},field63d0{},field63e0{},field63f0{},left_hand_offset6330{},right_hand_offset6340{},
            offset63a0{{-.075f,-.041f,-.003f,0}},offset63b0{{.075f,-.041f,-.003f,0}};
    };
    const Movement& movement() const {return movement_;}
    void seat_posture(std::uint32_t posture){ //00755500 updates SAME native62D0/65F0/65F4 after E0 posture.
        movement_.field62d0=posture;
        if(posture==2){movement_.flag65f0=true;movement_.field65f4=500;}
        else movement_.flag65f0=false;
    }
    const auto& attachment_pose()const{return attachment_pose_;}
    const auto& prone_pose()const{return prone_pose_;}
    template<class Human,class Control>void present_prone_pose(Human& human,Control& control){
        prone_pose_.present(frames_,skeleton_,model_,human,control,movement_,frame_elapsed_,seed_);
    }
    template<class Services>void select_attachment_pose(std::uint32_t flags,void* actor,std::uint32_t seat,Services& services){
        attachment_pose_.select(flags,actor,seat,services);
    }
    template<class Services>void present_attached_pose(std::uint32_t mode,Services& services){
        attachment_pose_.present_attached(mode,geometry_detail_,frame_elapsed_,model_,services);
    }
    template<class Services>void present_target_pose(std::uint32_t mode,Services& services){
        attachment_pose_.present_target(mode,geometry_detail_,frame_elapsed_,model_,services);
    }
    template<class Services>void present_vehicle_pose(std::uint32_t mode,Services& services){
        attachment_pose_.present_vehicle(mode,geometry_detail_,model_,fields661c_,fields6620_,services);
    }
    void clear_ground_geometry_flag(){movement_.flag65f0=false;} //007637D0, native65F0.
    void set_posture_transition(std::uint32_t posture){ //0076AF70, SAME geometry62D0/65F0/65F4.
        movement_.field62d0=posture;
        if(posture==2){movement_.flag65f0=true;movement_.field65f4=500;}
    }
    void capture_action_overlay(Frame clip){ //007B8C10, six retained key pairs for each original bone.
        if(!clip){movement_.flag6624=false;return;}
        std::uint32_t time=0; //Native shares this ignored key-time output between both engine calls.
        for(unsigned bone=0;bone!=human_bone_names.size();++bone){
            const auto index=frames_.human_clip_bone_index(clip,human_bone_names[bone]);
            for(std::int32_t key=0;key!=6;++key){
                auto& position=fields6620_[static_cast<unsigned>(key)][bone];
                if(key<frames_.human_clip_position_count(clip,index))
                    frames_.human_clip_position_key(clip,index,key,time,position);
                else position[0]=std::numeric_limits<float>::quiet_NaN();
                auto& rotation=fields661c_[static_cast<unsigned>(key)][bone];
                if(key<frames_.human_clip_rotation_count(clip,index))
                    frames_.human_clip_rotation_key(clip,index,key,time,rotation);
                else rotation[3]=std::numeric_limits<float>::quiet_NaN();
            }
        }
        movement_.flag6624=true;
    }
    void apply_hit_impulse(std::uint32_t mask,const Vector& value,std::int32_t duration){joints_.distribute_impulse(random_,mask,value,duration);} //007B3270, SAME process CRT and joint queues.
    template<class Services>void command_hit(std::uint32_t mask,const Vector& position,const Vector& direction,Services& services){ //007B8450.
        if(face_model_){
            auto clock=[&]{return services.game_frame_serial();};
            face_model_->apply_hit(position,direction,mask,random_,clock);
        }
        if(!(movement_.field6304&0x200u)||(geometry_effect_&&geometry_effect_->source))return;
        const float square=static_cast<float>((static_cast<long double>(direction[0])*direction[0]+
            static_cast<long double>(direction[1])*direction[1])+static_cast<long double>(direction[2])*direction[2]);
        if(!mask||!(square>9.99999993922529e-09f))return;
        const auto length=std::sqrt(static_cast<long double>(square)),inverse=1.L/length;
        auto impulse=direction; //Native temporary W is unwritten. Retain the known input padding.
        for(unsigned axis=0;axis!=3;++axis)impulse[axis]=static_cast<float>(inverse*direction[axis]);
        const auto scaled=length*.1111111119389534f;
        const float amount=scaled>150.f?150.f:static_cast<float>(scaled);
        for(unsigned axis=0;axis!=3;++axis)impulse[axis]=static_cast<float>(static_cast<long double>(impulse[axis])*amount);
        impulse[1]=static_cast<float>(static_cast<long double>(amount)*.3333333432674408f+impulse[1]);
        if(joints_.field68d0<9.99999993922529e-09f){
            auto* root=joints_.points()[0];
            if(!root)throw std::logic_error("Human hit impulse requires its actual initialized root point");
            joints_.reset_point(root,root->template get<Vector>(0x30),true);
        }
        apply_hit_impulse(mask,impulse,200);
    }
    void command_target_direction(const Vector& target,bool player) { //007B8E80, no normalization of its input.
        if(!(movement_.field6304&1u))return;
        auto snap=[&]{movement_.direction6360=movement_.direction6380=movement_.direction6370=target;movement_.time62c0=0;};
        if(human_&&player){snap();return;} //Native leaves62C4 and angle/sine untouched.
        const auto& current=movement_.direction6360;
        const auto dot=(static_cast<long double>(current[2])*target[2]+static_cast<long double>(current[1])*target[1])+static_cast<long double>(current[0])*target[0];
        movement_.angle63c0=static_cast<float>(dot);
        if(dot>1)movement_.angle63c0=1;else if(dot< -1)movement_.angle63c0=-1;
        if(1.L-std::abs(static_cast<long double>(movement_.angle63c0))<9.99999993922529e-09){snap();return;}
        const auto angle=std::acos(static_cast<long double>(movement_.angle63c0));
        movement_.angle63c0=static_cast<float>(angle);movement_.sine63c4=static_cast<float>(std::sin(angle));
        const auto duration=static_cast<long double>(movement_.angle63c0)*95.49296569824219f; //00816E78.
        movement_.time62c0=movement_.time62c4=std::isfinite(duration)?static_cast<std::uint32_t>(duration):0; //__ftol QWORD indefinite has LOW DWORD0.
        movement_.direction6370=movement_.direction6360;movement_.direction6380=target;
    }
    void blend_equipment_weapon(std::int32_t duration){equipped_weapon_.blend(duration);} //007B8D00.
    void unblend_equipment_weapon(std::int32_t duration){equipped_weapon_.unblend(duration);} //007B8D60.
    void suspend_equipment_weapon(std::int32_t duration,bool detach){equipped_weapon_.suspend(duration,detach);} //007B8E10.
    void suspend_equipment_for_item_action(std::int32_t duration){equipped_weapon_.suspend_for_item_action(duration);} //007B8DC0.
    void set_weapon_blend_duration(std::int32_t duration){ //00778B20 writes630C before6308, retaining actual FLOAT6318.
        movement_.field630c=static_cast<std::uint32_t>(duration);
        movement_.field6308=static_cast<std::uint32_t>(human_animation_time_integer(static_cast<long double>(duration)*human_animation_word_float(movement_.field6318)));
    }
    //007A7470's common frame sampling and selected ragdoll branch. The normal
    //presentation continuation shares this exact prefix, including its clock
    //and motion samples; it does not run when the reaction branch consumes it.
    template<class Human,class Control>bool present_reaction_frame(Human& human,Control& control,std::uint32_t elapsed){
        if(human_!=&human||control_!=&control||!model_)throw std::logic_error("Human geometry frame lost its actual owners");
        frame_elapsed_=elapsed;++seed_; //Native62B4/62B8.
        auto countdown=movement_.field6410-elapsed;
        movement_.field6410=countdown;
        if(human_frame_signed_word(countdown)<1){
            movement_.field6410=countdown+100u;
            movement_.field63e0=movement_.field63d0;
            const auto world=frames_.actor_world_matrix(model_);
            const float scale=static_cast<float>(1000.L/human_frame_signed_word(elapsed)); //0080F708.
            for(unsigned axis=0;axis!=2;++axis)
                movement_.field63f0[axis]=static_cast<float>((static_cast<long double>(world[12+axis])-movement_.position6400[axis])*scale);
            const float z=world[14]-movement_.position6400[2];
            movement_.field63f0[2]=z*scale;
        }
        const auto world=frames_.actor_world_matrix(model_);
        movement_.position6400={{world[12],world[13],world[14],world[15]}};
        const float fraction=static_cast<float>(static_cast<long double>(human_frame_signed_word(movement_.field6410))*.01f);
        const float complement=1.f-fraction;
        for(unsigned axis=0;axis!=3;++axis){
            const float latest=complement*movement_.field63f0[axis];
            movement_.field63d0[axis]=static_cast<float>(static_cast<long double>(fraction)*movement_.field63e0[axis]+latest);
        }
        geometry_detail_=(control.animation_state_word(0x22c)&0x380u)==0; //Native62C9.
        if(movement_.field6304&0x200u){
            control.present_ragdoll_geometry();
            return true;
        }
        return false;
    }
    template<class Human,class Control,class Runtime>void present_frame(Human& human,Control& control,std::uint32_t elapsed,Runtime& runtime){ //007A7470.
        if(present_reaction_frame(human,control,elapsed))return;
        if(human_frame_signed_word(human.properties.stats_ec[0])<1)return;
        if(movement_.flag65f0&&control.animation_look_word(0xb8)!=5&&
           ((control.mode_flags()&7u)==7u||control.human_is_player()))
            present_prone_pose(human,control); //007A8520.
        const auto flags=movement_.field6304;
        if((flags&1u)||(!(flags&0x3du)&&(flags&2u))){
            const auto weapon=equipped_weapon_.model();
            if(!weapon||!frames_.object_parent(weapon)||(!equipped_weapon_.left()&&!equipped_weapon_.right())||
               ((movement_.field62d8&0x80000000u)&&!equipped_weapon_.left())){
                movement_.field6304=0;movement_.flag62c8=false;
                return;
            }
            if(flags&1u)present_aimed_weapon(human,control); //007B9CB0.
            else if((control.mode_flags()&2u)&&movement_.field62cc!=4&&(movement_.field62d8&0x80000000u))
                present_stowed_weapon(control.mode_flags()); //007B9750.
        }else if(flags&4u)present_weapon_mount(control.mode_flags()); //007BA0C0.
        else if(flags&8u){
            control.present_attached_geometry(); //007AE3C0.
            const auto world=frames_.actor_world_matrix(model_);
            pose_head_direction(Vector{{world[8],world[9],world[10],world[11]}},1.f); //007ADA10.
        }
        else if(flags&0x10u)control.present_vehicle_geometry(); //007AF320.
        else if(flags&0x20u)control.present_target_geometry(); //007AEBE0.
        equipped_weapon_.advance_frame(elapsed);
        if(movement_.field6304&0x40u)control.present_target_geometry();
        if(movement_.field6304&0x100u)present_equipment_overlay(); //007AFD70.
        if(movement_.field6304&0x80u)present_secondary_clip(); //007AFC20.
        if(control.mode_flags()&2u){
            if(look_.flags||(movement_.field6304&0x35u))present_look_geometry(control.mode_flags(),elapsed); //007ACCC0.
            control.resolve_model_geometry_camera();
        }
        if(runtime.human_model_debug_enabled())runtime.present_human_model_debug(*this);
    }
private:
    void capture_effect_camera(){ //007B0910's6830 publication in both retained-clip and joint branches.
        const auto camera=camera_fpv();
        if(!camera)throw std::runtime_error("Human geometry effect requires its actual FPV camera");
        const auto world=frames_.actor_world_matrix(camera);
        geometry_effect_->camera_position={{world[12],world[13],world[14],world[15]}};
    }
    template<class Services>void initialize_effect_impulse(std::uint32_t flags,Vector impulse,Services& services){
        if(flags&0x4000u){ //007B2B8A, explosion branch omitted by older alias-damaged exports.
            const auto square=(static_cast<long double>(impulse[0])*impulse[0]+
                static_cast<long double>(impulse[1])*impulse[1])+static_cast<long double>(impulse[2])*impulse[2];
            const auto length=std::sqrt(square),inverse=1.L/length;
            for(unsigned axis=0;axis!=3;++axis)impulse[axis]=static_cast<float>(impulse[axis]*inverse);
            float amount=static_cast<float>(length*.2f);if(amount>450.f)amount=450.f;
            for(unsigned axis=0;axis!=3;++axis)impulse[axis]*=amount;
            impulse[1]=static_cast<float>(static_cast<long double>(amount)*.3333333432674408f+impulse[1]);
            apply_hit_impulse(flags,impulse,200);
            return;
        }
        const auto world=frames_.actor_world_matrix(model_);
        const float projection=static_cast<float>((static_cast<long double>(impulse[1])*world[9]+
            static_cast<long double>(impulse[2])*world[10])+static_cast<long double>(impulse[0])*world[8]);
        if(std::signbit(projection)){
            const auto forward=frames_.actor_world_matrix(model_);
            Vector offset{};
            for(unsigned axis=0;axis!=3;++axis)offset[axis]=forward[8+axis]*.65f;
            const auto root=frames_.actor_world_matrix(skeleton_.bones()[9].frame);
            Vector origin{};
            for(unsigned axis=0;axis!=3;++axis)origin[axis]=offset[axis]+root[12+axis];
            if(!services.human_geometry_ray(origin,Vector{{0,-3,0,0}},0xffffffffu,0,false)){
                const auto first=frames_.actor_world_matrix(model_);
                const auto second=frames_.actor_world_matrix(model_);
                const auto dot=(static_cast<long double>(impulse[1])*second[5]+
                    static_cast<long double>(impulse[2])*second[6])+static_cast<long double>(impulse[0])*second[4];
                for(unsigned axis=0;axis!=3;++axis){
                    const auto product=dot*first[4+axis];
                    impulse[axis]=-static_cast<float>(axis?impulse[axis]-static_cast<float>(product):impulse[axis]-product);
                }
            }
        }
        const auto square=(static_cast<long double>(impulse[0])*impulse[0]+
            static_cast<long double>(impulse[1])*impulse[1])+static_cast<long double>(impulse[2])*impulse[2];
        float amount=1;
        long double x=impulse[0];
        if(!(square<9.99999993922529e-09f)){
            const auto length=std::sqrt(square),inverse=1.L/length;
            x*=inverse;impulse[1]=static_cast<float>(impulse[1]*inverse);impulse[2]=static_cast<float>(impulse[2]*inverse);
            amount=static_cast<float>(length*.2222222238779068f);if(amount>150.f)amount=150.f;
        }
        impulse[0]=static_cast<float>(x*amount);impulse[1]*=amount;impulse[2]*=amount;
        const auto combined_x=static_cast<long double>(impulse[0])+movement_.field63d0[0];
        const auto combined_y=static_cast<long double>(impulse[1])+movement_.field63d0[1];
        const auto combined_z=static_cast<long double>(impulse[2])+movement_.field63d0[2];
        if((combined_z*combined_z+combined_y*combined_y)+combined_x*combined_x<225.f){
            for(unsigned index:{12u,15u})if(auto* point=joints_.points()[index])
                joints_.queue_secondary_impulse(point,point->template get<Vector>(0x30),300);
            const auto inverse=1.L/amount;amount=30.f;
            impulse[0]=static_cast<float>(static_cast<long double>(impulse[0])*inverse*30.f);
            impulse[1]=static_cast<float>(impulse[1]*inverse);impulse[2]=static_cast<float>(impulse[2]*inverse);
            impulse[1]*=30.f;impulse[2]*=30.f;
        }
        impulse[1]=static_cast<float>(static_cast<long double>(amount)*.3333333432674408f+impulse[1]);
        apply_hit_impulse(flags,impulse,250);
    }
    void stop_contact_voice(std::int32_t& handle)noexcept{
        try{
            if(!stop_voice_)throw std::logic_error("Human contact voice lost its actual audio retirement binding");
            stop_voice_(handle);
        }catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
    }
    void advance_aim_direction(){ //007B9CB0, actual62C0 countdown and retained angle/sine.
        if(!movement_.time62c0)return;
        movement_.time62c0-=frame_elapsed_;
        const auto remaining=human_frame_signed_word(movement_.time62c0);
        if(remaining<0){movement_.time62c0=0;movement_.direction6360=movement_.direction6380;return;}
        interpolate_aim_direction(remaining);
    }
    void interpolate_aim_direction(std::int32_t remaining){ //Shared arithmetic007B9CB0/007BA0C0, distinct timer policies.
        const float fraction=static_cast<float>(static_cast<long double>(remaining)/human_frame_signed_word(movement_.time62c4));
        const float inverse_sine=static_cast<float>(1.L/movement_.sine63c4);
        const auto finish=std::sin((1.L-fraction)*movement_.angle63c0)*inverse_sine;
        Vector target=movement_.direction6360;
        for(unsigned axis=0;axis!=3;++axis)target[axis]=static_cast<float>(finish*movement_.direction6380[axis]);
        const auto start=std::sin(static_cast<long double>(fraction)*movement_.angle63c0)*inverse_sine;
        for(unsigned axis=0;axis!=3;++axis){
            const float previous=static_cast<float>(start*movement_.direction6370[axis]);
            movement_.direction6360[axis]=target[axis]+previous;
        }
    }
    void adjust_aimed_camera(){ //007B9CB0, real retained62F8 and head bone6.
        const auto camera=camera3rd();
        if(!camera)throw std::logic_error("Human aimed camera requires its actual retained camera3RD frame");
        const auto origin=frames_.actor_world_matrix(model_);
        auto delta=frames_.car_transform(movement_.offset6390,frames_.actor_world_matrix(skeleton_.bones()[6].frame));
        for(unsigned axis=0;axis!=3;++axis)delta[axis]-=origin[12+axis];
        const auto basis=frames_.actor_world_matrix(model_);
        const auto position=frames_.actor_world_matrix(model_);
        const auto previous=frames_.actor_world_matrix(camera);
        const float old_projection=static_cast<float>(((static_cast<long double>(previous[14])-position[14])*basis[2]+
            (static_cast<long double>(previous[13])-position[13])*basis[1])+
            (static_cast<long double>(previous[12])-position[12])*basis[0]);
        const auto axis=frames_.actor_world_matrix(model_);
        const auto projection_basis=frames_.actor_world_matrix(model_);
        const auto difference=((static_cast<long double>(delta[1])*projection_basis[1]+
            static_cast<long double>(delta[2])*projection_basis[2])+static_cast<long double>(delta[0])*projection_basis[0])-old_projection;
        delta[0]=static_cast<float>(delta[0]-difference*axis[0]);
        const float y=static_cast<float>(difference*axis[1]),z=static_cast<float>(difference*axis[2]);
        delta[1]-=y;delta[2]-=z;
        const auto world=frames_.actor_world_matrix(model_);
        for(unsigned component=0;component!=3;++component)delta[component]+=world[12+component];
        const float vertical=std::abs(movement_.direction6360[1]);
        const auto current=frames_.actor_world_matrix(camera);
        const auto amount=static_cast<long double>(vertical)*human_animation_word_float(movement_.field6318);
        const auto complement=1.L-amount;
        const float old_x=static_cast<float>(complement*current[12]),old_y=static_cast<float>(complement*current[13]);
        const float new_y=static_cast<float>(delta[1]*amount),new_z=static_cast<float>(delta[2]*amount);
        frames_.object_position(camera,{{static_cast<float>(delta[0]*amount+old_x),new_y+old_y,
            static_cast<float>(new_z+complement*current[14])}});
    }
    static long double aim_motion_square(const Vector& value){
        return (static_cast<long double>(value[0])*value[0]+static_cast<long double>(value[1])*value[1])+
            static_cast<long double>(value[2])*value[2];
    }
    void limit_aim_vertical(){ //Shared prefix007BCFC0/007BB540/007BE6F0.
        auto world=frames_.actor_world_matrix(model_);
        auto& direction=movement_.direction6360;
        const auto dot=(static_cast<long double>(direction[2])*world[6]+
            static_cast<long double>(direction[1])*world[5])+static_cast<long double>(direction[0])*world[4];
        const float stored=static_cast<float>(dot);
        if(!(dot>.7f||stored<-.7f))return;
        const float sign=std::signbit(stored)?-1.f:1.f;
        world=frames_.actor_world_matrix(model_);
        Vector planar=direction;
        for(unsigned axis=0;axis!=3;++axis){
            const float projection=static_cast<float>(static_cast<long double>(stored)*world[4+axis]);
            planar[axis]=direction[axis]-projection;
        }
        const auto length=std::sqrt(1.L-static_cast<long double>(stored)*stored);
        if(!(length>9.99999993922529e-09f)){
            world=frames_.actor_world_matrix(model_);
            for(unsigned axis=0;axis!=3;++axis)direction[axis]=sign*world[4+axis];
            return;
        }
        const auto scale=.7141f/length;
        for(unsigned axis=0;axis!=3;++axis)planar[axis]=static_cast<float>(planar[axis]*scale);
        world=frames_.actor_world_matrix(model_);
        for(unsigned axis=0;axis!=3;++axis){
            const float projection=static_cast<float>(static_cast<long double>(sign)*.7f*world[4+axis]);
            direction[axis]=planar[axis]+projection;
        }
    }
    void limit_aim_forward(float minimum,float tangent){ //007BCFC0/007BB540 and007BA830.
        auto world=frames_.actor_world_matrix(model_);
        auto& direction=movement_.direction6360;
        const auto dot=(static_cast<long double>(direction[2])*world[10]+
            static_cast<long double>(direction[1])*world[9])+static_cast<long double>(direction[0])*world[8];
        if(!(dot<minimum))return;
        const float stored=static_cast<float>(dot);
        world=frames_.actor_world_matrix(model_);
        Vector planar=direction;
        for(unsigned axis=0;axis!=3;++axis){
            const float projection=static_cast<float>(static_cast<long double>(stored)*world[8+axis]);
            planar[axis]=direction[axis]-projection;
        }
        const auto length=std::sqrt(1.L-static_cast<long double>(stored)*stored);
        if(!(length>9.99999993922529e-09f)){
            world=frames_.actor_world_matrix(model_);
            direction={{world[8],world[9],world[10],world[11]}};
            return;
        }
        const auto scale=static_cast<long double>(tangent)/length;
        for(unsigned axis=0;axis!=3;++axis)planar[axis]=static_cast<float>(planar[axis]*scale);
        world=frames_.actor_world_matrix(model_);
        for(unsigned axis=0;axis!=3;++axis){
            const float projection=static_cast<float>(static_cast<long double>(minimum)*world[8+axis]);
            direction[axis]=planar[axis]+projection;
        }
    }
    void limit_special_aim_forward(){ //007BE6F0 has distinct projection/addition spills.
        auto world=frames_.actor_world_matrix(model_);
        auto& direction=movement_.direction6360;
        const auto dot=(static_cast<long double>(direction[2])*world[10]+
            static_cast<long double>(direction[1])*world[9])+static_cast<long double>(direction[0])*world[8];
        if(!(dot<.707f))return;
        const float stored=static_cast<float>(dot);
        world=frames_.actor_world_matrix(model_);
        const float z=static_cast<float>(static_cast<long double>(stored)*world[10]);
        const float x=static_cast<float>(direction[0]-static_cast<long double>(stored)*world[8]);
        const auto y=direction[1]-static_cast<long double>(stored)*world[9];
        const auto length=std::sqrt(1.L-static_cast<long double>(stored)*stored);
        if(!(length>9.99999993922529e-09f)){
            world=frames_.actor_world_matrix(model_);
            direction={{world[8],world[9],world[10],world[11]}};
            return;
        }
        const auto scale=.707f/length;
        const Vector planar{{static_cast<float>(x*scale),static_cast<float>(y*scale),
            static_cast<float>((static_cast<long double>(direction[2])-z)*scale),direction[3]}};
        world=frames_.actor_world_matrix(model_);
        for(unsigned axis=0;axis!=3;++axis)
            direction[axis]=static_cast<float>(static_cast<long double>(world[8+axis])*.707f+planar[axis]);
    }
    template<bool XFirst=false>Vector aim_hand_direction(){
        refresh_weapon_hand_offsets();
        auto direction=movement_.left_hand_offset6330;
        for(unsigned axis=0;axis!=3;++axis)direction[axis]-=movement_.right_hand_offset6340[axis];
        const auto x=static_cast<long double>(direction[0])*direction[0];
        const auto y=static_cast<long double>(direction[1])*direction[1];
        const auto z=static_cast<long double>(direction[2])*direction[2];
        normalize_human_ai_shot(direction,XFirst?(x+z)+y:(z+y)+x);
        return direction;
    }
    void correct_aimed_bone(unsigned bone,float x,float y){
        const auto hand=frames_.actor_world_matrix(skeleton_.bones()[20].frame);
        const Vector direction{{hand[0],hand[1],hand[2],hand[3]}};
        const auto blend=human_animation_word_float(movement_.field6318);
        const Vector weights{{x*blend,y*blend,blend*0.f,0}};
        correct_human_bone_direction(frames_,skeleton_,model_,bone,direction,movement_.direction6360,weights);
    }
    void present_kind_four_aim(bool moving){ //Identical kind4 tail007BCFC0/007BB540, also007BE6F0's stationary case.
        if(moving){
            const auto first=frames_.actor_world_matrix(skeleton_.bones()[7].frame);
            const auto second=frames_.actor_world_matrix(skeleton_.bones()[4].frame);
            Vector up{{second[12]-first[12],second[13]-first[13],second[14]-first[14],second[15]}};
            normalize_human_ai_shot(up,(static_cast<long double>(up[2])*up[2]+
                static_cast<long double>(up[1])*up[1])+static_cast<long double>(up[0])*up[0]);
            const auto blend=human_animation_word_float(movement_.field6318);
            const Vector weights{{blend*0.f,blend*0.f,blend*.5f,0}};
            correct_human_bone_direction(frames_,skeleton_,model_,7,up,Vector{{0,1,0,0}},weights);
            correct_aimed_bone(12,.15f,.35f);
            correct_aimed_bone(7,.25f,.35f);
            correct_aimed_bone(14,.25f,.5f);
        }else{
            correct_aimed_bone(12,.35f,.75f);
            correct_aimed_bone(7,.35f,.75f);
            correct_aimed_bone(14,.75f,.75f);
        }
        refresh_weapon_hand_offsets();
        const auto hand=frames_.actor_world_matrix(skeleton_.bones()[20].frame);
        movement_.field6350=interpolate_human_directions(Vector{{hand[0],hand[1],hand[2],hand[3]}},
            movement_.direction6360,human_animation_word_float(movement_.field6318));
        pose_equipment_weapon(movement_.field6350,true,true,1.f);
        if(movement_.field62d8&0x80000000u){
            movement_.left_hand_offset6330=frames_.car_transform(movement_.offset63a0,frames_.actor_world_matrix(skeleton_.bones()[21].frame));
            const auto grip=frames_.actor_world_matrix(equipped_weapon_.left());
            pose_hand_chain(17,19,Vector{{grip[12],grip[13],grip[14],grip[15]}},movement_.left_hand_offset6330,
                skeleton_.bones()[21].frame,human_animation_word_float(movement_.field631c),nullptr);
        }
    }
    void restore_aim_right_hand(const Vector& saved){
        refresh_weapon_hand_offsets();
        const auto& current=movement_.right_hand_offset6340;
        Vector target=saved;
        target[0]=static_cast<float>((static_cast<long double>(current[0])-saved[0])*.8f+saved[0]);
        const float y=static_cast<float>((static_cast<long double>(current[1])-saved[1])*.8f);
        target[1]=y+saved[1];
        const float z=current[2]-saved[2];
        target[2]=static_cast<float>(static_cast<long double>(z)*.8f+saved[2]);
        pose_hand_chain(16,18,target,current,skeleton_.bones()[20].frame,1.f,nullptr);
    }
    void finish_aimed_weapon(Vector direction){
        const auto selected=movement_.field62d8&0xffu;
        if(selected==0||selected==1){
            const auto hand=frames_.actor_world_matrix(selected==1?weapon1_:weapon2_);
            Vector reference{{hand[0],hand[1],hand[2],hand[3]}};
            if(!selected)for(unsigned axis=0;axis!=3;++axis)reference[axis]=-reference[axis];
            direction=(movement_.field62d8&0x80000000u)?
                interpolate_human_directions(reference,direction,human_animation_word_float(movement_.field631c)):reference;
        }
        movement_.field6350=interpolate_human_directions(direction,movement_.direction6360,human_animation_word_float(movement_.field6318));
        pose_equipment_weapon(movement_.field6350,true,true,human_animation_word_float(movement_.field631c));
    }
    void present_aimed_arm_chains(){
        if(!geometry_detail_)return;
        refresh_weapon_hand_offsets();
        const Vector down{{0,-1,0,0}};
        if(movement_.field62d8&0x80000000u){
            const auto grip=frames_.actor_world_matrix(equipped_weapon_.left());
            pose_hand_chain(17,19,Vector{{grip[12],grip[13],grip[14],grip[15]}},movement_.left_hand_offset6330,
                skeleton_.bones()[21].frame,human_animation_word_float(movement_.field631c),equipped_weapon_.butt()?&down:nullptr);
        }
        if(equipped_weapon_.butt()){
            const auto grip=frames_.actor_world_matrix(equipped_weapon_.right());
            pose_hand_chain(16,18,Vector{{grip[12],grip[13],grip[14],grip[15]}},movement_.right_hand_offset6340,
                skeleton_.bones()[20].frame,1.f,nullptr);
        }
    }
    void refresh_weapon_hand_offsets(){ //007AFD70/007B9750, actual right-before-left engine queries.
        movement_.right_hand_offset6340=frames_.car_transform(movement_.offset63b0,frames_.actor_world_matrix(skeleton_.bones()[20].frame));
        movement_.left_hand_offset6330=frames_.car_transform(movement_.offset63a0,frames_.actor_world_matrix(skeleton_.bones()[21].frame));
    }
    void present_weapon_overlay(const Vector& weights){ //007AFD70, non-kind4 tail.
        const auto weapon=equipped_weapon_.model();
        const auto parent=frames_.object_parent(weapon);
        frames_.human_geometry_weapon_link(weapon,Frame{});
        try{
            auto desired=movement_.field6350;
            desired[0]=static_cast<float>(static_cast<long double>(movement_.field6350[0])*.9396926164627075f-
                static_cast<long double>(movement_.field6350[2])*.3420201539993286f);
            desired[2]=static_cast<float>(static_cast<long double>(movement_.field6350[0])*.3420201539993286f+
                static_cast<long double>(movement_.field6350[2])*.9396926164627075f);
            const auto square=(static_cast<long double>(desired[2])*desired[2]+
                static_cast<long double>(desired[1])*desired[1])+static_cast<long double>(desired[0])*desired[0];
            normalize_human_ai_shot(desired,square);
            correct_human_bone_direction(frames_,skeleton_,model_,9,desired,movement_.field6350,weights);
            correct_human_bone_direction(frames_,skeleton_,model_,8,desired,movement_.field6350,weights);
            const auto blend=human_animation_word_float(movement_.field631c);
            pose_equipment_weapon(movement_.field6350,true,true,blend);
            const auto right_hand=skeleton_.bones()[20].frame,left_hand=skeleton_.bones()[21].frame;
            refresh_weapon_hand_offsets();
            if(movement_.field62d8&0x80000000u){
                const auto point=frames_.actor_world_matrix(equipped_weapon_.left());
                const Vector target{{point[12],point[13],point[14],point[15]}},preferred{{0,-1,0,0}};
                pose_hand_chain(17,19,target,movement_.left_hand_offset6330,left_hand,blend,equipped_weapon_.butt()?&preferred:nullptr);
            }
            if(equipped_weapon_.butt()){
                const auto point=frames_.actor_world_matrix(equipped_weapon_.right());
                const Vector target{{point[12],point[13],point[14],point[15]}};
                pose_hand_chain(16,18,target,movement_.right_hand_offset6340,right_hand,1.f,nullptr);
            }
        }catch(...){
            const auto failure=std::current_exception();
            try{frames_.human_geometry_weapon_link(weapon,parent);}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
            std::rethrow_exception(failure);
        }
        frames_.human_geometry_weapon_link(weapon,parent);
    }
    void reset_loading() { //0079BD30(false), state written before any model-dependent work.
        const auto random=random_();if(random>0x7fff)throw std::runtime_error("Human geometry requires shared CRT random");
        seed_=random+1;
        look_={};
        prone_pose_.reset();
        saved_body_angles_.fill(0); //0079C18B..0079C1A1 clears eleven words at691C.
        geometry_effect_.reset();
        rest_surface_emitted_=false; //0079BD30 clears native6740.
        attachment_pose_.clear();
        secondary_pose_=Frame{}; //0079BD30 clears6630, scratch6638/6694 is reset by007AE320.
    }
    void bind_frames() {
        auto camera=frames_.actor_find(model_,"camera3RD",0xffff);
        if(camera)camera3rd_.emplace(frames_.retain_actor_frame(camera));
        camera=frames_.actor_find(model_,"cameraFPV",0xffff);
        if(camera)camera_fpv_.emplace(frames_.retain_actor_frame(camera));
        weapon1_=frames_.actor_find(model_,"hw1",0xffff);weapon2_=frames_.actor_find(model_,"hw2",0xffff);
        marker_z_=frames_.actor_find(model_,"z",0xffff);
        feet_={{skeleton_.bones()[21].frame,skeleton_.bones()[20].frame}};
    }
    void reset_movement() { //007B85C0(false).
        equipped_weapon_.clear();
        movement_={};
        for(auto* direction:{&movement_.direction6370,&movement_.direction6380,&movement_.direction6360}) {
            const auto world=frames_.actor_world_matrix(model_);
            *direction=actor_activation_direction(Vector{{world[8],world[9],world[10],world[11]}});
        }
        const auto world=frames_.actor_world_matrix(model_);
        movement_.position6400={{world[12],world[13],world[14],world[15]}};
        //007B85C0 repeats these two lookups after releasing its motion resource.
        weapon1_=frames_.actor_find(model_,"hw1",0xffff);weapon2_=frames_.actor_find(model_,"hw2",0xffff);
        equipped_weapon_.bind_hands(weapon1_,weapon2_);
        if(camera3rd_)movement_.offset6390={{0,.25f,0,0}};
    }
    Frames& frames_;Random& random_;
    HumanSkeleton<Frame> skeleton_;
    HumanLoadingJoints<Frames> joints_;
    Frame model_{};const void* human_=nullptr;const void* control_=nullptr;
    std::optional<Owner> camera3rd_,camera_fpv_;
    Frame weapon1_{},weapon2_{},marker_z_{};
    std::array<Frame,2> feet_{};
    std::uint32_t seed_=0,frame_elapsed_=0;
    std::function<void(std::int32_t&)> stop_voice_; //Borrowed control audio receiver, retained across bind/clear/repeated entry.
    HumanPhysicsGlobals* physics_globals_=nullptr; //Borrowed SAME startup/process owner as DD0 physical state.
    bool rest_surface_emitted_=false;
    bool geometry_detail_=true;
    Movement movement_;
    HumanWeaponBinding<Frames,Movement> equipped_weapon_{frames_,movement_};
    HumanAttachmentPose<Frames,Movement> attachment_pose_{frames_,skeleton_,movement_};
    OfflineFaces* offline_faces_=nullptr;bool offline_quality_=false;std::int32_t offline_camouflage_=0;
    std::unique_ptr<HumanFaceModel<Frames>> face_model_;
    HumanLookState look_;
    std::array<std::uint32_t,11> saved_body_angles_{};
    std::optional<GeometryEffect> geometry_effect_;
    Frame secondary_pose_{};
    std::uint32_t secondary_pose_time_=0;
    std::uint32_t equipment_overlay_time_=0;
    std::array<std::uint32_t,23> secondary_pose_current_{},secondary_pose_previous_{};
    HumanPronePose<Frames> prone_pose_;
    alignas(16)std::array<std::array<Vector,23>,6> fields661c_{},fields6620_{};
};
} //namespace hd2::game
#endif
