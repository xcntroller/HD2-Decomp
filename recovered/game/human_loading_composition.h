#ifndef HD2_GAME_HUMAN_LOADING_COMPOSITION_H
#define HD2_GAME_HUMAN_LOADING_COMPOSITION_H

#include "game/human_inline_loading_factory.h"
#include "game/human_loading_activation.h"
#include "game/human_loading_inventory.h"
#include "game/human_loading_state_factory.h"

namespace hd2::game {

//One containing lifetime for a real Human. Construct before0059F2C0 and retire
//before deleting the actor or any shared world/process resources. Publication
//of inventory/B0/inline/activation refers to these exact same owned instances.
//This source composition does not create a world, supply missing consumers or
//implement simulation/first-frame delivery.
template<class InlineFactory,class Inputs,class Contexts,class Store,class Speech,
         class Sight,class Sound,class Proximity,class Camera,class Players>
class HumanLoadingComposition final : public HumanLoadingLifetime {
public:
    using Frames=typename InlineFactory::Frames;using Human=typename InlineFactory::Human;
    using World=typename InlineFactory::World;using Random=typename InlineFactory::Random;
    using Inline=typename InlineFactory::Inline;using Presentation=typename InlineFactory::Presentation;
    using Effects=typename InlineFactory::Effects;using Items=typename InlineFactory::Items;
    using StateFactory=HumanLoadingStateFactory<Frames,Human,Inputs,Random,Contexts,World,Inline,Camera,Presentation>;
    using States=HumanStateLoadingOwner<Human,StateFactory>;
    using Inventory=HumanLoadingInventoryCommands<Frames,Human,Items,States,Inline,Players,Presentation,World>;
    using Activation=HumanLoadingActivation<Frames,Human,Inline,States,World,Store,Effects,Speech,Sight,Sound,Proximity,Camera>;
    HumanLoadingComposition(typename InlineFactory::Dependencies dependencies,Human& human,
            Inputs& inputs,MissionTableSet& second,MissionNavigation* navigation,
            HumanInputCallbacks& input_callbacks,HumanAIAction1Pool& actions,Contexts& contexts,
            Store& profiles,Speech& speech,Sight& sight,Sound& sound,Proximity& proximity,Camera& camera,Players& players)
        :frames_(dependencies.frames),human_(human),factory_(dependencies),control_(factory_),
         state_factory_(dependencies.frames,inputs,second,navigation,input_callbacks,actions,dependencies.random,
             contexts,dependencies.world,control_,camera,dependencies.presentation),states_(human,state_factory_),
         inventory_(dependencies.frames,human,dependencies.items,states_,control_,players,dependencies.presentation,dependencies.world),
         activation_(dependencies.frames,human,control_,states_,dependencies.world,profiles,dependencies.effects,speech,sight,sound,proximity,camera){
        //Activation constructed/published its borrower only after inventory and
        //state ownership exist. Route the actor through this containing guard.
        human_.loading_lifetime=this;
    }
    ~HumanLoadingComposition()override{
        frontend::CleanupErrors errors;
        if(started_){
            if(!(retired_&1u))errors.attempt([&]{deactivate(false);});
            if(!(retired_&2u))errors.attempt([&]{deactivate(true);});
        }
        //Retain inventory while any failed B0 close is retired. Inline clear
        //follows B0 even when a real endpoint throws, before member destruction.
        errors.attempt([&]{states_.retire();});control_.clear();
        if(human_.loading_lifetime==this)human_.loading_lifetime=&activation_;
        if(errors.failed())try{errors.rethrow();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}
    }
    HumanLoadingComposition(const HumanLoadingComposition&)=delete;
    HumanLoadingComposition& operator=(const HumanLoadingComposition&)=delete;
    void activate(bool authority)override{
        if(human_.loading_lifetime!=this)throw std::logic_error("Human loading lifetime binding changed before activation");
        started_=true;retired_=0;activation_.activate(authority);
    }
    void deactivate(bool authority)override{
        activation_.deactivate(authority);retired_|=authority?2u:1u;
        if(retired_==3u)started_=false;
    }
    Inline& control(){return control_;}States& states(){return states_;}Inventory& inventory(){return inventory_;}
    StateFactory& state_factory(){return state_factory_;}
private:
    Frames& frames_;Human& human_;
    InlineFactory factory_;Inline control_;StateFactory state_factory_;States states_;Inventory inventory_;Activation activation_;
    unsigned retired_=0;bool started_=false;
};

} //namespace hd2::game
#endif
