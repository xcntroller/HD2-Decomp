#ifndef HD2_HOST_GAME_FOWL_SOUND_LOADING_H
#define HD2_HOST_GAME_FOWL_SOUND_LOADING_H
#include "host/game_scene_actors.h"
#include "frontend/spatial_sound.h"
#include <algorithm>
#include <exception>
#include <utility>
#include <vector>
namespace hd2::host {
//005BA860 through actual SOUND registrations. The callback has no userdata;
//one containing world routes through the SAME Scene2 EC registry. Its listener
//owner outlives actors and is cleared before the sound manager/scene retires.
template<class SoundOwner>class GameFowlSoundLoading {
public:
    using Frame=GameActorServices::Frame;using Actor=game::SceneActor<GameActorServices>;
    GameFowlSoundLoading(GameActorServices& frames,SoundOwner& sound):frames_(frames),sound_(sound){
        if(current_)throw std::logic_error("Fowl sound callback already belongs to a live world");current_=this;
    }
    ~GameFowlSoundLoading(){clear();if(current_==this)current_=nullptr;}
    GameFowlSoundLoading(const GameFowlSoundLoading&)=delete;GameFowlSoundLoading& operator=(const GameFowlSoundLoading&)=delete;
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
        if(!input.listener)return;
        const auto* data=frames_.find_actor_frame_data(static_cast<Frame>(input.listener));if(!data||!data->words[0])return;
        auto* actor=reinterpret_cast<Actor*>(data->words[0]);auto* fowl=dynamic_cast<game::FowlGeneratorSceneActor<GameActorServices>*>(actor);
        if(!fowl)throw std::runtime_error("Fowl sound callback resolves to another actor family");
        if(!fowl->loading)throw std::runtime_error("Fowl sound callback requires its actual generated-bird owner");
        fowl->loading->heard(static_cast<std::uint32_t>(input.group),input.volume);
    }
private:
    static void WINAPI heard(const frontend::HeardSpatialSound* input)noexcept {
        if(!current_||!input)return;
        try{current_->notify(*input);}catch(...){if(!current_->failure_)current_->failure_=std::current_exception();}
    }
    inline static GameFowlSoundLoading* current_=nullptr;
    GameActorServices& frames_;SoundOwner& sound_;std::vector<std::int32_t> slots_;std::exception_ptr failure_;
};
} //namespace hd2::host
#endif
