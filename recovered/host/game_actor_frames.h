#ifndef HD2_HOST_GAME_ACTOR_FRAMES_H
#define HD2_HOST_GAME_ACTOR_FRAMES_H

#include "game/mission_script_rotations.h"
#include "game/door_rotation_loading.h"

#include "host/game_car_frames.h"
#include "game/human_face_mesh_pool.h"
#include "game/mission_script_frame_snapshots.h"

namespace hd2::host {

//Shared real frame endpoints for actor initialization. Entry registries,
//script contexts and concrete actor dispatch belong to the containing world.
class GameActorFrames : public GameCarFrames {
public:
    using GameCarFrames::GameCarFrames;
    MenuDriver::Owner retain_actor_frame(Frame frame) {return retain(frame);}
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
    void actor_link_preserving_world(Frame frame,Frame parent) {
        if(call<std::int32_t>(frame,0x38,parent,std::uint32_t{1})<0)
            throw std::runtime_error("Cannot preserve actor pose while linking its parent");
    }
    void physics_link_root(Frame frame) { //0040B5E0 preserves world pose while reparenting.
        const auto root=field<Frame>(scene_.frame(),0x1f8);
        if(call<std::int32_t>(frame,0x38,root,std::uint32_t{1})<0)
            throw std::runtime_error("Cannot link physics object to actual scene root");
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
    Matrix rigid_matrix_product(const Matrix& first,const Matrix& second) {
        Matrix result{};
        using Product=Matrix*(WINAPI*)(const void*,void*,const void*);
        exported<Product>("??DS_matrix@@QBG?AU0@ABU0@@Z",0x2d830)(first.data(),result.data(),second.data());
        return result;
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
    game::MissionQuaternion human_matrix_rotation(const Matrix& matrix) {
        game::MissionQuaternion result{};
        using Make=void(WINAPI*)(void*,const void*);
        exported<Make>("?Make@S_quat@@QAGXABUS_matrix@@@Z",0x2f220)(result.data(),matrix.data());
        return result;
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
    void human_equipment_local_matrix(Frame frame,const Matrix& matrix) { //0078DF60's repeated matrix assignment.
        put(frame,0x80,matrix);Vector scale{};
        using Scale=Vector*(WINAPI*)(const void*,void*);
        exported<Scale>("?GetScale@S_matrix@@QBG?AUS_vector@@XZ",0x2db20)(address(frame,0x80),scale.data());
        for(unsigned axis=0;axis!=3;++axis)put(frame,0xd0+axis*4,scale[axis]); //DC is unwritten SIMD padding.
        put(frame,0xe0,(field<std::uint32_t>(frame,0xe0)&0xffffffd7u)|0x40000014u);
    }
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
    std::uint32_t human_model_animation_flags(Frame model,std::uint32_t slot) {return call<std::uint32_t>(model,0xa8,slot);}
    void human_model_animation_flags(Frame model,std::uint32_t slot,std::uint32_t flags) {call<void>(model,0xa4,slot,flags);}
    bool human_model_animation_enabled(Frame model,std::uint32_t slot) {return call<std::uint8_t>(model,0xac,slot)!=0;}
    void human_model_animation(Frame model,Frame clip,std::uint32_t slot,float weight,std::uint32_t flags) {
        call<void>(model,0x84,clip,slot,weight,flags);
    }
    void human_model_advance(Frame model,const std::array<std::int32_t,8>& times) {call<void>(model,0xb0,times.data());}
    void human_model_remove_animation(Frame model,std::uint32_t slot) {call<void>(model,0x8c,slot);}
    void human_model_parameter_c4(Frame model,std::uint32_t value) {call<void>(model,0xc4,value);}
    std::int32_t human_clip_duration(Frame clip) {return call<std::int32_t>(clip,8);}
    void human_clip_word8(Frame clip,std::uint32_t value) {put(clip,8,value);}
    Frame human_scene_sector(const Vector& position) {return call<Frame>(scene_.frame(),0xe0,position.data());}
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
    std::int32_t load_human_holster_model(Frame model,const char* key) {return resources_.load_model(model,key,0x10);}
    void remove_human_holster_frame(Frame model,Frame frame) {call<void>(model,0x80,frame);}
    void human_equipment_opacity(Frame frame,float opacity) {call<void>(frame,0xa4,opacity);}
    void human_equipment_name(Frame frame,const std::string& name) {
        if(call<std::int32_t>(frame,0x34,name.c_str())<0)throw std::runtime_error("Cannot name Human equipment");
    }
    void effects_model_opacity(Frame model,float opacity) {call<void>(model,0x50,opacity);} //00486DF0, distinct from OBJ_+A4.
    void effects_frame_fade_alpha(Frame frame,float alpha){call<void>(frame,0xa8,alpha);} //00486E60, distinct from both model+50 and visual+A4.
    void effects_model_camera_amount(Frame model,float value) {call<void>(model,0x54,value);} //00486D30; distinct from model+50 opacity.
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
    std::array<float,3> human_light_colour(Frame light) const {return field<std::array<float,3>>(light,0x130);}
    float human_light_multiplier(Frame light) const {return field<float>(light,0x168);}
    std::uint32_t human_frame_epoch(Frame frame) const {return vertex_frame_epoch(frame);}
    std::uint32_t human_scene_epoch() const {return vertex_scene_epoch();}
    Vector collision_sweep_rotate(const Vector& value,const Matrix& matrix) {
        Vector result{};
        using Rotate=Vector*(WINAPI*)(const void*,void*,const void*);
        exported<Rotate>("?RotateByMatrix@S_vector@@QBG?AU1@ABUS_matrix@@@Z",0x2ec80)(value.data(),result.data(),matrix.data());
        return result;
    }
    MenuDriver::Owner create_game_camera() {return driver_.create(3);}
    MenuDriver::Owner create_human_sound() {return driver_.create(4);}
    bool human_sound_available() {
        using Get=Frame(__cdecl*)();const auto driver=exported<Get>("ISndGetDriver",0x68fa0)();
        using Available=std::uint8_t(WINAPI*)(Frame);
        return scene_method<Available>(driver,module_,0x0c,0x7a840)(driver)!=0;
    }
    std::int32_t load_human_sound(Frame sound,const char* name) {return resources_.load_sound(sound,name,0);}
    void human_sound_enabled(Frame sound,bool enabled) {call<void>(sound,0x30,std::uint32_t(enabled));}
    void human_sound_parameter7c(Frame sound,std::uint32_t value) {call<void>(sound,0x7c,value);}
    void human_sound_volume(Frame sound,float value) {call<void>(sound,0x8c,value);}
    void human_sound_range(Frame sound,float near_range,float far_range,float first,float second) {call<void>(sound,0x80,near_range,far_range,first,second);}
    void human_sound_parameter90(Frame sound,std::uint32_t value) {call<void>(sound,0x90,value);}
    void human_sound_parameter94(Frame sound,std::uint32_t value) {call<void>(sound,0x94,value);}
    void human_sound_tag(Frame sound,std::uint32_t value) {put(sound,0xec,value);}
    std::uint32_t human_sound_tag(Frame sound) const {return field<std::uint32_t>(sound,0xec);}
    void human_sound_detach(Frame sound) {call<void>(sound,0x38,Frame{},std::uint32_t{0});}
    void human_sound_missing(const char* name,std::int32_t) {std::fprintf(stderr,"AESound :%s : not found\n",name);}
    void hud_injury_prepare(Frame visual) {call<void>(visual,0xa0);}
    void hud_position(Frame frame,const std::array<float,3>& position) {call<void>(frame,8,position.data());}
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
    std::uint32_t hud_driver_state(std::uint32_t state)const{return driver_.render_state(state);}
    void hud_driver_state(std::uint32_t state,std::uint32_t value){(void)driver_.set_render_state(state,value);}
    MenuDriver::Owner create_hud_info_model(){return driver_.create(9);}
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
    void hud_material14(Frame material,float value){call<void>(material,0x14,value);}
    void hud_visual_a4(Frame visual,float value){call<void>(visual,0xa4,value);}
    bool hud_frame_visible(Frame frame)const{return (field<std::uint32_t>(frame,0xe0)&1u)!=0;}
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
    std::string camera_sector_properties(Frame sector)const{return car_user_data(sector);} //0046C370 uses Frame+F0 text.
    void active_game_camera(Frame camera) {
        using Active=void(WINAPI*)(Frame,Frame);
        scene_method<Active>(scene_.frame(),module_,0x90,0x471a0)(scene_.frame(),camera);
    }
    void game_camera_fov(Frame camera,float value) {
        using Fov=void(WINAPI*)(Frame,float);
        scene_method<Fov>(camera,module_,0x74,0x09e30)(camera,value);
    }
    void game_camera_range(Frame camera,float near_clip,float far_clip) {
        using Range=void(WINAPI*)(Frame,float,float);
        scene_method<Range>(camera,module_,0x7c,0x09ef0)(camera,near_clip,far_clip);
    }
    void game_camera_parameter78(Frame camera,float value) {call<void>(camera,0x78,value);}
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
