#ifndef HD2_GAME_HUD_SIGNAL_ACTIONS_H
#define HD2_GAME_HUD_SIGNAL_ACTIONS_H
#include <array>
#include <cstdint>
#include <stdexcept>

namespace hd2::game {
//00810528 kind3 request, interpreted fields only.005A45E0/0053D910 copies this
//stack record into its existing owned queue; that producer must preserve the
//original queued-actor retirement contract. UI retains neither record nor loan.
//Native vector padding/vtable and unused fourth position lane are not payload.
template<class Actor>struct HudSignalCommand {
    std::array<float,4> position{};
    std::uint32_t command=0,action=0xffffffffu;
    Actor* target=nullptr;
    std::int32_t parameter=-1;
    Actor* sender=nullptr;
};
template<class Signals,class Queries>void dispatch_hud_signal(const Signals& signals,Queries& queries){ //004C0990.
    using Actor=typename Queries::Actor;
    auto* selected=queries.selected_human();if(!selected)throw std::logic_error("Signal command requires actual selected Human");
    if(signals.selected60<0||signals.selected60>9)throw std::out_of_range("Signal command requires selected row");
    const auto action=signals.rows[static_cast<std::size_t>(signals.selected60)*2+1];
    HudSignalCommand<Actor> request;
    switch(signals.field0c){
    case 51:case 54:
        request.position=(action==10||action==11)?queries.signal_point(queries.signal_range()):queries.signal_direction(*selected);
        request.command=action;break;
    case 52:case 55:
        if(action==18||action==20)request.position=queries.signal_point(queries.signal_range());
        else if(action==19)request.position=queries.signal_point(40.f);
        else request.position=queries.signal_direction(*selected);
        request.command=action;break;
    case 53:case 56:
        request.command=signals.field0c;request.action=action-100u;
        request.target=signals.target64?queries.resolve(signals.target64):nullptr;
        if(request.action==6&&request.target&&queries.kind(*request.target)==9)request.parameter=signals.selected60;
        break;
    case 57:case 58:case 59:
        request.position=queries.signal_direction(*selected);request.command=action;break;
    default:return;
    }
    request.sender=signals.human68?queries.resolve(signals.human68):nullptr;
    queries.submit_signal(*selected,request);
}
//004C08A0: camera origin, actual selected Human direction, query exclusion,
//110000 ray, restoration BEFORE publication. Returns hit versus miss endpoint.
template<class Queries>bool hud_signal_point(float range,std::array<float,4>& position,Queries& queries){
    auto* selected=queries.selected_human();if(!selected)throw std::logic_error("Signal point requires actual selected Human");
    const auto direction=queries.signal_direction(*selected);std::array<float,4> travel{};
    for(unsigned axis=0;axis!=3;++axis)travel[axis]=static_cast<float>(static_cast<long double>(range)*direction[axis]);
    position=queries.signal_origin();queries.exclude_signal_human(*selected);
    decltype(queries.signal_ray(position,travel)) hit;
    try{hit=queries.signal_ray(position,travel);}catch(...){queries.restore_signal_human(*selected);throw;}
    queries.restore_signal_human(*selected);
    if(hit){position=*hit;return true;}
    for(unsigned axis=0;axis!=3;++axis)position[axis]=static_cast<float>(static_cast<long double>(position[axis])+travel[axis]);
    return false;
}
} //namespace hd2::game
#endif
