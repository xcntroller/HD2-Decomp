#ifndef HD2_GAME_HUMAN_INLINE_LOADING_FACTORY_H
#define HD2_GAME_HUMAN_INLINE_LOADING_FACTORY_H

#include "game/human_control_loading.h"
#include "game/human_equipment_loading.h"
#include "game/human_inline_loading.h"

namespace hd2::game {

//Concrete source factory for005AB330: E0/DD0/230 are constructed by the
//existing recovered owners. All external resources are borrowed from the SAME
//world; no per-stage successful callback or replacement control is supplied.
template<class FrameBackend,class HumanActor,class CollisionBuckets,class AudioOwner,
         class RandomSource,class AnimationEvents,class EffectsOwner,class PresentationOwner,
         class WorldServices,class ItemOwner,class AnimationResources,class CollisionResources,class Sweep>
class HumanInlineLoadingFactory {
public:
    using Frames=FrameBackend;using Human=HumanActor;using Collisions=CollisionBuckets;
    using Audio=AudioOwner;using Random=RandomSource;using Callbacks=AnimationEvents;
    using Effects=EffectsOwner;using Presentation=PresentationOwner;using World=WorldServices;
    using Items=ItemOwner;using Animations=AnimationResources;
    using Control=HumanControlLoading<Frames,Human,Collisions,Audio,Random,Callbacks,Effects,Presentation,World>;
    using Look=HumanLookControl<Control>;
    using EquipmentServices=HumanEquipmentServices<Human,Control,Look,Items,World>;
    using Equipment=HumanEquipmentLoading<Frames,Items,Animations,EquipmentServices,typename Control::Shadow>;
    using Inline=HumanInlineLoading<Human,Control,Equipment,HumanInlineLoadingFactory>;
    using Descriptor=typename Control::Physical::Descriptor;
    struct Dependencies {
        Frames& frames;Collisions& collisions;DynamicCollisionExclusions<Descriptor>& exclusions;
        const HumanMorphTables& morph;HumanPhysicsGlobals& physics_globals;
        Audio& audio;Random& random;Callbacks& callbacks;Effects& effects;Presentation& presentation;
        World& world;Items& items;Animations& animations;const frontend::PreMissionItemCatalogue& catalogue;
        CollisionResources& resources;Sweep& sweep;const CollisionTree<typename Frames::Frame>& tree;
    };
    explicit HumanInlineLoadingFactory(Dependencies dependencies):dependencies_(dependencies){}
    HumanInlineLoadingFactory(const HumanInlineLoadingFactory&)=delete;
    HumanInlineLoadingFactory& operator=(const HumanInlineLoadingFactory&)=delete;
    std::unique_ptr<Control> make_human_control(){
        auto& d=dependencies_;
        return std::make_unique<Control>(d.frames,d.collisions,d.exclusions,d.morph,d.physics_globals,
            d.audio,d.random,d.callbacks,d.effects,d.presentation,d.world);
    }
    std::unique_ptr<Equipment> make_human_equipment(Human& human,Control& control,Look& look){
        auto& d=dependencies_;
        //Inline98 has already retired any previous230 before reinitialization.
        //The service borrows this exact DD0/E0 and outlives the returned230.
        if(equipment_services_&&equipment_services_->visibility_bound())
            throw std::logic_error("Retire Human230 before replacing its DD0/E0 services");
        equipment_services_=std::make_unique<EquipmentServices>(human,control,look,d.items,d.world);
        return std::make_unique<Equipment>(d.frames,d.items,d.animations,*equipment_services_,control.shadow(),d.catalogue,human);
    }
    auto& human_collision_resources(){return dependencies_.resources;}
    auto& human_collision_sweep(){return dependencies_.sweep;}
    const auto* human_collision_tree(){return &dependencies_.tree;}
    std::uint32_t mode_flags()const{return dependencies_.world.mode_flags();}
    std::int32_t human_current_player_id(){return dependencies_.world.human_current_player_id();}
private:
    Dependencies dependencies_;std::unique_ptr<EquipmentServices> equipment_services_;
};

} //namespace hd2::game
#endif
