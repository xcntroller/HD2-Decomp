#ifndef HD2_GAME_HUMAN_SCENE_ACTOR_H
#define HD2_GAME_HUMAN_SCENE_ACTOR_H

#include "game/scene_actor.h"
#include "game/actor_initial_properties.h"
#include "game/human_perception.h"
#include "game/human_ai_event_queue.h"
#include "game/human_loading_state_view.h"
#include "game/human_loading_inventory_receiver.h"
#include "game/human_formation.h"
#include "game/human_perception_records.h"
#include "game/human_activation_fields.h"
#include "game/human_loading_lifetime.h"
#include <exception>

namespace hd2::frontend {struct HumanRecord;}

namespace hd2::game {
struct SpeechDefinition;
struct MissionTableRecord;

//0059BF00's two aligned cold control allocations. These types intentionally
//expose actual typed AI-event queues only after binding the process pool.
//Other raw command producers and Human simulation remain separate. Unknown/
//unwritten original bytes earn no credit.
struct alignas(16) HumanColdCommandControl {
    struct Queue {std::array<std::uintptr_t,11> words{};HumanAIEventQueue events;}; //00533E90/005AA890, typed pointers outside words.
    Queue first,second;
    std::array<std::vector<std::uint32_t>,2> lists; //005AA7D0, at78/88.
    std::uint32_t field10=0;
    bool flag14=false;
    void bind_event_pool(HumanAIEventPool& pool){first.events.bind(pool);second.events.bind(pool);}
    void advance_events(std::uint32_t elapsed){ //005AD700: FIRST then SECOND, both reverse; actual typed deques.
        for(const auto* queue:{&first,&second})for(auto word:queue->words)
            if(word)throw std::logic_error("Unknown raw Human command queue requires its actual frame producer");
        first.events.advance(elapsed);second.events.advance(elapsed);
    }
    void clear_empty() { //005AE450: known typed events plus guarded unknown raw producers.
        for(const auto& queue:{&first,&second})for(auto word:queue->words)
            if(word)throw std::logic_error("Unknown raw Human command queue requires its actual producer");
        first.events.clear();second.events.clear();lists[0].clear();lists[1].clear();flag14=false;field10=0;
    }
};

//Human+2C4, allocated by0059BF00 and reset by0059F2C0. The vector at zero
//feeds0057EEC0 and0057EF30. Its producer is the shared005647C0 movement pass.
template<class Actor>
struct alignas(16) HumanSceneMovementState {
    explicit HumanSceneMovementState(Actor* actor) : owner(actor) {}
    Actor* owner;
    std::array<float,3> steering{};
    HumanVector position20{},velocity30{},direction40{},origin50{},target60{};
    HumanVector previous70{},previous80{};
    HumanVector side90{},upA0{},forwardB0{},centreC0{};
    std::array<float,3> radiiD0{};
    float speedDC=0;
    std::vector<HumanAINavigationJobPool::Lease>* path=nullptr; //E0 borrows the actual B0+F4 vector owner.
    MissionTableRecord* target_record=nullptr; //EC.
    MissionTableRecord* floor_record=nullptr; //F0.
    MissionTableRecord* projected_floor=nullptr; //F4.
    std::uint32_t field100=0xffffffffu;
    std::uint16_t fielde4=0;
    std::uint16_t path_deadline=0;
    float path_distance=0;
    bool flag10=false,flag104=true,flag105=true,flag106=true,flag107=false,flag108=false,flag109=false;
    std::uint8_t flag10a=0;
    void reset_loading(Actor& actor) { //0059F2C0 tail.
        owner=&actor;path=nullptr;target_record=floor_record=projected_floor=nullptr;
        fielde4=0;flag104=true;field100=0xffffffffu;
        steering.fill(0);position20.fill(0);direction40.fill(0);
        flag10=flag109=false;flag10a=0;flag105=flag106=true;
        const std::uint32_t unwritten=0x7fc00000u;
        std::memcpy(&previous70[0],&unwritten,sizeof unwritten);
        std::memcpy(&previous80[0],&unwritten,sizeof unwritten);
    }
};

template<class Backend,class Runtime>
class HumanSceneActor final : public SceneActor<Backend> {
public:
    using Frame=typename Backend::Frame;
    HumanSceneActor(Backend& backend,Runtime& runtime,HumanStorageScalars scalars={})
        : SceneActor<Backend>(backend,3),properties(initial_human_properties(runtime.mode_flags())),
          command(std::make_unique<HumanColdCommandControl>()),
          scene_movement(std::make_unique<HumanSceneMovementState<HumanSceneActor>>(this)),runtime_(runtime),scalars_(scalars) {}
    ~HumanSceneActor() override { //0059E3E0; actual proximity/player endpoints are mandatory.
        if(field188!=-1)cleanup([&] {runtime_.release_human_proximity(field188);});
        cleanup([&] {
            const auto id=this->base_.id10();
            //Original signed<32 admits-1 and then indexes before the array.
            //Reject that undefined index by omitting unregistered native IDs.
            if((runtime_.mode_flags()&7)!=7&&id>=0&&id<32)
                runtime_.clear_human_player_slot(static_cast<std::uint32_t>(id));
        });
        if(command) {
            cleanup([&] {command->clear_empty();});
            cleanup([&] {command->clear_empty();}); //Called twice by original destructor.
            command.reset();
        }
        this->base_.clear_storage(); //005A2A40 precedes second aligned owner release.
        scene_movement.reset(); //0059E3E0 frees2C4 after005A2A40.
    }
    bool initialize(Frame frame) override { //0059E5E0.
        if(!this->base_.initialize_frame(frame,scalars_)||this->backend_.frame_kind(frame)!=9)return false;
        properties.field170=0xffffffffu;flag90=false;field160=nullptr;fieldb0=0;
        field2b8=field2bc=Frame{};field208=field204=field94=0;return true;
    }
    void load(SerializedBytes& bytes) override {properties.load(bytes);} //0059E800.
    void activate(bool authority) {
        if(!loading_lifetime)throw std::runtime_error("Human activation requires its actual containing loading owner");
        loading_lifetime->activate(authority);
    }
    void deactivate(bool authority) {
        if(!loading_lifetime)throw std::runtime_error("Human retirement requires its actual containing loading owner");
        loading_lifetime->deactivate(authority);
    }
    HumanActorProperties properties;
    std::string fieldbc;
    std::array<std::uint32_t,9> fieldc0{{0,0x3f000000,0,0x3f800000,0,0,0,0,0}}; //0059C270.
    std::vector<std::uint32_t> field190;
    HumanPerceptionLists<SceneActor<Backend>*> perception; //Actual268/278/288/298 actor borrowers.
    HumanPerceptionRecords<SceneActor<Backend>*> list2ac; //Actual pooled sound-perception records; bind shared process pool first.
    std::unique_ptr<HumanColdCommandControl> command;
    std::unique_ptr<HumanSceneMovementState<HumanSceneActor>> scene_movement;
    HumanLoadingStateView<HumanSceneActor>* loading_state=nullptr; //Borrowed actual B0; containing state owner retires it first.
    HumanLoadingInventoryReceiver<Backend>* loading_inventory=nullptr; //Actual command/transfer owner, before B0 bind.
    std::unique_ptr<HumanFormation<Backend,HumanSceneActor>> formation; //Native2C8 owned formation; retire members before Human teardown.
    HumanSceneActor* formation_leader=nullptr; //Native2CC borrower, distinct from own2C8.
    HumanLoadingLifetime* loading_lifetime=nullptr; //Borrowed owner; world retires/unbinds it before deleting Human.
    frontend::HumanRecord* profile_record=nullptr; //Borrowed actual HumanStore B8, outlives the mission.
    Frame head_marker{}; //Borrowed native260, resolved z then f before inline98 activation.
    Frame field2b8{},field2bc{}; //Actual borrowed dubbing attachments, not frame pointers in DWORD scalar storage.
    SceneActor<Backend>* field160=nullptr; //Actual borrowed attachment actor, never a pointer in a DWORD.
    std::int32_t field164=-1;
    HumanActivationFields activation;
    SpeechDefinition* speech_definition=nullptr; //Actual borrowed2C0; release its shared cache reference on deactivation.
    std::uint32_t field94=0,field98=0,fielda0=0,fielda8=0,fieldb0=0,fieldb8=0,
        field174=0,field204=0,field208=0,
        field258=0xffffffffu,field25c=0xffffffffu,field2a4=0xffffffffu;
    std::int32_t field188=-1;
    bool flag90=false;
private:
    template<class Work> void cleanup(Work&& work) noexcept {
        try {work();}catch(...) {this->backend_.record_actor_cleanup_failure(std::current_exception());}
    }
    Runtime& runtime_;
    HumanStorageScalars scalars_;
};

} //namespace hd2::game
#endif
