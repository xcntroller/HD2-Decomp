#ifndef HD2_GAME_MISSION_SCRIPT_EXPRESSION_PARSER_H
#define HD2_GAME_MISSION_SCRIPT_EXPRESSION_PARSER_H

#include "game/mission_script_expression.h"

namespace hd2::game {

//0042D170/00411D00 tokenization and00412760 grouping. This retains native
//group/operator lists, including its narrow folding passes, rather than assigning
//a different language's precedence or replacing runtime predicates with values.
class MissionScriptExpressionParser {
public:
    MissionScriptExpressionParser(MissionScriptProgram& program,const MissionScriptSymbols& symbols)
        :program_(program),symbols_(symbols){}
    std::shared_ptr<MissionScriptExpression> parse(MissionScriptLexer& lexer) {
        raw_.clear();balance_=0;
        while(lexer.next()) {
            auto text=lexer.token();
            if(text=="="||text==">"||text=="<"||text=="!") {
                const auto saved=lexer.state().cursor;
                lexer.next();const auto next=lexer.token();
                if(next=="="||(text=="<"&&next==">"))text+=next;
                else {auto state=lexer.state();state.cursor=saved;lexer.state(state);} //Native restores cursor only.
            }
            if(text==",")continue; //0042D170 skips argument commas instead of emitting raw expression tokens.
            if(!classify(text)) {
                if(text!=";") {
                    auto state=lexer.state();
                    if(!state.cursor||!*state.cursor)return {};
                    --*state.cursor;lexer.state(state); //0042D170 returns ')' or ',' to its containing parser.
                }
                if(raw_.empty())return {};
                auto root=group(0,raw_.size());
                return root?std::shared_ptr<MissionScriptExpression>(std::move(root)):nullptr;
            }
        }
        return {}; //Native expression collection requires its actual closing token.
    }
private:
    struct Raw {std::uint32_t token,value,kind;}; //Expression4C records12.
    bool classify(const std::string& text) { //00411D00.
        const auto* command=mission_script_command(text);const auto opcode=command?command->opcode:0;
        std::uint32_t token=0,value=0,kind=0;
        switch(opcode) {
        case 0x108:token=4;break;case 0x109:token=5;break;case 0x10a:token=6;break;case 0x10b:token=7;break;
        case 0x10c:token=10;break;case 0x10d:token=11;break;case 0x10e:token=12;break;
        case 0x10f:token=14;break;case 0x110:token=13;break;case 0x111:token=15;break;
        default:
            if(MissionScriptLexer::equal(text,"AND"))token=8;
            else if(MissionScriptLexer::equal(text,"OR"))token=9;
            else if(MissionScriptLexer::equal(text,"NOT")||text=="!")token=16;
            else if(text=="("){token=2;++balance_;}
            else if(text==")"){token=3;if(--balance_<0)return false;}
            else if(text==";")return false;
            else {
                const bool previous_minus=!raw_.empty()&&raw_.back().token==5;
                const auto* variable=symbols_.variable(text);
                if(variable&&!previous_minus) {kind=variable->kind;value=word(variable->index);}
                else if(const auto* predicate=mission_script_predicate(text)){token=1;value=predicate->opcode;}
                else {
                    char* end=nullptr;const auto number=std::strtod(text.c_str(),&end);
                    const float stored=static_cast<float>(number); //00411F67 FLOAT spill, unlike ordinary argument literals.
                    if(end!=text.c_str()+text.size()) {
                        if(text.size()<2||text.front()!='"'||text.back()!='"')return false;
                        kind=4;value=static_cast<std::uint32_t>(program_.strings.size());program_.strings.push_back(text.substr(1,text.size()-2));
                    }else {
                        kind=2;value=static_cast<std::uint32_t>(program_.integers.size());auto integer=mission_script_integer_word(stored);
                        if(previous_minus) {
                            const auto before=raw_.size()>1?raw_[raw_.size()-2].token:0u;
                            if(raw_.size()==1||before==2||before==6||before==7||before==12||before==15||before==13||before==14||before==11)integer=0u-integer;
                        }
                        program_.integers.push_back({integer,false});
                    }
                }
            }
        }
        raw_.push_back({token,value,kind});return true;
    }
    static unsigned priority(std::uint32_t op) { //00412D70 tests homogeneity, not a full sort.
        if(op==4||op==5)return 2;
        if(op==6||op==7)return 3;
        if((10<=op&&op<=12)||op==15)return 1;
        return 0;
    }
    static bool homogeneous(const MissionScriptExpression& expression) {
        if(expression.operators.empty())return true;
        const auto first=priority(expression.operators[0]);
        for(auto op:expression.operators)if(priority(op)!=first)return false;
        return true;
    }
    std::unique_ptr<MissionScriptExpression> group(std::size_t begin,std::size_t end) { //00412760.
        if(begin==end)return {};
        auto expression=std::make_unique<MissionScriptExpression>();unsigned negate=0;
        for(auto n=begin;n<end;) {
            if(negate==2)negate=0;else if(negate==1)negate=2;
            const auto& raw=raw_[n];
            if(raw.token==16){negate=1;++n;continue;}
            if(raw.token==0) {
                auto child=std::make_unique<MissionScriptExpression>();child->values[0]=raw.value;child->kinds[0]=raw.kind;child->inverted=negate==2;
                expression->children.push_back(std::move(child));++n;continue;
            }
            if(raw.token==1) {
                const MissionScriptPredicate* predicate=nullptr;
                for(const auto& item:mission_script_predicates)if(item.opcode==raw.value){predicate=&item;break;}
                if(!predicate||n+1>=end||raw_[n+1].token!=2)return {};
                auto child=std::make_unique<MissionScriptExpression>();child->predicate=true;child->predicate_opcode=raw.value;child->inverted=negate==2;
                n+=2;const auto count=mission_script_predicate_arguments(*predicate);
                for(unsigned arg=0;arg!=count;++arg) {
                    if(n<end&&raw_[n].token==5)++n; //Native minus marker is skipped AFTER its literal was classified.
                    if(n>=end||raw_[n].token!=0||raw_[n].kind!=predicate->kinds[arg])return {};
                    child->values[arg]=raw_[n].value;child->kinds[arg]=raw_[n].kind;++n;
                }
                if(n>=end||raw_[n].token!=3)return {};
                expression->children.push_back(std::move(child));++n;continue;
            }
            if(raw.token==2) {
                auto close=n+1;unsigned nesting=1;
                for(;close<end;++close) {if(raw_[close].token==2)++nesting;else if(raw_[close].token==3&&!--nesting)break;}
                if(close==end)return {};
                auto child=group(n+1,close);if(!child)return {};
                //Native group constructor clears byte21; pending NOT is only
                //assigned by its literal/predicate branches, not this branch.
                expression->children.push_back(std::move(child));n=close+1;continue;
            }
            //00412540 accepts4..12 and15. Raw <=/>=13/14 are not silently
            //promoted to a different grammar; malformed runtime lists reject
            //at their bounded operator access if actually evaluated.
            if((raw.token>=4&&raw.token<=12)||raw.token==15)expression->operators.push_back(raw.token);
            ++n;
        }
        if(homogeneous(*expression))return expression;
        for(unsigned pass=0;pass!=4;++pass) { //00412560's original ordered binary folds.
            for(std::size_t n=0;n<expression->operators.size();) {
                const auto op=expression->operators[n];
                const bool fold=pass==0?(op==6||op==7):pass==1?(op==4||op==5):pass==2?(10<=op&&op<=12):(op==8||op==9);
                if(!fold){++n;continue;}
                if(n+1>=expression->children.size())return {};
                auto group=std::make_unique<MissionScriptExpression>();group->children.push_back(std::move(expression->children[n]));group->children.push_back(std::move(expression->children[n+1]));group->operators.push_back(op);
                expression->children[n]=std::move(group);expression->children.erase(expression->children.begin()+static_cast<std::ptrdiff_t>(n+1));expression->operators.erase(expression->operators.begin()+static_cast<std::ptrdiff_t>(n));
            }
            if(homogeneous(*expression))return expression;
        }
        return {};
    }
    static std::uint32_t word(std::int32_t value){std::uint32_t result;std::memcpy(&result,&value,4);return result;}
    MissionScriptProgram& program_;const MissionScriptSymbols& symbols_;
    std::vector<Raw> raw_;int balance_=0;
};
} //namespace hd2::game
#endif
