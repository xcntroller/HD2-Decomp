#ifndef HD2_GAME_MISSION_SCRIPT_MODEL_VISIBILITY_H
#define HD2_GAME_MISSION_SCRIPT_MODEL_VISIBILITY_H

#include "game/scene_actor.h"

namespace hd2::game {

//0042B650 case5F's actual model branch. Humans resolves the SAME containing
//inline98 owner (DD0 shadow/230 equipment), never a reconstructed duplicate.
//Animation actors require their real005B54C0 owner; Players/View are the same
//current control2C and FPV owner used by0045DAC0.
template<class Frames,class Human,class Effects,class Humans,class Animation,class Players,class View>
class MissionScriptModelVisibility {
public:
    using Frame=typename Frames::Frame;
    MissionScriptModelVisibility(Frames& frames,Effects& effects,Humans& humans,Animation& animation,
                                 Players& players,View& view,std::uint32_t& mode)
        :frames_(frames),effects_(effects),humans_(humans),animation_(animation),players_(players),view_(view),mode_(mode){}
    void script_model_visibility(Frame frame,bool enabled) {
        const auto* data=frames_.find_actor_frame_data(frame);
        auto* actor=data?reinterpret_cast<SceneActor<Frames>*>(data->words[0]):nullptr;
        if(actor&&actor->base().type()==3) {
            auto* human=dynamic_cast<Human*>(actor);if(!human)throw std::runtime_error("Script model EC Human type disagrees with its actual owner");
            auto& control=humans_.human_inline(*human);
            if(!control.initialized()||!control.control()||!control.equipment())throw std::runtime_error("Script visibility requires the actual active Human inline98");
            auto& shadow=control.control()->shadow();
            if(!enabled)shadow.clear(); //00758C60, the same DD0 A9C owner.
            else {
                control.equipment()->refresh(frame); //007866D0→007920B0; same actual230.
                if(const auto model=shadow.frame())frames_.object_visible(model,true); //Native forces A9C visible after refresh.
            }
            frames_.object_visible(frame,enabled);
            if(enabled&&players_.control().current_player()==human)view_.update_player_visibility();
            frames_.update_scene_object(frame);return;
        }
        //No Human actor: generic GEffects shadow model and its retained query.
        if(!enabled) {
            if(const auto shadow=frames_.find_human_shadow_model(frame)) {
                auto reference=frames_.retain_actor_frame(shadow); //Original GEffects virtual7C adds this reference.
                frames_.object_visible(shadow,false);frames_.update_scene_object(shadow);reference.reset();
            }
        }else (void)effects_.build(mode_,frame,false); //GEffects virtual78, not Human puppet path.
        if(actor) {
            const auto kind=actor->base().type();
            if(kind==25)animation_.set_animation_actor_visibility(*actor,enabled); //005B54C0 actual listE0 owner.
            else if(kind==34||kind==36) {
                effects_.frame_visibility(frame,enabled,0x2088);frames_.update_scene_object(frame);return;
            }
        }
        frames_.object_visible(frame,enabled);frames_.update_scene_object(frame);
    }
private:
    Frames& frames_;Effects& effects_;Humans& humans_;Animation& animation_;Players& players_;View& view_;std::uint32_t& mode_;
};
} //namespace hd2::game
#endif
