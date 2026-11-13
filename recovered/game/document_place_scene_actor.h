#ifndef HD2_GAME_DOCUMENT_PLACE_SCENE_ACTOR_H
#define HD2_GAME_DOCUMENT_PLACE_SCENE_ACTOR_H
#include "game/scene_actor.h"
#include "game/actor_properties.h"
#include "game/rigid_body.h"
#include "game/prepared_spawn_packets.h"
#include "frontend/cleanup.h"
#include <functional>
#include <optional>
#include <utility>
namespace hd2::game {
//Actual type23/0060DB70. Descriptor and visual belong to this actor; World,
//catalogue, item cache and packet stream are borrowed from the containing World.
template<class Backend>class DocumentPlaceSceneActor final:public SceneActor<Backend>{
public:
    using Frame=typename Backend::Frame;using Descriptor=RigidBodyCollisionDescriptor<Frame>;
    using Visual=decltype(std::declval<Backend&>().create_document_visual(0u));
    explicit DocumentPlaceSceneActor(Backend& backend):SceneActor<Backend>(backend,23){}
    ~DocumentPlaceSceneActor()override{if(remove_collision_)remove_collision_();}
    bool initialize(Frame frame)override{return this->base_.initialize_frame(frame);} //0060DC00→00610200.
    void load(SerializedBytes& bytes)override{properties.load(bytes);}
    template<class Activation,class Collisions>void activate(bool authority,Activation& activation,Collisions& collisions){ //0060DC70.
        this->base_.activate_frame(authority,activation);if(flag90)return;
        if(activation_started_)throw std::logic_error("Retire partial DocumentPlace activation before retry");
        activation_started_=true;
        if(descriptor_.bucket)throw std::logic_error("Retire partial DocumentPlace activation before retry");
        if(collision_owner_&&collision_owner_!=&collisions)throw std::logic_error("DocumentPlace collision owner changed");
        static_assert(noexcept(collisions.remove_for_cleanup(descriptor_)),"DocumentPlace cleanup must not throw");
        if(!collision_owner_){remove_collision_=[this,&collisions]()noexcept{collisions.remove_for_cleanup(descriptor_);};collision_owner_=&collisions;}
        const auto frame=this->base_.frame();const auto bounds=this->backend_.physics_object_bounds(frame,false);
        if(!(bounds.minimum[0]<bounds.maximum[0]))throw std::runtime_error("DocumentPlace native collision centre is unwritten for invalid bounds");
        descriptor_=Descriptor{};descriptor_.flags=0x40;descriptor_.mask=2;descriptor_.frame=frame;descriptor_.actor=this;
        descriptor_.minimum=bounds.minimum;descriptor_.maximum=bounds.maximum;
        descriptor_.matrix=this->backend_.actor_world_matrix_pointer(frame);
        inverse_=this->backend_.car_inverse_matrix(this->backend_.actor_world_matrix(frame));descriptor_.inverse=&inverse_;
        const auto scale=this->backend_.car_scale(frame);const auto yz=scale[1]>scale[2]?scale[1]:scale[2];
        descriptor_.radius=static_cast<float>(static_cast<long double>(scale[0]>yz?scale[0]:yz)*bounds.radius);
        descriptor_.position=this->backend_.car_transform(bounds.centre,this->backend_.actor_world_matrix(frame));
        set_state(0,activation,collisions);flag90=true; //Native writes90 only after state effects.
    }
    template<class Activation,class Collisions>void deactivate(bool authority,Activation& activation,Collisions& collisions){ //0060DF20.
        frontend::CleanupErrors errors;
        if(flag90||activation_started_)errors.attempt([&]{set_state(3,activation,collisions);});
        //Also retire a failed insertion/visual prefix, without claiming activation.
        if(descriptor_.bucket&&remove_collision_){remove_collision_();}
        flag140=false;visual_.reset();flag90=false;activation_started_=false;
        errors.attempt([&]{this->base_.deactivate_frame(authority,activation);});errors.rethrow();
    }
    template<class Activation,class Collisions>void set_state(std::uint32_t state,Activation& activation,Collisions& collisions){ //0060EB60.
        if(state==state148)return;
        state148=state;
        switch(state){
        case 0:this->base_.name(activation.actor_localized(properties.caption94));this->backend_.object_visible(this->base_.frame(),true);
            if(!flag140){collisions.insert(descriptor_);}
            flag140=true;return;
        case 1:this->base_.name(activation.actor_item_caption(properties.item98));ensure_visual(activation);
            if(flag140){collisions.remove(descriptor_);}
            flag140=false;return;
        case 2:if(visual_)this->backend_.object_visible(visual_->get(),false);this->backend_.object_visible(this->base_.frame(),false);
            if(flag140){collisions.remove(descriptor_);}
            flag140=false;return;
        case 3:if(flag140){collisions.remove(descriptor_);}
            flag140=false;visual_.reset();return;
        default:return;
        }
    }
    template<class Activation>void ensure_visual(Activation& activation){ //0060E860.
        auto& frames=this->backend_;const auto base=this->base_.frame();
        if(!visual_){const auto source=activation.document_visual_template(properties.item98);
            auto owner=frames.create_document_visual(0x5f4a424fu);if(!owner)throw std::runtime_error("Cannot create actual document visual");
            visual_.emplace(std::move(owner));frames.copy_document_visual(visual_->get(),source);}
        const auto frame=visual_->get();frames.document_visual_link(frame,base);
        frames.mounted_local_direction(frame,typename Backend::Vector{{0,0,1,0}});
        frames.car_sound_head(frame,typename Backend::Vector{{0,0,0,0}});frames.object_visible(frame,true);
        frames.update_scene_object(frame);frames.update_scene_object(base);frames.actor_link_preserving_world(frame,Frame{});
        frames.place_human_reaction_model(frame);frames.update_scene_object(frame);frames.update_scene_object(base);frames.object_visible(base,false);
    }
    template<class Human,class Activation,class Collisions>bool place(Human& human,Activation& activation,Collisions& collisions){ //0060E9B0.
        if(state148)return false;
        const auto identity=activation.document_item_identity(human,properties.item98,true);
        if(!identity)return false;
        activation.document_remove_item(human,*identity);
        auto& packet=activation.session_broadcast_packets().begin(13,0);
        packet.write(static_cast<std::uint32_t>(this->base_.id10()),10);
        packet.write(0,0); //Original log(0)→x87 indefinite QWORD→low DWORD0. NOT a two-bit tag.
        packet.write(static_cast<std::uint32_t>(human.base().id10()),10);packet.write(static_cast<std::uint32_t>(*identity),16);packet.finish_broadcast();
        set_state(1,activation,collisions);return true;
    }
    template<class Activation,class Collisions>void receive_loading_packet(SessionPacketStream& packet,Activation& activation,Collisions& collisions){ //0060E420/0060EA70.
        const auto event=packet.read(0);
        if(event==0){const auto human=packet.read(10);const auto item=static_cast<std::int16_t>(packet.read(16));
            if(!(activation.mode_flags()&1u)&&activation.document_receive_remove(human,item))set_state(1,activation,collisions);
        }else if(event==1)set_state(packet.read(2),activation,collisions); //Native branch unreachable with zero-bit read.
    }
    void write_snapshot(SessionPacketStream& packet)const{packet.write(state148,2);} //0060E4D0.
    template<class Activation>void read_snapshot(SessionPacketStream& packet,Activation& activation){if(packet.read(2)==1)ensure_visual(activation);} //0060E510 does not store state.
    const Descriptor& collision_descriptor()const{return descriptor_;}Frame visual()const{return visual_?visual_->get():Frame{};}
    DocumentPlaceActorProperties properties;bool flag90=false,flag140=false;std::uint32_t state148=3;
private:
    Descriptor descriptor_{};typename Backend::Matrix inverse_{};std::optional<Visual> visual_;
    std::function<void()> remove_collision_;void* collision_owner_=nullptr;bool activation_started_=false;
};
} //namespace hd2::game
#endif
