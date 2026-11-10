#ifndef HD2_HOST_GAME_CAR_FRAMES_H
#define HD2_HOST_GAME_CAR_FRAMES_H

#include "host/game_scene_objects.h"
#include "game/car_geometry_math.h"
#include "game/car_particles.h"

namespace hd2::host {

//Real LS3DF endpoints required by004E2E30 and its geometry helpers. This is a
//resource/transform bridge, not a successful replacement for Car construction.
class GameCarFrames : public GameSceneObjects {
public:
    using GameSceneObjects::GameSceneObjects;
    using Vector = std::array<float,4>;
    using Matrix = std::array<float,16>;
    struct Bounds {Vector minimum{},maximum{};};
    struct ResourceRelease {
        HMODULE module=nullptr;
        void operator()(Frame frame) const noexcept {
            if(!frame)return;
            using Release=void(WINAPI*)(Frame);
            checked_engine_method<Release>(frame,module,0)(frame);
        }
    };
    using ResourceOwner=std::unique_ptr<EngineFrame,ResourceRelease>;
    struct CarProjector {
        MenuDriver::Owner visual{nullptr,nullptr},light{nullptr,nullptr};
        std::array<ResourceOwner,3> textures{};
    };

    std::vector<Frame> car_parts(Frame model,const char* pattern,std::uint32_t mask) {
        PartEnumeration context;
        using Enumerate=std::int32_t(WINAPI*)(Frame,std::int32_t(WINAPI*)(Frame,void*),void*,std::uint32_t,const char*);
        scene_method<Enumerate>(model,module_,0x44,0x1de40)(model,&collect_part,&context,mask,pattern);
        //Original ignores enumeration's result, including80000006. Native
        //callback exceptions must still unwind rather than escape its C ABI.
        if(context.failure)std::rethrow_exception(context.failure);
        return std::move(context.frames);
    }
    Frame car_part(Frame model,const char* pattern,std::uint32_t mask) {
        auto matches=car_parts(model,pattern,mask);
        return matches.empty()?nullptr:matches.back(); //004F9CB0 writes every match, returns0.
    }
    MenuDriver::Owner retain_car_frame(Frame frame) {return retain(frame);}
    std::string car_user_data(Frame frame) const {
        const auto* data=field<const char*>(frame,0xf0);
        return data?data:"";
    }
    std::string car_model_resource(Frame model) const {
        const auto* text=field<const char*>(model,0x134);return text?text:"";
    }
    std::vector<Frame> car_model_frames(Frame model) const {
        if(frame_kind(model)!=9)throw std::runtime_error("Car ancillary frame table requires a model");
        const auto count=field<std::uint32_t>(model,0x128);
        const auto table=field<Frame*>(model,0x120);
        if(count&&!table)throw std::runtime_error("Car ancillary model frame table is missing");
        if(count>0x100000)throw std::runtime_error("Car ancillary model frame table exceeds bounds");
        return count?std::vector<Frame>(table,table+count):std::vector<Frame>{};
    }
    std::string car_short_name(Frame frame) const {
        const auto name=frame_name(frame);
        const auto offset=field<std::int16_t>(frame,0xe6);
        if(offset<0||static_cast<std::size_t>(offset)>name.size())throw std::runtime_error("Invalid Car frame short-name offset");
        return name.substr(static_cast<std::size_t>(offset));
    }
    MenuDriver::Owner create_car_ancillary_model() {return driver_.create(9);}
    bool load_car_ancillary_model(Frame model,const std::string& name) {
        return resources_.load_model(model,recovered::model_resource_key(name),0)==0;
    }
    Vector car_position(Frame frame) const {return field<Vector>(frame,0xb0);}
    Frame car_find(Frame frame,const char* pattern,std::uint32_t mask) {
        return call<Frame>(frame,0x48,pattern,mask);
    }
    Vector car_forward(Frame frame) {
        const auto flags=field<std::uint32_t>(frame,0xe0);
        if(!(flags&0x10)) {
            put(frame,0xe0,flags|0x10u);
            using Normalize=void(WINAPI*)(void*);
            exported<Normalize>("?Normalize@S_quat@@QAGXXZ",0x30540)(address(frame,0xc0));
            using Set=void(WINAPI*)(void*,const void*);
            exported<Set>("?SetRot3@S_matrix@@QAGXABUS_quat@@@Z",0x2d450)(address(frame,0x80),address(frame,0xc0));
            return field<Vector>(frame,0xa0);
        }
        return game::car_normalize(field<Vector>(frame,0xa0));
    }
    game::MissionQuaternion car_rotation(Frame frame) {
        auto flags=field<std::uint32_t>(frame,0xe0);
        if(!(flags&8)) {
            using Make=void(WINAPI*)(void*,const void*);
            exported<Make>("?Make@S_quat@@QAGXABUS_matrix@@@Z",0x2f220)(address(frame,0xc0),address(frame,0x80));
            put(frame,0xe0,flags|8u);
        }
        return field<game::MissionQuaternion>(frame,0xc0);
    }
    Matrix car_local_matrix(Frame frame) { //Complete ordinary004C8140 body.
        const auto flags=field<std::uint32_t>(frame,0xe0);
        if(!(flags&0x10)) {
            put(frame,0xe0,flags|0x14u);
            using Set=void(WINAPI*)(void*,const void*);
            exported<Set>("?SetRot3@S_matrix@@QAGXABUS_quat@@@Z",0x2d450)(address(frame,0x80),address(frame,0xc0));
        } else {
            if(flags&4)return field<Matrix>(frame,0x80);
            if(!(flags&8)) {
                using Make=void(WINAPI*)(void*,const void*);
                exported<Make>("?Make@S_quat@@QAGXABUS_matrix@@@Z",0x2f220)(address(frame,0xc0),address(frame,0x80));
            }
            put(frame,0xe0,field<std::uint32_t>(frame,0xe0)|0xcu);
        }
        for(std::size_t row=0;row!=3;++row) {
            const auto scale=field<float>(frame,0xd0+row*4);
            for(std::size_t column=0;column!=3;++column) {
                const auto offset=0x80+row*16+column*4;
                put(frame,offset,field<float>(frame,offset)*scale);
            }
        }
        return field<Matrix>(frame,0x80);
    }
    void car_direction(Frame frame,const Vector& direction) {
        call<void>(frame,0x1c,direction.data(),static_cast<const float*>(nullptr));
    }
    Matrix car_world_matrix(Frame frame) {
        if(!(field<std::uint32_t>(frame,0xe0)&0x20)) {
            using Update=void(__thiscall*)(Frame);
            exported<Update>("?UpdateWMatrixProc@I3D_frame@@AAEXXZ",0x1e390)(frame);
        }
        return field<Matrix>(frame,0x40);
    }
    Matrix car_inverse_matrix(const Matrix& value) {
        Matrix result{};
        using Inverse=std::uint8_t(WINAPI*)(void*,const void*);
        if(!exported<Inverse>("?Inverse@S_matrix@@QAG_NABU1@@Z",0x2c620)(result.data(),value.data()))
            throw std::runtime_error("Singular Car geometry transform");
        return result;
    }
    Vector car_transform(const Vector& point,const Matrix& matrix) {
        Vector result{};
        using Transform=Vector*(WINAPI*)(const void*,void*,const void*);
        exported<Transform>("??DS_vector@@QBG?AU0@ABUS_matrix@@@Z",0x2eab0)(point.data(),result.data(),matrix.data());
        return result;
    }
    void car_transform_in_place(Vector& value,const Matrix& matrix) {
        using Transform=Vector*(WINAPI*)(void*,const void*);
        exported<Transform>("??XS_vector@@QAGAAU0@ABUS_matrix@@@Z",0x2ea40)(value.data(),matrix.data());
    }
    float car_vector_angle(const Vector& first,const Vector& second) {
        using Angle=float(WINAPI*)(const void*,const void*);
        return exported<Angle>("?AngleTo@S_vector@@QBGMABU1@@Z",0x2e8c0)(first.data(),second.data());
    }
    bool car_frame_visible(Frame frame) const {return (field<std::uint32_t>(frame,0xe0)&1)!=0;}
    Vector car_scale(Frame frame) {
        if(!object_parent(frame))return field<Vector>(frame,0xd0);
        const auto matrix=car_world_matrix(frame);Vector result{};
        using Scale=Vector*(WINAPI*)(const void*,void*);
        exported<Scale>("?GetScale@S_matrix@@QBG?AUS_vector@@XZ",0x2db20)(matrix.data(),result.data());
        return result;
    }
    Bounds car_bounds(Frame frame) {
        Bounds result;
        using Get=Bounds*(WINAPI*)(Frame,Bounds*,Frame);
        scene_method<Get>(driver_.frame(),module_,0xac,0x1ae10)(driver_.frame(),&result,frame);
        return result;
    }
    Bounds car_dummy_bounds(Frame frame) const {
        if(frame_kind(frame)!=6)throw std::runtime_error("Car collision bounds require a real dummy frame");
        return field<Bounds>(frame,0x120);
    }
    Frame prepare_car_deform_mesh(Frame visual, bool glass) {
        auto* source=field<Frame>(visual,0x1e0);
        if(!source)throw std::runtime_error("Deformable Car visual has no mesh");
        auto mesh=own_car_resource(call<Frame>(driver_.frame(),0x6c));
        if(!mesh)throw std::runtime_error("Car deformable mesh factory failed");
        if(call<std::int32_t>(mesh.get(),0xc,source)!=0)
            throw std::runtime_error("Cannot copy Car deformable mesh");
        call<void>(visual,0x9c,mesh.get(),std::uint32_t{0});
        auto* assigned=mesh.get();
        //004FA520 releases its local reference after visual assignment. The
        //visual retains the mesh subsequently used for glass material setup.
        mesh.reset();
        if(glass) {
            auto* lod=call<Frame>(assigned,4,std::uint32_t{0});
            auto* material_slot=call<Frame>(lod,0x14,std::uint32_t{0});
            auto* original=field<Frame>(material_slot,4);
            auto material=own_car_resource(call<Frame>(driver_.frame(),0x58));
            if(!material)throw std::runtime_error("Car glass material factory failed");
            call<void>(material.get(),0x20,original);
            call<void>(material_slot,0,material.get());
        }
        return assigned;
    }
    Frame car_mesh(Frame visual) const {return field<Frame>(visual,0x1e0);}
    void car_projector_scale(Frame frame,const Vector& scale) { //004F4E40, no frame24 call.
        for(unsigned axis=0;axis!=3;++axis)put(frame,0xd0+axis*4,scale[axis]);
        car_rotation(frame);
        put(frame,0xe0,(field<std::uint32_t>(frame,0xe0)&0xfffffecbu)|0x40000008u);
    }
    CarProjector car_projector(Frame marker,const std::array<std::string,3>& names,
                              std::size_t count,const Vector& light_direction) {
        if(count!=1&&count!=3)throw std::logic_error("Invalid Car projector texture count");
        CarProjector result;
        result.visual=create_visual(0x4a4f5250); //004E87C0/004E8D80, actual PROJ factory.
        const auto world=car_world_matrix(marker);
        auto* visual=result.visual.get();
        object_position(visual,{world[12],world[13],world[14]});
        car_direction(visual,game::car_normalize(Vector{world[8],world[9],world[10],0}));
        object_scale(visual,{8,5,20});
        const Vector colour{.5f,.5f,.5f,0};call<void>(visual,0xb0,colour.data());
        call<void>(visual,0xb4,std::uint32_t{4});link_scene_object(visual,nullptr);
        for(std::size_t i=0;i<count;++i) {
            Frame texture=nullptr;
            const auto status=graphics_.methods->load_texture(&graphics_,names[i].c_str(),"",0,&texture);
            auto owned=own_car_resource(texture);
            if(status!=0) {
                //The single-texture helper keeps the untextured projector;
                //the three-texture helper releases everything and returns null.
                if(count==3)return {};
            } else if(count==1)call<void>(visual,0x9c,texture);
            else result.textures[i]=std::move(owned);
        }
        call<void>(visual,0xa4,std::uint32_t{0});object_visible(visual,false);update_scene_object(visual);
        result.light=driver_.create(2);
        if(result.light) {
            auto* light=result.light.get();
            object_position(light,{world[12],world[13],world[14]});
            car_direction(light,light_direction);
            call<void>(light,0x74,std::uint32_t{2});call<void>(light,0x94,std::uint32_t{0x11});
            call<void>(light,0x88,2.f);call<void>(light,0x98,60.f);
            call<void>(light,0xa4,.5199999809265137f,1.2200000286102295f);
            call<void>(light,0x78,.7689999938011169f,.675000011920929f,.5410000085830688f);
            link_scene_object(light,nullptr);object_visible(light,false);update_scene_object(light);
        }
        return result;
    }
    MenuDriver::Owner create_car_particle_frame() {return create_visual(0x52544d45);}
    ResourceOwner load_car_glass_texture(const std::string& primary,const std::string& secondary) {
        Frame texture=nullptr;
        //004FE850 passes the actual inline secondary pointer, even when empty.
        const auto result=graphics_.methods->load_texture(&graphics_,primary.c_str(),secondary.c_str(),4,&texture);
        auto owner=own_car_resource(texture);
        if(result!=0)return ResourceOwner(nullptr,ResourceRelease{module_});
        return owner;
    }
    ResourceOwner load_car_particle_texture(const std::string& primary,const std::string& secondary,unsigned flags) {
        Frame texture=nullptr;
        const auto result=graphics_.methods->load_texture(&graphics_,primary.c_str(),secondary.empty()?nullptr:secondary.c_str(),flags,&texture);
        auto owner=own_car_resource(texture);
        if(result!=0)return ResourceOwner(nullptr,ResourceRelease{module_});
        return owner;
    }
    void car_particle_texture(Frame frame,Frame texture) {
        using Method=void(WINAPI*)(Frame,Frame);
        scene_method<Method>(frame,module_,0x9c,0x3af50)(frame,texture);
    }
    void car_particle_name(Frame frame,const char* name) {call<void>(frame,0x34,name);}
    void car_particle_unit_scale(Frame frame) {object_scale(frame,{1,1,1});}
    void reset_car_particle(Frame frame) {
        using Method=void(WINAPI*)(Frame);
        scene_method<Method>(frame,module_,0xa8,0x3b8a0)(frame);
    }
    void car_particle_property(Frame frame,unsigned index,std::uint32_t value) {
        using Method=std::int32_t(WINAPI*)(Frame,std::uint32_t,std::uint32_t);
        if(scene_method<Method>(frame,module_,0xa0,0x3af80)(frame,index,value)<0)
            throw std::runtime_error("Cannot set Car emitter property");
    }
    void car_particle_vector_property(Frame frame,unsigned index,const Vector& value) {
        if(index!=0x1d&&index!=0x1e)throw std::logic_error("Invalid Car emitter vector property");
        using Method=std::int32_t(WINAPI*)(Frame,std::uint32_t,const void*);
        if(scene_method<Method>(frame,module_,0xa0,0x3af80)(frame,index,value.data())<0)
            throw std::runtime_error("Cannot set Car emitter vector");
    }
    std::size_t car_particle_region_count(Frame frame) const {return field<std::uint32_t>(frame,0x1fc);}
    Vector car_particle_region(Frame frame,std::size_t index) const {
        if(index>=car_particle_region_count(frame))throw std::out_of_range("Car emitter texture region");
        const auto* regions=field<const unsigned char*>(frame,0x1f4);
        if(!regions)throw std::runtime_error("Car emitter region storage is missing");
        Vector result;std::memcpy(result.data(),regions+index*0x3c,sizeof(result));return result;
    }
    void select_car_particle_region(Frame frame,std::size_t index) {
        if(index>=car_particle_region_count(frame))throw std::out_of_range("Car emitter region selection");
        using Method=void(WINAPI*)(Frame,std::uint32_t);
        scene_method<Method>(frame,module_,0xc0,0x1baf0)(frame,static_cast<std::uint32_t>(index));
    }
    void clear_car_particle_region(Frame frame,std::size_t index) {
        if(index>=car_particle_region_count(frame))throw std::out_of_range("Car emitter region removal");
        using Method=void(WINAPI*)(Frame,std::uint32_t);
        scene_method<Method>(frame,module_,0xc8,0x3bed0)(frame,static_cast<std::uint32_t>(index));
    }
    void add_car_particle_region(Frame frame,const game::CarParticleRegion& value,std::int32_t index) {
        std::array<std::uint32_t,13> request{};
        std::memcpy(request.data(),value.bounds.data(),sizeof(value.bounds));
        std::copy(value.controls.begin(),value.controls.end(),request.begin()+4);
        using Method=std::int32_t(WINAPI*)(Frame,const void*,std::int32_t);
        scene_method<Method>(frame,module_,0xc4,0x3ba20)(frame,request.data(),index);
    }
    std::string car_particle_texture_name(Frame frame) const {
        const auto texture=field<Frame>(frame,0x1f0);
        if(!texture)throw std::runtime_error("Car emitter texture is missing");
        const auto* name=field<const char*>(texture,0x18);return name?name:"";
    }
    void advance_car_particle(Frame frame,std::uint32_t elapsed){call<void>(frame,0x20,elapsed);}
    std::int32_t car_particle_live_count(Frame frame)const{return field<std::int32_t>(frame,0x220);}
    void activate_car_particle(Frame frame,bool active) {
        using Method=void(WINAPI*)(Frame,std::uint8_t);
        scene_method<Method>(frame,module_,0xac,0x3bfb0)(frame,std::uint8_t(active));
    }
    void prepare_car_strip_mesh(Frame visual) {
        if(call<std::int32_t>(visual,0xa0)<0) //10036160, clone only when shared.
            throw std::runtime_error("Cannot prepare private Car strip mesh");
    }
    template<class Visit> void car_strip_vertices(Frame visual,Visit&& visit) {
        const auto mesh=car_mesh(visual);
        if(!mesh||!field<std::uint32_t>(mesh,0x44))throw std::runtime_error("Car strip has no LOD0 mesh");
        const auto lod=call<Frame>(mesh,4,std::uint32_t{0}); //10031E20.
        const auto count=field<std::uint32_t>(lod,0x54);
        if(field<std::uint32_t>(lod,0x64)!=32)throw std::runtime_error("Unsupported Car strip vertex format");
        const auto data=call<const unsigned char*>(lod,4,std::uint32_t{0}); //10031380, writable lock.
        struct Unlock {
            Frame lod;HMODULE module;
            ~Unlock() {
                using Method=void(WINAPI*)(Frame);
                checked_engine_method<Method>(lod,module,8)(lod); //10031480.
            }
        } unlock{lod,module_};
        if(count&&!data)throw std::runtime_error("Cannot lock Car strip vertices");
        visit(count,[&](std::size_t index) {
            if(index>=count)throw std::out_of_range("Car strip vertex index");
            Vector value{};std::memcpy(value.data(),data+index*32,12);return value;
        });
    }
    template<class Visit>void car_update_strip_vertices(Frame visual,Visit&& visit){ //00506700.
        const auto mesh=car_mesh(visual);
        if(!mesh||!field<std::uint32_t>(mesh,0x44))throw std::runtime_error("Car strip update requires its actual LOD0 mesh");
        const auto lod=call<Frame>(mesh,4,std::uint32_t{0});const auto count=field<std::uint32_t>(lod,0x54);
        if(field<std::uint32_t>(lod,0x64)!=32)throw std::runtime_error("Unsupported Car strip update vertex format");
        auto* data=call<unsigned char*>(lod,4,std::uint32_t{0});bool locked=true;
        struct Unlock {Frame lod;HMODULE module;bool& locked;~Unlock(){if(locked){using Method=void(WINAPI*)(Frame);checked_engine_method<Method>(lod,module,8)(lod);}}} unlock{lod,module_,locked};
        if(count&&!data)throw std::runtime_error("Cannot lock Car strip update vertices");
        const auto vertex=[&](std::size_t index){if(index>=count)throw std::out_of_range("Car strip update vertex index");return data+index*32;};
        visit(count,[&](std::size_t index){Vector result{};std::memcpy(result.data(),vertex(index),12);return result;},
            [&](std::size_t index,const Vector& position){std::memcpy(vertex(index),position.data(),12);},
            [&](std::size_t index,float amount){auto* address=vertex(index)+0x18;float value;std::memcpy(&value,address,4);value+=amount;std::memcpy(address,&value,4);});
        call<void>(lod,8);locked=false;call<void>(lod,0x2c);call<void>(mesh,0x14);
        call<void>(visual,0xac,std::uint32_t{0});update_scene_object(visual);
    }
    void car_scroll_strip(Frame visual,float amount){ //00506C76/00506E50.
        if(frame_kind(visual)!=1||call<std::uint32_t>(visual,4)!=0x5f4a424fu)return;
        car_update_strip_vertices(visual,[&](std::size_t count,auto&&,auto&&,auto&& add){for(auto n=count;n;)add(--n,amount);});
    }
    void remove_car_part(Frame model,Frame part) {
        link_scene_object(part,nullptr);
        call<void>(model,0x80,part);
    }
private:
    ResourceOwner own_car_resource(Frame frame) const {return ResourceOwner(frame,ResourceRelease{module_});}
    struct PartEnumeration {std::vector<Frame> frames;std::exception_ptr failure;};
    static std::int32_t WINAPI collect_part(Frame frame,void* opaque) noexcept {
        auto& context=*static_cast<PartEnumeration*>(opaque);
        if(context.failure)return 0;
        try {context.frames.push_back(frame);}catch(...){context.failure=std::current_exception();}
        return 0; //004F9CC0 returns0 to continue collecting.
    }
};

} //namespace hd2::host
#endif
