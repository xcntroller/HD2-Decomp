#ifndef HD2_GAME_SHOT_COMBAT_FRAMES_H
#define HD2_GAME_SHOT_COMBAT_FRAMES_H
#include "game/shot_hit_sort.h"
#include "game/shot_visual_frames.h"
namespace hd2::game::shot_combat {
using shot_motion::stored;using shot_visual::square_zyx;
inline float length(long double square){const float value=stored(square);std::int32_t bits;std::memcpy(&bits,&value,4);
    return bits<0x322bcc77?0.f:std::abs(static_cast<long double>(value)-1.f)<9.99999993922529e-09f?1.f:stored(std::sqrt(static_cast<long double>(value)));}
inline float distance(const HumanVector&a,const HumanVector&b){const auto x=static_cast<long double>(a[0])-b[0],y=static_cast<long double>(a[1])-b[1],z=static_cast<long double>(a[2])-b[2];return length((x*x+y*y)+z*z);}
inline HumanVector displacement(const HumanVector& velocity,std::uint32_t elapsed){HumanVector result{};for(unsigned a=0;a!=3;++a)result[a]=stored(static_cast<long double>(stored(static_cast<long double>(human_frame_signed_word(elapsed))*velocity[a]))*.0010000000474974513f);return result;}
inline HumanVector add(const HumanVector&a,const HumanVector&b){auto result=a;for(unsigned n=0;n!=3;++n)result[n]=stored(static_cast<long double>(a[n])+b[n]);return result;}
inline long double dot(const HumanVector&a,const HumanVector&b){return (static_cast<long double>(a[2])*b[2]+static_cast<long double>(a[1])*b[1])+static_cast<long double>(a[0])*b[0];}
template<class Record>float power_at(Record& record,float distance){ //0044B3B0; native38/3C.
    const auto power=record.words.template get<float>(0x58),range=record.payload_real(0x34);
    if(distance<=range)return power;
    const auto loss=(static_cast<long double>(distance)-range)*record.payload_real(0x38);
    return power<loss?0.f:stored(static_cast<long double>(power)-loss);
}
inline void ricochet(HumanVector& velocity,const HumanVector& normal,float amount){ //00448380: retain original speed, both normalizations, FLOAT dot*amount spill.
    const auto speed=length(square_zyx(velocity));normalize_human_ai_shot(velocity,square_zyx(velocity));
    const float projection=stored(((static_cast<long double>(velocity[1])*normal[1]+static_cast<long double>(velocity[2])*normal[2])+static_cast<long double>(velocity[0])*normal[0])*amount);
    for(unsigned a=0;a!=3;++a)velocity[a]=stored(static_cast<long double>(velocity[a])-static_cast<long double>(normal[a])*projection);
    normalize_human_ai_shot(velocity,square_zyx(velocity));for(unsigned a=0;a!=3;++a)velocity[a]=stored(static_cast<long double>(velocity[a])*speed);
}
template<class Hit,class Pick>void refine(Hit& hit,const Pick& pick,const HumanVector& origin,const HumanVector& direction){
    for(unsigned a=0;a!=3;++a){const auto product=static_cast<long double>(direction[a])*pick.distance;hit.position[a]=stored((a==2?stored(product):product)+origin[a]);}
    hit.normal=pick.normal;
}
template<class Record>void publish(Record& record,const HumanVector& end){
    record.words.set(0x20,end);const auto previous=record.words.template get<HumanVector>(0x30);HumanVector delta{};
    for(unsigned a=0;a!=3;++a){delta[a]=stored(static_cast<long double>(end[a])-previous[a]);}
    record.words.set(0x40,delta);
}
//0044B910. Ray exclusion ends BEFORE inspecting any actor or invoking damage.
//Contact power is computed before a detailed geometry pick refines the point.
template<class Record,class World>unsigned bullet(Record& record,std::uint32_t elapsed,World& world){
    auto& w=record.words;if(w.template get<std::uint8_t>(0x78))return 1;
    if(w.template get<std::uint8_t>(0xb0)&&w.template get<float>(0x14)!=0)elapsed=human_packet_ftol(static_cast<long double>(human_frame_signed_word(elapsed))*.5f);
    w.set(0x50,0u);const auto origin=w.template get<HumanVector>(0x20);w.set(0x30,origin);
    const auto travel=displacement(w.template get<HumanVector>(0x10),elapsed);auto direction=w.template get<HumanVector>(0x10);normalize_human_ai_shot(direction,square_zyx(direction));
    auto hits=world.shot_travel_hits(record,origin,travel,0x70);shot_hit_sort::sort(hits);auto end=add(origin,travel);
    using ActorPointer=decltype(record.source);ActorPointer previous=nullptr;
    for(auto& hit:hits){
        const float power=power_at(record,stored(static_cast<long double>(distance(hit.position,w.template get<HumanVector>(0x20)))+w.template get<float>(0x70)));
        const auto flags=world.human_geometry_flags(hit);auto frame=world.shot_hit_frame(hit);decltype(frame) picked{};std::uint32_t material=0xffffffffu;ActorPointer actor=nullptr;bool material_ready=false;
        if(!(flags&0x40u)){
            material=flags>>24;
            if((flags&0xc0u)&&frame&&world.shot_static_frame_in_scene(frame)){
                const auto detail=world.shot_detailed_pick(w.template get<HumanVector>(0x20),travel,frame);if(!detail)continue;
                picked=detail->frame;refine(hit,*detail,w.template get<HumanVector>(0x20),direction);if(world.mode_flags()&1u)world.shot_script_hit(picked);
            }
            if(material==77)continue;
            if(material==76){w.set(0x78,std::uint8_t{1});end=hit.position;break;}
            material_ready=true;
        }else{
            actor=world.item_melee_actor(hit);if(previous&&previous==actor)continue;previous=actor;if(actor==record.source)continue;
            if(!actor){
                const auto detail=world.shot_detailed_pick(w.template get<HumanVector>(0x20),travel,frame);if(!detail)continue;
                picked=detail->frame;refine(hit,*detail,w.template get<HumanVector>(0x20),direction);
                const auto part=world.shot_version_material(picked);
                if(part){
                    material=static_cast<std::uint32_t>(*part);
                    if(world.mode_flags()&1u){const auto damage=static_cast<long double>(power)-world.shot_material_absorption(material);
                        if(!world.shot_version_damage(picked,human_frame_signed_word(human_packet_ftol(damage))))picked=decltype(frame){};}
                }else{
                    material=world.shot_picked_material(picked);
                    if(world.shot_hit_has_physics(frame)){if(material==0xffffffffu)material=world.shot_picked_material(frame);world.shot_physics_impulse(frame,w.template get<HumanVector>(0x10),hit.position);}
                    if((world.mode_flags()&1u)&&picked)world.shot_script_hit(picked);
                }
                material_ready=true;
            }else if(!w.template get<std::uint8_t>(0x6c)){
                const auto kind=actor->base().type();
                if(kind==3){
                    if(world.shot_friendly_human(*actor,record.source))continue;
                    const auto zone=world.shot_human_zone(*actor,w.template get<HumanVector>(0x20),travel);
                    if(zone==0xff){if((world.mode_flags()&1u)&&!w.template get<std::uint8_t>(0x6c))world.shot_human_miss(*actor,record.source,w.template get<HumanVector>(0x10),true);continue;}
                    material=zone==11?12u:static_cast<std::uint32_t>(world.shot_contact_material(*actor,decltype(frame){}));
                    if(world.mode_flags()&1u){const auto absorption=world.shot_material_absorption(material);if(static_cast<long double>(absorption)<power)world.shot_human_damage(*actor,stored(static_cast<long double>(power)-absorption),hit.position,direction,record.source,zone);}
                    material_ready=true;
                }else if(kind==21){
                    if(world.mode_flags()&1u)world.shot_actor_damage(*actor,1,power,w.template get<HumanVector>(0x20),travel,record.source,decltype(frame){});
                    const auto detail=world.shot_detailed_pick(w.template get<HumanVector>(0x20),travel,actor->base().frame());if(!detail)continue;
                    picked=detail->frame;refine(hit,*detail,w.template get<HumanVector>(0x20),direction);material=static_cast<std::uint32_t>(world.shot_contact_material(*actor,picked));material_ready=true;
                }else if(kind==4){
                    if(world.shot_source_attached_to(record.source,*actor))continue;
                    if(world.mode_flags()&1u)world.shot_actor_damage(*actor,1,power,hit.position,direction,record.source,decltype(frame){});
                    material=world.shot_car_material(*actor,w.template get<HumanVector>(0x20),direction);
                    if(material==0xffffffffu)continue;
                    if(world.mode_flags()&1u){world.shot_script_hit(actor->base().frame());}
                    material_ready=true;
                }else{
                    const auto detail=world.shot_detailed_pick(w.template get<HumanVector>(0x20),travel,actor->base().frame());if(!detail)continue;
                    picked=detail->frame;refine(hit,*detail,w.template get<HumanVector>(0x20),direction);material=static_cast<std::uint32_t>(world.shot_contact_material(*actor,picked));
                    if(world.mode_flags()&1u)world.shot_script_hit(actor->base().frame());
                    if(world.mode_flags()&1u){const auto absorption=world.shot_material_absorption(material);if(static_cast<long double>(absorption)<power)world.shot_actor_damage(*actor,1,stored(static_cast<long double>(power)-absorption),hit.position,direction,record.source,picked);}
                    material_ready=true;
                }
            }
        }
        if(!material_ready)continue;
        if(material==0xffffffffu){world.shot_missing_material(picked);w.set(0x78,std::uint8_t{1});end=hit.position;break;}
        if((world.mode_flags()&2u)&&(!actor||actor->base().type()!=3)){
            world.shot_impact_sound(record.source,material,hit.position,w.template get<std::uint8_t>(0x6c)!=0);
            world.shot_bullet_impact_mark(material,hit.position,hit.normal,actor&&actor->base().type()==11?decltype(frame){}:picked);
        }
        const auto absorption=world.shot_material_absorption(material);
        if(static_cast<long double>(absorption)<=power){const auto threshold=world.shot_material_threshold(material);
            if(threshold!=0xffffffffu&&static_cast<long double>(threshold)<=power){w.set(0x58,stored(static_cast<long double>(w.template get<float>(0x58))-threshold));continue;}
            w.set(0x78,std::uint8_t{1});end=hit.position;break;
        }
        if(w.template get<std::uint8_t>(0x6d)||dot(direction,hit.normal)<-.949999988079071f){w.set(0x78,std::uint8_t{1});end=hit.position;break;}
        w.set(0x6d,std::uint8_t{1});auto velocity=w.template get<HumanVector>(0x10);
        const float amount=stored(static_cast<long double>(world.shot_engine_random())*.10000000149011612f+1.100000023841858f);ricochet(velocity,hit.normal,amount);
        for(unsigned a=0;a!=3;++a){velocity[a]=stored(static_cast<long double>(velocity[a])*.5f);}
        w.set(0x10,velocity);w.set(0x58,stored(static_cast<long double>(w.template get<float>(0x58))*.30000001192092896f));end=hit.position;break;
    }
    w.set(0x14,stored(static_cast<long double>(w.template get<float>(0x14))-(static_cast<long double>(stored(human_frame_signed_word(elapsed)))*.0010000000474974513f)*9.800000190734863f));
    if(!w.template get<std::uint8_t>(0x78)){
        const auto travelled=stored(static_cast<long double>(distance(end,w.template get<HumanVector>(0x20)))+w.template get<float>(0x70));w.set(0x70,travelled);
        if(travelled>600.f)w.set(0x78,std::uint8_t{1});
    }else if(!(world.mode_flags()&2u))return 1;
    if(world.mode_flags()&2u)world.shot_bullet_flyby(record,w.template get<HumanVector>(0x20),end);
    publish(record,end);return 0;
}
//00449290. Material2C is SIGNED here (FILD DWORD), unlike bullet/grenade.
//A blocking contact first publishes20/40; blast/effects and78 follow that store.
template<class Record,class World>unsigned capsule(Record& record,std::uint32_t elapsed,World& world){
    auto& w=record.words;
    if(w.template get<std::uint8_t>(0x78)){
        const auto particle=w.template get<std::int32_t>(0x70);if(particle>=0)world.stop_shot_particle(particle);
        if(record.incoming_sound()&&world.shot_frame_visible(record.incoming_sound()))world.shot_hide_frame(record.incoming_sound());
        if(record.frame()&&world.shot_frame_visible(record.frame()))world.shot_hide_frame(record.frame());
        w.set(0x70,std::int32_t{-1});const auto timer=w.template get<std::uint32_t>(0x84);if(timer)w.set(0x84,timer-elapsed);
        return w.template get<std::int32_t>(0x84)<1?1u:0u;
    }
    w.set(0x50,0u);const auto origin=w.template get<HumanVector>(0x20);w.set(0x30,origin);
    const auto travel=displacement(w.template get<HumanVector>(0x10),elapsed);auto direction=w.template get<HumanVector>(0x10);normalize_human_ai_shot(direction,square_zyx(direction));
    auto hits=world.shot_travel_hits(record,origin,travel,0x74);shot_hit_sort::sort(hits);
    auto end=add(origin,travel);HumanVector end_normal{{0,1,0,0}};std::uint32_t end_material=0xffffffffu;bool explode=false;float factor=1;
    const auto direct_damage=record.payload_real(0x58);using ActorPointer=decltype(record.source);ActorPointer previous=nullptr;
    for(auto& hit:hits){
        const auto flags=world.human_geometry_flags(hit);auto frame=world.shot_hit_frame(hit);decltype(frame) picked{};
        std::uint32_t material=0xffffffffu;bool material_ready=false,force_stop=false;ActorPointer actor=nullptr;
        if(!(flags&0x40u)){
            material=flags>>24;
            if(flags&0xc0u){const auto detail=world.shot_detailed_pick(w.template get<HumanVector>(0x20),travel,frame);
                if(detail){picked=detail->frame;refine(hit,*detail,w.template get<HumanVector>(0x20),direction);if(world.mode_flags()&1u)world.shot_script_hit(picked);}}
            if(material==77)continue;
            if(material==76){w.set(0x78,std::uint8_t{1});w.set(0x84,0u);end=hit.position;end_normal=hit.normal;end_material=material;break;}
            factor=.5f;material_ready=true;
        }else{
            actor=world.item_melee_actor(hit);if(previous&&previous==actor)continue;previous=actor;
            if(!actor){
                const auto detail=world.shot_detailed_pick(w.template get<HumanVector>(0x20),travel,frame);if(!detail)continue;
                picked=detail->frame;refine(hit,*detail,w.template get<HumanVector>(0x20),direction);const auto part=world.shot_version_material(picked);
                if(part){material=static_cast<std::uint32_t>(*part);if((world.mode_flags()&1u)&&!world.shot_version_damage(picked,human_frame_signed_word(human_packet_ftol(w.template get<float>(0x58)))))picked=decltype(frame){};}
                else{material=world.shot_picked_material(picked);
                    if(world.shot_hit_has_physics(frame)){if(material==0xffffffffu)material=world.shot_picked_material(frame);world.shot_physics_impulse(frame,w.template get<HumanVector>(0x10),hit.position);}
                    if((world.mode_flags()&1u)&&picked)world.shot_script_hit(picked);}
                material_ready=true;
            }else if(!w.template get<std::uint8_t>(0x6c)){
                const auto kind=actor->base().type();
                if(kind==3){
                    if(world.shot_human_zone(*actor,w.template get<HumanVector>(0x20),travel)==0xff){if(world.mode_flags()&1u)world.shot_human_miss(*actor,record.source,w.template get<HumanVector>(0x10),false);continue;}
                    material=static_cast<std::uint32_t>(world.shot_contact_material(*actor,decltype(frame){}));force_stop=true;factor=1;material_ready=true;
                }else if(kind==4){
                    if(world.shot_source_attached_to(record.source,*actor))continue;
                    if(world.mode_flags()&1u){auto point=hit.position;for(unsigned a=0;a!=3;++a){const auto offset=static_cast<long double>(direction[a])*.10000000149011612f;point[a]=stored(static_cast<long double>(point[a])-(a==2?stored(offset):offset));}
                        const auto amount=world.shot_capsule_car_boost(record)?stored(static_cast<long double>(direct_damage)*4.f):direct_damage;
                        world.shot_actor_damage(*actor,7,amount,point,direction,record.source,decltype(frame){});}
                    material=world.shot_car_material(*actor,w.template get<HumanVector>(0x20),travel);
                    if(material==0xffffffffu){const auto detail=world.shot_detailed_pick(w.template get<HumanVector>(0x20),travel,actor->base().frame());if(!detail)continue;picked=detail->frame;material=static_cast<std::uint32_t>(world.shot_contact_material(*actor,picked));if(material==0xffffffffu)continue;}
                    if(world.mode_flags()&1u){world.shot_script_hit(actor->base().frame());}
                    factor=.10000000149011612f;force_stop=true;material_ready=true;
                }else{
                    const auto detail=world.shot_detailed_pick(w.template get<HumanVector>(0x20),travel,actor->base().frame());if(!detail)continue;
                    picked=detail->frame;refine(hit,*detail,w.template get<HumanVector>(0x20),direction);material=static_cast<std::uint32_t>(world.shot_contact_material(*actor,picked));
                    if(world.mode_flags()&1u){world.shot_script_hit(actor->base().frame());}
                    force_stop=true;material_ready=true;
                }
            }
        }
        if(!material_ready)continue;
        const auto threshold=human_frame_signed_word(world.shot_material_threshold(material));
        if(threshold==-1||force_stop||w.template get<float>(0x58)<static_cast<long double>(threshold)){
            explode=true;end=hit.position;end_normal=hit.normal;end_material=material;break;
        }
        w.set(0x58,stored(static_cast<long double>(w.template get<float>(0x58))-threshold));
        if((world.mode_flags()&2u)&&((material>=31&&material<=34)||material==41||material==43||material==62||material==70)){
            world.shot_impact_sound(record.source,material,hit.position,w.template get<std::uint8_t>(0x6c)!=0);world.human_damage_material_effect(material,hit.position,hit.normal);
        }
    }
    w.set(0x14,stored(static_cast<long double>(w.template get<float>(0x14))-(static_cast<long double>(stored(human_frame_signed_word(elapsed)))*.0010000000474974513f)*9.800000190734863f));
    if(!w.template get<std::uint8_t>(0x78)){
        const auto travelled=stored(static_cast<long double>(distance(end,w.template get<HumanVector>(0x20)))+w.template get<float>(0x74));w.set(0x74,travelled);
        if(travelled>600.f){w.set(0x78,std::uint8_t{1});w.set(0x84,0u);}
    }else if(!(world.mode_flags()&2u))return 1;
    publish(record,end);
    if(explode){
        if(world.mode_flags()&1u){auto centre=w.template get<HumanVector>(0x20);for(unsigned a=0;a!=3;++a){const auto offset=static_cast<long double>(end_normal[a])*.5f;centre[a]=stored((a==2?stored(offset):offset)+centre[a]);}world.shot_capsule_explosion(record,centre,factor);}
        if(world.mode_flags()&2u)world.shot_capsule_effect(record,w.template get<HumanVector>(0x20),end_normal,end_material);
        w.set(0x78,std::uint8_t{1});if(w.template get<std::uint8_t>(0x88))w.set(0x84,1500u);
    }
    return 0;
}
//004486B0/00451E40. The selected Player is reread as native; source is not
//the listener. Emitter result is ignored and6E publishes AFTER the SOUND call.
template<class Record,class World>void flyby(Record& record,const HumanVector& start,const HumanVector& end,World& world){
    if(record.words.template get<std::uint8_t>(0x6e)||world.shot_current_player()==record.source)return;
    if(!world.shot_current_player())return;
    auto* player=world.shot_current_player();if(!player)throw std::logic_error("Bullet flyby lost native third Player read");
    const auto listener=world.shot_player_head_position(*player);HumanVector travel{},from{},to{};
    for(unsigned a=0;a!=3;++a){travel[a]=stored(static_cast<long double>(end[a])-start[a]);from[a]=stored(static_cast<long double>(start[a])-listener[a]);to[a]=stored(static_cast<long double>(end[a])-listener[a]);}
    normalize_human_ai_shot(to,square_zyx(to));normalize_human_ai_shot(from,square_zyx(from));normalize_human_ai_shot(travel,square_zyx(travel));
    if(!(dot(from,travel)<=0&&dot(to,travel)>=0))return;
    const auto square=square_zyx(travel);
    if(square>=9.99999993922529e-09f){
        auto relative=record.words.template get<HumanVector>(0x20);for(unsigned a=0;a!=3;++a)relative[a]=stored(static_cast<long double>(relative[a])-listener[a]);
        const float projection=stored(dot(relative,travel)/square);HumanVector perpendicular{};
        for(unsigned a=0;a!=3;++a)perpendicular[a]=stored(static_cast<long double>(relative[a])-stored(static_cast<long double>(projection)*travel[a]));
        if(square_zyx(perpendicular)>=9.f)return;
    }
    world.shot_flyby_sound(listener);record.words.set(0x6e,std::uint8_t{1});
}
} //namespace hd2::game::shot_combat
#endif
