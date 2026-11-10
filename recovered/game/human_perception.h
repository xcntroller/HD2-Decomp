#ifndef HD2_GAME_HUMAN_PERCEPTION_H
#define HD2_GAME_HUMAN_PERCEPTION_H

#include <array>
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <utility>
#include <cstddef>

namespace hd2::game {

//Human268/278/288/298 are four lists of BORROWED actors.2AC is a separate
//pooled-record list and is deliberately not represented by this owner.
template<class Actor>class HumanPerceptionLists {
public:
    enum Group:unsigned {hostile=0,friendly=1,dead=2,neutral=3};
    template<class Notify>void add_hostile(Actor actor,Notify&& notify){if(append(hostile,actor))notify(actor);} //005A4A10→B0+98.
    template<class Notify>void add_dead(Actor actor,Notify&& notify){if(append(dead,actor))notify(actor);} //005A4840→B0+A0.
    void add_friendly(Actor actor){(void)append(friendly,actor);} //005A4BE0.
    void add_neutral(Actor actor){(void)append(neutral,actor);} //005A4DA0.
    template<class Notify>void remove_hostile(Actor actor,Notify&& notify){remove(hostile,actor,std::forward<Notify>(notify));} //005A5020→B0+9C.
    void remove_friendly(Actor actor){remove(friendly,actor,[](Actor){});} //005A4FC0, no native callback.
    void remove_neutral(Actor actor){remove(neutral,actor,[](Actor){});} //005A4F60, no native callback.
    void clear()noexcept{for(auto& list:lists_)list.clear();} //0059FA60, retain allocation; no per-entry release.
    auto& lists(){return lists_;}const auto& lists()const{return lists_;}
private:
    bool append(unsigned group,Actor actor) {
        auto& list=lists_[group];
        for(auto n=list.size();n;)if(list[--n]==actor)return false;
        list.push_back(actor);return true;
    }
    template<class Notify>void remove(unsigned group,Actor actor,Notify&& notify) {
        auto& list=lists_[group];
        for(auto n=list.size();n;)if(list[--n]==actor){list.erase(list.begin()+static_cast<std::ptrdiff_t>(n));notify(actor);}
    }
    std::array<std::vector<Actor>,4> lists_;
};

//005A5250: source notification routing, not an AI event-queue substitute.
//Bindings must resolve the SAME live B0, friendship and actual B0+98/9C/A0
//and Players0045D4A0 consumers. Sight owns its typed target records.
template<class Human,class Targets,class Bindings>
void notify_human_sight(Human& human,bool lost,std::uint32_t index,Targets& targets,Bindings& bindings) {
    auto* state=bindings.human_sight_state(human);
    if(!state||(!lost&&(state->words().template get<std::uint32_t>(0x50)&0x40000000u)))return;
    auto* target=targets.target(static_cast<std::int32_t>(index));
    if(!target)throw std::runtime_error("Human sight notification requires a live actual target");
    if(target->kind==0) {
        auto* other=dynamic_cast<Human*>(target->actor);
        if(!other)throw std::runtime_error("Human sight target has a different actor family");
        if(other==&human)return;
        if(lost) {
            human.perception.remove_hostile(other,[&](auto*){bindings.human_sight_enemy_lost(human,*other);});
            human.perception.remove_neutral(other);human.perception.remove_friendly(other);
        }else {
            const auto bits=other->properties.stats_ec[0];
            const auto health=bits&0x80000000u?static_cast<std::int64_t>(bits)-0x100000000ll:static_cast<std::int64_t>(bits);
            if(health<1)human.perception.add_dead(other,[&](auto*){bindings.human_sight_dead_acquired(human,*other);});
            else if(!bindings.human_sight_friendly(human,*other))
                human.perception.add_hostile(other,[&](auto*){bindings.human_sight_enemy_acquired(human,*other);});
            else {
                const auto a=human.properties.field19c,b=other->properties.field19c;
                //005A0A50 repeats005A0AB0 only for the1/2 pair.
                const bool special=((a==1&&b==2)||(a==2&&b==1))&&bindings.human_sight_friendly(human,*other);
                if(special)human.perception.add_neutral(other);else human.perception.add_friendly(other);
            }
        }
    }else if(target->kind==1) {
        if(human.properties.field19c!=1&&state->words().template get<std::uint32_t>(0x48)!=3)return;
        //Native has no lost check for Dog targets.0045D4A0 runs in either case.
        if(bindings.sight_dog_health(*target->actor)>0)bindings.player_sight_dog(human,*target->actor);
    }
}

//005A50A0: collect indices before any callbacks mutate actual perception lists.
//Reverse hostile, then reverse neutral, then reverse queued loss+acquisition.
template<class Human,class Sight,class Bindings>void refresh_human_perception(Human& human,Sight& sight,Bindings& bindings){
    const auto alive=[](const auto& actor){return !(actor.properties.stats_ec[0]&0x80000000u)&&actor.properties.stats_ec[0]!=0;};
    if(!alive(human))return;
    const auto actual=[](auto actor)->Human&{auto* result=dynamic_cast<Human*>(actor);if(!result)throw std::runtime_error("Human perception list lost its actual Human actor");return *result;};
    std::vector<std::uint32_t> queued;
    auto& hostile=human.perception.lists()[0];
    for(auto n=hostile.size();n;){auto& other=actual(hostile.at(--n));if(!alive(other))queued.push_back(other.field25c);}
    auto& neutral=human.perception.lists()[3];
    for(auto n=neutral.size();n;){
        auto& other=actual(neutral.at(--n));
        if(!alive(other)||bindings.human_sight_hostile(&human,&other))queued.push_back(other.field25c);
        else{
            bool visible=false;(void)sight.human_relation_feedback(static_cast<std::int32_t>(human.field258),static_cast<std::int32_t>(other.field25c),nullptr,&visible);
            if(visible)bindings.adjust_human_sight_alignment(human,-1.f);
        }
    }
    for(auto n=queued.size();n;){const auto index=queued[--n];bindings.human_sight_changed(&human,true,index);bindings.human_sight_changed(&human,false,index);}
}
} //namespace hd2::game
#endif
