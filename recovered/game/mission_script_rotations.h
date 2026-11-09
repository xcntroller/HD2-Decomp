#ifndef HD2_GAME_MISSION_SCRIPT_ROTATIONS_H
#define HD2_GAME_MISSION_SCRIPT_ROTATIONS_H

#include "game/mission_script_context.h"

namespace hd2::game {

//00428B30's cutscene rectangle. Width/height are unsigned FILD64 inputs;
//image height and margin spill to FLOAT, bottom remains extended until ftol.
inline std::array<std::int32_t,4> mission_script_cutscene_viewport(std::uint32_t width,std::uint32_t height) {
    const float image=static_cast<float>(static_cast<long double>(width)*.0625f*9.f);
    const float margin=static_cast<float>((static_cast<long double>(height)-image)*.5f);
    const auto convert=[](long double value){const auto bits=static_cast<std::uint32_t>(static_cast<std::int64_t>(value));std::int32_t result;std::memcpy(&result,&bits,4);return result;};
    std::int32_t right;std::memcpy(&right,&width,4);
    return {{0,convert(margin),right,convert(static_cast<long double>(height)-margin)}};
}

//Module4's84C retained rotation records.004274A0 replaces an existing
//record's parameters without taking another reference or snapshot. The actual
//00428B30 frame-update slot is separate from00428490's script instruction tick.
template<class Frames,class Snapshots>class MissionScriptRotations {
public:
    using Frame=typename Frames::Frame;
    using Owner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    struct Record {
        Owner frame;std::uint32_t axis;float remaining,rate;
        Record(Owner value,std::uint32_t direction,float angle,float speed)
            :frame(std::move(value)),axis(direction),remaining(angle),rate(speed){}
    };
    MissionScriptRotations(Frames& frames,Snapshots& snapshots):frames_(frames),snapshots_(snapshots){}
    ~MissionScriptRotations(){clear();}
    const auto& records()const{return records_;}
    void clear()noexcept{while(!records_.empty())records_.pop_back();} //00428D00, reverse retained-frame release.
    void configure(Frame frame,std::uint32_t axis,std::int32_t angle,std::int32_t speed) { //004274A0/00428CC0.
        if(!frame)throw std::logic_error("Script rotation requires an actual frame");
        const auto remaining=radians(angle),rate=radians(speed);
        for(std::size_t n=records_.size();n;)if(records_[--n]->frame.get()==frame) {
            auto& record=*records_[n];record.axis=axis;record.remaining=remaining;record.rate=rate;return;
        }
        auto record=std::make_unique<Record>(frames_.retain_actor_frame(frame),axis,remaining,rate);
        snapshots_.remember(frame,2);records_.push_back(std::move(record));
    }
    template<class Context,class Runtime>std::optional<bool> execute(Context& context,const MissionScriptInstruction& instruction,Runtime& runtime) {
        if(instruction.opcode<0x74||instruction.opcode>0x76)return std::nullopt;
        if(!runtime.script_authority())return false;
        //The native helper is called even for a null slot (which would fault in
        //00428CC0); reject that undefined branch instead of advancing successfully.
        const auto frame=context.slot(instruction.operands.at(0));
        const auto& values=context.program().integers;
        configure(frame,instruction.opcode-0x74,signed_word(values.at(instruction.operands.at(1)).value),
                  signed_word(values.at(instruction.operands.at(2)).value));
        context.advance(runtime);return true;
    }
    void update(std::int32_t elapsed) { //00428B30's reverse step→release→swap/pop pass.
        for(std::size_t n=records_.size();n;)if(!step(*records_[--n],elapsed)) {
            records_[n].reset();
            if(n+1!=records_.size())records_[n]=std::move(records_.back());
            records_.pop_back();
        }
    }
private:
    bool step(Record& record,std::int32_t elapsed) { //00427200.
        const auto frame=record.frame.get();(void)frames_.actor_rotation(frame); //Lazy quaternion cache BEFORE elapsed math.
        auto increment=(static_cast<long double>(elapsed)*.0010000000474974513f)*std::abs(static_cast<long double>(record.rate));
        const bool negative=record.remaining<0;
        const auto remaining=static_cast<long double>(record.remaining)+(negative?increment:-increment);
        record.remaining=static_cast<float>(remaining);bool active=true;
        if((!negative&&remaining<0)||(negative&&remaining>0)) { //Strict crossing; equality stays active until a later update.
            active=false;increment+=negative?-remaining:remaining; //Clamp actual applied angle, not the stored remainder.
        }
        const float angle=static_cast<float>(negative?-increment:increment);
        frames_.script_rotate_frame(frame,record.axis,angle); //Actual RotateX/Y/ZMatrix *= frame local matrix; GetScale, update.
        return active;
    }
    static float radians(std::int32_t value){return static_cast<float>(static_cast<long double>(value)*.01745329238474369f);}
    static std::int32_t signed_word(std::uint32_t word){std::int32_t result;std::memcpy(&result,&word,4);return result;}
    Frames& frames_;Snapshots& snapshots_;std::vector<std::unique_ptr<Record>> records_;
};
} //namespace hd2::game
#endif
