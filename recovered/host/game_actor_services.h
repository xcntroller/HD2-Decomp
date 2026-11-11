#ifndef HD2_HOST_GAME_ACTOR_SERVICES_H
#define HD2_HOST_GAME_ACTOR_SERVICES_H

#include "host/game_actor_frames.h"
#include "game/actor_base.h"
#include "game/actor_references.h"
#include "game/actor_frame_data.h"
#include "game/scene_actor.h"
#include <exception>
#include <functional>

namespace hd2::host {

static_assert(sizeof(game::ActorFrameData)==32,"Actor frame data requires the i386 host ABI");

//Concrete shared endpoints for native common actor lifetimes. Actual item and
//script owners register here; their interfaces have no successful defaults.
//This is not an actor factory or a constructed/activated mission world.
//Containing world: destroy actors, then script/item producers, then services.
class GameActorServices : public GameActorFrames {
public:
    using GameActorFrames::GameActorFrames;
    void bind_vertex_animation_preference(std::function<bool()> query){
        if(vertex_animation_preference_||!query)throw std::logic_error("Vertex animation preference requires its actual containing profile getter once");
        vertex_animation_preference_=std::move(query);
    }
    bool vertex_animations_enabled(){ //00474740/004749A0→006B6740→006B33E0(1), resolve live selected profile.
        if(!vertex_animation_preference_)throw std::logic_error("Vertex animation update has no actual profile preference owner");
        return vertex_animation_preference_();
    }
    using Actor=game::ActorBase<GameActorServices>;
    class Item {
    public:
        virtual ~Item()=default;
        virtual float scalar() const=0; //Entry virtual+2C consumed by007D6970/007D7220.
        virtual std::uint32_t definition_identifier() const=0;
        virtual std::uint32_t instance_family() const=0;
        virtual long double volume() const=0;
    };
    class Context {
    public:
        virtual ~Context()=default;
        virtual void remove_actor(Actor*) noexcept=0; //Actual context's004CE8F0.
    };
    using Entry=Item*;
    Entry register_entry(std::unique_ptr<Item> owner) {return entries_.add(std::move(owner));}
    float entry_scalar(Entry entry) const {
        if(!entry)throw std::logic_error("Actor slot contains a null entry");
        return entry->scalar();
    }
    std::uint32_t entry_definition(Entry entry) const {if(!entry)throw std::logic_error("Missing item entry");return entry->definition_identifier();}
    std::uint32_t entry_family(Entry entry) const {if(!entry)throw std::logic_error("Missing item entry");return entry->instance_family();}
    long double entry_volume(Entry entry) const {if(!entry)throw std::logic_error("Missing item entry");return entry->volume();}
    void release_entry(Entry entry) noexcept {entries_.release_entry(entry);}
    void clear_actor_entries() noexcept {entries_.clear();} //After actors/queues, before item services/catalogue.
    void clear_file_frame_data()noexcept{frame_data_.clear();} //Source sidecar tail, including retained frames outside the scene enumeration.
    void register_context(Context& context) {contexts_.add(context);}
    void unregister_context(Context& context) noexcept {contexts_.remove(context);}
    void clear_actor_references(Actor* actor) noexcept {contexts_.clear_actor_references(actor);}
    std::uint32_t actor_type(const Actor* actor) const {
        if(!actor)throw std::logic_error("Collision actor is missing its native owner");
        return actor->type();
    }
    std::int32_t actor_id(const Actor* actor) const {
        if(!actor)throw std::logic_error("Collision actor is missing its native owner");
        return actor->id10();
    }
    template<class Instance> void bind_scene_actor_frame(Frame frame,Instance* actor) {
        frame_data_.bind(frame,reinterpret_cast<std::uintptr_t>(actor));
    }
    void unbind_scene_actor_frame(Frame frame) noexcept {frame_data_.unbind(frame);}
    game::ActorFrameData& actor_frame_data(Frame frame) {return frame_data_.ensure(frame);}
    const game::ActorFrameData* find_actor_frame_data(Frame frame) const {return frame_data_.find(frame);}
    void erase_actor_frame_data(Frame frame){
        (void)frame_data_.find(frame); //Reject foreign opaque allocations; never free an unknown producer's data.
        frame_data_.erase(frame); //006E9B20, retract opaque pointer before releasing its actual allocation/frame.
    }
    void remove_unused_actor_frame_data(Frame frame) noexcept {frame_data_.remove_if_unused(frame);}
    void register_car_actor(game::SceneActor<GameActorServices>* actor) {
        if(!actor)throw std::logic_error("Car registry requires an owner");
        car_actors_.push_back(actor); //004D0B60,0089BC28.
    }
    void unregister_car_actor(game::SceneActor<GameActorServices>* actor) noexcept {
        for(auto n=car_actors_.size();n!=0;)if(car_actors_[--n]==actor) {
            car_actors_[n]=car_actors_.back();car_actors_.pop_back();break; //004D0F20.
        }
    }
    const std::vector<game::SceneActor<GameActorServices>*>& car_actors() const {return car_actors_;}
    void reset_dog_shared_state() noexcept {dog_shared_state_.fill(0);}
    void record_actor_cleanup_failure(std::exception_ptr failure) noexcept {
        if(!cleanup_failure_)cleanup_failure_=std::move(failure);
    }
    void rethrow_actor_cleanup_failure() {
        auto failure=std::exchange(cleanup_failure_,{});
        if(failure)std::rethrow_exception(failure);
    }
private:
    std::function<bool()> vertex_animation_preference_;
    std::array<std::uint32_t,0x45> dog_shared_state_{};
    std::vector<game::SceneActor<GameActorServices>*> car_actors_;
    std::exception_ptr cleanup_failure_;
    game::ActorFrameDataStore<GameActorFrames> frame_data_{*this};
    game::ActorEntryRegistry<std::unique_ptr<Item>> entries_;
    game::ActorContextRegistry<Context> contexts_;
};

} //namespace hd2::host
#endif
