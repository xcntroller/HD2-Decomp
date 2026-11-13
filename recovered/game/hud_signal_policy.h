#ifndef HD2_GAME_HUD_SIGNAL_POLICY_H
#define HD2_GAME_HUD_SIGNAL_POLICY_H
#include "game/hud_signal_tables.h"
#include <stdexcept>
namespace hd2::game {
//004BF640: signal rows store caption/action, unlike roll's action/caption.
inline void set_hud_signal_row(HudSignalTables::Row& rows,std::size_t index,std::uint32_t caption,std::uint32_t action){
    if(index>=10)throw std::out_of_range("Signal row exceeds native ten-row table");
    rows[index*2]=caption;rows[index*2+1]=action==50?72:action+100u;
}
template<class Queries,class Actor>bool hud_signal_target_eligible(Queries& q,Actor* target,Actor* human){ //004C10F0.
    if(!q.offline()||!target)return false;
    switch(q.kind(*target)){
    case 2:case 6:case 7:case 8:case 9:case 11:case 13:case 15:case 20:case 21:case 22:case 23:case 28:case 30:case 35:return true;
    case 3:return target!=human;
    case 4:{auto* selected=q.selected_human();if(!selected)throw std::logic_error("Signal Car target requires selected Human");
        if(!q.car_interaction_available(*target)&&!q.car_interaction_blocked(*target,*selected))return false;return !q.car_destroyed(*target);}
    case 25:return !q.animation_active(*target)&&!q.animation_excludes_target(*target);
    default:return false;
    }
}
//004BFFE0. Row copies come from the actual process table; Human/actor queries
//are loans from existing owners. Populated Human orders must be supplied.
template<class Signals,class Queries>void populate_hud_signals(Signals& s,const HudSignalTables::Table& table,Queries& q){
    auto& rows=s.rows;const auto copy=[&](unsigned row){rows=table.at(row);};
    const auto disable=[&](unsigned row){rows.at(row*2+1)=72;};
    const auto put=[&](unsigned row,std::uint32_t caption,std::uint32_t action){set_hud_signal_row(rows,row,caption,action);};
    if(s.flag6c){copy(10);return;}
    auto* selected=q.selected_human();auto* human=s.human68?q.resolve(s.human68):nullptr;
    auto* target=s.target64?q.resolve(s.target64):nullptr;
    switch(s.field0c){
    case 50:
        copy(0);
        if(q.offline()){
            if(!hud_signal_target_eligible(q,target,human)){disable(2);disable(5);}
            bool all_empty=true,all_pending=true;
            if(!human){
                for(auto n=q.roster_count();n;){--n;auto* candidate=q.roster_human(n);
                    if(candidate!=selected){candidate=q.roster_human(n);if(q.health(*candidate)>0){candidate=q.roster_human(n);
                        if(q.orders_count(*candidate)==0)all_pending=false;else all_empty=false;}}
                }
                if(!all_pending){if(all_empty)disable(7);disable(8);if(!selected)throw std::logic_error("Signal formation query requires selected Human");if(q.formation_kind(*selected)==13)disable(6);return;}
            }else if(q.orders_count(*human)==0){disable(7);disable(6);disable(8);return;}
            for(unsigned row:{0u,1u,2u,3u,4u,5u,6u,8u})disable(row);return;
        }
        disable(2);disable(5);
        if(!selected)throw std::logic_error("Signal item query requires selected Human");
        if(q.has_definition(*selected,0xf4)||q.has_definition(*selected,0xf3)){rows[17]=0x3b;rows[16]=0x1c28;}return;
    case 51:copy(1);return;case 52:copy(2);return;case 54:copy(4);return;case 55:copy(5);return;
    case 53:case 56:
        copy(s.field0c==53?3:6);selected=q.selected_human();if(!target)return;
        switch(q.kind(*target)){
        case 2:put(0,0x1b6d,2);put(1,0x1b82,0x16);return;
        case 3:
            if(q.health(*target)<1){
                if(human&&target==q.focus(*human)){put(0,0x1b70,5);put(1,0x1b83,50);put(2,0x1b84,50);put(3,0x1b8a,50);}
                else{put(0,0x1b71,6);put(1,0x1b83,0x17);put(2,0x1b84,50);put(3,0x1b8a,0x21);}return;
            }
            if(q.unavailable(*target)){put(0,0x1b85,0x19);put(1,0x1b83,0x17);put(2,0x1b84,q.human_flag1a5(*target)?50:0x18);put(3,0x1b8a,0x21);return;}
            if(!q.in_roster(*target)){put(0,0x1b85,0x19);put(1,0x1b86,0x1a);put(2,0x1b7e,q.suppressed(*target)?0x13:50);return;}
            {bool medical=human?q.medical_item(*human)!=0:q.any_medical_for(*target);put(0,0x1b7a,medical?0xf:50);put(1,0x1b8a,0x21);return;}
        case 4:
            if(!selected)throw std::logic_error("Signal occupied Car query requires selected Human");
            if(q.car_interaction_blocked(*target,*selected)){put(0,0x1b82,0x16);return;}
            if(human){
                auto* attached=q.attachment(*human);if(attached==target){put(0,0x1b70,5);return;}
                put(0,0x1b6e,3);if(attached&&q.attachment_seat_special(*human,*attached))put(1,0x1b82,0x16);return;
            }else{
                bool inside=false,outside=false;
                for(auto n=q.roster_count();n;){auto* candidate=q.roster_human(--n);
                    if(candidate!=selected&&q.health(*candidate)>0&&q.orders_count(*candidate)==0){if(q.attachment(*candidate)==target)inside=true;else outside=true;}}
                if(inside){if(outside){put(0,0x1b6e,3);put(1,0x1b70,5);}else put(0,0x1b70,5);}
                else{put(0,0x1b6e,3);put(1,0x1b82,0x16);}return;
            }
        case 6:
            if(!q.door_locked(*target)){
                put(0,0x1b6f,0x1b);put(1,0x1b81,0x1c);put(2,0x1b80,0x15);put(3,0x1b7d,!q.door_use_blocked(*target)&&q.door_has_link(*target)?0x12:50);
                auto* current=q.selected_human();put(4,0x1b6b,q.door_state(*target)==0&&current&&q.has_definition(*current,q.door_key(*target))?0:50);return;
            }
            {constexpr std::uint32_t captions[]{0x1b6f,0x1b81,0x1b80,0x1b7d};for(unsigned row=0;row!=4;++row)put(row,captions[row],50);}
            put(4,0x1b79,q.door_pickable(*target)?0xe:50);
            {const auto key=q.door_key(*target);const bool present=human?q.has_definition(*human,key):q.any_definition(key);put(5,0x1b6c,present?1:50);}return;
        case 7:{if(q.switch_state(*target)!=1){put(0,0x1b6e,50);return;}const auto definition=q.switch_definition(*target);
            const bool present=human?q.has_definition(*human,definition):q.any_definition(definition);put(0,0x1b6e,present?3:50);return;}
        case 8:case 13:case 28:put(0,0x1b6e,3);return;
        case 9:{const auto maximum=q.supply_maximum(*target);const bool limited=q.supply_limited(*target);
            if(!q.supply_uses_list(*target)){
                bool enabled=limited;
                if(human){const auto weapon=q.weapon(*human);if(limited&&weapon&&q.ammunition_definition(weapon)>0)enabled=q.definition_count(*human,q.ammunition_definition(weapon))<maximum;}
                put(0,0x1b8d,enabled?6:50);return;
            }
            const auto& list=q.supply_items(*target,true);
            for(std::size_t n=0;n<list.size();++n){const auto definition=list[n];const auto caption=q.item_caption(definition);
                bool enabled=limited;
                if(limited&&human&&q.definition_pickup_capacity(*human,definition)<1)enabled=q.definition_count(*human,definition)<maximum;
                put(n,caption,enabled?6:50);
            }return;}
        case 11:put(0,0x1b71,6);return;
        case 15:{const bool present=human?q.has_definition(*human,0xe5):q.any_definition(0xe5);put(0,0x1b6a,present?3:50);return;}
        case 20:case 21:{const bool attached=human&&q.attachment(*human)==target;put(0,attached?0x1b70:0x1b6e,attached?5:3);return;}
        case 22:{const auto state=q.explosive_state(*target);
            if(state==0){const auto definition=q.explosive_definition(*target);const bool present=human?q.has_definition(*human,definition):q.any_definition(definition);put(0,0x1b87,present?4:50);put(1,0x1b6d,50);}
            else if(state==1){put(0,0x1b87,50);put(1,0x1b6d,0x26);}else if(state==2){put(0,0x1b87,50);put(1,0x1b6d,50);}return;}
        case 23:{const auto definition=q.special_definition(*target);const auto caption=q.special_caption(*target);const bool present=human?q.has_definition(*human,definition):q.any_definition(definition);put(0,caption,present?0x22:50);return;}
        case 25:if(!q.animation_active(*target))put(0,0x1b82,0x16);return;
        case 30:put(0,0x1b85,0x19);return;
        case 35:put(0,0x1b7b,!human||q.placed_item_available(*human)?0x10:50);return;
        default:return;
        }
    case 57:
        copy(7);if(!selected)throw std::logic_error("Signal formation query requires selected Human");
        {const auto kind=q.formation_kind(*selected);if(kind==3)disable(0);else if(kind==0)disable(1);else if(kind==1)disable(2);}
        if(q.formation_scale_minimum())disable(3);else if(q.formation_scale_maximum())disable(4);return;
    case 58:copy(9);return;
    case 59:
        copy(8);if(!selected)throw std::logic_error("Signal item query requires selected Human");if(!q.has_definition(*selected,0xf3))disable(0);if(!q.has_definition(*selected,0xf4))disable(1);return;
    default:return;
    }
}
} //namespace hd2::game
#endif
