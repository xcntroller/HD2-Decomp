#ifndef HD2_GAME_HUMAN_GROUND_MOTION_H
#define HD2_GAME_HUMAN_GROUND_MOTION_H

#include "game/human_ground_collision.h"
#include "game/human_ladder_control.h"
#include "game/human_joint_owners.h"
#include <type_traits>

namespace hd2::game {

inline long double human_fall_displacement(std::int32_t elapsed,float speed,float phase){ //00789BC0.
    long double distance=0,velocity=speed;
    while(elapsed>0){
        if(static_cast<long double>(elapsed)+phase<=85.f)
            return static_cast<long double>(elapsed)*.001f*velocity+distance;
        const auto duration=85.L-phase;
        distance+=static_cast<long double>(.001f)*duration*velocity;
        velocity+=static_cast<long double>(9.81f)*.3333333432674408f;
        elapsed=static_cast<std::int32_t>(static_cast<long double>(elapsed)-duration);
        phase=0;
    }
    return distance;
}

template<class Hit>bool select_human_ground_contact(const CollisionMotion<Hit>& request,bool prone,
        HumanVector& normal,std::optional<Hit>& identity){ //00788600, reverse native contact order.
    identity.reset();normal[0]=normal[1]=normal[2]=0;
    bool steep=false;
    for(std::size_t reverse=request.contact_count;reverse!=0;){
        const auto index=--reverse;
        const auto& contact=request.contacts.at(index);
        if(!contact)throw std::logic_error("Human motion contact has no actual retained query");
        if(contact->sphere&&!prone)continue;
        if(prone&&index&&request.contacts[index-1]->normal[1]>=0&&contact->normal[1]<=0)continue;
        steep=contact->normal[1]<-.3f||contact->normal[1]>.6f;
        if(steep)identity=contact->identity;
        normal=contact->normal;
        if(normal[1]>0)return steep;
    }
    return steep;
}

template<class Control,class Look>
void apply_human_slope_motion(Control& control,const Look& look,float normal_y,float vertical_rate,float alignment,
        HumanVector& destination,const HumanVector& origin,std::int32_t elapsed){ //00766F10.
    using namespace collision_motion_math;
    auto& state=control.physical().control_frame_words();auto& movement=control.movement().frame_words();
    float amount=0;
    if(normal_y<=.95f){
        const auto displacement=movement.template get<HumanVector>(0x10);
        const float horizontal_rate=static_cast<float>((static_cast<long double>(displacement[0])*displacement[0]*1000.f+
            static_cast<long double>(displacement[2])*displacement[2]*1000.f)/elapsed);
        if(horizontal_rate>0&&horizontal_rate<1)vertical_rate/=horizontal_rate;
        if(normal_y>0&&std::abs(static_cast<long double>(vertical_rate))>.20000000298023224){
            if(alignment<0)alignment=-alignment;
            amount=static_cast<float>((1.L-normal_y)*alignment*1.5f);
            const auto stance=look.state().template get<std::uint32_t>(0x50);
            if(stance==1||stance==4)amount*=.7f;
            else if(stance==0)amount*=.5f;
            if(amount>1||normal_y<=.55f)amount=1;
        }
    }
    const auto previous=movement.template get<float>(0x54);
    if(std::abs((static_cast<long double>(previous)-amount)*100.f)/elapsed>.17000000178813934){
        const auto step=static_cast<long double>(elapsed)*.17000000178813934f*.01f;
        amount=static_cast<float>(previous<=amount?step+previous:previous-step);
    }
    movement.set(0x54,amount);
    float playback=static_cast<float>(1.L-static_cast<long double>(amount)*.5f);
    if(playback<.65f)playback=.65f;
    if(state.template get<std::int32_t>(0x274)<1)control.tracks().motion_animation_rate(0,playback);
    if(amount==0)return;
    if(amount>=.9f){destination=origin;return;}
    const auto padding=movement.template get<float>(0x1c);
    for(unsigned axis=0;axis!=3;++axis)
        destination[axis]=static_cast<float>(destination[axis]-(static_cast<long double>(destination[axis])-origin[axis])*amount);
    destination[3]=padding;
}

template<class Control,class Look,class Hit>
void apply_human_ground_contact_slope(Control& control,const Look& look,CollisionMotion<Hit>& request,
        const HumanVector& normal,bool steep,bool meaningful,bool primary_hit,const HumanVector& origin,
        std::int32_t elapsed){ //007671C0.
    using namespace collision_motion_math;
    auto& state=control.physical().control_frame_words();state.set(0x758,0.f);
    if(primary_hit&&meaningful&&state.template get<std::int32_t>(0x1d8)>189&&
            std::abs(static_cast<long double>(request.position[1])-request.centers[0][1])<=1.){
        const float height=static_cast<float>((static_cast<long double>(normal[0])+normal[2])*0.f+normal[1]);
        const auto direction=normalized({{request.displacement[0],0,request.displacement[2],0}});
        const auto side=normalized({{normal[0],0,normal[2],0}});
        const float alignment=static_cast<float>((static_cast<long double>(direction[1])*side[1]+
            static_cast<long double>(side[0])*direction[0])+static_cast<long double>(direction[2])*side[2]);
        if(height<.9f&&steep)state.set(0x758,std::max(.5f,-(alignment*height)));
        if(height>.5f&&alignment<0){
            const auto rate=static_cast<float>((1000.L/static_cast<float>(elapsed))*(static_cast<long double>(request.position[1])-origin[1]));
            apply_human_slope_motion(control,look,height,rate,alignment,request.position,origin,elapsed);return;
        }
    }
    apply_human_slope_motion(control,look,1.f,1.f,1.f,request.position,origin,elapsed);
}

template<class Control,class Human,class Services,class Runtime>
void apply_human_ground_material(Control& control,Human& human,Services& services,Runtime& runtime,std::int32_t material,std::int32_t elapsed){ //0075EEE0.
    if(material<68)return;
    auto& state=control.physical().control_frame_words();
    const float time=static_cast<float>(elapsed);
    switch(material){
    case 70:services.apply_human_damage(human,15u,time*.12f,HumanVector{},HumanVector{},static_cast<Human*>(nullptr),0);return;
    case 71:services.apply_human_damage(human,6u,time*.16f,HumanVector{},HumanVector{},static_cast<Human*>(nullptr),0);state.set(0x75c,71u);return;
    case 90:services.apply_human_damage(human,6u,time*.14f,HumanVector{},HumanVector{},static_cast<Human*>(nullptr),0);state.set(0x75c,90u);return;
    case 72:services.apply_human_damage(human,6u,-1.f,HumanVector{},HumanVector{},static_cast<Human*>(nullptr),0);return;
    case 93:services.apply_human_damage(human,9u,-1.f,HumanVector{},HumanVector{},static_cast<Human*>(nullptr),0);return;
    default:break;
    }
    const bool eligible=state.template get<std::int32_t>(0x1d8)<190||(services.game_word28()/100u)%10u==0;
    if((material==68||material==69||material==98)&&eligible&&(control.mode_flags()&1u))
        runtime.human_ground_surface_effect(human,material); //00451900 receives the actual current model world position.
}

template<class Control,class Human,class Services,class Hit,class Runtime>
void apply_human_ground_material(Control& control,Human& human,Services& services,Runtime& runtime,
        const std::optional<Hit>& identity,std::int32_t elapsed){ //0075F180 direct contact path.
    auto& state=control.physical().control_frame_words();state.set(0x75c,std::int32_t{-1});state.set(0x760,std::uint8_t{1});
    if(identity)apply_human_ground_material(control,human,services,runtime,human_frame_signed_word(services.human_geometry_material(*identity)),elapsed);
}

template<class Control,class Human,class Services,class Hit,class Runtime>
void apply_human_ground_contacts(Control& control,Human& human,Services& services,Runtime& runtime,
        const CollisionMotion<Hit>& request,std::int32_t elapsed){ //0075F180 reverse native three-slot duplicate suppression.
    auto& state=control.physical().control_frame_words();state.set(0x75c,std::int32_t{-1});state.set(0x760,std::uint8_t{1});
    std::array<std::int32_t,3> recent{{-1,-1,-1}};
    for(std::size_t reverse=request.contact_count;reverse!=0;){
        const auto index=--reverse;const auto material=human_frame_signed_word(services.human_geometry_material(request.contacts.at(index)->identity));
        if(material!=recent[0]&&material!=recent[1]&&material!=recent[2])apply_human_ground_material(control,human,services,runtime,material,elapsed);
        recent[index%3]=material;
    }
}

template<class Control,class Look,class Human,class Services,class Runtime>
std::int32_t query_human_ground_water(Control& control,Look& look,Human& human,Services& services,Runtime& runtime){ //0074EA80.
    auto& state=control.physical().control_frame_words();auto& movement=control.movement().frame_words();
    const auto position=human.base().world_position();
    constexpr float inactive=-10000.099609375f;
    if(movement.template get<float>(0x118)==-std::numeric_limits<float>::max()||state.template get<std::int32_t>(0x9ac)!=-10000)return 0;
    if(movement.template get<float>(0x118)<position[1]){
        movement.set(0x10c,inactive);movement.set(0x108,inactive);movement.set(0x114,inactive);return 0;
    }
    movement.set(0x10c,inactive);movement.set(0x108,inactive);
    const auto posture=look.state().template get<std::uint32_t>(0x4c);
    float depth=posture==1?1.2f:posture==2?1.f:1.4f,shallow=posture==1||posture==2?.7f:.9f;
    auto origin=position;
    if(posture<=2&&look.state().template get<std::uint32_t>(0xb8)==5){
        const auto direction=human.base().world_direction();
        origin[0]=static_cast<float>(origin[0]+static_cast<long double>(direction[0])*.3f);
        origin[2]=static_cast<float>(origin[2]+static_cast<long double>(direction[2])*.3f);
    }
    const auto water=services.human_geometry_water_ray(origin,{{0,600,0,0}},0);
    float surface=-1000.f;
    if(water){movement.set(0x10c,water->position[1]-position[1]);surface=water->position[1];state.set(0x764,services.human_geometry_material(*water));}
    else movement.set(0x10c,-1.f);
    if(movement.template get<float>(0x10c)<0){
        movement.set(0x10c,inactive);movement.set(0x108,inactive);movement.set(0x114,water?surface:inactive);return 0;
    }
    movement.set(0x114,surface);state.set(0x230,std::int32_t{-8});
    const auto floor=[&]{
        auto& physical=control.physical();
        struct Restore {decltype(physical)& owner;~Restore(){owner.restore_query(false);}} restore{physical};
        physical.exclude_query(false);
        return services.human_geometry_ray(origin,{{0,-600,0,0}},0xffffffffu,0x230000u);
    }();
    if(movement.template get<float>(0x10c)>.2f)look.mark_frame_feedback(0x800u);
    movement.set(0x108,floor?static_cast<float>((static_cast<long double>(surface)-floor->position[1])+.01f):10000.f);
    const auto attachment=state.template get<std::int32_t>(0xa2c),equipment=state.template get<std::int32_t>(0xa30);
    if(attachment==-10000&&equipment==-10000){
        const auto floor_mode=look.state().template get<std::uint32_t>(0xb8)==5;
        if(depth<movement.template get<float>(0x108)){
            if(!floor_mode){
                const auto minimum=posture==0?1.f:posture==1?.7f:.4f;
                if(posture<=2&&movement.template get<float>(0x10c)<minimum)return 0;
            }
            return 1;
        }
        if(!floor_mode)return 0;
        return !(movement.template get<float>(0x108)<shallow&&movement.template get<float>(0x10c)<movement.template get<float>(0x108));
    }
    if(attachment!=-10000&&equipment==-10000)return 0;
    runtime.retire_human_water_attachment(human); //005A97C0, positive offline174 command/packet receiver.
    if(movement.template get<float>(0xfc)!=inactive)movement.set(0xfc,human.base().world_position()[1]);
    return 0;
}

template<class Control,class Human,class Services,class Runtime>
void apply_human_fall_damage(Control&,Human&,Services&,Runtime&,bool);

template<class Control,class Look,class Services,class Runtime,class Publish>
void begin_human_ground_water(Control& control,Look& look,Services& services,Runtime& runtime,Publish& publish){ //0076AC80.
    auto& state=control.physical().control_frame_words();auto& movement=control.movement().frame_words();
    if(movement.template get<float>(0xfc)!=-10000.099609375f)apply_human_fall_damage(control,**control.human_slot_address(),services,runtime,true); //00770C30.
    control.face().frame_running(false,true,services.human_control_animation_scale());
    const auto animation=state.template get<std::int32_t>(0x1d8);
    const bool forward=animation==190||animation==198,backward=animation==191||animation==199;
    movement.set(0x12c,static_cast<std::uint8_t>(backward));
    const bool alternate=control.frame_special_movement_item();
    look.force_frame_commands(true);
    try{look.frame_movement_animation(forward||backward?(alternate?92:90):(alternate?93:91),publish);}
    catch(...){look.force_frame_commands(false);throw;}
    look.force_frame_commands(false);look.control_movement_mode(4,true,publish);look.mark_frame_feedback(0x420u);
    services.human_presentation_equipment(**control.human_slot_address()).set_hidden(true); //00791C60.
    float upward=.5f;
    if(movement.template get<float>(0x10c)!=-10000.099609375f)
        upward=std::min(upward,static_cast<float>(static_cast<long double>(movement.template get<float>(0x10c))-.15f));
    const auto scale=services.human_control_animation_scale();
    const auto duration=[&](long double value){
        value/=scale;
        if(!std::isfinite(value)||value< -0x1p63L||value>=0x1p63L)return std::int32_t{0};
        return human_frame_signed_word(static_cast<std::uint32_t>(static_cast<std::int64_t>(value)));
    };
    control.tracks().motion_impulse({{0,upward,0,0}},duration(800.L));movement.set(0x110,duration(1200.L));
}

template<class Movement>void advance_human_fall_velocity(Movement& movement,std::int32_t elapsed,bool drag){ //00789B00.
    auto speed=movement.template get<float>(0xf4),phase=movement.template get<float>(0xf8);
    if(drag){
        const auto next=speed-static_cast<long double>(speed)*.4f*(static_cast<long double>(elapsed)*.01f);
        movement.set(0xf8,0.f);movement.set(0xf4,static_cast<float>(next));
        if(next<5.f)movement.set(0xf4,5.f);
        return;
    }
    while(elapsed>0){
        if(static_cast<long double>(elapsed)+phase<=85.f){movement.set(0xf8,static_cast<float>(static_cast<long double>(elapsed)+phase));return;}
        speed=static_cast<float>(static_cast<long double>(9.81f)*.3333333432674408f+speed);movement.set(0xf4,speed);
        elapsed=static_cast<std::int32_t>(static_cast<long double>(elapsed)-(85.L-phase));phase=0;movement.set(0xf8,phase);
    }
}

template<class Control,class Human,class Services,class Runtime>
void apply_human_fall_damage(Control& control,Human& human,Services& services,Runtime& runtime,bool final){ //00770C30.
    auto& movement=control.movement().frame_words();auto& state=control.physical().control_frame_words();
    const bool local=(control.mode_flags()&7u)==7u||control.human_is_player()||control.animation_authority_actor();
    if(!local)return;
    constexpr float inactive=-10000.099609375f;
    const bool water=movement.template get<float>(0x114)!=inactive;
    const float threshold=water?6.f:3.f,scale=water?60.f:240.f;
    const auto previous=movement.template get<float>(0xfc);
    const auto position=human.base().world_position();
    if((!final||previous!=inactive)&&previous<100000.f&&threshold<static_cast<long double>(previous)-position[1]){
        float amount=static_cast<float>(((static_cast<long double>(previous)-position[1])-threshold)*scale);
        const auto health=human_frame_signed_word(human.properties.stats_ec[0]);
        if(!final){
            if(health>amount)return;
            const auto hit=services.human_geometry_ray(position,{{0,-50,0,0}},0,0,false);
            if(hit&&(services.human_geometry_flags(*hit)&0x50000u)==0x50000u){
                amount=static_cast<float>(((static_cast<long double>(previous)-human.base().world_position()[1])-6.f)*240.f*.25f);
                if(amount<static_cast<float>(health))return;
            }
        }
        if(!state.template get<std::uint8_t>(0xa90)&&
                ((control.mode_flags()&7u)==7u||runtime.human_fall_damage_authority(control))){
            if(control.mode_flags()&1u)services.apply_human_damage(human,0,std::min(amount,1000.f),HumanVector{},HumanVector{},&human,0); //Actual Human virtualD4.
            else runtime.send_human_fall_damage(human,amount); //005A9BF0, original packet receiver owns truncation.
        }
        movement.set(0xfc,inactive);
    }else if(final)movement.set(0xfc,inactive);
}

template<class Control,class Look,class Human,class Services,class Runtime,class Publish>
void finish_human_walk_fall(Control& control,Look& look,Human& human,Services& services,
        Runtime& runtime,bool impact,Publish& publish){ //00770B90.
    apply_human_fall_damage(control,human,services,runtime,true);
    auto& state=control.physical().control_frame_words();auto& movement=control.movement().frame_words();
    if(state.template get<std::int32_t>(0x1d8)==17||state.template get<std::uint8_t>(0xa90))look.frame_movement_animation(45,publish,false);
    if(impact){
        if(movement.template get<float>(0xf4)>9.81f&&human.activation.vector1b0[1]<-1.f)
            control.play_presentation_track_sound(17); //0075E770, SAME physical sound vector and shared pool.
        state.set(0xa90,std::uint8_t{0});
    }
    movement.set(0xf4,0.f);movement.set(0xf8,0.f);movement.set(0x60,std::int32_t{-10000});movement.set(0x5c,-10000.099609375f);
}

template<class Control,class Look,class Human,class Services,class Hit,class Runtime,class Publish>
void advance_human_walk_fall(Control& control,Look& look,Human& human,Services& services,const std::optional<Hit>& identity,
        bool steep,std::int32_t elapsed,bool wall,bool publish_position,Runtime& runtime,Publish& publish){ //00767530.
    using namespace collision_motion_math;
    auto& physical=control.physical();auto& state=physical.control_frame_words();auto& movement=control.movement().frame_words();
    constexpr float inactive=-10000.099609375f;
    const bool accelerating=(!identity||!steep)&&movement.template get<float>(0xf4)!=0;
    if(accelerating||!identity){
        const auto water=query_human_ground_water(control,look,human,services,runtime);
        if(!water){
            const auto height=human.base().world_position()[1],previous=movement.template get<float>(0xfc);
            if(previous==inactive||previous<height||(accelerating&&identity&&!wall))movement.set(0xfc,height);
            else apply_human_fall_damage(control,human,services,runtime,false);
            advance_human_fall_velocity(movement,elapsed,state.template get<std::uint8_t>(0xa90)!=0);
        }else{
            movement.set(0xf4,movement.template get<std::int32_t>(0x110)<0?0.f:accelerating?2.f:4.f);movement.set(0xf8,0.f);
        }
        const auto animation=state.template get<std::int32_t>(0x1d8);
        if(accelerating&&movement.template get<std::int32_t>(0x60)==-10000&&animation!=22&&animation!=21&&animation!=20){
            struct Restore {decltype(physical)& owner;~Restore(){owner.restore_query(false);}} restore{physical};
            physical.exclude_query(false);
            const auto sphere=physical.posture_spheres().at(0);
            const auto floor=services.human_geometry_ray(sphere,{{0,-20,0,0}},0xffbfffffu,0x20000u);
            const float distance=floor?static_cast<float>(std::abs(static_cast<long double>(human.base().world_position()[1])-floor->position[1])+.5):150.f;
            if((!floor||distance>1.f)&&state.template get<std::int32_t>(0x1ac)==1){
                const auto hit=services.human_geometry_sweep().sweep(sphere,{{0,-distance,0,0}},static_cast<float>(static_cast<long double>(sphere[3])*.7f),0xffbfffffu,0x20000u);
                movement.set(0x60,std::int32_t{-10000});
                const float ground=hit?static_cast<float>((static_cast<long double>(hit->position[1])-sphere[3])+.1f):sphere[1]-distance;
                movement.set(0x5c,ground);
                const auto height=human.base().world_position()[1];const float drop=std::abs(height-ground);
                if(height<ground||drop<1.3f)movement.set(0x60,std::abs(static_cast<long double>(distance)-drop)>=1.?-10000:0);
                else if(drop<2.5f)movement.set(0x60,1u);
                else if(drop<5.f)movement.set(0x60,2u);
                else{
                    if(look.state().template get<std::uint32_t>(0x4c)==0&&!state.template get<std::uint8_t>(0xa90)){
                        state.set(0x1fc,17u);normalize_human_primary_animation(state,0x1fc);look.frame_movement_animation(17,publish,false);
                    }
                    if(state.template get<std::uint8_t>(0xa90))movement.set(0x60,2u);
                }
            }
        }
        const auto displacement=movement.template get<HumanVector>(0x10);
        const bool local=(control.mode_flags()&7u)==7u||control.human_is_player()||control.animation_authority_actor()||state.template get<std::int32_t>(0x1ac)!=1;
        if(square(displacement)>=1.e-16f&&!identity&&local&&publish_position&&
                (!accelerating||!state.template get<std::uint8_t>(0x761))){
            human.base().world_position(add(human.base().world_position(),scale(displacement,.9f)));movement.set(0x51,std::uint8_t{1});
        }
    }
    const auto kind=movement.template get<std::int32_t>(0x60);
    if(kind==-10000||movement.template get<float>(0x5c)==inactive||identity||state.template get<std::int32_t>(0x1ac)!=1||movement.template get<float>(0x114)!=inactive)return;
    const float seconds=static_cast<float>(elapsed)*.001f;
    const auto height=human.base().world_position()[1];
    const auto base=static_cast<long double>(seconds)*movement.template get<float>(0xf4)*1.5f+movement.template get<float>(0x5c);
    const auto posture=look.state().template get<std::uint32_t>(0x4c);
    if(kind==1){
        if(base+1.6f<height)return;
        if(posture==0){state.set(0x1fc,22u);normalize_human_primary_animation(state,0x1fc);return;}
        if(posture!=1)return;
    }else{
        if(kind<2||kind>3||base+2.9f<height)return;
        if(posture==0){state.set(0x1fc,20u);normalize_human_primary_animation(state,0x1fc);return;}
    }
    state.set(0x1fc,21u);normalize_human_primary_animation(state,0x1fc);
}

//007646F0. Required positive material damage, water/fall and actor interaction
//workers remain explicit Runtime receivers, preserving their original order.
template<class Control,class Look,class Human,class Services,class Runtime,class Publish>
bool advance_human_walk_motion(Control& control,Look& look,Human& human,Services& services,
        std::int32_t elapsed,Runtime& runtime,Publish& publish){
    using namespace collision_motion_math;
    auto& physical=control.physical();auto& state=physical.control_frame_words();
    auto& movement=control.movement().frame_words();
    const auto word=[&](unsigned offset){return state.template get<std::int32_t>(offset);};
    const auto move_word=[&](unsigned offset){return movement.template get<std::int32_t>(offset);};
    const auto move_real=[&](unsigned offset){return movement.template get<float>(offset);};
    const auto add_timer=[&](unsigned offset){movement.set(offset,movement.template get<std::uint32_t>(offset)+static_cast<std::uint32_t>(elapsed));};
    const auto player=control.human_is_player(),authority=control.animation_authority_actor();
    const bool local=(control.mode_flags()&7u)==7u||player||authority;
    const bool publish_position=local||word(0x1ac)!=1;
    const auto original_sphere=physical.posture_spheres().at(0);
    auto posture=look.state().template get<std::uint32_t>(0x4c);
    physical.frame_globals().ground_normal={{0,1,0,0}};
    auto displacement=state.template get<HumanVector>(0x4d0);movement.set(0x10,displacement);
    const float seconds=static_cast<float>(elapsed)*.001f;
    if(state.template get<std::uint32_t>(0x22c)&4u){
        const auto original=human.base().world_position();auto destination=add(original,displacement);
        if(services.project_human_walk_position(human,destination)){
            const float unprojected=original[1]+displacement[1],maximum=seconds*4.5f;
            if(maximum<std::abs(static_cast<long double>(unprojected)-destination[1]))
                destination[1]=unprojected<=destination[1]?unprojected+maximum:unprojected-maximum;
        }
        if(move_word(0xec)<1){
            if(square(displacement)<1.e-16f){
                state.set(0x228,state.template get<std::uint32_t>(0x228)+static_cast<std::uint32_t>(elapsed));
                if(word(0x228)>10000000)state.set(0x228,10000000u);
            }else{
                const auto change=subtract(destination,original);const auto direction=normalized(change);
                const auto angle=std::acos(std::abs((static_cast<long double>(direction[2])+direction[0])*0.f+direction[1]));
                const float cosine=static_cast<float>(std::cos(1.5707963705062866L-angle));
                const float rate=static_cast<float>((1000.L/static_cast<float>(elapsed))*(static_cast<long double>(destination[1])-original[1]));
                apply_human_slope_motion(control,look,cosine,rate,direction[1]>0?1.f:0.f,destination,original,elapsed);
                if(publish_position){human.base().world_position(destination);movement.set(0x51,std::uint8_t{1});}
            }
            state.set(0x758,0.f);return false;
        }
    }
    using Hit=typename std::remove_reference_t<decltype(services.human_geometry_sweep())>::Hit;
    CollisionMotion<Hit> request;
    std::optional<Hit> primary,ground;
    bool supported=true,stationary=false,steep=false,secondary_steep=false,meaningful=false;
    HumanVector normal{};
    if(move_word(0xd0)!=-10000)posture=movement.template get<std::uint32_t>(0xd4);
    if(!physical.primary_count())return false;
    const auto original_position=human.base().world_position();
    auto offset=subtract(original_position,original_sphere);
    if(move_word(0xd0)!=-10000){
        const auto remaining=human_frame_signed_word(movement.template get<std::uint32_t>(0xd0)-static_cast<std::uint32_t>(elapsed));
        movement.set(0xd0,remaining<1?-10000:remaining);
    }
    if(move_word(0x64)!=-10000){
        const auto previous=move_word(0x64);
        movement.set(0x64,movement.template get<std::uint32_t>(0x64)-static_cast<std::uint32_t>(elapsed));
        const auto duration=move_word(0x64)<0?previous:elapsed;
        auto impulse=scale(movement.template get<HumanVector>(0x70),static_cast<float>(duration));
        impulse=scale(impulse,.002f);displacement=add(displacement,impulse);movement.set(0x10,displacement);
    }
    const auto probe_ground=[&](float margin,std::optional<Hit>& first,std::optional<Hit>& last){
        struct Restore {decltype(physical)& owner;~Restore(){owner.restore_query(false);}} restore{physical};
        physical.exclude_query(false);
        const auto sphere=physical.posture_spheres().at(0);
        first=services.human_geometry_ray(sphere,{{0,static_cast<float>(-static_cast<long double>(sphere[3])-margin),0,0}},0xc8002u,0x230000u);
        if(posture==2){
            const auto end=physical.posture_spheres().at(static_cast<std::size_t>(physical.primary_count()-1));
            last=services.human_geometry_ray(end,{{0,static_cast<float>(-static_cast<long double>(end[3])-margin),0,0}},0xc8002u,0x230000u);
        }
    };
    const auto publish_ground=[&](const std::optional<Hit>& hit){
        state.set(0x750,hit?services.human_geometry_material(*hit):0xffffffffu);
        if(hit)physical.ground_contact(*hit);else physical.clear_ground_contact();
    };
    bool solve=true;
    if(square(displacement)<1.e-16f){
        if(state.template get<std::uint32_t>(0xa7c)>2000&&word(0x228)>0){
            if(word(0x228)>10000000)state.set(0x228,10000000u);
            if(!state.template get<std::uint8_t>(0x761)&&move_real(0xf4)==0)stationary=true;
            else{
                std::optional<Hit> last;probe_ground(.1f,primary,last);stationary=primary.has_value()||last.has_value();
                if(stationary){
                    publish_ground(primary);
                    if(control.mode_flags()&1u)apply_human_ground_material(control,human,services,runtime,primary,elapsed);
                    if(!primary&&last){
                        publish_ground(last);
                        if(control.mode_flags()&1u)apply_human_ground_material(control,human,services,runtime,last,elapsed);
                    }
                    const auto& selected=primary?primary:last;
                    state.set(0x761,static_cast<std::uint8_t>((services.human_geometry_flags(*selected)>>6)&1u));
                }
            }
        }
        state.set(0x228,state.template get<std::uint32_t>(0x228)+static_cast<std::uint32_t>(elapsed));
        if(move_real(0xf4)<=epsilon){
            if(!stationary)movement.set(0xf4,9.81f);
            else solve=false;
        }else if(stationary)solve=false;
        if(!solve){
            state.set(0x762,static_cast<std::uint8_t>(primary&&(primary->normal[1]<-.3f||primary->normal[1]>.6f)));
            if(word(0x274)<1)control.tracks().motion_animation_rate(0,1.f);
        }
    }else if(move_real(0xf4)<=epsilon)movement.set(0xf4,4.f);
    if(solve){
        displacement[1]=static_cast<float>(displacement[1]-human_fall_displacement(elapsed,move_real(0xf4),move_real(0xf8)));
        movement.set(0x14,displacement[1]);
        query_human_ground_collision(control,look,human,services,request,primary,supported,offset,0.f);
        if(primary&&posture==2&&request.position[1]-request.centers[0][1]<static_cast<float>(elapsed)*-.0017){
            request.position=request.centers[0];request.position[1]=static_cast<float>(static_cast<long double>(elapsed)*.001+request.centers[0][1]);
            if(!request.contact_count)throw std::logic_error("Human ground impulse requires actual contact normal");
            control.tracks().motion_impulse(scale(request.contacts[request.contact_count-1]->normal,.4f),80);
        }
        steep=select_human_ground_contact(request,posture==2,normal,ground);
        state.set(0x762,static_cast<std::uint8_t>(steep));publish_ground(ground);
        if(control.mode_flags()&1u)apply_human_ground_contacts(control,human,services,runtime,request,elapsed);
        if(ground&&!state.template get<std::uint8_t>(0x761))
            state.set(0x761,static_cast<std::uint8_t>((services.human_geometry_flags(*ground)>>6)&1u));
        else if(state.template get<std::uint8_t>(0x761)){
            std::optional<Hit> first,last;probe_ground(.15f,first,last);state.set(0x761,std::uint8_t{0});
            if(first){state.set(0x761,static_cast<std::uint8_t>((services.human_geometry_flags(*first)>>6)&1u));secondary_steep=first->normal[1]<-.3f||first->normal[1]>.6f;}
            if(last&&!state.template get<std::uint8_t>(0x761))state.set(0x761,static_cast<std::uint8_t>((services.human_geometry_flags(*last)>>6)&1u));
        }
        meaningful=square(normal)>=1.e-16f;
        apply_human_ground_contact_slope(control,look,request,normal,steep,meaningful,primary.has_value(),original_sphere,elapsed);
    }
    if(move_word(0x64)<1){movement.set(0x64,std::int32_t{-10000});movement.set(0x70,0.f);movement.set(0x74,0.f);movement.set(0x78,0.f);}
    if(local&&!stationary){
        const bool wall=request.contact_count&&request.contacts[request.contact_count-1]->normal[1]<.26;
        const bool falling=!(primary&&supported&&!steep&&move_real(0xf4)>static_cast<long double>(9.81f)*1.5f);
        advance_human_walk_fall(control,look,human,services,primary,falling&&steep,elapsed,wall,true,runtime,publish); //00767530.
    }
    const auto clear_stall=[&]{movement.set(0x80,0u);movement.set(0x84,-10000.099609375f);};
    if(stationary)clear_stall();
    else{
        if(primary){
            request.position=add(request.position,offset);
            const auto delta=subtract(request.position,human.base().world_position());
            if(local){
                const auto animation=word(0x1d8);
                const bool interacting=look.state().template get<std::uint32_t>(0xb8)==5||
                    (animation>=106&&animation<=108)||animation==8||animation==14;
                (void)resolve_human_motion_contacts(control,look,human,services,request,interacting,runtime); //00788840.
            }
            if(std::abs(static_cast<long double>(delta[1]))*.5+std::abs(delta[0])+std::abs(delta[2])>=.001f&&publish_position){
                if(!state.template get<std::uint8_t>(0x761)){
                    human.base().world_position(request.position);state.set(0x228,0u);movement.set(0x51,std::uint8_t{1});
                }else if((std::abs(static_cast<long double>(delta[0]))>.0001||std::abs(static_cast<long double>(delta[2]))>.0001)&&
                        (std::abs(static_cast<long double>(displacement[0]))>.0001||std::abs(static_cast<long double>(displacement[2]))>.0001||(!steep&&!secondary_steep))){
                    human.base().world_position(request.position);state.set(0x228,0u);
                    control.publish_dynamic_ground_position();movement.set(0x51,std::uint8_t{1});
                }
            }
        }
        if(word(0x1d8)==17&&primary)runtime.human_walk_debug_beep(); //Original MessageBeep(-1).
        if((supported||move_real(0xf4)>9.81f)&&primary&&steep){
            finish_human_walk_fall(control,look,human,services,runtime,move_real(0xf4)>9.81f,publish); //00770B90.
            clear_stall();
        }else{
            state.set(0x228,0u);
            if((!supported||!steep)&&move_real(0x114)==-10000.099609375f){
                if(posture!=2&&request.contact_count&&(human.properties.field19c==1||(control.mode_flags()&7u)!=7u)&&
                        word(0x1d8)!=56&&(move_word(0x64)<1||request.contacts[request.contact_count-1]->normal[0]!=0||request.contacts[request.contact_count-1]->normal[2]!=0)){
                    if(move_word(0x80)<250||(move_real(0x84)!=-10000.099609375f&&
                            std::abs(static_cast<long double>(move_real(0x84))-human.base().world_position()[1])>.15000000596046448)){
                        if(!supported||move_real(0xf4)<=9.81f||!primary||steep){
                            auto direction=request.contacts[request.contact_count-1]->normal;direction[1]=0;
                            if(square(direction)<1.e-16f)movement.set(0x64,std::int32_t{-10000});
                            else{
                                direction=scale(normalized(direction),1.5f);movement.set(0x70,direction);
                                movement.set(0x64,!supported||physical.frame_globals().ground_normal[1]<=.58f?100u:0u);
                                const auto timer=move_word(0x80);
                                if(timer<1)movement.set(0x84,human.base().world_position()[1]);
                                else if(timer>250){movement.set(0x80,static_cast<std::uint32_t>(timer)-250u);movement.set(0x84,human.base().world_position()[1]);}
                                add_timer(0x80);
                            }
                        }else control.tracks().motion_impulse(scale(request.contacts[request.contact_count-1]->normal,.6666666865348816f),120);
                    }else if(supported&&move_real(0xf4)>9.81f&&primary&&!steep)
                        control.tracks().motion_impulse(scale(request.contacts[request.contact_count-1]->normal,.6666666865348816f),120);
                }
                if(word(0x1d8)==17&&state.template get<std::uint8_t>(0xa90))look.frame_movement_animation(45,publish,false);
                if(word(0x1d8)==17&&move_word(0x64)!=-10000&&human.activation.vector1b0[1]>=-.01f){
                    movement.set(0x64,std::int32_t{-10000});clear_stall();
                }
            }else{
                if(word(0x1d8)==17)look.frame_movement_animation(45,publish,false);
                clear_stall();
            }
        }
    }
    if(move_word(0x110)<0){add_timer(0x110);if(move_word(0x110)>=0)movement.set(0x110,0u);}
    else{
        if(!stationary)add_timer(0x94);
        bool check=move_word(0x94)>=151;
        if(human.properties.field19c==2){
            const auto flags=state.template get<std::uint32_t>(0x22c);
            check=static_cast<float>(move_word(0x94))>((flags&0x100u)?500.f:(flags&0x40u)?400.f:300.f);
        }
        if(check){
            const auto water=query_human_ground_water(control,look,human,services,runtime); //0074EA80.
            movement.set(0x94,0u);
            if(water>0){begin_human_ground_water(control,look,services,runtime,publish);movement.set(0x98,std::uint8_t{1});}
            else if(!water)movement.set(0x98,std::uint8_t{0});
        }
    }
    return primary.has_value();
}

} //namespace hd2::game
#endif
