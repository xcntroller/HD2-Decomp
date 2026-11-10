#ifndef HD2_GAME_HUMAN_SIGHT_RELATIONS_H
#define HD2_GAME_HUMAN_SIGHT_RELATIONS_H

#include <stdexcept>
#include <cstdint>
#include <initializer_list>

namespace hd2::game {
template<class Human>void set_human_sight_alignment(Human& human,float value,std::uint32_t mode){ //005A6250, direct setter, no clamp/game34 gate.
    human.activation.field16c=(mode&7u)==7u?value:0.f;
}

//005A60D0: actual offline/game34 alignment amount. Holder order and repeated
//entry lookup match native; zero descriptor44 resets, upper clamp100 only.
template<class Human,class EntryFlags>void adjust_human_sight_alignment(Human& human,float amount,
        std::uint32_t mode,bool game34,EntryFlags&& entry_flags){
    auto& value=human.activation.field16c;
    if((mode&7u)!=7u||!game34||human.field174){value=0;return;}
    auto* storage=human.base().storage();if(!storage||storage->single())throw std::runtime_error("Human alignment update requires actual multi-holder storage");
    for(unsigned index:{0u,4u,3u,2u,5u}){
        auto* holder=storage->holder(index);if(!holder)throw std::runtime_error("Human alignment update lost its actual holder");
        if(holder->entry()&&entry_flags(holder->entry())==0){value=0;return;}
    }
    value=static_cast<float>(static_cast<long double>(value)+amount);if(value>100.f)value=100.f;
}

//005A84F0. game34 and16C are actual containing-game/Human scalars; lazy
//getters preserve native short circuits. Entry flags read SAME item's descriptor44.
template<class Human,class Roster,class RemoteAI,class Scalar,class EntryFlags>
std::uint32_t human_sight_alignment(Human& human,const Roster& roster,bool game34,
        RemoteAI&& remote_ai,Scalar&& scalar16c,EntryFlags&& entry_flags) {
    if(!game34) {
        if(roster.contains(&human))return 0;
    }else {
        if(!remote_ai()) {
            if(roster.contains(&human)&&scalar16c(human)<=0.f)return 0; //Ordered <=; NaN does not satisfy.
        }else if(human.properties.field19c==1)return 0;
        auto* storage=human.base().storage();
        if(!storage||storage->single()||!storage->holder(4))throw std::runtime_error("Human alignment requires actual holder4");
        auto* holder=storage->holder(4);
        if(holder->entry())return entry_flags(holder->entry()); //Native repeats getter before descriptor44 read.
    }
    const auto kind=human.properties.field19c;
    return kind==2||kind==5?1u:kind==4?0u:2u;
}

//005A0AB0's OFFLINE mode7 branch. Roster is the SAME module11 control list;
//alignment must invoke actual005A84F0 when reached. Non7 uses005A09A0 and is
//a separate required receiver, not inferred from these offline relationships.
template<class Human,class Actor,class Roster,class Alignment>
bool offline_human_sight_friendly(Human& human,Actor& actor,const Roster& roster,Alignment&& alignment) {
    const auto type=actor.base().type();
    if(type==3) {
        auto* other=dynamic_cast<Human*>(&actor);
        if(!other)throw std::runtime_error("Human Sight relationship requires the actual Human family");
        if(roster.contains(&human)) {
            if(roster.contains(other))return true;
            if(other->properties.field19c==5)return true;
            if(other->properties.field19c==2)return false;
        }
        if(human.properties.flags1a4[2]&&roster.contains(other))return false;
        const auto first=human.properties.field19c,second=other->properties.field19c;
        const auto group0=[](std::uint32_t kind){return kind==3||kind==4||kind==1;};
        const auto group1=[](std::uint32_t kind){return kind==2||kind==5;};
        if((group0(first)&&group0(second))||(group1(first)&&group1(second)))return true;
        const auto a=alignment(human),b=alignment(*other); //Original calls BOTH before testing either result.
        if(a!=2&&b!=2)return a==b;
        return true;
    }
    return type==30&&human.properties.field19c!=1;
}
} //namespace hd2::game
#endif
