#ifndef HD2_GAME_HUMAN_ACTIVE_AI_AUTHORITY_BODY_H
#define HD2_GAME_HUMAN_ACTIVE_AI_AUTHORITY_BODY_H
#include "game/human_ai_frame_metrics.h"
#include "game/human_frame_stats.h"
#include "game/human_animation_requests.h"
#include "game/human_ai_cover.h"
#include "game/human_active_ai_cautious.h"
#include "game/human_active_ai_target_modes.h"
#include "game/human_active_ai_specialized_target.h"
#include <functional>
namespace hd2::game {
//00537FB0. Populated F4/action-mode changes use the SAME A4 record and
//already recovered00524A30 direction mask; no new posture/input owner.
template<class AI>void adjust_active_navigation_stance(AI& ai,bool ready){
    if(!ai.first_frame_inputs()||!ai.second_frame_inputs())return;
    ai.refresh_item();const auto* point=ai.point();if(!point||!ai.item())return;
    const auto mask=human_ai_cover::direction(ai.target_direction());
    const bool low=(point->reserved17[2]&mask)!=0;
    std::uint8_t blocked=low?1:0;if(point->reserved17[0]&mask)blocked=2;
    const auto flags=ai.words().template get<std::uint32_t>(0x50);
    if(!ready){
        if(point->reserved17[1]&mask)blocked|=4;
        if((blocked&6u)&&!(flags&2u)){ai.pending_posture(2);return;}
        if(low&&!(flags&1u))ai.pending_posture(1);
    }else{
        if((blocked&2u)&&!(flags&4u))ai.state().set(0x178,4u);
        if(low&&!(flags&2u))ai.pending_posture(2);
    }
}
//00526550, AFTER navigation/gait. The native switch tests individual return
//values, not a mask. In particular ordinary no-target8 falls through unchanged.
template<class AI,class World>void advance_active_aim_policy(AI& ai,std::uint32_t elapsed,bool following,World& world){
    const auto validate=world.human_common_receiver_guard(*ai.human());
    const auto result=ai.advance_frame_aim(elapsed,world);validate();auto& state=ai.state();
    switch(result){
    case 1:
        if(ai.item()){
            const auto deadline=world.human_frame_output_item(ai.item()).field5c;
            const auto remaining=deadline?human_frame_signed_word(deadline-world.game_word28()):0;
            auto posture=4u;
            if(following)if(auto* leader=ai.human()->formation_leader)posture=world.active_follow_state(*leader).words().template get<std::uint32_t>(0x50)&7u;
            if(remaining<400&&!(ai.words().template get<std::uint32_t>(0x50)&4u)&&!state.template get<std::uint32_t>(0x178)&&(!(state.template get<std::uint32_t>(0x174)&2u)||posture==4))ai.pending_posture(4);
        }
        break;
    case 2:world.queue_active_ai_speech(ai,0x70u,true,7000u,false);break;
    case 0x10:if(state.template get<std::int32_t>(0x1a0)<200)(void)ai.reselect_weapon();break;
    case 0x20:case 0x400:ai.pending_second_mode(12);break;
    case 0x200:
        if(!(ai.resources().query().state.template get<std::uint32_t>(0x50)&0x10000000u)&&!state.template get<std::uint32_t>(0x178)&&(ai.words().template get<std::uint32_t>(0x50)&7u)==4)ai.pending_posture(2);
        ai.pending_second_mode(13);break;
    case 0x800:{
        const bool moved=world.active_holster_spare_ammunition(*ai.human());validate();
        if(!moved&&state.template get<std::int32_t>(0x1a0)<200){(void)ai.reselect_weapon();validate();}
        ai.pending_second_mode(14);break;
    }
    default:break;
    }
    validate();
}
//0052F700, AL result only. The populated perception list shortens the next
//selection interval; DWORD underflow behavior is owned by0052B810's prefix.
template<class AI>bool active_weapon_selection_due(AI& ai){
    auto& state=ai.state();if(state.template get<std::uint32_t>(0x1a0))return false;
    state.set(0x1a0,ai.human()->perception.lists()[0].empty()?5000u:400u);return true;
}
//0052D0D0 ordinary selection, including its refresh BEFORE the no-item return.
template<class AI,class Frames,class World>auto select_active_movement_target(AI& ai,Frames& frames,World& world,const HumanVector* filter=nullptr){
    using Human=std::remove_reference_t<decltype(*ai.human())>;
    ai.refresh_item();
    using Actor=std::remove_reference_t<decltype(*ai.task_target)>;
    Actor* selected=ai.item()?select_human_hostile<Human>(ai,frames,world,filter):nullptr;
    return ai.item()?refine_active_task_target(ai,selected,frames,world):selected;
}
//0052A5D0. Original0083C150/170/190/1B0 are INTEGER tables. The two
//interpolations run in opposite directions and truncate BEFORE their .1 spans.
template<class AI,class Actor,class Frames,class World>void set_active_target_intervals(AI& ai,Actor* target,Frames& frames,World& world){
    std::int32_t fire=-1,pause=-1,fire_span=-1,pause_span=-1;
    if(target){ai.refresh_item();if(ai.item()){
        constexpr std::array<std::array<std::int32_t,8>,4> tables{{
            {{95,380,25,2500,480,1000,1,2500}},{{50,380,25,625,480,1000,1,900}},
            {{93,380,25,4900,480,2000,100,10000}},{{380,580,100,10000,1200,2000,1,10000}}
        }};
        const auto command=world.active_follow_item_command(ai.item());
        if(command>=1&&command<=4){const auto& t=tables.at(command-1);
            const auto a=frames.actor_world_matrix(target->base().frame()),b=frames.actor_world_matrix(ai.human()->base().frame());
            const auto x=static_cast<long double>(b[12])-a[12],y=static_cast<long double>(b[13])-a[13],z=static_cast<long double>(b[14])-a[14];
            const float square=static_cast<float>((z*z+x*x)+y*y);
            if(square<static_cast<float>(t[2]))fire=t[1];
            else if(static_cast<long double>(t[3])<square)fire=t[0];
            else fire=human_animation_time_integer((1.L-(static_cast<long double>(square)-static_cast<float>(t[2]))/(t[3]-t[2]))*(t[1]-t[0])+t[0]);
            fire_span=human_animation_time_integer(static_cast<long double>(fire)*.1f);
            if(square<static_cast<float>(t[6]))pause=t[4];
            else if(static_cast<long double>(t[7])<square)pause=t[5];
            else pause=human_animation_time_integer(((static_cast<long double>(square)-static_cast<float>(t[6]))/(t[7]-t[6]))*(t[5]-t[4])+t[4]);
            pause_span=human_animation_time_integer(static_cast<long double>(pause)*.1f);
        }
    }}
    auto& words=ai.resources().target().state;words.set(0x3c,fire);words.set(0x40,fire_span);words.set(0x44,pause);words.set(0x48,pause_span);
}
inline std::array<std::uint32_t,4> active_burst_profile(std::uint32_t kind){ //0052B080, min/span pairs.
    if(kind==3||kind==12)return {{1000,1000,1000,1500}};
    if(kind==4)return {{2500,500,1500,500}};
    if(kind==5)return {{4500,2000,2000,2000}};
    return {{1500,1000,1500,1000}};
}
//0052AC20. Same model/head, inline sector and actual scene collision tree.
//004D0430 at the blocked-ray tail is a genuine RET16, not a debug service.
template<class AI,class Frames,class World>bool active_target_crouch_clear(AI& ai,Frames& frames,World& world){
    auto* target=ai.resources().target().actor;if(!target)return false;
    const auto a=frames.actor_world_matrix(ai.human()->base().frame()),b=frames.actor_world_matrix(target->base().frame());
    HumanVector origin{{a[12],a[13],a[14],a[15]}},delta{};
    for(unsigned n=0;n!=3;++n)delta[n]=static_cast<float>(static_cast<long double>(b[12+n])-origin[n]);
    const float square=static_cast<float>((static_cast<long double>(delta[2])*delta[2]+static_cast<long double>(delta[0])*delta[0])+static_cast<long double>(delta[1])*delta[1]);
    std::uint32_t bits;std::memcpy(&bits,&square,4);
    const auto length=human_frame_signed_word(bits)<0x322bcc77?0.L:std::abs(static_cast<long double>(square)-1)<9.99999993922529e-9f?1.L:std::sqrt(static_cast<long double>(square));
    if(length<0)return false;
    if(std::abs(static_cast<long double>(delta[1])/length)>.17364799976348877)return false;
    const auto kind=world.active_follow_item_kind(ai.item());if(kind==0||kind==6||kind==7||kind==9)return false;
    if(world.active_follow_sector(*ai.human())!=world.active_follow_root_sector())return false;
    delta[1]=0;const float horizontal=static_cast<float>(static_cast<long double>(delta[0])*delta[0]+static_cast<long double>(delta[2])*delta[2]);
    if(!(std::abs(static_cast<long double>(horizontal)-1)<9.99999993922529e-9f)){
        if(!(horizontal<9.99999993922529e-9f)){const auto inverse=1.L/std::sqrt(static_cast<long double>(horizontal));for(unsigned n=0;n!=3;++n)delta[n]=static_cast<float>(static_cast<long double>(delta[n])*inverse);}
        else if(delta[0]!=0)delta[0]=delta[0]<0?-1.f:1.f;
        else if(delta[2]!=0)delta[2]=delta[2]<0?-1.f:1.f;
        else delta[1]=1.f;
    }
    origin[0]=static_cast<float>(static_cast<long double>(delta[0])*.5f+origin[0]);
    origin[1]=static_cast<float>((static_cast<long double>(origin[1])+.75f)+static_cast<long double>(delta[1])*.5f);
    origin[2]=static_cast<float>(static_cast<long double>(delta[2])*.5f+origin[2]);
    auto point=target->base().type()==3?world.human_ai_target_human(*target).head_marker:target->base().frame();
    if(!point)throw std::runtime_error("Active crouch target requires its actual head/model frame");
    const auto pose=frames.actor_world_matrix(point);HumanVector end{{pose[12],pose[13],pose[14],pose[15]}};
    if(target->base().type()!=3)end[1]=static_cast<float>(static_cast<long double>(end[1])+.5f);
    for(unsigned n=0;n!=3;++n)delta[n]=static_cast<float>(static_cast<long double>(end[n])-origin[n]);
    return !world.human_geometry_ray(origin,delta,3u,0u);
}
template<class AI>bool active_movement_has_aim(AI& ai){
    const auto& target=ai.resources().target();const auto v=target.state.template get<std::array<float,3>>(0x10);
    return target.actor||((static_cast<long double>(v[2])*v[2]+static_cast<long double>(v[1])*v[1])+static_cast<long double>(v[0])*v[0]>=1.e-16f);
}
//0052B810 burst timer/query58 and ordinary posture branch; engine RandFloat
//and CRT rand are different owners and are sampled only at their native gates.
template<class AI,class Frames,class World>void advance_active_burst_policy(AI& ai,std::uint32_t elapsed,bool following,Frames& frames,World& world){
    auto& state=ai.state();auto& query=ai.resources().query().state;
    const auto kind=world.active_follow_item_kind(ai.item());
    if(active_movement_has_aim(ai)){
        auto timer=state.template get<std::uint32_t>(0x1a8);
        if((query.template get<std::uint32_t>(0x50)&0x10000000u)&&!query.template get<std::uint8_t>(0x58)){
            if(human_frame_signed_word(timer)<0)state.set(0x1a8,1u);
            else{
                timer-=elapsed;state.set(0x1a8,timer);
                if(human_frame_signed_word(timer)<1&&ai.item()){
                    const auto current=world.active_follow_item_kind(ai.item());
                    if(human_frame_signed_word(current)<0||current>1){
                        query.set(0x58,std::uint8_t{1});const auto profile=active_burst_profile(world.active_follow_item_kind(ai.item()));
                        state.set(0x1a8,0u-(world.human_frame_random_word()%profile[1]+profile[0]));
                    }else state.set(0x1a8,0xffffffffu);
                }
            }
        }else if(human_frame_signed_word(timer)>0)state.set(0x1a8,0u);
        else{
            timer+=elapsed;state.set(0x1a8,timer);
            if(human_frame_signed_word(timer)>=0){
                query.set(0x58,std::uint8_t{0});const auto profile=active_burst_profile(world.active_follow_item_kind(ai.item()));
                state.set(0x1a8,world.human_frame_random_word()%profile[3]+profile[2]);
                const auto current=world.active_follow_item_kind(ai.item());if(human_frame_signed_word(current)>=0&&current<2)state.set(0x1a8,0u);
                const auto posture=state.template get<std::uint32_t>(0x178);
                if(!following&&(query.template get<std::uint32_t>(0x50)&0x10000000u)&&posture&&(ai.words().template get<std::uint32_t>(0x50)&7u)!=posture)ai.pending_posture(posture);
            }
            if((ai.words().template get<std::uint32_t>(0x50)&7u)>=3&&ai.point()&&kind!=5&&active_target_crouch_clear(ai,frames,world))ai.pending_posture(2);
        }
    }else{
        if(query.template get<std::uint8_t>(0x58)){
            state.set(0x1a8,static_cast<std::uint32_t>(human_animation_time_integer((static_cast<long double>(world.human_sound_random())+1.f)*1000.f)));
            query.set(0x58,std::uint8_t{0});
        }
        const auto posture=state.template get<std::uint32_t>(0x178),task=state.template get<std::uint32_t>(0x220);
        if(!following&&posture&&(ai.words().template get<std::uint32_t>(0x50)&7u)!=posture&&
           (state.template get<std::uint8_t>(0x202)||task!=6||!ai.task_target||ai.task_target->base().type()!=3))ai.pending_posture(posture);
    }
    if(kind==5&&active_movement_has_aim(ai)){
        const bool admitted=state.template get<std::uint8_t>(0x18b)||active_launcher_target_clear(ai,frames,world);
        if(((query.template get<std::uint32_t>(0x50)&0x10000000u)&&!query.template get<std::uint8_t>(0x58))||!admitted)ai.resources().target().bind_actor(nullptr);
        else{
            if((ai.words().template get<std::uint32_t>(0x50)&7u)!=1u){ai.resources().target().bind_actor(nullptr);ai.pending_posture(1);}
            state.set(0x18b,std::uint8_t{1});
        }
    }else state.set(0x18b,std::uint8_t{0});
}
//0052B810 ordinary on-foot authority path. Specialized branches stop at their
//native selection point, after earlier writes, rather than supplying idle work.
template<class AI,class Frames,class World>void advance_active_movement_policy(AI& ai,std::uint32_t elapsed,bool following,Frames& frames,World& world){
    auto* human=ai.human();const auto validate=world.human_common_receiver_guard(*human);
    auto* selected=world.active_process_target(); //00456860, actual process98 BEFORE timers.
    auto& state=ai.state();const auto decrement=[&](unsigned offset){
        const auto old=state.template get<std::uint32_t>(offset);if(!old)return;
        const auto next=old-elapsed;state.set(offset,next);if(human_frame_signed_word(next)<0)state.set(offset,0u);
    };
    decrement(0x198);decrement(0x1a0);decrement(0x19c);
    if(!state.template get<std::uint32_t>(0x19c)&&ai.words().template get<std::uint8_t>(0xac)){ai.pending_clear_item_ready();validate();}
    auto* threat=world.active_nearby_dog(ai,7.f);validate();
    if(threat)throw std::runtime_error("Active movement requires0052B580 Dog-target policy");
    const auto task=state.template get<std::uint32_t>(0x220);bool specialized=false;
    if(!state.template get<std::uint8_t>(0x202)&&task==22)throw std::runtime_error("Active movement requires0052B810 specialized task22 policy");
    const auto flags=state.template get<std::uint32_t>(0x174);
    if(flags&0x10u){
        const auto own=human->base().world_position();
        if(!state.template get<std::uint32_t>(0x194)){ai.saved_frame_position(false,own);validate();}
        else{
            const auto direction=active_saved_position_direction(state.template get<HumanVector>(0x140),own);
            auto* target=select_active_movement_target(ai,frames,world,&direction);validate();
            if(target)ai.resources().target().bind_actor(target);else ai.resources().target().bind_vector(direction);
        }
    }else{
    const bool admit=!(flags&2u)||active_cautious_attacker(ai,true,world);validate();
    if(admit||selected){
    if(!selected&&!(flags&4u)){selected=select_active_movement_target(ai,frames,world);validate();}
    specialized=!state.template get<std::uint8_t>(0x202)&&(task==25||task==26);
    if(specialized){
        ai.require_live_task_target();if(!ai.task_target)throw std::runtime_error("Active target completion dereferences native NULL224");
        if(world.active_task_actor_unavailable(*ai.task_target)){state.set(0x200,std::uint8_t{1});state.set(0x206,std::uint8_t{1});}validate();
    }
    if(!state.template get<std::uint8_t>(0x202)&&task==41)selected=nullptr;
    if(selected&&!specialized&&ai.has_pending_orders_loading()){
        const auto own_flags=ai.words().template get<std::uint32_t>(0x50);
        if(own_flags&0x400u)selected=nullptr;
        if((own_flags&0x200u)&&!active_cautious_attacker(ai,true,world))selected=nullptr;
    }
    if(!specialized&&(following||!ai.has_pending_orders_loading())&&(ai.active_event_mode()==0||ai.active_event_mode()==2))selected=nullptr;
    if(state.template get<std::uint8_t>(0x18a)&&ai.item()&&!world.active_special_hand(ai.item()))selected=nullptr;
    if(flags&0x200u){
        const auto model=frames.actor_world_matrix(human->base().frame());validate();const auto position=state.template get<HumanVector>(0x1e0);
        auto value=ai.resources().target().state.template get<HumanVector>(0x10); //Native temporary W unwritten; retained typed lane has no native claim.
        for(unsigned n=0;n!=3;++n)value[n]=static_cast<float>(static_cast<long double>(position[n])-model[12+n]);
        ai.resources().target().bind_vector(value);
    }else ai.resources().target().bind_actor(selected);
    set_active_target_intervals(ai,selected,frames,world);validate();
    }else ai.resources().target().bind_actor(nullptr); //Native cautious NULL branch does NOT reset range intervals.
    if(!selected&&!state.template get<std::uint32_t>(0x198)){
        if(!state.template get<std::uint32_t>(0x1ac)){
            if(!(flags&4u)){world.active_idle_look(ai);validate();}
            state.set(0x23c,0u); //The bit4 path also publishes the native NULL result.
            state.set(0x1ac,1000u);
        }else decrement(0x1ac);
        if((flags&2u)&&!state.template get<std::uint32_t>(0x19c)){
            HumanVector direction{};std::uint32_t posture=0;select_active_cautious_posture(ai,direction,posture,world);validate();
            if(posture==1)posture=2;
            auto* leader=human->formation_leader;
            if(!leader||(posture<=(world.active_follow_state(*leader).words().template get<std::uint32_t>(0x50)&7u)&&posture!=(ai.words().template get<std::uint32_t>(0x50)&7u)))ai.pending_posture(posture);
            direction[1]=0;ai.resources().direction().set_target(direction);
        }
    }
    }
    //Fresh target after the preceding publications, as in the native tail.
    auto& target=ai.resources().target();
    if(target.actor){auto& other=world.human_ai_target_human(*target.actor);if(((other.properties.field19c==2&&other.loading_state&&other.loading_state->suppress_hostile_selection())||world.human_unavailable(other))&&(!specialized||target.actor!=ai.task_target))target.bind_actor(nullptr);}
    if(!active_movement_has_aim(ai)){
        threat=world.active_nearby_dog(ai,30.f);validate();
        if(threat&&threat->base().type()==30)throw std::runtime_error("Active movement requires0052B580 distant Dog-target policy");
    }
    if(target.actor&&world.active_armoured_target(*target.actor)&&ai.item()&&world.active_follow_item_kind(ai.item())!=6)target.bind_actor(nullptr);
    if(active_weapon_selection_due(ai)){(void)ai.reselect_weapon();validate();}
    auto* previous=target.actor;std::function<void()> previous_live;if(previous)previous_live=world.active_event_actor_guard(*previous);
    if(ai.item()){
        advance_active_burst_policy(ai,elapsed,following,frames,world);validate();
    }
    if(!ai.item())state.set(0x18b,std::uint8_t{0});
    const auto& query_state=ai.resources().query().state;
    if((query_state.template get<std::uint32_t>(0x50)&0x10000000u)&&!query_state.template get<std::uint8_t>(0x58))state.set(0x18b,std::uint8_t{0});
    if(ai.item()&&world.active_follow_item_command(ai.item())==3){
        auto& query=ai.resources().query().state;const auto movement=query.template get<std::uint32_t>(0x54)&0x1f000u;
        if((query.template get<std::uint32_t>(0x50)&0x10000000u)&&!query.template get<std::uint8_t>(0x58)&&movement>0x1000u&&movement!=0x8000u){
            target.bind_actor(nullptr);if(ai.words().template get<std::uint8_t>(0xac))ai.pending_clear_item_ready();validate();
        }
    }
    if(previous&&!target.actor){
        previous_live();const auto a=human->base().world_position(),b=previous->base().world_position();
        ai.resources().direction().set_target({{static_cast<float>(static_cast<long double>(b[0])-a[0]),0.f,static_cast<float>(static_cast<long double>(b[2])-a[2]),0.f}});
    }
    if(ai.item()){
        auto& item=world.human_frame_output_item(ai.item());
        if(item.capacity()!=-1.f&&item.amount()==0.f&&!world.active_has_ammunition(*human,item.definition().ammunition()))target.bind_actor(nullptr);
    }
    if(ai.item()&&target.actor&&world.active_follow_item_kind(ai.item())<2){
        const auto a=human->base().world_position(),b=target.actor->base().world_position();
        const auto x=static_cast<long double>(b[0])-a[0],y=static_cast<long double>(b[1])-a[1],z=static_cast<long double>(b[2])-a[2];
        const float square=static_cast<float>((x*x+y*y)+z*z);std::uint32_t bits;std::memcpy(&bits,&square,4);
        if(human_frame_signed_word(bits)<0x322bcc77||std::abs(static_cast<long double>(square)-1)<9.99999993922529e-9f||std::sqrt(static_cast<long double>(square))<1.2000000476837158f)
            {advance_active_close_target(ai,*target.actor,frames,world);validate();}
    }
    if(active_movement_has_aim(ai))state.set(0x19c,5000u);
}
//0045F3A0. Reverse roster/perception traversal and CRT selection use the
//actual borrowers. An empty candidate vector genuinely returns NULL.
template<class Human,class Frames,class World>auto* select_active_shared_enemy(Human& human,bool exclude_armour,Frames& frames,World& world){
    using Actor=std::remove_reference_t<decltype(*human.perception.lists()[0].front())>;
    std::vector<Actor*> candidates;
    auto& roster=world.human_roster();
    for(auto n=roster.size();n;){
        auto* other=roster.at(static_cast<std::int32_t>(--n));
        if(!other)throw std::runtime_error("Active shared-enemy lookup lost roster Human");
        if(other==&human||human_frame_signed_word(other->properties.stats_ec[0])<=0)continue;
        const auto a=frames.actor_world_matrix(human.base().frame()),b=frames.actor_world_matrix(other->base().frame());
        const auto x=static_cast<long double>(b[12])-a[12],y=static_cast<long double>(b[13])-a[13],z=static_cast<long double>(b[14])-a[14];
        if(!(((x*x+y*y)+z*z)<=1600.f))continue;
        const auto& enemies=other->perception.lists()[0];
        for(auto m=enemies.size();m;){
            auto* enemy=enemies[--m];if(!enemy)throw std::runtime_error("Active shared-enemy lookup reached native NULL target");
            if(exclude_armour&&world.active_armoured_target(*enemy))continue;
            if(!world.active_event_hostile(human,enemy))candidates.push_back(enemy);
        }
    }
    return candidates.empty()?nullptr:candidates.at(world.human_frame_random_word()%candidates.size());
}
//00527560: each priority restarts the scratch vector. Indices refer to the
//SAME processed deque; armoured exclusions resolve the actual scene handle.
template<class AI,class World>std::vector<std::size_t> active_cover_event_candidates(AI& ai,std::uint32_t kind,bool exclude_armour,World& world){
    std::vector<std::size_t> result;auto& events=ai.command()->second.events;
    for(auto n=events.size();n;){const auto index=--n;const auto* event=events.at(index);
        if(!event)throw std::logic_error("Active processed-event slot is empty");
        if(event->words[0]!=kind)continue;
        if(exclude_armour&&human_frame_signed_word(event->words[4])>=0){auto* actor=world.human_ai_resolve_actor(event->words[4]);if(actor&&world.active_armoured_target(*actor))continue;}
        result.push_back(index);
    }
    return result;
}
//005276E0 ordinary on-foot event/cover decision, including priority indices,
//native reverse selection-sort and selected-record return BEFORE compaction.
template<class AI,class Frames,class World>bool advance_active_cover_decision(AI& ai,Frames& frames,World& world){
    auto& state=ai.state();if(state.template get<std::uint32_t>(0x198)||(ai.has_pending_orders_loading()&&(ai.words().template get<std::uint32_t>(0x50)&0x400u)))return false;
    auto* human=ai.human();const auto validate=world.human_common_receiver_guard(*human);
    const bool exclude_armour=!world.active_has_heavy_weapon(*human);validate(); //005328D0 is false for this unattached path.
    auto* command=ai.command();if(!command)throw std::logic_error("Active cover decision lost actual commandB4");
    auto& events=command->second.events;
    std::vector<std::size_t> candidates;
    for(const auto kind:{0x40000015u,0x40000011u,0x40000010u,0x3000000bu,0x3000000cu}){
        candidates=active_cover_event_candidates(ai,kind,exclude_armour,world);validate();if(!candidates.empty())break;
    }
    const auto resolve=[&](std::size_t index){const auto* event=events.at(index);if(!event)throw std::logic_error("Active cover selection lost its processed event");auto* actor=world.human_ai_resolve_actor(event->words[4]);validate();return actor;};
    const auto distance=[&](auto& actor){
        const auto a=actor.base().world_position(),b=human->base().world_position();
        const auto x=static_cast<long double>(b[0])-a[0],y=static_cast<long double>(b[1])-a[1],z=static_cast<long double>(b[2])-a[2];
        return static_cast<float>((z*z+y*y)+x*x);
    };
    for(auto n=candidates.size();n;){const auto index=--n;if(auto* actor=resolve(candidates[index])){
        auto square=distance(*actor);if(!index)break;
        for(auto j=index;j;){--j;if(auto* other=resolve(candidates[j])){
            const auto other_square=distance(*other);if(other_square<square){std::swap(candidates[j],candidates[index]);square=other_square;}
        }}
    }}
    auto head=decltype(human->head_marker){};
    for(const auto index:candidates){
        if(auto* actor=resolve(index)){
            auto& target=world.human_ai_target_human(*actor);
            if(world.human_unavailable(target)||(target.properties.field19c==2&&target.loading_state&&target.loading_state->suppress_hostile_selection()))continue;
            const auto* event=events.at(index);if(!event)throw std::logic_error("Active cover retirement lost its selected record");
            if(event->words[0]!=0x10000003u){
                events.retire_at(index);head=target.head_marker;break;
            }
        }
    }
    if(!head){auto* target=select_active_shared_enemy(*human,exclude_armour,frames,world);validate();if(target)head=world.human_ai_target_human(*target).head_marker;}
    if(!head){
        using Actor=std::remove_reference_t<decltype(*human->perception.lists()[0].front())>;
        std::vector<Actor*> candidates;const auto& enemies=human->perception.lists()[0];
        for(auto n=enemies.size();n;){auto* enemy=enemies[--n];if(!enemy)throw std::runtime_error("Active cover selection reached native NULL enemy");if(!exclude_armour||!world.active_armoured_target(*enemy))candidates.push_back(enemy);}
        if(!candidates.empty()){auto* enemy=candidates.at(world.human_frame_random_word()%candidates.size());head=world.human_ai_target_human(*enemy).head_marker;}
        if(!head)return false;
    }
    auto& direction=ai.resources().direction();
    if((ai.resources().query().state.template get<std::uint32_t>(0x50)&0x10000000u)&&ai.resources().flag().state.template get<std::uint32_t>(0)==1){
        direction.target_frame(head);state.set(0x198,static_cast<std::uint32_t>(human_animation_time_integer(static_cast<long double>(world.human_sound_random())*1000.f+2500.f)));return true;
    }
    const auto position=human->base().world_position();const auto pose=frames.actor_world_matrix(head);validate();
    HumanVector delta{{static_cast<float>(static_cast<long double>(pose[12])-position[0]),0.f,static_cast<float>(static_cast<long double>(pose[14])-position[2]),0.f}};
    const float square=static_cast<float>(static_cast<long double>(delta[0])*delta[0]+static_cast<long double>(delta[2])*delta[2]);
    if(!(std::abs(static_cast<long double>(square)-1.f)<9.99999993922529e-9f)){
        if(!(square<9.99999993922529e-9f)){const auto inverse=1.L/std::sqrt(static_cast<long double>(square));for(unsigned n=0;n!=3;++n)delta[n]=static_cast<float>(static_cast<long double>(delta[n])*inverse);}
        else if(delta[0]!=0)delta[0]=delta[0]<0?-1.f:1.f;
        else if(delta[2]!=0)delta[2]=delta[2]<0?-1.f:1.f;
        else delta[1]=1.f;
    }
    direction.set_current(delta);direction.set_target(delta);
    state.set(0x198,static_cast<std::uint32_t>(human_animation_time_integer(static_cast<long double>(world.human_sound_random())*1000.f+3000.f)));return true;
}
//00526550 through its actual return, after navigation and movement policy.
template<class AI,class Inline,class World,class Runtime>void finish_active_onfoot_authority(AI& ai,std::uint32_t elapsed,Inline& control,World& world,Runtime& runtime){
    auto* human=ai.human();const auto validate=world.human_common_receiver_guard(*human);
    //005335E0 is RET4. Reuse the actual00583C30 and00588710 owners.
    ai.apply_frame_movement(control);validate();advance_human_endurance(*human,ai,control,elapsed);validate();
    world.active_frame_reload_policy(ai,control);validate();
    const bool checked=advance_active_cover_decision(ai,world.human_command_frames(),world);validate();
    if(!(ai.words().template get<std::uint32_t>(0x50)&0x400000u)&&!checked){
        const auto v=ai.resources().target().state.template get<std::array<float,3>>(0x10);
        const auto square=(static_cast<long double>(v[2])*v[2]+static_cast<long double>(v[1])*v[1])+static_cast<long double>(v[0])*v[0];
        const bool busy=ai.resources().target().actor||square>=1.e-16f||!human->perception.lists()[0].empty();
        ai.resources().direction().look_interval(!busy,4000u);ai.look_command(busy?0u:4u,control);validate();
    }
    ai.apply_frame_items(control,world,runtime,elapsed,0u);validate();
    ai.select_frame_direction_preset();validate();ai.advance_frame_direction(elapsed,control);validate();
    auto* command=ai.command();if(!command)throw std::logic_error("Active authority tail lost actual processed-event owner");
    auto& events=command->second.events;
    for(auto n=events.size();n;){const auto index=--n;const auto* event=events.at(index);
        if(!event)throw std::logic_error("Active authority tail reached empty processed-event slot");
        if(event->words[0]!=0x10000003u)continue;
        if(human_frame_signed_word(event->words[2])>750){events.retire_at(index);world.queue_active_ai_speech(ai,0x6au,true,300u,false);validate();}
        break; //Only the last matching event is considered, even when younger.
    }
    ai.advance_frame_self_treatment();validate();
}
} //namespace hd2::game
#endif
