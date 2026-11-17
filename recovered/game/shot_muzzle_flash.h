#ifndef HD2_GAME_SHOT_MUZZLE_FLASH_H
#define HD2_GAME_SHOT_MUZZLE_FLASH_H
#include "game/human_state_frame_timers.h"
#include <optional>
#include <memory>

namespace hd2::game {
//00453D80/00454060/00454120, actual ShotMgrB0 light/retained flash record.
//Frames and the linked Scene sectors outlive this owner.
template<class Frames>class ShotMuzzleFlash {
public:
    using Frame=typename Frames::Frame;
    using Owner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    explicit ShotMuzzleFlash(Frames& frames):frames_(frames){}
    ~ShotMuzzleFlash(){try{clear();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}}
    ShotMuzzleFlash(const ShotMuzzleFlash&)=delete;ShotMuzzleFlash& operator=(const ShotMuzzleFlash&)=delete;
    void create(Frame parent,Frame flash,std::uint32_t duration){
        if(light_||flash_)throw std::logic_error("Retire muzzle flash before another initialization");
        duration_=duration;if(!parent){time_=duration;return;}
        time_=0;light_.emplace(frames_.create_human_equipment_light());
        if(!light_->get())throw std::runtime_error("Muzzle flash requires its actual kind2 light");
        frames_.configure_shot_light(light_->get(),parent);
        if(flash){
            flash_.emplace(frames_.retain_actor_frame(flash));frames_.object_visible(flash,true);
            frames_.first_person_flash_opacity(flash,1.f);frames_.update_scene_object(flash);
        }
    }
    unsigned tick(std::uint32_t elapsed){
        if(flash_&&human_frame_signed_word(time_)>50){
            frames_.object_visible(flash_->get(),false);frames_.update_scene_object(flash_->get());flash_.reset();
        }
        if(human_frame_signed_word(time_)>=human_frame_signed_word(duration_)){
            if(light_){frames_.object_visible(light_->get(),false);frames_.update_scene_object(light_->get());}
            return 2;
        }
        if(!light_||!light_->get())throw std::runtime_error("Muzzle flash timer requires its actual light");
        const auto fraction=static_cast<long double>(human_frame_signed_word(time_))/human_frame_signed_word(duration_);
        frames_.shot_light_intensity(light_->get(),static_cast<float>(1.f-fraction));
        frames_.shot_light_range(light_->get(),static_cast<float>(4.f-fraction));frames_.update_scene_object(light_->get());
        time_+=elapsed;return 0;
    }
    void clear(){
        if(light_){frames_.remove_shot_light_sectors(light_->get());frames_.human_sound_link(light_->get(),Frame{});light_.reset();}
        if(flash_){frames_.object_visible(flash_->get(),false);flash_.reset();}
    }
    std::uint32_t time()const{return time_;}
private:
    Frames& frames_;std::optional<Owner> light_,flash_;std::uint32_t time_=0,duration_=0;
};
} //namespace hd2::game
#endif
