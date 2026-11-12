#ifndef HD2_HOST_GAME_ACTOR_FRAMES_H
#define HD2_HOST_GAME_ACTOR_FRAMES_H
#include "game/car_mesh_fragments.h"

#include "game/mission_script_rotations.h"
#include "game/door_rotation_loading.h"
#include "game/scene_light_sample.h"
#include "game/volumetric_lights.h"

#include "host/game_car_frames.h"
#include "game/human_face_mesh_pool.h"
#include "game/mission_script_frame_snapshots.h"
#include <mmsystem.h>
#include <cstdlib>

namespace hd2::game {struct MissionParticleVertex;}
namespace hd2::host {

//Shared real frame endpoints for actor initialization. Entry registries,
//script contexts and concrete actor dispatch belong to the containing world.
class GameActorFrames : public GameCarFrames {
public:
    using GameCarFrames::GameCarFrames;
    MenuDriver::Owner retain_actor_frame(Frame frame) {return retain(frame);}
    std::uint32_t visual_lightmap_group(Frame frame)const{return field<std::uint32_t>(frame,0x210);}
    void set_visual_lightmap_group(Frame frame,std::uint32_t group){call<void>(frame,0xc8,group);} //0048D270/0048C790.
    std::int32_t vertex_random(){return std::rand();} //00474B40, SAME process CRT stream.
    void script_cutscene_viewport() { //00428B30, same actual graphics and world scene.
        const auto width=graphics_.methods->width(&graphics_);const auto height=graphics_.methods->height(&graphics_);
        const auto rectangle=game::mission_script_cutscene_viewport(width,height);
        using Set=void(WINAPI*)(Frame,std::int32_t,std::int32_t,std::int32_t,std::int32_t);
        scene_method<Set>(scene_.frame(),module_,0x148,0x48440)(scene_.frame(),rectangle[0],rectangle[1],rectangle[2],rectangle[3]);
    }
    void script_rotate_frame(Frame frame,std::uint32_t axis,float angle) { //00427200's actual engine matrix receivers.
        Matrix rotation{};using Rotate=Matrix*(WINAPI*)(void*,float);
        if(axis==1)exported<Rotate>("?RotateYMatrix@@YG?AUS_matrix@@M@Z",0x2c4d0)(rotation.data(),angle);
        else if(axis==2)exported<Rotate>("?RotateZMatrix@@YG?AUS_matrix@@M@Z",0x2c530)(rotation.data(),angle);
        else exported<Rotate>("?RotateXMatrix@@YG?AUS_matrix@@M@Z",0x2c470)(rotation.data(),angle);
        const auto local=car_local_matrix(frame);using Multiply=Matrix*(WINAPI*)(void*,const void*);
        exported<Multiply>("??XS_matrix@@QAGAAU0@ABU0@@Z",0x2d6b0)(rotation.data(),local.data());
        put(frame,0x80,rotation);Vector scale{};using Scale=Vector*(WINAPI*)(const void*,void*);
        exported<Scale>("?GetScale@S_matrix@@QBG?AUS_vector@@XZ",0x2db20)(rotation.data(),scale.data());
        put(frame,0xd0,scale);put(frame,0xe0,(field<std::uint32_t>(frame,0xe0)&0xffffffd7u)|0x40000014u);
        update_scene_object(frame);
    }
    game::MissionScriptLightSnapshot script_light_snapshot(Frame light)const {
        return {field<std::array<float,4>>(light,0x1a0),field<std::uint32_t>(light,0x16c)};
    }
    void script_restore_light(Frame light,const game::MissionScriptLightSnapshot& original) {
        call<void>(light,0xb0,original.colour.data());call<void>(light,0x8c,original.parameter);
    }
    void script_light_enabled(Frame light,bool enabled) {
        if(enabled)call<void>(light,0x94,std::uint32_t{0});
        call<void>(light,0xac,std::uint32_t(enabled),std::uint32_t{0});update_scene_object(light);
    }
    void effects_light_visibility(Frame light,bool enabled){call<void>(light,0xac,std::uint32_t(enabled),std::uint32_t{0});}
    MenuDriver::Owner create_prepared_human_model(){return driver_.create(9);}
    void link_prepared_human_root(Frame model) {
        if(const auto root=field<Frame>(scene_.frame(),0x1f8)) {
            if(call<std::int32_t>(model,0x38,root,std::uint32_t{0})<0)
                throw std::runtime_error("Cannot link prepared Human to actual scene root");
        }
    }
    void prepared_human_direction(Frame model,const Vector& direction){call<void>(model,0x1c,direction.data(),static_cast<const void*>(nullptr));}
    struct MissionLightProperties {bool visible;std::int32_t type;std::uint32_t flags;};
    MissionLightProperties mission_light_properties(Frame frame)const {
        return {(field<std::uint32_t>(frame,0xe0)&1u)!=0,field<std::int32_t>(frame,0x164),field<std::uint32_t>(frame,0x160)};
    }
    Bounds mission_frame_world_bounds(Frame frame) {
        if(!(field<std::uint32_t>(frame,0xe0)&0x100u))call<void>(frame,0x58);
        return field<Bounds>(frame,0x10);
    }
    template<class Visit>void enumerate_activation_scene(bool root,std::uint32_t mask,Visit&& visit) {
        using Visitor=std::remove_reference_t<Visit>;
        struct Context {
            Visitor& visit;std::exception_ptr failure;
            static std::int32_t WINAPI invoke(Frame frame,void* opaque)noexcept {
                auto& context=*static_cast<Context*>(opaque);
                if(!context.failure)try{context.visit(frame);}catch(...){context.failure=std::current_exception();}
                return 0;
            }
        } context{visit,{}};
        auto* scope=root?field<Frame>(scene_.frame(),0x1f8):scene_.frame();
        if(!scope)throw std::runtime_error("Mission activation requires its actual scene scope");
        call<std::int32_t>(scope,0x44,&Context::invoke,&context,mask,static_cast<const char*>(nullptr));
        if(context.failure)std::rethrow_exception(context.failure);
    }
    void* actor_opaque_data(Frame frame) const noexcept {return field<void*>(frame,0xec);}
    void actor_opaque_data(Frame frame,void* data) noexcept {put(frame,0xec,data);}
    Frame actor_find(Frame model,const char* name,std::uint32_t mask) {return car_find(model,name,mask);}
    Frame human_sight_marker(Frame model,const char* name) { //0046EBF0/0046EDE0: engine enumeration stops with2.
        struct Context {
            const char* name;Frame found=nullptr;
            static std::int32_t WINAPI visit(Frame frame,void* opaque) noexcept {
                auto& context=*static_cast<Context*>(opaque);
                const auto* full=field<const char*>(frame,0xe8);
                const auto* short_name=full?full+field<std::int16_t>(frame,0xe6):"";
                if(_stricmp(short_name,context.name)!=0)return 0;
                context.found=frame;return 2;
            }
        } context{name,nullptr};
        using Enumerate=std::int32_t(WINAPI*)(Frame,std::int32_t(WINAPI*)(Frame,void*),void*,std::uint32_t,const char*);
        scene_method<Enumerate>(model,module_,0x44,0x1de40)(model,&Context::visit,&context,0x400,nullptr);
        return context.found;
    }
    Matrix actor_world_matrix(Frame frame) {return car_world_matrix(frame);}
    void formation_matrix_direction(Matrix& matrix,const Vector& direction) {
        using Set=void(WINAPI*)(void*,const void*);
        exported<Set>("?SetDir@S_matrix@@QAGXABUS_vector@@@Z",0x2c7a0)(matrix.data(),direction.data());
    }
    Vector formation_transform(const Vector& vector,const Matrix& matrix) {
        Vector result{};using Transform=Vector*(WINAPI*)(const void*,void*,const void*);
        exported<Transform>("??DS_vector@@QBG?AU0@ABUS_matrix@@@Z",0x2eab0)(vector.data(),result.data(),matrix.data());
        return result;
    }
    const Matrix* actor_world_matrix_pointer(Frame frame) {
        (void)car_world_matrix(frame);return reinterpret_cast<const Matrix*>(address(frame,0x40));
    }
    Vector actor_local_scale(Frame frame) const {return field<Vector>(frame,0xd0);}
    bool physics_inverse_world_below_half(Frame frame) { //0040BD70 copies frame10..3F, compares the final FLOAT at3C.
        if(!(field<std::uint32_t>(frame,0xe0)&0x100u))call<void>(frame,0x58);
        return field<float>(frame,0x3c)<.5f;
    }
    bool version_collision_retires_immediately(Frame frame) { //0043C320, sphere radius3C after lazy bounds58.
        if(!(field<std::uint32_t>(frame,0xe0)&0x100u))call<void>(frame,0x58);
        return field<float>(frame,0x3c)<=.10000000149011612f;
    }
    void actor_link_preserving_world(Frame frame,Frame parent) {
        if(call<std::int32_t>(frame,0x38,parent,std::uint32_t{1})<0)
            throw std::runtime_error("Cannot preserve actor pose while linking its parent");
    }
    void physics_link_root(Frame frame) { //0040B5E0 preserves world pose while reparenting.
        const auto root=field<Frame>(scene_.frame(),0x1f8);
        if(call<std::int32_t>(frame,0x38,root,std::uint32_t{1})<0)
            throw std::runtime_error("Cannot link physics object to actual scene root");
    }
    void physics_publish_pose(Frame frame,const Vector& position,const std::array<float,4>& rotation){ //0040AB80, native frame08, direct normalized quaternion, SceneEC, frame24.
        call<void>(frame,8,position.data());object_rotation(frame,rotation);auto_parent_object(frame);update_scene_object(frame);
    }
    void physics_publish_position(Frame frame,const Vector& position){call<void>(frame,8,position.data());update_scene_object(frame);} //0040F5A2; no quaternion or parent update here.
    void replay_human_model_position(Frame frame,const Vector& position){ //00759E20, Scene144 BEFORE frame24.
        call<void>(frame,8,position.data());refresh_dynamic_human_model(frame);update_scene_object(frame);
    }
    void physics_restore_pose(Frame frame,const Vector& position,const Vector& rotation){object_rotation(frame,rotation);call<void>(frame,8,position.data());update_scene_object(frame);} //0040D1B1/0040D9BF, ONE24 after both writes, no SceneEC.
    Vector physics_local_translation(Frame frame){const auto matrix=car_local_matrix(frame);return {{matrix[12],matrix[13],matrix[14],matrix[15]}};} //0040F5E3/58 lazy local matrix.
    Vector physics_local_sphere_centre(Frame frame){(void)rigid_visual_bounds(frame);return field<Vector>(frame,0x1c0);}
    Vector physics_rotate_about_axis(const Vector& rotation,const Vector& axis,float angle){
        const auto matrix=human_axis_rotation(axis,angle);Vector result{};using Apply=void*(WINAPI*)(const void*,void*,const void*);
        exported<Apply>("?RotateByMatrix@S_quat@@QBG?AU1@ABUS_matrix@@@Z",0x30440)(rotation.data(),result.data(),matrix.data());
        return result;
    }
    void physics_publish_axis_rotation(Frame frame,const Vector& rotation,const Vector& axis,float angle){object_rotation(frame,physics_rotate_about_axis(rotation,axis,angle));update_scene_object(frame);}
    Vector physics_rotate_offset(const Vector& offset,const Vector& axis,float angle){return collision_sweep_rotate(offset,human_axis_rotation(axis,angle));}
    Vector physics_slerp(const Vector& first,const Vector& last,float factor,bool shortest){
        Vector result{};using Slerp=void*(WINAPI*)(const void*,void*,const void*,float,std::uint8_t);
        exported<Slerp>("?Slerp@S_quat@@QBG?AU1@ABU1@M_N@Z",0x302e0)(first.data(),result.data(),last.data(),factor,static_cast<std::uint8_t>(shortest));return result;
    }
    void human_clip_blend_position(Frame frame,const Vector& target,float influence){ //007B02B0, direct B0 before the signed flag80 gate.
        const auto previous=field<Vector>(frame,0xb0);
        const auto complement=1.L-influence;
        Vector position=previous;
        position[0]=static_cast<float>(complement*previous[0]+static_cast<long double>(target[0])*influence);
        const float old_y=static_cast<float>(complement*previous[1]),old_z=static_cast<float>(complement*previous[2]);
        const float new_z=static_cast<float>(static_cast<long double>(target[2])*influence);
        position[1]=static_cast<float>(old_y+static_cast<long double>(target[1])*influence);
        position[2]=old_z+new_z;
        car_suspension_position(frame,position);
    }
    void human_clip_blend_rotation(Frame frame,const Vector& target,float influence){ //007B02B0, actual engine quaternion cache.
        const auto flags=field<std::uint32_t>(frame,0xe0);
        if(!(flags&8u)){
            put(frame,0xe0,flags|8u);
            using Make=void(WINAPI*)(void*,const void*);
            exported<Make>("?Make@S_quat@@QAGXABUS_matrix@@@Z",0x2f220)(address(frame,0xc0),address(frame,0x80));
        }
        const float complement=1.f-influence;
        car_steering_rotation(frame,physics_slerp(target,field<Vector>(frame,0xc0),complement,false));
    }
    template<class Visit>void enumerate_physics_scene(Visit&& visit) { //00410B00/00410B40.
        using Visitor=std::remove_reference_t<Visit>;
        struct Context {
            Visitor& visit;std::exception_ptr failure;
            static std::int32_t WINAPI invoke(Frame frame,void* opaque) noexcept {
                auto& context=*static_cast<Context*>(opaque);
                if(!context.failure)try {context.visit(frame);}catch(...) {context.failure=std::current_exception();}
                return 0;
            }
        } context{visit,{}};
        using Enumerate=std::int32_t(WINAPI*)(Frame,std::int32_t(WINAPI*)(Frame,void*),void*,std::uint32_t,const char*);
        scene_method<Enumerate>(scene_.frame(),module_,0x44,0x46880)(scene_.frame(),&Context::invoke,&context,0xffff,nullptr);
        if(context.failure)std::rethrow_exception(context.failure);
    }
    struct PhysicsBounds {Vector minimum,maximum,centre;float radius;};
    PhysicsBounds physics_object_bounds(Frame frame,bool own_visual) { //00407390/0040E810.
        if(own_visual&&frame_kind(frame)==1) {
            const auto bounds=rigid_visual_bounds(frame);
            return {bounds.minimum,bounds.maximum,field<Vector>(frame,0x1c0),field<float>(frame,0x1cc)};
        }
        const auto bounds=car_bounds(frame);
        PhysicsBounds result{bounds.minimum,bounds.maximum,{},0};
        if(!(bounds.minimum[0]<bounds.maximum[0])) {
            if(own_visual)throw std::runtime_error("Physics object centre is unwritten for invalid aggregate bounds");
            return result;
        }
        Vector half{};
        for(unsigned axis=0;axis!=3;++axis) {
            const auto extent=static_cast<float>(static_cast<long double>(bounds.maximum[axis])-bounds.minimum[axis]);
            half[axis]=static_cast<float>(static_cast<long double>(extent)*.5f);
            result.centre[axis]=static_cast<float>(static_cast<long double>(bounds.minimum[axis])+half[axis]);
        }
        result.radius=static_cast<float>(std::sqrt((static_cast<long double>(half[2])*half[2]+
            static_cast<long double>(half[1])*half[1])+static_cast<long double>(half[0])*half[0]));
        return result;
    }
    Frame rigid_first_child(Frame frame) const {return field<Frame>(frame,0x10c);}
    Frame rigid_next_sibling(Frame frame) const {return field<Frame>(frame,0x110);}
    Bounds rigid_visual_bounds(Frame frame) {
        if(!(field<std::uint32_t>(frame,0x1d0)&0x20000u))call<void>(frame,0x90);
        return field<Bounds>(frame,0x1a0);
    }
    std::uint32_t actor_visual_flags(Frame frame)const{return field<std::uint32_t>(frame,0x1d0);}
    Frame human_eye_mesh(Frame visual)const{return field<Frame>(visual,0x1e0);}
    std::uint32_t human_eye_lod_count(Frame mesh)const{return field<std::uint32_t>(mesh,0x44);}
    Frame human_eye_lod(Frame mesh,std::uint32_t index){return call<Frame>(mesh,4,index);}
    std::uint32_t human_eye_group_count(Frame lod)const{return field<std::uint32_t>(lod,0x58);}
    Frame human_eye_group(Frame lod,std::uint32_t index){return call<Frame>(lod,0x14,index);}
    Frame human_eye_group_material(Frame group)const{return field<Frame>(group,4);}
    std::optional<std::string> human_eye_material_name(Frame material) { //0048A3F0 preserves all THREE lazy resolves.
        const auto resolve=[&]{if(field<std::uint32_t>(material,8)&0x4000000u)call<void>(material,0x44);};
        resolve();if(!field<Frame>(material,0xc))return std::nullopt;
        resolve();auto texture=field<Frame>(material,0xc);
        if(!texture)throw std::runtime_error("Human eye material lost its resolved texture");
        (void)field<const char*>(texture,0x18);
        resolve();texture=field<Frame>(material,0xc);
        if(!texture)throw std::runtime_error("Human eye material lost its actual texture name");
        const auto* name=field<const char*>(texture,0x18);return name?name:"";
    }
    std::optional<std::string> clothing_eye_material_name(Frame material) { //0048A780's additional TWO lazy resolves.
        const auto resolve=[&]{if(field<std::uint32_t>(material,8)&0x4000000u)call<void>(material,0x44);};
        resolve();if(!field<Frame>(material,0xc))return std::nullopt;
        resolve();const auto texture=field<Frame>(material,0xc);
        if(!texture)throw std::runtime_error("Clothing eye material lost its resolved texture");
        const auto* name=field<const char*>(texture,0x18);return name?name:"";
    }
    std::vector<Frame> clothing_frame_links(Frame frame)const { //00489FD0: native kind7+120/+128 snapshot, no AddRef.
        const auto count=field<std::uint32_t>(frame,0x128);const auto list=field<Frame*>(frame,0x120);
        if(count>0x100000u||(count&&!list))throw std::runtime_error("Invalid clothing frame link array");
        return count?std::vector<Frame>(list,list+count):std::vector<Frame>{};
    }
    void add_clothing_frame_link(Frame frame,Frame value){call<void>(frame,0x74,value);}
    ResourceOwner load_human_eye_texture(const std::string& name) {
        Frame texture=nullptr;
        const auto result=graphics_.methods->load_texture(&graphics_,name.c_str(),"",0x400,&texture);
        auto owner=ResourceOwner(texture,ResourceRelease{module_});
        if(result!=0)return ResourceOwner(nullptr,ResourceRelease{module_});
        return owner;
    }
    void prepare_human_eye_group(Frame group){call<void>(group,0x18);}
    void human_eye_material_texture(Frame material,Frame texture){call<void>(material,0x1c,texture);}
    ResourceOwner retain_human_eye_material(Frame material) {
        put(material,4,field<std::uint32_t>(material,4)+1u);
        return ResourceOwner(material,ResourceRelease{module_});
    }
    void human_eye_group_material(Frame group,Frame material){call<void>(group,0,material);}
    std::string human_face_model_resource(Frame model) const { //007968A0 uses actual model134, case-sensitive.
        const auto* name=field<const char*>(model,0x134);return name?name:"";
    }
    MenuDriver::Owner load_version_resource(Frame model){ //0043CE40, SAME category5 owner/cache/search directories.
        auto name=human_face_model_resource(model);
        if(name.size()>=128)throw std::length_error("Versions model name exceeds native buffer");
        if(const auto dot=name.rfind('.');dot!=std::string::npos)name.resize(dot);
        if(const auto slash=name.rfind('\\');slash!=std::string::npos)name.erase(0,slash+1);
        MenuDriver::Owner result(nullptr,nullptr);(void)resources_.cache(5).get("d"+name,result,{},0x10);return result;
    }
    std::int32_t reload_version_model(Frame model){return resources_.load_model(model,human_face_model_resource(model),0);}
    MenuDriver::Owner make_version_replacement(Frame old,Frame variant){ //0043BA60 prefix, preserve old visual subtype.
        auto next=create_visual(visual_type(old));const auto frame=next.get();
        call<void>(frame,0x4c,variant);call<void>(frame,0x38,actor_parent(old),std::uint32_t{0});
        car_suspension_position(frame,actor_position(old));
        prepared_human_direction(frame,car_forward(old));put(frame,0xd0,field<Vector>(old,0xd0));
        (void)car_rotation(frame);put(frame,0xe0,(field<std::uint32_t>(frame,0xe0)&0xfffffecbu)|0x40000008u);
        update_scene_object(frame);return next;
    }
    void attach_version_replacement(Frame old,Frame next){ //0043BA60 AFTER EC transfer/clear, before old reference release.
        const auto model=field<Frame>(old,0x100);if(!model)return;
        call<void>(model,0x7c,next);call<void>(model,0x80,old);call<void>(old,0x30,static_cast<const char*>(nullptr));update_scene_object(old);
        for(auto n=actor_child_count(old);n;){const auto child=actor_child(old,--n);
            if(child&&visual_type(child)==0x4a4f5250u){call<void>(child,0x38,next,std::uint32_t{1});update_scene_object(child);}
        }
        if(visual_type(old)==0x50414d4cu&&visual_type(next)==0x50414d4cu){
            const auto offset=field<std::uint32_t>(old,0x210)*0x38u;
            call<void>(next,0xc4,field<std::uint32_t>(old,0x218+offset));const auto target=field<std::uint32_t>(next,0x210)*0x38u;
            put(next,0x21c+target,field<std::uint32_t>(old,0x21c+offset));put(next,0x220+target,field<std::uint32_t>(old,0x220+offset));
            call<void>(next,0xc8,field<std::uint32_t>(old,0x210));const Vector up{{0,1,0,0}};
            call<void>(next,0xb4,scene_.frame(),up.data(),field<std::uint32_t>(old,0x21c+offset),std::uint32_t{4},std::uint32_t{0},std::uint32_t{0});
            update_scene_object(next);
        }
    }
    void finish_version_replacement(Frame next){
        for(auto n=actor_child_count(next);n;){const auto child=actor_child(next,--n);
            if(child&&visual_type(child)==0x4a4f5250u){call<void>(child,0xbc,std::uint32_t{0},std::uint32_t{0});update_scene_object(child);}
        }
    }
    void version_debris_pose(Frame target,Frame source){ //0043B140, WORLD forward normalized, rotation0C.
        auto world=actor_world_matrix(source);const Vector position{{world[12],world[13],world[14],world[15]}};
        call<void>(target,8,position.data());world=actor_world_matrix(source);
        const auto direction=game::actor_activation_direction(Vector{{world[8],world[9],world[10],world[11]}});
        call<void>(target,0x0c,direction.data(),std::uint32_t{0});
        Vector scale;
        if(!actor_parent(source))scale=field<Vector>(source,0xd0);
        else{world=actor_world_matrix(source);using Get=Vector*(WINAPI*)(const void*,void*);
            scale={};exported<Get>("?GetScale@S_matrix@@QBG?AUS_vector@@XZ",0x2db20)(world.data(),scale.data());}
        put(target,0xd0,scale);(void)car_rotation(target);
        put(target,0xe0,(field<std::uint32_t>(target,0xe0)&0xfffffecbu)|0x40000008u);update_scene_object(target);
    }
    void prepare_version_debris(Frame group,Frame source){if(const auto parent=actor_parent(group))version_debris_pose(parent,source);}
    MenuDriver::Owner create_version_fragment(Frame child){
        auto result=create_visual(visual_type(child));const auto frame=result.get();call<void>(frame,0x4c,child);
        put(frame,0x1d0,field<std::uint32_t>(frame,0x1d0)&0xffffdfffu);
        const auto world=actor_world_matrix(child);const Vector position{{world[12],world[13],world[14],world[15]}};
        const auto root=field<Frame>(scene_.frame(),0x1f8);if(!root)throw std::logic_error("Version debris requires actual scene root");
        call<void>(frame,0x38,root,std::uint32_t{0});update_scene_object(frame);
        //Original position is captured BEFORE reparenting the clone.
        call<void>(frame,8,position.data());const auto fresh=actor_world_matrix(child);
        const auto direction=game::actor_activation_direction(Vector{{fresh[8],fresh[9],fresh[10],fresh[11]}});
        call<void>(frame,0x0c,direction.data(),std::uint32_t{0});Vector scale;
        if(!actor_parent(child))scale=field<Vector>(child,0xd0);
        else{const auto matrix=actor_world_matrix(child);scale={};using Get=Vector*(WINAPI*)(const void*,void*);
            exported<Get>("?GetScale@S_matrix@@QBG?AUS_vector@@XZ",0x2db20)(matrix.data(),scale.data());}
        put(frame,0xd0,scale);(void)car_rotation(frame);put(frame,0xe0,(field<std::uint32_t>(frame,0xe0)&0xfffffecbu)|0x40000008u);
        update_scene_object(frame);place_human_reaction_model(frame);update_scene_object(frame);return result;
    }
    struct CarFragmentResources {
        MenuDriver::Owner visual;ResourceOwner mesh,lod;
        void retire(){lod.reset();mesh.reset();visual.reset();}
    };
    MenuDriver::Owner create_car_detached_clone(Frame source,const Vector& position,const Vector& direction){ //00479450 prefix.
        if(!visual_type(source))return MenuDriver::Owner{nullptr,nullptr};
        auto visual=create_visual(visual_type(source));if(!visual)throw std::runtime_error("Cannot create actual detached Car visual");
        call<void>(visual.get(),0x4c,source);call<void>(visual.get(),8,position.data());
        call<void>(visual.get(),0xc,direction.data(),std::uint32_t{0});update_scene_object(visual.get());return visual;
    }
    CarFragmentResources create_car_fragment(Frame source,const std::array<game::CarFragmentVertex,6>& vertices,Frame material){
        CarFragmentResources result{create_visual(0x5f4a424fu),ResourceOwner(call<Frame>(driver_.frame(),0x6c),ResourceRelease{module_}),
            ResourceOwner(call<Frame>(driver_.frame(),0x68),ResourceRelease{module_})};
        if(!result.visual||!result.mesh||!result.lod)throw std::runtime_error("Cannot create actual mesh-fragment visual/mesh/LOD");
        const auto lod=result.lod.get();call<void>(lod,0x28,std::uint32_t{6});auto* data=call<unsigned char*>(lod,4,std::uint32_t{0});
        if(!data)throw std::runtime_error("Cannot write actual fragment vertices");
        for(std::size_t n=0;n!=vertices.size();++n){const auto& vertex=vertices[n];const std::array<float,8> values{{
            vertex.position[0],vertex.position[1],vertex.position[2],vertex.normal[0],vertex.normal[1],vertex.normal[2],vertex.uv[0],vertex.uv[1]}};
            std::memcpy(data+n*32,values.data(),32);
        }
        call<void>(lod,8);
        const auto group=call<Frame>(driver_.frame(),0x64);if(!group)throw std::runtime_error("Cannot create actual fragment face group");
        //This group uses SetMaterial at0, not resource Release. Preserve the
        //driver/LOD face-group transfer; do not give it a COM-style deleter.
        call<void>(group,0,material);auto* indices=call<std::uint16_t*>(group,8,std::uint32_t{2});
        if(!indices)throw std::runtime_error("Cannot write actual fragment face indices");
        for(std::uint16_t n=0;n!=6;++n)indices[n]=n;
        call<void>(lod,0x1c,group);call<void>(lod,0x2c);call<void>(result.mesh.get(),8,lod,std::uint32_t{0});
        const auto visual=result.visual.get();call<void>(visual,0x9c,result.mesh.get(),std::uint32_t{0});
        const auto root=field<Frame>(scene_.frame(),0x1f8);if(!root)throw std::logic_error("Fragment requires actual containing Scene root");
        call<void>(visual,0x38,root,std::uint32_t{0});const auto position=actor_world_matrix(source);
        call<void>(visual,8,position.data()+12);car_steering_rotation(visual,human_matrix_rotation(actor_world_matrix(source)));
        call<void>(visual,0xb0);update_scene_object(visual);return result;
    }
    template<class Visit>void car_fragment_triangles(Frame frame,Visit&& visit){ //00485BA0.
        if(!(actor_visual_flags(frame)&0x800u))return;
        const auto mesh=car_mesh(frame);const auto first=call<Frame>(mesh,4,std::uint32_t{0});
        const auto group=call<Frame>(first,0x14,std::uint32_t{0});const auto count=field<std::uint32_t>(group,8);
        const auto lod=call<Frame>(mesh,4,std::uint32_t{0});const auto* vertices=call<const unsigned char*>(lod,4,std::uint32_t{0});
        const auto material=field<Frame>(group,4);const auto vertex_count=field<std::uint32_t>(lod,0x54);
        if(field<std::uint32_t>(lod,0x64)!=32)throw std::runtime_error("Fragment source requires original32-byte vertex format");
        for(auto n=count;n;){const auto* indices=call<const std::uint16_t*>(group,0xc,std::uint32_t{0});
            if(!indices||!vertices)throw std::runtime_error("Fragment source has no actual face/vertex data");
            const auto triangle=--n;std::array<game::CarFragmentVertex,3> input;
            for(unsigned corner=0;corner!=3;++corner){const auto index=indices[triangle*3+corner];if(index>=vertex_count)throw std::out_of_range("Fragment source vertex index");
                const auto* data=vertices+index*32;std::memcpy(input[corner].position.data(),data,12);std::memcpy(input[corner].uv.data(),data+24,8);
            }
            visit(input,material);
        }
    }
    Frame human_face_lod(Frame visual) {
        const auto mesh=field<Frame>(visual,0x1e0);
        if(!mesh)throw std::runtime_error("Skinned Human face visual has no mesh");
        return call<Frame>(mesh,4,std::uint32_t{0});
    }
    template<class Visit>void with_human_face_vertices(Frame lod,Visit&& visit) { //00797020 lock spans all seven groups.
        const auto* bytes=call<const unsigned char*>(lod,4,std::uint32_t{0});
        struct Unlock {
            Frame lod;HMODULE module;
            ~Unlock(){using Method=void(WINAPI*)(Frame);checked_engine_method<Method>(lod,module,8)(lod);}
        } unlock{lod,module_};
        const auto count=field<std::uint32_t>(lod,0x54);
        if(!bytes&&count)throw std::runtime_error("Cannot read actual Human face LOD0 vertices");
        std::vector<game::HumanFaceMeshData::Position> vertices(count);
        for(std::uint32_t n=0;n!=count;++n)std::memcpy(vertices[n].data(),bytes+static_cast<std::size_t>(n)*32,12);
        visit(vertices);
    }
    template<class Visit>void with_human_face_hit_mesh(Frame lod,Frame visual,Visit&& visit){ //00799760 locks before the live skinned-position query.
        const auto* locked=call<const unsigned char*>(lod,4,std::uint32_t{0});
        if(!locked)return;
        struct Unlock {
            Frame lod;HMODULE module;
            ~Unlock(){using Method=void(WINAPI*)(Frame);checked_engine_method<Method>(lod,module,8)(lod);}
        } unlock{lod,module_};
        struct Mesh {
            const unsigned char* positions;const unsigned char* coordinates;std::uint32_t count;
            game::HumanFaceMeshData::Position position(std::uint16_t vertex)const{
                if(vertex>=count||!positions)throw std::runtime_error("Human face hit requires its actual live vertex position");
                game::HumanFaceMeshData::Position value;
                std::memcpy(value.data(),positions+static_cast<std::size_t>(vertex)*16,12);return value;
            }
            std::array<float,2> uv(std::uint16_t vertex)const{
                if(vertex>=count||!coordinates)throw std::runtime_error("Human face hit requires its actual retained UV coordinates");
                std::array<float,2> value;
                std::memcpy(value.data(),coordinates+static_cast<std::size_t>(vertex)*8,8);return value;
            }
        } mesh{call<const unsigned char*>(visual,0xbc,std::uint32_t{0}),field<const unsigned char*>(lod,0x4c),field<std::uint32_t>(lod,0x54)};
        visit(mesh);
    }
    template<class Visit>void with_human_face_stain_texture(Frame texture,std::uint32_t width,Visit&& visit){ //00799BB0 lock precedes the CRT stamp draw.
        if(!texture){visit(nullptr);return;}
        std::uint32_t pitch=0;
        auto* pixels=call<unsigned char*>(texture,0x14,std::uint32_t{0},&pitch,std::uint32_t{0});
        struct Unlock {
            Frame texture;HMODULE module;
            ~Unlock(){using Method=void(WINAPI*)(Frame);checked_engine_method<Method>(texture,module,0x18)(texture);}
        } unlock{texture,module_};
        if(pitch!=width*2)return; //Original rejects before any stamp draw. Source releases the acquired lock on this path.
        visit(pixels);
    }
    std::vector<game::HumanFaceMeshData::Triangle> human_face_triangles(Frame lod,unsigned index) {
        const auto group=call<Frame>(lod,0x14,static_cast<std::uint32_t>(index));
        if(!group)throw std::runtime_error("Required Human face material group is missing");
        const auto* bytes=call<const unsigned char*>(group,0x0c,std::uint32_t{0});
        const auto count=field<std::uint32_t>(group,8);
        if(!bytes&&count)throw std::runtime_error("Cannot read actual Human face triangles");
        std::vector<game::HumanFaceMeshData::Triangle> triangles(count);
        for(std::uint32_t n=0;n!=count;++n)std::memcpy(triangles[n].data(),bytes+static_cast<std::size_t>(n)*6,6);
        return triangles;
    }
    void human_face_prepare_visual(Frame visual) { //007982C0 before UV binding.
        call<void>(visual,0xa0);const auto mesh=field<Frame>(visual,0x1e0);
        if(!mesh)throw std::runtime_error("Human face preparation lost its actual mesh");
        call<void>(mesh,0x18);
    }
    void human_face_prepare_uv(Frame lod) { //007982C0: same lock and native38 finish, not08.
        const auto* vertices=call<const unsigned char*>(lod,4,std::uint32_t{0});
        if(!field<void*>(lod,0x4c)) {
            call<void>(lod,0x3c,std::uint32_t{2});
            const auto count=field<std::uint32_t>(lod,0x54);auto* uv=field<unsigned char*>(lod,0x4c);
            if(count&&(!vertices||!uv))throw std::runtime_error("Cannot allocate Human face UV pairs");
            for(std::uint32_t n=0;n!=count;++n)std::memcpy(uv+static_cast<std::size_t>(n)*8,vertices+static_cast<std::size_t>(n)*32+24,8);
        }
        call<void>(lod,0x38,std::uint32_t{0},std::uint32_t{0},std::uint32_t{0});
    }
    void human_face_finish_visual(Frame visual,Frame lod) {
        put(lod,0x64,(field<std::uint32_t>(lod,0x64)|8u));
        put(lod,0x64,(field<std::uint32_t>(lod,0x64)&~2u)|5u);
        call<void>(visual,0xc4);
    }
    Frame human_face_group(Frame lod,unsigned index){return call<Frame>(lod,0x14,static_cast<std::uint32_t>(index));}
    void human_face_prepare_group(Frame lod,unsigned index) {
        const auto group=human_face_group(lod,index);
        if(!group)throw std::runtime_error("Human face mask group is missing");
        call<void>(group,0x18);
    }
    void human_face_group_texture(Frame lod,unsigned index,Frame texture) {
        const auto group=human_face_group(lod,index);
        if(!group)throw std::runtime_error("Human face texture group is missing");
        const auto material=field<Frame>(group,4);
        if(!material)throw std::runtime_error("Human face group has no actual material");
        call<void>(material,0x2c,texture);call<void>(material,0x30,std::uint32_t{6});call<void>(material,0x34,std::uint32_t{5});
    }
    std::array<std::uint32_t,2> human_face_texture_size(Frame texture) const {
        return {{field<std::uint16_t>(texture,0x20),field<std::uint16_t>(texture,0x22)}};
    }
    void human_face_upload(Frame texture,const std::vector<std::uint16_t>& pixels,std::uint32_t height) {
        std::uint32_t pitch=0;
        auto* bytes=call<unsigned char*>(texture,0x14,std::uint32_t{0},&pitch,std::uint32_t{0});
        if(!bytes)return; //Native keeps the material assignment when lock returns null.
        struct Unlock {
            Frame texture;HMODULE module;
            ~Unlock(){using Method=void(WINAPI*)(Frame);checked_engine_method<Method>(texture,module,0x18)(texture);}
        } unlock{texture,module_};
        const auto count=static_cast<std::uint64_t>(height)*pitch;
        if(count>static_cast<std::uint64_t>(pixels.size())*2)
            throw std::runtime_error("Human face texture pitch exceeds its native mask allocation");
        if(count)std::memcpy(bytes,pixels.data(),static_cast<std::size_t>(count));
    }
    std::array<Vector,8> hud_project_world_corners(const std::array<Vector,8>& points){
        std::array<Vector,8> projected{};
        using Project=std::int32_t(WINAPI*)(Frame,const void*,void*,std::uint32_t);
        //Scene vtable1009B798+98→100476A0, RET16; actual scene's active camera.
        if(scene_method<Project>(scene_.frame(),module_,0x98,0x476a0)(scene_.frame(),points.data(),projected.data(),8)<0)
            throw std::runtime_error("Cannot project HUD actor corners through the actual world scene");
        return projected;
    }
    Vector hud_project_world_point(Frame scene,const Vector& point){
        if(scene!=scene_.frame())throw std::logic_error("HUD pointer requires SAME actual world scene");
        Vector projected{};using Project=std::int32_t(WINAPI*)(Frame,const void*,void*,std::uint32_t);
        if(scene_method<Project>(scene_.frame(),module_,0x98,0x476a0)(scene_.frame(),point.data(),projected.data(),1)<0)
            throw std::runtime_error("Cannot project HUD pointer through actual world scene");
        return projected;
    }
    std::uint32_t hud_pointer_hit_tag(Frame hud_scene,const Vector& origin,const Vector& direction){
        //Scene1009B798+FC ->10010C20, RET40, including six trailing zeroes.
        using Pick=Frame(WINAPI*)(Frame,const void*,const void*,std::uint32_t,void*,void*,std::uint32_t,std::uint32_t,void*,std::uint32_t);
        const auto hit=scene_method<Pick>(hud_scene,module_,0xfc,0x10c20)(hud_scene,origin.data(),direction.data(),0x4040000,nullptr,nullptr,0,0,nullptr,0);
        return hit?field<std::uint32_t>(hit,0xec):0;
    }
    std::pair<Frame,std::uint32_t> hud_inventory_hit(Frame hud_scene,const Vector& origin,const Vector& direction){ //004A39B0 uses40000, no4040000 interaction mask.
        using Pick=Frame(WINAPI*)(Frame,const void*,const void*,std::uint32_t,void*,void*,std::uint32_t,std::uint32_t,void*,std::uint32_t);
        const auto hit=scene_method<Pick>(hud_scene,module_,0xfc,0x10c20)(hud_scene,origin.data(),direction.data(),0x40000,nullptr,nullptr,0,0,nullptr,0);
        return {hit,hit?field<std::uint32_t>(hit,0xec):0};
    }
    std::pair<Vector,Vector> hud_world_screen_ray(Frame world,std::int32_t x,std::int32_t y){
        if(world!=scene_.frame())throw std::logic_error("HUD command ray requires SAME actual World scene");
        std::pair<Vector,Vector> result{};
        using Ray=std::int32_t(WINAPI*)(Frame,std::int32_t,std::int32_t,void*,void*);
        (void)scene_method<Ray>(world,module_,0x94,0x47250)(world,x,y,result.first.data(),result.second.data()); //004BA070, RET20; original ignores status.
        return result;
    }
    Matrix rigid_matrix_product(const Matrix& first,const Matrix& second) {
        Matrix result{};
        using Product=Matrix*(WINAPI*)(const void*,void*,const void*);
        exported<Product>("??DS_matrix@@QBG?AU0@ABU0@@Z",0x2d830)(first.data(),result.data(),second.data());
        return result;
    }
    std::array<float,4> physics_rotate_quaternion(const std::array<float,4>& rotation,const Vector& angles){ //0040ADE0, native engine math call order.
        Matrix z{},y{},x{};using Rotate=Matrix*(WINAPI*)(void*,float);
        exported<Rotate>("?RotateZMatrix@@YG?AUS_matrix@@M@Z",0x2c530)(z.data(),angles[2]);
        exported<Rotate>("?RotateYMatrix@@YG?AUS_matrix@@M@Z",0x2c4d0)(y.data(),angles[1]);
        exported<Rotate>("?RotateXMatrix@@YG?AUS_matrix@@M@Z",0x2c470)(x.data(),angles[0]);
        const auto xy=rigid_matrix_product(x,y);const auto xyz=rigid_matrix_product(xy,z);
        std::array<float,4> result{};using Apply=void*(WINAPI*)(const void*,void*,const void*);
        exported<Apply>("?RotateByMatrix@S_quat@@QBG?AU1@ABUS_matrix@@@Z",0x30440)(rotation.data(),result.data(),xyz.data());return result;
    }
    std::string actor_user_data(Frame frame) const {return car_user_data(frame);}
    Frame actor_parent(Frame frame) const {return object_parent(frame);}
    bool human_model_has_script_context(Frame model)const { //0059F5BF, SAME modelEC frame-data record.
        const auto* data=field<const std::uint32_t*>(model,0xec);return data&&data[1]!=0;
    }
    template<class Visit>void enumerate_human_control_frames(Frame model,Visit&& visit) { //0045D710, actual model+44(mask400).
        using Visitor=std::remove_reference_t<Visit>;
        struct Context {
            Visitor& visit;std::exception_ptr failure;
            static std::int32_t WINAPI invoke(Frame frame,void* opaque)noexcept {
                auto& context=*static_cast<Context*>(opaque);
                if(!context.failure)try{context.visit(frame);}catch(...){context.failure=std::current_exception();}
                return 0; //0045D7C0 always continues.
            }
        } context{visit,{}};
        using Enumerate=std::int32_t(WINAPI*)(Frame,std::int32_t(WINAPI*)(Frame,void*),void*,std::uint32_t,const char*);
        scene_method<Enumerate>(model,module_,0x44,0x1de40)(model,&Context::invoke,&context,0x400,nullptr);
        if(context.failure)std::rethrow_exception(context.failure);
    }
    Vector actor_position(Frame frame) const {return car_position(frame);}
    void item_actor_unit_scale(Frame frame) {call<void>(frame,0x54,1.f);} //005B68D0 actual uniform scale receiver.
    game::MissionQuaternion actor_rotation(Frame frame) {return car_rotation(frame);}
    game::MissionQuaternion item_marker_rotation(Frame frame) { //007D5250 publishes the cache flag before Make.
        const auto flags=field<std::uint32_t>(frame,0xe0);
        if(!(flags&8u)){
            put(frame,0xe0,flags|8u);
            using Make=void(WINAPI*)(void*,const void*);
            exported<Make>("?Make@S_quat@@QAGXABUS_matrix@@@Z",0x2f220)(address(frame,0xc0),address(frame,0x80));
        }
        return field<game::MissionQuaternion>(frame,0xc0);
    }
    void item_auxiliary_link(Frame frame,Frame parent){(void)call<std::int32_t>(frame,0x38,parent,0u);} //007D5380 ignores native status.
    void item_auxiliary_forward(Frame frame){const Vector forward{{0,0,1,0}};call<void>(frame,0x1c,forward.data(),0.f);} //007D5380, THREE native direction lanes.
    void human_mounted_model_position(Frame model,const Vector& position){call<void>(model,8,position.data());} //00793490, no early24.
    void human_mounted_model_sector(Frame model,Frame sector){(void)call<std::int32_t>(model,0x38,sector,1u);} //00793490 ignores native status.
    void human_mounted_model_visibility(Frame model,Frame source){ //79367D..68D retains native ECX upper bytes, masks CL only.
        const auto argument=(static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(source))&0xffffff00u)|
            (field<std::uint8_t>(source,0xe0)&1u);
        call<void>(model,0x30,argument);
    }
    game::MissionQuaternion human_matrix_rotation(const Matrix& matrix) {
        game::MissionQuaternion result{};
        using Make=void(WINAPI*)(void*,const void*);
        exported<Make>("?Make@S_quat@@QAGXABUS_matrix@@@Z",0x2f220)(result.data(),matrix.data());
        return result;
    }
    void human_set_matrix_rotation(Matrix& matrix,const Vector& rotation){
        using Set=void(WINAPI*)(void*,const void*);
        exported<Set>("?SetRot3@S_matrix@@QAGXABUS_quat@@@Z",0x2d450)(matrix.data(),rotation.data());
    }
    void human_set_matrix_quaternion(Matrix& matrix,const Vector& rotation){ //007A43C0 uses full SetRot.
        using Set=void(WINAPI*)(void*,const void*);
        exported<Set>("?SetRot@S_matrix@@QAGXABUS_quat@@@Z",0x2d260)(matrix.data(),rotation.data());
    }
    Matrix human_direction_matrix(const Vector& direction,const Vector& up){ //007C02F0, full SetDir with explicit up.
        Matrix result{};using Set=void(WINAPI*)(void*,const void*,const void*);
        exported<Set>("?SetDir@S_matrix@@QAGXABUS_vector@@0@Z",0x2ced0)(result.data(),direction.data(),up.data());
        return result;
    }
    void human_set_direction_roll(Matrix& matrix,const Vector& direction,float roll){ //007A8520, actual full SetDir overload.
        using Set=void(WINAPI*)(void*,const void*,float);
        exported<Set>("?SetDir@S_matrix@@QAGXABUS_vector@@M@Z",0x2cad0)(matrix.data(),direction.data(),roll);
    }
    void human_store_bone_rotation(Frame bone,Matrix matrix,bool unit_translation_w=false){ //007A6230 stores without a per-bone24 update.
        const auto position=field<Vector>(bone,0xb0);
        for(unsigned axis=0;axis!=4;++axis)matrix[12+axis]=position[axis];
        if(unit_translation_w)matrix[15]=1;
        human_equipment_local_matrix(bone,matrix);
    }
    void human_publish_bone_rotation(Frame bone,Matrix matrix,bool unit_translation_w=false){ //007BF980, preserve direct B0 before publishing80 and24.
        human_store_bone_rotation(bone,matrix,unit_translation_w);
        update_scene_object(bone);
    }
    void human_geometry_weapon_link(Frame weapon,Frame parent){ //007AFD70, native LinkTo flags5 and ignored result.
        (void)call<std::int32_t>(weapon,0x38,parent,std::uint32_t{5});
    }
    Matrix human_axis_rotation(const Vector& axis,float angle) {
        game::MissionQuaternion rotation{};Matrix result{};
        using Make=void(WINAPI*)(void*,const void*,float);
        exported<Make>("?Make@S_quat@@QAGXABUS_vector@@M@Z",0x2f070)(rotation.data(),axis.data(),angle);
        using Rotation=Matrix*(WINAPI*)(const void*,void*);
        exported<Rotation>("?RotationMatrix@S_quat@@QBG?AUS_matrix@@XZ",0x2ee70)(rotation.data(),result.data());
        return result;
    }
    Vector camera_interpolate_direction(const Vector& previous,const Vector& desired,float factor) {
        game::MissionQuaternion first{},last{},result{};Vector direction{};
        using Set=void(WINAPI*)(void*,const void*,float);
        auto set=exported<Set>("?SetDir@S_quat@@QAGXABUS_vector@@M@Z",0x2ff50);
        set(first.data(),previous.data(),0.f);set(last.data(),desired.data(),0.f);
        using Slerp=void*(WINAPI*)(const void*,void*,const void*,float,std::uint8_t);
        exported<Slerp>("?Slerp@S_quat@@QBG?AU1@ABU1@M_N@Z",0x302e0)(first.data(),result.data(),last.data(),factor,0);
        using Get=void*(WINAPI*)(const void*,void*);
        exported<Get>("?GetDir@S_quat@@QBG?AUS_vector@@XZ",0x2efd0)(result.data(),direction.data());
        return direction;
    }
    void human_equipment_local_matrix(Frame frame,const Matrix& matrix) { //Shared0078DF60/007A59C0 matrix assignment.
        put(frame,0x80,matrix);Vector scale{};
        using Scale=Vector*(WINAPI*)(const void*,void*);
        exported<Scale>("?GetScale@S_matrix@@QBG?AUS_vector@@XZ",0x2db20)(address(frame,0x80),scale.data());
        for(unsigned axis=0;axis!=3;++axis)put(frame,0xd0+axis*4,scale[axis]); //DC is unwritten SIMD padding.
        put(frame,0xe0,(field<std::uint32_t>(frame,0xe0)&0xffffffd7u)|0x40000014u);
    }
    void human_equipment_shift_z(Frame frame,float amount){ //00792CE0: static80 suppresses position and dirty stores.
        const auto flags=field<std::uint32_t>(frame,0xe0);if(flags&0x80u)return;
        put(frame,0xb8,static_cast<float>(static_cast<long double>(field<float>(frame,0xb8))+amount));
        put(frame,0xe0,(flags&0xfffffedfu)|0x40000000u);
    }
    void human_equipment_action_link(Frame frame,Frame parent,std::uint32_t flags){(void)call<std::int32_t>(frame,0x38,parent,flags);} //0078DF60/00790520 ignore status.
    void human_equipment_action_visibility(Frame frame,const void* human,bool hidden){
        const auto argument=(static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(human))&0xffffff00u)|std::uint32_t(!hidden);
        call<void>(frame,0x30,argument);
    }
    Bounds human_equipment_bounds_data(Frame frame)const{return field<Bounds>(frame,0x10);}
    void human_equipment_link(Frame frame,Frame parent,std::uint32_t flags) {
        if(call<std::int32_t>(frame,0x38,parent,flags)<0)
            throw std::runtime_error("Cannot attach required Human equipment frame");
    }
    void human_unit_pose(Frame frame) { //0079C320; native DC is an unwritten SIMD padding word.
        for(unsigned axis=0;axis!=3;++axis)put(frame,0xd0+axis*4,1.f);
        (void)car_rotation(frame);
        put(frame,0xe0,(field<std::uint32_t>(frame,0xe0)&0xfffffecbu)|0x40000008u);
    }
    float human_volume_radius(Frame frame) const { //0075C070/0075B870, kind6 bound X extent.
        return static_cast<float>((static_cast<long double>(field<float>(frame,0x130))-field<float>(frame,0x120))*.5f);
    }
    std::uint32_t human_morph_count(Frame frame) { //00780AD0 uses the inline morph receiver, not the visual vtable.
        return call<std::uint32_t>(reinterpret_cast<Frame>(address(frame,0x120)),0);
    }
    void human_morph_weight(Frame frame,std::uint32_t group,std::uint32_t index,float weight) {
        call<void>(reinterpret_cast<Frame>(address(frame,0x120)),8,group,index,weight);
    }
    Frame human_model_clip(Frame model,std::uint32_t slot) {return call<Frame>(model,0x88,slot);}
    float human_model_weight(Frame model,std::uint32_t slot) {return call<float>(model,0x98,slot);}
    void human_model_weight(Frame model,std::uint32_t slot,float weight) {call<void>(model,0x94,slot,weight);}
    std::int32_t human_model_time(Frame model,std::uint32_t slot) {return call<std::int32_t>(model,0xa0,slot);}
    void human_model_time(Frame model,std::uint32_t slot,std::int32_t time) {call<void>(model,0x9c,slot,time);}
    std::int32_t human_model_channel_count(Frame model){return call<std::int32_t>(model,0xd0);}
    Frame* human_model_channels(Frame model){return call<Frame*>(model,0xd4);}
    Frame human_model_channel_target(Frame binding){return field<Frame>(binding,0xec);}
    void remove_human_model_channel_animation(Frame binding,std::int32_t slot){call<void>(binding,0,slot);} //00775510, channel virtual0 takes the slot.
    std::uint32_t human_model_animation_flags(Frame model,std::uint32_t slot) {return call<std::uint32_t>(model,0xa8,slot);}
    void human_model_animation_flags(Frame model,std::uint32_t slot,std::uint32_t flags) {call<void>(model,0xa4,slot,flags);}
    bool human_model_animation_enabled(Frame model,std::uint32_t slot) {return call<std::uint8_t>(model,0xac,slot)!=0;}
    void human_model_animation(Frame model,Frame clip,std::uint32_t slot,float weight,std::uint32_t flags) {
        call<void>(model,0x84,clip,slot,weight,flags);
    }
    void human_model_advance(Frame model,const std::array<std::int32_t,8>& times) {call<void>(model,0xb0,times.data());}
    void human_model_remove_animation(Frame model,std::uint32_t slot) {call<void>(model,0x8c,slot);}
    void human_equipment_tick_model(Frame model,std::int32_t elapsed){call<void>(model,0x20,elapsed);} //00790520 case9.
    void suppress_human_mount_visuals(Frame model){ //0075DF70 native model120/128 rereads, visual1 virtual80(0).
        for(std::int32_t n=0;n<field<std::int32_t>(model,0x128);++n){
            const auto list=field<Frame*>(model,0x120);
            if(!list||n>=0x100000)throw std::runtime_error("Mounted Human has invalid frame array");
            const auto frame=list[n];if(!frame)throw std::runtime_error("Mounted Human has null visual");
            if(frame_kind(frame)==1)call<void>(frame,0x80,0u);
        }
    }
    void restore_human_mount_visuals(Frame model){ //0075E290, native flattened model120/128 array is reread each visit.
        for(std::uint32_t n=0;n<field<std::uint32_t>(model,0x128);++n){
            const auto list=field<Frame*>(model,0x120);
            if(!list||n>=0x100000u)throw std::runtime_error("Human mount model has an invalid frame array");
            const auto frame=list[n];
            if(!frame)throw std::runtime_error("Human mount model contains a null frame");
            if(frame_kind(frame)==1)call<void>(frame,0x80,2u);
        }
    }
    void human_model_parameter_c4(Frame model,std::uint32_t value) {call<void>(model,0xc4,value);}
    std::int32_t human_clip_duration(Frame clip) {return call<std::int32_t>(clip,8);}
    std::int32_t human_clip_bone_index(Frame clip,const char* bone){return call<std::int32_t>(clip,0x14,bone);}
    std::int32_t human_clip_position_count(Frame clip,std::int32_t bone){return call<std::int32_t>(clip,0x18,bone);}
    void human_clip_position_key(Frame clip,std::int32_t bone,std::int32_t key,std::uint32_t& time,Vector& position){
        (void)call<std::int32_t>(clip,0x1c,bone,key,&time,position.data());
    }
    std::int32_t human_clip_rotation_count(Frame clip,std::int32_t bone){return call<std::int32_t>(clip,0x20,bone);}
    void human_clip_rotation_key(Frame clip,std::int32_t bone,std::int32_t key,std::uint32_t& time,Vector& rotation){
        (void)call<std::int32_t>(clip,0x24,bone,key,&time,rotation.data());
    }
    void human_clip_word8(Frame clip,std::uint32_t value) {put(clip,8,value);}
    Frame human_scene_sector(const Vector& position) {return call<Frame>(scene_.frame(),0xe0,position.data());}
    Bounds blast_frame_bounds(Frame frame){ //006DDFC0 uses driver+A8, DISTINCT from Car bounds+AC.
        if(frame_kind(frame)==6)return field<Bounds>(frame,0x120);
        Bounds storage{};using Get=const Bounds*(WINAPI*)(Frame,Bounds*,Frame);
        const auto* result=checked_engine_method<Get>(driver_.frame(),module_,0xa8)(driver_.frame(),&storage,frame);
        if(!result)throw std::runtime_error("Actual blast target bounds are unavailable");return *result;
    }
    template<class Clear>bool visit_blast_human_markers(Frame model,Clear&& clear){ //006DE560, actual 'a' visual skeletal table214.
        const auto visual=actor_find(model,"a",1);
        if(!visual)throw std::runtime_error("Blast occlusion requires Human a visual");
        const auto* table=field<const Frame*>(visual,0x214);
        if(!table)throw std::runtime_error("Blast occlusion requires actual Human skeletal frames");
        constexpr std::array<unsigned,6> indices{{24,3,14,10,17,21}}; //008434EC.
        for(const auto index:indices){const auto marker=table[index];
            if(!marker)throw std::runtime_error("Missing actual Human blast marker");
            if(clear(marker))return true; //Do not read later markers after first clear ray.
        }
        return false;
    }
    Frame effects_lensflare() { //00485B70 searches actual scene+1FC frame's+48, not scene+7C.
        const auto root=field<Frame>(scene_.frame(),0x1fc);
        if(!root)throw std::runtime_error("Effects lensflare lookup requires the actual loaded scene root");
        return actor_find(root,"lensflare",0xffff);
    }
    void human_presentation_driver_state(std::uint32_t state,std::uint32_t value) {(void)driver_.set_render_state(state,value);}
    void register_human_model(Frame model) { //00785BE0, original scene+138 receives q then z.
        const auto z=actor_find(model,"z",0xffff),q=actor_find(model,"q",0xffff);
        call<void>(scene_.frame(),0x138,model,q,z);
    }
    void refresh_dynamic_human_model(Frame model){call<void>(scene_.frame(),0x144,model);} //007646F0/00755500, original scene receiver.
    void detach_human_effect_model(Frame model){(void)call<std::int32_t>(model,0x38,Frame{},std::uint32_t{5});} //007B0910 ignores native LinkTo return.
    void human_reaction_link(Frame model,Frame parent){(void)call<std::int32_t>(model,0x38,parent,std::uint32_t{1});} //0074D510/occupied seat removal ignore LinkTo status.
    Frame human_visual_bone(Frame visual,unsigned index){ //005A3CC0, actual borrowed skeletal frame table214.
        if(!visual)throw std::logic_error("Human damage point requires its actual skeletal visual");
        const auto* bones=field<const Frame*>(visual,0x214);
        if(!bones||index>=25)throw std::logic_error("Human damage point has no actual bone table entry");
        return bones[index];
    }
    void shot_blood_scale(Frame frame,float size,float lane){
        put(frame,0xd0,size);put(frame,0xd4,size);put(frame,0xd8,.05000000074505806f);put(frame,0xdc,lane);
        car_rotation(frame);put(frame,0xe0,(field<std::uint32_t>(frame,0xe0)&0xfffffecbu)|0x40000008u);
    }
    void human_reaction_model_direction(Frame model,const Vector& direction){call<void>(model,0xc,&direction,0.f);}
    void place_human_reaction_model(Frame model){const auto world=actor_world_matrix(model);const Vector position{{world[12],world[13],world[14],world[15]}};call<void>(scene_.frame(),0xec,model,&position);}
    void register_human_effect_model(Frame model,Frame z,Frame q){call<void>(scene_.frame(),0x138,model,z,q);} //007B2B3D..007B2B4C, actual retained markers.
    void unregister_human_model(Frame model) {call<void>(scene_.frame(),0x13c,model);}
    void flag_human_model(Frame model) {put(model,0xe0,field<std::uint32_t>(model,0xe0)|0x400000u);}
    using HumanAnimationCallback=std::int32_t(WINAPI*)(Frame,std::uint32_t,const char*,std::int32_t);
    void human_animation_callback(Frame marker,HumanAnimationCallback callback,std::uint32_t flags) {
        call<void>(marker,0x28,callback,flags);
    }
    MenuDriver::Owner create_human_holster_model() {return driver_.create(9);}
    MenuDriver::Owner create_human_equipment_light() {return driver_.create(2);}
    void human_equipment_lens_parameters(Frame lens,bool enabled) { //007933D0's direct lens fields.
        put(lens,0x1e4,enabled?.13f:.1f);put(lens,0x244,enabled?.22f:.15f);
        put(lens,0x1e0,.75f);put(lens,0x240,enabled?100.f:50.f);
    }
    void human_equipment_action_light(Frame light,Frame fire) { //007931B0's actual light virtual receivers.
        call<void>(light,0x74,std::uint32_t{1});
        const std::array<float,3> colour{{.35f,.35f,.1f}};call<void>(light,0x7c,colour.data());
        call<void>(light,0x88,.4f);call<void>(light,0x8c,std::uint32_t{0});call<void>(light,0x90,std::uint32_t{0});
        call<void>(light,0x9c,.3f,std::uint32_t{0},std::uint32_t{0});call<void>(light,0x98,1.5f);
        human_equipment_link(light,fire,0);
    }
    void human_equipment_place_light(Frame light) {
        const auto world=actor_world_matrix(light);
        const auto sector=human_scene_sector(Vector{{world[12],world[13],world[14],world[15]}});
        if(!sector)throw std::runtime_error("Human action light has no actual scene sector");
        add_light_sector(light,sector);
    }
    void human_equipment_remove_light_sectors(Frame light) { //Count/list are reread after every native removal.
        for(std::uint32_t n=0;n<field<std::uint32_t>(light,0x128);++n) {
            const auto list=field<Frame*>(light,0x120);
            if(!list)throw std::runtime_error("Human action light sector list is missing");
            call<void>(list[n],0x78,light);
        }
    }
    void human_equipment_restore_action(Frame model,const std::array<float,4>& position,const std::array<float,4>& rotation) {
        call<void>(model,8,position.data());object_rotation(model,rotation);
    }
    void human_action_model_pose(Frame model,const Vector& position,const Vector& direction,Frame parent){ //00755500 case8.
        call<void>(model,8,position.data());
        call<void>(model,0x0c,direction.data(),0.f);
        (void)call<std::int32_t>(model,0x38,parent,std::uint32_t{1}); //Original ignores the LinkTo result.
        call<void>(model,0x24);
    }
    void human_loose_item_pose(Frame model,const Vector& position,const Vector& direction){ //00478EE0, frame14 differs from direction0C.
        call<void>(model,0x14,position.data(),direction.data(),static_cast<const void*>(nullptr));
        call<void>(model,0x24);
    }
    std::int32_t load_human_holster_model(Frame model,const char* key) {return resources_.load_model(model,key,0x10);}
    void remove_human_holster_frame(Frame model,Frame frame) {call<void>(model,0x80,frame);}
    void human_equipment_opacity(Frame frame,float opacity) {call<void>(frame,0xa4,opacity);}
    void human_equipment_name(Frame frame,const std::string& name) {
        if(call<std::int32_t>(frame,0x34,name.c_str())<0)throw std::runtime_error("Cannot name Human equipment");
    }
    void effects_model_opacity(Frame model,float opacity) {call<void>(model,0x50,opacity);} //00486DF0, distinct from OBJ_+A4.
    void effects_frame_fade_alpha(Frame frame,float alpha){call<void>(frame,0xa8,alpha);} //00486E60, distinct from both model+50 and visual+A4.
    void effects_model_camera_amount(Frame model,float value) {call<void>(model,0x54,value);} //00486D30; distinct from model+50 opacity.
    float effects_human_fade_amount(Frame visual)const{return field<float>(visual,0x1ec);} //00489180 reads actual child a/flag-mask1.
    void detach_human_projection(Frame frame){link_scene_object(frame,Frame{});call<void>(scene_.frame(),0x84,frame);} //0078A5F0, release owning reference separately AFTER scene removal.
    void human_projection_mode(Frame frame,unsigned value){call<void>(frame,0xb8,value);}
    void human_projection_blend(Frame frame,unsigned value){call<void>(frame,0xb4,value);}
    float human_projection_alpha(Frame frame)const{return field<float>(frame,0x1e8);}
    void human_projection_alpha(Frame frame,float value){call<void>(frame,0xa8,value);}
    void human_footprint_colour(Frame frame,float value){const Vector colour{{value,value,value,0}};call<void>(frame,0xb0,colour.data());}
    void human_footprint_add(Frame frame){call<void>(scene_.frame(),0x80,frame);}
    void human_footprint_range(Frame frame,float value){put(frame,0x1e4,value);}
    void human_footprint_project(Frame frame){call<void>(frame,0xbc,std::uint32_t{0},std::uint32_t{0});}
    MenuDriver::Owner human_shadow_template(const std::string& key) {
        MenuDriver::Owner result{nullptr,nullptr};
        if(resources_.cache(4).get(key,result)!=0)result.reset(); //004894E0 requires exact zero after a cache miss.
        return result;
    }
    MenuDriver::Owner create_human_shadow_model() {return driver_.create(15);}
    void copy_human_shadow_model(Frame destination,Frame source) {
        if(call<std::int32_t>(destination,0x4c,source)<0)throw std::runtime_error("Cannot copy required shadow model");
    }
    Frame frame_model_owner(Frame frame) const {return field<Frame>(frame,0x100);}
    Frame find_human_shadow_model(Frame model) { //00489470/004894B0 stop at first owned kind15 descendant.
        if(!model)return Frame{};
        struct Context {
            GameActorFrames& frames;Frame expected,found=nullptr;
            static std::int32_t WINAPI invoke(Frame frame,void* opaque) noexcept {
                auto& context=*static_cast<Context*>(opaque);
                if(context.frames.frame_model_owner(frame)!=context.expected)return 0;
                context.found=frame;return 2;
            }
        } context{*this,model};
        using Enumerate=std::int32_t(WINAPI*)(Frame,std::int32_t(WINAPI*)(Frame,void*),void*,std::uint32_t,const char*);
        checked_engine_method<Enumerate>(model,module_,0x44)(model,&Context::invoke,&context,0x2000,nullptr);
        return context.found;
    }
    void human_shadow_name(Frame frame,const std::string& name) {
        if(call<std::int32_t>(frame,0x34,name.c_str())<0)throw std::runtime_error("Cannot name shadow model");
    }
    void add_human_shadow_model(Frame model,Frame shadow) {call<void>(model,0x7c,shadow);}
    void remove_human_shadow_model(Frame model,Frame shadow) {call<void>(model,0x80,shadow);}
    void human_shadow_sine_squared(Frame shadow,float value) {call<void>(shadow,0x8c,value);}
    void human_shadow_direction(Frame shadow,const std::array<std::uint32_t,3>& value) {
        const std::array<std::uint32_t,4> padded{{value[0],value[1],value[2],0}};
        call<void>(shadow,0x90,padded.data());
    }
    void human_shadow_dimensions(Frame shadow,std::uint32_t width,std::uint32_t height) {call<void>(shadow,0x7c,width,height);}
    void human_shadow_distance(Frame shadow,std::uint32_t value) {put(shadow,0x1fc,value);}
    void human_shadow_range(Frame shadow,std::uint32_t value) {call<void>(shadow,0x80,value);}
    void human_shadow_scale(Frame shadow,std::uint32_t value) {call<void>(shadow,0x84,value);}
    std::uint32_t human_sector_light_count(Frame sector) const {return field<std::uint32_t>(sector,0x1ac);}
    Frame human_sector_light(Frame sector,std::uint32_t index) const {
        if(index>=human_sector_light_count(sector))throw std::out_of_range("Human sector light index");
        const auto lights=field<Frame*>(sector,0x1a4);
        if(!lights)throw std::runtime_error("Human sector light list is unavailable");
        return lights[index];
    }
    std::uint32_t human_light_type(Frame light) const {return field<std::uint32_t>(light,0x164);}
    game::SceneLightSample scene_light_sample(Frame light)const{
        game::SceneLightSample result;result.visible=(field<std::uint32_t>(light,0xe0)&1u)!=0;
        result.kind=field<std::uint32_t>(light,0x164);result.flags=field<std::uint32_t>(light,0x160);
        result.colour=field<Vector>(light,0x1a0);result.position=field<Vector>(light,0x224);result.direction=field<Vector>(light,0x230);
        result.attenuation={{field<float>(light,0x244),field<float>(light,0x248),field<float>(light,0x24c),0}};
        result.range=field<float>(light,0x174);result.outer_cosine=field<float>(light,0x1d0);
        result.inner_cosine=field<float>(light,0x1d4);result.cone_scale=field<float>(light,0x1d8);return result;
    }
    std::array<float,3> human_light_colour(Frame light) const {return field<std::array<float,3>>(light,0x130);}
    float human_light_multiplier(Frame light) const {return field<float>(light,0x168);}
    std::uint32_t human_frame_epoch(Frame frame) const {return vertex_frame_epoch(frame);}
    std::uint32_t human_scene_epoch() const {return vertex_scene_epoch();}
    bool human_control_visible(Frame frame)const{return (field<std::uint8_t>(frame,0xe0)&1u)!=0;}
    Vector human_footstep_local_direction(Frame frame){ //0075D88D, normalize only an already cached local matrix.
        const bool cached=(field<std::uint32_t>(frame,0xe0)&0x10u)!=0;
        if(!cached){
            put(frame,0xe0,field<std::uint32_t>(frame,0xe0)|0x10u);
            using Normalize=void(WINAPI*)(void*);exported<Normalize>("?Normalize@S_quat@@QAGXXZ",0x30540)(address(frame,0xc0));
            using Set=void(WINAPI*)(void*,const void*);exported<Set>("?SetRot3@S_matrix@@QAGXABUS_quat@@@Z",0x2d450)(address(frame,0x80),address(frame,0xc0));
        }
        Vector direction=field<Vector>(frame,0xa0);
        if(cached){
            const auto square=(static_cast<long double>(direction[1])*direction[1]+static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[0])*direction[0];
            constexpr float epsilon=9.99999993922529e-09f;
            if(!(std::abs(square-1.L)<epsilon)){
                const float stored=static_cast<float>(square);
                if(!(stored<epsilon)){const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));for(unsigned axis=0;axis!=3;++axis)direction[axis]=static_cast<float>(direction[axis]*inverse);}
                else{const auto axis=direction[0]!=0?0u:direction[2]!=0?2u:1u;direction[axis]=direction[axis]<0?-1.f:1.f;}
            }
        }
        return direction;
    }
    void human_control_set_position(Frame frame,const Vector& position){ //007599F0.
        call<void>(frame,0x08,position.data());
        call<void>(scene_.frame(),0x144,frame);
        update_scene_object(frame);
    }
    void human_control_set_direction(Frame frame,const Vector& direction,float roll){ //00759B80.
        call<void>(frame,0x0c,direction.data(),roll);
        update_scene_object(frame);
    }
    Vector collision_sweep_rotate(const Vector& value,const Matrix& matrix) {
        Vector result{};
        using Rotate=Vector*(WINAPI*)(const void*,void*,const void*);
        exported<Rotate>("?RotateByMatrix@S_vector@@QBG?AU1@ABUS_matrix@@@Z",0x2ec80)(value.data(),result.data(),matrix.data());
        return result;
    }
    void particle_point_direction(Matrix& matrix,const Vector& direction){ //0043F730/0043FB40, SetDir3 preserves translation.
        using Set=void(WINAPI*)(void*,const void*);
        exported<Set>("?SetDir3@S_matrix@@QAGXABUS_vector@@@Z",0x2c7d0)(matrix.data(),direction.data());
    }
    auto create_mission_particle_visual(){ //00440530, SAME driver EMTR factory and matching frame release.
        auto visual=create_car_particle_frame();if(!visual)throw std::runtime_error("Mission particle EMTR factory returned null");
        put(visual.get(),0x1e1,std::uint8_t{1});car_particle_name(visual.get(),"MR_Particle");return visual;
    }
    void mission_particle_texture(Frame visual,const std::string& name){ //00440800, GetIGraph+3C(name,NULL,2,out), bind9C then release.
        Frame texture=nullptr;const auto result=graphics_.methods->load_texture(&graphics_,name.c_str(),nullptr,2,&texture);
        ResourceOwner owner(texture,ResourceRelease{module_});
        if(result==0){if(!owner)throw std::runtime_error("Particle texture loader returned null success");car_particle_texture(visual,owner.get());}
    }
    void mission_particle_initial_mode(Frame visual,std::uint32_t kind,const std::array<std::uint32_t,2>& special){
        car_particle_property(visual,0x13,kind==3?0x80u:1u<<(kind&31u));
        if(kind==3){call<void>(visual,0xb0,special[0]);call<void>(visual,0xb8,special[1]);}
    }
    void mission_particle_world(Frame visual,const Matrix& matrix){
        put(visual,0x11c,0xffffffffu);put(visual,0x40,matrix);put(visual,0xe0,field<std::uint32_t>(visual,0xe0)|0x20u);
    }
    void mission_particle_place(Frame scene,Frame visual){const auto world=actor_world_matrix(visual);
        const Vector position{{world[12],world[13],world[14],world[15]}};call<void>(scene,0xec,visual,position.data());}
    void mission_particle_epoch(Frame visual,std::uint32_t epoch){put(visual,0x11c,epoch);}
    std::uint32_t mission_particle_scene_epoch(Frame scene)const{return field<std::uint32_t>(scene,0x11c);}
    Matrix mission_particle_inverse(Frame visual){ //00441460 caches the actual EMTR inverse130 under native1D0 bit100000.
        const auto flags=field<std::uint32_t>(visual,0x1d0);if(flags&0x40u)return game::human_identity();
        if(!(flags&0x100000u)){put(visual,0x1d0,flags|0x100000u);(void)actor_world_matrix(visual);
            using Inverse=std::uint8_t(WINAPI*)(void*,const void*);
            (void)exported<Inverse>("?Inverse@S_matrix@@QAG_NABU1@@Z",0x2c620)(address(visual,0x130),address(visual,0x40));}
        return field<Matrix>(visual,0x130);
    }
    float mission_particle_depth(Frame scene,const Vector& position){ //00441670 uses actual camera158/168/178/188, not a substitute camera matrix.
        const auto camera=field<Frame>(scene,0x15c);if(!camera)throw std::logic_error("Particle depth requires actual scene camera");
        return static_cast<float>(((static_cast<long double>(position[1])*field<float>(camera,0x168)+
            static_cast<long double>(position[2])*field<float>(camera,0x178))+static_cast<long double>(position[0])*field<float>(camera,0x158))+field<float>(camera,0x188));
    }
    void mission_particle_bounds(Frame visual,const std::array<float,12>& bounds){call<void>(visual,0x74,bounds.data());}
    Matrix mission_particle_camera_matrix(Frame scene){const auto camera=field<Frame>(scene,0x15c);
        if(!camera)throw std::logic_error("Particle publication requires actual scene camera");return actor_world_matrix(camera);}
    Matrix mission_particle_rotate_matrix(const Matrix& first,const Matrix& second){Matrix result{};using Rotate=Matrix*(WINAPI*)(const void*,void*,const void*);
        exported<Rotate>("?RotateByMatrix@S_matrix@@QBG?AU1@ABU1@@Z",0x2d9b0)(first.data(),result.data(),second.data());return result;}
    game::MissionParticleVertex* mission_particle_vertices(Frame visual,std::uint32_t count,float depth){return call<game::MissionParticleVertex*>(visual,0xd4,count,depth);}
    Matrix human_y_rotation(float angle){ //007694F0, actual LS3DF RotateYMatrix.
        Matrix result{};using Rotate=Matrix*(WINAPI*)(void*,float);
        exported<Rotate>("?RotateYMatrix@@YG?AUS_matrix@@M@Z",0x2c4d0)(result.data(),angle);
        return result;
    }
    Matrix human_z_rotation(float angle){ //007A32C0/007A3500, actual LS3DF RotateZMatrix.
        Matrix result{};using Rotate=Matrix*(WINAPI*)(void*,float);
        exported<Rotate>("?RotateZMatrix@@YG?AUS_matrix@@M@Z",0x2c530)(result.data(),angle);
        return result;
    }
    void mark_human_air_shadow(Frame frame){put(frame,0xe0,field<std::uint32_t>(frame,0xe0)|0x80u);} //00769920.
    MenuDriver::Owner create_game_camera() {return driver_.create(3);}
    MenuDriver::Owner create_human_sound() {return driver_.create(4);}
    bool human_sound_available() {
        using Get=Frame(__cdecl*)();const auto driver=exported<Get>("ISndGetDriver",0x68fa0)();
        using Available=std::uint8_t(WINAPI*)(Frame);
        return scene_method<Available>(driver,module_,0x0c,0x7a840)(driver)!=0;
    }
    std::int32_t load_car_sound(Frame sound,const std::string& name,std::uint32_t flags){return resources_.load_sound(sound,name,flags);} //0051BD50→0044A1C0.
    std::string car_sound_filename(Frame sound)const{
        const auto file=field<Frame>(sound,0x140);if(!file)return {};
        const auto cow=field<const char*>(file,0x0c);return cow?cow+8:""; //0051C0D0, SAME actual sound resource owner.
    }
    void car_sound_pitch(Frame sound,float value){call<void>(sound,0x88,value);}
    bool car_sound_state(Frame sound){return call<std::uint8_t>(sound,0x78,std::uint32_t{0})!=0;}
    float car_sound_volume(Frame sound)const{return field<float>(sound,0x16c);} //0051B560.
    std::uint32_t car_sound_elapsed(Frame sound){return call<std::uint32_t>(sound,0xa4);} //0051A950.
    void car_sound_stop(Frame sound){call<void>(sound,0xac,std::uint32_t{0},std::uint32_t{1});} //0051B8F0.
    void car_sound_parameter98(Frame sound,float value){call<void>(sound,0x98,value);}
    void car_sound_cone(Frame sound,float first,float second){call<void>(sound,0x84,first,second);}
    void car_sound_register(Frame receiver,Frame sound){call<void>(receiver,0x7c,sound);} //00517CF0.
    void car_sound_unregister(Frame receiver,Frame sound){call<void>(receiver,0x80,sound);} //005181C0.
    void car_light_register(Frame receiver,Frame light){call<void>(receiver,0x74,light);} //004F6080.
    void car_light_unregister(Frame receiver,Frame light){call<void>(receiver,0x78,light);} //004F61E0.
    void car_projector_texture(Frame projector,Frame texture){call<void>(projector,0x9c,texture);}
    void car_light_power(Frame light,float value){call<void>(light,0x88,value);}
    void car_camera_position(Frame marker,const Vector& position){call<void>(marker,8,position.data());}
    std::uint32_t car_settling_ticks()const{return timeGetTime();} //004F67B0, actual multimedia clock.
    void car_place_in_scene(Frame model,const Vector& centre){call<void>(scene_.frame(),0xec,model,centre.data());} //004EAA07/0050B214.
    std::int32_t load_car_skid_model(Frame model,const std::string& key){return resources_.load_model(model,key,0);} //004EC02C/48, verbatim category1 keys.
    MenuDriver::Owner car_skid_frame(Frame model){
        const auto frame=call<Frame>(model,0x40,std::uint32_t{0});
        if(!frame)throw std::runtime_error("Car skid model has no actual frame0");
        return retain(frame); //004EC06C/004EC0C1, actual WORD E4 ownership.
    }
    void car_link_skid_root(Frame frame){link_scene_object(frame,field<Frame>(scene_.frame(),0x1f8));}
    void car_camera_direction(Frame marker,const Vector& direction,float roll){call<void>(marker,0x0c,direction.data(),roll);}
    void car_suspension_position(Frame frame,const Vector& position){ //00512390's direct parent-position write.
        const auto flags=field<std::uint32_t>(frame,0xe0);if(flags&0x80u)return;
        for(unsigned axis=0;axis!=3;++axis)put(frame,0xb0+axis*4,position[axis]);
        put(frame,0xe0,(flags&0xfffffedfu)|0x40000000u);
    }
    void car_steering_rotation(Frame frame,const std::array<float,4>& rotation){ //00507400.
        put(frame,0xc0,rotation);using Normalize=void(WINAPI*)(void*);
        exported<Normalize>("?Normalize@S_quat@@QAGXXZ",0x30540)(address(frame,0xc0));
        put(frame,0xe0,(field<std::uint32_t>(frame,0xe0)&0xfffffecbu)|0x40000008u);
    }
    void car_steering_wheel_direction(Frame frame,const Vector& direction,float roll){call<void>(frame,0x1c,direction.data(),roll);}
    void car_body_pose(Frame frame,const Vector& position,const Vector& forward,const Vector& up){ //00510E64.
        car_suspension_position(frame,position);using Set=void(WINAPI*)(void*,const void*,const void*);
        exported<Set>("?SetDir3@S_matrix@@QAGXABUS_vector@@0@Z",0x2cf10)(address(frame,0x80),forward.data(),up.data());
        put(frame,0xe0,(field<std::uint32_t>(frame,0xe0)&0xfffffed3u)|0x40000010u);
    }
    void rigid_body_pose(Frame frame,const Vector& position,const Vector& forward,const Vector& up){
        call<void>(frame,0x10,position.data(),forward.data(),up.data()); //004CCC53, actual frame virtual10/RET16.
    }
    void rigid_body_position(Frame frame,const Vector& position){call<void>(frame,8,position.data());}
    void rigid_publish_frame(Frame frame){ //004C93FA..004C9424, distinct from auto-parent's world-position input.
        call<void>(frame,0x24);
        if(!(field<std::uint32_t>(frame,0xe0)&0x100u))call<void>(frame,0x58);
        call<void>(scene_.frame(),0xec,frame,address(frame,0x30));
    }
    Vector rigid_descriptor_position(Frame frame)const{return field<Vector>(frame,0x30);}
    bool rigid_intersection_line(const Vector& first,const Vector& second,Vector& point,Vector& direction){
        using Intersect=std::uint8_t(WINAPI*)(const void*,const void*,void*,void*);
        return exported<Intersect>("?IntersectionLine@S_plane@@QBG_NABU1@AAUS_vector@@1@Z",0x306c0)(first.data(),second.data(),point.data(),direction.data())!=0;
    }
    long double rigid_vector_angle(const Vector& first,const Vector& second){
        //The native caller consumes ST0 directly with FSIN, without a FLOAT spill.
        using Angle=long double(WINAPI*)(const void*,const void*);
        return exported<Angle>("?AngleTo@S_vector@@QBGMABU1@@Z",0x2e8c0)(first.data(),second.data());
    }
    std::array<float,4> car_axis_quaternion(const Vector& axis,float angle){
        std::array<float,4> result{};using Make=void(WINAPI*)(void*,const void*,float);
        exported<Make>("?Make@S_quat@@QAGXABUS_vector@@M@Z",0x2f070)(result.data(),axis.data(),angle);return result;
    }
    Matrix car_quaternion_matrix(const std::array<float,4>& rotation){
        Matrix result{};using Make=Matrix*(WINAPI*)(const void*,void*);
        exported<Make>("?RotationMatrix@S_quat@@QBG?AUS_matrix@@XZ",0x2ee70)(rotation.data(),result.data());return result;
    }
    Matrix car_matrix_product(Matrix first,const Matrix& second){
        using Multiply=Matrix*(WINAPI*)(void*,const void*);
        exported<Multiply>("??XS_matrix@@QAGAAU0@ABU0@@Z",0x2d6b0)(first.data(),second.data());return first;
    }
    void car_loading_forward_basis(Frame frame){
        const Vector forward{{0,0,1,0}},up{{0,1,0,0}};using Set=void(WINAPI*)(void*,const void*,const void*);
        exported<Set>("?SetDir3@S_matrix@@QAGXABUS_vector@@0@Z",0x2cf10)(address(frame,0x80),forward.data(),up.data());
        put(frame,0xe0,(field<std::uint32_t>(frame,0xe0)&0xfffffed3u)|0x40000010u);
    }
    void car_sound_head(Frame sound,const Vector& head){
        const auto flags=field<std::uint32_t>(sound,0xe0);if(flags&0x80u)return;
        for(unsigned n=0;n!=3;++n)put(sound,0xb0+n*4,head[n]);
        put(sound,0xe0,(flags&0xfffffedfu)|0x40000000u);
    }
    std::int32_t load_human_sound(Frame sound,const char* name) {return resources_.load_sound(sound,name,0);}
    void human_sound_enabled(Frame sound,bool enabled) {call<void>(sound,0x30,std::uint32_t(enabled));}
    void human_sound_parameter7c(Frame sound,std::uint32_t value) {call<void>(sound,0x7c,value);}
    void human_sound_volume(Frame sound,float value) {call<void>(sound,0x8c,value);}
    float human_sound_volume(Frame sound)const{return field<float>(sound,0x16c);}
    std::int32_t human_sound_duration(Frame sound){return call<std::int32_t>(sound,0xa8);}
    void human_sound_attach(Frame sound,Frame parent,const std::array<float,3>& position){ //00785290.
        call<void>(sound,0x38,parent,std::uint32_t{0});
        object_position(sound,position);
    }
    void human_sound_range(Frame sound,float near_range,float far_range,float first,float second) {call<void>(sound,0x80,near_range,far_range,first,second);}
    void human_sound_parameter90(Frame sound,std::uint32_t value) {call<void>(sound,0x90,value);}
    void human_sound_parameter94(Frame sound,std::uint32_t value) {call<void>(sound,0x94,value);}
    void human_sound_tag(Frame sound,std::uint32_t value) {put(sound,0xec,value);}
    std::uint32_t human_sound_tag(Frame sound) const {return field<std::uint32_t>(sound,0xec);}
    void human_sound_detach(Frame sound) {call<void>(sound,0x38,Frame{},std::uint32_t{0});}
    void human_sound_missing(const char* name,std::int32_t) {std::fprintf(stderr,"AESound :%s : not found\n",name);}
    void hud_injury_prepare(Frame visual) {call<void>(visual,0xa0);}
    void hud_injury_movie_texture(Frame visual,Frame texture){ //004AD380: ONE LOD0 lookup, then first-group material.
        const auto material=hud_first_material_from_lod(first_person_material_lod(visual));
        if(!material)throw std::logic_error("HUD injury visual requires its actual material");
        call<void>(material,0x1c,texture);
    }
    void hud_position(Frame frame,const std::array<float,3>& position) {call<void>(frame,8,position.data());}
    void hud_pointer_position(Frame frame,const Vector& position){ //004ABAF0's client-inline B0/E0 stores; sign bit of low BYTE gates them.
        auto flags=field<std::uint32_t>(frame,0xe0);if(flags&0x80u)return;
        for(unsigned n=0;n!=3;++n)put(frame,0xb0+n*4,position[n]);
        put(frame,0xe0,(flags&0xfffffedfu)|0x40000000u);
    }
    void hud_group18(Frame visual) { //004AB230 calls first LOD0 group's actual+18; no material substitution.
        const auto lod=first_person_material_lod(visual);
        if(!lod)throw std::runtime_error("HUD info visual has no actual LOD0");
        const auto group=field<Frame>(lod,0x5c);
        if(!group)throw std::runtime_error("HUD info first group is missing");
        call<void>(group,0x18);
    }
    Frame hud_group_material(Frame lod,std::uint32_t index) { //004A8A90 skips a null material, unlike00492610.
        const auto group=call<Frame>(lod,0x14,index);
        if(!group)throw std::runtime_error("HUD material group is missing");
        return field<Frame>(group,4);
    }
    void hud_material_group18(Frame lod,std::uint32_t index) {
        const auto group=call<Frame>(lod,0x14,index);
        if(!group)throw std::runtime_error("HUD replacement material group is missing");
        call<void>(group,0x18);
    }
    ResourceOwner retain_hud_texture(Frame texture) { //004A9010 increments texture's32-bit14, not frame's16-bitE4.
        if(!texture)throw std::runtime_error("HUD texture retain requires actual texture");
        const auto count=field<std::uint32_t>(texture,0x14);
        if(count==0xffffffffu)throw std::runtime_error("HUD texture reference overflow");
        put(texture,0x14,count+1);return ResourceOwner(texture,ResourceRelease{module_});
    }
    void hud_frame_tag(Frame frame,std::uint32_t value) {put(frame,0xec,value);}
    void hud_visual_flags(Frame visual,std::uint32_t flags) {put(visual,0x1d0,field<std::uint32_t>(visual,0x1d0)|flags);}
    Frame hud_first_material(Frame visual) { //004B30E0 repeats LOD0 then reads first group's material without lazy texture resolution.
        const auto lod=first_person_material_lod(visual);
        return hud_first_material_from_lod(lod);
    }
    Frame hud_first_material_from_lod(Frame lod) { //004B4980 uses firstGROUP material AFTER a single LOD0 read.
        if(!lod)throw std::runtime_error("HUD soldier visual has no actual LOD0");
        const auto group=field<Frame>(lod,0x5c);
        if(!group)throw std::runtime_error("HUD soldier first group is missing");
        return field<Frame>(group,4);
    }
    void hud_unit_scale(Frame frame) {
        for(unsigned axis=0;axis!=3;++axis)put(frame,0xd0+axis*4,1.f);
        //004B30E0 copies an unwritten fourth stack lane to DC; preserve known padding.
        (void)car_rotation(frame);
        put(frame,0xe0,(field<std::uint32_t>(frame,0xe0)&0xfffffecbu)|0x40000008u);
    }
    bool volume_visible(Frame frame)const{return car_frame_visible(frame);}
    void volume_bind_texture(Frame texture){(void)graphics_.methods->bind_texture(&graphics_,texture,0);}
    void volume_set_dir3(Matrix& matrix,const Vector& forward,const Vector& up){
        using Set=void(WINAPI*)(void*,const void*,const void*);
        exported<Set>("?SetDir3@S_matrix@@QAGXABUS_vector@@0@Z",0x2cf10)(matrix.data(),forward.data(),up.data());
    }
    void volume_world_transform(const Matrix& matrix){
        if(graphics_.methods->set_world_transform(&graphics_,matrix.data())<0)throw std::runtime_error("Actual volumetric world transform failed");
    }
    void volume_graphics_state(std::uint32_t state,std::uint32_t value){graphics_.methods->set_state(&graphics_,state,value);}
    void volume_draw(const game::VolumetricVertex* vertices,std::uint32_t triangles){
        if(graphics_.methods->draw_primitive(&graphics_,3,triangles,vertices,0)<0)throw std::runtime_error("Actual volumetric draw failed");
        observe_data("execution","volumetric primitive drawn","triangle count",triangles);
    }
    std::uint32_t hud_driver_state(std::uint32_t state)const{return driver_.render_state(state);}
    void hud_driver_state(std::uint32_t state,std::uint32_t value){(void)driver_.set_render_state(state,value);}
    MenuDriver::Owner create_hud_info_model(){return driver_.create(9);}
    MenuDriver::Owner create_shot_model(){return driver_.create(9);}
    MenuDriver::Owner create_shot_surface(){return driver_.create(0x4a4f5250u);} //004546E0, PROJ driver extension.
    void shot_surface_texture(Frame frame,Frame texture){call<void>(frame,0x9c,texture);}
    void shot_surface_scale(Frame frame,float x,float y,float z){
        put(frame,0xd0,x);put(frame,0xd4,y);put(frame,0xd8,z); //Native DC is unwritten; retain its known prior lane.
        (void)car_rotation(frame);
        put(frame,0xe0,(field<std::uint32_t>(frame,0xe0)&0xfffffecbu)|0x40000008u);
    }
    void shot_surface_direction(Frame frame,const Vector& direction,float roll){call<void>(frame,0xc,direction.data(),roll);}
    void shot_surface_enabled(Frame frame,bool value){call<void>(frame,0xb4,std::uint32_t(value));}
    void shot_surface_link(Frame frame,Frame sector){call<void>(frame,0x38,sector,std::uint32_t{1});}
    void shot_surface_mode(Frame frame,std::uint32_t value){call<void>(frame,0xb8,value);}
    void shot_surface_colour(Frame frame,float amount){
        const Vector colour{{amount*field<float>(frame,0x1f0),amount*field<float>(frame,0x1f4),amount*field<float>(frame,0x1f8),0}};
        call<void>(frame,0xb0,colour.data());
    }
    std::int32_t load_shot_model(Frame model,const char* name){return resources_.load_model(model,name,0);} //004290A0 verbatim resource keys.
    Frame shot_hidden_sector()const{return field<Frame>(scene_.frame(),0x200);}
    void shot_query_performance_clock(){LARGE_INTEGER frequency{},counter{};(void)QueryPerformanceFrequency(&frequency);(void)QueryPerformanceCounter(&counter);} //0044DDE0 discards both results.
    ResourceOwner load_shot_texture(const std::string& name){
        Frame texture=nullptr;(void)graphics_.methods->load_texture(&graphics_,name.c_str(),nullptr,2,&texture);
        return ResourceOwner(texture,ResourceRelease{module_}); //0044E520 ignores HRESULT; own the ACTUAL output, even on nonzero status.
    }
    MenuDriver::Owner create_hud_scene(){return driver_.create(13);}
    void hud_render_scene(Frame scene){call<void>(scene,0x78,std::uint32_t{0});} //0049B9F0 Scene78(scene,0).
    MenuDriver::Owner create_hud_light(){return driver_.create(2);}
    void hud_fit_viewport(Frame scene,std::int32_t width,std::int32_t height) {
        if(field<std::int32_t>(scene,0x244)!=0||field<std::int32_t>(scene,0x248)!=0||
                field<std::int32_t>(scene,0x24c)!=width||field<std::int32_t>(scene,0x250)!=height)
            call<void>(scene,0x148,std::int32_t{0},std::int32_t{0},width,height);
    }
    void hud_active_camera(Frame scene,Frame camera){call<void>(scene,0x90,camera);}
    void hud_camera_direction(Frame camera){const std::array<float,4> direction{{0,0,1,0}};call<void>(camera,0x1c,direction.data(),std::uint32_t{0});}
    void hud_pointer_direction(Frame frame,const Vector& direction,float angle){call<void>(frame,0x1c,direction.data(),angle);}
    void hud_camera88(Frame camera,std::uint32_t value){call<void>(camera,0x88,value);}
    void hud_camera84(Frame camera,float value){call<void>(camera,0x84,value);}
    void hud_light_colour(Frame light,const std::array<float,4>& value){call<void>(light,0x84,value.data());}
    void hud_light_power(Frame light,float value){call<void>(light,0x90,value);}
    void hud_root_add_light(Frame scene,Frame light){add_light_sector(light,field<Frame>(scene,0x1f8));}
    std::int32_t load_hud_info_model(Frame model,const char* name,std::uint32_t flags){return resources_.load_model(model,name,flags);}
    std::int32_t load_hud_map_model(Frame model,const char* path) { //004A4AE0 bypasses category1 cache: direct model74, flags0.
        return call<std::int32_t>(model,0x74,path,std::uint32_t{0},static_cast<void*>(nullptr),static_cast<void*>(nullptr));
    }
    void hud_info_scale(Frame frame,float x,float y,float z) {
        put(frame,0xd0,x);put(frame,0xd4,y);put(frame,0xd8,z);
        //00496450 copies an unwritten fourth stack lane; preserve known DC.
        (void)car_rotation(frame);
        put(frame,0xe0,(field<std::uint32_t>(frame,0xe0)&0xfffffecbu)|0x40000008u);
    }
    void hud_inventory_slot_opacity(Frame frame,float value){call<void>(frame,0xa8,value);} //004A2010 uses visual A8, distinct from equipment A4.
    ResourceOwner load_item_icon_texture(const std::string& name){ //007DF650, success is EXACTLY zero.
        Frame texture=nullptr;const auto status=graphics_.methods->load_texture(&graphics_,name.c_str(),nullptr,0,&texture);
        ResourceOwner owner(texture,ResourceRelease{module_});if(status!=0)owner.reset();return owner;
    }
    ResourceOwner load_hud_death_texture() { //Original ignores loader result; required returned owner remains authoritative.
        Frame texture=nullptr;
        (void)graphics_.methods->load_texture(&graphics_,"x_fc_death.bmp","x_fc_a.bmp",4,&texture);
        return ResourceOwner(texture,ResourceRelease{module_});
    }
    void hud_message_position(Frame frame,float x,float y,float z) { //00495F40 writesXYZ only, preserving native positionBC.
        const auto flags=field<std::uint32_t>(frame,0xe0);
        if(flags&0x80u)return;
        put(frame,0xb0,x);put(frame,0xb4,y);put(frame,0xb8,z);
        put(frame,0xe0,(flags&0xfffffedfu)|0x40000000u);
    }
    std::array<float,12> hud_message_bounds(Frame frame) { //00495900 uses frame10 bounds, not HUD text world-bound170.
        if(!(field<std::uint32_t>(frame,0xe0)&0x100u))call<void>(frame,0x58);
        return field<std::array<float,12>>(frame,0x10);
    }
    void hud_material_flags(Frame material,std::uint32_t flags){put(material,8,field<std::uint32_t>(material,8)|flags);}
    void hud_material_vector(Frame material,std::uint32_t slot,const std::array<float,4>& value){call<void>(material,slot,value.data());}
    Frame hud_portrait_texture(void* portrait)const{return static_cast<Frame>(portrait);} //Typed HumanStore+20 resource borrower, no new original body.
    void hud_material14(Frame material,float value){call<void>(material,0x14,value);}
    void hud_visual_a4(Frame visual,float value){call<void>(visual,0xa4,value);}
    bool hud_frame_visible(Frame frame)const{return (field<std::uint32_t>(frame,0xe0)&1u)!=0;}
    void hud_crosshair_axis_position(Frame frame,unsigned axis,float value){ //004AA840 direct parent-position lane.
        const auto flags=field<std::uint32_t>(frame,0xe0);if(flags&0x80u)return;
        if(axis>1)throw std::logic_error("Invalid HUD crosshair position axis");
        put(frame,0xb0+axis*4,value);put(frame,0xe0,(flags&0xfffffedfu)|0x40000000u);
    }
    void hud_crosshair_name_scale(Frame frame) {
        for(unsigned axis=0;axis!=3;++axis)put(frame,0xd0+axis*4,2.f);
        //004A9740 copies an unwritten fourth stack lane; preserve known frame padding.
        (void)car_rotation(frame);
        put(frame,0xe0,(field<std::uint32_t>(frame,0xe0)&0xfffffecbu)|0x40000008u);
    }
    std::array<float,8> hud_scroll_bounds(Frame frame) {
        if(!(field<std::uint32_t>(frame,0xe0)&0x100u))call<void>(frame,0x58);
        return field<std::array<float,8>>(frame,0x10);
    }
    void hud_scroll_position(Frame frame,const std::array<float,4>& value){call<void>(frame,8,value.data());}
    MenuDriver::Owner create_hud_inventory_visual() {
        const auto visual=call<Frame>(driver_.frame(),0x50,std::uint32_t{0x5f4a424f});
        if(!visual)throw std::runtime_error("HUD dragging visual factory returned null");
        return MenuDriver::Owner(visual,scene_method<MenuDriver::Release>(visual,module_,0,0x1c370));
    }
    void hud_inventory_duplicate(Frame destination,Frame source){call<void>(destination,0x4c,source);}
    void hud_inventory_forward(Frame frame,const std::array<float,4>& direction){call<void>(frame,0x0c,direction.data(),std::uint32_t{0});}
    void hud_inventory_mesh18(Frame visual){
        const auto mesh=field<Frame>(visual,0x1e0);
        if(!mesh)throw std::runtime_error("HUD inventory visual mesh is missing");
        call<void>(mesh,0x18);
    }
    void hud_inventory_link_root(Frame frame,Frame scene){call<void>(frame,0x38,field<Frame>(scene,0x1f8),std::uint32_t{0});}
    void hud_inventory_add(Frame scene,Frame frame){call<void>(scene,0x80,frame);}
    Frame hud_active_camera(Frame scene) const {return field<Frame>(scene,0x15c);}
    std::array<float,2> hud_camera_clips(Frame camera) const {return {{field<float>(camera,0x124),field<float>(camera,0x128)}};}
    std::array<float,8> hud_text_world_bounds(Frame visual) {
        if(!(field<std::uint32_t>(visual,0x1d0)&0x40000u)) {
            using Update=void(WINAPI*)(Frame);
            exported<Update>("?UpdateWBoundProc@I3D_visual@@AAGXXZ",0x571b0)(visual);
        }
        return field<std::array<float,8>>(visual,0x170);
    }
    const void* hud_text_world_bounds_borrower(Frame visual){ //004BA390, original pointer78 borrows actual visual170 until HUD scene retirement.
        (void)hud_text_world_bounds(visual);return reinterpret_cast<const unsigned char*>(visual)+0x170;
    }
    Frame hud_find(Frame scene,const char* name,std::uint32_t mask) {return call<Frame>(scene,0x7c,name,mask);}
    Frame hud_first_texture(Frame visual) { //0049B730/0049B620 follow actual LOD0+5C first group.
        const auto lod=first_person_material_lod(visual);
        if(!lod)throw std::runtime_error("HUD artwork visual has no actual LOD0");
        const auto group=field<Frame>(lod,0x5c);
        if(!group)throw std::runtime_error("HUD artwork first material group is missing");
        const auto material=field<Frame>(group,4);
        if(!material)throw std::runtime_error("HUD artwork first material is missing");
        return first_person_material_texture(material);
    }
    game::MissionQuaternion first_person_neutral_rotation() {
        game::MissionQuaternion result{};const std::array<float,3> axis{{0,0,1}};
        using Make=void(WINAPI*)(void*,const void*,float);
        exported<Make>("?Make@S_quat@@QAGXABUS_vector@@M@Z",0x2f070)(result.data(),axis.data(),0.f);
        return result;
    }
    void first_person_model_restart(Frame model,std::uint32_t slot) {call<void>(model,0x90,slot);}
    void first_person_flash_opacity(Frame flash,float value) {call<void>(flash,0x50,value);}
    void first_person_flash_scale(Frame flash,float scale) {
        for(unsigned axis=0;axis!=3;++axis)put(flash,0xd0+axis*4,scale);
        //00492A90 copies an unwritten stack lane to DC; keep known padding.
        (void)car_rotation(flash);
        put(flash,0xe0,(field<std::uint32_t>(flash,0xe0)&0xfffffecbu)|0x40000008u);
    }
    void first_person_flash_rotation(Frame flash,float angle) {
        const std::array<float,3> forward{{0,0,1}};
        call<void>(flash,0x1c,forward.data(),angle);
    }
    Frame first_person_material_lod(Frame visual) {
        const auto mesh=field<Frame>(visual,0x1e0);
        if(!mesh)throw std::runtime_error("First-person hands visual mesh is missing");
        return call<Frame>(mesh,4,std::uint32_t{0});
    }
    std::uint32_t first_person_material_count(Frame lod) const {return field<std::uint32_t>(lod,0x58);}
    Frame first_person_group_material(Frame lod,std::uint32_t index) {
        const auto group=call<Frame>(lod,0x14,index);
        if(!group)throw std::runtime_error("First-person hands material group is missing");
        const auto material=field<Frame>(group,4);
        if(!material)throw std::runtime_error("First-person hands material is missing");
        return material;
    }
    Frame first_person_material_texture(Frame material) {
        if(field<std::uint32_t>(material,8)&0x04000000u)call<void>(material,0x44);
        return field<Frame>(material,0xc);
    }
    std::string first_person_texture_name(Frame texture) const {
        const auto* name=field<const char*>(texture,0x18);return name?name:"";
    }
    void first_person_material_texture(Frame material,Frame texture) {call<void>(material,0x1c,texture);}
    Frame first_person_scene_root() const {return field<Frame>(scene_.frame(),0x204);}
    void first_person_model_add(Frame model,Frame frame) {call<void>(model,0x7c,frame);}
    void first_person_model_remove(Frame model,Frame frame) {call<void>(model,0x80,frame);}
    void first_person_model_advance(Frame model,std::int32_t time) {call<void>(model,0x20,time);}
    void first_person_zero_position(Frame frame) { //00491130 writes only XYZ, unlike the four-word0041D110 helper.
        const auto flags=field<std::uint32_t>(frame,0xe0);
        if(!(flags&0x80u)) {
            for(unsigned axis=0;axis!=3;++axis)put(frame,0xb0+axis*4,0.f);
            put(frame,0xe0,(flags&0xfffffedfu)|0x40000000u);
        }
    }
    void first_person_forward(Frame frame) {
        const std::array<float,3> forward{{0,0,1}};
        call<void>(frame,0x1c,forward.data(),std::uint32_t{0});
    }
    MenuDriver::Owner create_item_template_model() {return driver_.create(9);}
    void configure_item_cigarette(Frame model) { //007D76C0, borrowed lensf lookup; +24 updates, it does not release.
        if(const auto lens=call<Frame>(model,0x48,"lensf",std::uint32_t{1})) {
            put(lens,0x1e4,.15f);put(lens,0x244,.35f);
            put(lens,0x1e0,1.f);put(lens,0x240,120.f);
            object_visible(lens,false);update_scene_object(lens);
        }
    }
    std::int32_t load_item_template_model(Frame model,const std::string& key) {
        return resources_.load_model(model,key,0); //007DF750 preserves the resource key verbatim.
    }
    Frame create_item_frame(Frame source) {
        const auto kind=frame_kind(source);
        auto owner=kind==1?create_visual(visual_type(source)):driver_.create(kind);
        return owner.release(); //007DC460, rather than ordinary single-frame release, owns it now.
    }
    void copy_item_frame(Frame destination,Frame source) {
        if(call<std::int32_t>(destination,0x4c,source)<0)
            throw std::runtime_error("Cannot copy required item model frame");
    }
    std::int16_t item_frame_references(Frame frame) const {return field<std::int16_t>(frame,0xe4);}
    void release_item_frame(Frame frame) {
        scene_method<MenuDriver::Release>(frame,module_,0,0x1c370)(frame);
    }
    Frame active_game_camera() const {return field<Frame>(scene_.frame(),0x15c);}
    Frame game_scene_find(const std::string& name,std::uint32_t flags){return scene_.find_frame(name,flags);} //Actual scene virtual7C, not Frame child48.
    float human_face_camera_parameter(Frame camera)const{return field<float>(camera,0x120);} //00781310's original distance gate.
    float sight_view_distance()const{ //00465DE0 via SAME scene+15C camera+128, not controller's saved240 frame.
        const auto camera=active_game_camera();if(!camera)throw std::runtime_error("Sight view distance requires actual active scene camera");
        return field<float>(camera,0x128);
    }
    bool script_scene_available()const{return scene_.frame()!=nullptr;} //0041D030's actual game18 borrower.
    MenuDriver::Owner static_weapon_shells(){MenuDriver::Owner result(nullptr,nullptr);if(resources_.cache(1).get("GunShells",result)!=0)result.reset();return result;} //004C76C0, model category1.
    void static_weapon_axis_angle(Frame frame,Vector& axis,float& angle){
        const auto q=actor_rotation(frame);using Inverse=void(WINAPI*)(const void*,void*,float*);
        exported<Inverse>("?Inverse@S_quat@@QBGXAAUS_vector@@AAM@Z",0x30160)(q.data(),axis.data(),&angle);
    }
    void static_weapon_restore_axis(Frame frame,const Vector& axis,float angle){
        game::MissionQuaternion q{};using Make=void(WINAPI*)(void*,const void*,float);
        exported<Make>("?Make@S_quat@@QAGXABUS_vector@@M@Z",0x2f070)(q.data(),axis.data(),angle);
        object_rotation(frame,q);update_scene_object(frame);
    }
    Vector static_weapon_world_scale(Frame frame){
        if(!actor_parent(frame))return actor_local_scale(frame);
        const auto matrix=actor_world_matrix(frame);Vector scale{};using Scale=Vector*(WINAPI*)(const void*,void*);
        exported<Scale>("?GetScale@S_matrix@@QBG?AUS_vector@@XZ",0x2db20)(matrix.data(),scale.data());return scale;
    }
    MenuDriver::Owner create_supply_helper(std::uint32_t kind){return driver_.create(kind);}
    std::int32_t load_supply_model(Frame model,const std::string& name,std::uint32_t flags){return resources_.load_model(model,name,flags);} //004290A0, verbatim key.
    MenuDriver::Owner load_supply_animation(const std::string& name){
        MenuDriver::Owner result(nullptr,nullptr);if(resources_.cache(3).get(name,result)!=0)result.reset();return result; //0041FC10.
    }
    void prepare_supply_morph(Frame morph){call<void>(morph,0xa0);}
    void supply_model_tick(Frame model,std::uint32_t elapsed){call<void>(model,0x20,elapsed);}
    MenuDriver::Owner create_fowl_model(){return driver_.create(9);}
    MenuDriver::Owner create_animation_actor_model(){return driver_.create(9);}
    std::int32_t load_animation_actor_model(Frame model,const std::string& name){return resources_.load_model(model,recovered::model_resource_key(name),0x10);}
    void mine_model_name(Frame frame,const char* name){call<void>(frame,0x34,name);} //005FDF90 ignores HRESULT.
    void animation_actor_name(Frame frame,const std::string& name){if(call<std::int32_t>(frame,0x34,name.c_str())<0)throw std::runtime_error("Cannot name Animation helper model");}
    void animation_actor_scale(Frame frame,const Vector& scale){
        put(frame,0xd0,scale);
        if(!(field<std::uint32_t>(frame,0xe0)&8u)) {
            using Make=void(WINAPI*)(void*,const void*);
            exported<Make>("?Make@S_quat@@QAGXABUS_matrix@@@Z",0x2f220)(address(frame,0xc0),address(frame,0x80));
        }
        put(frame,0xe0,(field<std::uint32_t>(frame,0xe0)&0xfffffecbu)|0x40000008u);
    }
    template<class Visit>void enumerate_animation_actor_materials(Frame model,Visit&& visit){
        using Visitor=std::remove_reference_t<Visit>;
        struct Context {
            Visitor& visit;std::exception_ptr failure;
            static std::int32_t WINAPI invoke(Frame frame,void* opaque)noexcept {
                auto& context=*static_cast<Context*>(opaque);
                try{return context.visit(frame)?2:0;}catch(...){context.failure=std::current_exception();return 2;}
            }
        } context{visit,{}};
        using Enumerate=std::int32_t(WINAPI*)(Frame,std::int32_t(WINAPI*)(Frame,void*),void*,std::uint32_t,const char*);
        scene_method<Enumerate>(model,module_,0x44,0x1de40)(model,&Context::invoke,&context,0xffff,nullptr);
        if(context.failure)std::rethrow_exception(context.failure);
    }
    std::int32_t load_fowl_template(Frame model,const std::string& name){return resources_.load_model(model,recovered::model_resource_key(name),0x10);}
    void clone_fowl_model(Frame model,Frame source){if(call<std::int32_t>(model,0xd8,source)<0)throw std::runtime_error("Cannot clone required Fowl model");}
    void add_fowl_model(Frame model){call<void>(scene_.frame(),0x80,model);}
    void fowl_name(Frame model,const char* name){if(call<std::int32_t>(model,0x34,name)<0)throw std::runtime_error("Cannot name generated Fowl model");}
    void fowl_wing_rotations(Frame first,Frame second,float angle){
        auto rotation=door_axis_rotation({{0,0,1,0}},angle);object_rotation(first,rotation);
        rotation[3]=-rotation[3];object_rotation(second,rotation);
    }
    Frame camera_root_sector()const{return field<Frame>(scene_.frame(),0x1f8);}
    void effects_enable_loading_sectors(){ //004885D0, root THEN hidden sector94(1).
        for(const auto offset:{0x1f8u,0x200u}){
            const auto sector=field<Frame>(scene_.frame(),offset);
            if(!sector)throw std::runtime_error("Effects loading requires its actual scene sectors");
            call<void>(sector,0x94,std::uint32_t{1});
        }
    }
    void effects_link_root(Frame frame){actor_link_preserving_world(frame,camera_root_sector());}
    void effects_scene_water(){call<void>(scene_.frame(),0x11c,std::uint32_t{0},std::uint32_t{0});}
    void effects_restore_water_children(bool visible){ //00486520, actual scene1FC child methods3C/40, reverse order.
        const auto sector=field<Frame>(scene_.frame(),0x1fc);
        if(!sector)throw std::runtime_error("Water visibility requires its actual scene sector");
        for(auto n=call<std::uint32_t>(sector,0x3c);n;){
            const auto child=call<Frame>(field<Frame>(scene_.frame(),0x1fc),0x40,--n);
            object_visible(child,visible);
        }
    }
    std::array<float,3> effects_active_camera_parameters()const{
        const auto camera=field<Frame>(scene_.frame(),0x15c);
        if(!camera)throw std::runtime_error("Water loading requires the actual active scene camera");
        return {{field<float>(camera,0x120),field<float>(camera,0x124),field<float>(camera,0x128)}};
    }
    std::string camera_sector_properties(Frame sector)const{return car_user_data(sector);} //0046C370 uses Frame+F0 text.
    void active_game_camera(Frame camera) {
        using Active=void(WINAPI*)(Frame,Frame);
        scene_method<Active>(scene_.frame(),module_,0x90,0x471a0)(scene_.frame(),camera);
    }
    float game_camera_fov_value(Frame camera){return field<float>(camera,0x120);} //00465E80.
    float game_camera_parameter78_value(Frame camera){return field<float>(camera,0x12c);} //00465E30.
    void game_camera_fov(Frame camera,float value) {
        using Fov=void(WINAPI*)(Frame,float);
        scene_method<Fov>(camera,module_,0x74,0x09e30)(camera,value);
    }
    void game_camera_range(Frame camera,float near_clip,float far_clip) {
        using Range=void(WINAPI*)(Frame,float,float);
        scene_method<Range>(camera,module_,0x7c,0x09ef0)(camera,near_clip,far_clip);
    }
    void game_camera_parameter78(Frame camera,float value) {call<void>(camera,0x78,value);}
    void game_camera_parameter8c(Frame camera,float value) {call<void>(camera,0x8c,value);}
    void game_camera_parameter80(Frame camera,float first,float second) {call<void>(camera,0x80,first,second);}
    void game_camera_name(Frame camera,const char* name) {
        if(call<std::int32_t>(camera,0x34,name)<0)throw std::runtime_error("Cannot name game camera");
    }
    std::array<float,2> vertex_camera_projection(Frame camera) const {
        return {{field<float>(camera,0x120),field<float>(camera,0x138)}};
    }
    std::uint32_t vertex_frame_epoch(Frame frame) const {return field<std::uint32_t>(frame,0x11c);}
    std::uint32_t vertex_scene_epoch() const {return field<std::uint32_t>(scene_.frame(),0x11c);}
    std::int32_t vertex_animation_lod(Frame frame) const {
        return (field<std::uint32_t>(frame,0x1d0)&0x10)?0:field<std::int32_t>(frame,0x1e4);
    }
    template<class Write>void write_vertex_animation(Frame frame,std::uint32_t index,std::uint32_t count,Write&& write) {
        const auto mesh=field<Frame>(frame,0x1e0);
        if(!mesh||index>=field<std::uint32_t>(mesh,0x44))throw std::runtime_error("Missing vertex animation LOD");
        const auto lod=call<Frame>(mesh,4,index);
        if(!lod||field<std::uint32_t>(lod,0x64)!=32||count>field<std::uint32_t>(lod,0x54))
            throw std::runtime_error("Vertex animation disagrees with actual mesh format/count");
        auto* data=call<unsigned char*>(lod,4,std::uint32_t{0});
        {
            struct Unlock {
                Frame lod;HMODULE module;
                ~Unlock() {using Method=void(WINAPI*)(Frame);checked_engine_method<Method>(lod,module,8)(lod);}
            } unlock{lod,module_};
            if(count&&!data)throw std::runtime_error("Cannot lock vertex animation mesh");
            for(std::uint32_t vertex=0;vertex!=count;++vertex)write(vertex,data+vertex*32);
        }
        call<void>(frame,0xb0); //00473960 invalidates the visual after unlocking.
    }
    float volume_random_unit() {
        using Random=float(__cdecl*)();
        return exported<Random>("RandFloat",0x2c450)();
    }
    std::uint32_t actor_child_count(Frame frame) {
        using Count=std::uint32_t(WINAPI*)(Frame);
        return scene_method<Count>(frame,module_,0x3c,0x1db80)(frame);
    }
    Frame actor_child(Frame frame,std::uint32_t index) {
        using Child=Frame(WINAPI*)(Frame,std::uint32_t);
        return scene_method<Child>(frame,module_,0x40,0x1dba0)(frame,index);
    }
    Vector door_sphere_centre(Frame frame) {
        if(!(field<std::uint32_t>(frame,0xe0)&0x100))call<void>(frame,0x58);
        return field<Vector>(frame,0x30);
    }
    std::string actor_short_name(Frame frame)const {
        const auto* full=engine_frame_name(frame);if(!full)return {};
        const auto offset=field<std::int16_t>(frame,0xe6);
        if(offset<0||static_cast<std::size_t>(offset)>std::strlen(full))throw std::runtime_error("Invalid actor short-name offset");
        return full+offset;
    }
    Frame door_presentation_sector()const{return field<Frame>(scene_.frame(),0x200);}
    bool door_recently_visible(Frame frame)const{return field<std::uint32_t>(scene_.frame(),0x11c)-field<std::uint32_t>(frame,0x11c)<10u;}
    std::array<float,4> door_axis_rotation(const Vector& axis,float angle) {
        std::array<float,4> rotation{};using Make=void(WINAPI*)(void*,const void*,float);
        exported<Make>("?Make@S_quat@@QAGXABUS_vector@@M@Z",0x2f070)(rotation.data(),axis.data(),angle);return rotation;
    }
    void door_roll_handle(Frame frame,float angle) {
        Vector direction;
        if(!(field<std::uint32_t>(frame,0xe0)&0x10u))direction=car_forward(frame);
        else direction=game::door_handle_direction(field<Vector>(frame,0xa0));
        call<void>(frame,0x1c,direction.data(),angle); //Actual roll FLOAT, not an up-vector pointer.
    }
    void door_portal_closed(Frame portal,bool closed){call<void>(portal,0,std::uint32_t(closed));}
    void door_portal_attachment_closed(std::uintptr_t portal,bool closed){door_portal_closed(reinterpret_cast<Frame>(portal),closed);}
    Frame door_sector(const Vector& position) {return call<Frame>(scene_.frame(),0xe0,position.data());}
    std::uint32_t door_portal_count(Frame sector,unsigned side) const {return field<std::uint32_t>(sector,side?0x264:0x254);}
    Frame door_portal(Frame sector,unsigned side,std::uint32_t index) const {
        if(index>=door_portal_count(sector,side))throw std::out_of_range("Door portal index");
        const auto list=field<Frame*>(sector,side?0x25c:0x24c);
        if(!list)throw std::runtime_error("Door sector portal list is missing");
        return list[index];
    }
    Vector door_portal_centre(Frame frame) const {return field<Vector>(frame,0x180);}
    float door_portal_radius(Frame frame) const {return field<float>(frame,0x190);}
    std::uintptr_t door_portal_attachment(Frame frame) const {return field<std::uintptr_t>(frame,0x19c);}
    game::MissionQuaternion actor_world_rotation(Frame frame) {
        const auto world=actor_world_matrix(frame);game::MissionQuaternion result{};
        using Make=void(WINAPI*)(void*,const void*);
        exported<Make>("?Make@S_quat@@QAGXABUS_matrix@@@Z",0x2f220)(&result,world.data());
        return result;
    }
    Bounds actor_bounds(Frame frame) {return car_bounds(frame);}
    template<class Visit> void actor_vertices(Frame visual,Visit&& visit) {
        car_strip_vertices(visual,std::forward<Visit>(visit)); //Same LOD0 lock/32-byte vertex ABI.
    }
};

} //namespace hd2::host
#endif
