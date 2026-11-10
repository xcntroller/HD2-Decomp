#ifndef HD2_GAME_HUMAN_MODEL_GEOMETRY_H
#define HD2_GAME_HUMAN_MODEL_GEOMETRY_H

#include "game/human_loading_joints.h"
#include "game/human_look_state.h"
#include "game/human_weapon_binding.h"
#include "game/human_face_model.h"
#include <limits>
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
    using OfflineFaces=HumanFaceResources<Frames>;
    struct OfflineBinding {bool player;std::optional<float> clothing;};
    HumanModelGeometry(Frames& frames,Random& random)
        :frames_(frames),random_(random),joints_(frames,skeleton_) {reset_loading();}
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
        skeleton_.clear();
        equipped_weapon_.clear();
        face_model_.reset(); //0079BAC0 retires62B0 before retained camera frames62F8/FC.
        camera3rd_.reset();camera_fpv_.reset();model_=Frame{};human_=control_=nullptr;
        weapon1_=weapon2_=marker_z_=Frame{};feet_={};
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
        std::uint32_t field62cc=0,field62d0=0,field65f4=0,field62d8=1,
            field6304=0,field6308=0,field630c=300,field6310=0,field6314=0,field6318=0,field631c=0,field6410=100;
        std::int32_t field62dc=0;
        std::uint32_t time62c0=0,time62c4=0;
        float angle63c0=0,sine63c4=0;
        Vector direction6360{},direction6370{},direction6380{},offset6390{},
            position6400{},field6350{},field63d0{},field63e0{},field63f0{},
            offset63a0{{-.075f,-.041f,-.003f,0}},offset63b0{{.075f,-.041f,-.003f,0}};
    };
    const Movement& movement() const {return movement_;}
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
private:
    void reset_loading() { //0079BD30(false), state written before any model-dependent work.
        const auto random=random_();if(random>0x7fff)throw std::runtime_error("Human geometry requires shared CRT random");
        seed_=random+1;
        look_={};
        for(unsigned group=2;group!=5;++group)for(auto& value:directions_[group])value=Vector{{0,1,0,0}};
        directions_[4][0][0]=std::numeric_limits<float>::quiet_NaN(); //Native660C first word only.
        fields6614_={};
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
    std::uint32_t seed_=0;
    Movement movement_;
    HumanWeaponBinding<Frames,Movement> equipped_weapon_{frames_,movement_};
    OfflineFaces* offline_faces_=nullptr;bool offline_quality_=false;std::int32_t offline_camouflage_=0;
    std::unique_ptr<HumanFaceModel<Frames>> face_model_;
    HumanLookState look_;
    std::array<std::uint8_t,6> fields65f8_{};
    alignas(16)std::array<std::array<Vector,6>,6> directions_{};
    std::array<std::uint32_t,6> fields6614_{};
    alignas(16)std::array<std::array<Vector,23>,6> fields661c_{},fields6620_{};
};
} //namespace hd2::game
#endif
