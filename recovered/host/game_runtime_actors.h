#ifndef HD2_HOST_GAME_RUNTIME_ACTORS_H
#define HD2_HOST_GAME_RUNTIME_ACTORS_H

#include "host/game_actor_services.h"
#include "game/placed_item_actor.h"
#include "game/dynamic_item_actor.h"
#include "game/detached_item_actor.h"
#include "game/runtime_actor_registry.h"
#include "game/mine_scene_actor.h"
#include "game/shot_effect.h"
#include "game/car_scene_actor.h"
#include "game/runtime_mine_creation.h"
#include "game/runtime_actor_packets.h"
#include "frontend/localized_text.h"

namespace hd2::host {

//004770B0's separate runtime owner. Borrow the SAME frame-data/entry services
//as Scene2, not a second GameActorServices instance. Activation supplies actual
//common registry endpoints and0049EF70 presentation-reference cleanup; the
//containing world must keep it, navigation and entries alive through clear().
template<class Activation,class Collisions,class Items>
class GameRuntimeActors {
public:
    using Frame=GameActorServices::Frame;
    using Base=GameActorServices::Actor;
    using Instance=game::SceneActor<GameActorServices>;
    using ActorOwner=std::unique_ptr<Instance>;
    using ItemActor=game::PlacedItemActor<GameActorServices,Activation>;
    using LooseItemActor=game::DynamicItemActor<GameActorServices,Activation,Collisions,Items>;
    using DetachedItem=game::DetachedItemActor<GameActorServices,GameRuntimeActors,Collisions>;
    using Mine=game::MineSceneActor<GameActorServices,GameRuntimeActors,Collisions>;
    using Body=typename LooseItemActor::Body;
    using Descriptor=typename Body::Descriptor;
    using Entry=GameActorServices::Entry;
    struct StaticItem {Frame frame;void* object;Descriptor* descriptor;ItemActor* actor=nullptr;};
    GameRuntimeActors(GameActorServices& frames,Activation& activation,game::MissionNavigation& navigation,
                      const frontend::LocalizedTextCache& text,Collisions& collisions,typename Body::Registry& bodies,Items& items)
        : frames_(frames),activation_(activation),navigation_(navigation),text_(text),collisions_(collisions),bodies_(bodies),items_(items) {}
    ActorOwner make_runtime_actor(std::uint32_t type) {
        if(type==2)return std::make_unique<Mine>(frames_,*this,collisions_);
        if(type==35)return std::make_unique<ItemActor>(frames_,activation_,navigation_,frontend::lookup_localized_text(text_,0x1bf6));
        if(type==11)return std::make_unique<LooseItemActor>(frames_,activation_,collisions_,bodies_,items_);
        if(type==12)return std::make_unique<DetachedItem>(frames_,*this,collisions_,bodies_);
        throw std::runtime_error("Required runtime actor constructor is not reconstructed: "+std::to_string(type));
    }
    ItemActor* create_item(Frame frame,std::int32_t identity=-1) {
        return static_cast<ItemActor*>(registry_.create(35,frame,identity));
    }
    LooseItemActor* create_loose_item(Frame frame,std::int32_t identity=-1) {
        return static_cast<LooseItemActor*>(registry_.create(11,frame,identity));
    }
    Mine* create_mine(Frame frame,std::int32_t identity=-1){return static_cast<Mine*>(registry_.create(2,frame,identity));}
    Instance* clone_car_detached(Frame source,const GameActorServices::Vector& position,const GameActorServices::Vector& direction){ //00479450.
        auto visual=frames_.create_car_detached_clone(source,position,direction);if(!visual)return nullptr;
        const auto identity=game::consume_runtime_detached_identity(field74_);auto* actor=registry_.create(12,visual.get(),identity);
        visual.reset();return actor; //Factory reference release AFTER registry/activation.
    }
    void launch_car_detached(Instance* actor,const game::CarDetachedPartRequest& request,unsigned mode){
        if(mode!=1)throw std::runtime_error("Detached part launch requires native mode1");
        required_detached(actor).launch(request,true);
    }
    void launch_car_detached(Instance* actor,const game::CarDetachedWheelRequest& request,unsigned mode){
        if(mode!=0)throw std::runtime_error("Detached wheel launch requires native mode0");
        required_detached(actor).launch(request);
    }
    void update_detached_body(Body& body,std::uint32_t elapsed){activation_.update_detached_body(body,elapsed);}
    void restore_detached_camera_amount(Frame frame){activation_.restore_detached_camera_amount(frame);}
    void fade_detached(Frame frame,std::int32_t remaining){activation_.fade_detached(frame,remaining);}
    GameActorServices& frames(){return frames_;}
    void contact_mine(Instance* borrowed,Instance* touching,std::uint32_t slot){
        if(!registry_.owns(borrowed))throw std::logic_error("Mine contact borrower has no actual live runtime owner");
        auto* mine=dynamic_cast<Mine*>(borrowed);if(!mine)throw std::runtime_error("Required nonMine trigger contact receiver is unavailable");
        mine->contact(touching,slot);
    }
    Instance* mine_contact_source(Instance* touching){
        //004D11F0 is direct Car24AC, distinct from seat0/current occupant.
        auto* car=dynamic_cast<game::CarSceneActor<GameActorServices>*>(touching);
        if(!car)throw std::logic_error("Mine fallback source requires actual touching Car24AC owner");
        return car->driver24ac;
    }
    const auto& mine_contact_definition(std::int32_t identity){
        const auto* definition=items_.catalogue().item(identity);
        if(!definition)throw std::logic_error("Mine contact requires actual item definition48");return *definition;
    }
    void publish_mine_contact(std::int32_t identity,std::uint32_t command){activation_.publish_mine_contact(identity,command);}
    void mine_explosion(Instance* source,const game::HumanVector& position,float radius,float power,std::int32_t sound){activation_.mine_explosion(source,position,radius,power,sound);}
    Mine* place_mine(Instance* human,std::int32_t definition,const game::HumanVector& position,const game::HumanVector& direction,bool publish){
        return game::create_runtime_mine(frames_,*this,items_.catalogue(),human,definition,position,direction,publish);
    }
    Instance* present_mine(std::int32_t identity,std::int32_t human,std::int32_t definition,const game::HumanVector& position,const game::HumanVector& direction){
        return game::present_runtime_mine(frames_,*this,items_.catalogue(),identity,human,definition,position,direction);
    }
    auto* mine_scene_actor(std::int32_t identity){return activation_.mine_scene_actor(identity);}
    bool consume_mine_inventory(Instance* human,std::int32_t definition){return activation_.consume_mine_inventory(human,definition);}
    void publish_mine(std::int32_t identity,std::int32_t human,std::int32_t definition,const game::HumanVector& position,const game::HumanVector& direction){
        activation_.publish_mine(identity,human,definition,position,direction);
    }
    bool receive_packet(game::SessionPacketStream& packet,bool authority){return game::receive_runtime_actor_packet(*this,packet,authority);}
    void receive_runtime_authority(Instance& actor,game::SessionPacketStream&){
        if(actor.base().type()==2||actor.base().type()==12)return; //005FEBA0/005FDCF0 genuine RET4.
        throw std::runtime_error("Required runtime actor authority packet leaf is unavailable");
    }
    void receive_runtime_presentation(Instance& actor,game::SessionPacketStream& packet){
        if(actor.base().type()==2){static_cast<Mine&>(actor).receive(packet);return;}
        if(actor.base().type()==12)return; //005FDCE0 genuine RET4, no payload.
        throw std::runtime_error("Required runtime actor presentation packet leaf is unavailable");
    }
    void set_runtime_actor_active(Base& base,bool value){set_active(base,value);}
    void set_scene_actor_active(Base& base,bool value){activation_.set_scene_actor_active(base,value);}
    std::string mine_caption(std::int32_t identity){
        const auto* definition=items_.catalogue().item(identity);
        if(!definition)throw std::logic_error("Mine caption requires actual item definition");
        return frontend::lookup_localized_text(text_,static_cast<std::int32_t>(definition->caption));
    }
    void mine_effect(std::int32_t identity,const GameActorServices::Vector& position,const GameActorServices::Vector& direction){
        const auto* definition=items_.catalogue().item(identity);
        if(!definition)throw std::logic_error("Mine effect requires actual item definition");
        const auto effect=game::mine_effect_definition(*definition);
        activation_.emit_mine_effect(position,direction,effect.particle,effect.sound);
    }
    void set_active(Base& base,bool value) {registry_.active(registry_.actor_for_base(base),value);}
    void begin_activation(bool authority) {registry_.begin_activation(authority);field74_=0;}
    void queue_static_item(Frame frame,void* object,Descriptor& descriptor) { //00479540, three borrowed handles.
        if(!frame||!object)throw std::runtime_error("Static item queue requires actual frame and physics object");
        static_items_.push_back(StaticItem{frame,object,&descriptor});
    }
    void activate(bool authority) { //00477660/004794D0/00479530, required mission branches.
        begin_activation(authority);
        if(authority||!(mode_flags()&1)) {
            for(std::size_t index=0;index!=static_items_.size();++index) {
                const auto queued=static_items_[index];
                auto* actor=create_item(queued.frame);
                actor->physics_object(queued.object);static_items_[index].actor=actor;
                queued.descriptor->actor=actor;
                try {collisions_.move(*queued.descriptor);}catch(...) {registry_.remove(actor);throw;}
            }
        }
        if(!authority||!(mode_flags()&1))return;
        //Items3 precedes this module12 in the original ascending activation.
        //Use its actual shared placement array, not another decoded item file.
        for(std::size_t index=0;index!=items_.placed_items().size();++index) {
            const auto placed=items_.placed_items()[index];
            auto& item=items_.runtime_item(placed.entry);
            if(item.flag1d&&item.definition().kind()==10){
                const auto pose=game::placed_mine_pose(activation_,placed.position);
                const auto* mine=place_mine(nullptr,static_cast<std::int32_t>(item.definition().identifier),pose.first,pose.second,false);
                items_.remove_runtime_item(placed.entry); //007DC8A0 after00478B90, including a failed model load.
                if(!mine)throw std::runtime_error("Required placed Mine model resource failed to load");
                continue;
            }
            auto model=item.model();
            if(!model)throw std::runtime_error("Placed mission item model is unavailable");
            frames_.object_position(model,{placed.position[0],placed.position[1],placed.position[2]});
            frames_.object_rotation(model,placed.rotation);
            auto* actor=create_loose_item(model);
            try {actor->bind_item(placed.entry,{0,0,0,0});}
            catch(...) {registry_.remove(actor);throw;}
            if((mode_flags()&7)!=7&&item.definition().kind()==20)retained_items_.push_back(placed);
        }
        items_.consume_placed_items();
    }
    void remove(Instance* actor) noexcept {registry_.remove(actor);}
    template<class Human>void remove_human_links(Human& human){ //00479700, SAME runtime Mine9C borrower and native increment-after-swap quirk.
        registry_.remove_owned_forward([&](Instance& actor){
            if(actor.base().type()!=2)return false;
            auto* mine=dynamic_cast<Mine*>(&actor);
            if(!mine)throw std::logic_error("Runtime type2 lost actual Mine owner");
            return mine->human_reference()==static_cast<Instance*>(&human);
        });
    }
    void clear() noexcept {registry_.clear();}
    void finish(bool authority) { //00477A90; actor owners BEFORE both spawn buffers.
        if(!authority&&(mode_flags()&1))return;
        registry_.clear();
        std::vector<StaticItem>().swap(static_items_);
        std::vector<typename Items::Placed>().swap(retained_items_);
    }
    void clear_static_queue() { //004795E0; descriptor owners must outlive registered actors.
        for(const auto& queued:static_items_)if(queued.actor)
            throw std::logic_error("Static item queue cannot clear before its runtime actors");
        static_items_.clear();
    }
    void remove_static_item(void* object) noexcept { //Native rollback/teardown guard for the borrowed producer.
        for(auto index=static_items_.size();index!=0;)if(static_items_[--index].object==object) {
            if(auto* actor=static_items_[index].actor)registry_.remove(actor);
            static_items_.erase(static_items_.begin()+static_cast<std::ptrdiff_t>(index));
        }
    }
    const auto& active_actors() const {return registry_.active_actors();}
    bool owns(const Instance* actor)const{return registry_.owns(actor);}
    template<class Derived>void tick(bool authority,std::uint32_t elapsed,Derived& derived) {
        registry_.visit_active([&](Instance& actor){
            frames_.observe_data("execution","Runtime actor frame",authority?"authority":"presentation",actor.base().type());
            if(actor.base().type()==35)return; //00611050/60 genuine RET4 in00812750.
            if(actor.base().type()==2){static_cast<Mine&>(actor).update(authority,elapsed);return;}
            if(actor.base().type()==12){static_cast<DetachedItem&>(actor).update(authority,elapsed);return;}
            if(actor.base().type()==11){derived.update_loose_item(static_cast<LooseItemActor&>(actor),authority,elapsed);return;}
            throw std::runtime_error("Required runtime actor frame receiver is unavailable");
        });
    }
    Instance* identity(std::int32_t value) const {return registry_.identity(value);}
    std::uint32_t mode_flags() const {return activation_.mode_flags();}
    void bind_runtime_actor_frame(Frame frame,Instance* actor) {frames_.bind_scene_actor_frame(frame,actor);}
    void unbind_runtime_actor_frame(Frame frame) noexcept {frames_.unbind_scene_actor_frame(frame);}
    void activate_runtime_actor(Instance& actor,bool authority) {
        if(actor.base().type()==2)static_cast<Mine&>(actor).activate(authority);
        else if(actor.base().type()==12)static_cast<DetachedItem&>(actor).activate(authority);
        else if(actor.base().type()==11)static_cast<LooseItemActor&>(actor).activate(authority);
        else item(actor).activate(authority);
    }
    void deactivate_runtime_actor(Instance& actor,bool authority) {
        if(actor.base().type()==2)static_cast<Mine&>(actor).deactivate(authority);
        else if(actor.base().type()==12)static_cast<DetachedItem&>(actor).deactivate(authority);
        else if(actor.base().type()==11)static_cast<LooseItemActor&>(actor).deactivate(authority);
        else {
            for(auto& queued:static_items_)if(queued.actor==&actor) {
                queued.descriptor->actor=nullptr;queued.actor=nullptr;
            }
            item(actor).deactivate(authority);
        }
    }
    void remove_actor_presentation_references(Instance& actor) {activation_.remove_actor_presentation_references(actor);}
    void record_actor_cleanup_failure(std::exception_ptr failure) noexcept {frames_.record_actor_cleanup_failure(std::move(failure));}
private:
    DetachedItem& required_detached(Instance* actor){
        if(!registry_.owns(actor))throw std::logic_error("Detached Car item has no actual live registry owner");
        auto* detached=dynamic_cast<DetachedItem*>(actor);if(!detached)throw std::logic_error("Car launch requires its actual runtime type12");return *detached;
    }
    static ItemActor& item(Instance& actor) {
        if(actor.base().type()!=35)throw std::runtime_error("Runtime actor activation receiver is unavailable");
        return static_cast<ItemActor&>(actor);
    }
    GameActorServices& frames_;
    Activation& activation_;
    game::MissionNavigation& navigation_;
    const frontend::LocalizedTextCache& text_;
    Collisions& collisions_;
    typename Body::Registry& bodies_;
    Items& items_;
    std::vector<StaticItem> static_items_;
    std::vector<typename Items::Placed> retained_items_;
    std::uint32_t field74_=0;
    game::RuntimeActorRegistry<GameRuntimeActors> registry_{*this};
};

} //namespace hd2::host
#endif
