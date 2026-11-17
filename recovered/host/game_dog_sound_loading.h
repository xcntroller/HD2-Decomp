#ifndef HD2_HOST_GAME_DOG_SOUND_LOADING_H
#define HD2_HOST_GAME_DOG_SOUND_LOADING_H
#include "host/game_scene_actors.h"
#include "frontend/spatial_sound.h"
#include <algorithm>
#include <exception>
#include <utility>
#include <vector>
namespace hd2::host {
//005D4DB0 registration to actual005D6000 notification receiver. Callback
//exceptions latch on this SAME World and are rethrown at its module frame gate.
template<class SoundOwner>class GameDogSoundLoading {
public:
    using Frame=GameActorServices::Frame;using Actor=game::SceneActor<GameActorServices>;
    GameDogSoundLoading(GameActorServices& frames,SoundOwner& sound):frames_(frames),sound_(sound){
        if(current_)throw std::logic_error("Dog sound callback already belongs to a live world");current_=this;
    }
    ~GameDogSoundLoading(){clear();if(current_==this)current_=nullptr;}
    GameDogSoundLoading(const GameDogSoundLoading&)=delete;GameDogSoundLoading& operator=(const GameDogSoundLoading&)=delete;
    std::int32_t add_listener(Frame frame) {
        slots_.reserve(slots_.size()+1);
        const auto slot=sound_.add_game_sound_listener(frame,reinterpret_cast<void*>(&heard),[&](Frame value){
            const auto* data=value?frames_.find_actor_frame_data(value):nullptr;return data?reinterpret_cast<Actor*>(data->words[0]):nullptr;
        },[](Actor* actor){return actor->base().type();});
        if(slot>=0)slots_.push_back(slot);return slot;
    }
    void remove_listener(std::int32_t slot){
        sound_.remove_game_sound_listener(slot);const auto found=std::find(slots_.begin(),slots_.end(),slot);if(found!=slots_.end())slots_.erase(found);
    }
    void clear()noexcept {
        while(!slots_.empty()){const auto slot=slots_.back();slots_.pop_back();try{sound_.remove_game_sound_listener(slot);}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}}
    }
    void rethrow_callback_failure(){if(auto failure=std::exchange(failure_,{}))std::rethrow_exception(failure);}
    void notify(const frontend::HeardSpatialSound& input) {
        sound_.dog_sound_notification(input);
    }
private:
    static void WINAPI heard(const frontend::HeardSpatialSound* input)noexcept {
        if(!current_||!input)return;
        try{current_->notify(*input);}catch(...){if(!current_->failure_)current_->failure_=std::current_exception();}
    }
    inline static GameDogSoundLoading* current_=nullptr;
    GameActorServices& frames_;SoundOwner& sound_;std::vector<std::int32_t> slots_;std::exception_ptr failure_;
};
} //namespace hd2::host
#endif
