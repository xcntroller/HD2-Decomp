#ifndef HD2_INSPECT_TRACE_SUPPORT_H
#define HD2_INSPECT_TRACE_SUPPORT_H
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>
#include <utility>
namespace hd2::inspection {
struct Span {std::size_t start=0,end=0;};
struct Statement {Span span;std::uint32_t opcode;std::string token;std::size_t first_pc,last_pc;bool accepted;};
struct Argument {std::size_t statement,slot;std::uint32_t kind;std::int32_t pool;Span span;};
struct Failure {std::string category,message,evidence,subject;Span span;std::int64_t pc=-1;bool set=false;};
struct Trace {std::vector<Statement> statements;std::vector<Argument> arguments;Span last;Failure failure;std::string expected;Span expected_span;};
inline thread_local Trace* active=nullptr;
inline void error(std::string category,std::string message,std::string evidence,Span span) {
    if(active&&!active->failure.set)active->failure={std::move(category),std::move(message),std::move(evidence),{},span,-1,true};
}
inline void validation(std::string category,std::string subject,std::int64_t pc=-1) {
    if(active&&!active->failure.set)active->failure={std::move(category),{},"0042C850",std::move(subject),active->last,pc,true};
}
struct Guard {std::function<void()> close;~Guard(){close();}};
template<class Lexer> Guard lex(Lexer& lexer) {
    return {[&lexer]{if(active)active->last=lexer.token().empty()?Span{lexer.source().size(),lexer.source().size()}:Span{lexer.span_begin(),lexer.span_end()};}};
}
template<class Lexer> bool expect(Lexer& lexer,std::string_view expected) {
    lexer.next();const bool ok=Lexer::equal(lexer.token(),expected);
    if(!ok&&active){active->expected=std::string(expected);active->expected_span={lexer.span_begin(),lexer.span_end()};}
    return ok;
}
template<class Lexer,class Program>struct StatementGuard {
    Lexer& lexer;Program& program;std::size_t start,first;std::string token;std::uint32_t opcode;bool accepted=false;
    StatementGuard(Lexer& l,Program& p,std::uint32_t op):lexer(l),program(p),start(l.span_begin()),first(p.instructions.size()),token(l.token()),opcode(op){}
    ~StatementGuard(){
        if(!active)return;
        auto end=lexer.state().cursor.value_or(lexer.source().size());
        while(end>start&&(lexer.source()[end-1]==' '||lexer.source()[end-1]=='\t'||lexer.source()[end-1]=='\r'||lexer.source()[end-1]=='\n'))--end;
        active->statements.push_back({{start,end},opcode,token,first,program.instructions.size(),accepted});
    }
};
inline void argument(std::size_t slot,std::uint32_t kind,std::int32_t pool,Span span) {
    if(active)active->arguments.push_back({active->statements.size(),slot,kind,pool,span});
}
}
#endif
