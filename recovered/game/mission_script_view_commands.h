#ifndef HD2_GAME_MISSION_SCRIPT_VIEW_COMMANDS_H
#define HD2_GAME_MISSION_SCRIPT_VIEW_COMMANDS_H

#include "game/mission_script_context.h"
#include "game/scene_actor.h"
#include "game/session_activation.h"

namespace hd2::game {

//00416E10 opcodes3A/7B/7C/7D/FD. These borrow actual mission-camera/FPV/
//Effects/session owners. The camera's borrowed288 look target is separate
//from its retained248 Human-follow target. No HUD/console or camera defaults.
template<class Frames,class Human,class Camera,class FPV,class Effects>class MissionScriptViewCommands {
public:
    using Frame=typename Frames::Frame;using Context=MissionScriptContext<Frames>;
    MissionScriptViewCommands(Frames& frames,Camera& camera,FPV& fpv,Effects& effects,
            SessionActivationState& session,const std::uint32_t& mode)
        :frames_(frames),camera_(camera),fpv_(fpv),effects_(effects),session_(session),mode_(mode){}
    template<class Runtime>std::optional<bool> execute(Context& context,const MissionScriptInstruction& instruction,Runtime& runtime) {
        switch(instruction.opcode){case 0x3a:case 0x7b:case 0x7c:case 0x7d:case 0xfd:break;default:return std::nullopt;}
        if(!runtime.script_authority())return false;
        switch(instruction.opcode){
        case 0x3a:{
            const auto model=context.slot(instruction.operands.at(0));
            const auto text=context.program().strings.at(instruction.operands.at(1));
            const auto* data=model?frames_.find_actor_frame_data(model):nullptr;
            auto* actor=data?reinterpret_cast<SceneActor<Frames>*>(data->words[0]):nullptr;
            if(actor&&actor->base().type()==3){
                auto* human=dynamic_cast<Human*>(actor);
                if(!human)throw std::logic_error("Face texture command requires its actual Human");
                human->fieldbc=text; //005A87C0 stores even if GEffects declines the texture.
                (void)effects_.human_eye_texture(human->base().frame(),human->fieldbc);
            }else (void)effects_.human_eye_texture(model,text);
            break;
        }
        case 0x7b:{
            const auto next=context.slot(instruction.operands.at(0));if(!next)break;
            if(!camera_.owns_frame(next)){
                if(fpv_.enabled()&&!context.saved_first_person()){context.saved_first_person()=true;fpv_.enable(mode_,false);}
                if(fpv_.secondary_enabled()&&!context.saved_secondary_view()){context.saved_secondary_view()=true;fpv_.secondary_view(mode_,false);}
            }
            if(frames_.frame_kind(next)!=3)break; //View flags above still change for a noncamera frame.
            if(camera_.owns_frame(next)){
                if(context.saved_first_person()){context.saved_first_person()=false;fpv_.enable(mode_,true);}
                if(context.saved_secondary_view()){context.saved_secondary_view()=false;fpv_.secondary_view(mode_,true);}
            }
            if(!frames_.script_scene_available())break;
            if(const auto previous=frames_.active_game_camera()){
                const auto clips=frames_.hud_camera_clips(previous);frames_.game_camera_range(next,clips[0],clips[1]);
                frames_.game_camera_parameter8c(next,frames_.vertex_camera_projection(previous)[1]);
                frames_.game_camera_fov(next,frames_.vertex_camera_projection(previous)[0]);
                frames_.update_scene_object(next);
            }
            frames_.active_game_camera(next);break;
        }
        case 0x7c:{
            const auto destination=instruction.operands.at(0);
            context.slot(destination,Frame{}); //Release BEFORE reading scene15C and retaining the new camera.
            if(frames_.script_scene_available())if(const auto camera=frames_.active_game_camera())context.slot(destination,camera);
            break;
        }
        case 0x7d:camera_.look_target(context.slot(instruction.operands.at(0)),-1);break;
        case 0xfd:session_.words.set(0x36,std::uint8_t(context.program().integers.at(instruction.operands.at(0)).value!=0));break;
        default:throw std::logic_error("Unreachable script view command");
        }
        context.advance(runtime);return true;
    }
private:
    Frames& frames_;Camera& camera_;FPV& fpv_;Effects& effects_;SessionActivationState& session_;const std::uint32_t& mode_;
};

} //namespace hd2::game
#endif
