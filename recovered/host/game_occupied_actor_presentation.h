#ifndef HD2_HOST_GAME_OCCUPIED_ACTOR_PRESENTATION_H
#define HD2_HOST_GAME_OCCUPIED_ACTOR_PRESENTATION_H
#include "game/actor_seat_presentation.h"
#include "game/world_loading_dependency_link.h"
#include "game/car_loading_activation.h"
#include "game/static_weapon_loading.h"
#include <type_traits>
namespace hd2::game {template<class Frames,class Services,class Items>class MountedWeaponControlLoading;}
namespace hd2::host {
//UI virtual8C/98/9C/A0/B4/C4/C8/CC over the SAME World/Scene2/Items owners.
//No occupied Actor, control, item, marker or frame is retained by this facade.
//Bind after the dependency aggregate exists; unbind AFTER FPV/Actors retire.
template<class Dependencies,class Frames,class ActorWorld>
class GameOccupiedActorPresentation final:public game::ActorSeatPresentation<game::SceneActor<Frames>> {
public:
    using Actor=game::SceneActor<Frames>;using Car=game::CarSceneActor<Frames>;using Weapon=game::StaticWeaponSceneActor<Frames>;
    using Items=typename ActorWorld::Items;
    void bind_dependencies(Dependencies& dependencies){dependencies_.bind(dependencies);}
    void unbind_dependencies(Dependencies& dependencies)noexcept{dependencies_.unbind(dependencies);}
    std::uint32_t kind(Actor& actor)override{validate(actor);return actor.base().type();}
    std::uint32_t flags(Actor& actor,std::int32_t seat)override{
        validate(actor);
        if(actor.base().type()==4){ //004D9FC0: actual loaded seat24 flags.
            const auto& seats=car_loading(car(actor)).seats().seats();
            return valid(seat,seats.size())?seats[static_cast<std::size_t>(seat)].flags:0;
        }
        if(actor.base().type()==21){ //00604A10: FIRST case-sensitive SEAT substring.
            const auto& seats=weapon_loading(weapon(actor)).seats();if(!valid(seat,seats.size()))return 0;
            const auto text=d().scene.frames().actor_user_data(seats[static_cast<std::size_t>(seat)].frame);
            return game::actor_marker_seat_flags(text);
        }
        return actor_receiver(actor).flags(actor,seat);
    }
    bool has_control(Actor& actor,std::int32_t seat)override{
        validate(actor);
        if(actor.base().type()==4)return bool(car_slot(car(actor),seat)); //004DA020.
        if(actor.base().type()==21)return seat==0; //00604D20, independent of seat count/failed flag.
        return actor_receiver(actor).has_control(actor,seat);
    }
    const frontend::PreMissionItem* definition(Actor& actor,std::int32_t seat)override{
        validate(actor);
        if(actor.base().type()==4){auto& slot=car_slot(car(actor),seat);if(!slot)return nullptr; //004DA1D0.
            return read_car_control<const frontend::PreMissionItem*>(slot,[&](auto& loading,auto&){
                return control_definition(loading.failed(),loading.weapon_item());
            });
        }
        if(actor.base().type()==21){ //00604DA0→004C57B0, seat ignored.
            auto& loading=weapon_loading(weapon(actor));return control_definition(loading.presentation_failed(),loading.weapon_item());
        }
        return actor_receiver(actor).definition(actor,seat);
    }
    std::uint32_t current(Actor& actor,std::int32_t seat)override{
        validate(actor);
        if(actor.base().type()==4){auto& slot=car_slot(car(actor),seat);if(!slot)return 0; //004DA040.
            return read_car_control<std::uint32_t>(slot,[&](auto& loading,auto&){return item_amount(loading.weapon_item());});
        }
        if(actor.base().type()==21)return item_amount(weapon_loading(weapon(actor)).weapon_item()); //00604D30→004C6550.
        return actor_receiver(actor).current(actor,seat);
    }
    std::uint32_t reserve(Actor& actor,std::int32_t seat)override{ //00611140; re-query98/B4 in original order.
        validate(actor);
        if(actor.base().type()!=4&&actor.base().type()!=21)return actor_receiver(actor).reserve(actor,seat);
        if(!has_control(actor,seat))return 0;
        const auto* item=definition(actor,seat);if(!item)throw std::logic_error("Seat ammunition requires its actual weapon definition");
        return game::actor_seat_ammunition(actor,d().world_actors.items(),item->ammunition());
    }
    void view(Actor& actor,std::int32_t seat,bool enabled)override{
        validate(actor);
        if(actor.base().type()==4){car_loading(car(actor)).seats().view(checked_car_index(car(actor),seat),enabled);return;} //004DA090.
        if(actor.base().type()==21){weapon_loading(weapon(actor)).presentation_view(enabled);return;} //00604DB0→004C6D90.
        actor_receiver(actor).view(actor,seat,enabled);
    }
    std::optional<game::HumanHudProgress> progress(Actor& actor,std::int32_t seat)override{
        validate(actor);
        if(actor.base().type()==4){auto& slot=car_slot(car(actor),seat);if(!slot)return std::nullopt; //004DA270.
            return read_car_control<std::optional<game::HumanHudProgress>>(slot,[](auto&,auto& state){return game::mounted_weapon_hud_progress(state);});
        }
        if(actor.base().type()==21){auto& value=weapon(actor);(void)weapon_loading(value);return game::mounted_weapon_hud_progress(value.control);} //00604E10.
        return actor_receiver(actor).progress(actor,seat);
    }
    bool hides_cursor(Actor& actor,std::int32_t seat)override{
        validate(actor);typename Frames::Frame marker{};
        if(actor.base().type()==4){ //004DA2A0: actual geometry seat2C marker.
            const auto& seats=car(actor).geometry->auxiliary().seats();if(!valid(seat,seats.size()))return false;
            marker=seats[static_cast<std::size_t>(seat)].frame;
        }else if(actor.base().type()==21){ //00604E30: actual loaded seat08 marker.
            const auto& seats=weapon_loading(weapon(actor)).seats();if(!valid(seat,seats.size()))return false;
            marker=seats[static_cast<std::size_t>(seat)].frame;
        }else return actor_receiver(actor).hides_cursor(actor,seat);
        if(!marker)throw std::logic_error("Seat cursor visibility lost its authored marker");
        return game::actor_marker_hides_cursor(d().scene.frames().actor_user_data(marker));
    }
private:
    Dependencies& d(){return dependencies_.get();}
    static bool valid(std::int32_t seat,std::size_t size){return seat>=0&&static_cast<std::size_t>(seat)<size;}
    void validate(Actor& actor){
        game::require_actor_seat_identity(d().scene.frames(),actor);
    }
    static Car& car(Actor& actor){auto* value=dynamic_cast<Car*>(&actor);if(!value||!value->geometry)throw std::logic_error("Seat presentation requires actual Car geometry");return *value;}
    static Weapon& weapon(Actor& actor){auto* value=dynamic_cast<Weapon*>(&actor);if(!value)throw std::logic_error("Seat presentation requires actual StaticWeapon owner");return *value;}
    auto& car_loading(Car& actor){
        using Queries=decltype(d().scene.car_queries());
        using Loading=game::CarLoadingActivation<Frames,ActorWorld,typename ActorWorld::Collisions,Queries>;
        auto* loading=dynamic_cast<Loading*>(actor.loading.get());if(!loading)throw std::logic_error("Seat presentation requires actual Car loading owner");return *loading;
    }
    auto& weapon_loading(Weapon& actor){
        using Loading=game::StaticWeaponLoading<Frames,ActorWorld,typename ActorWorld::Collisions,Items>;
        auto* loading=dynamic_cast<Loading*>(actor.loading.get());if(!loading)throw std::logic_error("Seat presentation requires actual StaticWeapon loading owner");return *loading;
    }
    static std::size_t checked_car_index(Car& actor,std::int32_t seat){
        if(!valid(seat,actor.fields24b4.size()))throw std::out_of_range("Seat presentation Car control index");return static_cast<std::size_t>(seat);
    }
    static auto& car_slot(Car& actor,std::int32_t seat){return actor.fields24b4.at(checked_car_index(actor,seat));}
    template<class Result,class Slot,class Read>Result read_car_control(Slot& slot,Read&& read){
        if constexpr(std::is_integral_v<Slot>){
            //This older implementation has only opaque tokens. They cannot be
            //cast to an owner. New master supplies the real mounted lifetime.
            throw std::runtime_error("Car seat presentation requires the real mounted control owner from P22-HUM");
        }else{
            using Loading=game::MountedWeaponControlLoading<Frames,ActorWorld,Items>;
            auto* loading=dynamic_cast<Loading*>(slot->loading.get());
            if(!loading)throw std::logic_error("Car seat presentation lost its actual mounted control lifetime");
            return read(*loading,slot->state);
        }
    }
    const frontend::PreMissionItem* control_definition(bool failed,typename Frames::Entry entry){
        return game::mounted_weapon_hud_definition(d().world_actors.items(),failed,entry);
    }
    std::uint32_t item_amount(typename Frames::Entry entry){
        return game::mounted_weapon_hud_amount(d().world_actors.items(),entry);
    }
    static game::ActorSeatPresentation<Actor>& actor_receiver(Actor& actor){
        //Future Actor20/34 OWN receivers live on that actual Actor, not another
        //World registry. Cross-cast only after validate() above; no cached loan.
        auto* receiver=dynamic_cast<game::ActorSeatPresentation<Actor>*>(&actor);
        if(!receiver)throw std::runtime_error("Seat presentation requires its actual Actor family owner/virtual receivers (including kinds20/34)");
        return *receiver;
    }
    game::WorldLoadingDependencyLink<Dependencies> dependencies_;
};
} //namespace hd2::host
#endif
