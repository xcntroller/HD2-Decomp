#ifndef HD2_GAME_EFFECTS_FRAME_FADES_H
#define HD2_GAME_EFFECTS_FRAME_FADES_H

#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>
#include <vector>
#include <stdexcept>

namespace hd2::game {

//GEffects200 fade-out queue00488D60 and its00487240 update pass. Actual
//positive fade-in producers/210 queue are not supplied by this reached subset;
//do not combine it with another owner that writes those same native queues.
//Frames must outlive all retained records. Explicit clear/destructor is a source
//lifetime guard, not a claim that004888C0 clears these queues (it does not).
template<class Frames>class EffectsFrameFades {
public:
    using Frame=typename Frames::Frame;
    using Owner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    struct Record {
        Owner frame;std::uint32_t remaining,duration;bool hide;
        Record(Owner value,std::uint32_t time,bool hidden):frame(std::move(value)),remaining(time),duration(time),hide(hidden){}
    };
    explicit EffectsFrameFades(Frames& frames):frames_(frames){}
    ~EffectsFrameFades(){clear();}
    const auto& records()const{return records_;}
    bool opacity(Frame frame,float amount) { //00486E60, ungated frame+A8(1-amount), not model+50 or visual+A4.
        if(!frame)return false;
        frames_.effects_frame_fade_alpha(frame,static_cast<float>(1.L-amount));return true;
    }
    void fade_out(Frame frame,std::int32_t duration,bool hide) { //00488D60; duplicate records are allowed.
        if(!frame)return;
        if(duration<=0){(void)opacity(frame,1);if(hide)frames_.object_visible(frame,false);return;}
        records_.push_back(std::make_unique<Record>(frames_.retain_actor_frame(frame),static_cast<std::uint32_t>(duration),hide));
    }
    void fade_in_immediate_tree(Frame frame,bool show,unsigned depth=0) { //00489060→00488EE0 duration<=0 only.
        if(!frame)throw std::runtime_error("Recursive frame fade-in requires an actual frame");
        if(depth>256)throw std::runtime_error("Frame fade-in hierarchy exceeds bounds");
        const auto count=frames_.actor_child_count(frame);if(count>0x100000u)throw std::runtime_error("Frame fade-in child count exceeds bounds");
        for(auto n=count;n;)fade_in_immediate_tree(frames_.actor_child(frame,--n),show,depth+1);
        switch(frames_.visual_type(frame)) {
        case 0x50414d4c:case 0x44524242:case 0x4850524d:case 0x4d474e53:
        case 0x524d4d53:case 0x5252494d:case 0x5f4a424f:
            (void)opacity(frame,0);if(show)frames_.object_visible(frame,true);break;
        default:break;
        }
        //Immediate00488EE0 does not allocate or cancel opposite records.
        //Positive fade-in/210 queue and its cancellation remain separate work.
    }
    void fade_out_tree(Frame frame,std::int32_t duration,bool hide,unsigned depth=0) { //004890F0→00488D60.
        if(!frame||depth>256)throw std::runtime_error("Recursive frame fade-out requires a bounded actual hierarchy");
        const auto count=frames_.actor_child_count(frame);if(count>0x100000u)throw std::runtime_error("Frame fade-out child count exceeds bounds");
        for(auto n=count;n;)fade_out_tree(frames_.actor_child(frame,--n),duration,hide,depth+1);
        switch(frames_.visual_type(frame)) {
        case 0x50414d4c:case 0x44524242:case 0x4850524d:case 0x4d474e53:
        case 0x524d4d53:case 0x5252494d:case 0x5f4a424f:fade_out(frame,duration,hide);break;
        default:break;
        }
    }
    void update(std::uint32_t elapsed) { //00487240: reverse traversal, signed wrapping remaining, swap/pop completion.
        for(std::size_t n=records_.size();n;) {
            auto& record=*records_[--n];record.remaining-=elapsed;
            if(signed_word(record.remaining)<=0) {
                (void)opacity(record.frame.get(),1);if(record.hide)frames_.object_visible(record.frame.get(),false);
                records_[n].reset();if(n+1!=records_.size())records_[n]=std::move(records_.back());records_.pop_back();
            }else {
                const auto amount=record.duration?static_cast<float>(1.L-static_cast<long double>(signed_word(record.remaining))/signed_word(record.duration)):0.f;
                (void)opacity(record.frame.get(),amount);
            }
        }
    }
    void clear()noexcept{while(!records_.empty())records_.pop_back();}
private:
    static std::int32_t signed_word(std::uint32_t bits){std::int32_t result;std::memcpy(&result,&bits,4);return result;}
    Frames& frames_;std::vector<std::unique_ptr<Record>> records_;
};
} //namespace hd2::game
#endif
