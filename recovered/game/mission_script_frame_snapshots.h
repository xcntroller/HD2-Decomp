#ifndef HD2_GAME_MISSION_SCRIPT_FRAME_SNAPSHOTS_H
#define HD2_GAME_MISSION_SCRIPT_FRAME_SNAPSHOTS_H

#include "game/mission_script_program.h"
#include <array>
#include <cstdint>
#include <exception>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace hd2::game {

struct MissionScriptLightSnapshot {std::array<float,4> colour{};std::uint32_t parameter=0;}; //Native snapshot00/30.

//00423670 and00423BF0: capture once, OR later masks, retain BOTH frame and
//original parent.00427B80 performs a visibility pass before destroying these
//records. SetOn masks20/30 and rotation mask2 are supported; other transform/
//model-animation mutators must supply their actual restoration pass first.
template<class Frames>class MissionScriptFrameSnapshots {
public:
    using Frame=typename Frames::Frame;
    using Owner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    struct Record {
        std::optional<Owner> frame,parent;
        std::array<float,4> position{},rotation{};
        MissionScriptLightSnapshot light{};
        std::uint32_t mask=0;bool visible=false;
    };
    explicit MissionScriptFrameSnapshots(Frames& frames):frames_(frames){}
    ~MissionScriptFrameSnapshots(){try{clear();}catch(...){frames_.record_actor_cleanup_failure(std::current_exception());}}
    MissionScriptFrameSnapshots(const MissionScriptFrameSnapshots&)=delete;
    MissionScriptFrameSnapshots& operator=(const MissionScriptFrameSnapshots&)=delete;
    const auto& records()const{return records_;}
    void remember(Frame frame,std::uint32_t mask) {
        if(!frame)throw std::logic_error("Script frame snapshot requires an actual frame");
        if(mask&~0x32u)throw std::runtime_error("Script frame snapshot requires actual position/animation restoration");
        for(std::size_t n=records_.size();n;)if(records_[--n]->frame->get()==frame){records_[n]->mask|=mask;return;}
        auto record=std::make_unique<Record>();record->frame.emplace(frames_.retain_actor_frame(frame));
        const auto parent=frames_.object_parent(frame);
        record->rotation=frames_.actor_rotation(frame);record->visible=frames_.car_frame_visible(frame);
        if(parent)record->parent.emplace(frames_.retain_actor_frame(parent));
        record->position=frames_.actor_position(frame);
        if(frames_.frame_kind(frame)==4)record->light=frames_.script_light_snapshot(frame);
        record->mask=mask;records_.push_back(std::move(record));
    }
    void clear() { //Supported2/20/30 subset of00427B80, then00423BF0 for EVERY record.
        std::exception_ptr failure;
        const auto attempt=[&](auto&& action){try{action();}catch(...){if(!failure)failure=std::current_exception();}};
        for(std::size_t n=records_.size();n;) {
            auto& record=*records_[--n];if(record.mask&0x20u)attempt([&]{frames_.object_visible(record.frame->get(),record.visible);});
            if(record.mask&2u)attempt([&]{frames_.object_rotation(record.frame->get(),record.rotation);});
        }
        while(!records_.empty()) {
            auto record=std::move(records_.back());records_.pop_back();attempt([&]{retire(*record);});
        }
        if(failure)std::rethrow_exception(failure);
    }
    void restore_one(Frame frame) { //00423B40: snapshot destructor only, then swap/pop (no finish prepass).
        for(std::size_t n=records_.size();n;)if(records_[--n]->frame->get()==frame) {
            auto record=std::move(records_[n]);
            if(n+1!=records_.size())records_[n]=std::move(records_.back());
            records_.pop_back();retire(*record);return;
        }
    }
    template<class Context,class Runtime>std::optional<bool> execute(Context& context,const MissionScriptInstruction& instruction,Runtime& runtime) {
        if(instruction.opcode!=0x6d)return std::nullopt; //FRM_TeleportNearFrameBack,00416E10 case1C.
        if(!runtime.script_authority())return false;
        const auto frame=context.slot(instruction.operands.at(0));if(!frame)return false;
        restore_one(frame);context.advance(runtime);return true;
    }
private:
    void retire(Record& record) { //00423BF0, even when mask20 was not set.
        std::exception_ptr failure;const auto frame=record.frame->get();
        const auto attempt=[&](auto&& action){try{action();}catch(...){if(!failure)failure=std::current_exception();}};
        attempt([&]{frames_.object_visible(frame,record.visible);});
        if(record.parent){attempt([&]{frames_.link_scene_object(frame,record.parent->get());});record.parent.reset();}
        attempt([&]{frames_.update_scene_object(frame);});
        attempt([&]{if(frames_.frame_kind(frame)==4)frames_.script_restore_light(frame,record.light);});
        attempt([&]{frames_.update_scene_object(frame);});record.frame.reset();
        if(failure)std::rethrow_exception(failure);
    }
    Frames& frames_;std::vector<std::unique_ptr<Record>> records_;
};
} //namespace hd2::game
#endif
