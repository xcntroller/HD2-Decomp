#ifndef HD2_GAME_HUMAN_PLAYERS_SUBGROUP_H
#define HD2_GAME_HUMAN_PLAYERS_SUBGROUP_H
#include "game/players_module_loading.h"
#include "game/human_formation.h"
#include <map>
namespace hd2::game {
//004608A0/004603C0/00460850/00460980/00460A10, actual module11 subgroup.
//The source-only Context registration guards native borrowed Humans even when
//00460B30 drops its pointer vector; it reuses the actual actor-reference owner.
template<class Frames,class Human,class Runtime>
class HumanPlayersSubgroup final:public PlayersSubgroup<Human>,public Frames::Context {
public:
    using Formation=std::remove_reference_t<decltype(*std::declval<Human&>().formation)>;
    using Actor=typename Frames::Actor;
    HumanPlayersSubgroup(Frames& frames,MissionNavigation& navigation,Runtime& runtime,Human* leader,std::int32_t identity)
        :frames_(frames),runtime_(runtime),roster_(identity),formation_(frames,navigation){
        formation_.subgroup_policy();frames_.register_context(*this);registered_=true;
        try{
            (void)add(leader);
            //004608A0 selects the leader even when004603C0 rejects its B0.
            //Track that separate formation borrower for late ActorBase removal.
            if(leader)references_.emplace(&leader->base(),leader);
            (void)formation_.select_leader(leader);formation_.select_subgroup_mode();
        }
        catch(...){close();throw;}
    }
    ~HumanPlayersSubgroup()override{close();}
    Human* leader()const override{return formation_.leader();}
    bool contains(Human* human)const override{return roster_.contains(human);}
    bool add(Human* human)override{ //004603C0: Script kind FIRST, then roster, formation, fresh B0 binding.
        if(!human||!human->loading_state||human->loading_state->words().template get<std::uint32_t>(0x48)!=5)return false;
        if(!roster_.append(human))return false;
        references_.emplace(&human->base(),human);
        (void)formation_.add(human);runtime_.bind_players_subgroup_formation(*human,&formation_);return true;
    }
    bool remove(Human* human,std::int32_t* index=nullptr)override{ //00460850.
        if(!roster_.remove(human,index))return false;
        references_.erase(&human->base());(void)formation_.remove(human);
        if(human->loading_state&&human->loading_state->words().template get<std::uint32_t>(0x48)==5)
            runtime_.bind_players_subgroup_formation(*human,static_cast<Formation*>(nullptr));
        return true;
    }
    void mode(std::int32_t value)override{formation_.select_mode(value);}
    void scale(const HumanVector& value)override{formation_.scale(value);}
    void tick(std::uint32_t elapsed)override{formation_.advance(elapsed,runtime_);} //00460A10→0059BA70, no empty receiver.
    void destroy(bool)override{close();} //00460960; containing unique owner supplies the allocation retirement.
    const auto& roster()const{return roster_;}Formation& formation(){return formation_;}
    void remove_actor(Actor* actor)noexcept override{ //Source safety: ActorBase's late destructor cannot inspect dead Human members/B0.
        const auto found=references_.find(actor);if(found==references_.end())return;
        auto* human=found->second;references_.erase(found);(void)roster_.remove(human);
        try{(void)formation_.remove(human);}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
    }
    void close()noexcept{
        if(closed_)return;
        closed_=true;
        for(auto n=roster_.size();n;){auto* human=roster_.at(static_cast<std::int32_t>(--n));
            try{(void)remove(human);}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
        }
        formation_.close();references_.clear();roster_.clear();
        if(registered_){frames_.unregister_context(*this);registered_=false;}
    }
private:
    Frames& frames_;Runtime& runtime_;HumanRoster<Human> roster_;Formation formation_;
    std::map<Actor*,Human*> references_;bool registered_=false,closed_=false;
};
} //namespace hd2::game
#endif
