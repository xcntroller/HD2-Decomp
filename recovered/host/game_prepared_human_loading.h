#ifndef HD2_HOST_GAME_PREPARED_HUMAN_LOADING_H
#define HD2_HOST_GAME_PREPARED_HUMAN_LOADING_H
#include "host/game_world_loading.h"
#include "game/prepared_human_clothing.h"
#include "game/session_peer_records.h"
#include <mmsystem.h>
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
        if((mode_flags()&7u)!=7u)throw std::logic_error("Network Human setup requires its separate actual peer identity");
        configure_session(human,soldier,kind,personality,nullptr,0);
    }
    void configure_session(Human& human,frontend::PreMissionSoldier& soldier,std::uint32_t kind,std::uint32_t personality,
            const std::array<std::uint32_t,12>* stats,std::uint32_t peer){
        game::configure_prepared_human_with_peer(human,soldier,kind,personality,stats,d_.profiles,d_.audio,d_.audio.random_source(),*this,d_.scene.effects(),
            [this,peer](auto& actor,auto& record){
                if((mode_flags()&7u)==7u)return;
                if(actor.loading_state)actor.loading_state->selection_suppressed(static_cast<std::uint8_t>(record.flag08));
                game::set_session_peer_side(d_.startup.game_control_records(),d_.startup,peer,static_cast<std::uint32_t>(record.specialization),false);
                if(record.flag09)d_.session.words.set(0x78,peer);
            });
        (void)d_.world_actors.prepare_human_loading(human); //After cold clothing, BEFORE either direct activation pass.
    }
    Human* create_session_human(std::uint32_t peer,std::int32_t identity,const game::HumanVector& position,const game::HumanVector& direction,
            frontend::PreMissionSoldier& soldier,const std::array<std::uint32_t,12>* stats,std::int32_t clothing,bool alternate){
        return game::create_session_human(d_.scene.frames(),d_.startup.loaded_resources(),d_.scene.frames(),*this,d_.catalogue,
            position,direction,peer,identity,soldier,stats,clothing,alternate);
    }
    std::uint32_t mode_flags(){return d_.startup.application_flags();}
    void enter_control_lock(unsigned index){d_.startup.enter_control_lock(index);}
    void leave_control_lock(unsigned index)noexcept{d_.startup.leave_control_lock(index);}
    bool human_remote_ai_enabled(){return d_.startup.human_remote_ai_enabled();}
    std::uint32_t peer_human_ticks(){return timeGetTime();}
    void append_peer_auxiliary_human(Human* human){(void)d_.players.auxiliary().human_roster().append(human);} //0045FDE0→00461AA0.
    void publish_peer_human(std::uint32_t peer,Human& human){game::publish_session_peer_human(d_.startup.game_control_records(),peer,human,*this);}
    void populate(Human& human,frontend::PreMissionSoldier& soldier){
        game::populate_prepared_human_storage_live(human,soldier,d_.world_actors.items(),clothing_,[this]{return mode_flags();});
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
