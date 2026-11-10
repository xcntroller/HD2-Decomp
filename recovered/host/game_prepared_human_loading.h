#ifndef HD2_HOST_GAME_PREPARED_HUMAN_LOADING_H
#define HD2_HOST_GAME_PREPARED_HUMAN_LOADING_H
#include "host/game_world_loading.h"
#include "game/prepared_human_clothing.h"
namespace hd2::host {

//Actual00612A60/0059EDC0/0059EB60/005A6930 composition. Cold properties,
//speech/profile/storage/clothing precede lazy DD0/B0 activation. WorldActors
//publishes the SAME lifetime map before00611C90 invokes Human.activate directly.
//Does not allocate another scene, item module, speech cache, input or roster.
class GamePreparedHumanLoading {
public:
    using Dependencies=world_loading::Dependencies;using Human=world_loading::Human;
    struct DubbingModels {
        StartupSession& startup;
        void retire_model(EngineFrame* model){startup.retire_game_dubbing(model);}
    };
    using Clothing=game::PreparedHumanClothing<GameActorServices,LoadedResources,world_loading::Actors::Items,
        game::EffectsShadowModels<GameActorServices>,GameHumanAudioLoading::Random,DubbingModels,world_loading::Control>;
    explicit GamePreparedHumanLoading(Dependencies& dependencies)
        :d_(dependencies),dubbing_{d_.startup},clothing_(d_.scene.frames(),d_.startup.loaded_resources(),d_.world_actors.items(),d_.scene.effects(),
            d_.audio.random_source(),dubbing_,d_.players.control(),d_.catalogue,game::SessionTimeWord(d_.session)){}
    Human* operator()(const game::HumanVector& position,const game::HumanVector& direction,std::int32_t identity,
                      frontend::PreMissionSoldier& soldier,std::int32_t clothing,bool alternate){
        return game::create_prepared_human(d_.scene.frames(),d_.startup.loaded_resources(),d_.scene.frames(),*this,
            d_.catalogue,position,direction,identity,soldier,clothing,alternate);
    }
    void configure(Human& human,frontend::PreMissionSoldier& soldier,std::uint32_t kind,std::uint32_t personality){
        game::configure_prepared_human(human,soldier,kind,personality,nullptr,d_.profiles,d_.audio,d_.audio.random_source(),*this,d_.scene.effects());
        (void)d_.world_actors.prepare_human_loading(human); //After cold clothing, BEFORE either direct activation pass.
    }
    void populate(Human& human,frontend::PreMissionSoldier& soldier){
        game::populate_prepared_human_storage(human,soldier,d_.world_actors.items(),clothing_,d_.startup.application_flags());
    }
    void spawn_model_created(EngineFrame* frame){ //00611C90 frameEC+4 guard→0041D960(0).
        const auto* data=d_.scene.frames().find_actor_frame_data(frame);if(!data||!data->words[1])return;
        auto* context=d_.script_link.context_for_frame(frame);
        if(!context||reinterpret_cast<std::uintptr_t>(context)!=data->words[1])
            throw std::logic_error("Spawn event requires SAME actual frame Script context");
        context->use(false);
    }
private:
    Dependencies& d_;DubbingModels dubbing_;Clothing clothing_;
};
} //namespace hd2::host
#endif
