#ifndef HD2_GAME_MISSION_SCRIPT_CONTEXT_H
#define HD2_GAME_MISSION_SCRIPT_CONTEXT_H

#include "game/mission_script_expression.h"
#include <optional>

namespace hd2::game {

//0041DCB0/0041DF20: the small script context is independent of the larger
//004CE8F0 actor/AI context registry. Frames supplies the SAME frame EC block
//used by actors. Native pool90 references have their own retained lifetimes.
template<class Frames>class MissionScriptContext {
public:
    using Frame=typename Frames::Frame;
    using FrameOwner=decltype(std::declval<Frames&>().retain_actor_frame(std::declval<Frame>()));
    MissionScriptContext(Frames& frames,Frame frame,bool borrowed=false):frames_(frames),frame_(frame),borrowed_(borrowed) {
        if(frame_&&!borrowed_) {
            auto& data=frames_.actor_frame_data(frame_);
            if(data.words[1])throw std::runtime_error("Frame already owns an actual script context");
            owner_.emplace(frames_.retain_actor_frame(frame_));
            data.words[1]=reinterpret_cast<std::uintptr_t>(this);
        }
    }
    virtual ~MissionScriptContext() {
        clear();
        if(frame_&&!borrowed_) {
            auto& data=frames_.actor_frame_data(frame_);data.words[1]=0;
            owner_.reset();
        }
    }
    MissionScriptContext(const MissionScriptContext&)=delete;
    MissionScriptContext& operator=(const MissionScriptContext&)=delete;
    Frame frame()const{return frame_;}
    virtual std::uint32_t family()const{return 0;} //0041DF10, actual context virtual+04.
    MissionScriptProgram& program(){return program_;}
    const MissionScriptProgram& program()const{return program_;}
    std::int32_t pc()const{return pc_;}
    bool ready()const{return ready_;}
    bool ended()const{return ended_;}
    std::size_t slot_count()const{return frame_slots_.size();} //004202F0 pool90 count.
    bool during_cutscene()const{return flags_[5];} //Native byte9, module00428490 filter.
    bool& saved_first_person(){return flags_[3];} //Native byte7,00416E10 SetActiveCamera.
    bool& saved_secondary_view(){return flags_[4];} //Native byte8.
    std::uint32_t elapsed_work()const{return work_microseconds_;} //NativeD0.
    Frame slot(std::size_t n)const{return frame_slots_.at(n)?frame_slots_[n]->get():Frame{};}
    void slot(std::size_t n,Frame value) {
        auto& entry=frame_slots_.at(n);entry.reset(); //Native releases old BEFORE acquiring new.
        if(value)entry.emplace(frames_.retain_actor_frame(value));
    }
    void clear() { //0041E140: clear actual references before their containing world dies.
        program_.clear();returns_.clear();field10_=field14_=0;
        for(std::size_t n=frame_slots_.size();n;)frame_slots_[--n].reset();
        frame_slots_.clear();
    }
    void begin(bool authority) { //0041D8D0: no instruction is executed here.
        if(!authority)return;
        if(frame_slots_.empty())frame_slots_.resize(program_.frame_slots);
        else if(frame_slots_.size()!=program_.frame_slots)throw std::logic_error("Script recompilation requires clearing retained frame slots");
        pending_pc_=-1;delay_=0;pc_=program_.start==-1?0:program_.start;
        ready_=true;flags_.fill(false);
    }
    template<class Runtime>void exit(Runtime& runtime) { //0041DB80→0041DC00.
        ready_=true;pc_=program_.events[7];if(pc_==-1)return;
        --pc_;block(runtime);
    }
    template<class Runtime>void death(Runtime& runtime) { //0041D920.
        if(!runtime.script_authority())return;
        if(program_.events[0]==-1){finish(runtime);return;}
        pc_=program_.events[0];delay_=0;ready_=true;ended_=false;
    }
    void use(bool authority) { //0041D960; neither use nor hit clears delay/ended.
        if(authority&&(!ready_||(pc_!=program_.events[0]&&pc_!=program_.events[3]))&&program_.events[1]!=-1){pc_=program_.events[1];ready_=true;}
    }
    void hit(bool authority) { //0041DBB0.
        if(authority&&(!ready_||(pc_!=program_.events[0]&&pc_!=program_.events[3]))&&program_.events[2]!=-1){pc_=program_.events[2];ready_=true;}
    }
    void alarm(bool authority) { //0041DA20: native byte5 blocks this event.
        if(authority&&(ended_||!ready_||pc_!=program_.events[0])&&!flags_[1]&&program_.events[3]!=-1){pc_=program_.events[3];delay_=0;ready_=true;}
    }
    bool alarm_done(bool authority) { //0041DA80.
        if(!authority||(ready_&&pc_==program_.events[0])||program_.events[4]==-1)return false;
        pc_=program_.events[4];delay_=0;ready_=true;ended_=false;return true;
    }
    bool cutscene(std::uint32_t identity,bool authority) { //0041D9B0.
        if(!authority)return false;
        for(std::size_t n=program_.cutscenes.size();n;)if(program_.cutscenes[--n].identity==identity){enter_event(program_.cutscenes[n].pc);flags_[5]=true;return true;}
        return false;
    }
    bool cutscene_done(std::uint32_t identity,bool authority) { //0041DAE0.
        if(!authority)return false;
        for(std::size_t n=program_.cutscene_done.size();n;)if(program_.cutscene_done[--n].identity==identity){enter_event(program_.cutscene_done[n].pc);flags_[5]=false;return true;}
        return false;
    }
    void load(bool authority) { //0041DB50: preserves delay/ended.
        if(authority&&program_.events[6]!=-1){pc_=program_.events[6];ready_=true;}
    }
    template<class Runtime>void signal(std::int32_t identity,Runtime& runtime) { //00416610.
        if(flags_[0]||!runtime.script_authority()||(ready_&&(pc_==program_.events[0]||pc_==program_.events[3])))return;
        const auto mask=std::uint32_t{1}<<(static_cast<std::uint32_t>(identity)&31);
        if(field14_&mask)return;
        for(std::size_t n=program_.signals.size();n;)if(program_.signals[--n].identity==static_cast<std::uint32_t>(identity)) {
            //Actual frame-EC actor type3 additionally requires B0 kind5.
            if(!runtime.script_accept_signal(*this))return;
            ready_=true;delay_=0;pc_=program_.signals[n].pc;return;
        }
        if(identity<32)field10_|=mask; //Signed native comparison; negative IDs retain x86 shift masking.
    }
    bool consume_signal(std::uint32_t identity) { //0041E3F0 case9, _SignalReceived.
        const auto mask=std::uint32_t{1}<<(identity&31);if(!(field10_&mask))return false;
        field10_&=~mask;return true;
    }
    //004167B0 advances ONE instruction per containing module tick. Its 500ms
    //tests are strict signed >, with elapsed added AFTER the periodic checks.
    //Runtime supplies actual predicates, command receivers and measured clock.
    template<class Runtime>void step(std::uint32_t elapsed,Runtime& runtime) {
        if(!runtime.script_authority())return;
        if(pending_pc_!=-1)throw std::runtime_error("Pending script stream operation needs its actual receiver");
        if(ended_)return;
        const auto started=runtime.script_microseconds();
        if(!flags_[2]) {
            while(signed_word(period_elapsed_)>500) {
                for(std::size_t n=program_.whens.size();n;) {
                    auto& when=program_.whens[--n];if(!when.enabled)continue;
                    when.elapsed+=500;
                    if(signed_word(when.elapsed)<=signed_word(when.period))continue;
                    when.elapsed-=when.period;
                    if(!number(when.expression,runtime))continue;
                    ready_=true;pc_=when.pc;when.enabled=false;delay_=0;
                }
                period_elapsed_-=500;
            }
            period_elapsed_+=elapsed;
        }
        if(delay_) {
            delay_-=elapsed;
            if(signed_word(delay_)>0){finish_clock(started,runtime);return;}
            delay_=0;ready_=true;++pc_;
        }
        if(ready_&&valid_pc()) {
            ready_=false;
            const auto& instruction=program_.instructions.at(static_cast<std::size_t>(pc_));
            switch(instruction.opcode) {
            case 0x27:delay_=integer(instruction,0);break;
            case 0x4d:pc_=signed_word(instruction.operands.at(0));ready_=true;break;
            case 0x50:case 0x53:case 0x55:case 0x56:case 0x57:case 0x58:case 0x59:case 0x5a:case 0x5b:case 0x5c:case 0xa4:
                pc_=after_block(pc_+1);
                if(valid_pc())ready_=true;else{pc_=last_pc();ready_=false;}
                break;
            case 0x51:
                if(returns_.size()<100){returns_.push_back(pc_+1);pc_=signed_word(instruction.operands.at(0));ready_=true;}
                break;
            case 0x52:
                if(returns_.empty())finish(runtime);
                else {pc_=returns_.back();returns_.pop_back();ready_=true;}
                break;
            case 0x112:block(runtime);break;
            default:execute(instruction,runtime);break;
            }
        }
        finish_clock(started,runtime);
    }
    template<class Runtime>void advance(Runtime& runtime) { //00416CF0 guarded by008692D0/game28.
        if(!runtime.script_authority()||!runtime.script_can_advance())return;
        ++pc_;ready_=valid_pc();
    }
    void defer_actor_suspend() { //00421C00/00421970 first suspend invocation.
        delay_=2000;--pc_;ready_=true;
    }
    //0041DC00 synchronous Block/exit interpreter. A malformed/looping script
    //must not lock the host; the bounded fault adds no original behavior credit.
    template<class Runtime>void block(Runtime& runtime) {
        if(!runtime.script_authority())return;
        const auto start=++pc_;const auto end=after_block(start);
        std::size_t budget=0;
        while(valid_pc()) {
            if(pc_<start||pc_>=end){ready_=true;return;}
            if(++budget>1000000)throw std::runtime_error("Script synchronous block exceeds bounded execution");
            const auto previous=pc_;const auto& instruction=program_.instructions.at(static_cast<std::size_t>(pc_));
            if(instruction.opcode!=0x105&&instruction.opcode!=0x106)execute(instruction,runtime);
            if(pc_==previous)++pc_;
        }
    }
private:
    template<class Runtime>struct Predicates {
        MissionScriptContext& context;Runtime& runtime;
        std::uint32_t evaluate_script_predicate(std::uint32_t opcode,const std::array<std::uint32_t,4>& values,const std::array<std::uint32_t,4>& kinds,const MissionScriptProgram& program) {
            if(!runtime.script_authority())return 0;
            if(opcode==9)return context.consume_signal(program.integers.at(values[0]).value);
            return runtime.evaluate_script_predicate(context,opcode,values,kinds,program);
        }
    };
    template<class Runtime>std::uint32_t number(std::uint32_t index,Runtime& runtime){Predicates<Runtime> predicates{*this,runtime};return expression(index).number(&program_,predicates);}
    void enter_event(std::int32_t pc){ready_=true;delay_=0;ended_=false;pc_=pc;}
    template<class Runtime>void finish(Runtime& runtime){ready_=false;ended_=true;runtime.script_return_end(*this);} //00416DB0 Human B0-kind5 cleanup.
    template<class Runtime>void execute(const MissionScriptInstruction& instruction,Runtime& runtime) { //00416E10 core cases, ORIGINAL opcodes.
        switch(instruction.opcode) {
        case 0x4e:
            if(number(instruction.operands.at(0),runtime)){++pc_;ready_=true;return;}
            pc_=after_block(pc_+1);
            if(!valid_pc()){pc_=last_pc();ready_=false;return;}
            if(program_.instructions.at(static_cast<std::size_t>(pc_)).opcode==0x4f)++pc_;
            ready_=true;return;
        case 0x4f:
            pc_=after_block(pc_+1);ready_=valid_pc();if(!ready_)pc_=last_pc();return;
        case 0x50: //00416E10 case0D, reached through synchronous block dispatch.
            pc_=after_block(pc_+1);advance(runtime);return;
        case 0x105:case 0x106:++pc_;ready_=true;return;
        case 0x107: {
            const auto& root=expression(instruction.operands.at(0));const auto kind=instruction.operands.at(1),index=instruction.operands.at(2);
            if(kind==2)program_.integers.at(index).value=number(instruction.operands.at(0),runtime);
            else if(kind==4)program_.strings.at(index)=root.string(&program_);
            ++pc_;ready_=true;return;
        }
        case 0x78: {
            const auto index=instruction.operands.at(0);
            if(index<frame_slots_.size())slot(index,frame_);
            advance(runtime);return;
        }
        case 0x8b:program_.whens.at(instruction.operands.at(0)).enabled=integer(instruction,1)!=0;advance(runtime);return;
        case 0x8e:program_.whens.at(instruction.operands.at(0)).period=integer(instruction,1);advance(runtime);return;
        case 0x96:flags_[0]=integer(instruction,0)!=0;if(flags_[0])field10_=0;advance(runtime);return;
        case 0x97:flags_[1]=integer(instruction,0)!=0;advance(runtime);return;
        case 0x98:flags_[2]=integer(instruction,0)!=0;advance(runtime);return;
        case 0x99: {
            const auto mask=std::uint32_t{1}<<(integer(instruction,0)&31);
            if(integer(instruction,1))field14_&=~mask;else field14_|=mask;
            advance(runtime);return;
        }
        default:runtime.execute_script_command(*this,instruction);return; //Required actual base/Human/Dog receivers; no default success.
        }
    }
    std::uint32_t integer(const MissionScriptInstruction& instruction,std::size_t n)const{return program_.integers.at(instruction.operands.at(n)).value;}
    const MissionScriptExpression& expression(std::uint32_t n)const {
        const auto& value=program_.expressions.at(n);if(!value)throw std::runtime_error("Script expression owner is missing");return *value;
    }
    bool valid_pc()const{return static_cast<std::uint32_t>(pc_)<program_.instructions.size();}
    std::int32_t last_pc()const{return program_.instructions.empty()?-1:static_cast<std::int32_t>(program_.instructions.size()-1);}
    std::int32_t after_block(std::int32_t start)const {
        const auto& open=program_.instructions.at(static_cast<std::size_t>(start));
        if(open.opcode!=0x105||open.operands.empty())throw std::runtime_error("Script control flow has no actual block payload");
        const auto end=std::int64_t(start)+open.operands[0]+2;
        if(end>std::numeric_limits<std::int32_t>::max())throw std::runtime_error("Script block exceeds native PC");
        return static_cast<std::int32_t>(end);
    }
    static std::int32_t signed_word(std::uint32_t value){std::int32_t result;std::memcpy(&result,&value,4);return result;}
    template<class Runtime>void finish_clock(std::uint64_t started,Runtime& runtime){work_microseconds_=static_cast<std::uint32_t>(runtime.script_microseconds()-started);}
    Frames& frames_;Frame frame_;bool borrowed_;std::optional<FrameOwner> owner_;
    MissionScriptProgram program_;std::vector<std::optional<FrameOwner>> frame_slots_;std::vector<std::int32_t> returns_;
    std::array<bool,6> flags_{};std::int32_t pending_pc_=-1,pc_=-1;
    std::uint32_t delay_=0,period_elapsed_=0,field10_=0,field14_=0,work_microseconds_=0;
    bool ready_=false,ended_=false;
};
} //namespace hd2::game
#endif
