#ifndef HD2_GAME_MINE_SCENE_ACTOR_H
#define HD2_GAME_MINE_SCENE_ACTOR_H
#include "game/scene_actor.h"
#include "game/rigid_body.h"
#include "game/actor_activation_math.h"
#include "game/shot_effect.h"
#include <optional>
#include <vector>
namespace hd2::game {
//Original runtime type2, factory005FDE30 allocates0x120;005FDEC0/005FDF60.
//Runtime registry owns it. Human9C, model20, collision grid and environment
//remain borrowed; deactivate BEFORE the containing World/grid/frames retire.
template<class Frames,class Environment,class Collisions>class MineSceneActor final:public SceneActor<Frames> {
public:
    using Frame=typename Frames::Frame;using Vector=std::array<float,4>;
    using Descriptor=RigidBodyCollisionDescriptor<Frame>;
    MineSceneActor(Frames& frames,Environment& environment,Collisions& collisions)
        :SceneActor<Frames>(frames,2),environment_(environment),collisions_(collisions){}
    ~MineSceneActor()override{
        //Native ordered retirement unregisters in005FDFD0. Source fallback
        //also retracts an acquired/failed registration before descriptor free.
        collisions_.remove_for_cleanup(descriptor_);
        std::vector<unsigned char>().swap(table108_); //005FDF60 frees table storage BEFORE common/base retirement.
    }
    bool initialize(Frame frame)override{ //005FDF90: name BEFORE common initialization, HRESULT ignored.
        if(!frame)throw std::logic_error("Mine initialization requires actual frame");
        this->backend_.mine_model_name(frame,"Actor_mine");return this->base_.initialize_frame(frame);
    }
    void load(SerializedBytes&)override{} //005FDF20 genuinely returns0/RET4; generic loading caller ignores return.
    void activate(bool authority){ //005FDFB0; no native binding/counter defaults added here.
        registered99_=false;registered99_defined_=true;this->base_.activate_frame(authority,environment_);
    }
    void deactivate(bool authority){ //005FDFD0 unregister BEFORE99 clear/common00610510.
        if(!registered99_defined_)throw std::logic_error("Mine deactivation requires its actual native99 producer");
        if(registered99_)collisions_.remove(descriptor_);
        registered99_=false;this->base_.deactivate_frame(authority,environment_);
    }
    void bind(SceneActor<Frames>* human,std::int32_t definition,std::uint32_t material){ //005FE010 ordered fields100/94/98/90/9C.
        material100_=material;elapsed94_=0;active98_=false;definition90_=definition;human9c_=human;bound_=true;
        const auto model=this->base_.frame();
        if(definition==70||definition==71){
            const auto world=this->backend_.actor_world_matrix(model);
            auto direction=actor_activation_direction(Vector{{world[8],world[9],world[10],world[11]}});
            const float distance=definition==70?.07000000029802322f:.07999999821186066f;
            for(unsigned axis=0;axis!=3;++axis)direction[axis]=static_cast<float>(static_cast<long double>(direction[axis])*distance);
            const auto position=this->backend_.actor_world_matrix(model);std::array<float,3> moved{};
            for(unsigned axis=0;axis!=3;++axis)moved[axis]=static_cast<float>(static_cast<long double>(position[12+axis])-direction[axis]);
            this->backend_.object_position(model,moved);
        }
        this->backend_.object_visible(model,true);this->backend_.update_scene_object(model);
        this->backend_.place_human_reaction_model(model);this->backend_.update_scene_object(model);
        this->base_.name(environment_.mine_caption(definition)); //Required definition/localization LAST, after model mutations.
    }
    SceneActor<Frames>* human_reference()const{require_binding();return human9c_;}
    std::int32_t material()const{require_binding();return signed_word(material100_);} //005FEEB0 ignores its frame argument.
    void update(bool authority,std::uint32_t elapsed){ //005FEE70/90 select the SAME005FEE50 timer path.
        if(authority?(environment_.mode_flags()&2u)!=0:(environment_.mode_flags()&2u)==0)return;
        require_binding();if(signed_word(elapsed94_)<0)return;
        elapsed94_+=elapsed; //005FEE10 wraps DWORD before signed FILD/2000 comparison.
        if(signed_word(elapsed94_)>=2000){elapsed94_=0xffffffffu;arm();}
    }
    void arm(){ //005FEBB0 takes NO stack arguments, despite older decompiler exports.
        require_binding();if(descriptor_.bucket)throw std::logic_error("Mine rearm would overwrite a registered native descriptor");
        const auto model=this->base_.frame();auto world=this->backend_.actor_world_matrix(model);
        Vector position{{world[12],static_cast<float>(static_cast<long double>(world[13])+.20000000298023224f),world[14],0}};
        this->base_.world_position(position); //Native fourth stack lane is unwritten; source guard0, no ABI value claim.
        world=this->backend_.actor_world_matrix(model);
        this->base_.world_direction(actor_activation_direction(Vector{{world[8],world[9],world[10],world[11]}}));
        descriptor_=Descriptor{};descriptor_.flags=0x42;descriptor_.position=this->base_.world_position();
        descriptor_.radius=definition90_==71?.4000000059604645f:.15000000596046448f;
        descriptor_.frame=model;descriptor_.mask=0x40000;descriptor_.actor=this;
        collisions_.insert(descriptor_);elapsed94_=0xffffffffu;active98_=true;registered99_=true;registered99_defined_=true;
    }
    template<class Packet>void receive(Packet& packet){ //005FEB50's two-bit commands; authority005FEBA0 is genuine RET4.
        const auto command=packet.read(2);
        if(command==0){require_binding();active98_=false;environment_.mine_effect(definition90_,this->base_.world_position(),this->base_.world_direction());environment_.remove(this);}
        else if(command==1){active98_=false;environment_.remove(this);} //005FEEC0; no binding read before removal.
    }
    void contact(SceneActor<Frames>* touching,std::uint32_t){ //005FEFB0 RET8; second stack word is genuinely unused.
        auto& environment=environment_; //Native uses globals after explosion callbacks, even if this Mine retires during damage.
        require_binding();if(!active98_||human9c_==touching)return;
        if(!touching)throw std::logic_error("Armed Mine contact requires actual touching actor");
        const auto kind=touching->base().type();
        if((kind==3&&definition90_==71)||(kind==4&&definition90_==70))return;
        active98_=false;
        if(!registered99_defined_)throw std::logic_error("Armed Mine contact requires actual99 producer");
        if(registered99_)collisions_.remove(descriptor_);
        auto* source=human9c_;registered99_=false;
        if(!source)source=kind==3?touching:environment_.mine_contact_source(touching);
        const auto& definition=environment_.mine_contact_definition(definition90_); //Pointer lookup BEFORE wire; fields read AFTER.
        environment_.publish_mine_contact(this->base_.id10(),0);
        auto position=this->base_.world_position();position[1]=static_cast<float>(static_cast<long double>(position[1])+.20000000298023224f);
        const auto blast=mine_blast_definition(definition);
        environment.mine_explosion(source,position,blast.radius,blast.power,blast.sound);
        if(!(environment.mode_flags()&2u))environment.remove(this);
    }
    void disarm(){ //005FEEE0: no binding read; collision unregister waits for actual removal.
        active98_=false;environment_.publish_mine_contact(this->base_.id10(),1);
        if(!(environment_.mode_flags()&2u))environment_.remove(this);
    }
    bool active()const{require_binding();return active98_;}
    bool collision_registered()const{if(!registered99_defined_)throw std::logic_error("Mine99 has no actual producer");return registered99_;}
    std::uint32_t elapsed()const{require_binding();return elapsed94_;}const Descriptor& descriptor()const{return descriptor_;}
private:
    static std::int32_t signed_word(std::uint32_t value){std::int32_t out;std::memcpy(&out,&value,4);return out;}
    void require_binding()const{if(!bound_)throw std::logic_error("Mine fields require actual005FE010 binding");}
    Environment& environment_;Collisions& collisions_;Descriptor descriptor_;
    std::vector<unsigned char> table108_; //Cold108/10C/110 owner only; no unknown element interpretation/producer is exposed.
    SceneActor<Frames>* human9c_=nullptr;std::int32_t definition90_=0;std::uint32_t material100_=0,elapsed94_=0;
    //Only activated/bound producers define native99/98/90/94/9C/100. These cold
    //backing values are source guards, not claimed original constructor writes.
    bool bound_=false,active98_=false,registered99_=false,registered99_defined_=false;
};
}
#endif
