#ifndef HD2_GAME_MISSION_SCRIPT_LEXER_H
#define HD2_GAME_MISSION_SCRIPT_LEXER_H

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace hd2::game {

//0042CC00/0042CDD0/0042DED0/0042DF10. Own the source so native parser+4
//is a bounded optional offset, never a process pointer stored in a DWORD.
//Token8 has native1024-byte capacity. Native cursor-presence return is distinct
//from token presence: the last identifier returns false with a VALID token.
class MissionScriptLexer {
public:
    struct State {std::optional<std::size_t> cursor;std::uint32_t line=0,breaks=0;};
    explicit MissionScriptLexer(std::string source):source_(std::move(source)) {reset();}
    const std::string& source()const{return source_;}
    const std::string& token()const{return token_;}
    const State& state()const{return state_;}
    void state(State value) {
        if(value.cursor&&*value.cursor>source_.size())throw std::out_of_range("Script cursor exceeds source");
        state_=value;
    }
    void reset() {state_={std::size_t{0},0,0};token_.clear();}
    static bool identifier(char c,bool subsequent) { //0042DED0, ASCII only.
        return (subsequent&&'0'<=c&&c<='9')||('a'<=c&&c<='z')||('A'<=c&&c<='Z')||c=='_';
    }
    static bool number(char c,bool dot) {return (dot&&c=='.')||('0'<=c&&c<='9');} //0042DF10.
    bool next() { //0042CC00 returns native AL only; upper pointer bits have no source meaning.
        token_.clear();
        if(!state_.cursor||current()==0)return false;
        skip();if(!state_.cursor)return false;
        if(identifier(current(),false)) {
            do{append(current());advance();}while(identifier(current(),true));
            skip();return state_.cursor.has_value();
        }
        //Native DL=1 survives the failed identifier test at0042CC50/9A:
        //numeric tokens allow '.' even in the FIRST position (including '.').
        if(number(current(),true)) {
            do{append(current());advance();}while(number(current(),true));
            skip();return state_.cursor.has_value();
        }
        if(current()=='"') {
            do {
                append(current());advance();
                if(current()=='\r'||current()=='\n'||current()==0)break;
            }while(current()!='"');
            if(current()=='"'){append('"');advance();}
            skip();return state_.cursor.has_value();
        }
        if(current()=='/'&&following()=='/') {
            do {append(current());advance();}while(current()!='\r'&&current()!='\n'&&current()!=0);
            skip();return state_.cursor.has_value();
        }
        append(current());advance(); //Punctuation consumes exactly one char; no trailing skip.
        return state_.cursor.has_value();
    }
    bool expect(std::string_view expected) {next();return equal(token_,expected);} //0042CF60 consumes even on mismatch.
    static bool equal(std::string_view first,std::string_view second) {
        if(first.size()!=second.size())return false;
        for(std::size_t n=0;n!=first.size();++n)if(lower(first[n])!=lower(second[n]))return false;
        return true; //007E88F0 current script names are ASCII case-insensitive.
    }
private:
    static char lower(char c){return 'A'<=c&&c<='Z'?static_cast<char>(c+('a'-'A')):c;}
    char current()const {return state_.cursor&&*state_.cursor<source_.size()?source_[*state_.cursor]:0;}
    char following()const {return state_.cursor&&*state_.cursor+1<source_.size()?source_[*state_.cursor+1]:0;}
    void advance(){if(!state_.cursor||*state_.cursor>=source_.size())throw std::runtime_error("Script token passes source terminator");++*state_.cursor;}
    void append(char c){if(token_.size()>=1023)throw std::length_error("Script token exceeds original buffer");token_.push_back(c);}
    void skip() { //0042CDD0 counts CR only, including across mixed CR/LF whitespace.
        state_.breaks=0;
        while(state_.cursor) {
            const auto c=current();
            if(c!=' '&&c!='\n'&&c!='\r'&&c!='\t') {
                if(!c)state_.cursor.reset();
                return;
            }
            if(c=='\r'){++state_.line;++state_.breaks;}
            advance();
        }
    }
    std::string source_,token_;State state_;
};
} //namespace hd2::game
#endif
