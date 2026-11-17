#ifndef HD2_GAME_SHOT_BULLET_RECORD_H
#define HD2_GAME_SHOT_BULLET_RECORD_H

#include "frontend/pre_mission_data.h"
#include "game/human_joint_owners.h"
#include <functional>
#include <memory>
#include <optional>

namespace hd2::game {
template<class Frames>class SceneActor;

//Module14's runtime record interface. Implemented records own their actual
//resources; an unavailable selected frame receiver must fail, never tick idle.
template<class Frames>class ShotRuntimeRecord {
public:
    virtual ~ShotRuntimeRecord()=default;
    virtual std::uint32_t kind()const=0;
    virtual unsigned authority_tick(std::uint32_t)=0;
    virtual void presentation_tick(std::uint32_t)=0;
    virtual void retire()=0;
};

//0044DB40/00453140's SAME reusable C0 bullet record. Unwritten native lanes are
//defined for source safety only. Actor/payload/frame pointers are typed borrowers
//or matching owners, never truncated into the original DWORD layout.
template<class Frames>class alignas(16) ShotBulletRecord final:public ShotRuntimeRecord<Frames> {
public:
    using Frame=typename Frames::Frame;
    using Owner=decltype(std::declval<Frames&>().create_shot_model());
    ShotBulletRecord(){words.set(0x5c,1u);words.set(0x64,0u);words.set(0x68,0u);words.set(0x6c,std::uint8_t{0});}
    ~ShotBulletRecord(){try{retire();}catch(...){if(frames_)frames_->record_actor_cleanup_failure(std::current_exception());}}
    ShotBulletRecord(const ShotBulletRecord&)=delete;ShotBulletRecord& operator=(const ShotBulletRecord&)=delete;
    std::uint32_t kind()const override{return 1;}
    Frame frame()const{return visual_?visual_->get():Frame{};}
    unsigned authority_tick(std::uint32_t elapsed)override{
        if(!authority_)throw std::logic_error("Bullet requires actual0044B910 authority receiver");
        return authority_(elapsed);
    }
    void presentation_tick(std::uint32_t elapsed)override{
        if(!presentation_)throw std::logic_error("Bullet requires actual0044B400 presentation receiver");
        presentation_(elapsed);
    }
    void retire()override{ //0044B380→00448360, release visual BEFORE stopping B4.
        visual_.reset();
        if(!frames_)return; //Cold native B4 is unwritten; do not interpret defined source padding as a live handle.
        if(words.template get<std::int32_t>(0xb4)!=-1){
            if(!stop_particle_)throw std::logic_error("Bullet particle requires actual module17 retirement");
            stop_particle_(words.template get<std::int32_t>(0xb4));
        }
        words.set(0xb4,std::int32_t{-1});
        authority_={};presentation_={};stop_particle_={};source=nullptr;payload=nullptr;
        frames_=nullptr;
    }
    template<class Services>void initialize(Frames& frames,Services& services,SceneActor<Frames>& shooter,
            const HumanVector& position,const HumanVector& direction,const frontend::PreMissionItemPayload& definition,
            Frame trace,bool silent,bool flare){ //004482A0 then0044B030; base visual argument is genuinely NULL.
        if(frame()||authority_||presentation_)throw std::logic_error("Retire pooled bullet before reinitializing");
        frames_=&frames;source=&shooter;payload=&definition;
        words.set(0xb4,std::int32_t{-1}); //Failure guard only, before the native initializer's later B4 store.
        stop_particle_=[&services](std::int32_t identity){services.stop_shot_particle(identity);};
        authority_=[this,&services](std::uint32_t elapsed){return services.advance_shot_bullet_authority(*this,elapsed);};
        presentation_=[this,&services](std::uint32_t elapsed){services.advance_shot_bullet_presentation(*this,elapsed);};
        words.set(0x20,position);words.set(0x10,direction);words.set(0x6c,std::uint8_t(silent));words.set(0x6d,std::uint8_t{0});
        words.set(0x58,payload_real(0xc));
        const auto speed=projectile_speed();
        auto velocity=direction;for(unsigned axis=0;axis!=3;++axis)velocity[axis]=static_cast<float>(static_cast<long double>(speed)*velocity[axis]);
        words.set(0x10,velocity);words.set(0x70,0u);words.set(0x54,0u);words.set(0x30,position);
        for(auto offset:{0x40u,0x44u,0x48u})words.set(offset,0u);
        words.set(0xb8,std::uint8_t{0});words.set(0xb4,std::int32_t{-1});
        if(trace){visual_.emplace(frames.create_shot_trace(trace,words.template get<HumanVector>(0x30)));require_visual();}
        else if(flare){visual_.emplace(frames.create_shot_flare(words.template get<HumanVector>(0x30)));require_visual();}
        words.set(0x6d,std::uint8_t{0});words.set(0xb0,std::uint8_t(flare));words.set(0x6e,std::uint8_t{0});
        words.set(0x78,std::uint8_t{0});words.set(0x74,0u);
    }
    float payload_real(std::size_t offset)const{
        if(!payload||offset>payload->bytes.size()||payload->bytes.size()-offset<4)
            throw std::runtime_error("Bullet requires actual complete weapon payload");
        return frontend::pre_mission_detail::Bytes::real(frontend::pre_mission_detail::Bytes::word(payload->bytes.data()+offset));
    }
    float projectile_speed()const{ //Payload virtual14:007E0280/007E0320/007E0380.
        if(!payload)throw std::logic_error("Bullet has no actual payload");
        if(payload->type==1||payload->type==2)return 0;
        if(payload->type==3)return payload_real(0x34);
        if(payload->type==4)return payload_real(0x30);
        throw std::runtime_error("Bullet requires actual projectile-speed dispatch");
    }
    HumanJointWords<0xc0> words;
    SceneActor<Frames>* source=nullptr;
    const frontend::PreMissionItemPayload* payload=nullptr;
private:
    void require_visual()const{if(!frame())throw std::runtime_error("Bullet requires actual driver visual");}
    Frames* frames_=nullptr;
    std::optional<Owner> visual_;
    std::function<unsigned(std::uint32_t)> authority_;
    std::function<void(std::uint32_t)> presentation_;
    std::function<void(std::int32_t)> stop_particle_;
};
} //namespace hd2::game
#endif
