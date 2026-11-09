#ifndef HD2_GAME_MISSION_SCRIPT_PROGRAM_H
#define HD2_GAME_MISSION_SCRIPT_PROGRAM_H

#include "game/mission_script_commands.h"
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <memory>
#include <vector>

namespace hd2::game {
struct MissionScriptExpression;

//00414610/00414660/00416700 instruction image. Original opcode+4 allocations
//are owned values; operands are native pool/PC indices, never host pointers.
//The actual frame/context and VM consumers must own any populated frame slots.
struct MissionScriptInstruction {std::uint32_t opcode=0;std::vector<std::uint32_t> operands;};
struct MissionScriptProgram {
    struct Integer {std::uint32_t value;bool reusable;}; //Native A0 record8.
    struct Event {std::uint32_t identity;std::int32_t pc;};
    struct Whenever {std::uint32_t expression;std::int32_t pc;bool enabled=true;std::uint32_t period=500,elapsed=0;};
    std::vector<MissionScriptInstruction> instructions; //Native60.
    std::vector<std::shared_ptr<MissionScriptExpression>> expressions; //Native70, typed owned roots.
    std::vector<Whenever> whens; //Native80 owns0x14-byte records.
    std::vector<Integer> integers; //NativeA0, compiler literals are reusable=false.
    std::vector<std::string> strings; //NativeB0, case-insensitive reverse reuse.
    std::size_t frame_slots=0; //Native90 slots are initially NULL; runtime ownership is separate.
    std::array<std::int32_t,8> events{{-1,-1,-1,-1,-1,-1,-1,-1}}; //DC..F8.
    std::vector<Event> signals,cutscenes,cutscene_done; //30/40/50 owned pair8 records.
    std::int32_t start=-1; //Native1C.
    void clear(){instructions.clear();expressions.clear();whens.clear();integers.clear();strings.clear();signals.clear();cutscenes.clear();cutscene_done.clear();frame_slots=0;events.fill(-1);start=-1;}
    std::int32_t append(std::uint32_t opcode,std::vector<std::uint32_t> operands={}) {
        if(instructions.size()>=std::size_t(std::numeric_limits<std::int32_t>::max()))throw std::length_error("Script instruction count exceeds native PC");
        instructions.push_back({opcode,std::move(operands)});return static_cast<std::int32_t>(instructions.size()-1);
    }
};

//0042D920 numeric conversion: strtod then007E5E00 truncating signed64/EAX.
//Invalid integer-indefinite yields low DWORD0; integer literals are unmasked.
inline std::uint32_t mission_script_integer_word(double value) {
    const auto number=static_cast<long double>(value),minimum=static_cast<long double>(std::numeric_limits<std::int64_t>::min());
    if(!std::isfinite(number)||number<minimum||number>=-minimum)return 0;
    return static_cast<std::uint32_t>(static_cast<std::int64_t>(number));
}

//Compiler symbol and operand owner shared with the later expression parser.
//0042CEB0/0042D890 read reverse, and numeric literals preserve the native
//reusable flag test rather than silently deduplicating all equal constants.
class MissionScriptSymbols {
public:
    struct Variable {std::string name;std::uint32_t kind;std::int32_t index;}; //Compiler414 records12.
    explicit MissionScriptSymbols(MissionScriptProgram& program):program_(program){}
    const auto& variables()const{return variables_;}
    void clear(){variables_.clear();}
    void publish(std::string name,std::uint32_t kind,std::int32_t index){variables_.push_back({std::move(name),kind,index});}
    const Variable* variable(std::string_view name,std::uint32_t kind=0)const {
        for(std::size_t n=variables_.size();n;) {
            const auto& v=variables_[--n];
            if((!kind||kind==v.kind)&&MissionScriptLexer::equal(v.name,name))return &v;
        }
        return nullptr;
    }
    bool declare(std::string name,std::uint32_t declaration) { //0042ECD0 ordinary Integer/String/Frame declarations.
        if(variable(name))return false;
        std::uint32_t kind=0;std::int32_t index=-1;
        if(declaration==0x46){kind=2;index=static_cast<std::int32_t>(program_.integers.size());program_.integers.push_back({0,false});}
        else if(declaration==0x49){kind=4;index=static_cast<std::int32_t>(program_.strings.size());program_.strings.emplace_back();}
        else if(declaration==0x4b){kind=5;index=static_cast<std::int32_t>(program_.frame_slots++);}
        else return false; //Bool/Enum/Real declaration consumers remain outside this supported source branch.
        variables_.push_back({std::move(name),kind,index});return true;
    }
    std::int32_t operand(std::uint32_t kind,std::string token,MissionScriptLexer& lexer,bool resolve_variable=true) { //0042D890→0042D920.
        if(resolve_variable)if(const auto* v=variable(token,kind))return v->index;
        if(kind==2) {
            const bool negative=token=="-";if(negative){lexer.next();token=lexer.token();}
            char* end=nullptr;const auto number=std::strtod(token.c_str(),&end);
            std::uint32_t word=mission_script_integer_word(number);
            if(end<token.c_str()+token.size()) {
                if(MissionScriptLexer::equal(token,"true"))word=1;
                else if(MissionScriptLexer::equal(token,"false"))word=0;
                else return -1;
            }
            if(negative)word=0u-word;
            for(std::size_t n=program_.integers.size();n;) {
                const auto& record=program_.integers[--n];if(record.value==word&&record.reusable)return static_cast<std::int32_t>(n);
            }
            if(program_.integers.size()>=std::size_t(std::numeric_limits<std::int32_t>::max()))throw std::length_error("Script integer pool exceeds native index");
            const auto index=static_cast<std::int32_t>(program_.integers.size());program_.integers.push_back({word,false});return index;
        }
        if(kind==4&&token.size()>=2&&token.front()=='"'&&token.back()=='"') {
            //Original unbounded formatted copy/heap ABI is excluded. Actual
            //quoted payload remains verbatim, with no invented backslash escapes.
            auto text=token.substr(1,token.size()-2);
            for(std::size_t n=program_.strings.size();n;)if(MissionScriptLexer::equal(program_.strings[--n],text))return static_cast<std::int32_t>(n);
            if(program_.strings.size()>=std::size_t(std::numeric_limits<std::int32_t>::max()))throw std::length_error("Script string pool exceeds native index");
            const auto index=static_cast<std::int32_t>(program_.strings.size());program_.strings.push_back(std::move(text));return index;
        }
        return -1;
    }
private:
    MissionScriptProgram& program_;std::vector<Variable> variables_;
};
} //namespace hd2::game
#endif
