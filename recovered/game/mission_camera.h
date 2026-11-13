#ifndef HD2_GAME_MISSION_CAMERA_H
#define HD2_GAME_MISSION_CAMERA_H

#include "game/serialized_bytes.h"
#include "game/actor_activation_math.h"
#include "game/detached_camera.h"
#include "game/human_camera_state.h"
#include "frontend/cleanup.h"
#include <algorithm>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <string>
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
    Row& row(std::size_t index){return rows_.at(index);}
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
    Vector position{},direction{{0,-1,1,0}},previous_position{},previous_direction{},target_offset{}; //Native10/20/30/40.
    std::array<Vector,4> vectors{}; //Native60/70/80/90; fourth lanes are not reset.
    float fieldd0=0,field254=0,field258=0,field27c=0,field28c=0,field2a4=0,field2a8=0;
    std::uint32_t field250=0,field260=1,field268=0,field280=0,field284=0;
    std::optional<std::int32_t> spectator_index274; //Unwritten until0046CB20 command2 publishes selection.
    std::array<std::uint32_t,3> fields298{};
    bool flag26c=false,flag26d=false,flag26e=false,flag26f=true,flag270=false,flag278=false,flag290=false;
    bool flag2b0=false;std::uint32_t field2b4=0;
    std::uint32_t field294=0;
};

//0046CB20 commands0/2 used by ordinary spawn and0069AA50. The scene owns
//spectator frames; keep only the original index, with no new camera owner.
template<class Services,class Own,class Refresh,class Random>
void apply_spawn_camera_command(MissionCameraState& state,Services& services,Own&& own,
        std::uint32_t command,bool parameter_present,std::uint32_t parameter,std::uint32_t mode,
        Refresh&& refresh,Random&& random){
    if(command==0){
        state.flag26d=parameter_present?parameter!=0:!state.flag26d;
        if(state.flag26d)if(const auto active=services.active_game_camera()){
            const auto matrix=services.actor_world_matrix(active);
            state.position={{matrix[12],matrix[13],matrix[14],matrix[15]}};
        }
        if(state.flag26e){
            if(!state.flag26d){services.active_game_camera(own());state.flag26e=false;}
            else if(const auto spectator=services.game_scene_find("spectator",4))services.active_game_camera(spectator);
        }
        if(!state.flag26d)refresh();
        return;
    }
    if(command!=2)throw std::runtime_error("Camera command is outside recovered spawn dispatch");
    if((mode&7u)==7u)return; //Native offline command2 does not touch flags or draw random.
    state.flag26e=parameter_present?parameter!=0:!state.flag26e;
    if(!state.flag26e){services.active_game_camera(own());return;}
    std::int32_t count=0;
    while(services.game_scene_find("spectator"+std::to_string(count),4)){
        if(count==std::numeric_limits<std::int32_t>::max())throw std::length_error("Spectator count exceeds native signed index");
        ++count;
    }
    const auto word=static_cast<std::uint32_t>(random());std::int32_t draw;std::memcpy(&draw,&word,4);
    const std::uint32_t bits=0x38000100;float multiplier;std::memcpy(&multiplier,&bits,4);
    //FILD rand -> FMUL FLOAT0080FF64 -> FIMUL count -> store DOUBLE -> floor.
    //A FLOAT product would round RAND_MAX to1 and select past the last frame.
    const double product=static_cast<double>(static_cast<long double>(draw)*multiplier*count);
    const auto rounded=std::floor(product);
    if(!std::isfinite(rounded)||rounded<std::numeric_limits<std::int32_t>::min()||rounded>std::numeric_limits<std::int32_t>::max())
        throw std::runtime_error("Spectator random selection exceeds native signed index");
    state.spectator_index274=static_cast<std::int32_t>(rounded);
    if(const auto spectator=services.game_scene_find("spectator"+std::to_string(*state.spectator_index274),4))
        services.active_game_camera(spectator);
}

//0046CB20 command1 over SAME camera state/frame. Engine matrix is required.
template<class Services,class Frame>void apply_camera_blast_impulse(MissionCameraState& state,Services& services,Frame frame,
        const MissionCameraState::Vector& centre,std::uint32_t range,std::uint32_t mode){
        if(!(mode&2u))return;
        float distance_limit=static_cast<float>(range); //Native unsigned DWORD via zero-extended QWORD/FILD.
        if(state.field2b4)distance_limit=static_cast<float>(static_cast<long double>(distance_limit)*2.5f);
        if(!frame)throw std::logic_error("Blast camera impulse requires actual loaded camera240");
        const auto matrix=services.actor_world_matrix(frame);
        const auto dx=static_cast<long double>(centre[0])-matrix[12],dy=static_cast<long double>(centre[1])-matrix[13],dz=static_cast<long double>(centre[2])-matrix[14];
        const float square=static_cast<float>((dy*dy+dx*dx)+dz*dz); //0046D02C..3C.
        if(!std::isfinite(square))throw std::runtime_error("Non-finite blast camera distance");
        std::int32_t bits;std::memcpy(&bits,&square,4);
        const auto distance=bits<0x322bcc77?0.L:std::abs(static_cast<long double>(square)-1.f)<9.99999993922529e-09f?1.L:std::sqrt(static_cast<long double>(square));
        if(distance<distance_limit)state.field284=2000;
}

//Loaded portion of00465160 and matching00465610 camera lifetime. The containing
//scene must outlive this owner and keep the saved scene camera alive until clear.
//00462650's detached camera is a real separately-created kind3 owner; its later
//interactive controller is not entered by loading. Ordinary unattached Human
//target loading/retirement and ordinary00469300 simulation are supplied. The
//containing world must bind actual loaded actors/inline owners before updates.
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
    bool sound_special_mode()const{return state_.flag270;} //00465070, virtual7C; actual byte270, distinct from26D/26E/2B4.
    Frame saved_scene_camera() const {return previous_;}
    bool owns_frame(Frame candidate)const{return frame()==candidate;} //00465EE0 virtual4C, compares even null.
    template<class Control>void request_distance_hold(std::uint32_t duration,std::uint32_t mode,Control& players){ //00465AD0, virtual70.
        apply_human_camera_distance_hold(state_.field280,duration,mode,players);
    }
    Frame look_target()const{return look_target_;}
    void look_target(Frame target,std::int32_t angle) { //00465B20 virtual74, borrowed288, radians28C.
        look_target_=target;state_.field28c=static_cast<float>(static_cast<long double>(angle)*.01745329238474369f); //0080F6E8.
    }
    Frame target_frame() const {return target_?target_->get():Frame{};}
    const MissionCameraState& state() const {return state_;}
    const MissionCameraSettings& settings() const {return settings_;}
    void set_direction(const MissionCameraState::Vector& direction) { //00465EA0, SAME selected264 configuration.
        if(!(settings_.rows()[configuration_][4]&8u)&&state_.field260!=10)state_.direction=direction;
    }
    void set_zoom_control(bool enabled) { //0046CB20 command7; boolean is param3 presence, not param4.
        state_.field294=0;state_.flag290=enabled;
    }
    void blast_impulse(const MissionCameraState::Vector& centre,std::uint32_t range,std::uint32_t mode){ //0046CB20 command1.
        apply_camera_blast_impulse(state_,services_,frame(),centre,range,mode);
    }
    template<class Refresh>void command_offline(std::uint32_t command,bool parameter_present,std::uint32_t parameter,
                                               std::uint32_t mode,Refresh& refresh) { //0046CB20 commands0/2, loading/spawn path.
        if((mode&7u)!=7u)throw std::runtime_error("Camera command requires actual nonoffline spectator receivers");
        command_spawn(command,parameter_present,parameter,mode,refresh,[]()->std::uint32_t{throw std::logic_error("Offline camera attempted a nonoffline random producer");});
    }
    template<class Refresh,class Random>void command_spawn(std::uint32_t command,bool parameter_present,std::uint32_t parameter,
            std::uint32_t mode,Refresh& refresh,Random&& random){
        apply_spawn_camera_command(state_,services_,[this]{return frame();},command,parameter_present,parameter,mode,
            [&]{refresh(*this);},std::forward<Random>(random));
    }
    float fov(){const auto camera=frame();if(!camera)throw std::logic_error("FPV interpolation requires actual mission camera240");return services_.game_camera_fov_value(camera);}
    float parameter78(){const auto camera=frame();if(!camera)throw std::logic_error("FPV interpolation requires actual mission camera240");return services_.game_camera_parameter78_value(camera);}
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
            state_.field254=distance;previous_hit_=nullptr;state_.flag26c=true;
        }
        if(state_.field280&0x80000000u) {
            state_.field280=0;state_.field254=real(settings_.rows()[configuration_][2]);
        }
    }
    template<class Control,class Sector,class Retire>
    void refresh_current_loading_target(Control& control,bool first_person,bool zoom,Sector&& sector,Retire&& retire) {
        if(state_.flag26d||!frame())return;
        if(auto* human=control.current_player()) {
            select_loading_human_target(*human,sector(*human),first_person,zoom,std::forward<Retire>(retire));return;
        }
        //0046C370 no-current-Human branch retains248/264. It still processes
        //the final signed280 distance sentinel; it does not invent a target.
        if(state_.field280&0x80000000u){state_.field280=0;state_.field254=real(settings_.rows()[configuration_][2]);}
    }
    //00468130 ordinary Human follow. Queries are CameraCollisionQueries over
    //the SAME loaded tree/buckets; Inline is the actual current Human's owner.
    template<class Queries,class Inline,class FirstPerson>
    void follow(std::uint32_t elapsed,Human* current,Inline* control,Queries& queries,FirstPerson& first_person,std::uint32_t mode) {
        if(!frame()||!target_frame())return;
        if(state_.field260==8)throw std::runtime_error("Camera configuration8 requires actual attached target producer");
        auto& row=settings_.row(configuration_);
        const auto saved_smoothing=row[7],saved_distance=row[0];
        if(state_.flag26c)row[7]=bits(1000.f);
        if(signed_word(state_.field280)>0)row[0]=0;
        struct Restore {MissionCameraState& state;MissionCameraSettings::Row& row;std::uint32_t smoothing,distance;
            ~Restore(){row[7]=smoothing;row[0]=distance;state.flag26c=false;}} restore{state_,row,saved_smoothing,saved_distance};
        auto planar=state_.direction;planar[1]=0;
        const float angle=services_.car_vector_angle(planar,{{0,0,1,0}});
        const bool turn=std::abs(angle)<.07999999821186066; //0080FF20 DOUBLE.
        auto rotate=[&](auto& vector,float amount) {
            const auto matrix=services_.human_axis_rotation({{0,1,0,0}},amount);
            services_.car_transform_in_place(vector,matrix);
        };
        if(turn){rotate(state_.direction,.5f);rotate(state_.previous_direction,.5f);}
        state_.previous_direction=camera_normalize(state_.previous_direction,2);
        state_.direction=camera_normalize(state_.direction,2);
        const float separation=services_.car_vector_angle(state_.previous_direction,state_.direction);
        if(state_.field260!=7)state_.field258=16;
        const auto speed=state_.field258;state_.field258=6;
        float factor=separation==0?0.f:static_cast<float>(static_cast<long double>(static_cast<float>(elapsed))*speed*0x1.9bc65ep-9f/separation);
        if(factor>1)factor=1;
        auto direction=services_.camera_interpolate_direction(state_.previous_direction,state_.direction,factor);
        if(turn){rotate(state_.direction,-.5f);rotate(state_.previous_direction,-.5f);rotate(direction,-.5f);}
        direction=camera_normalize(direction,0);
        const auto target=services_.actor_world_matrix(target_frame());
        const auto radius=real(row[3]);
        if(state_.field260!=2) {
            state_.target_offset[1]=0;
            if(state_.flag270) {
                const auto height=state_.field27c;
                if(height<=target[13]) {
                    const auto bound=static_cast<long double>(radius)+height+.10000000149011612f;
                    if(target[13]<bound)state_.target_offset[1]=static_cast<float>(static_cast<long double>(radius)+height+((row[4]&0x80u)?.30000001192092896f:.10000000149011612f)-target[13]);
                }else {
                    const auto bound=(static_cast<long double>(height)-.10000000149011612f)-radius;
                    if(bound<target[13])state_.target_offset[1]=static_cast<float>(((static_cast<long double>(height)-((row[4]&0x80u)?.30000001192092896f:.10000000149011612f))-radius)-target[13]);
                }
            }
        }
        auto place=[&](float distance) {
            for(unsigned axis=0;axis!=3;++axis) {
                const float offset=static_cast<float>(static_cast<long double>(state_.previous_direction[axis])*distance);
                state_.previous_position[axis]=static_cast<float>((static_cast<long double>(target[12+axis])+state_.target_offset[axis])-offset);
            }
        };
        if(!(row[4]&0x80u)) {state_.previous_direction=direction;place(real(row[0]));}
        else {
            if(current&&current->field160)throw std::runtime_error("Camera collision requires actual attached Car classification");
            MissionCameraState::Vector start{},displacement{};
            const auto desired=real(row[0]);
            const float length=static_cast<float>(static_cast<long double>(desired)+1);
            for(unsigned a=0;a!=3;++a){start[a]=static_cast<float>(static_cast<long double>(target[12+a])+state_.target_offset[a]);displacement[a]=static_cast<float>(-static_cast<long double>(direction[a])*length);}
            if(current&&control)control->exclude_query(false);
            struct Exclusion {Inline* control;~Exclusion(){if(control)control->restore_query(false);}} exclusion{current?control:nullptr};
            auto hit=queries.sweep(start,displacement,radius,0x98000u,0x200000u);
            if(!hit) {
                hit=queries.ray(start,displacement,0x98000u,0x200000u);
                if(hit)for(unsigned a=0;a!=3;++a)hit->position[a]=static_cast<float>(static_cast<long double>(hit->normal[a])*(static_cast<long double>(radius)+.05000000074505806f)+hit->position[a]);
            }
            if(hit&&(hit->flags&0x40u)&&hit->frame) {
                const auto prefix=services_.frame_name(hit->frame),name=services_.frame_name(target_frame());
                if(name.compare(0,prefix.size(),prefix)==0)hit.reset(); //Original strncmp, empty prefix matches.
            }
            const auto delta=static_cast<long double>(static_cast<float>(elapsed))*real(row[7])*1.f*0x1.0624dep-10f;
            if(!hit) {
                if(state_.field254<=desired) {
                    state_.field254=static_cast<float>(static_cast<long double>(state_.field254)+delta);
                    if(desired<state_.field254)state_.field254=desired;
                }else state_.field254=std::max(0.f,static_cast<float>(static_cast<long double>(state_.field254)-delta));
                previous_hit_=nullptr;state_.previous_direction=direction;
            }else {
                std::array<long double,3> difference{};
                for(unsigned a=0;a!=3;++a)difference[a]=static_cast<long double>(start[a])-hit->position[a];
                const auto square=(difference[0]*difference[0]+difference[1]*difference[1])+difference[2]*difference[2];
                float distance=camera_length(square);
                if(desired<distance)distance=desired;
                if(state_.flag270)distance=static_cast<float>(static_cast<long double>(distance)+(state_.field27c<=target[13]?1.f:-1.f));
                if(distance<static_cast<long double>(radius)+.05000000074505806f)distance=0;
                if(!state_.flag270&&state_.field254==real(row[1])) {
                    state_.previous_direction=direction;previous_hit_=hit->identity;
                }else if(!(distance<=state_.field254)) { //Native unordered/greater branch.
                    state_.previous_direction=direction;previous_hit_=hit->identity;
                    state_.field254=static_cast<float>(static_cast<long double>(state_.field254)+delta);
                    if(distance<state_.field254)state_.field254=distance;
                    if(desired<state_.field254)state_.field254=desired;
                }else if(previous_hit_) {
                    state_.previous_direction=direction;state_.field254=distance;
                }else {
                    state_.field254=static_cast<float>(static_cast<long double>(state_.field254)-delta);
                    const float minimum=static_cast<float>(static_cast<long double>(distance)-.009999999776482582f);
                    if(state_.field254<minimum)state_.field254=minimum;
                    if(!state_.flag270&&state_.field254<real(row[1]))previous_hit_=hit->identity;
                }
            }
            //Native restores exclusion BEFORE first-person/position work.
            if(exclusion.control){exclusion.control->restore_query(false);exclusion.control=nullptr;}
            if(!state_.flag278)place(state_.field254);
            else {
                if(real(row[2])<=state_.field254&&(first_person.enabled()||first_person.secondary_enabled())) {
                    state_.flag278=false;first_person.enable(mode,false);
                }
                for(unsigned a=0;a!=3;++a)state_.previous_position[a]=static_cast<float>(static_cast<long double>(target[12+a])+state_.target_offset[a]);
            }
        }
        //Original signed280 wraps native subtraction/addition, clamping only
        //after the sign-specific operation. Config distance is restored above.
        if(signed_word(state_.field280)>0) {
            state_.field280-=elapsed;if(signed_word(state_.field280)<0)state_.field280=0;
        }
        if(signed_word(state_.field280)<0) {
            state_.field280+=elapsed;if(signed_word(state_.field280)>0)state_.field280=0;
        }
    }
    //00466970 ordinary, unattached Human pose/shake/breathing, script look and opacity.
    //target_human is resolved from target248's actual model100/Scene2 binding;
    //breathing is the containing process's SAME FLOAT00839268 (initial .005).
    template<class Inline,class FirstPerson>
    void apply_follow_pose(std::uint32_t elapsed,bool game64,Human* current,Human* target_human,
                           Inline* target_inline,FirstPerson& first_person,std::uint32_t mode,
                           std::int32_t local_player_id,float& breathing) {
        if(!frame()||game64)return;
        if(state_.flag26d||state_.field260==8)
            throw std::runtime_error("Camera pose requires actual debug/attached receivers");
        if(signed_word(state_.field284)>0) {
            state_.field284-=elapsed;
            if(signed_word(state_.field284)<0) {
                state_.field284=0;for(unsigned n=0;n!=2;++n)for(unsigned a=0;a!=3;++a)state_.vectors[n][a]=0;
            }else {
                const auto t=static_cast<long double>(signed_word(state_.field284));
                const auto x=t*0x1.f75104p-15f*std::sin(0x1.1eb852p-4f*t);
                const auto y=t*0x1.4f8b58p-15f*std::sin(t*0x1.47ae14p-4f);
                const auto z=t*0x1.2599eep-14f*std::sin(t*.05000000074505806f);
                state_.vectors[0][0]=static_cast<float>(x);state_.vectors[0][1]=static_cast<float>(y);state_.vectors[0][2]=static_cast<float>(z);
                state_.vectors[1][0]=static_cast<float>(z*.15000000596046448f);
                state_.vectors[1][1]=static_cast<float>(y*-.25f);state_.vectors[1][2]=static_cast<float>(x*.15000000596046448f);
            }
        }
        auto transformed_offset=[&](MissionCameraState::Vector value) {
            const auto matrix=services_.actor_world_matrix(frame());services_.car_transform_in_place(value,matrix);
            const auto updated=services_.actor_world_matrix(frame());
            for(unsigned a=0;a!=3;++a)value[a]=static_cast<float>(static_cast<long double>(value[a])-updated[12+a]);
            return value;
        };
        const auto positional=transformed_offset(state_.vectors[3]);
        std::array<float,3> position{};
        position[0]=static_cast<float>(static_cast<long double>(state_.previous_position[0])+state_.vectors[0][0]+positional[0]);
        position[1]=static_cast<float>(static_cast<long double>(positional[1])+state_.vectors[0][1]+state_.previous_position[1]);
        const float z=static_cast<float>(static_cast<long double>(state_.vectors[0][2])+state_.previous_position[2]);
        position[2]=static_cast<float>(static_cast<long double>(positional[2])+z);
        services_.object_position(frame(),position);
        if(look_target_)apply_look_target(elapsed);
        else {
        MissionCameraState::Vector direction{};
        if(!state_.flag290) {
            const auto offset=transformed_offset(state_.vectors[2]);
            direction[0]=static_cast<float>(static_cast<long double>(state_.previous_direction[0])+state_.vectors[1][0]+offset[0]);
            direction[1]=static_cast<float>(static_cast<long double>(offset[1])+state_.vectors[1][1]+state_.previous_direction[1]);
            const float last=static_cast<float>(static_cast<long double>(state_.vectors[1][2])+state_.previous_direction[2]);
            direction[2]=static_cast<float>(static_cast<long double>(offset[2])+last);
        }else {
            for(unsigned a=0;a!=3;++a)direction[a]=static_cast<float>(static_cast<long double>(state_.previous_direction[a])+state_.vectors[1][a]+state_.vectors[2][a]);
            auto cross=[](const auto& a,const auto& b) {
                MissionCameraState::Vector result{};
                for(unsigned n=0;n!=3;++n)result[n]=static_cast<float>(static_cast<long double>(a[(n+1)%3])*b[(n+2)%3]-static_cast<long double>(a[(n+2)%3])*b[(n+1)%3]);
                return result;
            };
            const auto side=cross(direction,MissionCameraState::Vector{{0,1,0,0}});
            auto vertical=camera_normalize(cross(side,direction),0),horizontal=camera_normalize(side,0);
            long double magnitude=0x1.89374cp-8f;
            if(current) {
                if(human_camera_action(*current,mode,local_player_id)==2)breathing=.0020000000949949026f;
                else if(!human_camera_focus(*current)) {
                    if(breathing<.00800000037997961f) {
                        const auto value=static_cast<long double>(signed_word(elapsed))*4.000000189989805e-06+breathing; //0080FEE0 DOUBLE, retained after FST.
                        breathing=static_cast<float>(value);
                        if(value>.00800000037997961f)breathing=.00800000037997961f;
                    }
                }else if(.001500000013038516f<breathing) {
                    const auto value=static_cast<long double>(breathing)-static_cast<long double>(signed_word(elapsed))*5.000000237487257e-06; //0080FEF0 DOUBLE.
                    breathing=static_cast<float>(value);
                    if(value<.001500000013038516f)breathing=.001500000013038516f;
                }
                magnitude=(2.f-human_loading_stat(*current,4))*breathing;
            }
            const auto time=static_cast<long double>(signed_word(state_.fields298[0]));
            const auto vertical_amount=magnitude*std::sin(std::cos(0x1.d7dbf4p-10f*time));
            const auto horizontal_amount=std::cos(std::sin(time*0x1.a36e2ep-11f+3.f)+1.0)*magnitude;
            for(unsigned a=0;a!=3;++a) {
                vertical[a]=static_cast<float>(vertical[a]*vertical_amount);horizontal[a]=static_cast<float>(horizontal[a]*horizontal_amount);
                direction[a]=static_cast<float>(static_cast<long double>(direction[a])+vertical[a]);
                direction[a]=static_cast<float>(static_cast<long double>(direction[a])+horizontal[a]);
            }
            direction=camera_normalize(direction,2); //00467222..238: (Y²+Z²)+X².
        }
        services_.car_direction(frame(),direction);
        }
        services_.update_scene_object(frame());
        if(!target_frame()||first_person.enabled()||first_person.secondary_enabled())return;
        if(target_human) {
            if(!target_human->head_marker||!target_human->base().frame())throw std::runtime_error("Camera opacity requires actual target Human head/model");
            const auto head=services_.actor_world_matrix(target_human->head_marker),body=services_.actor_world_matrix(target_human->base().frame());
            const auto camera=services_.actor_world_matrix(frame());
            const MissionCameraState::Vector point{{camera[12],camera[13],camera[14],camera[15]}},
                first{{head[12],head[13],head[14],head[15]}},last{{body[12],static_cast<float>(static_cast<long double>(body[13])+.10000000149011612f),body[14],body[15]}};
            float parameter;const auto square=camera_segment_square(point,first,last,parameter);
            float amount=0;
            if(frame()==services_.active_game_camera()&&square<=1.2000000476837158f) {
                const float stored=static_cast<float>(square);
                amount=stored<.20000000298023224f?1.f:static_cast<float>(1-(static_cast<long double>(stored)-.20000000298023224f));
            }
            if(amount!=state_.field2a4)state_.field2a4=amount;
        }
        if(state_.field2a4!=state_.field2a8) {
            if(state_.field2a4<=state_.field2a8) {
                state_.field2a8=static_cast<float>(static_cast<long double>(state_.field2a8)-.10000000149011612f);
                if(state_.field2a8<state_.field2a4)state_.field2a8=state_.field2a4;
            }else {
                state_.field2a8=static_cast<float>(static_cast<long double>(state_.field2a8)+.10000000149011612f);
                if(state_.field2a4<state_.field2a8)state_.field2a8=state_.field2a4;
            }
            if(target_human)set_human_camera_amount(*target_human,state_.field2a8,target_inline);
        }
    }
    //00469300's ordinary first world-frame path. World supplies typed lookup
    //of the SAME inline98 owners and target model's Scene2 Human. Input reads
    //the actual graphics EC/F0 and current profile39; no synthetic motion data.
    template<class Control,class Input,class World,class Queries,class FirstPerson>
    void simulate(std::uint32_t elapsed,std::uint32_t mode,Control& players,Input& input,World& world,
                  Queries& queries,FirstPerson& first_person,float& breathing) {
        if(state_.flag26e&&!state_.flag26d)return;
        if(state_.flag26d)throw std::runtime_error("Debug camera requires actual0046BEB0 receiver");
        if(state_.flag2b0)throw std::runtime_error("Detached camera requires actual0046B550 receiver");
        auto* current=players.current_player();
        if(current&&current->field160)throw std::runtime_error("Attached camera requires actual0046ADA0 receiver");
        auto mouse_x=input.mouse_x(),mouse_y=input.mouse_y();
        if(input.camera_mouse_inverted())mouse_y=signed_word(0u-static_cast<std::uint32_t>(mouse_y));
        auto* selected=players.selected_player(mode);
        state_.fields298[0]+=elapsed;
        if(!state_.flag26f||!world.camera_input_enabled()){mouse_x=0;mouse_y=0;}
        state_.target_offset[1]=0;
        auto* control=current?world.human_inline(*current):nullptr;
        if(state_.field260==2) {
            if(!selected) {
                mouse_x=mouse_y=0;if(current)state_.direction=human_camera_target_direction(*current);
            }
            if(current) {
                if(!control)throw std::runtime_error("First-person camera height query requires actual inline98");
                bool valid=false;float ignored_height;
                (void)control->query_obstruction_height(services_,valid,ignored_height);
                if(valid) {
                    const auto target=services_.actor_world_matrix(target_frame());const auto radius=real(settings_.rows()[configuration_][3]);
                    if(state_.field27c<=target[13]) {
                        const auto height=static_cast<long double>(radius)+state_.field27c+.10000000149011612f;
                        if(target[13]<height)state_.target_offset[1]=static_cast<float>(height-target[13]);
                    }else {
                        const auto height=(static_cast<long double>(state_.field27c)-.10000000149011612f)-radius;
                        if(height<target[13])state_.target_offset[1]=static_cast<float>(height-target[13]);
                    }
                }
            }
        }
        auto rotate=[&](const MissionCameraState::Vector& axis,float angle) {
            state_.direction=camera_normalize(state_.direction,2);
            const auto matrix=services_.human_axis_rotation(axis,angle);
            state_.direction=services_.collision_sweep_rotate(state_.direction,matrix);
            state_.direction=camera_normalize(state_.direction,2);
        };
        if(mouse_x&&(!selected||state_.field260==10))rotate({{0,-1,0,0}},static_cast<float>(static_cast<long double>(static_cast<float>(mouse_x))*0x1.0624dep-8f));
        const auto& row=settings_.rows()[configuration_];
        if(((row[4]&2u)||!selected)&&mouse_y) {
            state_.direction=camera_normalize(state_.direction,2);
            MissionCameraState::Vector axis{};
            if(state_.direction[0]==0&&state_.direction[2]==0&&(state_.direction[1]==1||state_.direction[1]==-1))axis={{1,0,0,0}};
            else axis={{-state_.direction[2],0,state_.direction[0],0}};
            axis=camera_normalize(axis,0);
            float angle=static_cast<float>(static_cast<long double>(static_cast<float>(mouse_y))*0x1.0624dep-8f);
            const auto pitch=std::asin(static_cast<long double>(state_.direction[1]));const auto requested=pitch-angle;
            if(real(row[5])<=requested)angle=static_cast<float>(pitch-real(row[5]));
            if(requested<=real(row[6]))angle=static_cast<float>(pitch-real(row[6]));
            rotate(axis,angle);
        }
        if((row[4]&1u)&&target_frame()) {
            state_.direction=camera_normalize(state_.direction,2);
            const bool attached_configuration=state_.field260==9||state_.field260==7||state_.field260==8;
            if((row[4]&8u)&&(attached_configuration||!selected)) {
                const auto matrix=services_.actor_world_matrix(target_frame());
                state_.direction=camera_normalize({{matrix[8],matrix[9],matrix[10],matrix[11]}},0);
                state_.direction=camera_normalize(state_.direction,2);
            }
        }
        follow(elapsed,current,control,queries,first_person,mode);
        auto* target_human=target_frame()?world.camera_target_human(target_frame()):nullptr;
        auto* target_inline=target_human?world.human_inline(*target_human):nullptr;
        apply_follow_pose(elapsed,world.camera_pose_suppressed(),current,target_human,target_inline,first_person,mode,world.local_player_id(),breathing);
        if(!frame())throw std::runtime_error("Camera update requires its loaded camera frame");
        services_.update_scene_object(frame());
        state_.field250-=elapsed;
        if(signed_word(state_.field250)<0) {
            const auto active=services_.active_game_camera();
            if(!active)throw std::runtime_error("Camera sector update has no active scene camera");
            const auto matrix=services_.actor_world_matrix(active);
            sector_=services_.human_scene_sector({{matrix[12],matrix[13],matrix[14],matrix[15]}});
            do {state_.field250+=500;}while(signed_word(state_.field250)<0);
        }
        decay_camera_offset(state_.vectors[2],state_.fields298[2],elapsed);
        decay_camera_offset(state_.vectors[3],state_.fields298[1],elapsed);
        if(state_.flag290) {
            state_.field294+=elapsed;
            if(static_cast<float>(signed_word(state_.field294))>5000.f)state_.field294=5000;
        }
    }
    Frame scene_sector()const{return sector_;} //004650C0, virtual8C, borrowed native24C.
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
        retire_target_={};look_target_=Frame{};sector_=Frame{};previous_hit_=nullptr;
        errors.rethrow(); //Source guards retract borrowed frames/descriptors before file scene reset.
    }
private:
    static std::uint32_t bits(float value){std::uint32_t result;std::memcpy(&result,&value,4);return result;}
    static std::int32_t signed_word(std::uint32_t word){std::int32_t value;std::memcpy(&value,&word,4);return value;}
    static float camera_length(long double square) {
        const float stored=static_cast<float>(square);
        if(signed_word(bits(stored))<0x322bcc77)return 0;
        return std::abs(square-1)>=9.99999993922529e-09f?static_cast<float>(std::sqrt(static_cast<long double>(stored))):1.f;
    }
    static MissionCameraState::Vector camera_normalize(MissionCameraState::Vector value,unsigned order) {
        const auto x=static_cast<long double>(value[0])*value[0],y=static_cast<long double>(value[1])*value[1],z=static_cast<long double>(value[2])*value[2];
        const auto square=order==0?(x+z)+y:order==1?(x+y)+z:(z+y)+x;const float stored=static_cast<float>(square);
        if(!std::isfinite(square))throw std::runtime_error("Non-finite camera direction");
        if(std::abs(square-1)>=9.99999993922529e-09f) {
            if(stored>=9.99999993922529e-09f){const auto scale=1/std::sqrt(static_cast<long double>(stored));for(unsigned a=0;a!=3;++a)value[a]=static_cast<float>(value[a]*scale);}
            else {const auto a=value[0]!=0?0u:value[2]!=0?2u:1u;value[a]=value[a]<0?-1.f:1.f;}
        }
        return value;
    }
    void apply_look_target(std::uint32_t elapsed) { //00467353..0046784F within00466970.
        if(!services_.script_scene_available())return;
        const auto active=services_.active_game_camera();if(!active)return;
        //The script may have selected an EXTERNAL camera. Native288 is a
        //borrower, independent of retained Human-follow248 and main-camera240.
        const auto camera=services_.actor_world_matrix(active);
        const auto target=services_.actor_world_matrix(look_target_);
        MissionCameraState::Vector desired{};
        for(unsigned a=0;a!=3;++a)desired[a]=static_cast<float>(static_cast<long double>(target[12+a])-camera[12+a]);
        desired=camera_normalize(desired,1); //004673E2: (X²+Y²)+Z².
        if(state_.field28c>0||std::isnan(state_.field28c)) {
            const auto updated=services_.actor_world_matrix(active);
            const auto previous=camera_normalize({{updated[8],updated[9],updated[10],updated[11]}},2);
            const auto angle=services_.car_vector_angle(previous,desired);
            const auto amount=static_cast<long double>(signed_word(elapsed))*state_.field28c*0x1.0624dep-10f/angle; //0080F514=.001f.
            float factor=static_cast<float>(amount);
            if(amount>1){factor=1;state_.field28c=-1;} //Strict >; equality retains the speed.
            desired=camera_normalize(services_.camera_interpolate_direction(previous,desired,factor),2);
        }
        services_.car_camera_direction(active,desired,0.f); //Camera virtual0C, not frame virtual1C.
        services_.update_scene_object(active); //Before common main-camera update, even when SAME.
    }
    static void decay_camera_offset(MissionCameraState::Vector& value,std::uint32_t& timer,std::uint32_t elapsed) {
        const auto square=(static_cast<long double>(value[2])*value[2]+static_cast<long double>(value[1])*value[1])+static_cast<long double>(value[0])*value[0];
        if(signed_word(bits(static_cast<float>(square)))<0x322bcc77)return;
        float length=camera_length(square);if(length==0)return;
        if(timer)timer-=elapsed;
        if(signed_word(timer)>0)return;
        length=static_cast<float>(static_cast<long double>(length)-static_cast<long double>(static_cast<float>(signed_word(elapsed-timer)))*0x1.0624dep-10f);
        if(length<0)length=0;
        value=camera_normalize(value,2);
        for(unsigned a=0;a!=3;++a)value[a]=static_cast<float>(static_cast<long double>(value[a])*length);
        timer=0;
    }
    static float real(std::uint32_t bits) {float value;std::memcpy(&value,&bits,4);return value;}
    void reset_loaded_state() {
        state_.flag26d=false;state_.field268=5;state_.flag26e=false;state_.flag26f=true;
        state_.flag270=false;state_.flag278=false;state_.field280=0;state_.field284=0;
        for(auto& value:state_.vectors)for(unsigned i=0;i!=3;++i)value[i]=0;
        look_target_=Frame{};state_.field28c=-1;sector_=Frame{};state_.field250=0;
        state_.flag290=false;state_.fields298.fill(0);state_.fieldd0=0;
    }
    Services& services_;
    MissionCameraSettings settings_;
    MissionCameraState state_;
    DetachedCamera<Services,Human> detached_; //SAME00462650/004626E0 controller, inactive during loading.
    std::optional<Owner> camera_;
    //00464DD0 constructs at static0089AEA0 (BSS): original unwritten50/58/27C
    //start at zero. Later posture/attached producers are separate recovery.
    Frame sector_{},look_target_{};const void* previous_hit_=nullptr; //Borrowed native24C/288/25C, never pointer DWORDs.
    Frame previous_{}; //Original244 borrows; scene owns it during this lifetime.
    std::optional<Owner> target_; //Original248 owns one additional frame reference.
    std::function<void(Frame)> retire_target_; //Required real model-owner/type3→005A9760(0) receiver; containing world outlives camera.
    std::size_t configuration_=1; //Original264 points at actual settings row, initially1.
};

} //namespace hd2::game
#endif
