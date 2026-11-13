#ifndef HD2_GAME_HUD_ROLL_POLICY_H
#define HD2_GAME_HUD_ROLL_POLICY_H
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <stdexcept>
namespace hd2::game {
inline bool hud_roll_vehicle_kind(std::uint32_t kind){return kind==4||kind==20||kind==21||kind==34;}
inline std::int32_t hud_roll_identity_caption(std::int32_t identity){const auto bits=std::uint32_t{0}-10u-static_cast<std::uint32_t>(identity);std::int32_t value;std::memcpy(&value,&bits,4);return value;}
//004B0F30. Queries borrow the selected Human, actual B0/DD0/Items/roster,
//occupied actors and game configuration. No UI copy of combat/action state.
template<class Queries,class Actor>bool hud_roll_ordinary_eligible(Queries& q,Actor* target,float distance){
    auto* human=q.selected_human();if(!human||q.player_mode(*human)!=0||q.action_active(*human))return false;
    if(q.focus(*human))return true;
    if(auto* attached=q.attachment(*human))if(hud_roll_vehicle_kind(q.kind(*attached)))return true;
    if(!target||(q.movement_animation(*human)!=0&&q.kind(*target)!=6))return false;
    if(q.weapon_pending(*human))return false;
    if(!(distance<=1.7999999523162842f))return false;
    switch(q.kind(*target)){
    case 2:case 6:case 7:case 11:case 13:case 15:case 20:case 22:case 23:case 35:return true;
    case 3:
        if(q.offline()){
            if(q.health(*target)<1||q.unavailable(*target))return true;
            if(q.in_roster(*target)){return q.movement_animation(*target)==0&&!q.navigation_floor(*target)&&!q.attachment(*target);}
            return q.human_kind(*target)==2&&q.suppressed(*target);
        }
        if(q.hud_style()==0)return false;
        if(q.special_game_mode()&&q.human_kind(*target)!=1){
            if(q.health(*target)<1||q.unavailable(*target)||q.suppressed(*target))return true;
        }
        {const auto first=q.team(*target);const auto second=q.team(*human);return first==second;}
    case 4:return q.car_interaction_available(*target)&&!q.car_destroyed(*target);
    case 9:return q.supply_state(*target)==0&&(q.supply_flags(*target)&1u);
    case 21:return q.static_weapon_ready(*target);
    default:return false;
    }
}
//004B11F0. Command eligibility is offline only and uses PREVIOUS30, not
//current2C or selected28. Its cached target88 remains a borrower.
template<class Queries,class Actor>bool hud_roll_command_eligible(Queries& q,Actor* target){
    if(!q.offline()||!target)return false;auto* human=q.previous_human();if(!human)return false;
    switch(q.kind(*target)){
    case 2:case 6:case 7:case 8:case 9:case 11:case 13:case 15:case 20:case 22:case 23:case 30:case 35:return true;
    case 3:
        if(target==human){if(q.focus(*target))return true;if(auto* attached=q.attachment(*target))return hud_roll_vehicle_kind(q.kind(*attached));return false;}
        {bool admitted=!q.in_roster(*target);
        if(!admitted)admitted=(q.movement_animation(*target)==0&&!q.navigation_floor(*target))||q.attachment(*target);
        return admitted&&(q.human_kind(*target)!=3||q.health(*target)<1);}
    case 4:return !q.car_destroyed(*target);
    case 21:return q.static_weapon_ready(*target);
    case 25:return !q.animation_active(*target)&&!q.animation_excludes_target(*target);
    default:return false;
    }
}
//004B13C0 ordinary rows. Row action50 means disabled, -1 means absent.
//Actor/Human methods below are read queries; no gameplay command is executed.
template<class Dialogs,class Queries>void populate_hud_roll_ordinary(Dialogs& dialogs,Queries& q){
    for(unsigned n=0;n!=10;++n)dialogs.set_roll_row(n,-1,-1);
    auto* human=q.selected_human();if(!human)return;
    const auto put=[&](unsigned n,std::int32_t caption,std::int32_t action){dialogs.set_roll_row(n,caption,action);};
    const auto finish=[&]{put(9,0x1c6a,0x33);};
    if(auto* focus=q.focus(*human)){q.set_target(dialogs,q.identity(*focus));put(0,0x1b70,5);finish();return;}
    if(auto* attached=q.attachment(*human);attached&&hud_roll_vehicle_kind(q.kind(*attached))){
        q.set_target(dialogs,q.identity(*attached));const auto kind=q.kind(*attached);
        if(kind==21){put(0,0x1b70,5);finish();return;}
        if(kind==4||kind==20){
            put(0,0x1b70,5);
            for(std::int32_t seat=0;seat<q.occupant_count(*attached);++seat){
                auto caption=q.seat_caption(*attached,seat);if(caption==-1)caption=seat+0x1bbc;
                std::int32_t action=0x32;
                if(q.human_seat(*human)!=seat){auto* occupant=q.occupant(*attached,seat);
                    if(q.offline()){if(!occupant)action=seat+8;else if(q.health(*occupant)<1)caption=0x1b71;else if(q.in_roster(*occupant))action=seat+8;}
                    else if(!occupant)action=seat+8;else{occupant=q.occupant(*attached,seat);if(!occupant)throw std::logic_error("Roll occupant retired during identity read");caption=hud_roll_identity_caption(q.actor_id(*occupant));}
                }
                put(seat+1,caption,action);
            }
        }else{ //Kind34 omits the exit row and preserves occupied names.
            for(std::int32_t seat=0;seat<q.occupant_count(*attached);++seat){
                auto caption=q.seat_caption(*attached,seat);if(caption==-1)caption=seat+0x1bbc;
                auto action=0x32;if(q.human_seat(*human)!=seat){action=seat+8;if(q.occupant(*attached,seat)){auto* occupant=q.occupant(*attached,seat);if(!occupant)throw std::logic_error("Roll occupant retired during identity read");caption=hud_roll_identity_caption(q.actor_id(*occupant));}}
                put(seat,caption,action);
            }
        }
        finish();return;
    }
    auto* target=q.target(dialogs);if(!target){finish();return;}
    switch(q.kind(*target)){
    case 2:put(0,0x1b6d,2);break;
    case 3:
        if(q.offline()){
            if(q.in_roster(*target)){
                put(0,0x1b72,7);
                if(q.health(*target)<1){
                    put(1,0x1b7f,0x32);put(2,0x1b7a,0x32);
                    put(3,0x1b71,!q.navigation_floor(*target)&&(q.pose(*target)&4u)&&!q.flag18(*target)?6:0x32);
                }else{
                    if(!q.focus(*human)){
                        const bool can_heal=q.health_percent(*human)<100&&q.medical_item(*target);
                        put(1,0x1b7f,can_heal?0x1d:0x32);
                        const bool can_treat=q.medical_item(*human)&&q.health_percent(*target)<100;
                        put(2,0x1b7a,can_treat?0xf:0x32);
                    }else{put(1,0x1b7f,0x32);put(2,0x1b7a,0x32);}
                    put(3,0x1b71,0x32);
                }
            }else if(q.health(*target)<1){put(0,0x1b72,7);put(1,0x1b71,!q.navigation_floor(*target)&&(q.pose(*target)&4u)&&!q.flag18(*target)?6:0x32);}
            else if(q.unavailable(*target))put(0,0x1b72,7);
            else if(q.human_kind(*target)==2&&q.suppressed(*target))put(0,0x1b7e,0x13);
        }else{
            const auto team=q.team(*target),own_team=q.team(*human);
            if(team==own_team){const auto medical=q.medical_item(*human);const auto health=q.health(*target);
                put(0,0x1b7a,medical&&health>0&&q.health_percent(*target)<100&&!q.focus(*target)?0xf:0x32);
            }else if(q.special_game_mode()){
                if(q.unavailable(*target)||q.health(*target)<1)put(0,0x1b72,7);
                else if(q.human_kind(*target)==2&&q.suppressed(*target))put(0,0x1b7e,0x13);
            }
        }break;
    case 4:case 20:
        for(std::int32_t seat=0;seat<q.occupant_count(*target);++seat){
            auto caption=q.seat_caption(*target,seat);if(caption==-1)caption=seat+0x1bbc;
            auto* occupant=q.occupant(*target,seat);std::int32_t action;
            if(!occupant)action=seat+8;
            else if(q.offline()){
                if(q.health(*occupant)<1&&(q.pose(*occupant)&4u)){action=6;caption=0x1b71;}
                else action=q.in_roster(*occupant)?seat+8:0x32;
            }else{auto* again=q.occupant(*target,seat);if(!again)throw std::logic_error("Roll occupant retired during identity read");caption=hud_roll_identity_caption(q.actor_id(*again));action=0x32;}
            put(seat,caption,action);
        }break;
    case 6:{
        const auto state=q.door_state(*target);
        if(!q.door_locked(*target)){
            put(0,0x1b6f,state==1?0x32:0x1b);put(1,0x1b81,state==0?0x32:0x1c);
            put(2,0x1b6b,state==0&&q.has_definition(*human,q.door_key(*target))?0:0x32);
        }else{
            put(0,0x1b6f,0x32);put(1,0x1b81,0x32);
            put(2,0x1b6c,q.has_definition(*q.selected_human(),q.door_key(*target))?1:0x32);
            put(3,0x1b79,q.door_pickable(*target)?0xe:0x32);
        }break;}
    case 7:put(0,0x1b6e,q.switch_available(*target,*human)?3:0x32);break;
    case 9:{
        const bool limited=q.supply_limited(*target);const auto maximum=q.supply_maximum(*target);
        if(q.supply_uses_list(*target)){
            const auto team=q.team(*human);const auto& list=q.supply_items(*target,team!=0);
            for(std::size_t n=0;n<list.size();++n){const auto item=list[n];const auto caption=q.item_caption(item);
                put(n,caption,limited&&q.definition_count(*human,item)<maximum?6:0x32);}
        }else{
            const auto weapon=q.weapon(*human);bool enabled=false;
            if(limited&&weapon){const auto first=q.ammunition_definition(weapon);const auto second=first==0?q.ammunition_definition(weapon):first;
                enabled=q.ammunition_count(*human,first!=0||second>=1?1:0)<maximum;}
            put(0,0x1b8d,enabled?6:0x32);
        }break;}
    case 11:put(0,0x1b71,q.item_container_count(*target)>0?6:0x32);break;
    case 13:put(0,0x1b6e,3);break;
    case 15:{auto* selected=q.selected_human();put(0,0x1b6a,selected&&q.has_definition(*selected,0xe5)?3:0x32);break;}
    case 21:{auto* occupant=q.occupant(*target,0);
        if(!occupant)put(0,0x1b6e,8);
        else if(!q.offline())put(0,hud_roll_identity_caption(q.actor_id(*occupant)),0x32);
        else if(q.health(*occupant)<1&&(q.pose(*occupant)&4u))put(0,0x1b71,6);
        else put(0,0x1b6e,q.in_roster(*occupant)?8:0x32);break;}
    case 22:{const auto state=q.explosive_state(*target);put(0,0x1b87,state==0&&q.has_definition(*human,q.explosive_definition(*target))?4:0x32);put(1,0x1b6d,state==1?0x26:0x32);break;}
    case 23:put(0,q.special_caption(*target),q.has_definition(*human,q.special_definition(*target))?0x22:0x32);break;
    case 35:put(0,0x1b7b,q.placed_item_available(*human)?0x10:0x32);break;
    default:break;
    }
    finish();
}
//004B1EE0 command rows. It intentionally has different actor gates, seat
//captions and action numbers from the ordinary menu above.
template<class Dialogs,class Queries>void populate_hud_roll_command(Dialogs& dialogs,Queries& q){
    for(unsigned n=0;n!=10;++n)dialogs.set_roll_row(n,-1,-1);
    auto* human=q.previous_human();auto* target=q.target(dialogs);
    if(!target)throw std::logic_error("Command roll population requires actual cached target");
    if(!human)throw std::logic_error("Command roll population requires actual previous Human");
    const auto put=[&](unsigned n,std::int32_t caption,std::int32_t action){dialogs.set_roll_row(n,caption,action);};
    switch(q.kind(*target)){
    case 2:put(0,0x1b6d,2);break;
    case 3:
        if(q.health(*target)<1){put(0,0x1b71,!q.flag18(*target)?6:0x32);put(1,0x1b83,0x17);put(2,0x1b84,0x32);put(3,0x1b8a,0x21);}
        else if(q.unavailable(*target)){
            put(0,0x1b85,0x19);put(1,0x1b83,0x17);put(2,0x1b84,!q.human_flag1a5(*target)?0x18:0x32);put(3,0x1b8a,0x21);
        }else if(!q.in_roster(*target)){
            if(q.human_kind(*target)==4)put(0,0x1b89,0x20);
            else{
                put(0,0x1b85,0x19);put(1,0x1b86,q.has_definition(*human,0x16)||q.has_definition(*human,7)||q.has_definition(*human,0x104)?0x1a:0x32);
                put(2,0x1c42,q.item_kind_count(*human,9)?0x25:0x32);put(3,0x1b7e,q.suppressed(*target)?0x13:0x32);
            }
        }else if(target==q.previous_human()){
            if(auto* focus=q.focus(*target)){q.set_target(dialogs,q.identity(*focus));put(0,0x1b70,5);}
            else if(auto* attachment=q.attachment(*target);attachment&&hud_roll_vehicle_kind(q.kind(*attachment))){
                q.set_target(dialogs,q.identity(*attachment));put(0,0x1b70,5);
                for(std::int32_t seat=0;seat<q.occupant_count(*attachment);++seat)put(seat+1,seat+0x1bbc,q.occupant(*attachment,seat)?0x32:seat+8);
            }
        }else{
            put(0,0x1b89,0x20);put(1,0x1b7a,q.health_percent(*target)<100&&q.medical_item(*human)?0xf:0x32);put(2,0x1b8a,0x32);
        }break;
    case 4:
        if(q.car_interaction_blocked(*target,*human)){put(0,0x1b82,0x16);break;}
        if(target==q.attachment(*q.previous_human())){put(0,0x1b70,5);break;}
        for(std::int32_t seat=0;seat<q.occupant_count(*target);++seat){auto caption=q.seat_caption(*target,seat);if(caption==-1)caption=seat+0x1bbc;
            auto* occupant=q.occupant(*target,seat);const auto action=!occupant?seat+8:q.health(*occupant)<1?6:0x32;
            if(occupant&&action==6)caption=0x1b71;put(seat,caption,action);}
        put(0,0x1b82,0x16);break; //Original final row overwrites seat0.
    case 6:
        put(0,0x1b6f,0x1b);put(1,0x1b81,0x1c);put(2,0x1b80,0x15);
        put(3,0x1b7d,!q.door_use_blocked(*target)&&q.door_has_link(*target)?0x12:0x32);
        if(!q.door_locked(*target))put(4,0x1b6b,q.has_definition(*human,q.door_key(*target))?0:0x32);
        else{put(4,0x1b79,q.door_pickable(*target)?0xe:0x32);put(5,0x1b6c,q.has_definition(*human,q.door_key(*target))?1:0x32);}break;
    case 7:{const auto definition=q.switch_definition(*target);put(0,0x1b6e,(definition<0||q.has_definition(*human,definition))&&q.switch_state(*target)==1?3:0x32);break;}
    case 8:case 13:put(0,0x1b6e,3);break;
    case 9:{const bool limited=q.supply_limited(*target);const auto maximum=q.supply_maximum(*target);
        if(q.supply_uses_list(*target)){const auto team=q.team(*human);const auto& list=q.supply_items(*target,team!=0);
            for(std::size_t n=0;n<list.size();++n){const auto item=list[n];const auto caption=q.item_caption(item);put(n,caption,limited&&q.definition_count(*human,item)<maximum?6:0x32);}
        }else{const auto weapon=q.weapon(*human);bool enabled=false;
            if(limited&&weapon){if(q.ammunition_definition(weapon)>0)enabled=q.ammunition_count(*human,q.ammunition_definition(weapon))<maximum;}
            put(0,0x1b8d,enabled?6:0x32);
        }break;}
    case 11:put(0,0x1b71,6);break;
    case 15:put(0,0x1b6a,3);break;
    case 20:
        if(target==q.attachment(*q.previous_human()))put(0,0x1b70,5);
        else for(std::int32_t seat=0;seat<q.occupant_count(*target);++seat)put(seat,seat+0x1bbc,q.occupant(*target,seat)?0x32:seat+8);
        break;
    case 21:
        if(target==q.attachment(*q.previous_human()))put(0,0x1b70,5);
        else if(auto* occupant=q.occupant(*target,0)){
            if(q.health(*occupant)<1)put(0,0x1b71,6);
            else if(q.hostile(*occupant,*human))put(0,0x1b85,0x19);else put(0,0x1b6e,0x32);
        }else put(0,0x1b6e,8);break;
    case 22:{const auto state=q.explosive_state(*target);if(state>=0&&state<=2){put(0,0x1b87,state==0&&q.has_definition(*human,q.explosive_definition(*target))?4:0x32);put(1,0x1b6d,state==1?0x26:0x32);}break;}
    case 23:put(0,q.special_caption(*target),q.has_definition(*human,q.special_definition(*target))?0x22:0x32);break;
    case 25:if(!q.animation_active(*target))put(0,0x1b82,0x16);break;
    case 30:put(0,0x1b85,0x19);put(1,0x1b86,0x1a);break;
    case 35:put(0,0x1b7b,q.placed_item_available(*human)?0x10:0x32);break;
    default:break;
    }
    put(9,0x1c6a,0x33);
}
} //namespace hd2::game
#endif
