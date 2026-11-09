#ifndef HD2_GAME_MISSION_SCRIPT_COMPILER_H
#define HD2_GAME_MISSION_SCRIPT_COMPILER_H

#include "game/mission_script_expression_parser.h"
#include <algorithm>

namespace hd2::game {

//0042CB80/0042CF90/0042C850 source front end. It constructs the actual ordered
//instruction/value/symbol image; runtime context/VM execution is a separate
//required consumer. Unsupported declaration/value families fail explicitly.
//File loading and #include expansion must precede this prepared-source entry.
class MissionScriptCompiler {
public:
    explicit MissionScriptCompiler(MissionScriptProgram& program):program_(program),symbols_(program){}
    const MissionScriptSymbols& symbols()const{return symbols_;}
    const std::string& error()const{return error_;}
    bool compile(std::string source) {
        program_.clear();symbols_.clear();labels_.clear();blocks_.clear();error_.clear();
        MissionScriptLexer lexer(std::move(source));
        for(;;) {
            lexer.next();
            if(lexer.token().empty())break;
            if(!statement(lexer)) {
                if(error_.empty())error_="Invalid script token: "+lexer.token()+" at CR line "+std::to_string(lexer.state().line)+" byte "+(lexer.state().cursor?std::to_string(*lexer.state().cursor):"EOF");
                return false;
            }
            if(!lexer.state().cursor)break;
        }
        return validate();
    }
private:
    struct Label {std::string name;std::int32_t pc;};
    bool statement(MissionScriptLexer& lexer) { //0042C0B0.
        const auto token=lexer.token();
        if(token.size()>1&&token[0]=='/'&&token[1]=='/')return true;
        const auto* command=mission_script_command(token);const auto opcode=command?command->opcode:0u;
        switch(opcode) {
        case 0:
            if(const auto* variable=symbols_.variable(token))return assignment(lexer,*variable);
            error_="Unknown script command: "+token;return false;
        case 0x46:case 0x48:case 0x49:case 0x4a:case 0x4b: {
            lexer.next();const auto name=lexer.token();if(!symbols_.declare(name,opcode))return false;
            const auto variable=*symbols_.variable(name);
            lexer.next();
            if(lexer.token()=="="&&(opcode==0x46||opcode==0x49))return assignment_expression(lexer,variable);
            return lexer.token()==";";
        }
        case 0x4c:return label(lexer);
        case 0x4d:case 0x51:return jump(lexer,opcode);
        case 0x4e:return conditional(lexer);
        case 0x50:return whenever(lexer);
        case 0x8b:case 0x8e:return set_whenever(lexer,opcode);
        case 0x4f:case 0x112:program_.append(opcode);return true; //0042E3D0/0042E340.
        case 0x52:program_.append(opcode);return lexer.expect(";"); //0042E380.
        case 0x53:case 0xa4:case 0x5c:return numeric_event(lexer,opcode);
        case 0x54: //0042E410: ScriptStartPoint has a colon and emits NO instruction.
            if(!lexer.expect(":"))return false;
            if(program_.start!=-1)return false;
            program_.start=pc();return true;
        case 0x55:case 0x56:case 0x57:case 0x58:case 0x59:case 0x5a:case 0x5b:
            return simple_event(lexer,opcode);
        case 0x105: //0042D740 opening brace, payload allocated only when its close is parsed.
            blocks_.push_back(program_.append(opcode));return true;
        case 0x106: {
            if(blocks_.empty())return false;
            const auto start=blocks_.back();blocks_.pop_back();
            program_.instructions.at(static_cast<std::size_t>(start)).operands={static_cast<std::uint32_t>(pc()-start-1)};
            program_.append(opcode);return true;
        }
        case 0x107:case 0x108:case 0x109:case 0x10a:case 0x10b:case 0x10c:case 0x10d:case 0x10e:case 0x10f:case 0x110:return false;
        default:return normal_command(lexer,opcode);
        }
    }
    std::int32_t expression(MissionScriptLexer& lexer) { //0042D170→00416750.
        MissionScriptExpressionParser parser(program_,symbols_);auto root=parser.parse(lexer);
        if(!root)return -1;
        const auto index=static_cast<std::int32_t>(program_.expressions.size());program_.expressions.push_back(std::move(root));return index;
    }
    bool assignment(MissionScriptLexer& lexer,MissionScriptSymbols::Variable variable) { //0042E160.
        if(!lexer.expect("="))return false;
        return assignment_expression(lexer,variable);
    }
    bool assignment_expression(MissionScriptLexer& lexer,const MissionScriptSymbols::Variable& variable) {
        const auto index=expression(lexer);if(index<0)return false;
        program_.append(0x107,{word(index),variable.kind,word(variable.index)});
        return true; //0042ECD0/0042E160 do not demand a semicolon after the expression; ')' or '}' may be returned to the caller.
    }
    bool conditional(MissionScriptLexer& lexer) { //0042E2A0.
        (void)lexer.expect("(");const auto index=expression(lexer);if(index<0)return false;
        program_.append(0x4e,{word(index)});if(!lexer.expect(")"))return false;
        const auto cursor=lexer.state().cursor;
        if(!lexer.expect("{"))return false;
        auto state=lexer.state();state.cursor=cursor;lexer.state(state);return true; //Native restores pointer only; CR counters retain lookahead.
    }
    bool whenever(MissionScriptLexer& lexer) { //0042E7C0, actual80 record and compiler414 kind8 identity.
        lexer.next();const auto name=lexer.token();
        if(const auto* old=symbols_.variable(name))if(old->kind==8&&old->index>=0)return false;
        (void)lexer.expect("(");const auto index=expression(lexer);if(index<0)return false;
        program_.append(0x50,{word(index)});
        const auto identity=static_cast<std::int32_t>(program_.whens.size());
        program_.whens.push_back({word(index),pc(),true,500,0});symbols_.publish(name,8,identity);
        //Native patches SetWhenever(8B) forward operands only.8E forward-time
        //references remain negative and fail0042C850's explicit final check.
        for(auto& instruction:program_.instructions)if(instruction.opcode==0x8b&&!instruction.operands.empty()) {
            const auto reference=signed_word(instruction.operands[0]);
            if(reference<0) {
                const auto slot=static_cast<std::size_t>(-1-static_cast<std::int64_t>(reference));
                if(slot>=symbols_.variables().size())throw std::runtime_error("Whenever forward symbol exceeds actual records");
                if(MissionScriptLexer::equal(symbols_.variables()[slot].name,name))instruction.operands[0]=word(identity);
            }
        }
        return lexer.expect(")");
    }
    bool set_whenever(MissionScriptLexer& lexer,std::uint32_t opcode) { //0042F530/0042F920.
        if(!lexer.expect("("))return false;
        lexer.next();const auto name=lexer.token();
        std::int32_t identity=-1;
        if(const auto* variable=symbols_.variable(name)) {
            if(variable->kind!=8)return false;
            identity=variable->index;
            if(identity==-1)identity=-1-static_cast<std::int32_t>(variable-symbols_.variables().data());
        }else {identity=-1-static_cast<std::int32_t>(symbols_.variables().size());symbols_.publish(name,8,-1);}
        if(!lexer.expect(","))return false;
        lexer.next();const auto value=symbols_.operand(2,lexer.token(),lexer,false);if(value<0)return false;
        program_.append(opcode,{word(identity),word(value)}); //D920 called directly; second parameter never resolves a variable.
        return lexer.expect(")")&&lexer.expect(";");
    }
    bool normal_command(MissionScriptLexer& lexer,std::uint32_t opcode) { //0042EC20/0042DD40.
        if(!lexer.expect("("))return false;
        std::vector<std::uint32_t> operands;
        const auto count=mission_script_argument_count(opcode);operands.reserve(count);
        for(unsigned n=0;n!=count;++n) {
            lexer.next();const auto index=symbols_.operand(mission_script_argument_kind(opcode,n),lexer.token(),lexer);
            if(index==-1)return false;
            operands.push_back(static_cast<std::uint32_t>(index));
            if(n+1<count&&!lexer.expect(","))return false;
        }
        if(!lexer.expect(")")||!lexer.expect(";"))return false;
        program_.append(opcode,std::move(operands));return true;
    }
    bool simple_event(MissionScriptLexer& lexer,std::uint32_t opcode) { //0042E730.
        if(!lexer.expect("(")||!lexer.expect(")"))return false;
        const auto event=opcode==0x55?0u:opcode==0x56?1u:opcode==0x57?3u:opcode==0x58?2u:
            opcode==0x59?6u:opcode==0x5a?7u:4u;
        program_.append(opcode);program_.events.at(event)=pc();return true;
    }
    bool numeric_event(MissionScriptLexer& lexer,std::uint32_t opcode) { //0042E460/0042E550/0042E640.
        (void)lexer.expect("("); //Native ignores ONLY this first comparison result.
        lexer.next();const auto text=lexer.token();char* end=nullptr;
        const auto value=std::strtod(text.c_str(),&end);const auto identity=mission_script_integer_word(value);
        if(end<text.c_str()+text.size()||!lexer.expect(")"))return false;
        program_.append(opcode,{identity});
        auto& list=opcode==0x53?program_.signals:opcode==0xa4?program_.cutscenes:program_.cutscene_done;
        //00415FE0/004161F0/00416400 refuse duplicate IDs; parser deliberately
        //ignores that return, retaining the first mapping and BOTH markers.
        for(std::size_t n=list.size();n;)if(list[--n].identity==identity)return true;
        list.push_back({identity,pc()});return true;
    }
    bool jump(MissionScriptLexer& lexer,std::uint32_t opcode) { //0042F2E0/0042FD10.
        lexer.next();const auto name=lexer.token();std::int32_t found=-1;
        for(std::size_t n=labels_.size();n;)if(MissionScriptLexer::equal(labels_[--n].name,name)){found=static_cast<std::int32_t>(n);break;}
        std::int32_t target;
        if(found<0) {target=-1-static_cast<std::int32_t>(labels_.size());labels_.push_back({name,-1});}
        else if(labels_[static_cast<std::size_t>(found)].pc==-1)target=-1-found;
        else {target=labels_[static_cast<std::size_t>(found)].pc;labels_.push_back({name,target});}
        program_.append(opcode,{word(target)});return lexer.expect(";");
    }
    bool label(MissionScriptLexer& lexer) { //0042F0A0, forward patches point AFTER previous instruction.
        lexer.next();const auto name=lexer.token();std::int32_t found=-1;
        for(std::size_t n=labels_.size();n;)if(MissionScriptLexer::equal(labels_[--n].name,name)){found=static_cast<std::int32_t>(n);break;}
        if(found<0)labels_.push_back({name,pc()});
        else {
            auto& label=labels_[static_cast<std::size_t>(found)];if(label.pc!=-1)return false;
            for(auto& instruction:program_.instructions)if((instruction.opcode==0x4d||instruction.opcode==0x51)&&
                    !instruction.operands.empty()&&instruction.operands[0]==word(-1-found)) {
                instruction.operands[0]=word(pc());label.pc=pc();
            }
        }
        return lexer.expect(":");
    }
    bool validate() { //0042C850 postparse checks; compilation success never executes init.
        for(const auto& label:labels_)if(label.pc<0){error_="Unresolved script label: "+label.name;return false;}
        if(!blocks_.empty()){error_="Unclosed script block";return false;}
        for(std::size_t n=0;n!=program_.instructions.size();++n) {
            const auto opcode=program_.instructions[n].opcode;
            if((opcode==0x8b||opcode==0x8e)&&signed_word(program_.instructions[n].operands.at(0))<0){error_="Unresolved Whenever reference";return false;}
            if(opcode==0x4f&&(n==0||program_.instructions[n-1].opcode!=0x106)){error_="Script Else has no preceding block";return false;}
            const bool block=opcode==0x4f||opcode==0x53||opcode==0x55||opcode==0x57||opcode==0x5b||opcode==0x5c||opcode==0xa4;
            if(block&&(n+1==program_.instructions.size()||program_.instructions[n+1].opcode!=0x105)){error_="Script event/control has no following block";return false;}
        }
        return true;
    }
    std::int32_t pc()const{return static_cast<std::int32_t>(program_.instructions.size());}
    static std::int32_t signed_word(std::uint32_t value){std::int32_t result;std::memcpy(&result,&value,4);return result;}
    static std::uint32_t word(std::int32_t value){std::uint32_t result;std::memcpy(&result,&value,4);return result;}
    MissionScriptProgram& program_;MissionScriptSymbols symbols_;
    std::vector<Label> labels_;std::vector<std::int32_t> blocks_;std::string error_;
};
} //namespace hd2::game
#endif
