#ifndef HD2_GAME_HUMAN_ROUND_RESET_H
#define HD2_GAME_HUMAN_ROUND_RESET_H
#include "game/human_formation.h"
#include "game/human_state_frame_timers.h"
namespace hd2::game {
//0059D100 with offline0059C4D0, remoteAI0059CA00 or local0059CEC0.
//Borrow the SAME Human/frame/inline/state/World owners; no factory replacement.
template<class Human,class Frames,class Inline,class States,class Services,class Effects,class Sight,class Sound,class Proximity>
void reset_retained_human_round(Human& human_,Frames& frames_,Inline& control_,States& states_,Services& services_,
        Effects& effects_,Sight& sight_,Sound& sound_,Proximity& proximity_,bool restart){ //0059D100, actual virtual44.
        frames_.object_visible(human_.base().frame(),true);
        human_.base().reset_frame(services_);
        const bool offline=(services_.mode_flags()&7u)==7u;
        const bool remote_ai=!offline&&services_.human_remote_ai_enabled();
        auto& fields=human_.activation;
        if(offline){ //0059C4D0; hidden150 is cleared BEFORE the effect callback.
            if(fields.flag150){fields.flag150=false;effects_.human_fades().visibility(human_,0,true);
                control_.shadow_visible(frames_,true);services_.round_human_projection_visibility(human_);}
        }
        if(offline||remote_ai){
            human_.fieldbc.clear();clear_human_formation(human_);retire_human_formation(human_);
            fields.field254=0;fields.flag214=false;
            if(offline)human_.base().reset_frame(services_); //Native offline receiver repeats00610710.
            human_.perception.clear();if(offline)fields.flag20c=true;human_.list2ac.clear();
            human_.properties.stats_ec=human_.properties.stats_11c;
            fields.e4=fields.e8=0;fields.field14c=static_cast<float>(human_frame_signed_word(human_.properties.stats_ec[2]));
            human_.fieldc0={{0,0x3f000000u,0,0x3f800000u,0,0,0,0,0}};
            fields.field17c=fields.field180=fields.field184=0;
        }
        if(!offline&&control_.control()->collision_disabled())control_.control()->frame_collision_disabled(false,*control_.look());
        if(remote_ai)fields.flag20c=human_.base().id10()>31;
        if(!offline&&!remote_ai){human_.properties.stats_ec=human_.properties.stats_11c;
            fields.e4=fields.e8=0;fields.field14c=static_cast<float>(human_frame_signed_word(human_.properties.stats_ec[2]));}
        fields.vector1b0[0]=fields.vector1b0[1]=fields.vector1b0[2]=0; //Native W is unwritten stack data.
        human_.field160=nullptr;human_.field164=-1;fields.field154=0;fields.field158=0;
        const auto matrix=frames_.actor_world_matrix(human_.base().frame());fields.position1d0={{matrix[12],matrix[13],matrix[14],matrix[15]}};
        fields.field1ac=0;human_.field174=nullptr;
        if(auto* state=states_.get())state->refresh_item(); //005A6A90 BEFORE inline reset and old B0 close.
        fields.field15c=0;
        if(!offline&&!remote_ai)fields.flag20c=false;
        control_.reset_round();
        states_.retire_round([&]{human_.command->clear_empty();});
        human_.field258=human_.field25c=0xffffffffu;
        if(offline||remote_ai){
            if(offline||human_.field2a4!=0xffffffffu)sound_.remove_listener(human_frame_signed_word(human_.field2a4));
        }
        human_.field2a4=0xffffffffu;
        if(offline&&human_.properties.field19c==1&&human_frame_signed_word(human_.properties.field170)>=0)services_.round_reconfigure_live_soldier(human_);
        if(offline||remote_ai){
            const bool script=frames_.human_model_has_script_context(human_.base().frame());
            states_.select_initial(script,services_.mode_flags(),services_.human_current_player_id(),control_);
            if(offline&&human_.properties.field19c==1&&!script&&restart&&(services_.mode_flags()&2u))
                services_.round_append_player_human(human_);
        }else states_.select_player(false,services_.mode_flags(),services_.human_current_player_id(),control_);
        if(services_.human_is_player(human_)&&(services_.mode_flags()&2u))services_.round_refresh_human_view();
        if(offline){
            human_.field2a4=static_cast<std::uint32_t>(sound_.add_listener(human_.base().frame()));
            human_.field258=static_cast<std::uint32_t>(sight_.add_observer(human_.base().frame(),15));
            human_.field25c=static_cast<std::uint32_t>(sight_.add_target(human_.base().frame(),15));
            if(restart)human_.field188=proximity_.add_human_proximity(human_,0);
        }else if(remote_ai){
            if(human_.properties.field19c!=1&&(services_.mode_flags()&1u)){
                human_.field258=static_cast<std::uint32_t>(sight_.add_observer(human_.base().frame(),15));
                human_.field25c=static_cast<std::uint32_t>(sight_.add_target(human_.base().frame(),15));}
            if(services_.mode_flags()&1u){human_.field2a4=static_cast<std::uint32_t>(sound_.add_listener(human_.base().frame()));human_.field188=proximity_.add_human_proximity(human_,0);}
        }
        if(!offline&&(!remote_ai||human_.properties.field19c==1)){
            auto* state=states_.get();if(state&&(state->words().template get<std::uint32_t>(0x48)==1||state->words().template get<std::uint32_t>(0x48)==7))state->selection_suppressed(0);
        }
        if(human_.scene_movement)human_.scene_movement->reset_loading(human_);
        if(restart&&(services_.mode_flags()&2u)){services_.round_restore_human_clothing(human_);services_.adjust_human_sight_alignment(human_,100.f);}
    
}
}
#endif
