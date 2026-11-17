#ifndef HD2_GAME_SHOT_MOTION_FRAMES_H
#define HD2_GAME_SHOT_MOTION_FRAMES_H
#include "game/human_state_frame_timers.h"
#include "game/human_packet_commands.h"
#include <cmath>
#include <cstring>
namespace hd2::game {
namespace shot_motion {
inline float stored(long double value){return static_cast<float>(value);}
inline long double square(const HumanVector& v){return (static_cast<long double>(v[0])*v[0]+static_cast<long double>(v[1])*v[1])+static_cast<long double>(v[2])*v[2];}
inline long double dot(const HumanVector& a,const HumanVector& b){ //00454660, Z/Y/X.
    return (static_cast<long double>(a[2])*b[2]+static_cast<long double>(a[1])*b[1])+static_cast<long double>(a[0])*b[0];
}
//00454360. The sound uses the OLD position, before publishing the contact.
//007183A0 is a ray (layers1/flags0), not a sphere sweep.
template<class Record,class World>unsigned shell(Record& record,std::uint32_t elapsed,World& world){
    auto& w=record.words;if(w.template get<std::uint32_t>(0x2c)==3)return 2;
    w.set(0x28,w.template get<std::uint32_t>(0x28)+elapsed);
    auto position=w.template get<HumanVector>(0),velocity=w.template get<HumanVector>(0x10);HumanVector displacement{};
    const float milliseconds=stored(human_frame_signed_word(elapsed));
    for(unsigned a=0;a!=3;++a){
        //Native X product remains extended; Y/Z spill before the .001 multiply.
        const auto product=static_cast<long double>(human_frame_signed_word(elapsed))*velocity[a];
        displacement[a]=stored((a==0?product:stored(product))*.0010000000474974513f);
    }
    const auto hit=world.shot_shell_ray(record,position,displacement);
    velocity=w.template get<HumanVector>(0x10);
    if(!hit){for(unsigned a=0;a!=3;++a)position[a]=stored(static_cast<long double>(position[a])+displacement[a]);}
    else if(position[0]==hit->position[0]&&position[1]==hit->position[1]&&position[2]==hit->position[2]){
        for(unsigned a=0;a!=3;++a)velocity[a]=0; //Native scratch W is unproduced.
        w.set(0x10,velocity);
    }else{
        w.set(0x2c,w.template get<std::uint32_t>(0x2c)+1);
        const float speed_square=stored(square(velocity));std::int32_t bits;std::memcpy(&bits,&speed_square,4);
        if(bits<0x322bcc77||(std::abs(static_cast<long double>(speed_square)-1.f)<9.99999993922529e-09f?1.L:std::sqrt(static_cast<long double>(speed_square)))<.10000000149011612f){for(unsigned a=0;a!=3;++a)velocity[a]=0;}
        else{
            const auto impulse=dot(velocity,hit->normal)*1.2000000476837158f;
            for(unsigned a=0;a!=3;++a)velocity[a]=stored(static_cast<long double>(velocity[a])-hit->normal[a]*impulse);
            if(dot(velocity,hit->normal)<0)for(unsigned a=0;a!=3;++a)velocity[a]=-velocity[a]; //00454680 negates, does NOT normalize.
        }
        w.set(0x10,velocity);world.shot_shell_sound(position,w.template get<std::int32_t>(0x30));
        //Only XYZ have a source-level collision result; native padding is unclaimed.
        for(unsigned a=0;a!=3;++a)position[a]=hit->position[a];
    }
    w.set(0,position);
    if(record.frame())world.shot_motion_position(record.frame(),position);
    //Native reloads velocity after the visual callbacks.
    velocity=w.template get<HumanVector>(0x10);
    if(square(velocity)!=0){velocity[1]=stored(static_cast<long double>(velocity[1])-(static_cast<long double>(milliseconds)*.0010000000474974513f)*3.799999952316284f);w.set(0x10,velocity);}
    return human_frame_signed_word(w.template get<std::uint32_t>(0x28))>2000?2u:0u; //Native local5C explicitly initialized0.
}
//0044D290, called from authority0044D260 only without mode2; otherwise from
//presentation leaf0044D240. Body update precedes EVERY state/timer observation.
template<class Record,class World>void grenade(Record& record,std::uint32_t elapsed,World& world){
    world.shot_grenade_body(record,elapsed);auto& w=record.words;
    if(w.template get<std::uint32_t>(0x678)==2){
        record.body().close();
        if(world.mode_flags()&1u){auto point=world.shot_motion_world_position(record.frame());const auto normal=w.template get<HumanVector>(0x680);
            for(unsigned a=0;a!=3;++a)point[a]=stored(static_cast<long double>(stored(static_cast<long double>(normal[a])*.5f))+point[a]);
            world.shot_grenade_explosion(record,point);
        }
        if(world.mode_flags()&2u)world.shot_grenade_effect(record,world.shot_motion_world_position(record.frame()),-1);
        w.set(0x678,1u);return;
    }
    w.set(0x670,w.template get<std::uint32_t>(0x670)+elapsed);
    const auto timer=human_frame_signed_word(w.template get<std::uint32_t>(0x670)),fuse=w.template get<std::int32_t>(0x674);
    if(fuse>0&&fuse<timer){
        record.body().close();
        if(world.mode_flags()&1u)world.shot_grenade_explosion(record,world.shot_motion_world_position(record.frame()));
        if(world.mode_flags()&2u)world.shot_grenade_effect(record,world.shot_motion_world_position(record.frame()),w.template get<std::int32_t>(0x690));
        w.set(0x678,1u);
    }
    if(timer>=15001)w.set(0x678,1u);
}
template<class Record,class World>unsigned grenade_authority(Record& record,std::uint32_t elapsed,World& world){
    if(record.words.template get<std::uint32_t>(0x678)==1)return 1;
    if(!(world.mode_flags()&2u))grenade(record,elapsed,world);
    return record.words.template get<std::uint32_t>(0x678);
}
template<class Record,class World>void grenade_presentation(Record& record,std::uint32_t elapsed,World& world){
    if(record.words.template get<std::uint32_t>(0x678)!=1)grenade(record,elapsed,world);
}
//0044D570:1 pass-through,2 collision response,3 stop. Material2C is an
//UNSIGNED integer threshold with FFFFFFFF sentinel, not FLOAT resistance.
template<class Record,class Event,class World>int grenade_contact(Record& record,unsigned kind,Event* event,World& world){
    if(kind!=1)return 0;
    if(world.shot_grenade_initial_passthrough(record))return 1;
    if(!event||!event->primitive)throw std::logic_error("Grenade contact requires native primitive/event");
    auto& primitive=*event->primitive;auto* actor=world.shot_contact_actor(primitive);const float power=record.payload_real(0xc);
    if(!(primitive.flags&0x40u)){
        const auto material=primitive.flags>>24;
        if(material==77)return 1;
        if(material==76){record.words.set(0x678,1u);return 3;}
        const auto threshold=world.shot_material_threshold(material);
        if(threshold!=0xffffffffu&&static_cast<long double>(threshold)<power)return 1;
    }else if(actor){
        if(actor==record.source)return 1;
        (void)world.shot_contact_material(*actor,primitive.frame); //Discarded D8 call remains observable.
    }else{
        const auto master=world.shot_frame_versions(primitive.frame);
        if(master){
            auto start=event->position;HumanVector displacement{};
            for(unsigned a=0;a!=3;++a){
                displacement[a]=stored(static_cast<long double>(event->normal[a])*-1.f);
                const auto offset=static_cast<long double>(event->normal[a])*.10000000149011612f;
                start[a]=stored((a==2?stored(offset):offset)+event->position[a]);
            }
            const auto picked=world.shot_contact_pick(start,displacement,primitive.frame);
            if(picked){
                const auto material=world.shot_picked_material(picked),threshold=world.shot_material_threshold(material);
                if(threshold!=0xffffffffu&&static_cast<long double>(threshold)<power){
                    if(world.mode_flags()&1u)world.shot_versions_damage(master,picked,material==62?10000:human_frame_signed_word(human_packet_ftol(power)));
                    return 1;
                }
            }
        }
    }
    if(record.words.template get<std::int32_t>(0x674)==0){record.words.set(0x678,2u);return 3;}
    if((world.mode_flags()&1u)&&actor&&world.shot_contact_type(*actor)==3)world.shot_grenade_human_damage(*actor,event->position,event->normal,record.source);
    if(world.mode_flags()&2u)world.shot_grenade_impact_sound(record);
    return 2;
}
} //namespace shot_motion
} //namespace hd2::game
#endif
