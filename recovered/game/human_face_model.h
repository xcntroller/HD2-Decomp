#ifndef HD2_GAME_HUMAN_FACE_MODEL_H
#define HD2_GAME_HUMAN_FACE_MODEL_H

#include "game/human_face_resources.h"
#include <algorithm>
#include <optional>

namespace hd2::game {

//Geometry62B0,00798200/00798250/007982C0. Shared resources outlive this
//owner; the geometry's model/visual/LOD and slot textures are borrowed.
template<class Frames>class HumanFaceModel final : public HumanFaceResources<Frames>::User {
public:
    using Frame=typename Frames::Frame;
    using Resources=HumanFaceResources<Frames>;
    HumanFaceModel(Frames& frames,Resources& shared):frames_(frames),shared_(shared){}
    ~HumanFaceModel(){clear();}
    HumanFaceModel(const HumanFaceModel&)=delete;
    HumanFaceModel& operator=(const HumanFaceModel&)=delete;
    void bind(Frame model,bool player,std::int32_t camouflage,std::optional<float> clothing,unsigned operation=0) {
        if(!shared_.loaded())throw std::logic_error("Offline Human face resources must be initialized before geometry");
        if(operation==1) {
            if(private_textures_){private_textures_=false;use_private_textures();}
            return;
        }
        if(operation!=0&&operation!=2)return;
        if(operation==2&&data_){shared_.meshes().release(data_);data_=nullptr;}
        model_=model;
        if(!data_)data_=shared_.meshes().acquire(model);
        visual_=frames_.actor_find(model,"a",0xffff);
        lod_=visual_&&(frames_.actor_visual_flags(visual_)&0x800u)?frames_.human_face_lod(visual_):Frame{};
        if(visual_)frames_.human_face_prepare_visual(visual_);
        if(!lod_)return;
        frames_.human_face_prepare_uv(lod_);
        if(operation==0){fields10_.fill(0);epoch_=0;prepare_masks();}
        else if(private_textures_){private_textures_=false;use_private_textures();}
        set_variant(shared_.next_variant(player,camouflage));
        shadow_=false;
        if(clothing)set_shadow(*clothing>.1f);
        frames_.human_face_finish_visual(visual_,lod_);
    }
    void clear() noexcept {
        for(auto& mask:masks_)std::vector<std::uint16_t>().swap(mask);
        std::vector<std::uint16_t>().swap(scratch_);
        if(slot_registered_)shared_.release_textures(this);
        if(data_)shared_.meshes().release(data_);
        data_=nullptr;model_=visual_=lod_=Frame{};textures_.fill(Frame{});private_textures_=slot_registered_=false;
        fields10_.fill(0);epoch_=width_=height_=width_mask_=height_mask_=width_log_=height_log_=0;
        shadow_=false;variant_=0;
    }
    void set_shadow(bool enabled) { //00799350, unavailable while a camouflage variant is selected.
        if(variant_||enabled==shadow_)return;
        if(enabled) {
            if(masks_[0].empty())return;
            blend(*human_face_patterns::overlays[0]);
        }else restore_mask();
        shadow_=enabled;
        if(private_textures_)upload_first();
        else set_group_texture(0,enabled?shared_.shadow():shared_.base());
    }
    void set_variant(std::uint8_t variant) { //00799540.
        if(variant_==variant)return;
        if(variant>8)throw std::out_of_range("Human face overlay variant");
        if(variant) {
            if(masks_[0].empty())return;
            restore_mask();blend(*human_face_patterns::overlays[variant]);
        }else restore_mask();
        variant_=variant;
        if(private_textures_)upload_first();
        else set_group_texture(0,variant?shared_.camouflage(variant):shared_.base());
    }
    void use_private_textures() { //00799080; partial native loops do not set38.
        if(private_textures_||!lod_)return;
        const auto* slot=shared_.acquire_textures(this);if(!slot)return;
        slot_registered_=true;
        for(unsigned n=0;n!=7;++n) {
            if(!frames_.human_face_group(lod_,n)||masks_[n].empty())return;
            textures_[n]=slot->textures[n]?slot->textures[n].get():Frame{};
            if(!textures_[n])throw std::runtime_error("Human face private texture slot is missing its actual texture");
            frames_.human_face_upload(textures_[n],masks_[n],height_);
            set_group_texture(n,textures_[n]);
        }
        private_textures_=true;
    }
    void restore_face_shared_textures() override { //00799270, also the tail of00799170.
        if(!shared_.base()||!shared_.shadow())return;
        const auto variant=shared_.camouflage(variant_?variant_:1);if(!variant)return;
        for(unsigned n=0;n!=7;++n) {
            if(!frames_.human_face_group(lod_,n))return;
            set_group_texture(n,n==0?(variant_?variant:(shadow_?shared_.shadow():shared_.base())):shared_.base());
            textures_[n]=Frame{};
        }
        private_textures_=false;
    }
    void restore_shared_textures() { //00799170 additionally removes the slot borrower first.
        if(!private_textures_||!lod_)return;
        shared_.release_textures(this);slot_registered_=false;restore_face_shared_textures();
    }
    void prepare_masks() { //00798E40, including scratch at18+7*4, not a nonexistent eighth group.
        if(private_textures_)restore_shared_textures();
        if(!lod_||!shared_.base())return;
        const auto dimensions=frames_.human_face_texture_size(shared_.base());
        width_=dimensions[0];height_=dimensions[1];
        width_mask_=width_-1;height_mask_=height_-1;
        width_log_=log2_floor(width_);height_log_=log2_floor(height_);
        if(!shared_.shadow()||!shared_.camouflage(variant_?variant_:1))return;
        const auto count=static_cast<std::size_t>(width_)*height_;
        for(unsigned n=0;n!=7;++n) {
            frames_.human_face_prepare_group(lod_,n);
            auto& mask=masks_[n];
            if(mask.empty())mask.resize(count);
            if(mask.size()!=count)throw std::runtime_error("Human face dimensions changed while native masks are allocated");
            Frame texture=shared_.base();
            if(n==0&&(variant_||shadow_)) {
                const auto& pattern=*human_face_patterns::overlays[variant_];
                if(count>pattern.size())throw std::runtime_error("Human face mask exceeds original overlay pixels");
                std::copy_n(pattern.begin(),count,mask.begin());
                texture=variant_?shared_.camouflage(variant_):shared_.shadow();
            }else std::fill(mask.begin(),mask.end(),0);
            set_group_texture(n,texture);
        }
        if(scratch_.empty())scratch_.resize(count);
        if(scratch_.size()!=count)throw std::runtime_error("Human face scratch dimensions changed");
        std::fill(scratch_.begin(),scratch_.end(),0);private_textures_=false;
    }
    std::uint32_t face_texture_epoch() const override{return epoch_;}
    void face_texture_epoch(std::uint32_t epoch){epoch_=epoch;} //Actual runtime producer must supply its clock.
    const auto& masks() const{return masks_;}
    const auto& scratch() const{return scratch_;}
    const HumanFaceMeshData* mesh_data() const{return data_;}
    bool private_textures() const{return private_textures_;}
    bool shadow() const{return shadow_;}
    unsigned variant() const{return variant_;}
private:
    static std::uint32_t log2_floor(std::uint32_t value){std::uint32_t result=0;while(value>>=1)++result;return result;}
    void set_group_texture(unsigned group,Frame texture){frames_.human_face_group_texture(lod_,group,texture);}
    void restore_mask() {if(!masks_[0].empty()&&!scratch_.empty())masks_[0]=scratch_;}
    void blend(const std::array<std::uint16_t,4096>& source) {
        auto& mask=masks_[0];if(mask.size()<source.size())throw std::runtime_error("Human face blending requires original4096 pixels");
        for(std::size_t n=0;n!=source.size();++n) {
            const std::uint32_t from=source[n],to=mask[n],a=from>>12,b=to>>12,total=a+b;
            std::uint32_t value=0;
            if(total) {
                value=std::max(a,b)<<12;
                for(auto bits:{0xf00u,0xf0u,0xfu})value|=(((to&bits)*b+(from&bits)*a)/total)&bits;
            }
            mask[n]=static_cast<std::uint16_t>(value);
        }
    }
    void upload_first(){if(textures_[0])frames_.human_face_upload(textures_[0],masks_[0],height_);}
    Frames& frames_;Resources& shared_;
    Frame model_{},visual_{},lod_{};HumanFaceMeshData* data_=nullptr;
    std::array<std::vector<std::uint16_t>,7> masks_{};std::vector<std::uint16_t> scratch_;
    std::array<Frame,7> textures_{};
    std::array<std::uint8_t,7> fields10_{};
    std::uint32_t epoch_=0,width_=0,height_=0,width_mask_=0,height_mask_=0,width_log_=0,height_log_=0;
    bool private_textures_=false,slot_registered_=false,shadow_=false;std::uint8_t variant_=0;
};
} //namespace hd2::game
#endif
