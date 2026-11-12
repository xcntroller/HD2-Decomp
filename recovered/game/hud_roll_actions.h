#ifndef HD2_GAME_HUD_ROLL_ACTIONS_H
#define HD2_GAME_HUD_ROLL_ACTIONS_H
#include "game/hud_signal_actions.h"
#include "game/hud_roll_policy.h"
namespace hd2::game {
//0080F720 kind2 request. Identity is copied; no borrowed actor is queued by UI.
struct HudRollCommand{std::uint32_t action;std::int32_t identity;std::uint32_t parameter=0;};
template<class Roll,class Queries>void dispatch_hud_roll(Roll& roll,Queries& queries){ //004B2830, before common target-clear/hide tail.
    if(roll.selected84<0||roll.selected84>9)throw std::out_of_range("Roll activation requires actual selected row");
    if(roll.selected84==9)return; //Exit does not query selected Human.
    auto* human=queries.selected_human();if(!human)throw std::logic_error("Roll action requires actual selected Human");
    const auto action=roll.rows[static_cast<std::size_t>(roll.selected84)].action;
    const auto target=[&]{if(!roll.field88)throw std::logic_error("Roll action reached native null target");return queries.resolve(roll.field88);};
    const auto submit=[&](std::uint32_t value,auto& actor,std::uint32_t parameter=0){queries.submit_roll(*human,HudRollCommand{value,queries.actor_id(actor),parameter});};
    switch(action){
    case 0:case 1:case 3:case 4:case 14:case 15:case 16:case 19:case 27:case 28:case 34:submit(static_cast<std::uint32_t>(action),*target());break;
    case 2:case 38:{auto& actor=*target();const auto kind=queries.kind(actor);submit(static_cast<std::uint32_t>(action),actor,kind);break;}
    case 5:queries.submit_roll(*human,HudRollCommand{5,-1,0});break;
    case 6:{auto& actor=*target();switch(queries.kind(actor)){
        case 3:submit(6,actor);break;
        case 4:case 20:case 21:if(auto* occupant=queries.occupant(actor,static_cast<std::size_t>(roll.selected84)))submit(6,*occupant);break;
        case 9:queries.pickup_roll_supply(*human,actor,roll.selected84);break;
        case 11:queries.pickup_roll_item(*human,actor);break;
        default:break;
        }break;}
    case 7:queries.show_roll_inventory(*target());break;
    case 8:case 9:case 10:case 11:case 12:case 13:
        if(!roll.field88)if(auto* attached=queries.attachment(*human))if(hud_roll_vehicle_kind(queries.kind(*attached)))roll.field88=queries.identity(*attached);
        submit(3,*target(),static_cast<std::uint32_t>(roll.rows[static_cast<std::size_t>(roll.selected84)].action)-8u);break;
    case 29:{HudSignalCommand<typename Queries::Actor> request;request.command=53;request.action=15;request.target=human;
        request.sender=roll.field88?queries.resolve(roll.field88):nullptr;queries.submit_signal(*human,request);break;}
    default:break;
    }
}
} //namespace hd2::game
#endif
