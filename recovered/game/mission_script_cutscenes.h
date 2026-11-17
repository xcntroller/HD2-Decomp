#ifndef HD2_GAME_MISSION_SCRIPT_CUTSCENES_H
#define HD2_GAME_MISSION_SCRIPT_CUTSCENES_H
#include "game/mission_script_context.h"

namespace hd2::game {
//00423C70/00423EB0: retained camera828, projection82C/830, identity834,
//context event/filter flags and SAME World HUD/input/FPV/dubbing lifetimes.
template<class Frames,class Contexts,class Snapshots,class Animations,class Runtime>
class MissionScriptCutscenes {
public:
    using Frame=typename Frames::Frame;
    using Owner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    MissionScriptCutscenes(Frames& frames,Contexts& contexts,Snapshots& snapshots,Animations& animations,Runtime& runtime,
                          std::uint32_t& active,std::uint32_t& timer)
        :frames_(frames),contexts_(contexts),snapshots_(snapshots),animations_(animations),runtime_(runtime),active_(active),timer_(timer){}
    void begin(std::uint32_t identity){
        active_=1;saved_command_=runtime_.script_command_visible();runtime_.script_show_command(false);
        const auto camera=frames_.active_game_camera();if(camera)camera_.emplace(frames_.retain_actor_frame(camera));
        identity_=identity;
        for(auto n=contexts_.values().size();n;){
            auto* context=contexts_.values().at(--n).get();
            if(context&&!context->cutscene(identity_,runtime_.script_authority()))runtime_.script_cutscene_actor(*context,false);
        }
        runtime_.script_cutscene_window_query();runtime_.script_player_input(false);
        frames_.script_cutscene_initial_viewport();
        const auto current=frames_.active_game_camera();
        if(!current)throw std::runtime_error("Cutscene start reached native NULL active camera");
        projection_=frames_.vertex_camera_projection(current);
        frames_.game_camera_parameter8c(current,1.7777777910232544f);
        frames_.game_camera_fov(current,runtime_.script_cutscene_fov());runtime_.script_clear_dubbing();
    }
    void end(){
        if(!static_cast<std::uint8_t>(active_))return;
        active_=0; //Native clears824 BEFORE any camera/Human callbacks.
        if(!camera_)throw std::runtime_error("Cutscene end reached native NULL saved camera828");
        const auto saved=camera_->get();frames_.game_camera_parameter8c(saved,projection_[1]);frames_.game_camera_fov(saved,projection_[0]);
        if(frames_.script_scene_available())frames_.active_game_camera(saved);
        camera_.reset();bool first=false,secondary=false;
        for(auto n=contexts_.values().size();n;){
            auto* context=contexts_.values().at(--n).get();if(!context)continue;
            if(!context->cutscene_done(identity_,runtime_.script_authority()))runtime_.script_cutscene_actor(*context,true);
            if(context->saved_first_person()){context->saved_first_person()=false;first=true;}
            if(context->saved_secondary_view()){context->saved_secondary_view()=false;secondary=true;}
        }
        if(first)runtime_.script_restore_first_person();
        if(secondary)runtime_.script_restore_secondary_view();
        if(frames_.script_scene_available()){
            //0049B7C0(1) is a real RET4, no HUD flag mutation.
            timer_=500;frames_.script_cutscene_full_viewport();
            const auto current=frames_.active_game_camera();
            if(!current)throw std::runtime_error("Cutscene end reached native NULL current camera");
            frames_.game_camera_parameter8c(current,1.3333333730697632f);
        }
        runtime_.script_cutscene_fade();
        //868 spline-model producer is not supplied by any reconstructed command;
        //87C/88C are actual separately owned animation records.
        animations_.end_cutscene([&](Frame model){snapshots_.restore_one(model);});
        runtime_.script_clear_dubbing();runtime_.script_show_command(saved_command_);saved_command_=false;
    }
    void tick_timer(std::uint32_t elapsed){ //00428490, BEFORE loaded/context filter.
        if(signed_word(timer_)<=0)return;
        timer_-=elapsed;if(signed_word(timer_)<=0){timer_=0;runtime_.script_player_input(true);}
    }
    template<class Context>std::optional<bool> execute(Context& context,const MissionScriptInstruction& instruction){
        if(instruction.opcode!=0xa2&&instruction.opcode!=0xa3&&instruction.opcode!=0x90)return std::nullopt;
        if(!runtime_.script_authority())return false;
        if(instruction.opcode==0xa2)begin(context.program().integers.at(instruction.operands.at(0)).value);
        else if(instruction.opcode==0xa3)end();
        else runtime_.script_player_input(context.program().integers.at(instruction.operands.at(0)).value!=0);
        context.advance(runtime_);return true;
    }
private:
    static std::int32_t signed_word(std::uint32_t value){std::int32_t result;std::memcpy(&result,&value,4);return result;}
    Frames& frames_;Contexts& contexts_;Snapshots& snapshots_;Animations& animations_;Runtime& runtime_;std::uint32_t& active_;std::uint32_t& timer_;
    std::optional<Owner> camera_;std::array<float,2> projection_{};std::uint32_t identity_=0;bool saved_command_=false;
};
} //namespace hd2::game
#endif
