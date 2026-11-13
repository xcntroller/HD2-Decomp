#ifndef HD2_GAME_HUMAN_PLAYER_SHOT_DIRECTION_H
#define HD2_GAME_HUMAN_PLAYER_SHOT_DIRECTION_H

#include "game/human_ai_shot_direction.h"
#include "game/human_state_frame_timers.h"

namespace hd2::game {
namespace human_player_shot {
inline long double square_xyz(const HumanVector& v){
    return (static_cast<long double>(v[0])*v[0]+static_cast<long double>(v[1])*v[1])+static_cast<long double>(v[2])*v[2];
}
inline long double square_zyx(const HumanVector& v){
    return (static_cast<long double>(v[2])*v[2]+static_cast<long double>(v[1])*v[1])+static_cast<long double>(v[0])*v[0];
}
inline void cross(const HumanVector& a,const HumanVector& b,HumanVector& out){
    const auto x=static_cast<long double>(a[1])*b[2]-static_cast<long double>(a[2])*b[1];
    const auto y=static_cast<long double>(a[2])*b[0]-static_cast<long double>(a[0])*b[2];
    const auto z=static_cast<long double>(a[0])*b[1]-static_cast<long double>(a[1])*b[0];
    out[0]=static_cast<float>(x);out[1]=static_cast<float>(y);out[2]=static_cast<float>(z);
}
}

//00544D20, ECX direction, EDX first basis, one stack second-basis pointer.
//Native copies an unwritten temporary W. Only XYZ is reconstructed; retain
//the caller's defined W rather than claiming a homogeneous-coordinate writer.
inline void make_player_shot_basis(const HumanVector& direction,HumanVector& first,HumanVector& second){
    const auto y0=static_cast<long double>(direction[1])*0.f;
    first[0]=static_cast<float>(y0-direction[2]);
    first[1]=static_cast<float>(static_cast<long double>(direction[2])*0.f-static_cast<long double>(direction[0])*0.f);
    first[2]=static_cast<float>(static_cast<long double>(direction[0])-y0);
    normalize_human_ai_shot(first,static_cast<float>(human_player_shot::square_xyz(first)));
    human_player_shot::cross(first,direction,first);
    normalize_human_ai_shot(first,static_cast<float>(human_player_shot::square_xyz(first)));
    human_player_shot::cross(first,direction,second);
    normalize_human_ai_shot(second,static_cast<float>(human_player_shot::square_xyz(second)));
}

//00542B10 (absent frozen inventory), Player B0 virtualEC. The HUD also reads
//this metric; it is separate from the weapon payload's own spread scalar.
template<class Player>long double player_state_shot_spread(const Player& player){
    auto* human=player.human();if(!human)throw std::logic_error("Player spread requires its actual Human");
    const auto flags=player.words().template get<std::uint32_t>(0x50);
    long double value=(flags&4u)?.25f:(flags&2u)?.1f:.05f;
    if(flags&0xf8u){
        if(flags&0x1000u)value+=.1f;
        else if(flags&0x2000u)value+=.5f;
        else if(flags&0x4000u)value+=.75f;
    }
    return value+(100.L-human_frame_signed_word(human->properties.stats_ec[4]))*.01f;
}

//00545140. Payload virtual10 is read BEFORE the offline integrity branch.
//Queries borrow the SAME game3C/guard tables and LS3DF random stream. This
//never creates an integrity result, substitutes CRT rand or executes a shot.
template<class Item,class Queries>void spread_player_shot(Item& item,HumanVector& direction,Queries& queries){
    float spread=item.player_shot_spread();
    if((queries.mode_flags()&7u)==7u&&queries.player_mission_ordinal()>=9&&!queries.human_integrity_matches(100))spread=50.f;
    else if(spread==0)return;
    HumanVector first{},second{};make_player_shot_basis(direction,first,second);
    const auto a=(static_cast<long double>(queries.human_sound_random())-.5f)*2;
    for(unsigned axis=0;axis!=3;++axis)second[axis]=static_cast<float>(second[axis]*a);
    const auto b=(static_cast<long double>(queries.human_sound_random())-.5f)*2;
    const auto first_x=static_cast<long double>(first[0])*b;
    first[1]=static_cast<float>(first[1]*b);first[2]=static_cast<float>(first[2]*b);
    second[0]=static_cast<float>(first_x+second[0]);
    second[1]=static_cast<float>(static_cast<long double>(second[1])+first[1]);
    second[2]=static_cast<float>(static_cast<long double>(second[2])+first[2]);
    normalize_human_ai_shot(second,static_cast<float>(human_player_shot::square_zyx(second)));
    const auto amplitude=static_cast<long double>(spread)*.01f*queries.human_sound_random();
    //The original keeps X extended through the final squared length, while
    //Y/Z offsets round to FLOAT before their additions and publication.
    const auto x=static_cast<long double>(second[0])*amplitude+direction[0];
    direction[0]=static_cast<float>(x);
    direction[1]=static_cast<float>(static_cast<long double>(static_cast<float>(second[1]*amplitude))+direction[1]);
    direction[2]=static_cast<float>(static_cast<long double>(static_cast<float>(second[2]*amplitude))+direction[2]);
    const auto square=(static_cast<long double>(direction[2])*direction[2]+static_cast<long double>(direction[1])*direction[1])+x*x;
    constexpr float epsilon=9.99999993922529e-09f;
    if(std::abs(square-1)<epsilon)return;
    const auto stored=static_cast<float>(square);
    if(!(stored<epsilon)){
        const auto inverse=1.L/std::sqrt(static_cast<long double>(stored));
        direction[0]=static_cast<float>(x*inverse);
        direction[1]=static_cast<float>(direction[1]*inverse);direction[2]=static_cast<float>(direction[2]*inverse);
    }else{
        const auto axis=x!=0?0u:direction[2]!=0?2u:1u;
        direction[axis]=(axis==0?x:direction[axis])<0?-1.f:1.f;
    }
}

//005454D0. Each draw publishes one direction component before the next draw;
//failure of a later observation/draw must retain the preceding stores.
template<class Player,class Item,class Queries,class Publish>
void recoil_player_shot(Player& player,Item& item,HumanVector& direction,Queries& queries,Publish& publish){
    if((queries.mode_flags()&7u)==7u&&queries.player_recoil_disabled()&&queries.human_difficulty()!=4)return;
    const auto metric=player_state_shot_spread(player);float amount=static_cast<float>(metric);
    if(metric==0)return;
    switch(item.definition().kind()){
    case 2:amount=static_cast<float>(static_cast<long double>(amount)*.8f);break;
    case 3:amount=static_cast<float>(static_cast<long double>(amount)*.3f);break;
    case 4:
        if(item.definition().kind()==4&&item.player_secondary_recoil_exception())break;
        [[fallthrough]];
    case 5:amount=static_cast<float>(static_cast<long double>(amount)*.4f);break;
    case 12:amount=static_cast<float>(static_cast<long double>(amount)*.5f);break;
    default:break;
    }
    if((queries.mode_flags()&2u)&&queries.player_zoom_enabled())amount=static_cast<float>(static_cast<long double>(amount)*.5f);
    const auto* human=player.human();if(!human)throw std::logic_error("Player recoil requires its actual Human");
    const auto stamina=queries.player_shot_stamina(*human); //Actual005A62A0, AFTER zoom.
    amount=static_cast<float>((2.L-static_cast<long double>(stamina)*.01f)*amount);
    if((queries.mode_flags()&7u)==7u){
        const auto difficulty=queries.human_difficulty();
        if(difficulty==1)amount=static_cast<float>(static_cast<long double>(amount)*.3f);
        else if(difficulty==2)amount=static_cast<float>(static_cast<long double>(amount)*.8f);
    }
    direction[0]=static_cast<float>((static_cast<long double>(queries.human_sound_random())-.5f)*amount*.2f+direction[0]);
    publish(0,direction[0]);
    direction[1]=static_cast<float>(static_cast<long double>(queries.human_sound_random())*amount*.2f+direction[1]);
    publish(1,direction[1]);
    direction[2]=static_cast<float>((static_cast<long double>(queries.human_sound_random())-.5f)*amount*.2f+direction[2]);
    publish(2,direction[2]);
    normalize_human_ai_shot(direction,static_cast<float>(human_player_shot::square_xyz(direction)));
    for(unsigned axis=0;axis!=3;++axis)publish(axis,direction[axis]);
}
template<class Player,class Item,class Queries>
void recoil_player_shot(Player& player,Item& item,HumanVector& direction,Queries& queries){
    auto publish=[](unsigned,float){};recoil_player_shot(player,item,direction,queries,publish);
}
} //namespace hd2::game
#endif
