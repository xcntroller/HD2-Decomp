#ifndef HD2_GAME_MISSION_SCRIPT_MODEL_ANIMATIONS_H
#define HD2_GAME_MISSION_SCRIPT_MODEL_ANIMATIONS_H

#include "game/mission_script_context.h"

namespace hd2::game {

//00416E10 5D,00424260/00424310 and00427B80. Script owns87C model
//references plus88C cutscene flags; actual World48 independently owns ticking.
template<class Frames,class Snapshots,class Models,class Runtime>class MissionScriptModelAnimations {
public:
    using Frame=typename Frames::Frame;
    using Owner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    struct Record{Owner model;bool cutscene;};
    MissionScriptModelAnimations(Frames& frames,Snapshots& snapshots,Models& models,Runtime& runtime)
        :frames_(frames),snapshots_(snapshots),models_(models),runtime_(runtime){}
    ~MissionScriptModelAnimations(){try{clear();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}}
    const auto& records()const{return records_;}
    void add(Frame frame,bool cutscene){ //00424260, reverse identity dedup BEFORE marker/world work.
        for(std::size_t n=records_.size();n;)if(records_[--n].model.get()==frame)return;
        runtime_.script_animation_marker(frame,true);frames_.register_animated_model(frame);
        records_.push_back(Record{frames_.retain_actor_frame(frame),cutscene});
    }
    void remove(Frame frame){ //00424310: marker,World Release,Script Release, parallel swap/pop.
        for(std::size_t n=records_.size();n;)if(records_[--n].model.get()==frame){
            runtime_.script_animation_marker(frame,false);frames_.unregister_animated_model(frame);
            records_[n].model.reset();
            if(n+1!=records_.size())records_[n]=std::move(records_.back());
            records_.pop_back();return;
        }
    }
    void clear(){ //00427B80, AFTER contexts Exit/destroy. Preserve cleanup after a required failure.
        std::exception_ptr failure;
        while(!records_.empty()){
            auto& record=records_.back();const auto frame=record.model.get();
            try{runtime_.script_animation_marker(frame,false);}catch(...){if(!failure)failure=std::current_exception();}
            try{frames_.unregister_animated_model(frame);}catch(...){if(!failure)failure=std::current_exception();}
            record.model.reset();records_.pop_back();
        }
        if(failure)std::rethrow_exception(failure);
    }
    void restore(Frame model){ //00427B80 mask8;004281D0 detaches only external immediate children.
        struct Attachment{std::string parent_name;Frame frame;};
        std::vector<Attachment> attachments;
        for(std::uint32_t slot=7;slot;)frames_.human_model_animation(model,Frame{},--slot,1.f,0);
        frames_.enumerate_script_model_frames(model,[&](Frame child){
            if(frames_.frame_model_owner(child)==model)return;
            const auto parent=frames_.object_parent(child);
            if(!parent)throw std::runtime_error("Script animation attachment requires its actual parent");
            if(frames_.frame_model_owner(parent)!=model&&parent!=model)return;
            attachments.push_back(Attachment{frames_.frame_name(parent),child});
            frames_.link_scene_object(child,Frame{});
        });
        const auto resource=frames_.human_face_model_resource(model); //Copy BEFORE reload invalidates model134.
        (void)frames_.load_supply_model(model,resource,0);
        models_.script_restore_model_shadows(model);frames_.update_scene_object(model);
        for(std::size_t n=attachments.size();n;){
            const auto& attachment=attachments[--n];
            auto parent=frames_.actor_find(model,attachment.parent_name.c_str(),0xffffu);if(!parent)parent=model;
            frames_.link_scene_object(attachment.frame,parent);frames_.update_scene_object(attachment.frame);
            frames_.update_scene_object(parent);snapshots_.replace_saved_parent(attachment.frame,parent);
        }
    }
    template<class Context>std::optional<bool> execute(Context& context,const MissionScriptInstruction& instruction,
                                                                    Runtime& runtime,bool cutscene){
        if(instruction.opcode!=0x5d)return std::nullopt;
        if(!runtime.script_authority())return false;
        const auto frame=context.slot(instruction.operands.at(0));
        if(frame&&frames_.frame_kind(frame)==9){
            const auto name=context.program().strings.at(instruction.operands.at(1));
            const auto slot=context.program().integers.at(instruction.operands.at(2)).value;
            std::optional<decltype(frames_.load_supply_animation(name))> clip;
            if(name.size()>1)clip.emplace(frames_.load_supply_animation(name));
            if(clip&&*clip){
                frames_.script_animation_clip_mode(clip->get(),1);
                frames_.human_model_animation(frame,clip->get(),slot,1.f,0);clip->reset();
                frames_.human_model_time(frame,slot,0);snapshots_.remember(frame,8);add(frame,cutscene);
                frames_.update_scene_object(frame);
            }else{
                frames_.human_model_time(frame,slot,0);frames_.human_model_remove_animation(frame,slot);
                frames_.human_model_animation(frame,Frame{},slot,1.f,0);remove(frame);
            }
        }
        context.advance(runtime);return true;
    }
private:
    Frames& frames_;Snapshots& snapshots_;Models& models_;Runtime& runtime_;std::vector<Record> records_;
};
} //namespace hd2::game
#endif
