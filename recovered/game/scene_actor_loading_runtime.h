#ifndef HD2_GAME_SCENE_ACTOR_LOADING_RUNTIME_H
#define HD2_GAME_SCENE_ACTOR_LOADING_RUNTIME_H

#include "game/scene_actor_runtime.h"
#include "frontend/spatial_sound.h"
#include <cstring>
#include <stdexcept>

namespace hd2::game {

//Concrete source dispatch for the same Scene2 Human/Switch actor runtime.
//Dependencies supplies actual process mode, checked module context lookup,
//shared SOUND emission, process proximity and nonoffline player-record owner.
//It may borrow a containing owner declared before Frames/modules are built;
//none of these callbacks may run until their actual required owner exists.
//Construction itself creates no service, actor, context or readiness state.
template<class Frames,class Dependencies>
class SceneActorLoadingRuntime final : public SceneActorRuntime<typename Frames::Frame,typename Frames::Vector> {
public:
    using Frame=typename Frames::Frame;using Vector=typename Frames::Vector;
    explicit SceneActorLoadingRuntime(Dependencies& dependencies):dependencies_(dependencies){}
    std::uint32_t mode_flags()const override{return dependencies_.mode_flags();}
    bool switch_has_context(Frame frame)override{return dependencies_.script_context_for_frame(frame)!=nullptr;}
    void notify_switch_state(Frame frame,std::uint32_t)override { //006136F0→0041D960, pushed state argument unused/RET4.
        auto* context=dependencies_.script_context_for_frame(frame);
        if(!context)throw std::logic_error("Switch notification lost its actual script context owner");
        context->use((mode_flags()&1u)!=0); //Actual Use event PC request, never variable assignment or immediate Block.
    }
    void play_switch_sound(std::uint32_t identity,const Vector& position)override { //006136F0's actual00433FA0 request.
        frontend::SpatialSoundRequest request;request.flags=1;request.group=1;
        std::memcpy(&request.entry,&identity,4);request.position=position;
        (void)dependencies_.emit_scene_actor_sound(request);
    }
    void release_human_proximity(std::int32_t index)override{dependencies_.actor_proximity().release_human_proximity(index);} //SAME006DFE50 process owner.
    void clear_human_player_slot(std::uint32_t index)override{dependencies_.clear_human_player_slot(index);} //Actual nonoffline record consumer; no fabricated slot array.
private:
    Dependencies& dependencies_;
};

} //namespace hd2::game
#endif
