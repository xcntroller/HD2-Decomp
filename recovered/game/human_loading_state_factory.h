#ifndef HD2_GAME_HUMAN_LOADING_STATE_FACTORY_H
#define HD2_GAME_HUMAN_LOADING_STATE_FACTORY_H
#include "game/human_state_damage.h"
#include "game/human_common_command.h"

#include "game/human_active_ai_state_loading.h"
#include "game/human_guard_ai_state_loading.h"
#include "game/human_area_ai_state_loading.h"
#include "game/human_script_ai_state_loading.h"
#include "game/human_passive_states_loading.h"
#include "game/human_player_state_loading.h"
#include "game/human_state_loading_owner.h"
#include "game/human_frame_feedback.h"
#include "game/human_sight_relations.h"

namespace hd2::game {

//Concrete005A3F20 derived-source dispatch. All states share these exact owners;
//B0 publishes directly to the SAME Human's typed borrower. Services implements
//real equipment/Players/music/cleanup receivers; Contexts resolves actual VM state.
//Camera/Presentation/Inline are the existing recovered owners.
//This factory does not initialize a Human or advertise containing-world readiness.
template<class Frames,class Human,class Inputs,class Random,class Contexts,
         class Services,class Inline,class Camera,class Presentation>
class HumanLoadingStateFactory {
public:
    using Base=HumanStateBaseLoading<Frames,Human,Inputs>;
    using AI=HumanAIStateLoading<Frames,Human,Inputs>;
    using Player=HumanPlayerStateLoading<Frames,Human,Inputs>;
    using Active=HumanActiveAIStateLoading<Frames,Human,Inputs,Services>;
    using Guard=HumanGuardAIStateLoading<Frames,Human,Inputs,Random,Contexts>;
    using Script=HumanScriptAIStateLoading<Frames,Human,Inputs,Contexts>;
    using Passive=HumanPassiveAIStateLoading<Frames,Human,Inputs>;
    using Remote=HumanRemoteStateLoading<Frames,Human,Inputs>;
    using Area=HumanAreaAIStateLoading<Frames,Human,Inputs,Contexts,Services>;
    HumanLoadingStateFactory(Frames& frames,Inputs& inputs,MissionTableSet& second,MissionNavigation* navigation,
            HumanInputCallbacks& callbacks,HumanAIActionPools& actions,Random& random,Contexts& contexts,
            Services& services,Inline& control,Camera& camera,Presentation& presentation)
        :frames_(frames),inputs_(inputs),second_(second),navigation_(navigation),callbacks_(callbacks),actions_(actions),
         random_(random),contexts_(contexts),services_(services),control_(control),camera_(camera),presentation_(presentation){}
    std::unique_ptr<Base> make_human_state(std::uint32_t kind) {
        std::unique_ptr<Base> result;
        switch(kind) {
        case 1:{auto player=std::make_unique<Player>(frames_,inputs_,callbacks_);player->bind_sight_services(services_);result=std::move(player);break;}
        case 3:result=std::make_unique<Active>(frames_,inputs_,second_,callbacks_,actions_.kind1,services_);break;
        case 4:result=std::make_unique<Guard>(frames_,inputs_,second_,callbacks_,actions_,random_,contexts_);break;
        case 5:result=std::make_unique<Script>(frames_,inputs_,second_,callbacks_,contexts_);break;
        case 6:result=std::make_unique<Passive>(frames_,inputs_,second_,callbacks_);break;
        case 7:result=std::make_unique<Remote>(frames_,inputs_,callbacks_);break;
        case 8:result=std::make_unique<Area>(frames_,inputs_,second_,callbacks_,actions_.kind3,contexts_,services_);break;
        default:throw std::runtime_error("No original Human B0 factory case");
        }
        result->bind_frame_command_packets(services_);
        result->bind_common_command_receiver([this](Base& state,const HumanPacketCommand& command){
            dispatch_human_deferred_common_command(state,command,
                [this](Base& current,const HumanPacketCommand& value){return common_command(current,value);});
        });
        if(kind==4||kind==8)result->bind_special_music_reference([this](bool enabled){services_.human_special_music_reference(enabled);});
        return result;
    }
    void bind_human_state(Base& state,Human& human,const Base* previous) {
        const auto mode=services_.mode_flags();
        switch(state.words().template get<std::uint32_t>(0x48)) {
        case 1:static_cast<Player&>(state).bind(human,previous,navigation_,mode,control_,camera_,presentation_);break;
        case 3:static_cast<Active&>(state).bind(human,previous,navigation_,mode,control_);break;
        case 4:static_cast<Guard&>(state).bind(human,previous,navigation_,mode,control_);break;
        case 5:(void)static_cast<Script&>(state).bind(human,previous,navigation_,mode,control_);break; //Original ignores context-bool return.
        case 6:static_cast<Passive&>(state).bind(human,previous,navigation_,mode,control_);break;
        case 7:static_cast<Remote&>(state).bind(human,previous,navigation_,mode,control_);break;
        case 8:static_cast<Area&>(state).bind(human,previous,navigation_,mode,control_);break;
        default:throw std::runtime_error("Unrecovered Human B0 bind dispatch");
        }
    }
    void close_human_state(Base& state) {
        switch(state.words().template get<std::uint32_t>(0x48)) {
        case 1:static_cast<Player&>(state).close(services_.mode_flags(),presentation_);break;
        case 3:static_cast<Active&>(state).close_loading();break;
        case 4:static_cast<Guard&>(state).close_loading();break;
        case 5:static_cast<Script&>(state).close_loading();break;
        case 6:static_cast<Passive&>(state).close_loading();break;
        case 7:static_cast<Remote&>(state).close(services_.mode_flags());break;
        case 8:static_cast<Area&>(state).close_loading();break;
        default:throw std::runtime_error("Unrecovered Human B0 close dispatch");
        }
    }
    void publish_human_state(Human& human,Base* state)noexcept{
        if(human.scene_movement)human.scene_movement->path=nullptr; //E0 borrows the retiring B0's vector.
        human.loading_state=state;
    }
    template<class Queries,class Runtime>bool advance_guard_or_area_frame(Base& state,bool authority,std::uint32_t elapsed,Queries& queries,Runtime& runtime){
        const auto kind=state.words().template get<std::uint32_t>(0x48);
        if(kind!=4&&kind!=8)throw std::logic_error("Guard or Area frame requires its actual B0 state");
        if(!authority){
            static_cast<AI&>(state).advance_frame_presentation(services_.mode_flags());
            return false;
        }
        if(kind==4)return static_cast<Guard&>(state).advance_authority_frame(elapsed,queries,control_,services_,runtime);
        return static_cast<Area&>(state).advance_authority_frame(elapsed,queries,control_,services_,runtime);
    }
    template<class Queries,class Runtime>
    void advance_player_frame(Base& state,bool authority,std::uint32_t elapsed,Queries& queries,Runtime& runtime){
        auto& player=static_cast<Player&>(state);
        if(authority)player.advance_authority_frame(elapsed,services_,queries,control_,runtime);
        else player.advance_presentation_frame(elapsed,services_,queries,control_,runtime);
    }
    void resume_script_after_ai(Base& state){
        if(state.words().template get<std::uint32_t>(0x48)==5)static_cast<Script&>(state).resume_after_ai(control_);
    }
    template<class Runtime>void advance_script_presentation(Base& state,std::uint32_t elapsed,Runtime& runtime){
        if(state.words().template get<std::uint32_t>(0x48)!=5)
            throw std::logic_error("Script presentation requires its actual B0 family");
        static_cast<Script&>(state).advance_script_presentation(elapsed,services_,runtime);
    }
    template<class Queries,class Runtime>void advance_script_authority(Base& state,std::uint32_t elapsed,Queries& queries,Runtime& runtime){
        if(state.words().template get<std::uint32_t>(0x48)!=5)
            throw std::logic_error("Script authority requires its actual B0 family");
        static_cast<Script&>(state).advance_script_authority(elapsed,queries,control_,services_,runtime);
    }
    void reset_active_ai_orders(Base& state){
        if(state.words().template get<std::uint32_t>(0x48)!=3)throw std::logic_error("Active AI reset requires state3");
        static_cast<Active&>(state).reset_orders_loading();
    }
    void player_select_human(Human& human){services_.player_select_human(human);} //0045DA60.
    bool human_remote_ai_enabled(){return services_.human_remote_ai_enabled();} //006B66B0.
    void record_state_cleanup_failure(std::exception_ptr error)noexcept{services_.record_state_cleanup_failure(error);}
    auto& services(){return services_;}auto& contexts(){return contexts_;} //SAME owners for derived sound dispatch.
    auto& control(){return control_;}
    template<class FrameServices>void apply_inline_feedback(Base& state,std::uint32_t flags,FrameServices& frames){
        const auto kind=state.words().template get<std::uint32_t>(0x48);
        switch(kind){
        case 1:apply_human_frame_feedback(state,flags,false,frames);return;
        case 3:case 4:case 5:case 6:case 8:apply_human_frame_feedback(state,flags,true,frames);return;
        case 7:throw std::runtime_error("Remote Human frame feedback requires actual005941B0");
        default:throw std::runtime_error("Human frame feedback has no actual B0 receiver");
        }
    }
    void notify_inline_frame_command(Base& state,std::uint32_t command){ //Actual virtual08 subset reached by+80 feedback.
        if((services_.mode_flags()&7u)!=7u)throw std::runtime_error("Human feedback command requires actual0073E5A0 packet receiver");
        switch(state.words().template get<std::uint32_t>(0x48)){
        case 3:case 4:case 5:case 6:case 8:return; //005785B0 genuinely does nothing in offline7, AFTER actual item execution.
        case 1:
            if(command==1){static_cast<Player&>(state).player_words().set(0x534,10000u);return;} //00542920, exact readiness command.
            if(command==0)return; //Actual default branch, before nonoffline transport.
            if(command==8){auto* human=state.human();if(!human)throw std::logic_error("Player feedback command has no bound Human");set_human_sight_alignment(*human,-10.f,services_.mode_flags());return;} //00542920→005A6250.
            if(command==4||command==6||command==7)return; //Actual default branch of00542920, no offline packet.
            throw std::runtime_error("Other player command requires its actual00542920 effects");
        default:throw std::runtime_error("Human command08 requires its actual current B0 receiver");
        }
    }
    void suspend_human_state(Base& state,bool value) { //Actual virtualF0 targets.
        switch(state.words().template get<std::uint32_t>(0x48)){
        case 1:case 7:return; //00583250 is genuine RET4.
        case 3:case 4:case 5:case 6:case 8:break; //005786C0.
        default:throw std::runtime_error("Unrecovered Human state suspension receiver");
        }
        if((services_.mode_flags()&7u)==7u)return;
        auto* input=state.input();if(!input)throw std::runtime_error("Human suspension requires its actual input record");
        input->word38=value?0u:7u;
    }
    bool common_command(Base& state,const HumanPacketCommand& command){ //Existing packet boundary without a frame Runtime.
        const auto kind=state.words().template get<std::uint32_t>(0x48);
        if(kind!=1&&(kind<3||kind>8))throw std::runtime_error("Human common command has no proven B0 receiver");
        return execute_human_common_command(state,command,services_);
    }
    template<class Runtime>bool common_command(Base& state,const HumanPacketCommand& command,Runtime& runtime){ //Actual shared DC00586200.
        const auto kind=state.words().template get<std::uint32_t>(0x48);
        if(kind!=1&&(kind<3||kind>8))throw std::runtime_error("Human common command has no proven B0 receiver");
        return execute_human_common_command(state,command,services_,runtime);
    }
    void script_context(Base& state,typename Contexts::Context* context){
        auto* script=dynamic_cast<Script*>(&state);
        if(!script||state.words().template get<std::uint32_t>(0x48)!=5)
            throw std::logic_error("Script binding notification requires the actual current state5");
        script->script_context(context);
    }
    void script_formation(Base& state,typename Script::Formation* formation){
        auto* script=dynamic_cast<Script*>(&state);
        if(!script||state.words().template get<std::uint32_t>(0x48)!=5u)
            throw std::logic_error("Formation binding requires actual Script B0");
        script->script_formation(formation); //0058D900; producer owns borrower retirement before formation destruction.
    }
    void retire_dead_state(Base& state){ //Virtual84:005836E0, with Player0053BD00 tail.
        retire_human_dead_state(state,services_,frames_);
        if(state.words().template get<std::uint32_t>(0x48)==1){
            auto& player=static_cast<Player&>(state);
            services_.retire_dead_player_models(player);
            presentation_.set_player_mode(player,*state.human(),0,services_.mode_flags());
            if((services_.mode_flags()&7u)!=7u&&player.player_words().template get<std::uint8_t>(0x550))
                services_.retire_dead_player_objective(player);
        }
    }
    void retire_presented_dead_state(Base& state){ //Virtual88, distinct from authority84.
        const auto type=state.words().template get<std::uint32_t>(0x48);
        if(type!=1&&(type<3||type>8))throw std::runtime_error("Presented death has no actual B0 family"); //All eight proven vtables use00583940, except Player0053BE10.
        auto& words=state.words();words.set(0x50,words.template get<std::uint32_t>(0x50)|0x800u); //00583940.
        if(words.template get<std::int32_t>(0x64)>=0)services_.abort_human_dead_action(state);
        if(type==1){ //0053BE10, AFTER base cancellation.
            auto& player=static_cast<Player&>(state);services_.retire_presented_dead_player(player);
            presentation_.set_player_mode(player,*state.human(),0,services_.mode_flags());
            services_.retire_presented_dead_player_tail(player);
        }
    }
    void flush_scene_retirement_commands(Base& state,bool second){ //005A7FE0→actual B0+FC.
        const auto type=state.words().template get<std::uint32_t>(0x48);
        if(type!=1&&(type<3||type>8))throw std::runtime_error("Scene retirement has no actual B0 command family");
        if(second)state.process_second_frame_commands();else state.process_first_frame_commands();
    }
    void notify_nearby_player_damage(Base& state){
        if(state.words().template get<std::uint32_t>(0x48)!=3)throw std::logic_error("Nearby Player damage requires actual Active AI state");
        static_cast<Active&>(state).notify_nearby_player_damage();
    }
    void notify_dead_formation(Base& state){ //00595710 calls005380C0 only for actual Active AI state3.
        if(state.words().template get<std::uint32_t>(0x48)!=3)return;
        static_cast<Active&>(state).retire_formation_leader();
    }
    void reset_item_player_mode(Base& state){
        if(state.words().template get<std::uint32_t>(0x48)!=1)throw std::logic_error("Player item mode reset requires actual Player B0");
        presentation_.set_player_mode(static_cast<Player&>(state),*state.human(),0,services_.mode_flags());
    }
    void notify_damage(Base& state,std::uint32_t kind,std::uint32_t hit_kind,std::int32_t health,Human* source,
            const HumanVector& point,const HumanVector& direction){
        const auto type=state.words().template get<std::uint32_t>(0x48);
        if(type==3||type==7)return; //005A43B0/00583200, genuine RET24, absent frozen.
        if(type==1){notify_human_ai_damage(static_cast<Player&>(state),kind,hit_kind,health,source,point,direction,services_,frames_);return;}
        if(type!=4&&type!=5&&type!=6&&type!=8)throw std::logic_error("Human damage has no actual B0 family");
        notify_human_ai_damage(static_cast<AI&>(state),kind,hit_kind,health,source,point,direction,services_,frames_);
        if(type==5&&(kind==1||kind==2||kind==3)&&source&&human_frame_signed_word(state.human()->properties.stats_ec[0])>0&&
                !(state.words().template get<std::uint32_t>(0x50)&0x80000000u)){
            auto& script=static_cast<Script&>(state);
            if(!(script.state().template get<std::uint32_t>(0x1b8)&0x40u))script.damage_interrupt(*source); //00590400→0041DA20.
        }
    }
private:
    Frames& frames_;Inputs& inputs_;MissionTableSet& second_;MissionNavigation* navigation_;
    HumanInputCallbacks& callbacks_;HumanAIActionPools& actions_;Random& random_;Contexts& contexts_;
    Services& services_;Inline& control_;Camera& camera_;Presentation& presentation_;
};
} //namespace hd2::game
#endif
