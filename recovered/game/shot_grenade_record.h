#ifndef HD2_GAME_SHOT_GRENADE_RECORD_H
#define HD2_GAME_SHOT_GRENADE_RECORD_H
#include "game/shot_bullet_record.h"
#include <algorithm>

namespace hd2::game {
//00450400/0044CFF0 specialized6A0 projectile. Body is the SAME actual rigid
//type/registry/buckets used by the containing World, not a second simulator.
template<class Frames,class Body>class ShotGrenadeRecord final:public ShotRuntimeRecord<Frames> {
public:
    using Frame=typename Frames::Frame;using Owner=decltype(std::declval<Frames&>().create_shot_model());
    template<class Factory>ShotGrenadeRecord(Frames& frames,Factory&& factory):frames_(frames){
        words.set(0x64,0u);words.set(0x68,0u);words.set(0x6c,std::uint8_t{0});
        body_=factory();if(!body_)throw std::runtime_error("Grenade requires actual rigid body");
        words.set(0x5c,3u);words.set(0x680,0u);words.set(0x684,1.f);words.set(0x688,0u);words.set(0x690,std::int32_t{-1});
    }
    ~ShotGrenadeRecord(){try{retire();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}}
    std::uint32_t kind()const override{return 3;}
    Frame frame()const{return visual_?visual_->get():Frame{};}
    Body& body(){if(!body_)throw std::logic_error("Grenade rigid body has retired");return *body_;}
    unsigned authority_tick(std::uint32_t elapsed)override{
        if(!authority_)throw std::runtime_error("Grenade requires actual0044D260 authority receiver");
        return authority_(elapsed);
    }
    void presentation_tick(std::uint32_t elapsed)override{
        if(!presentation_)throw std::runtime_error("Grenade requires actual0044D240 visual receiver");
        presentation_(elapsed);
    }
    void retire()override{ //0044D210, then004505B0's embedded body destructor.
        if(body_)body_->close();
        visual_.reset();body_.reset();authority_={};presentation_={};source=nullptr;payload=nullptr;
    }
    template<class World,class Item,class Human>void initialize(World& world,Item& item,Human& human,
            const HumanVector& position,const HumanVector& direction,std::uint32_t argument){
        authority_=[this,&world](std::uint32_t elapsed){return world.advance_shot_grenade_authority(*this,elapsed);};
        presentation_=[this,&world](std::uint32_t elapsed){world.advance_shot_grenade_presentation(*this,elapsed);};
        words.set(0x678,0u);words.set(0x10,direction);source=&human;payload=&item.definition().payloads[0];
        words.set(0x20,position);words.set(0x6c,std::uint8_t(argument));words.set(0x6d,std::uint8_t{0});words.set(0x58,payload_real(0xc));
        if(const auto model=item.model()){
            visual_.emplace(frames_.create_shot_shell(model));if(!frame())throw std::runtime_error("Grenade model clone is missing");
            frames_.shot_projectile_root(frame());
        }
        if(!frame())throw std::runtime_error("Grenade requires actual item model before native A8");
        float speed=static_cast<float>(static_cast<long double>(item.field98)*payload_real(0x34));if(speed<5.f)speed=5.f;
        auto velocity=direction;for(unsigned axis=0;axis!=3;++axis)velocity[axis]=static_cast<float>(static_cast<long double>(speed)*velocity[axis]);
        words.set(0x10,velocity);frames_.shot_grenade_parameter(frame(),1.f);frames_.configure_shot_capsule(frame(),velocity,position);
        words.set(0x670,0u);words.set(0x674,payload_word(0x48));
        body().bind(frame());typename Body::Parameters parameters;
        parameters.motion=velocity;parameters.mass=2;parameters.coefficient=.5f;parameters.flag40=true;parameters.flag41=false;parameters.callback_context=this;
        parameters.callback=[this,&world](unsigned kind,void* context,typename Body::ContactEvent* event){
            if(context!=this)throw std::logic_error("Grenade contact lost its actual record context");
            return world.shot_grenade_contact(*this,kind,event);
        };
        body().configure(parameters);body().reset();words.set(0x698,0u);words.set(0x694,item.definition().fields38[0]);
    }
    HumanJointWords<0x6a0> words;SceneActor<Frames>* source=nullptr;const frontend::PreMissionItemPayload* payload=nullptr;
private:
    std::uint32_t payload_word(std::size_t offset)const{
        if(!payload||offset>payload->bytes.size()||payload->bytes.size()-offset<4)throw std::runtime_error("Grenade requires complete native weapon payload");
        return frontend::pre_mission_detail::Bytes::word(payload->bytes.data()+offset);
    }
    float payload_real(std::size_t offset)const{return frontend::pre_mission_detail::Bytes::real(payload_word(offset));}
    Frames& frames_;std::unique_ptr<Body> body_;std::optional<Owner> visual_;
    std::function<unsigned(std::uint32_t)> authority_;std::function<void(std::uint32_t)> presentation_;
};
} //namespace hd2::game
#endif
