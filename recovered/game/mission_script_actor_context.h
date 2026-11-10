#ifndef HD2_GAME_MISSION_SCRIPT_ACTOR_CONTEXT_H
#define HD2_GAME_MISSION_SCRIPT_ACTOR_CONTEXT_H

#include "game/mission_script_context.h"
#include "game/scene_actor.h"

namespace hd2::game {

//00421BC0/00421930 add the real borrowed actor10C and zero byte108 to the
//ordinary retained frame/EC context. Commands still require the actual derived
//00421C00/00421970 receiver; family selection supplies no successful command.
template<class Frames,std::uint32_t Family>class ActorMissionScriptContext final : public MissionScriptContext<Frames> {
public:
    static_assert(Family==1||Family==2,"Actual actor context family is Human1 or Dog2");
    using Actor=SceneActor<Frames>;
    ActorMissionScriptContext(Frames& frames,typename Frames::Frame frame,Actor& actor)
        :MissionScriptContext<Frames>(frames,frame,false),actor_(actor){}
    std::uint32_t family()const override{return Family;} //00421BF0/00421960.
    Actor& actor()const{return actor_;}bool flag108()const{return flag108_;}
    void flag108(bool value){flag108_=value;}
private:
    Actor& actor_;bool flag108_=false;
};

//00427730 factory selection from the SAME frame EC word0, before compiler or
//Human state notifications. All non-Human/Dog cases genuinely use the base.
template<class Frames>std::unique_ptr<MissionScriptContext<Frames>> make_mission_script_context(
        Frames& frames,typename Frames::Frame frame){
    using Actor=SceneActor<Frames>;
    const auto* data=frames.find_actor_frame_data(frame);
    auto* actor=data?reinterpret_cast<Actor*>(data->words[0]):nullptr;
    if(actor)switch(actor->base().type()){
    case 3:return std::make_unique<ActorMissionScriptContext<Frames,1>>(frames,frame,*actor);
    case 30:return std::make_unique<ActorMissionScriptContext<Frames,2>>(frames,frame,*actor);
    default:break;
    }
    return std::make_unique<MissionScriptContext<Frames>>(frames,frame,false);
}

} //namespace hd2::game
#endif
