#ifndef HD2_GAME_WORLD_SOUND_LOADING_H
#define HD2_GAME_WORLD_SOUND_LOADING_H

#include "frontend/spatial_sound.h"
#include <stdexcept>

namespace hd2::game {

//Actual actor/B0/inline/camera consumers of00433FA0/00435B00. SoundOwner is
//the SAME process SOUND owner (StartupSession on host); World owns the actual
//checked per-Human Composition. This facade creates no voices or shadow state.
//Its actor/scene/camera borrowers must survive synchronous hearing callbacks
//and cleanup, and all listeners must retire before the containing world.
template<class Actors,class World,class Composition,class Camera,class SoundOwner>
class WorldSoundLoading {
public:
    using Actor=typename Actors::Instance;using Human=typename Actors::Human;
    WorldSoundLoading(Actors& actors,World& world,Camera& camera,SoundOwner& sound)
        :actors_(actors),world_(world),camera_(camera),sound_(sound){}
    std::int32_t emit(const frontend::SpatialSoundRequest& request){return sound_.emit_game_sound(request,*this);}
    unsigned sound_actor_type(void* actor){return actual_actor(actor).base().type();}
    auto sound_actor_sector(void* actor){
        auto& human=actual_human(actor);
        return world_.template human_loading_owner<Composition>(human).control().scene_sector(); //005AC4C0, actual DD0 sector, including null.
    }
    auto sound_camera_sector(){return camera_.scene_sector();} //004650C0 virtual8C; no replacement scene query.
    bool sound_special_mode(){return camera_.sound_special_mode();} //00465070 virtual7C; SAME byte270.
    bool sound_actor_flag18(void* actor){ //005A6420, actual B0+50 bit18hex.
        auto& human=actual_human(actor);
        if(!human.loading_state)throw std::runtime_error("Spatial sound suppression requires actual Human B0");
        return (human.loading_state->words().template get<std::uint32_t>(0x50)&0x40000u)!=0;
    }
    void record_actor_cleanup_failure(std::exception_ptr failure)noexcept{actors_.record_actor_cleanup_failure(std::move(failure));}
private:
    static Actor& actual_actor(void* actor){
        if(!actor)throw std::runtime_error("Spatial sound actor lookup requires its actual source identity");
        return *static_cast<Actor*>(actor);
    }
    static Human& actual_human(void* actor){
        auto& value=actual_actor(actor);
        auto* human=dynamic_cast<Human*>(&value);
        if(!human||value.base().type()!=3)throw std::runtime_error("Spatial sound Human lookup received another actor family");
        return *human;
    }
    Actors& actors_;World& world_;Camera& camera_;SoundOwner& sound_;
};
} //namespace hd2::game
#endif
