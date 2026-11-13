#ifndef HD2_GAME_SHOT_CAPSULE_RECORD_H
#define HD2_GAME_SHOT_CAPSULE_RECORD_H
#include "game/shot_bullet_record.h"
#include "game/human_state_frame_timers.h"

namespace hd2::game {
//00451670/00448CE0's selected weapon branch: particle91, incoming sound=false,
//special90=false. Other callers' incoming-sound production is not inferred.
template<class Frames>class ShotCapsuleRecord final:public ShotRuntimeRecord<Frames> {
public:
    using Frame=typename Frames::Frame;using Owner=decltype(std::declval<Frames&>().create_shot_model());
    ShotCapsuleRecord(){words.set(0x5c,4u);words.set(0x64,0u);words.set(0x68,0u);words.set(0x6c,std::uint8_t{0});}
    ~ShotCapsuleRecord(){try{retire();}catch(...){if(frames_)frames_->record_actor_cleanup_failure(std::current_exception());}}
    std::uint32_t kind()const override{return 4;}
    Frame frame()const{return visual_?visual_->get():Frame{};}
    unsigned authority_tick(std::uint32_t elapsed)override{
        if(!authority_)throw std::runtime_error("Capsule requires actual00449290 authority receiver");
        return authority_(elapsed);
    }
    void presentation_tick(std::uint32_t elapsed)override{
        if(!presentation_)throw std::runtime_error("Capsule requires actual00449030 visual receiver");
        presentation_(elapsed);
    }
    void retire()override{ //00448FC0: sound, particle while model lives, model, camera9, common base.
        if(!frames_)return;
        if(frame()){
            if(words.template get<std::int32_t>(0x70)>=0){
                if(!stop_)throw std::logic_error("Capsule requires actual module17 stop");
                stop_(words.template get<std::int32_t>(0x70));
            }
            words.set(0x70,std::int32_t{-1});visual_.reset();
        }
        if(words.template get<std::uint8_t>(0x88)){
            if(!camera_)throw std::logic_error("Capsule requires actual camera9 receiver");
            camera_();
        }
        visual_.reset();authority_={};presentation_={};stop_={};camera_={};source=nullptr;payload=nullptr;frames_=nullptr;
    }
    template<class World,class Human>void initialize(Frames& frames,World& world,Human& human,
            const HumanVector& position,const HumanVector& direction,const frontend::PreMissionItemPayload& definition,
            Frame model,bool silent){
        frames_=&frames;words.set(0x70,std::int32_t{-1});words.set(0x88,std::uint8_t{0}); //Partial-failure guards, no native early-store credit.
        stop_=[&world](std::int32_t identity){world.stop_shot_particle(identity);};camera_=[&world]{world.stop_capsule_camera();};
        authority_=[this,&world](std::uint32_t elapsed){return world.advance_shot_capsule_authority(*this,elapsed);};
        presentation_=[this,&world](std::uint32_t elapsed){world.advance_shot_capsule_presentation(*this,elapsed);};
        source=&human;payload=&definition;words.set(0x20,position);words.set(0x10,direction);words.set(0x6c,std::uint8_t(silent));
        words.set(0x6d,std::uint8_t{0});words.set(0x58,payload_real(0xc));
        if(model){visual_.emplace(frames.create_shot_shell(model));if(!frame())throw std::runtime_error("Capsule requires actual model clone");frames.shot_projectile_root(frame());}
        auto velocity=direction;for(unsigned axis=0;axis!=3;++axis)velocity[axis]=static_cast<float>(static_cast<long double>(velocity[axis])*payload_real(0x34));
        words.set(0x10,velocity);
        if(frame()){
            frames.configure_shot_capsule(frame(),velocity,position);
            auto reverse=direction;for(unsigned axis=0;axis!=3;++axis)reverse[axis]=-reverse[axis];
            words.set(0x70,world.emit_human_particle(91u,position,reverse,frame(),true,true));
        }else words.set(0x70,std::int32_t{-1});
        words.set(0x74,0u);words.set(0x54,0u);words.set(0x30,position);
        for(auto offset:{0x40u,0x44u,0x48u,0x7cu,0x80u,0x84u,0x8cu})words.set(offset,0u);
        words.set(0x78,std::uint8_t{0});words.set(0x88,std::uint8_t{0});words.set(0x90,std::uint8_t{0});
    }
    HumanJointWords<0xa0> words;SceneActor<Frames>* source=nullptr;const frontend::PreMissionItemPayload* payload=nullptr;
private:
    float payload_real(std::size_t offset)const{
        if(!payload||offset>payload->bytes.size()||payload->bytes.size()-offset<4)throw std::runtime_error("Capsule requires complete native weapon payload");
        return frontend::pre_mission_detail::Bytes::real(frontend::pre_mission_detail::Bytes::word(payload->bytes.data()+offset));
    }
    Frames* frames_=nullptr;std::optional<Owner> visual_;
    std::function<unsigned(std::uint32_t)> authority_;std::function<void(std::uint32_t)> presentation_;
    std::function<void(std::int32_t)> stop_;std::function<void()> camera_;
};
} //namespace hd2::game
#endif
