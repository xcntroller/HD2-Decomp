#ifndef HD2_GAME_HUMAN_LOADING_ACTIVATION_H
#define HD2_GAME_HUMAN_LOADING_ACTIVATION_H

#include "game/human_loading_stats.h"
#include "game/human_joint_math.h"
#include "game/human_loading_lifetime.h"
#include "game/speech_definitions.h"
#include "frontend/cleanup.h"
#include <cstdio>

namespace hd2::game {

//0059F2C0/0059FA60 loading composition. One instance borrows the SAME Human,
//inline98, B0 owner/factory, shared profile/speech/pools and Sight/SOUND owners.
//Services must implement real Players/difficulty/formation/attachment/proximity
//receivers; no successful defaults or containing-world readiness are provided.
//Explicit deactivate(false/true) precedes destruction of these dependencies.
template<class Frames,class Human,class Inline,class States,class Services,class Store,
         class Effects,class Speech,class Sight,class Sound,class Proximity>
class HumanLoadingActivation : public HumanLoadingLifetime {
public:
    HumanLoadingActivation(Frames& frames,Human& human,Inline& control,States& states,Services& services,
            Store& profiles,Effects& effects,Speech& speech,Sight& sight,Sound& sound,Proximity& proximity)
        :frames_(frames),human_(human),control_(control),states_(states),services_(services),profiles_(profiles),
         effects_(effects),speech_(speech),sight_(sight),sound_(sound),proximity_(proximity){
        if(human_.loading_lifetime)throw std::logic_error("Human already has its containing loading owner");
        human_.loading_lifetime=this;
    }
    ~HumanLoadingActivation()override{if(human_.loading_lifetime==this)human_.loading_lifetime=nullptr;}
    HumanLoadingActivation(const HumanLoadingActivation&)=delete;
    HumanLoadingActivation& operator=(const HumanLoadingActivation&)=delete;
    void activate(bool authority)override {
        human_.base().activate_frame(authority,services_);
        const auto mode=services_.mode_flags();
        if((mode&7u)!=7u&&!authority&&(mode&2u)&&services_.human_current_player_id()==human_.base().id10()) {
            const auto delay=services_.human_respawn_delay_seconds();
            if(delay>0)services_.hud_human_respawn(0x4f59u,static_cast<std::uint32_t>(delay)*1000u);
        }
        if(!human_.flag90) {
            if(local_started_)throw std::logic_error("Retire partial Human activation before retry");
            initialize_local(mode);
        }
        auto& fields=human_.activation;
        if(!authority) {
            fields.colour1e0[0]=fields.colour1e0[1]=fields.colour1e0[2]=1;fields.field200=0;
            //005ABA70 is genuine constant1. W was uninitialized stack data.
            if((mode&7u)!=7u&&signed_word(human_.properties.stats_ec[0])<1)
                services_.human_initial_dead_notification(human_); //005A15D0(0E,0,0,-1,zero,zero).
        }else {
            //005ABA80 is genuine constant1, then Human16C resets before registration.
            fields.field16c=0;
            if((mode&7u)==7u||services_.human_remote_ai_enabled()) {
                const auto model=human_.base().frame();
                human_.field258=static_cast<std::uint32_t>(sight_.add_observer(model,15));
                human_.field25c=static_cast<std::uint32_t>(sight_.add_target(model,15));
                sight_.target_mode(signed_word(human_.field25c),0);
                human_.field2a4=static_cast<std::uint32_t>(sound_.add_listener(model));
                human_.field188=proximity_.add_human_proximity(human_,0); //006DFCB0, SAME008AED54 pair owner.
            }
        }
        fields.vector1c0=fields.vector1b0;
        if(human_.context)human_.context->reset_loading(human_);
    }
    void deactivate(bool authority)override {
        const auto mode=services_.mode_flags();frontend::CleanupErrors errors;
        if(human_.flag90||local_started_) {
            errors.attempt([&]{services_.remove_human_detached_camera_reference(human_);}); //004650A0→00463040.
            update_human_loading_profile_mortality(human_,mode);
            if((mode&7u)==7u?human_.properties.field19c==1:services_.human_is_player(human_))
                errors.attempt([&]{services_.remove_human_player(human_);}); //Players virtual24.
            human_.profile_record=nullptr;human_.flag90=false;
            errors.attempt([&]{
                if(!human_.field190.empty())throw std::runtime_error("Human190 requires its actual typed kind1/4 task producer");
                human_.field190.clear(); //00583980/005AA9E0: no raw-pointer reinterpretation.
            });
            errors.attempt([&]{states_.retire();}); //B0+20, delete, clear borrower BEFORE inline98.
            control_.clear();
            errors.attempt([&]{human_.command->clear_empty();});
            human_.perception.clear();human_.list2ac.clear();
            human_.properties.stats_ec=human_.properties.stats_11c;
            errors.attempt([&]{
                auto* storage=human_.base().storage();if(!storage)throw std::runtime_error("Human loading retirement lost actual inventory");
                storage->clear_human_entries();
            });
            if(human_.field160)errors.attempt([&]{services_.detach_human_attachment(*human_.field160,human_.field164);}); //Actor+80.
            human_.field160=nullptr;human_.field164=-1;
            errors.attempt([&]{services_.clear_human_formation(human_);}); //005A91A0(0).
            errors.attempt([&]{services_.retire_human_formation(human_);}); //005A9120.
            local_started_=false;
        }
        //005ABB10/20 are genuine RETs. Authority unregister runs even after the
        //presentation pass already retired local owners and cleared flag90.
        if(authority) {
            if((mode&7u)==7u||services_.human_remote_ai_enabled()) {
                remove_index(human_.field258,[&](auto id){sight_.remove_observer(id);},errors);
                remove_index(human_.field25c,[&](auto id){sight_.remove_target(id);},errors);
                remove_index(human_.field2a4,[&](auto id){sound_.remove_listener(id);},errors);
                if(human_.field188!=-1)errors.attempt([&]{proximity_.remove_human_proximity(human_.field188);human_.field188=-1;});
            }
            if(human_.properties.field19c!=4&&human_.properties.field19c!=5)human_.properties.field170=0xffffffffu;
        }
        errors.attempt([&]{human_.base().deactivate_frame(authority,services_);});
        if(human_.speech_definition){speech_.release(human_.speech_definition);human_.speech_definition=nullptr;} //0059EF80 only ensures cache,005B0390 releases.
        errors.rethrow();
    }
    SpeechDefinition* speech_definition()const{return human_.speech_definition;}
    bool initialized()const{return human_.flag90;}
private:
    static std::int32_t signed_word(std::uint32_t bits){std::int32_t value;std::memcpy(&value,&bits,4);return value;}
    template<class Work>static void remove_index(std::uint32_t& index,Work&& work,frontend::CleanupErrors& errors) {
        if(index!=0xffffffffu)errors.attempt([&]{work(signed_word(index));index=0xffffffffu;});
    }
    void initialize_local(std::uint32_t mode) {
        local_started_=true;human_.fieldbc.clear();const auto model=human_.base().frame();
        if(auto parent=frames_.actor_parent(model);parent&&frames_.frame_kind(parent)!=5) {
            const auto world=frames_.actor_world_matrix(model);
            const HumanVector position{{world[12],static_cast<float>(static_cast<long double>(world[13])+.5f),world[14],0}};
            if(auto sector=frames_.human_scene_sector(position)) {
                frames_.actor_link_preserving_world(model,sector);frames_.update_scene_object(model);
            }
        }
        auto& fields=human_.activation;fields.rotation17c.fill(0);human_.field94=0;
        if(human_.properties.field19c==2)apply_human_difficulty_stats(human_.properties,services_.human_difficulty()); //006B6700.
        human_.properties.stats_ec=human_.properties.stats_11c;
        if(human_.properties.field19c==4||human_.properties.field19c==5)
            bind_human_loading_profile(human_,signed_word(human_.properties.field170),profiles_,effects_);
        fields.vector1b0[0]=fields.vector1b0[1]=fields.vector1b0[2]=0; //Retain undefined fourth lane.
        fields.e4=fields.e8=0;fields.field14c=static_cast<float>(signed_word(human_.properties.stats_ec[2]));
        human_.field160=nullptr;human_.field164=-1;fields.field154=fields.field158=0;
        const auto world=frames_.actor_world_matrix(model);fields.position1d0={{world[12],world[13],world[14],world[15]}};
        fields.field1ac=0;fields.flag150=false;human_.field204=0;fields.flag20c=true;fields.field210=0;
        human_.head_marker=frames_.actor_find(model,"z",0x400);
        if(!human_.head_marker)human_.head_marker=frames_.actor_find(model,"f",0x400);
        human_.field174=0;
        human_.list2ac.bind(services_.human_perception_record_pool());
        control_.initialize(human_,human_.properties.field1a0);
        states_.select_initial(frames_.human_model_has_script_context(model),mode,services_.human_current_player_id(),control_);
        if(auto* state=states_.get())state->refresh_item(); //005A6A90→00584830, repeated AFTER state bind.
        fields.field15c=fields.field254=0;fields.flag214=false;
        if(human_.properties.field19c!=1) {
            const auto index=signed_word(human_.properties.stats_ec[10]);
            if(!index)std::fprintf(stderr,"%s have empty speechset ! Please set it.\n",frames_.frame_name(model).c_str());
            human_.speech_definition=speech_.acquire(index);
            if(!human_.speech_definition) {
                human_.speech_definition=speech_.acquire(0);
                std::fprintf(stderr,"%s doesnt have speechset - using default.\n",frames_.frame_name(model).c_str());
            }
        }
        human_.flag90=true;fields.flag178=false;
    }
    Frames& frames_;Human& human_;Inline& control_;States& states_;Services& services_;Store& profiles_;
    Effects& effects_;Speech& speech_;Sight& sight_;Sound& sound_;
    Proximity& proximity_;
    bool local_started_=false;
};
} //namespace hd2::game
#endif
