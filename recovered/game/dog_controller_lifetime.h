#ifndef HD2_GAME_DOG_CONTROLLER_LIFETIME_H
#define HD2_GAME_DOG_CONTROLLER_LIFETIME_H
#include "game/scene_actor.h"
#include "game/dog_controller_process.h"
#include <optional>
namespace hd2::game {
template<class Frames>struct DogControllerState {
    using Actor=SceneActor<Frames>;
    explicit DogControllerState(std::int32_t kind):kind14(kind),byte74(static_cast<std::uint8_t>(kind)){movement30.xyz=direction40.xyz=std::array<float,3>{{0,0,1}};}
    Actor* actor10=nullptr;const std::int32_t kind14;Actor* follow18=nullptr;void* move1c=nullptr;
    std::uint32_t flags20=0x2000u;
    DogControllerVector movement30,direction40,saved50;
    std::array<float,64>* first60=nullptr;std::array<float,64>* second64=nullptr;std::uint32_t field68=0;
    void* navigation70=nullptr;const std::uint8_t byte74;
    std::vector<DogRoutePoint*> route78;std::int32_t timer84=0;Actor* target88=nullptr;Actor* damage_source8c=nullptr;
    DogControllerVector target90,followa0;std::int32_t timerb0=0,timerb4=0;bool flagb8=false;std::int32_t timerbc=0;
    void* scriptc0=nullptr;std::optional<std::int32_t> timerc4;
};
//The actual Actor534 owner. Unrecovered controller bodies remain required
//virtual receivers; no numeric presence flag or successful idle control exists.
template<class Frames>class DogControllerLifetime {
public:
    using Actor=SceneActor<Frames>;
    virtual ~DogControllerLifetime()=default;
    virtual std::int32_t dog_control_kind()const=0; //14.
    virtual Actor* dog_control_target()=0; //24, actual88 (not follow18).
    virtual long double dog_control_speed()=0; //28, ST0.
    virtual void dog_control_dead()=0; //005CE6E0.
    virtual void dog_control_damage(std::uint32_t,Actor*)=0; //54.
    virtual DogControllerState<Frames>& dog_control_state()=0;
    virtual bool dog_control_bind(Actor&)=0; //00/04.
    virtual void dog_control_restore(DogControllerLifetime&)=0; //10.
    virtual void dog_control_follow(Actor*)=0; //30.
    virtual void dog_control_close()=0; //0C, followed separately by delete08.
    virtual void dog_control_script(void*)=0; //40.
    virtual void dog_control_authority(std::uint32_t)=0; //18, actual005CF0D0→005CE190 or005CFA20.
    virtual void dog_control_presentation(std::uint32_t)=0; //14, selected4/5 wrappers tail-jump actual RET4.
    virtual void dog_control_sight_acquired(Actor*)=0; //4C.
    virtual void dog_control_sight_lost(Actor*)=0; //50.
    virtual void dog_control_loud_sound()=0; //44, selected4/5 actual RET.
    virtual void dog_control_heard(DogHeardRecord&)=0; //48.
    virtual void dog_control_publish_inputs()=0; //6C,005CF0B0/005CFA00 tail-jump005CE110.
};
}
#endif
