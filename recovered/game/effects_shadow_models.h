#ifndef HD2_GAME_EFFECTS_SHADOW_MODELS_H
#define HD2_GAME_EFFECTS_SHADOW_MODELS_H

#include "game/effects_environment.h"
#include "frontend/cleanup.h"
#include <cmath>
#include <string>
#include <vector>

namespace hd2::game {

//GEffects004894E0 and00486DF0 use the SAME category4 loaded-resource cache
//and B700 environment as the containing world. Engine kind15 models are real
//borrowed model children; no placeholder texture/projection or renderer exists.
template<class Frames>
class EffectsShadowModels {
public:
    using Frame=typename Frames::Frame;
    EffectsShadowModels(Frames& frames,const EffectsEnvironment& environment):frames_(frames),environment_(environment) {}
    bool model_opacity(std::uint32_t mode,Frame model,float value) { //00486DF0.
        if(!(mode&2)||!model)return false;
        frames_.effects_model_opacity(model,value);return true;
    }
    void lensflare_visible(bool visible) { //00485B70, GEffects virtual+BC, ret4.
        if(const auto flare=frames_.effects_lensflare())frames_.object_visible(flare,visible);
    }
    bool model_camera_amount(Frame model,float amount) { //00486D30, GEffects virtual50; no mode gate.
        if(!model)return false;
        frames_.effects_model_camera_amount(model,1.f-amount);return true;
    }
    bool human_eye_texture(Frame model,std::string name) { //0048A5C0, GEffects virtualAC.
        if(!model)return false;
        if(name.size()>=64)throw std::length_error("Human eye texture exceeds native stack buffer");
        lower_ascii(name);
        if(name.find("e_f0")==std::string::npos)return false;
        if(const auto dot=name.find_last_of('.');dot!=std::string::npos)name.resize(dot);
        std::vector<Frame> groups;
        (void)collect_eye_groups(model,groups,0); //0048A3F0 stops at FIRST matching visual in reverse child traversal.
        if(groups.empty())return false;
        name+=".bmp";
        if(name.size()>=64)throw std::length_error("Human eye texture extension exceeds native stack buffer");
        auto texture=frames_.load_human_eye_texture(name);if(!texture)return false;
        frames_.prepare_human_eye_group(groups[0]); //Group+18 first clones/prepares its own material.
        const auto material=frames_.human_eye_group_material(groups[0]);
        if(!material)throw std::runtime_error("Human eye group lost its actual prepared material");
        frames_.human_eye_material_texture(material,texture.get()); //Material+1C, not face material+2C.
        auto retained=frames_.retain_human_eye_material(material); //Actual DWORD+4 temporary reference.
        for(std::size_t n=1;n<groups.size();++n)frames_.human_eye_group_material(groups[n],retained.get());
        retained.reset();texture.reset();return true; //Material temporary release BEFORE loaded texture release.
    }
    Frame human_eye_material(Frame model,std::vector<Frame>& groups) { //0048A780 shares actual0048A3F0 finder.
        return collect_eye_groups(model,groups,0);
    }
    Frame build(std::uint32_t mode,Frame model,bool human) {
        if(!(mode&2)||!model)return Frame{};
        if(const auto old=frames_.find_human_shadow_model(model)) {
            frames_.link_scene_object(old,Frame{});frames_.remove_human_shadow_model(model,old);
        }
        auto resource=frames_.car_model_resource(model);
        if(const auto dot=resource.find('.');dot!=std::string::npos)resource.resize(dot); //007E5A40=strstr(".").
        if(const auto slash=resource.find_last_of('\\');slash!=std::string::npos&&slash)resource.erase(0,slash+1);
        auto cached=frames_.human_shadow_template(human?"shdpuppet":"s"+resource);
        if(!cached)return Frame{}; //Original cache failure is an optional missing shadow.
        auto owner=frames_.create_human_shadow_model();
        if(!owner)throw std::runtime_error("Cannot create required shadow-model instance");
        const auto shadow=owner.get();frames_.copy_human_shadow_model(shadow,cached.get());cached.reset();
        frames_.human_shadow_name(shadow,"._shad."+frames_.frame_name(model));
        auto parent=frames_.actor_find(model,"a",0xffff);
        if(!parent||frames_.frame_model_owner(parent)!=model)parent=model;
        struct Rollback {
            Frames& frames;Frame model,shadow;bool linked=false,added=false,armed=true;
            ~Rollback(){if(armed) {
                if(linked)try{frames.link_scene_object(shadow,Frame{});}catch(...){frames.record_actor_cleanup_failure(std::current_exception());}
                if(added)try{frames.remove_human_shadow_model(model,shadow);}catch(...){frames.record_actor_cleanup_failure(std::current_exception());}
            }}
        } rollback{frames_,model,shadow};
        frames_.link_scene_object(shadow,parent);rollback.linked=true;
        frames_.add_human_shadow_model(model,shadow);rollback.added=true;
        const auto sine=std::sin(static_cast<long double>(0x1.6571855555556p-3)); //QWORD00810390.
        frames_.human_shadow_sine_squared(shadow,static_cast<float>(sine*sine));
        frames_.human_shadow_direction(shadow,environment_.vector_1c0);
        frames_.human_shadow_dimensions(shadow,environment_.width_1ec,environment_.height_1f0);
        frames_.human_shadow_distance(shadow,environment_.field_1e8);
        frames_.human_shadow_range(shadow,environment_.field_1e4);frames_.human_shadow_scale(shadow,environment_.scale_1e0);
        frames_.update_scene_object(shadow);rollback.armed=false;
        return shadow; //Factory reference releases; model+7C retains its own reference.
    }
private:
    static void lower_ascii(std::string& value){for(auto& byte:value)if(byte>='A'&&byte<='Z')byte=static_cast<char>(byte+('a'-'A'));}
    Frame collect_eye_groups(Frame frame,std::vector<Frame>& groups,unsigned depth) { //0048A3F0.
        if(depth>1024)throw std::runtime_error("Human eye frame hierarchy exceeds traversal bounds");
        groups.clear();Frame last{};
        if(frames_.frame_kind(frame)==1&&(frames_.actor_visual_flags(frame)&0x800u)) {
            const auto type=frames_.visual_type(frame);
            if(type==0x4d474e53u||type==0x524d4d53u)if(const auto mesh=frames_.human_eye_mesh(frame)) {
                for(std::uint32_t n=0;n<frames_.human_eye_lod_count(mesh);++n) {
                    const auto lod=frames_.human_eye_lod(mesh,n);
                    for(auto index=frames_.human_eye_group_count(lod);index;) {
                        const auto group=frames_.human_eye_group(lod,--index);
                        if(const auto material=frames_.human_eye_group_material(group)) {
                            auto texture_name=frames_.human_eye_material_name(material);
                            if(!texture_name)continue;
                            if(texture_name->size()>=256)throw std::length_error("Human eye material name exceeds native stack buffer");
                            lower_ascii(*texture_name);
                            if(texture_name->find("e_f0")!=std::string::npos){groups.push_back(group);last=material;}
                        }
                    }
                }
                if(!groups.empty())return last;
            }
        }
        for(auto n=frames_.actor_child_count(frame);n;) {
            if(const auto found=collect_eye_groups(frames_.actor_child(frame,--n),groups,depth+1))return found;
        }
        return Frame{};
    }
    Frames& frames_;const EffectsEnvironment& environment_;
};

//CD0's00758BF0/00758C60 shadow lifetime, native DD0+A9C. The shadow module
//returns a real model-owned frame; refresh takes the original additional ref.
template<class Frames,class Effects>
class HumanShadowModel {
public:
    using Frame=typename Frames::Frame;
    HumanShadowModel(Frames& frames,Effects& effects):frames_(frames),effects_(effects) {}
    ~HumanShadowModel(){clear();}
    HumanShadowModel(const HumanShadowModel&)=delete;
    HumanShadowModel& operator=(const HumanShadowModel&)=delete;
    void refresh(std::uint32_t mode,Frame model) {
        clear();shadow_=effects_.build(mode,model,true);if(!shadow_)return;
        auto reference=frames_.retain_actor_frame(shadow_);reference.release(); //Stored as the original raw field.
        frames_.human_shadow_distance(shadow_,0x41a00000u);
        frames_.object_visible(shadow_,frames_.car_frame_visible(model));
    }
    void clear() noexcept {
        if(!shadow_)return;
        const auto shadow=shadow_;shadow_=Frame{};
        attempt([&]{frames_.object_visible(shadow,false);});
        attempt([&]{frames_.link_scene_object(shadow,Frame{});});
        //Count AFTER unlink. Original frees two references if signed E4>1,
        //one if E4==1, none if E4<=0; never an unbounded release-to-zero loop.
        const auto references=frames_.item_frame_references(shadow);
        if(references>1)attempt([&]{frames_.release_item_frame(shadow);});
        if(references>=1)attempt([&]{frames_.release_item_frame(shadow);});
    }
    Frame frame() const{return shadow_;}
private:
    template<class Work>void attempt(Work&& work) noexcept {
        try{work();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
    }
    Frames& frames_;Effects& effects_;Frame shadow_{};
};
} //namespace hd2::game
#endif
