#ifndef HD2_GAME_HUMAN_ANIMATION_ARBITRATION_H
#define HD2_GAME_HUMAN_ANIMATION_ARBITRATION_H

#include "game/human_animation_tracks.h"
#include "game/human_holster_lookup.h"
#include "game/human_ladder_control.h"
#include "game/human_equipment_transition_sound.h"
#include <optional>

namespace hd2::game {

//Process scratch009833B8..0098354C and0084E810..0084E820. One containing
//animation-process owner supplies this state to every Human. Resources and
//masked bones are borrowed, with native pointers kept outside DWORD storage.
template<class Frame>struct HumanAnimationArbitrationState {
    using Primary=std::array<std::int32_t,3>;
    using Secondary=std::array<std::int32_t,2>;
    using Requests=std::array<HumanAnimationRequest<Frame>,4>;
    Primary primary{},requested_primary{},previous_primary{};
    Secondary equipment{},requested_equipment{},previous_equipment{},
        secondary{},requested_secondary{},previous_secondary{};
    Requests outgoing{},pending{};
    std::array<Frame,3> selected_clips{};
    std::array<Frame,8> masked_bones{};
    std::int32_t mask_count=-1;
    float primary_scale=1,primary_blend=-1,equipment_scale=1,secondary_scale=1;
    std::optional<unsigned> primary_table,equipment_table,secondary_table;
    bool changed=false;
};

inline void normalize_human_arbitration_primary(std::array<std::int32_t,3>& value){
    HumanJointWords<16> words;
    words.set(0,value);normalize_human_primary_animation(words,0);
    value=words.template get<std::array<std::int32_t,3>>(0);
}

inline bool human_primary_animation_blocking(std::int32_t id){ //0075C7E0, only the first argument is read.
    constexpr std::array<std::array<std::int32_t,2>,15> ranges{{
        {{0,3}},{{43,44}},{{30,33}},{{8,14}},{{18,24}},{{26,29}},{{36,38}},
        {{156,157}},{{160,161}},{{94,97}},{{47,53}},{{57,61}},{{162,165}},{{113,154}},{{7,7}}
    }};
    if(id==16||id==103)return true;
    for(const auto& range:ranges)if(id>=range[0]&&id<=range[1])return true;
    return false;
}
inline bool human_equipment_animation_blocking(std::int32_t id){ //0075C8D0.
    for(auto excluded:{30,31,32,87,88,91})if(id==excluded)return false;
    return true;
}
inline bool human_secondary_animation_blocking(std::int32_t id){ //0075C900.
    for(auto excluded:{25,52,71,72,75,65,66,74})if(id==excluded)return false;
    return true;
}
inline bool human_animation_family_matches(std::int32_t first,std::int32_t second){ //00779390.
    if(first==second)return true;
    for(const auto& range:std::array<std::array<std::int32_t,2>,3>{{{{45,54}},{{55,62}},{{63,65}}}})
        if(first>=range[0]&&first<=range[1]&&second>=range[0]&&second<=range[1])return true;
    return false;
}

template<class Control,class Look,class State>
void constrain_human_primary_animation(Control& control,const Look& look,State& scratch){ //0077C690.
    const auto& words=control.physical().control_frame_words();
    const auto word=[&](unsigned offset){return words.template get<std::int32_t>(offset);};
    const auto requested=[&](unsigned offset){return look.state().template get<std::int32_t>(offset);};
    auto& desired=scratch.requested_primary;const auto& current=scratch.primary;
    const auto normalize=[&]{normalize_human_arbitration_primary(desired);};
    const bool transferring=word(0xa20)==2||word(0xa20)==3;
    const bool firing=word(0x1e8)>=70&&word(0x1e8)<=82;
    if((desired[0]>=106&&desired[0]<=108)||desired[0]==8||desired[0]==14){
        if(desired[0]==107&&(desired[2]==2||requested(0x50)==5)){desired[0]=108;normalize();}
        return;
    }
    if(word(0x988)!=-10000)return;
    if(desired[0]>=41&&desired[0]<=65){
        if((requested(0x5c)!=0&&requested(0x54)!=6&&(transferring||current[0]==33))||firing){
            if(desired[1]==0&&desired[0]!=54&&desired[0]!=44&&desired[0]!=43){desired[0]=54;normalize();}
            else if(desired[1]==1&&desired[0]!=62){desired[0]=62;normalize();}
            else if(desired[1]==2&&desired[0]!=65){desired[0]=65;normalize();}
        }else{
            if(current[1]==0&&desired[0]==54){desired[0]=45;normalize();}
            else if(current[1]==1&&desired[0]==62){desired[0]=55;normalize();}
            else if(current[1]==2&&desired[0]==65){desired[0]=63;normalize();}
        }
    }
    if(!firing&&(!requested(0x5c)||requested(0x54)==6||word(0x1ac)!=1))return;
    if(desired[0]>=190&&desired[0]<=205){
        if(firing){desired[0]+=8;normalize();}
    }else if(current[0]>=190&&current[0]<=205&&desired[0]==206){
        if(current[0]<198&&firing){desired[0]=current[0]+8;normalize();}
        else if(current[0]>=198&&!firing){desired[0]=current[0]-8;normalize();}
    }
}

template<class Control,class Look,class Human,class Services,class Frames,class Runtime,class State,class Publish>
void transition_human_primary_animation(Control& control,Look& look,Human& human,Services& services,Frames& frames,
        Runtime& runtime,const State& scratch,bool blocking,Publish& publish){ //00779E60.
    auto& physical=control.physical();auto& words=physical.control_frame_words();
    auto& action=control.action().frame_words();auto& movement=control.movement().frame_words();auto& tracks=control.tracks();
    const auto& current=scratch.primary;const auto& previous=scratch.previous_primary;
    const auto word=[&](unsigned offset){return words.template get<std::int32_t>(offset);};
    const auto command=[&](unsigned offset){return action.template get<std::int32_t>(offset);};
    const auto requested=[&](unsigned offset){return look.state().template get<std::int32_t>(offset);};
    const auto scale=services.human_control_animation_scale();
    const auto normalize=[&]{normalize_human_primary_animation(action,0x10);};
    const auto followup=[&](std::int32_t id){action.set(0x10,id);normalize();};
    const auto posture=[&](std::int32_t value){action.set(0x14,value);normalize();};
    const auto forced_movement=[&]{
        look.force_frame_commands(true);
        try{look.frame_movement_animation(command(0x10),publish);}catch(...){look.force_frame_commands(false);throw;}
        look.force_frame_commands(false);
    };
    const auto clear_marker_offset=[&]{for(unsigned axis=0;axis!=3;++axis)action.set(0x70+axis*4,0.f);};
    const auto reset_marker=[&]{
        if(const auto marker=control.action().marker()){
            frames.object_position(marker,{{action.template get<float>(0x70),action.template get<float>(0x74),action.template get<float>(0x78)}});
            frames.update_scene_object(marker);
        }
    };
    if(!blocking){
        if(current[0]>=106&&current[0]<=108&&((control.mode_flags()&7u)==7u||control.human_is_player()||control.animation_authority_actor())){
            auto position=human.base().world_position();
            const auto* ladder=physical.control_action_actor()?&services.human_ladder_geometry(physical.control_action_actor()):nullptr;
            project_human_ladder_position(human,ladder,position);human.base().world_position(position);
            auto velocity=position;HumanVector zero{};
            tracks.motion_velocity(zero,0,0,velocity);tracks.motion_velocity(zero,0,0,velocity);
        }
    }else if(command(0x20)==-10000){
        action.set(0x24,0u);action.set(0x28,0x20000u);action.set(0x10,previous);
        posture(requested(0x4c));action.set(0x18,requested(0x50));
        words.set(0x968,0u);movement.set(0xe4,std::uint8_t{0});action.set(0x5c,0u);action.set(0x60,std::int32_t{-10000});
        bool lateral=false,shortened=false,posture_transition=false;
        const auto change_posture_motion=[&](std::int32_t id,std::int32_t next,unsigned sound,unsigned previous_posture){
            control.play_presentation_track_sound(sound);followup(id);posture(next);action.set(0x28,8u);
            movement.set(0xdc,0u);movement.set(0xe0,0u);words.set(0x9ac,0u);words.set(0x9b0,previous_posture);posture_transition=true;
        };
        switch(current[0]){
        case 0:case 1:case 2:case 3:{
            auto* target=physical.control_interaction_target();
            const auto model=target?target->base().frame():typename Frames::Frame{}; //Actual typed A3C, published by00751480 case1.
            if(!model)throw std::runtime_error("Human primary door transition requires its actual target model");
            if(const auto handle=frames.actor_find(model,"klika",0x20)){
                const auto position=human.base().world_position();const auto matrix=frames.actor_world_matrix(handle);
                auto direction=look.state().template get<HumanVector>(0x20);
                for(unsigned axis=0;axis!=3;++axis)direction[axis]=matrix[12+axis]-position[axis];
                words.set(0x998,human_ladder_turn_delay(control.movement(),human,direction,control.human_is_player()));
                look.current_direction(direction);
            }
            action.set(0x28,0x40000u);break;
        }
        case 7:followup(98);action.set(0x28,0x40000u);break;
        case 8:case 9:case 10:case 11:case 12:case 13:case 14:{
            constexpr std::array<std::uint32_t,7> heights{{0xbfd08312u,0x3e79db23u,0x3f0f9db2u,0x3f31eb85u,0x3f7851ecu,0x3fd22d0eu,0x3fad9168u}};
            constexpr std::array<float,7> delays{{0,554,754,400,750,426,400}};
            const auto index=static_cast<unsigned>(current[0]-8);
            if(current[0]==8||current[0]>=11)control.play_presentation_track_sound(current[0]==11?13u:current[0]==12?14u:15u);
            if(current[0]==8)control.finish_step_fall(look,runtime,publish);
            HumanVector impulse{};
            if(current[0]==13){
                impulse=human.base().world_direction();
                for(unsigned axis=0;axis!=3;++axis)impulse[axis]=static_cast<float>(-static_cast<long double>(impulse[axis])*.1f);
            }
            words.set(0x95c,heights[index]);words.set(0x960,static_cast<float>((current[0]==12?550.L:0.L)/scale));
            words.set(0x964,static_cast<float>(static_cast<long double>(delays[index])/scale));
            if(current[0]==8){followup(requested(0x50)==2?108:107);posture(0);action.set(0x28,0x20000u);}
            else if(word(0x954)==1){followup(56);posture(1);}
            else if(current[0]==14||command(0x10)==43||command(0x10)==44)followup(45);
            forced_movement();
            if(current[0]!=8)control.finish_step_fall(look,runtime,publish);
            if(current[0]==13){impulse[1]=0;tracks.motion_impulse(impulse,250);}
            clear_marker_offset();
            if(current[0]==8){
                auto& request=look.frame_words();request.set(0xd0,request.template get<std::uint32_t>(0xd0)&~1u);forced_movement();
            }
            reset_marker();
            if(current[0]==14&&physical.control_action_actor()){
                const auto& ladder=services.human_ladder_geometry(physical.control_action_actor());
                const auto x=static_cast<long double>(ladder.down_position()[0])-ladder.up_position()[0];
                const auto z=static_cast<long double>(ladder.down_position()[2])-ladder.up_position()[2];
                const float square=static_cast<float>(z*z+x*x);std::uint32_t bits;std::memcpy(&bits,&square,4);
                long double length=0;
                if(human_frame_signed_word(bits)>=0x322bcc77)length=std::abs(static_cast<long double>(square)-1)>=1.e-8f?std::sqrt(static_cast<long double>(square)):1.L;
                auto offset=action.template get<HumanVector>(0x90);
                for(unsigned axis=0;axis!=3;++axis)offset[axis]=static_cast<float>(length*ladder.direction()[axis]*.5f);
                const float squared=static_cast<float>(static_cast<long double>(offset[2])*offset[2]+static_cast<long double>(offset[0])*offset[0]);
                std::memcpy(&bits,&squared,4);
                if(human_frame_signed_word(bits)>0x322bcc76&&(std::abs(static_cast<long double>(squared)-1)<1.e-8f||std::sqrt(static_cast<long double>(squared))>=.05f)){
                    action.set(0x60,50u);offset[1]=0;action.set(0x90,offset);
                }
            }
            break;
        }
        case 18:case 23:case 24:{
            movement.set(0x100,0u);look.mark_frame_feedback(0x80);
            auto velocity=tracks.words().template get<HumanVector>(0x300);velocity[0]=velocity[2]=0;
            unsigned duration;
            if(current[0]==18){velocity[1]=8;duration=2400;}
            else if(current[0]==23){velocity[1]=requested(0x50)==2?9.5f:9.f;duration=requested(0x50)==2?3000u:2500u;}
            else{velocity[1]=9.8500003814697265625f;duration=requested(0x50)==2?2500u:2100u;}
            tracks.motion_deceleration(velocity,static_cast<std::int32_t>(duration));action.set(0x28,0x40000u);break;
        }
        case 20:case 22:followup(45);posture(0);action.set(0x28,8u);break;
        case 21:followup(55);posture(1);action.set(0x28,8u);look.frame_movement_animation(45,publish,false);break;
        case 26:change_posture_motion(55,1,5,0);break;
        case 27:change_posture_motion(46,0,6,1);break;
        case 28:followup(34);look.control_movement_mode(1,true,publish);break;
        case 29:case 37:case 38:followup(45);look.control_movement_mode(1,false,publish);look.control_movement_mode(2,false,publish);break;
        case 30:change_posture_motion(64,2,4,1);break;
        case 31:change_posture_motion(56,1,5,2);break;
        case 32:
            control.play_presentation_track_sound(4);posture(2);action.set(0x28,8u);posture_transition=true;
            movement.set(0xdc,0u);movement.set(0xe0,0u);words.set(0x9ac,0u);words.set(0x9b0,0u);
            if(previous[0]<190)followup(64);
            else{action.set(0x10,previous);posture(2);shortened=true;}
            break;
        case 33:change_posture_motion(46,0,6,2);break;
        case 36:followup(35);look.control_movement_mode(2,true,publish);break;
        case 43:case 44:
            if(human_primary_animation_blocking(command(0x10)))followup(45);
            if(requested(0x44)==current[0])look.frame_movement_animation(command(0x10),publish);
            break;
        case 47:case 48:case 49:case 50:case 51:case 52:case 53:case 54:followup(45);break;
        case 57:case 58:case 59:case 60:case 61:case 62:followup(55);break;
        case 65:followup(63);break;
        case 94:
            if(const auto model=look.action_model()){
                words.set(0xa30,0u);
                auto* mounted=services.template human_mounted_model_actor<Human>(model);
                if(!mounted)throw std::runtime_error("Human mount transition requires its actual model-bound actor");
                services.human_presentation_equipment(human).state().select(model,9,0xffffffffu,mounted->base().id10(),-1);
            }
            break;
        case 95:words.set(0xa30,2u);look.force_equipment_command(100);break;
        case 103:look.frame_movement_animation(command(0x10),publish);break;
        case 156:case 160:
            words.set(0xa2c,current[0]==156?0u:8u);
            services.human_presentation_equipment(human).state().select(look.action_model(),4,0xffffffffu,-4,-1);break;
        case 157:words.set(0xa2c,2u);look.force_equipment_command(100);break;
        case 161:words.set(0xa2c,10u);break;
        case 162:case 163:case 164:case 165:lateral=true;action.set(0x24,0u);break;
        case 192:case 193:case 194:case 195:case 196:case 197:case 200:case 201:case 202:case 203:case 204:case 205:
            action.set(0x10,current);action.set(0x24,0u);movement.set(0xdc,0u);movement.set(0xe0,0u);action.set(0x28,0x40000u);break;
        default:break;
        }
        if(current[0]>=113&&current[0]<=154){followup(113);clear_marker_offset();}
        if(current[0]<206){
            unsigned table;
            if(requested(0xa0)==1)table=0;
            else if(requested(0xa0)==2)table=1;
            else throw std::runtime_error("Human primary transition has no actual animation table");
            const auto row_index=[](const auto& value){
                const auto id=static_cast<std::int64_t>(value[0]);
                const auto row=id<190?id*3+value[1]:498+(id-190)*18+static_cast<std::int64_t>(value[1])*6+value[2];
                if(row<0||row>=786)throw std::runtime_error("Human primary transition exceeds its actual animation table");
                return static_cast<unsigned>(row);
            };
            const auto next=action.template get<typename State::Primary>(0x10);
            const auto blend=human_animation_time_integer(human_animation_word_float(services.human_animation_row(table,row_index(next)).words[1]));
            const auto clip=services.human_animation_row(table,row_index(current)).animation.get();
            if(!clip)action.set(0x20,0u);
            else{
                std::int32_t duration;
                if(current[0]<190&&lateral){
                    if(action.template get<float>(0xa0)!=-10000.099609375f)
                        tracks.primary_stop_fraction(frames.human_clip_duration(clip),action.template get<float>(0xa0));
                    duration=human_frame_signed_word(static_cast<std::uint32_t>(frames.human_clip_duration(clip))-200u);
                }else duration=human_frame_signed_word(static_cast<std::uint32_t>(frames.human_clip_duration(clip))-static_cast<std::uint32_t>(blend));
                action.set(0x20,duration);action.set(0x20,human_animation_time_integer(static_cast<long double>(duration)/scale));
                if(current[0]<190&&shortened)action.set(0x20,human_animation_time_integer(static_cast<long double>(command(0x20))*.75f));
                if(word(0x9ac)==0)words.set(0x9ac,command(0x20));
                if(current[0]>=113&&current[0]<=154){
                    if(word(0x1ac)==4)look.schedule_frame_posture(human_animation_time_integer(100.L/scale),current[0]==146||current[0]==148?0u:2u,publish);
                    words.set(0x9bc,human_animation_time_integer(300.L/scale));
                    words.set(0x9b8,human_animation_time_integer(static_cast<long double>(command(0x20))*1.1f));
                    words.set(0x9c0,human_animation_time_integer(static_cast<long double>(command(0x20))*1.7f));words.set(0x9c4,word(0x9b8)/4);
                }else{
                    if(current[0]>=11&&current[0]<=14){
                        if(word(0x954)==1){
                            look.frame_posture(1,publish,false);
                            look.schedule_frame_posture(human_animation_time_integer(static_cast<long double>(command(0x20))*1.1f),static_cast<unsigned>(word(0x954)),publish);
                        }
                    }else if(posture_transition)look.schedule_frame_posture(human_animation_time_integer(static_cast<long double>(command(0x20))*1.1f),static_cast<unsigned>(requested(0x4c)),publish);
                    if((current[0]>=11&&current[0]<=14)||current[0]==8)physical.activate_step_secondary(command(0x20));
                }
            }
            auto& request=look.frame_words();request.set(0xd0,request.template get<std::uint32_t>(0xd0)|action.template get<std::uint32_t>(0x28));
        }else action.set(0x20,std::int32_t{-10000});
        if(word(0x9ac)==0)words.set(0x9ac,std::int32_t{-10000});
    }
    action.set(0xa0,-10000.099609375f);
    if(current[0]==108)physical.play_frame_climb_voice(services);
    else physical.stop_frame_voice();
}

template<class Control,class Look,class Human,class Services,class Frames,class Runtime,class State,class Publish>
void arbitrate_human_primary_animation(Control& control,Look& look,Human& human,Services& services,Frames& frames,
        Runtime& runtime,State& scratch,Publish& publish){ //00777AD0.
    auto& physical=control.physical();auto& words=physical.control_frame_words();
    auto& action=control.action().frame_words();auto& movement=control.movement().frame_words();auto& tracks=control.tracks();
    auto& desired=scratch.requested_primary;auto& current=scratch.primary;const auto& previous=scratch.previous_primary;
    const auto word=[&](unsigned offset){return words.template get<std::int32_t>(offset);};
    const auto command=[&](unsigned offset){return action.template get<std::int32_t>(offset);};
    const auto requested=[&](unsigned offset){return look.state().template get<std::int32_t>(offset);};
    const auto scale=[&]{return services.human_control_animation_scale();};
    const auto normalize=[&]{normalize_human_arbitration_primary(desired);};
    const auto walking=[](std::int32_t id){return id>=41&&id<=65;};
    const auto climbing=[](std::int32_t id){return id>=106&&id<=108;};
    const auto sensitive=[](std::int32_t id){return id==43||id==44||id==54||id==62||id==65;};
    const auto pause_all=[&]{for(unsigned group=0;group!=3;++group){const auto time=tracks.track_time(group);if(time!=-10000)tracks.pause(group,time);}};
    if(word(0x988)!=-10000){
        if(desired[0]==206&&climbing(current[0])){pause_all();publish(0);physical.stop_frame_voice();}
        else if(!climbing(current[0])&&current[0]!=8&&desired[0]!=206&&desired[0]!=8){
            publish(0);look.frame_movement_animation(45,publish);pause_all();physical.stop_frame_voice();
        }else{
            if(climbing(current[0])){
                publish(current[0]==106?8u:16u);tracks.discard_pending_tracks(0);
                if(current[0]==108)physical.play_frame_climb_voice(services);
            }
            for(unsigned group=0;group!=3;++group)if(word(0x274+group*0x2c)>0)tracks.resume(group);
        }
    }
    bool blocking=human_primary_animation_blocking(desired[0]);
    if(command(0x24)>0||blocking)tracks.discard_pending_tracks(0);
    if(desired[0]>=113&&desired[0]<=154){desired[1]=word(0x9e0);normalize();}
    else{
        bool processed=false;
        if(command(0x20)!=-10000&&(!walking(word(0x1d8))||word(0x1fc)==206||(walking(word(0x1fc))&&!sensitive(word(0x1fc))))){
            processed=true;
            if(!tracks.track_reached(0,command(0x20))){
                if(desired!=current&&(command(0x24)!=-1||word(0x274)>0)&&current[0]>=162&&current[0]<=165){
                    tracks.resume(0);action.set(0x24,std::int32_t{-1});tracks.mirror_first_half(0);
                    words.set(0x594,std::int32_t{-10000});
                    auto& request=look.frame_words();request.set(0xd0,request.template get<std::uint32_t>(0xd0)&~action.template get<std::uint32_t>(0x28));
                }
                if((climbing(current[0])||(current[0]!=8&&current[0]!=14))&&climbing(desired[0])&&
                   (control.mode_flags()&7u)!=7u&&!control.human_is_player()&&!control.animation_authority_actor())action.set(0x20,std::int32_t{-10000});
                desired[0]=206;normalize();
            }else if(command(0x24)==1&&desired==action.template get<typename State::Primary>(0x10)){
                words.set(0x270,0u);words.set(0x274,0u);
            }else{
                if(command(0x24)==0){
                    if(((!(action.template get<std::uint32_t>(0x28)&0x40000u)&&!(current[0]>=30&&current[0]<=33))||desired[0]==206)){
                        desired=action.template get<typename State::Primary>(0x10);
                        if(current[0]>=8&&current[0]<=14){desired[1]=requested(0x4c);normalize();}
                    }
                }else{
                    tracks.fade_pair(0,human_animation_time_integer(350.L/scale()));
                    scratch.primary_blend=static_cast<float>(350.L/scale());current[0]=206;normalize_human_arbitration_primary(current);
                    if(command(0x10)==31||command(0x10)==33)desired=action.template get<typename State::Primary>(0x10);
                }
                action.set(0x20,std::int32_t{-10000});
                if(word(0x988)==-10000&&action.template get<std::uint32_t>(0x28)!=0x20000u){
                    auto& request=look.frame_words();request.set(0xd0,request.template get<std::uint32_t>(0xd0)&~action.template get<std::uint32_t>(0x28));
                }
            }
        }
        if(!processed){
            scratch.primary_scale=tracks.choose_primary(desired,control.human_is_player()||control.animation_authority_actor()||(control.mode_flags()&7u)==7u);
            if(blocking&&desired[0]==206)tracks.hold_pending_tracks(0);
            action.set(0x20,std::int32_t{-10000});
        }
    }
    if(desired[0]!=206)constrain_human_primary_animation(control,look,scratch);
    blocking=human_primary_animation_blocking(desired[0]);
    if(blocking)tracks.hold_pending_tracks(0);
    if(desired[0]!=206&&desired!=current){
        if((current[0]==43||current[0]==44)&&desired[0]!=43&&desired[0]!=44)movement.set(0x20,-10000.099609375f);
        if(current[0]>=8&&current[0]<=14){
            const auto factor=!words.template get<std::uint8_t>(0x96c)&&current[0]!=14?.08f:.25f;
            auto impulse=human.base().world_direction();
            for(unsigned axis=0;axis!=3;++axis)impulse[axis]*=factor;
            impulse[1]=0;tracks.motion_impulse(impulse,200);
        }
        current=desired;
        if(current[0]>=8&&current[0]<=14){current[1]=0;normalize_human_arbitration_primary(current);}
        if(current[0]<=205){
            if(requested(0xa0)==1)scratch.primary_table=0;
            else if(requested(0xa0)==2)scratch.primary_table=1;
            if(!scratch.primary_table)throw std::runtime_error("Human primary arbitration has no actual animation table");
            tracks.motion_animation_rate(0,1.f);
            const bool moving=current[0]>=190;
            const auto row=moving?498+(current[0]-190)*18+current[1]*6+current[2]:current[0]*3+current[1];
            if(row<0)throw std::runtime_error("Human primary arbitration reached a native negative table index");
            const auto& entry=services.human_animation_row(*scratch.primary_table,static_cast<unsigned>(row));
            auto& request=scratch.pending[0];
            scratch.selected_clips[0]=entry.animation.get();request.animation=scratch.selected_clips[0];request.weight=1;request.flags=entry.words[0]&0xeu;
            const auto duration=static_cast<long double>(human_animation_word_float(entry.words[1]))/scale(); //FLD77841A/7784AD, both row families store FLOAT.
            request.blend=!(scratch.primary_blend<=0.f)?scratch.primary_blend:static_cast<float>(duration*(moving?1.f:scratch.primary_scale));
            control.face().frame_running(moving&&(current[2]==2||current[2]==5),false,scale());
            if(!scratch.selected_clips[0])current=previous;
            if(command(0x24)==-1){action.set(0x24,0u);request.blend=200;}
            else{
                const auto floor=[](std::int32_t id){return id>=89&&id<=93;};
                if(floor(current[0])&&!floor(previous[0])&&previous[0]!=104&&previous[0]!=105)request.blend=static_cast<float>(1100.L/scale());
                else if(floor(previous[0])&&(current[0]==46||current[0]==56||current[0]==64||current[0]==190||current[0]==198))request.blend=static_cast<float>(600.L/scale());
                else if(current[1]!=previous[1]&&scratch.equipment[0]==32&&!walking(current[0])&&current[0]!=26&&current[0]!=27&&current[1]<2&&previous[1]<2){
                    request.blend=static_cast<float>(650.L/scale());words.set(0x9ac,2000u);words.set(0x9b0,previous[1]);
                }else if(previous[0]==43||previous[0]==44)request.blend=static_cast<float>(300.L/scale());
                else if(previous[0]>=8&&previous[0]<=14)request.blend=static_cast<float>(200.L/scale());
                else if(current[0]>=162&&current[0]<=165&&previous[0]>=162&&previous[0]<=165)request.flags=8;
                else if(previous[0]==6)request.blend=static_cast<float>(800.L/scale());
            }
            if(current[0]==158){control.face().equipment_primary_command(1);control.face().equipment_secondary_command(1);}
            else if(previous[0]==158){control.face().equipment_primary_command(0);control.face().equipment_secondary_command(0);}
            movement.set(0x104,static_cast<std::uint8_t>((current[0]==18||current[0]==24||current[0]==23)&&(previous[0]==191||previous[0]==199)));
        }
    }
    if(!scratch.selected_clips[0]){desired[0]=206;normalize();return;}
    words.set(0x1d8,current);
    blocking=human_primary_animation_blocking(current[0]);
    frames.human_clip_word8(scratch.selected_clips[0],blocking?1u:2u);
    if(current[0]<113||current[0]>154||word(0x1ac)>6)
        tracks.select(scratch.pending,desired,scratch.requested_equipment,scratch.requested_secondary,blocking);
    scratch.outgoing[0]=scratch.pending[0];scratch.pending[0].animation={};
    transition_human_primary_animation(control,look,human,services,frames,runtime,scratch,blocking,publish);
    control.prepare_motion_velocity(look,false);publish(0);
    std::optional<std::uint32_t> task;
    switch(current[0]){
    case 190:case 198:task=8;break;
    case 191:case 199:task=16;break;
    case 192:case 200:task=32;break;
    case 193:case 201:task=64;break;
    case 196:case 204:task=48;break;
    case 194:case 202:task=40;break;
    case 197:case 205:task=80;break;
    case 195:case 203:task=72;break;
    case 106:task=word(0x274)<1?8u:0u;break;
    case 107:case 108:task=word(0x274)<1?16u:0u;break;
    case 162:case 164:task=0x1000000;break;
    case 163:case 165:task=0x2000000;break;
    default:if(walking(current[0]))task=0;break;
    }
    if(task)publish(*task);
    scratch.changed=true;
}

template<class Look>void force_human_arbitration_action(Look& look,bool changed){
    look.force_frame_commands(true);
    try{look.action_command(75,changed);}catch(...){look.force_frame_commands(false);throw;}
    look.force_frame_commands(false);
}

template<class Control,class Look,class Publish>
void relax_human_ready_animation(Control& control,Look& look,Publish& publish){ //0077D3A0.
    const auto& words=control.physical().control_frame_words();
    const auto id=words.template get<std::int32_t>(0x1d8),posture=words.template get<std::int32_t>(0x1dc);
    if(posture==0&&id==54)look.frame_movement_animation(45,publish);
    else if(posture==1&&id==62)look.frame_movement_animation(55,publish);
    else if(posture==2&&id==65)look.frame_movement_animation(63,publish);
}

template<class Control,class Look,class Human,class Services,class State,class Publish>
void constrain_human_equipment_animation(Control& control,Look& look,Human& human,Services& services,
        const State& scratch,Publish& publish){ //0077C8D0.
    auto& words=control.physical().control_frame_words();
    const auto word=[&](unsigned offset){return words.template get<std::int32_t>(offset);};
    const auto requested=[&](unsigned offset){return look.state().template get<std::int32_t>(offset);};
    const auto primary=scratch.primary[0];const auto pending=word(0x20c);
    const auto finish=[&]{if(word(0x20c)==35&&!(words.template get<std::uint8_t>(0x190)&1u))words.set(0x20c,36u);};
    if(pending==92||word(0x1ac)!=1){finish();return;}
    auto& equipment=services.human_presentation_equipment(human);
    if(((primary>=0&&primary<=40&&primary!=9&&primary!=10)||primary==56||primary==103)&&
       word(0xa20)>=2&&pending!=34&&primary!=26&&primary!=27){
        words.set(0x20c,92u);words.set(0xa20,4u);equipment.transfers().event(1);finish();return;
    }
    if((primary>=106&&primary<=108)||(primary>=89&&primary<=93&&pending!=34&&pending!=35)){finish();return;}
    const auto mode=word(0xa20);
    if(mode!=3&&mode!=4){
        if(mode==0&&word(0x1e8)==91)relax_human_ready_animation(control,look,publish);
        finish();return;
    }
    const bool walking=word(0x1d8)>=41&&word(0x1d8)<=65;
    if(requested(0x4c)==2&&primary!=33&&primary!=31&&pending!=34&&pending!=35&&pending!=36&&
       !walking&&(primary<162||primary>165)){
        words.set(0x20c,92u);equipment.transfers().event(1);words.set(0xa20,4u);finish();return;
    }
    if(primary==206||pending==34){finish();return;}
    const auto feedback=[&]{
        look.mark_frame_feedback(8);
        if((word(0x1e8)==32||requested(0x54)==6)&&word(0xa28)==0)look.mark_frame_feedback(0x100);
    };
    if(requested(0x58)!=0&&requested(0x54)!=6){
        if(requested(0x58)<3)feedback();
        finish();return;
    }
    const auto secondary=scratch.secondary[0];
    const bool secondary_busy=(secondary>=8&&secondary<=29)||(secondary>=52&&secondary<=73);
    if(!secondary_busy){
        if(word(0xa24)==1)words.set(0xa24,0u);
        else if(word(0xa24)==0){
            feedback();
            if(word(0x20c)<33&&requested(0x54)!=6&&requested(0x54)!=2&&(word(0x1e8)<80||word(0x1e8)>82))equipment.transfers().event(0);
        }
    }else if(word(0xa24)==0){
        if(requested(0x54)!=6)equipment.transfers().event(1);
        words.set(0xa24,human_animation_time_integer(550.L/services.human_control_animation_scale())); //77CA8F.
    }
    const auto kind=requested(0x54);
    if(kind==6||(equipment.state().words().template get<std::uint8_t>(0x50)&1u)||kind==100){
        if(walking)relax_human_ready_animation(control,look,publish);
    }else if(kind!=0){
        if(requested(0x5c)==0||(requested(0x5c)==1&&kind==5&&requested(0x4c)==2)){
            words.set(0x20c,human.properties.field19c==1?30u:31u);words.set(0xa20,3u);
            if(requested(0x5c)==0&&walking)relax_human_ready_animation(control,look,publish);
        }else if(requested(0x5c)==1){
            words.set(0x20c,32u);words.set(0xa20,3u);
            const auto id=word(0x1d8),posture=word(0x1dc);
            if(id>=41&&id<=65&&id!=43&&id!=44){
                if(posture==0&&id!=54)look.frame_movement_animation(54,publish);
                else if(posture==1&&id!=62)look.frame_movement_animation(62,publish);
                else if(posture==2&&id!=65)look.frame_movement_animation(65,publish);
            }
        }
    }
    finish();
}

template<class Control,class Look,class Human,class Services,class Frames,class State>
void transition_human_secondary_animation(Control& control,Look& look,Human& human,Services& services,
        Frames& frames,const State& scratch){ //0077C140.
    auto& action=control.action().frame_words();auto& equipment=services.human_presentation_equipment(human);
    auto& tracks=control.tracks();
    const auto command=[&](unsigned offset){return action.template get<std::int32_t>(offset);};
    const auto requested=[&](unsigned offset){return look.state().template get<std::int32_t>(offset);};
    const auto& current=scratch.secondary;const auto& previous=scratch.previous_secondary;
    if(command(0x4c)!=-10000)return;
    if(previous[0]==75||human_secondary_animation_blocking(previous[0])){
        action.set(0x44,76u);look.action_command(75,true);
    }else action.set(0x44,previous);
    action.set(0x48,requested(0x4c));action.set(0x58,std::int32_t{-10000});
    const auto copy_cards=[&](std::int32_t identity,bool temporarily_hidden){
        if(!services.human_hand_action_model())return false;
        auto model=frames.create_item_template_model();
        if(!model)throw std::runtime_error("Human secondary action model factory returned null");
        frames.copy_item_frame(model.get(),services.human_hand_action_model());
        auto& state=equipment.state();
        if(temporarily_hidden)state.words().set(0x2c,std::uint8_t{0});
        state.select(model.get(),7,0xffffffffu,identity,-1);model.release(); //230's original factory reference, retired by its action-model lifecycle.
        return true;
    };
    switch(current[0]){
    case 17:
        if(!equipment.state().selected_model(7)&&copy_cards(-3,true)){
            equipment.transfers().event(11);equipment.state().words().set(0x2c,std::uint8_t{1});
        }
        action.set(0x44,18u);break;
    case 18:
        tracks.active_event_tag(2,0x54);
        if(requested(0x48)==17)force_human_arbitration_action(look,true);
        break;
    case 22:{auto& request=look.frame_words();request.set(0xd0,request.template get<std::uint32_t>(0xd0)|1u);request.set(0xd0,request.template get<std::uint32_t>(0xd0)|8u);}
        [[fallthrough]];
    case 23:
        equipment.state().select(look.action_model(),6,0xffffffffu,-7,-1);equipment.transfers().event(9);
        equipment.set_action_holster_visible(false);
        look.mark_frame_feedback(0x200);
        if(requested(0x48)==65){action.set(0x44,65u);tracks.active_event_tag(2,0x53);force_human_arbitration_action(look,true);}
        else tracks.active_event_tag(2,0x55);
        break;
    case 25:
        if(!equipment.state().selected_model(7)&&copy_cards(-2,false))equipment.transfers().event(11);
        tracks.active_event_tag(2,0x50);look.action_command(75,true);return;
    case 26:action.set(0x44,25u);tracks.active_event_tag(2,0x51);break;
    case 28:(void)copy_cards(-1,false);tracks.active_event_tag(2,0x50);break;
    case 29:equipment.state().select(look.action_model(),7,0xffffffffu,-4,-1);tracks.active_event_tag(2,0x52);break;
    case 52:case 65:case 66:case 71:case 72:case 74:return;
    case 70:tracks.pause_at_clip_fraction(2,.5f);return;
    case 73:{
        const auto model=look.action_model();
        if(!model)throw std::runtime_error("Human secondary action requires its actual retained model");
        auto retained=frames.retain_actor_frame(model);retained.release(); //Native E4 increment transfers one reference to230.
        equipment.state().select(model,7,0xffffffffu,-8,-1);equipment.transfers().event(11);look.action_command(75,true);tracks.active_event_tag(2,0x53);
        const auto index=static_cast<std::int64_t>(73)*3+requested(0x4c);
        if(index<0||index>=225)throw std::runtime_error("Human secondary suspension exceeds its actual animation table");
        const auto clip=services.human_animation_row(9,static_cast<unsigned>(index)).animation.get(); //0075CCC0(73,posture), raw clip duration.
        const auto duration=clip?frames.human_clip_duration(clip):-1;
        control.geometry().suspend_equipment_weapon(human_animation_time_integer(static_cast<long double>(duration)*.625f),false);
        break;
    }
    default:if(current[0]<30||current[0]>51)tracks.active_event_tag(2,0x51);break;
    }
    if(command(0x44)<75||command(0x44)==76){
        const auto index=static_cast<std::int64_t>(current[0])*3+current[1];
        if(index<0||index>=225)throw std::runtime_error("Human secondary wait exceeds its actual animation table");
        if(const auto clip=services.human_animation_row(9,static_cast<unsigned>(index)).animation.get()){
            const auto duration=frames.human_clip_duration(clip);action.set(0x4c,duration);
            const auto scale=services.human_control_animation_scale();
            if(command(0x44)==65||current[0]==73)action.set(0x4c,human_animation_time_integer(static_cast<long double>(duration)/scale));
            else action.set(0x4c,human_animation_time_integer(static_cast<long double>(duration)*(1.L/scale)-(1.L/scale)*560.f)); //77C5D2, preserve x87 subtraction order.
            return;
        }
    }
    action.set(0x4c,std::int32_t{-10000});
}

template<class Control,class Look,class Human,class Services,class Frames,class State>
void arbitrate_human_secondary_animation(Control& control,Look& look,Human& human,Services& services,
        Frames& frames,State& scratch){ //007793F0 and0077CD30 preprocessing.
    auto& words=control.physical().control_frame_words();auto& action=control.action().frame_words();
    auto& tracks=control.tracks();auto& equipment=services.human_presentation_equipment(human);
    auto& desired=scratch.requested_secondary;auto& current=scratch.secondary;
    const auto word=[&](unsigned offset){return words.template get<std::int32_t>(offset);};
    const auto command=[&](unsigned offset){return action.template get<std::int32_t>(offset);};
    const auto requested=[&](unsigned offset){return look.state().template get<std::int32_t>(offset);};
    const auto scale=services.human_control_animation_scale();
    if(desired[0]==19){
        desired[0]=76;equipment.toggle_frame_action(false);equipment.transfers().event(10);
        tracks.active_event_tag(2,-1);action.set(0x58,std::int32_t{-10000});action.set(0x4c,std::int32_t{-10000});
    }else if(desired[0]==61){desired[0]=76;force_human_arbitration_action(look,true);}
    else if(desired[0]==65){
        auto& items=services.human_reload_items();
        if(const auto entry=find_player_mode_holster(human,items,14,services.game_word28()))
            look.action_model(items.runtime_item(entry).model(),true);
        desired[0]=23;
    }
    if(tracks.active_event_tag(2)>=0||word(0xa20)==2||word(0xa20)==1)look.mark_frame_feedback(0x200);
    (void)tracks.consume_event_tag(2,0x51);
    if(tracks.consume_event_tag(2,0x52))equipment.transfers().event(6);
    else if(tracks.consume_event_tag(2,0x53)){
        if(current[0]==65&&desired[0]==75)tracks.active_event_tag(2,0x53);
        else{
            equipment.set_action_holster_visible(true); //007918C0, SAME actual holster group4.
            equipment.transfers().event(8);
            auto& request=look.frame_words();request.set(0xd0,request.template get<std::uint32_t>(0xd0)&~1u);
            request.set(0xd0,request.template get<std::uint32_t>(0xd0)&~8u);
        }
    }else if(tracks.consume_event_tag(2,0x55)){
        if(current[0]==65&&desired[0]==75)tracks.active_event_tag(2,0x55);
        else{
            equipment.set_action_holster_visible(true);
            const auto count=word(0x2b8);
            if(count<0||count>=5)throw std::runtime_error("Human secondary event exceeds its native five-word scratch");
            words.set(0x2a4+static_cast<unsigned>(count)*4,12u);words.set(0x2b8,count+1);
            equipment.transfers().event(8);
            auto& request=look.frame_words();request.set(0xd0,request.template get<std::uint32_t>(0xd0)&~1u);
            request.set(0xd0,request.template get<std::uint32_t>(0xd0)&~8u);
        }
    }else if(tracks.consume_event_tag(2,0x50)&&current[0]!=26){
        equipment.toggle_frame_action(false);equipment.transfers().event(10);
    }else if(tracks.consume_event_tag(2,0x54))action.set(0x58,static_cast<std::int32_t>(services.human_frame_random_word()%10000u+10000u));
    bool select=true;
    if(command(0x4c)==-10000){
        if(word(0x218)==75&&command(0x58)!=-10000&&command(0x58)<1){
            action.set(0x58,std::int32_t{-10000});words.set(0x218,18u);desired[0]=18;
        }
        scratch.secondary_scale=1; //00774B10 returns the actual constant0080F524.
    }else if(tracks.track_reached(2,command(0x4c))||(command(0x44)==65&&word(0x2cc)>0)){
        if(requested(0x48)!=75){action.set(0x44,requested(0x48));look.action_command(75,true);}
        if(command(0x44)==76){
            desired[0]=76;action.set(0x4c,std::int32_t{-10000});
            auto& request=look.frame_words();request.set(0xd0,request.template get<std::uint32_t>(0xd0)&~action.template get<std::uint32_t>(0x54));
        }else if(!(command(0x50)==1&&desired==action.template get<typename State::Secondary>(0x44))){
            if(command(0x50)==0)desired=action.template get<typename State::Secondary>(0x44);
            else tracks.motion_animation_rate(2,0);
            action.set(0x50,0u);action.set(0x4c,std::int32_t{-10000});
            auto& request=look.frame_words();request.set(0xd0,request.template get<std::uint32_t>(0xd0)&~action.template get<std::uint32_t>(0x54));
        }else{words.set(0x2c8,0u);words.set(0x2cc,0u);}
    }else{
        if(desired[0]!=current[0]&&word(0x2cc)>0&&desired[0]!=75){
            tracks.resume(2);action.set(0x44,76u);action.set(0x4c,200u);force_human_arbitration_action(look,true);
        }
        desired[0]=75;select=false;
    }
    if(select&&desired[0]!=75&&desired!=current){
        if(desired[0]==76){
            tracks.fade_pair(2,human_animation_time_integer((current[0]==65?1000.L:300.L)/scale));
            current=desired;force_human_arbitration_action(look,false);
        }else{
            current=desired;scratch.secondary_table=9;tracks.motion_animation_rate(1,1);
            const auto index=static_cast<std::int64_t>(current[0])*3+current[1];
            if(index<0||index>=225)throw std::runtime_error("Human secondary animation exceeds its actual table");
            const auto& row=services.human_animation_row(9,static_cast<unsigned>(index));
            auto& request=scratch.pending[2];scratch.selected_clips[2]=row.animation.get();request.animation=scratch.selected_clips[2];
            request.weight=1;request.blend=static_cast<float>(static_cast<long double>(human_animation_word_float(row.words[1]))/scale*scratch.secondary_scale);
            request.flags=row.words[0]&0xeu;
            if(!scratch.selected_clips[2]){
                current=scratch.previous_secondary;
                if(current[0]==76)words.set(0x1f4,75u);
                desired[0]=75;return;
            }
            const bool moving=word(0x1d8)>=190&&word(0x1d8)<=206;
            if(current[0]>=30&&current[0]<=51){
                request.weight=moving?.5f:.6f;
                for(unsigned index=0;index!=scratch.masked_bones.size();++index)scratch.masked_bones[index]=control.physical().bones().at(index+8);
                scratch.mask_count=7;
            }else if(current[0]>=0&&current[0]<=2){if(moving)request.weight=.75f;}
            else if(current[0]==66)request.weight=.65f;
        }
    }
    if(scratch.selected_clips[2]){
        if(current[0]<8)control.geometry().select_secondary_pose(scratch.selected_clips[2],control.mode_flags()); //007AE320.
        else{
            words.set(0x1f4,current);const bool blocking=human_secondary_animation_blocking(current[0]);
            frames.human_clip_word8(scratch.selected_clips[2],blocking?1u:2u);
            tracks.select(scratch.pending,scratch.requested_primary,scratch.requested_equipment,desired,blocking);
            scratch.outgoing[2]=scratch.pending[2];scratch.pending[2].animation={};
            if(current[0]==65)scratch.pending[2].separate=false;
        }
        transition_human_secondary_animation(control,look,human,services,frames,scratch);
        scratch.changed=true;
    }else{
        if(current[0]==76)words.set(0x1f4,75u);
        desired[0]=75;
    }
}

template<class Control,class Look,class Human,class Services,class Frames,class State>
void transition_human_equipment_animation(Control& control,Look& look,Human& human,Services& services,
        Frames& frames,const State& scratch){ //0077B870.
    auto& action=control.action().frame_words();auto& words=control.physical().control_frame_words();
    auto& geometry=control.geometry();auto& equipment=services.human_presentation_equipment(human);
    const auto& current=scratch.equipment;const auto& previous=scratch.previous_equipment;
    const auto command=[&](unsigned offset){return action.template get<std::int32_t>(offset);};
    const auto requested=[&](unsigned offset){return look.state().template get<std::int32_t>(offset);};
    const auto word=[&](unsigned offset){return words.template get<std::int32_t>(offset);};
    if(!human_equipment_animation_blocking(current[0])||command(0x38)!=-10000)return;
    action.set(0x2c,previous);action.set(0x30,requested(0x4c));action.set(0x34,requested(0x50));
    const auto old_kind=requested(0x54);
    bool suspend=false,posture_queue=false,draw_queue=false,special_attachment=false,item_action=false;
    const auto is_current_player=[&]{return (control.mode_flags()&2u)&&services.current_human_frame_player()==&human;};
    const auto first_person=[&](std::int32_t animation){services.human_frame_item_action_fpv(human,animation);};
    switch(current[0]){
    case 34:
        if(is_current_player())control.play_presentation_track_sound(11);
        action.set(0x2c,92u);action.set(0x40,0x100u);look.force_equipment_command(word(0x220));words.set(0xa20,1u);break;
    case 35:case 36:{
        if(current[0]==35)words.set(0x190,words.template get<std::uint32_t>(0x190)&~1u);
        if(is_current_player()){
            auto& items=services.human_reload_items();auto* storage=human.base().storage();
            const auto hand=[&]{
                if(!storage||!storage->holder(3))throw std::runtime_error("Human draw transition requires its actual hand holder");
                return storage->holder(3)->entry();
            };
            auto sound=10u;
            if(hand()){
                const auto entry=hand();if(!entry)throw std::runtime_error("Human draw transition lost its actual hand item");
                if(items.runtime_item(entry).definition().kind()==9)sound=12;
            }
            control.play_presentation_track_sound(sound);
        }
        words.set(0xa20,2u);draw_queue=true;words.set(0xa24,0u);action.set(0x40,0x100u);
        const bool outside_walk=word(0x1d8)<41||word(0x1d8)>65;
        if(requested(0x5c)==0){
            if(requested(0x54)==6||(requested(0x4c)==2&&outside_walk))action.set(0x2c,92u);
            else action.set(0x2c,human.properties.field19c==1?30u:31u);
        }else if(requested(0x5c)==1){
            if(requested(0x54)==6||(equipment.state().words().template get<std::uint8_t>(0x50)&1u)||requested(0x54)==100||
               (requested(0x4c)==2&&outside_walk))action.set(0x2c,92u);
            else action.set(0x2c,32u);
        }
        break;
    }
    case 37:action.set(0x2c,92u);break;
    case 76:case 77:
        item_action=true;
        emit_human_equipment_transition_sound(human,services.human_reload_items(),services,false);
        [[fallthrough]];
    case 70:case 71:if(is_current_player())first_person(2);action.set(0x2c,92u);break;
    case 79:case 90:
        item_action=true;
        emit_human_equipment_transition_sound(human,services.human_reload_items(),services,true);
        equipment.state().select(look.action_model(),7,0xffffffffu,-6,-1);
        if(command(0x2c)==91)action.set(0x2c,92u);
        special_attachment=true;break;
    case 80:case 81:case 82:
        if(is_current_player())first_person(6);
        posture_queue=true;suspend=true;break;
    default:break;
    }
    const auto restore_kind=[&]{
        look.force_equipment_command(old_kind);
        if(command(0x38)!=-10000){auto& request=look.frame_words();request.set(0xd0,request.template get<std::uint32_t>(0xd0)|action.template get<std::uint32_t>(0x40));}
    };
    if(command(0x2c)>90&&command(0x2c)!=92){action.set(0x38,std::int32_t{-10000});restore_kind();return;}
    std::optional<unsigned> table;
    if(requested(0x54)==0||requested(0x54)==6)table=2;
    else if(requested(0x54)>=1&&requested(0x54)<=5)table=static_cast<unsigned>(requested(0x54))+2;
    if(!table){action.set(0x38,std::int32_t{-10000});restore_kind();return;}
    const auto index=static_cast<std::int64_t>(current[0])*3+current[1];
    if(index<0||index>=273)throw std::runtime_error("Human equipment transition exceeds its actual animation table");
    const auto clip=services.human_animation_row(*table,static_cast<unsigned>(index)).animation.get();
    if(!clip){action.set(0x38,std::int32_t{-10000});restore_kind();return;}
    const auto duration=frames.human_clip_duration(clip);action.set(0x38,duration);
    const auto scale=services.human_control_animation_scale();
    if(suspend)geometry.suspend_equipment_weapon(duration,special_attachment);
    if(special_attachment)words.set(0xa24,human_animation_time_integer(static_cast<long double>(command(0x38))/scale));
    auto& queue=control.physical().control_action_queue();
    const auto enqueue=[&](std::uint32_t kind){
        queue.push_back({{static_cast<std::uint32_t>(human_animation_time_integer(static_cast<long double>(command(0x38))/scale)),kind,1}});
    };
    if(item_action){
        geometry.suspend_equipment_for_item_action(human_animation_time_integer(static_cast<long double>(command(0x38))/scale));
        if(requested(0x4c)==2)enqueue(14);
    }else if(posture_queue&&(requested(0x4c)==2||requested(0x54)==5))enqueue(13);
    else if(draw_queue&&requested(0x4c)==2)enqueue(15);
    long double trailing_time;
    if(command(0x2c)==92){
        const auto time=human_animation_time_integer(static_cast<long double>(command(0x38))*(1.L/scale)-(1.L/scale)*420.f); //77BFD9.
        action.set(0x38,time);trailing_time=time;
    }else{
        const auto followup=static_cast<std::int64_t>(command(0x2c))*3+command(0x30);
        if(followup<0||followup>=273)throw std::runtime_error("Human equipment follow-up exceeds its actual animation table");
        trailing_time=static_cast<long double>(command(0x38))/scale;
        const auto stored_duration=static_cast<float>(command(0x38)); //77C015 writes FLOAT before subtracting the follow-up row's raw blend.
        const auto blend=human_animation_word_float(services.human_animation_row(*table,static_cast<unsigned>(followup)).words[2]);
        const auto intermediate=human_animation_time_integer(static_cast<long double>(stored_duration)-blend);action.set(0x38,intermediate);
        action.set(0x38,human_animation_time_integer(static_cast<long double>(intermediate)/scale));
    }
    if(current[0]==34||current[0]==35||current[0]==36)words.set(0xa1c,human_animation_time_integer(trailing_time));
    restore_kind();
}

template<class Control,class Look,class Human,class Services,class Frames,class State,class Publish>
void arbitrate_human_equipment_animation(Control& control,Look& look,Human& human,Services& services,
        Frames& frames,State& scratch,Publish& publish){ //00778B20.
    auto& words=control.physical().control_frame_words();auto& action=control.action().frame_words();
    auto& tracks=control.tracks();auto& geometry=control.geometry();
    auto& desired=scratch.requested_equipment;auto& current=scratch.equipment;
    const auto word=[&](unsigned offset){return words.template get<std::int32_t>(offset);};
    const auto command=[&](unsigned offset){return action.template get<std::int32_t>(offset);};
    const auto requested=[&](unsigned offset){return look.state().template get<std::int32_t>(offset);};
    const auto scale=services.human_control_animation_scale();
    constrain_human_equipment_animation(control,look,human,services,scratch,publish);
    desired=words.template get<typename State::Secondary>(0x20c);
    if(command(0x38)==-10000){
        if(desired[0]<80||desired[0]>82)scratch.equipment_scale=tracks.choose_equipment(desired);
    }else if(!tracks.track_reached(1,command(0x38))){
        if(desired[0]!=91)tracks.queue_equipment(desired);
        if(desired[0]==35||desired[0]==36)action.set(0x2c,desired);
        desired[0]=91;return;
    }else if(command(0x2c)==92){
        desired[0]=92;action.set(0x38,std::int32_t{-10000});
        auto& request=look.frame_words();request.set(0xd0,request.template get<std::uint32_t>(0xd0)&~action.template get<std::uint32_t>(0x40));
    }else if(command(0x3c)==1&&desired==action.template get<typename State::Secondary>(0x2c)){
        words.set(0x29c,0u);words.set(0x2a0,0u);
    }else{
        if(command(0x3c)==0){
            if(command(0x2c)>=30&&command(0x2c)<=32&&(requested(0x54)==0||requested(0x54)==6))desired[0]=92;
            else desired=action.template get<typename State::Secondary>(0x2c);
        }else tracks.motion_animation_rate(1,0);
        action.set(0x3c,0u);action.set(0x38,std::int32_t{-10000});
        auto& request=look.frame_words();request.set(0xd0,request.template get<std::uint32_t>(0xd0)&~action.template get<std::uint32_t>(0x40));
    }
    if(desired[0]!=91&&(!human_animation_family_matches(current[0],desired[0])||desired[1]!=current[1])){
        if(desired[0]==32)geometry.blend_equipment_weapon(current[0]>=80&&current[0]<=82?0:300);
        else if(desired[0]!=39)geometry.unblend_equipment_weapon(desired[0]>=80&&desired[0]<=82?0:300); //007B8D60.
        if(desired[0]==92){
            current=desired;
            const auto mode=word(0xa20);
            if(mode==3||mode==4){
                long double fade,duration;
                if(desired[1]==2){fade=200;duration=100;scratch.changed=true;}
                else if(scratch.primary[0]>=8&&scratch.primary[0]<=14){fade=500;duration=250;}
                else if(mode==4){fade=250;duration=125;}
                else{fade=400;duration=200;}
                tracks.fade_pair(1,human_animation_time_integer(fade/scale));
                geometry.set_weapon_blend_duration(human_animation_time_integer(duration/scale));
                if((word(0xa20)!=4&&word(0xa20)!=3)||requested(0x54)==0||requested(0x54)==100)words.set(0xa20,0u);
            }else{
                tracks.fade_pair(1,human_animation_time_integer(350.L/scale));
                if(requested(0x54)!=6&&(requested(0x54)==0||requested(0x54)==100))words.set(0xa20,0u);
            }
        }else{
            current=desired;
            if(desired[0]<91){
                if(desired[0]==32&&scratch.previous_equipment[0]!=32)words.set(0xa28,human_animation_time_integer(150.L/scale));
                const auto old_kind=requested(0x54);
                if(current[0]==34)look.force_equipment_command(word(0x220));
                const auto kind=requested(0x54);
                scratch.equipment_table.reset();
                if(kind==0||kind==6)scratch.equipment_table=2;
                else if(kind>=1&&kind<=5)scratch.equipment_table=static_cast<unsigned>(kind)+2;
                look.force_equipment_command(old_kind);
                if(current[1]<0||current[1]>4)current[1]=requested(0x4c);
                if(scratch.equipment_table){
                    tracks.motion_animation_rate(1,1);
                    const auto index=static_cast<std::int64_t>(current[0])*3+current[1];
                    if(index<0||index>=273)throw std::runtime_error("Human equipment animation exceeds its actual table");
                    const auto& row=services.human_animation_row(*scratch.equipment_table,static_cast<unsigned>(index));
                    auto& request=scratch.pending[1];scratch.selected_clips[1]=row.animation.get();request.animation=scratch.selected_clips[1];
                    request.blend=static_cast<float>(static_cast<long double>(human_animation_word_float(row.words[2]))/scale*scratch.equipment_scale);
                    geometry.set_weapon_blend_duration(human_animation_time_integer(static_cast<long double>(request.blend)*.5f));
                    request.weight=human_animation_word_float(row.words[1]);request.flags=row.words[0]&0xeu;
                }
                if(!scratch.selected_clips[1]){
                    current=scratch.previous_equipment;
                    if(current[0]==92)words.set(0x1e8,91u);
                    desired[0]=91;return;
                }
                if((current[0]==30||current[0]==31||current[0]==32)&&scratch.previous_equipment[0]==current[0]&&current[1]!=scratch.previous_equipment[1])
                    scratch.pending[1].blend=static_cast<float>(550.L/scale*scratch.equipment_scale);
            }
        }
    }
    if(scratch.selected_clips[1]){
        if(current[0]==39){geometry.select_equipment_overlay(control.mode_flags());return;} //007AE390 ignores its clip argument.
        words.set(0x1e8,current);const bool blocking=human_equipment_animation_blocking(current[0]);
        frames.human_clip_word8(scratch.selected_clips[1],blocking?1u:2u);
        tracks.select(scratch.pending,scratch.requested_primary,desired,scratch.requested_secondary,blocking);
        scratch.outgoing[1]=scratch.pending[1];scratch.pending[1].animation={};
        transition_human_equipment_animation(control,look,human,services,frames,scratch);
        scratch.changed=true;
    }else{
        if(current[0]==92)words.set(0x1e8,91u);
        desired[0]=91;
    }
}

template<class Control,class Look,class Human,class Services,class Frames,class Runtime,class Publish>
void arbitrate_human_animation_frame(Control& control,Look& look,Human& human,Services& services,Frames& frames,Runtime& runtime,Publish& publish){ //00779B30.
    auto& scratch=services.human_animation_arbitration();
    auto& words=control.physical().control_frame_words();
    using State=std::remove_reference_t<decltype(scratch)>;
    const auto primary=[&](unsigned base){return words.template get<typename State::Primary>(base);};
    const auto secondary=[&](unsigned base){return words.template get<typename State::Secondary>(base);};
    scratch.changed=false;scratch.selected_clips.fill({});
    scratch.primary_scale=1;scratch.primary_blend=-1;scratch.equipment_scale=scratch.secondary_scale=1;
    scratch.previous_primary={{206,0,1}};scratch.previous_equipment={{91,0}};scratch.previous_secondary={{75,0}};
    scratch.mask_count=-1;
    scratch.primary=primary(0x1d8);scratch.equipment=secondary(0x1e8);scratch.secondary=secondary(0x1f4);
    scratch.requested_primary=primary(0x1fc);scratch.requested_equipment=secondary(0x20c);scratch.requested_secondary=secondary(0x218);
    for(unsigned group=0;group!=4;++group){
        scratch.pending[group].animation={};scratch.outgoing[group].animation={};
        if(group<3)scratch.pending[group].separate=true;
    }
    scratch.previous_primary=primary(0x1d8);scratch.previous_equipment=secondary(0x1e8);scratch.previous_secondary=secondary(0x1f4);
    if(scratch.primary[0]>=113&&scratch.primary[0]<=154)return;
    arbitrate_human_primary_animation(control,look,human,services,frames,runtime,scratch,publish); //00777AD0.
    arbitrate_human_secondary_animation(control,look,human,services,frames,scratch);
    arbitrate_human_equipment_animation(control,look,human,services,frames,scratch,publish);
    auto& tracks=control.tracks();
    if(scratch.changed){
        if(scratch.mask_count>=0)tracks.mask_bones(2,scratch.masked_bones,scratch.mask_count);
        tracks.update(1,true,&scratch.outgoing); //007724E0, first native DWORD0 is unused.
    }
    scratch.requested_primary[0]=206;normalize_human_arbitration_primary(scratch.requested_primary);
    scratch.requested_equipment[0]=91;scratch.requested_secondary[0]=75;
    words.set(0x1fc,scratch.requested_primary); //00782FA0 leaves native208 untouched.
    words.set(0x20c,scratch.requested_equipment);words.set(0x218,scratch.requested_secondary);
}

} //namespace hd2::game
#endif
