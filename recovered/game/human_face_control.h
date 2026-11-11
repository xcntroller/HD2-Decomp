#ifndef HD2_GAME_HUMAN_FACE_CONTROL_H
#define HD2_GAME_HUMAN_FACE_CONTROL_H

#include "game/human_morph_tables.h"
#include "game/human_joint_owners.h"
#include "game/human_camera_state.h"
#include "game/human_state_frame_timers.h"
#include <cmath>
#include <optional>

namespace hd2::game {

//DD0+610:00780990,007809C0,00780AD0 and00781250. Owns the loading
//morph state. Human/model children/marker are borrowed. The frame advances
//blink/expression/breathing through the existing process SOUND pool.
template<class Frames,class Human,class Random>
class HumanFaceControl {
public:
    using Frame=typename Frames::Frame;
    using SoundOwner=decltype(std::declval<Frames&>().create_human_sound());
    HumanFaceControl(Frames& frames,const HumanMorphTables& tables,Random& random)
        :frames_(frames),tables_(tables),random_(random) {state_.set(8,std::int32_t{1});}
    void bind(Human& human) {
        human_=&human;morph_=Frame{};property_=static_cast<std::int32_t>(human.properties.field1a0);
        state_.set(8,property_);
        const auto model=human.base().frame();
        if(!model)return; //Original does not reset remaining state in this branch.
        for(auto frame=frames_.rigid_first_child(model);frame;frame=frames_.rigid_next_sibling(frame))
            if(frames_.visual_type(frame)==0x524d4d53u){morph_=frame;break;}
        marker_=frames_.actor_find(model,"f",0xffff);
        for(auto offset:{0x4cu,0x64u,0x78u})state_.set(offset,std::int32_t{100});
        for(auto offset:{0x14u,0x18u,0x50u})state_.set(offset,std::int32_t{1});
        for(auto offset:{0x1cu,0x20u,0x28u,0x2cu,0x30u,0x34u,0x3cu,0x44u,0x48u,
            0x54u,0x58u,0x5cu,0x60u,0x68u,0x6cu,0x70u,0x74u})state_.set(offset,std::int32_t{0});
        for(auto offset:{0x24u,0x38u,0x80u})state_.set(offset,std::int32_t{-1});
        state_.set(0x40,std::uint8_t{0});state_.set(0x84,-1.f);
        if(morph_){apply(true);state_.set(0x10,std::uint8_t{0});}
    }
    void schedule(bool enabled) { //00781250; return EAX's incidental quotient is unused.
        if(!enabled)state_.set(0x24,std::int32_t{-1});
        else if(state_.template get<std::int32_t>(0x24)==-1)
            state_.set(0x24,static_cast<std::int32_t>(random_()%human_face_delay_range)+human_face_delay_base);
    }
    void equipment_primary_command(std::uint32_t value){state_.set(0x78,value);} //00781300.
    void equipment_secondary_command(std::uint32_t value){state_.set(0x64,value);} //007812F0.
    template<class Pool>void close_sound(Pool& pool) { //00780AB0, typed native7C owner.
        if(sound_){pool.give(std::move(*sound_));sound_.reset();}
    }
    template<class Pool>void present(std::uint32_t elapsed,std::uint32_t mode,bool visible_detail,
            bool player,bool current_player,float animation_scale,Pool& pool){ //007816B0.
        if(!human_)throw std::logic_error("Human face frame requires its actual Human");
        constexpr std::int32_t blink_duration=128,mouth_transition=178,expression_duration=300,eye_transition=514;
        constexpr std::int32_t breath_limit=21428; //007808B0/007808E0/00780910/00780940/00780970, original 1.4 scale.
        const auto integer=[&](unsigned offset){return state_.template get<std::int32_t>(offset);};
        const auto word=[&](unsigned offset){return state_.template get<std::uint32_t>(offset);};
        const auto mark=[&]{state_.set(0x10,std::uint8_t{1});};
        const auto add=[&](unsigned offset){state_.set(offset,word(offset)+elapsed);};
        const auto mouth=[&](std::int32_t selector,std::int32_t duration){
            state_.set(0x30,0u);state_.set(0x2c,integer(0x28));state_.set(0x28,selector);state_.set(0x34,duration);
        };
        const auto blink=[&](std::int32_t selector,std::int32_t duration){
            state_.set(0x1c,0u);state_.set(0x18,integer(0x14));state_.set(0x14,selector);state_.set(0x20,duration);
        };
        if(integer(0x20)>0){mark();add(0x1c);}
        if(integer(0x24)!=-1){
            const auto remaining=word(0x24)-elapsed;
            state_.set(0x24,remaining);
            if(human_frame_signed_word(remaining)<1){
                if(!integer(0x20)&&!integer(0x48)){
                    state_.set(0x24,static_cast<std::int32_t>(random_()%human_face_delay_range)+human_face_delay_base);
                    blink(0,blink_duration);mark();
                }else state_.set(0x24,500u);
            }
        }
        const long double onset=8500.L/animation_scale; //00816870/0084E80C.
        const auto running=integer(0x38);
        if(running>=0){
            add(0x38);
            if(running<onset&&integer(0x38)>=onset&&!integer(0x34))mouth(1,mouth_transition);
        }else if(integer(0x3c)>0){
            const auto endurance=stamina_percent(); //005A62A0.
            const auto recovery=integer(0x3c);
            if(endurance>=100||recovery<onset)
                state_.set(0x3c,truncate(static_cast<long double>(recovery)-static_cast<long double>(human_frame_signed_word(elapsed))*1.3f));
            else{
                const float percent=static_cast<float>((recovery-onset)*100/(breath_limit-onset));
                if(100.L-endurance<=percent){
                    const float start=static_cast<float>(onset);
                    state_.set(0x3c,truncate((1.L-static_cast<long double>(endurance)*.01f)*(breath_limit-static_cast<long double>(start))+start));
                }
            }
            if(integer(0x28)==1&&integer(0x3c)<=onset){
                state_.set(0x40,std::uint8_t{0});mouth(0,mouth_transition);
            }
        }
        if(integer(0x34)>0){mark();add(0x30);}
        if(integer(0x48)>0)add(0x44);
        const auto pending=integer(0x4c);
        if(!integer(0x34)&&!integer(0x48)&&pending!=100){
            mouth(pending,expression_duration);state_.set(0x44,0u);
            constexpr std::array<unsigned,4> holds{{400,700,1200,2200}};
            const auto kind=word(0x50);
            if(kind<holds.size())state_.set(0x48,expression_duration+holds[kind]);
            if(!integer(0x20))blink(integer(0x14),expression_duration);
            if(pending>=2&&pending<=4)state_.set(0x14,pending+1);
            state_.set(0x4c,100u);
        }
        for(unsigned offset:{0x54u,0x68u}){
            if(integer(offset+12)>0){mark();add(offset+8);}
            const auto request=integer(offset+16);
            if(request!=100&&!integer(offset+12)){
                state_.set(offset+8,0u);state_.set(offset+12,eye_transition);
                state_.set(offset+4,integer(offset));state_.set(offset,request);state_.set(offset+16,100u);
            }
        }
        if(state_.template get<std::uint8_t>(0x10)&&morph_&&visible_detail){
            state_.set(0x10,std::uint8_t{0});apply(false);
        }
        if(mode&2u)present_breathing(elapsed,onset,breath_limit,player,current_player,pool);
        if(integer(0x20)>0&&integer(0x1c)>=integer(0x20)){
            state_.set(0x1c,0u);state_.set(0x20,0u);
            if(!integer(0x14))blink(1,blink_duration);
        }
        if(integer(0x34)>0&&integer(0x30)>=integer(0x34)){
            state_.set(0x30,0u);state_.set(0x34,0u);
        }
        if(integer(0x48)&&integer(0x44)>=integer(0x48)){
            if(!integer(0x34)){
                state_.set(0x48,0u);state_.set(0x44,0u);mouth(0,expression_duration);
            }
            if(!integer(0x20))blink(1,integer(0x34));
        }
        for(unsigned offset:{0x54u,0x68u})if(integer(offset+12)>0&&integer(offset+8)>=integer(offset+12)){
            state_.set(offset+8,0u);state_.set(offset+12,0u);
        }
    }
    void apply(bool force) {
        if(!morph_)throw std::runtime_error("Human face has no actual morph visual");
        int current=19,previous=19;
        for(unsigned stage=0;stage!=4;++stage) {
            constexpr std::array<unsigned,4> starts{{0x14,0x28,0x54,0x68}};
            const auto offset=starts[stage];
            const auto duration=state_.template get<std::int32_t>(offset+12);
            if(!duration&&!force)continue;
            auto ratio=duration?static_cast<float>(static_cast<long double>(state_.template get<std::int32_t>(offset+8))/duration):1.f;
            if(ratio>1)ratio=1;
            current=select(stage,state_.template get<std::int32_t>(offset),current,false);
            previous=select(stage,state_.template get<std::int32_t>(offset+4),previous,true);
            const auto& first=tables_.record(property_,current);
            const auto& second=tables_.record(property_,previous);
            if(first.loaded!=1||second.loaded!=1)return; //Aborts ALL later groups.
            if(stage==1&&current==previous)continue;
            constexpr std::array<unsigned,4> groups{{1,0,4,3}};
            const auto group=groups[stage];
            const auto count=frames_.human_morph_count(morph_);
            if(!first.metadata[12+group]||!second.metadata[12+group])continue;
            if(count>8)throw std::runtime_error("Human morph visual exceeds its eight-coefficient group");
            const float complement=1.f-ratio;
            for(std::uint32_t index=0;index<count;++index) {
                const auto slot=group*8+index;
                const auto value=static_cast<float>(static_cast<long double>(ratio)*first.coefficients[slot]+
                    static_cast<long double>(complement)*second.coefficients[slot]);
                frames_.human_morph_weight(morph_,group,index,value);
            }
        }
    }
    const auto& state() const {return state_;}
    Frame morph() const {return morph_;}
    Frame marker() const {return marker_;}
private:
    static std::int32_t truncate(long double value){
        if(!std::isfinite(value)||value< -0x1p63L||value>=0x1p63L)return 0;
        return human_frame_signed_word(static_cast<std::uint32_t>(static_cast<std::int64_t>(value)));
    }
    std::int32_t stamina_percent()const{ //005A62A0.
        if(!human_->properties.stats_ec[0]||human_->activation.field14c==0.f)return 0;
        return truncate(human_->activation.field14c/(static_cast<long double>(human_frame_signed_word(human_->properties.stats_ec[2]))*.01f));
    }
    template<class Pool>void present_breathing(std::uint32_t elapsed,long double onset,std::int32_t limit,
            bool player,bool current_player,Pool& pool){ //00781BA0.
        const auto running=state_.template get<std::int32_t>(0x38);
        const auto amount=truncate((running==-1?state_.template get<std::int32_t>(0x3c):running)-onset);
        const float intensity=static_cast<float>(amount/(limit-onset));
        auto timer=state_.template get<std::int32_t>(0x80);
        if(timer>=0){
            timer=human_frame_signed_word(static_cast<std::uint32_t>(timer)-elapsed);
            state_.set(0x80,timer);
        }
        if(sound_&&(amount<=0||!current_player||human_camera_focus(*human_))){
            if(timer<1)close_sound(pool);
            return;
        }
        if(amount<=0)return;
        const bool starting=!sound_;
        if(!sound_||timer<1){
            close_sound(pool);
            const bool strong=intensity>=.8f; //0080FAD4.
            state_.set(0x40,static_cast<std::uint8_t>(strong));
            if(player){
                auto sound=pool.play(strong?3u:2u,marker_,{{0,0,.3f}});
                if(sound){
                    state_.set(0x80,frames_.human_sound_duration(sound.get()));
                    state_.set(0x84,frames_.human_sound_volume(sound.get()));
                    sound_.emplace(std::move(sound));
                }
            }
        }
        if(sound_){
            const auto volume=state_.template get<float>(0x84);
            frames_.human_sound_volume(sound_->get(),starting?0.f:intensity<.2f?intensity*volume*5.f:volume);
        }
    }
    int select(unsigned stage,int command,int fallback,bool previous) const {
        if(stage==0) {
            switch(command){case 0:return 8;case 1:return 7;case 3:return 4;case 4:return 5;case 5:return 6;default:return fallback;}
        }
        if(stage==1) {
            switch(command) {
            case 0:return 1;
            case 1:
                if(state_.template get<std::int32_t>(0x38)<1&&state_.template get<std::int32_t>(0x3c)<1)return 0;
                return previous?2:3-(state_.template get<std::uint8_t>(0x40)!=0);
            case 2:return 4;case 3:return 5;case 4:return 6;default:return fallback;
            }
        }
        return command>=0&&command<=2?9+static_cast<int>(stage==3)+command*2:fallback;
    }
    Frames& frames_;const HumanMorphTables& tables_;Random& random_;
    Human* human_=nullptr;Frame morph_{},marker_{};std::int32_t property_=1;
    std::optional<SoundOwner> sound_; //Native7C, actual retained breathing sound.
    HumanJointWords<0x88> state_; //Native pointer slots are represented above.
};
} //namespace hd2::game
#endif
