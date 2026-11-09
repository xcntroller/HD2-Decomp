#ifndef HD2_GAME_MISSION_CAMERA_H
#define HD2_GAME_MISSION_CAMERA_H

#include "game/serialized_bytes.h"
#include "game/actor_activation_math.h"
#include "game/detached_camera.h"
#include "frontend/cleanup.h"
#include <algorithm>
#include <functional>
#include <memory>
#include <optional>
#include <utility>

namespace hd2::game {

//00464EF0's eleven32-byte configurations,00465A00 and004656D0. Preserve
//uninterpreted DWORDs; consumers, rather than serialization, assign their meaning.
class MissionCameraSettings {
public:
    using Row=std::array<std::uint32_t,8>;
    MissionCameraSettings() {
        rows_.fill(Row{{0x40400000u,0x3f000000u,0x3f800000u,0x3e4ccccdu,
                        0x8bu,0x3f000000u,0xbf000000u,0x41400000u}});
    }
    const std::array<Row,11>& rows() const {return rows_;}
    std::uint32_t field2ac() const {return field2ac_;}
    void reset_outdoor_distance(){rows_[1][0]=field2ac_;} //0046C370 writes actual configuration1 distance.
    bool read(SerializedBytes bytes) {
        field2ac_=0x40400000u;
        auto header=next_mission_chunk(bytes);
        if(!header)throw std::runtime_error("Camera configuration has no version chunk");
        const auto version=header->bytes.word(); //Neither root ID is tested.
        bool copied=false;
        if(version==2) {
            auto data=next_mission_chunk(bytes);
            if(!data)throw std::runtime_error("Camera configuration has no data chunk");
            //Original retries352,320,...,32 bytes, leaving all later rows intact.
            const auto count=std::min<std::size_t>(rows_.size(),data->bytes.remaining()/32);
            for(std::size_t row=0;row!=count;++row)
                for(auto& word:rows_[row])word=data->bytes.word();
            copied=count!=0;
        }
        //An opened wrong-version/short configuration still uses native+100.
        field2ac_=rows_[1][0];return copied;
    }
    template<class Files>bool load(Files& files) {
        field2ac_=0x40400000u; //Missing file retains previous rows, with default distance3.
        const auto handle=files.open("tables\\gamecam.set",0);
        if(handle==0xffffffffu)return false;
        struct Close {Files& files;std::uint32_t handle;~Close(){files.close(handle);}} close{files,handle};
        const auto size=files.seek(handle,0,2);
        if(size==0xffffffffu||size>16u*1024u*1024u)throw std::runtime_error("Invalid camera configuration size");
        if(files.seek(handle,0,0)!=0)throw std::runtime_error("Cannot rewind camera configuration");
        std::vector<unsigned char> bytes(size);
        if(size&&files.read(handle,bytes.data(),size)!=size)throw std::runtime_error("Cannot read camera configuration");
        return read(SerializedBytes(bytes));
    }
private:
    std::array<Row,11> rows_;
    std::uint32_t field2ac_=0x40400000u;
};

struct MissionCameraState {
    using Vector=std::array<float,4>;
    Vector position{},direction{{0,-1,1,0}},previous_position{},previous_direction{}; //Native10/20/30/40.
    std::array<Vector,4> vectors{}; //Native60/70/80/90; fourth lanes are not reset.
    float fieldd0=0,field254=0,field258=0,field288=0,field28c=0;
    std::uint32_t field24c=0,field250=0,field260=1,field268=0,field280=0,field284=0;
    std::array<std::uint32_t,3> fields298{};
    bool flag26c=false,flag26d=false,flag26e=false,flag26f=true,flag270=false,flag278=false,flag290=false;
    bool flag2b0=false;std::uint32_t field2b4=0;
    std::uint32_t field25c=0,field2a4=0,field2a8=0,field294=0;
};

//Loaded portion of00465160 and matching00465610 camera lifetime. The containing
//scene must outlive this owner and keep the saved scene camera alive until clear.
//00462650's detached camera is a real separately-created kind3 owner; its later
//interactive controller is not entered by loading. Ordinary unattached Human
//target loading/retirement is supplied;00469300 camera simulation remains required.
template<class Services,class Human>
class MissionCamera {
public:
    using Frame=typename Services::Frame;
    using Owner=decltype(std::declval<Services&>().create_game_camera());
    explicit MissionCamera(Services& services) : services_(services),detached_(services) {}
    ~MissionCamera() {try{clear();}catch(...){services_.record_actor_cleanup_failure(std::current_exception());}}
    MissionCamera(const MissionCamera&)=delete;
    MissionCamera& operator=(const MissionCamera&)=delete;
    Frame frame() const {return camera_?camera_->get():Frame{};}
    Frame detached_frame() const {return detached_.frame();}
    auto& detached(){return detached_;} //004650A0, SAME owned controller, not only its frame.
    //004650A0 virtual88 returns the SAME detached controller;00462750 writes
    //its40 speed only.00462650 initializes1, independent of camera simulation.
    void set_detached_speed(float value){detached_.speed(value);}
    float detached_speed()const{return detached_.speed();}
    template<class Inline>void remove_detached_human(Human& human,Inline* control) {
        detached_.remove(human,[&](Human& value,float amount){set_human_camera_amount(value,amount,control);});
    }
    bool hud_suppressed()const{return state_.field2b4!=0;} //00465080, camera virtual80; not detached-camera state.
    Frame saved_scene_camera() const {return previous_;}
    Frame target_frame() const {return target_?target_->get():Frame{};}
    const MissionCameraState& state() const {return state_;}
    const MissionCameraSettings& settings() const {return settings_;}
    void set_direction(const MissionCameraState::Vector& direction) { //00465EA0, SAME selected264 configuration.
        if(!(settings_.rows()[configuration_][4]&8u)&&state_.field260!=10)state_.direction=direction;
    }
    void set_zoom_control(bool enabled) { //0046CB20 command7; boolean is param3 presence, not param4.
        state_.field294=0;state_.flag290=enabled;
    }
    bool set_fov(float value) { //00465E50, virtual50.
        const auto camera=frame();if(!camera)return false;
        services_.game_camera_fov(camera,value);services_.update_scene_object(camera);return true;
    }
    bool set_parameter78(float value) { //00465E00, virtual5C.
        const auto camera=frame();if(!camera)return false;
        services_.game_camera_parameter78(camera,value);services_.update_scene_object(camera);return true;
    }
    bool query_vector(std::uint32_t query,MissionCameraState::Vector& output) { //00465A10, virtual64.
        if(query==2){output=state_.previous_position;return true;} //Native30, saved POSITION.
        if(query==3){output=state_.position;return true;} //Native10.
        Frame camera{};
        if(query==0)camera=services_.active_game_camera();
        else if(query==1)camera=frame();else return false;
        if(!camera)return false; //Original leaves output untouched on failure.
        const auto matrix=services_.actor_world_matrix(camera);
        output={{matrix[12],matrix[13],matrix[14],matrix[15]}};return true;
    }
    bool query_direction(std::uint32_t query,MissionCameraState::Vector& output) { //00465B40, virtual68.
        if(query==2){output=state_.previous_direction;return true;} //Native40.
        if(query==3){output=state_.direction;return true;} //Native20, player handover query.
        Frame camera{};
        if(query==0)camera=services_.active_game_camera();
        else if(query==1)camera=frame();else return false;
        if(!camera)return false;
        const auto matrix=services_.actor_world_matrix(camera);
        MissionCameraState::Vector result{{matrix[8],matrix[9],matrix[10],matrix[11]}};
        const auto x=static_cast<long double>(result[0]),y=static_cast<long double>(result[1]),z=static_cast<long double>(result[2]);
        const auto length=(y*y+z*z)+x*x;const float stored=static_cast<float>(length);
        constexpr float epsilon=9.99999993922529e-09f;
        if(!(std::abs(length-1)<epsilon)) {
            if(!(stored<epsilon)) { //Native unordered comparison follows the normalization branch.
                const auto scale=1/std::sqrt(static_cast<long double>(stored));
                for(unsigned axis=0;axis!=3;++axis)result[axis]=static_cast<float>(result[axis]*scale);
            }else {const auto axis=result[0]!=0?0u:result[2]!=0?2u:1u;result[axis]=result[axis]<0?-1.f:1.f;}
        }
        output=result;return true; //Fourth lane is copied untouched.
    }
    void select_loading_human_target(Human& human,std::optional<Frame> control_sector,
            bool first_person,bool zoom,std::function<void(Frame)> retire) { //0046C370 ordinary unattached Human.
        if(state_.flag26d||!frame())return;
        if(human.field160)throw std::runtime_error("Attached camera target requires actual Car/weapon camera receivers");
        if(!retire)throw std::runtime_error("Camera target requires actual Human amount retirement");
        const auto model=human.base().frame();
        if(!model)throw std::runtime_error("Camera target requires an actual Human model");
        Frame next{};
        if(first_person||zoom) {
            next=services_.actor_find(model,"cameraFPV*",0x10020);state_.field260=2;
        }else {
            next=services_.actor_find(model,"camera3RD*",0x10020);
            Frame sector{};
            if(control_sector)sector=*control_sector; //005AC4C0, SAME CD0+14 sector, including null.
            else {
                const auto world=services_.actor_world_matrix(model);
                sector=services_.human_scene_sector({{world[12],world[13],world[14],world[15]}});
            }
            const auto properties=sector?services_.camera_sector_properties(sector):std::string{}; //Frame+F0, not its name.
            const bool outdoor=sector==services_.camera_root_sector()||properties.find("nointerier")!=std::string::npos||
                properties.find("NOINTERIER")!=std::string::npos;
            const bool close=properties.find("interierclose")!=std::string::npos||properties.find("INTERIERCLOSE")!=std::string::npos;
            state_.field280=0xffffffffu;
            if(state_.flag270)state_.field260=outdoor?5:6;
            else if(outdoor){state_.field260=1;settings_.reset_outdoor_distance();}
            else state_.field260=close?4:3;
        }
        configuration_=state_.field260;
        if(!next)next=model; //Original missing marker fallback to the SAME Human model.
        const auto old=target_frame();
        if(old!=next) {
            state_.field2a4=state_.field2a8=0;
            if(old) {
                if(!retire_target_)throw std::logic_error("Camera target lost its actual retirement receiver");
                retire_target_(old);target_.reset(); //005A9760(0) before old release.
            }
            retire_target_=std::move(retire);
            auto reference=services_.retain_actor_frame(next);
            if(!reference.get())throw std::runtime_error("Cannot retain actual camera target");
            target_.emplace(std::move(reference));
            const auto matrix=services_.actor_world_matrix(next);
            //Original(Y²+Z²)+X² is identical to this helper's commuting first
            //two terms. Keep extended unit comparison and stored-float sqrt.
            state_.direction=actor_activation_direction({{matrix[8],matrix[9],matrix[10],matrix[11]}});
            const auto distance=real(settings_.rows()[configuration_][0]);
            std::array<float,3> offset{};
            for(unsigned axis=0;axis!=3;++axis)offset[axis]=static_cast<float>(static_cast<long double>(state_.direction[axis])*distance);
            const auto world=services_.actor_world_matrix(next);
            for(unsigned axis=0;axis!=3;++axis) {
                const auto value=static_cast<float>(static_cast<long double>(world[12+axis])-offset[axis]);
                if(!std::isfinite(value))throw std::runtime_error("Non-finite camera target position");
                state_.position[axis]=state_.previous_position[axis]=value;
            }
            //Original copies an unwritten fourth stack word here. Preserve
            //the known loaded-camera lane instead of inventing that value.
            state_.previous_position[3]=state_.position[3];
            state_.field254=distance;state_.field25c=0;state_.flag26c=true;
        }
        if(state_.field280&0x80000000u) {
            state_.field280=0;state_.field254=real(settings_.rows()[configuration_][2]);
        }
    }
    template<class Files>void activate(bool authority,Files& files,float fov,float near_clip,float far_clip) {
        if(authority)return;
        if(camera_||previous_)throw std::logic_error("Game camera is already activated");
        state_.flag2b0=false;state_.field2b4=0;state_.field258=6;state_.flag26c=false;
        settings_.load(files);
        try {
            camera_.emplace(services_.create_game_camera());
            previous_=services_.active_game_camera();
            if(!camera_->get()||!previous_)throw std::runtime_error("Game camera or loaded scene camera is unavailable");
            auto camera=camera_->get();
            services_.game_camera_fov(camera,fov);
            services_.game_camera_range(camera,near_clip,far_clip);
            services_.active_game_camera(camera);
            const auto position=services_.actor_position(previous_);
            services_.object_position(camera,{{position[0],position[1],position[2]}});
            services_.object_rotation(camera,services_.actor_rotation(previous_));
            services_.game_camera_name(camera,"GameCamera");
            services_.game_camera_parameter78(camera,real(0x3f91361eu));
            services_.game_camera_parameter80(camera,.01f,10.f);
            services_.update_scene_object(camera);
            const auto matrix=services_.actor_world_matrix(camera);
            //0046536F..95 adds Y²+Z²+X²: the same sum as the shared normalizer's
            //Z²+Y²+X² (the first two extended terms commute). Preserve lane4.
            state_.direction=actor_activation_direction({{matrix[8],matrix[9],matrix[10],matrix[11]}});
            state_.previous_direction=state_.direction;
            const auto world=services_.actor_world_matrix(camera);
            state_.position={{world[12],world[13],world[14],world[15]}};
            state_.previous_position=state_.position;
            reset_loaded_state();
        } catch(...) {
            frontend::CleanupErrors errors(std::current_exception());errors.attempt([&]{clear();});errors.rethrow();
        }
    }
    void deactivate(bool authority) {if(!authority)clear();}
    void clear() {
        frontend::CleanupErrors errors;
        if(previous_) {
            const auto previous=std::exchange(previous_,Frame{});
            errors.attempt([&]{services_.active_game_camera(previous);});
        }
        camera_.reset(); //00465610 releases240 before retiring/releasing248.
        if(target_) {
            const auto target=target_->get();
            errors.attempt([&] {
                if(!retire_target_)throw std::logic_error("Camera target lost its actual retirement receiver");
                retire_target_(target);
            });
            target_.reset();
        }
        retire_target_={};errors.rethrow();
    }
private:
    static float real(std::uint32_t bits) {float value;std::memcpy(&value,&bits,4);return value;}
    void reset_loaded_state() {
        state_.flag26d=false;state_.field268=5;state_.flag26e=false;state_.flag26f=true;
        state_.flag270=false;state_.flag278=false;state_.field280=0;state_.field284=0;
        for(auto& value:state_.vectors)for(unsigned i=0;i!=3;++i)value[i]=0;
        state_.field288=0;state_.field28c=-1;state_.field24c=0;state_.field250=0;
        state_.flag290=false;state_.fields298.fill(0);state_.fieldd0=0;
    }
    Services& services_;
    MissionCameraSettings settings_;
    MissionCameraState state_;
    DetachedCamera<Services,Human> detached_; //SAME00462650/004626E0 controller, inactive during loading.
    std::optional<Owner> camera_;
    Frame previous_{}; //Original244 borrows; scene owns it during this lifetime.
    std::optional<Owner> target_; //Original248 owns one additional frame reference.
    std::function<void(Frame)> retire_target_; //Required real model-owner/type3→005A9760(0) receiver; containing world outlives camera.
    std::size_t configuration_=1; //Original264 points at actual settings row, initially1.
};

} //namespace hd2::game
#endif
