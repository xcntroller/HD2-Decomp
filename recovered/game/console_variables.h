#ifndef HD2_GAME_CONSOLE_VARIABLES_H
#define HD2_GAME_CONSOLE_VARIABLES_H
#include <array>
#include <cstdint>
#include <string_view>
namespace hd2::game {
//Process descriptors00844468..00844570, stride24. Name00/word04/type08/
//variableBYTE0C/domain10/value14.00720B80 indexes TYPE at00844470, not the
//record beginning. The three actual variables are integer/domain"0,1";
//type2 rows are commands, so0071FE70 allocates NO string-variable owners.
//Console command parser/dispatch, dictionary and side effects remain separate.
class ConsoleVariables {
public:
    struct Descriptor {std::string_view name;std::uint32_t word4,type;bool variable;std::string_view domain;};
    inline static constexpr std::array<Descriptor,11> descriptors{{
        {"sensitivity",1,1,false,{}},{"say",1,2,false,{}},{"sayteam",1,2,false,{}},
        {"kill",1,2,false,{}},{"happyammo",1,1,false,{}},{"hydroshock",1,1,true,"0,1"},
        {"sniper",1,1,true,"0,1"},{"heal",1,1,false,{}},{"giveitem",1,2,false,{}},
        {"anorexy",1,1,false,{}},{"no_gi",1,1,true,"0,1"}
    }};
    std::int32_t integer(std::size_t index)const { //00720B80; invalid native memory access becomes at() guard.
        const auto& d=descriptors.at(index);
        return d.variable&&d.type==1?values_.at(index):-1;
    }
    std::uint32_t set_integer_variable(std::size_t index,std::int32_t value) {
        const auto& d=descriptors.at(index);
        //Reached variable branch00720A80→00721B90 validates literal"0,1"
        //before storing. The generic command/domain/lexer path is not exposed.
        if(!d.variable||d.type!=1||d.domain!="0,1"||(value!=0&&value!=1))return 5;
        values_[index]=value;return 0;
    }
private:
    std::array<std::int32_t,11> values_{}; //All original static value14 DWORDs start0; persist across mission loads.
};
} //namespace hd2::game
#endif
