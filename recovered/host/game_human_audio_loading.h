#ifndef HD2_HOST_GAME_HUMAN_AUDIO_LOADING_H
#define HD2_HOST_GAME_HUMAN_AUDIO_LOADING_H

#include "host/startup_session.h"
#include "host/game_actor_services.h"
#include "host/game_speech_definitions.h"
#include "game/human_sound_pool.h"

namespace hd2::host {

//Concrete shared dependencies of0059F2C0/007855C0. Reuse startup's actual
//rand(), file resolver, resource caches and persistent variant windows. Human
//controls/definition references must close BEFORE this owner and frame services.
//No playback, Human activation or containing-world readiness is supplied here.
class GameHumanAudioLoading {
public:
    struct Random {
        StartupSession& startup;
        std::uint32_t operator()(){return startup.random_word();}
        void seed(std::uint32_t value){startup.seed_random(value);}
    };
    using Pools=game::HumanSoundPoolManager<GameActorServices,Random>;
    GameHumanAudioLoading(StartupSession& startup,GameActorServices& frames)
        :random_{startup},speech_(startup.pre_mission_files(),random_),pools_(frames,startup.human_sound_variants(),random_) {}
    typename Pools::Lease acquire_pool(){return pools_.acquire();}
    game::SpeechDefinition* acquire_speech(std::int32_t index){return speech_.acquire(index);}
    void release_speech(game::SpeechDefinition* definition) noexcept {speech_.release(definition);}
    Random& random_source() {return random_;}
    void configure_physics_voices() {random_.startup.set_game_sound_voice_limit(3,4);} //0075AE40→sound+90.
    void stop_voice(std::int32_t& handle) {random_.startup.stop_game_sound_voice(handle);}
private:
    Random random_;
    GameSpeechDefinitions<Random> speech_;
    Pools pools_;
};

} //namespace hd2::host
#endif
