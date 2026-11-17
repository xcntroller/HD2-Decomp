#ifndef HD2_GAME_HUMAN_AI_FRAME_METRICS_H
#define HD2_GAME_HUMAN_AI_FRAME_METRICS_H
#include "game/human_joint_owners.h"
#include "game/human_state_frame_timers.h"
#include <cmath>

namespace hd2::game {
//00570CA0, selected only by005710B0's actual Actor98 gate. Unlike the
//ordinary selector, a single mounted candidate runs the full reverse score.
template<class Human,class State,class Frames,class World>
Human* select_human_mounted_hostile(State& state,Frames& frames,World& world){
    auto& human=*state.human();auto* attachment=human.field160;
    if(!attachment)return nullptr;
    const auto kind=world.human_seat_weapon_kind(attachment,human.field164);if(!kind)return nullptr;
    const auto origin=frames.actor_world_matrix(human.base().frame());float best=-100000.f;Human* selected=nullptr;
    const auto& enemies=human.perception.lists()[0];
    for(auto n=enemies.size();n;){
        auto* target=dynamic_cast<Human*>(enemies[--n]);if(!target)throw std::logic_error("Mounted hostile list requires actual Human");
        if(human_frame_signed_word(target->properties.stats_ec[0])<1||world.human_unavailable(*target)||(target->properties.field19c==2&&target->loading_state&&target->loading_state->suppress_hostile_selection()))continue;
        if(*kind!=2&&human.properties.field19c==1&&target->field160&&target->field160->base().type()==4&&world.car_field5e0(target->field160))continue;
        const auto pose=frames.actor_world_matrix(target->base().frame());HumanVector delta{{static_cast<float>(static_cast<long double>(pose[12])-origin[12]),static_cast<float>(static_cast<long double>(pose[13])-origin[13]),static_cast<float>(static_cast<long double>(pose[14])-origin[14]),0}};
        const float square=static_cast<float>((static_cast<long double>(delta[0])*delta[0]+static_cast<long double>(delta[1])*delta[1])+static_cast<long double>(delta[2])*delta[2]);
        std::int32_t bits;std::memcpy(&bits,&square,4);constexpr float epsilon=9.99999993922529e-09f;
        const bool unit=std::abs(static_cast<long double>(square)-1)<epsilon;
        const auto distance=bits<0x322bcc77?0.L:unit?1.L:std::sqrt(static_cast<long double>(square));const float capped=distance>100?100.f:static_cast<float>(distance);
        auto* enemy=target->loading_state;if(!enemy)throw std::logic_error("Mounted hostile score requires actual B0");float score=0;
        if(enemy->words().template get<std::uint32_t>(0x48)==1){
            if(!unit){if(square>=epsilon){const auto scale=1/std::sqrt(static_cast<long double>(square));for(unsigned a=0;a<3;++a)delta[a]=static_cast<float>(delta[a]*scale);}else{const unsigned a=delta[0]!=0?0:delta[2]!=0?2:1;delta[a]=delta[a]<0?-1.f:1.f;}}
            const auto aim=enemy->target_direction();if((-static_cast<long double>(delta[0])*aim[0]-static_cast<long double>(delta[1])*aim[1])-static_cast<long double>(delta[2])*aim[2]>.9f)score=50.f;
        }else if(enemy->aims_at_human(human))score=50.f;
        const HumanVector position{{pose[12],pose[13],pose[14],pose[15]}};
        if(world.human_emplacement_can_target(attachment,human.field164,position))score+=20.f;
        score=static_cast<float>((100.L-capped)*.5f+score);
        if(enemy->words().template get<std::int32_t>(0xb0)<400)score+=20.f;
        const auto flags=enemy->words().template get<std::uint32_t>(0x50)&3u;if(!flags)score+=10.f;else if(flags&2u)score+=5.f;
        if(score>best){best=score;selected=target;}
    }
    return selected;
}

//005710B0, ordinary Human without an Actor98-controlled mount. The single candidate does not run the
//distance, direction, Sight or friendly-selection scoring pass.
template<class Human,class State,class Frames,class World>
Human* select_human_hostile(State& state,Frames& frames,World& world,const HumanVector* filter=nullptr){
    auto* human=state.human();
    if(!human)throw std::logic_error("Hostile selection requires the bound Human");
    const auto& enemies=human->perception.lists()[0];
    if(enemies.empty())return nullptr;
    if(human->field160&&world.human_pose_actor_has_control(human->field160,human->field164))
        return select_human_mounted_hostile<Human>(state,frames,world);
    const auto actual=[](auto* actor)->Human&{
        auto* target=dynamic_cast<Human*>(actor);
        if(!target)throw std::logic_error("Hostile selection requires actual Human perception entries");
        return *target;
    };
    const auto excluded=[&](const Human& target){
        if(world.human_unavailable(target))return true;
        return target.properties.field19c==2&&target.loading_state&&target.loading_state->suppress_hostile_selection(); //005A6450.
    };
    const auto alive=[](const Human& target){return human_frame_signed_word(target.properties.stats_ec[0])>0;};
    if(enemies.size()==1){
        auto& target=actual(enemies.front());
        return !excluded(target)&&alive(target)?&target:nullptr;
    }
    const auto model=[&](const Human& actor){
        if(!actor.base().frame())throw std::logic_error("Hostile selection requires the actual Human model");
        return frames.actor_world_matrix(actor.base().frame());
    };
    const auto own=model(*human);
    const auto dot=[](const HumanVector& a,const HumanVector& b){
        return (static_cast<long double>(a[2])*b[2]+static_cast<long double>(a[1])*b[1])+
            static_cast<long double>(a[0])*b[0];
    };
    constexpr float epsilon=9.99999993922529e-09f;
    Human* best=nullptr;
    float best_score=-100000.f;
    for(auto n=enemies.size();n;){
        auto& target=actual(enemies[--n]);
        if(!alive(target)||excluded(target))continue;
        const auto pose=model(target);
        HumanVector direction{};
        for(unsigned axis=0;axis!=3;++axis)
            direction[axis]=static_cast<float>(static_cast<long double>(pose[12+axis])-own[12+axis]);
        const float square=static_cast<float>(dot(direction,direction));
        std::uint32_t bits;
        std::memcpy(&bits,&square,sizeof bits);
        const bool unit=std::abs(static_cast<long double>(square)-1)<epsilon;
        float distance=0;
        if(human_frame_signed_word(bits)>=0x322bcc77){
            const auto length=unit?1.L:std::sqrt(static_cast<long double>(square));
            distance=length>100.L?100.f:static_cast<float>(length);
        }
        if(!unit){
            if(square>=epsilon){
                const auto inverse=1/std::sqrt(static_cast<long double>(square));
                for(unsigned axis=0;axis!=3;++axis)direction[axis]=static_cast<float>(direction[axis]*inverse);
            }else{
                const unsigned axis=direction[0]!=0?0:direction[2]!=0?2:1;
                direction[axis]=direction[axis]<0?-1.f:1.f;
            }
        }
        if(filter&&!(dot(direction,*filter)>=.9f))continue;
        const float facing=static_cast<float>(dot(direction,state.target_direction()));
        const auto* enemy=target.loading_state;
        if(!enemy)throw std::logic_error("Hostile scoring requires the enemy's actual B0");
        float score=100.f-distance;
        const auto& words=enemy->words();
        if(words.template get<std::uint32_t>(0x48)==1){
            for(unsigned axis=0;axis!=3;++axis)direction[axis]=-direction[axis];
            score+=dot(direction,enemy->target_direction())>.9f?50.f:30.f;
        }else if(enemy->aims_at_human(*human))score+=50.f;
        const auto mask=world.human_hostile_relation_mask(*human,target);
        if(mask&1u)score+=30.f;
        if(mask&0x10u)score+=10.f;
        if(mask&2u)score+=10.f;
        if(mask&4u)score+=5.f;
        if(mask&8u)score+=5.f;
        if(words.template get<std::int32_t>(0xb0)<400)score+=20.f;
        const auto posture=words.template get<std::uint32_t>(0x50)&3u;
        if(!posture)score+=10.f;
        else if(posture&2u)score+=5.f;
        if(facing>.95f)score+=5.f;
        const auto& friends=human->perception.lists()[1];
        for(auto m=friends.size();m;){
            auto& ally=actual(friends[--m]);
            if(&ally==human)continue;
            const auto* friendly=ally.loading_state;
            if(!friendly)throw std::logic_error("Hostile scoring requires the ally's actual B0");
            if(friendly->selected_hostile_is(target))score-=20.f;
        }
        if(score>best_score){
            best_score=score;
            best=&target;
        }
    }
    return best;
}

//00573240, one actual target; same native FLOAT square and signed bit length test.
template<class Human,class Actor,class Frames>long double human_ai_actor_distance(Human& human,Actor& target,Frames& frames){
    const auto first=frames.actor_world_matrix(human.base().frame()),second=frames.actor_world_matrix(target.base().frame());
    const auto x=static_cast<long double>(second[12])-first[12],y=static_cast<long double>(second[13])-first[13],z=static_cast<long double>(second[14])-first[14];
    const float square=static_cast<float>((z*z+y*y)+x*x);std::int32_t bits;std::memcpy(&bits,&square,4);
    return bits<0x322bcc77?0.L:std::abs(static_cast<long double>(square)-1)<9.99999993922529e-09f?1.L:std::sqrt(static_cast<long double>(square));
}
//005733E0: reverse actual hostile list, real model poses each iteration;
//extended differences, stored FLOAT square, SIGNED bit-pattern epsilon test.
template<class Human,class Frames>long double human_ai_closest_enemy_distance(Human& human,Frames& frames){
    float minimum=65535.f;const auto& enemies=human.perception.lists()[0];
    for(auto n=enemies.size();n;){
        auto* enemy=enemies[--n];if(!enemy)throw std::logic_error("AI distance requires actual hostile actor");
        const auto own=human.base().frame(),other=enemy->base().frame();
        if(!own||!other)throw std::runtime_error("AI distance requires actual model frames");
        const auto a=frames.actor_world_matrix(own),b=frames.actor_world_matrix(other);
        const auto x=static_cast<long double>(b[12])-a[12],y=static_cast<long double>(b[13])-a[13],z=static_cast<long double>(b[14])-a[14];
        const float square=static_cast<float>((z*z+y*y)+x*x);std::int32_t bits;std::memcpy(&bits,&square,4);
        const long double distance=bits<0x322bcc77?0.L:std::abs(static_cast<long double>(square)-1)<9.99999993922529e-09f?1.L:std::sqrt(static_cast<long double>(square));
        if(distance<minimum)minimum=static_cast<float>(distance);
    }
    return minimum;
}
//00584890 retains normalized X/Y extended until the final dot but spills Z.
//Its two normalization sums have different orders; no shared vector helper
//with earlier FLOAT stores is substituted here. Only native AL is consumed.
inline std::array<long double,3> human_ai_aim_metric_normalize(const HumanVector& input,bool displacement){
    std::array<long double,3> value{{input[0],input[1],input[2]}};
    const auto x=value[0]*value[0],y=value[1]*value[1],z=value[2]*value[2];
    const auto square=displacement?(z+x)+y:(z+y)+x;const float stored=static_cast<float>(square);
    constexpr float epsilon=9.99999993922529e-09f;
    if(!(std::abs(square-1)<epsilon)){
        if(!(stored<epsilon)){
            const auto scale=1/std::sqrt(static_cast<long double>(stored));value[0]*=scale;value[1]*=scale;
            value[2]=static_cast<float>(value[2]*scale);
        }else{const auto axis=value[0]!=0?0u:value[2]!=0?2u:1u;value[axis]=value[axis]<0?-1.L:1.L;}
    }
    return value;
}
template<class Observer,class State,class Target,class Frames>bool human_ai_aims_at(Observer& observer,State& state,Target& target,Frames& frames){
    const auto own=observer.base().frame(),other=target.base().frame();
    if(!own||!other)throw std::runtime_error("AI aim predicate requires actual model frames");
    const auto a=frames.actor_world_matrix(own),b=frames.actor_world_matrix(other);
    HumanVector delta{};for(unsigned axis=0;axis!=3;++axis)delta[axis]=static_cast<float>(static_cast<long double>(b[12+axis])-a[12+axis]);
    const auto aim=state.target_direction(); //Actual enemy B0 virtual4C, AFTER model queries.
    const auto d=human_ai_aim_metric_normalize(delta,true),v=human_ai_aim_metric_normalize(aim,false);
    return (v[2]*d[2]+v[0]*d[0])+v[1]*d[1]>.9848099946975708f; //008110C4, strict.
}
} //namespace hd2::game
#endif
