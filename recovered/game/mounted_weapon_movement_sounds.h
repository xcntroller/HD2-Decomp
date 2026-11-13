#ifndef HD2_GAME_MOUNTED_WEAPON_MOVEMENT_SOUNDS_H
#define HD2_GAME_MOUNTED_WEAPON_MOVEMENT_SOUNDS_H
#include "frontend/sound_catalog.h"
#include <optional>
#include <utility>

namespace hd2::game {
//Actual control120/124 SOUND owners, shared by StaticWeapon and Car gun seats.
//004C6E50 uses SAME process catalogue group3 and existing sound load-into cache.
template<class Frames>class MountedWeaponMovementSounds {
public:
    using Frame=typename Frames::Frame;
    using Owner=decltype(std::declval<Frames&>().create_human_sound());
    explicit MountedWeaponMovementSounds(Frames& frames):frames_(frames){}
    ~MountedWeaponMovementSounds(){clear();}
    Frame moving()const{return first_?first_->get():Frame{};}
    Frame stopping()const{return second_?second_->get():Frame{};}
    template<class Metadata,class Services>void load(Frame parent,Metadata&& metadata,Services& services){ //004C6E50.
        if(moving()||stopping())throw std::logic_error("Retire mounted movement sounds before loading again");
        const auto first=metadata("soundm");
        if(first>=0){
            const auto* entry=services.mounted_sound_entry(first);
            if(!entry)return; //Native missing soundm entry RETURNS, skipping sounds lookup.
            if(!entry->variants.empty())load_one(first_,parent,variant(*entry),true);
        }
        const auto second=metadata("sounds");
        if(second>=0)if(const auto* entry=services.mounted_sound_entry(second);entry&&!entry->variants.empty())
            load_one(second_,parent,variant(*entry),false);
    }
    void release_motion(std::uint32_t& flags){ //004C6DD0, after occupant/view clearing.
        if(!(flags&0x400u))return;
        if(auto frame=moving())frames_.human_sound_enabled(frame,false);
        if(auto frame=stopping())frames_.human_sound_enabled(frame,true);
        flags&=~0x400u; //Native publishes after both engine calls.
    }
    void begin_motion(std::uint32_t& flags){ //004C2580/004C3120 positive transition,124 BEFORE120.
        if(auto frame=stopping())frames_.human_sound_enabled(frame,false);
        if(auto frame=moving())frames_.human_sound_enabled(frame,true);
        flags|=0x400u;
    }
    void clear()noexcept{first_.reset();second_.reset();} //004C7630, release120 BEFORE124, without detach/disable.
private:
    static const frontend::CatalogSoundVariant& variant(const frontend::CatalogSoundEntry& entry){
        if(!entry.variants.front())throw std::logic_error("Mounted movement sound has a null original first variant");
        return *entry.variants.front();
    }
    void load_one(std::optional<Owner>& owner,Frame parent,const frontend::CatalogSoundVariant& definition,bool loop){
        owner.emplace(frames_.create_human_sound());const auto frame=owner->get();
        if(!frame)throw std::runtime_error("Mounted movement sound requires actual kind4 frame");
        //004370D0 copies the first variant's name; no random variant selection.
        if(frames_.load_human_sound(frame,definition.name.c_str())!=0){owner.reset();return;}
        frames_.human_sound_parameter7c(frame,1);
        constexpr float angle=0x1.921fb6p+0f; //Native literal3FC90FDB, pi/2.
        frames_.car_sound_cone(frame,angle,angle);
        if(loop)frames_.human_sound_parameter90(frame,1);
        frames_.human_sound_volume(frame,definition.float14);
        float near_range;std::memcpy(&near_range,&definition.word08,4);
        frames_.human_sound_range(frame,near_range,definition.float0c,.5f,.5f);
        frames_.link_scene_object(frame,parent);frames_.car_sound_head(frame,typename Frames::Vector{});
        frames_.human_sound_enabled(frame,false);
    }
    Frames& frames_;std::optional<Owner> first_,second_;
};
} //namespace hd2::game
#endif
