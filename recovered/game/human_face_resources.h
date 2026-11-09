#ifndef HD2_GAME_HUMAN_FACE_RESOURCES_H
#define HD2_GAME_HUMAN_FACE_RESOURCES_H

#include "game/human_face_mesh_pool.h"
#include "game/human_face_patterns.h"
#include <limits>

namespace hd2::game {

//Native0098ADF0: shared textures, mesh records and eight seven-texture slots.
//The world initializes00796430 before Human activation and closes007967A0
//after retiring all face owners. These textures are independent of DD0+610.
template<class Frames>class HumanFaceResources {
public:
    using Frame=typename Frames::Frame;
    using Owner=typename Frames::ResourceOwner;
    struct User {
        virtual ~User()=default;
        virtual std::uint32_t face_texture_epoch() const=0;
        virtual void restore_face_shared_textures()=0; //00799270, called before slot eviction.
    };
    struct Slot {User* user=nullptr;std::array<Owner,7> textures{};};
    explicit HumanFaceResources(Frames& frames):frames_(frames),meshes_(frames){}
    ~HumanFaceResources(){clear();}
    HumanFaceResources(const HumanFaceResources&)=delete;
    HumanFaceResources& operator=(const HumanFaceResources&)=delete;
    void initialize() { //00796430; the original extra argument is unused.
        if(loaded_)return;
        const auto previous=frames_.hud_driver_state(13);
        frames_.hud_driver_state(13,0);
        try {
            base_=load("stainNull.bmp","stainNullA.bmp",0x200004);
            shadow_=load("hshadow.bmp","hshadowA.bmp",0x200084);
            for(auto& texture:camouflage_)texture.reset();
            constexpr std::array<const char*,4> masks{{"camouflageA.bmp","camouflageB.bmp","camouflageC.bmp","camouflageD.bmp"}};
            for(unsigned n=0;n!=8;++n)camouflage_[n]=load(n<4?"camouflage.bmp":"camouflage2.bmp",masks[n&3],0x200084);
            for(auto& slot:slots_) {
                if(slot.user)slot.user->restore_face_shared_textures();
                slot.user=nullptr;
                for(auto& texture:slot.textures){texture.reset();texture=load("stainNull.bmp","stainNullA.bmp",0x200004);}
            }
            frames_.hud_driver_state(13,previous);
            meshes_.clear();variant_counter_=0;loaded_=true;
        }catch(...) {frames_.hud_driver_state(13,previous);clear();throw;}
    }
    void clear() noexcept { //007967A0: mesh records before main textures, variants, slots.
        meshes_.clear();base_.reset();shadow_.reset();
        for(auto& texture:camouflage_)texture.reset();
        for(auto& slot:slots_){slot.user=nullptr;for(auto& texture:slot.textures)texture.reset();}
        loaded_=false;
    }
    const Slot* acquire_textures(User* user) { //00796C90; unsigned strict minimum, ties keep first.
        if(!user)return nullptr;
        for(auto& slot:slots_)if(slot.user==user)return &slot;
        Slot* oldest=nullptr;auto epoch=std::numeric_limits<std::uint32_t>::max();
        for(auto& slot:slots_) {
            if(!slot.user){slot.user=user;return &slot;}
            const auto current=slot.user->face_texture_epoch();
            if(current<epoch){epoch=current;oldest=&slot;}
        }
        if(oldest){oldest->user->restore_face_shared_textures();oldest->user=user;}
        return oldest;
    }
    void release_textures(User* user) noexcept { //00796D50 clears only the first matching borrower.
        for(auto& slot:slots_)if(slot.user==user){slot.user=nullptr;return;}
    }
    std::uint8_t next_variant(bool player,std::int32_t camouflage) { //007982C0 uses actual mission0098A6B0.
        if(!player||(camouflage!=1&&camouflage!=2))return 0;
        ++variant_counter_;
        return static_cast<std::uint8_t>((variant_counter_&3u)+(camouflage==1?1:5));
    }
    Frame base() const{return base_?base_.get():Frame{};}
    Frame shadow() const{return shadow_?shadow_.get():Frame{};}
    Frame camouflage(unsigned variant) const {if(variant==0||variant>8)throw std::out_of_range("Human face camouflage variant");return camouflage_[variant-1]?camouflage_[variant-1].get():Frame{};}
    HumanFaceMeshPool<Frames>& meshes(){return meshes_;}
    bool loaded() const{return loaded_;}
private:
    Owner load(const char* primary,const char* secondary,unsigned flags) {
        auto owner=frames_.load_car_particle_texture(primary,secondary,flags);
        if(!owner)throw std::runtime_error(std::string("Required offline Human face texture failed: ")+primary);
        return owner;
    }
    Frames& frames_;HumanFaceMeshPool<Frames> meshes_;
    Owner base_{},shadow_{};std::array<Owner,8> camouflage_{};std::array<Slot,8> slots_{};
    std::uint16_t variant_counter_=0;bool loaded_=false;
};
} //namespace hd2::game
#endif
