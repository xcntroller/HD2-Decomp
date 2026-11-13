#ifndef HD2_GAME_HUMAN_DAMAGE_H
#define HD2_GAME_HUMAN_DAMAGE_H
#include "game/human_ai_shot_direction.h"
#include "game/human_loading_stats.h"
#include "game/human_state_frame_timers.h"
#include <array>

namespace hd2::game {
//005A1C90. Reached World/state/effect endpoints must supply their real bodies;
//this function never substitutes a successful health-only damage service.
template<class Human,class Services,class Frames>
void damage_human(Human& human,std::uint32_t kind,float amount,const HumanVector& point,
        const HumanVector& impulse,Human* source,std::uint32_t hit_kind,Services& services,Frames& frames){
    const auto mode=services.mode_flags();const bool offline=(mode&7u)==7u;
    auto& health=human.properties.stats_ec[0];
    const auto alive=[&]{return human_frame_signed_word(health)>0;};
    if(offline){
        if(services.human_damage_console(5)&&services.human_difficulty()!=4&&human.properties.field19c==2)amount=-1.f;
        if(services.human_damage_protection()&&services.human_damage_roster_contains(human))return;
    }
    if(kind==14){
        health=0;services.notify_human_damage(human,14,hit_kind,0,source,point,HumanVector{{0,1,0,0}});
        services.refresh_human_damage_geometry(human);services.stop_human_damage_motion(human);return;
    }
    auto direction=impulse;
    const auto square=(static_cast<long double>(direction[0])*direction[0]+static_cast<long double>(direction[2])*direction[2])+static_cast<long double>(direction[1])*direction[1];
    normalize_human_ai_shot(direction,square);
    std::int32_t reflected=0;
    const auto retaliate=[&](float power){
        if(!source)throw std::logic_error("Human damage retaliation requires its actual source");
        auto reverse=services.human_damage_target_direction(*source);for(unsigned axis=0;axis!=3;++axis)reverse[axis]=-reverse[axis];
        const auto selected=services.human_damage_random_position(*source);
        damage_human(*source,kind,power,selected,reverse,source,hit_kind,services,frames);
    };
    if(offline){
        if(source&&services.human_is_player(*source)&&!services.human_integrity_matches(34)&&
                static_cast<float>(static_cast<std::int32_t>(services.human_frame_random_word()))<327.66998291015625f)retaliate(amount);
    }else if(!services.human_damage_network_policy(human,kind,source,reflected))return;
    if((offline||services.human_remote_ai_enabled())&&amount!=-1.f&&kind==1){
        const auto difficulty=services.human_difficulty();const auto identity=human.properties.field19c;
        if(identity==1||identity==4){
            if(difficulty==1)amount*=.20000000298023224f;
            else if(difficulty==2)amount*=.4000000059604645f;
            else if(difficulty==4)amount*=1.2000000476837158f;
        }else if(difficulty==1&&source&&source->properties.field19c==1)amount*=1.2000000476837158f;
    }
    std::uint32_t material=0x42; //0059C2A0, genuine Human virtualD8 constant; absent frozen.
    std::uint32_t mask=8,damage=0;
    const auto truncate=[](long double value){return human_loading_stat_word(value);};
    switch(hit_kind){
    case 0:damage=truncate(amount);break;
    case 1:damage=truncate(static_cast<long double>(amount)*5.f);mask=1;break;
    case 2:case 3:damage=truncate(static_cast<long double>(amount)*double{.2});mask=2;break;
    case 4:case 5:damage=truncate(static_cast<long double>(amount)*double{.2});mask=4;break;
    case 6:case 7:damage=truncate(static_cast<long double>(amount)*double{.33});mask=0x10;break;
    case 8:case 9:damage=truncate(static_cast<long double>(amount)*double{.33});mask=0x20;break;
    case 11:{
        const auto draw=frames.volume_random_unit();
        if(draw<.30000001192092896f){
            if((offline||services.human_remote_ai_enabled())&&human.properties.field19c==1&&services.human_difficulty()==1)damage=truncate(amount);
            else damage=truncate(static_cast<long double>(amount)*5.f);
        }else damage=truncate(static_cast<long double>(amount)*.20000000298023224f);
        material=12;mask=1;break;
    }
    default:break;
    }
    if((mode&2u)&&kind!=3){
        if(kind==1){
            services.human_damage_impact_sound(human,material,point,!offline);services.human_damage_blood_surface(point,direction);
            auto reverse=impulse;
            const auto length=(static_cast<long double>(reverse[0])*reverse[0]+static_cast<long double>(reverse[1])*reverse[1])+static_cast<long double>(reverse[2])*reverse[2];
            normalize_human_ai_shot(reverse,length);for(unsigned axis=0;axis!=3;++axis)reverse[axis]=-reverse[axis];
            services.human_damage_material_effect(material,point,reverse);
        }else if(kind==2)services.human_damage_impact_sound(human,material,point,!offline);
    }
    if(!alive()){
        auto force=direction;for(unsigned axis=0;axis!=3;++axis)force[axis]*=amount;
        services.human_damage_hit(human,mask,point,force);return;
    }
    switch(kind){case 0:mask|=0x1000u;break;case 9:mask|=0x800u;break;case 10:mask|=0x400u;break;
        case 11:mask|=0x2000u;break;case 13:mask|=0x4000u;break;case 15:mask|=0x8000u;break;default:break;}
    if(kind!=5){
        auto force=impulse;for(unsigned axis=0;axis!=3;++axis)force[axis]*=amount==-1.f?1000.f:amount;
        services.human_damage_hit(human,mask,point,force);
    }
    if(amount==-1.f)health=0;else health-=damage;
    auto& injuries=human.activation.field15c;
    if(human.properties.field19c!=1&&(mask&0x30u)&&human_frame_signed_word(injuries)<3){
        ++injuries;if(injuries==3)services.human_damage_limb_limit(human);
    }
    if(!alive()){
        health=0;services.human_damage_death_source(human,source);
        if(source)services.human_damage_kill_credit(*source,human,kind);
        services.human_damage_script_death(human);services.human_damage_death_cleanup(human);
        if(!offline)services.human_damage_death_packet(human,source);
    }
    if(!(mode&2u)&&source)services.human_damage_packet(human,kind,hit_kind,*source);
    services.notify_human_damage(human,kind,hit_kind,human_frame_signed_word(health),source,point,direction);
    if(kind!=10){
        if(!alive()){
            services.human_damage_stop_speech(human);
            if(kind==11)services.human_damage_speech(human,0x74,200,1);
            else if(hit_kind!=1)services.human_damage_speech(human,0x73,200,0x20);
        }else if(!services.human_damage_speaking(human))services.human_damage_speech(human,0x75,200,0x20);
    }
    if(reflected>0)retaliate(static_cast<float>((static_cast<long double>(reflected)*.009999999776482582f)*amount));
}
} //namespace hd2::game
#endif
