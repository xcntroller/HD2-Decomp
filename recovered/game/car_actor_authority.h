#ifndef HD2_GAME_CAR_ACTOR_AUTHORITY_H
#define HD2_GAME_CAR_ACTOR_AUTHORITY_H
namespace hd2::game {
//004DE800: seat0 and actual local peer3C are independent of cached driver24AC.
//Retain native repeated virtual78(0) calls and late mode reads. Both null is
//genuine equality; a changed nonnull seat can require a fresh Human kind query.
template<class Mode,class Seat,class Local,class Kind>
bool car_actor_authority(Mode&& mode,Seat&& seat,Local&& local,Kind&& kind){
    if((mode()&7u)==7u)return true;
    const auto occupant=seat();const auto player=local(); //Original virtual78 BEFORE00730460.
    if(occupant!=player){
        if(seat()&&kind(seat())!=1u&&(mode()&1u))return true;
        if(seat()||!(mode()&1u))return false;
    }
    return true;
}
}
#endif
