#ifndef HD2_GAME_MISSION_SCRIPT_EXPRESSION_H
#define HD2_GAME_MISSION_SCRIPT_EXPRESSION_H

#include "game/mission_script_program.h"
#include <memory>

namespace hd2::game {

//00412480/00412760/00412560 typed5C expression tree. Native0..0C values,
//10..1C kinds,20 predicate/21 NOT and2C/3C children/operators remain distinct.
struct MissionScriptExpression {
    std::array<std::uint32_t,4> values{},kinds{};
    bool predicate=false,inverted=false;
    std::uint32_t predicate_opcode=0;
    std::vector<std::unique_ptr<MissionScriptExpression>> children;
    std::vector<std::uint32_t> operators;

    template<class Predicates>std::uint32_t number(const MissionScriptProgram* program,Predicates& predicates)const { //00411650.
        if(!program)return 0;
        std::uint32_t value=0;
        if(children.empty()) {
            if(predicate)value=predicates.evaluate_script_predicate(predicate_opcode,values,kinds,*program);
            else if(kinds[0]==2)value=program->integers.at(values[0]).value;
        }else {
            if(!children[0])throw std::runtime_error("Script expression has no actual first child");
            value=children[0]->number(program,predicates);
            for(std::size_t n=1;n<children.size();++n) {
                if(!children[n])continue;
                if(n-1>=operators.size())throw std::runtime_error("Script expression has no original operator for its child");
                const auto op=operators[n-1];
                if(op<4||op>15)continue; //Unknown native operators do not evaluate RHS.
                //Native AND/OR evaluate only their required right operand.
                if(op==8&&!value){value=0;continue;}if(op==9&&value){value=1;continue;}
                const auto right=children[n]->number(program,predicates);
                switch(op) {
                case 4:value+=right;break;case 5:value-=right;break;case 6:value*=right;break;
                case 7: {
                    const auto a=signed_word(value),b=signed_word(right);
                    if(!b||(a==std::numeric_limits<std::int32_t>::min()&&b==-1))throw std::runtime_error("Script signed division would trap");
                    value=static_cast<std::uint32_t>(a/b);break;
                }
                case 8:case 9:value=right!=0;break;
                case 10:value=signed_word(right)<signed_word(value);break;
                case 11:value=signed_word(value)<signed_word(right);break;
                case 12:value=value==right;break;case 13:value=signed_word(value)<=signed_word(right);break;
                case 14:value=signed_word(right)<=signed_word(value);break;case 15:value=value!=right;break;
                default:break; //Native unknown operator ignores its RHS result.
                }
            }
        }
        return inverted?std::uint32_t(value==0):value;
    }
    std::string string(const MissionScriptProgram* program)const { //00411860; predicate leaf genuinely produces empty text.
        if(!program)return {};
        std::string result;
        if(children.empty()) {
            if(!predicate)for(unsigned n=0;n!=4;++n) {
                if(kinds[n]==2)result+=std::to_string(signed_word(program->integers.at(values[n]).value));
                else if(kinds[n]==4)result+=program->strings.at(values[n]);
            }
        }else {
            if(!children[0])throw std::runtime_error("Script string expression has no first child");
            result=children[0]->string(program);
            for(std::size_t n=1;n<children.size();++n)if(children[n]) {
                if(n-1>=operators.size())throw std::runtime_error("Script string expression operator is missing");
                if(operators[n-1]==4)result+=children[n]->string(program);
            }
        }
        return result;
    }
private:
    static std::int32_t signed_word(std::uint32_t value){std::int32_t result;std::memcpy(&result,&value,4);return result;}
};
} //namespace hd2::game
#endif
