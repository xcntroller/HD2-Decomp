// Standalone source inspection. Runtime headers are only observed through the
// checked build-time overlays; game/runtime implementation is not modified.
#include "trace_support.h"
#include "game/mission_script_compiler.h"
#include <fstream>
#include <iostream>
#include <sstream>
using namespace hd2::game;
namespace trace=hd2::inspection;
static std::string quote(const std::string& s){
 std::ostringstream o;o<<'"';for(unsigned char c:s){if(c=='"'||c=='\\')o<<'\\'<<c;else if(c<32||c>=127){const char*h="0123456789abcdef";o<<"\\u00"<<h[c>>4]<<h[c&15];}else o<<c;}o<<'"';return o.str();
}
static void span(trace::Span s){std::cout<<"{\"start\":"<<s.start<<",\"end\":"<<s.end<<'}';}
static const char* name(unsigned op){for(auto i=mission_script_commands.rbegin();i!=mission_script_commands.rend();++i)if(i->opcode==op)return i->name;return "<internal>";}
static void kinds(const std::array<std::uint32_t,4>& values){std::cout<<'[';bool comma=false;for(auto k:values){if(!k)break;if(comma){std::cout<<',';}comma=true;std::cout<<k;}
  std::cout<<']';}
struct Token {std::string text,kind;trace::Span span;};
static Token token(const MissionScriptLexer& lexer){
 const auto& t=lexer.token();std::string kind="punctuation";
 if(t.rfind("//",0)==0)kind="comment";
 else if(t.front()=='"')kind="string";
 else if(MissionScriptLexer::identifier(t.front(),false))kind="identifier";
 else if(MissionScriptLexer::number(t.front(),true))kind="number";
 return {t,kind,{lexer.span_begin(),lexer.span_end()}};
}
int main(int argc,char**argv){
 bool json=false,prepared=false;std::string path;std::optional<std::size_t> lex_cursor;
 for(int i=1;i<argc;++i){std::string a=argv[i];
  if(a=="--json")json=true;
  else if(a=="--prepared")prepared=true;
  else if(a=="--lex-one"&&i+1<argc){try{std::string value=argv[++i];std::size_t n=0;auto offset=std::stoull(value,&n);if(n!=value.size()||value[0]=='-')throw std::invalid_argument("cursor");lex_cursor=offset;}catch(...){std::cerr<<"Invalid lexer cursor\n";return 2;}}
  else if(a=="--catalog"){
   std::cout<<"{\"schema_version\":2,\"commands\":[";bool comma=false;
   for(const auto& c:mission_script_commands){if(comma){std::cout<<',';}comma=true;
    std::cout<<"{\"name\":"<<quote(c.name)<<",\"opcode\":"<<c.opcode<<",\"raw_mask\":"<<c.mask<<",\"argument_kinds\":";
    if(auto*s=mission_script_signature(c.opcode))kinds(s->kinds);else std::cout<<"null";std::cout<<'}';}
   std::cout<<"],\"predicates\":[";comma=false;
   for(const auto&p:mission_script_predicates){if(comma){std::cout<<',';}comma=true;std::cout<<"{\"name\":"<<quote(p.name)<<",\"opcode\":"<<p.opcode<<",\"argument_kinds\":";kinds(p.kinds);std::cout<<'}';}
   std::cout<<"]}\n";return 0;
  }else if(a=="--help"){std::cout<<"hd2-inspect [--json] [--prepared] FILE\nhd2-inspect --catalog\nhd2-inspect --lex-one BYTE_OFFSET FILE\n";return 0;}
  else if(!a.empty()&&a[0]=='-'){std::cerr<<"Unknown option\n";return 2;}
  else if(path.empty())path=a;else{std::cerr<<"Expected one file\n";return 2;}
 }
 if(path.empty()){std::cerr<<"Expected a source file\n";return 2;}
 std::ifstream file(path,std::ios::binary);if(!file){std::cerr<<"Cannot open source\n";return 2;}
 std::string source;char ch;while(file.get(ch)){source+=ch;if(source.size()>65535)break;}
 if(file.bad()){std::cerr<<"Cannot read source\n";return 2;}
 trace::Trace observed;trace::active=&observed;
 if(lex_cursor){
  if(source.size()>65535||*lex_cursor>source.size()){std::cerr<<"Lexer input/cursor exceeds bounded source\n";return 2;}
  try{MissionScriptLexer lexer(source);lexer.state({*lex_cursor,0,0});lexer.next();
   std::cout<<"{\"schema_version\":2,\"token\":"<<quote(lexer.token())<<",\"span\":";span({lexer.span_begin(),lexer.span_end()});
   std::cout<<",\"cursor\":";if(lexer.state().cursor)std::cout<<*lexer.state().cursor;else std::cout<<"null";std::cout<<"}\n";return 0;
  }catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 2;}
 }
 MissionScriptProgram program;MissionScriptCompiler compiler(program);std::vector<Token> tokens;
 bool valid=false;std::string diagnostic;
 try{
  const auto limit=prepared?65535u:65534u;
  if(source.size()>limit){diagnostic="Source exceeds inspection read/expanded buffer limit";trace::error("source_limit",diagnostic,"0042C850",{limit,source.size()});}
  else if(auto n=source.find('\0');n!=std::string::npos){diagnostic="Embedded NUL: inspection refuses silent truncation";trace::error("embedded_nul",diagnostic,"0042C850",{n,n+1});}
  else if(!prepared&&source.find("#include")!=std::string::npos){auto n=source.find("#include");diagnostic="Include resolution requires an explicit caller-supplied context";trace::error("include_context_required",diagnostic,"0042C580/0042C4B0",{n,n+8});}
  // Tokenization reuses the actual lexer. Depth bound is defensive; it is not
  // claimed to be a limit imposed by the original executable.
  if(diagnostic.empty()){
   MissionScriptLexer lexer(source);unsigned nesting=0,operators=0;
   for(;;){lexer.next();if(lexer.token().empty())break;tokens.push_back(token(lexer));
    if(lexer.token()=="("||lexer.token()=="{"){if(++nesting>128){diagnostic="Inspection nesting limit (128) exceeded";trace::error("nesting_limit",diagnostic,"tool defensive restriction",tokens.back().span);break;}}
    else if((lexer.token()==")"||lexer.token()=="}")&&nesting)--nesting;
    const auto& text=lexer.token();
    if(text==";"||text=="{"||text=="}"||text==":")operators=0;
    else if(text=="+"||text=="-"||text=="*"||text=="/"||text=="<"||text==">"||text=="="||text=="!"||MissionScriptLexer::equal(text,"AND")||MissionScriptLexer::equal(text,"OR")||MissionScriptLexer::equal(text,"NOT")){
     if(++operators>256){diagnostic="Inspection operator-chain limit (256) exceeded";trace::error("operator_limit",diagnostic,"tool defensive restriction",tokens.back().span);break;}
    }
    if(!lexer.state().cursor)break;
   }
   if(diagnostic.empty()){valid=compiler.compile(source);if(!valid)diagnostic=compiler.error();}
  }
 }catch(const std::exception&e){diagnostic=e.what();}
 if(!valid&&diagnostic.empty())diagnostic="Unsupported or malformed source";
 auto failure=observed.failure;
 if(!valid&&!failure.set){failure={"syntax_rejected",diagnostic,"0042C850/0042C0B0",{},observed.last,-1,true};
  if(observed.expected_span.start==observed.last.start&&observed.expected_span.end==observed.last.end){failure.category="unexpected_token";failure.message="Expected "+observed.expected;failure.evidence="0042CF60";}}
 if(!valid&&failure.pc>=0){for(const auto&s:observed.statements)if(s.first_pc<=static_cast<std::size_t>(failure.pc)&&static_cast<std::size_t>(failure.pc)<s.last_pc){failure.span=s.span;break;}}
 if(!valid&&failure.category=="unclosed_block")failure.span={source.size(),source.size()};
 if(!valid&&failure.category=="unresolved_label"){
  for(const auto&s:observed.statements)if(s.opcode==0x4d||s.opcode==0x51){
   for(const auto&t:tokens)if(t.span.start>=s.span.start&&t.span.end<=s.span.end&&MissionScriptLexer::equal(t.text,failure.subject)){failure.span=t.span;goto found_label;}}
 }found_label:;
 const std::string limits="Recovered prepared-source front end only; no execution or runtime compatibility proof. Lexical mentions are not calls. Unknown semantics remain unknown. Inspector nesting/operator-chain/size/NUL guards are defensive. Failure omits compiled products.";
 if(!json){std::cout<<path<<": "<<(valid?"supported syntax accepted":"validation unavailable/rejected")<<'\n'<<diagnostic<<'\n'<<limits<<'\n';
  for(const auto&s:observed.statements)std::cout<<s.span.start<<".."<<s.span.end<<": "<<s.token<<'\n';
  if(valid)for(std::size_t pc=0;pc<program.instructions.size();++pc)std::cout<<pc<<": "<<name(program.instructions[pc].opcode)<<'\n';
  return valid?0:1;}
 std::cout<<"{\"schema_version\":2,\"file\":"<<quote(path)<<",\"syntax_accepted\":"<<(valid?"true":"false")<<",\"diagnostic\":"<<quote(diagnostic)<<",\"limitations\":["<<quote(limits)<<"],\"diagnostics\":[";
 if(!valid){std::cout<<"{\"severity\":\"error\",\"category\":"<<quote(failure.category)<<",\"message\":"<<quote(failure.message.empty()?diagnostic:failure.message)<<",\"compiler_message\":"<<quote(diagnostic)<<",\"evidence\":"<<quote(failure.evidence)<<",\"span\":";span(failure.span);std::cout<<",\"precision\":\"observed failure token or verified validation site\"}";}
 std::cout<<"],\"tokens\":[";bool comma=false;
 for(const auto&t:tokens){if(comma){std::cout<<',';}comma=true;std::cout<<"{\"text\":"<<quote(t.text)<<",\"kind\":"<<quote(t.kind)<<",\"span\":";span(t.span);std::cout<<'}';}
 std::cout<<"],\"statements\":[";comma=false;
 for(std::size_t n=0;n<observed.statements.size();++n){const auto&s=observed.statements[n];if(comma){std::cout<<',';}comma=true;
  std::cout<<"{\"index\":"<<n<<",\"name\":"<<quote(s.token)<<",\"opcode\":"<<s.opcode<<",\"parse_accepted\":"<<(s.accepted?"true":"false")<<",\"span\":";span(s.span);
  std::cout<<",\"pc_begin\":";if(valid)std::cout<<s.first_pc;else std::cout<<"null";
  std::cout<<",\"pc_end\":";if(valid)std::cout<<s.last_pc;else std::cout<<"null";std::cout<<'}';}
 std::cout<<"],\"arguments\":[";comma=false;
 for(const auto&a:observed.arguments){if(comma){std::cout<<',';}comma=true;
  std::cout<<"{\"statement\":"<<a.statement<<",\"slot\":"<<a.slot<<",\"kind\":"<<a.kind<<",\"pool_index\":";if(valid)std::cout<<a.pool;else std::cout<<"null";
  std::cout<<",\"span\":";span(a.span);std::cout<<'}';}
 std::cout<<"],\"variables\":[";comma=false;
 if(valid)for(const auto&v:compiler.symbols().variables()){if(comma){std::cout<<',';}comma=true;std::cout<<"{\"name\":"<<quote(v.name)<<",\"kind\":"<<v.kind<<",\"pool_index\":"<<v.index<<'}';}
 std::cout<<"],\"start_pc\":"<<(valid?program.start:-1)<<",\"event_slots\":[";
 if(valid)for(std::size_t n=0;n<program.events.size();++n){if(n)std::cout<<',';std::cout<<program.events[n];}
 std::cout<<"],\"event_tables\":{";comma=false;
 for(const auto& pair:std::vector<std::pair<std::string,const std::vector<MissionScriptProgram::Event>*>>{{"signals",&program.signals},{"cutscenes",&program.cutscenes},{"cutscene_done",&program.cutscene_done}}){
  if(comma){std::cout<<',';}comma=true;std::cout<<quote(pair.first)<<":[";
  if(valid)for(std::size_t n=0;n<pair.second->size();++n){if(n)std::cout<<',';const auto&e=(*pair.second)[n];std::cout<<"{\"identity\":"<<e.identity<<",\"entry_pc\":"<<e.pc<<'}';}
  std::cout<<']';}
 std::cout<<"},\"whenever_count\":"<<(valid?program.whens.size():0)<<",\"whenevers\":[";
 if(valid)for(std::size_t n=0;n<program.whens.size();++n){if(n)std::cout<<',';const auto&w=program.whens[n];std::cout<<"{\"index\":"<<n<<",\"entry_pc\":"<<w.pc<<",\"expression\":"<<w.expression<<",\"enabled\":"<<(w.enabled?"true":"false")<<",\"period\":"<<w.period<<'}';}
 std::cout<<"],\"instructions\":[";comma=false;
 std::size_t statement_cursor=0;
 if(valid)for(std::size_t pc=0;pc<program.instructions.size();++pc){const auto&i=program.instructions[pc];if(comma){std::cout<<',';}comma=true;
  std::cout<<"{\"pc\":"<<pc<<",\"name\":"<<quote(name(i.opcode))<<",\"opcode\":"<<i.opcode<<",\"operands\":[";
  for(std::size_t n=0;n<i.operands.size();++n){if(n)std::cout<<',';std::cout<<i.operands[n];}std::cout<<"],\"statement\":";
  while(statement_cursor<observed.statements.size()&&observed.statements[statement_cursor].last_pc<=pc)++statement_cursor;
  if(statement_cursor<observed.statements.size()&&observed.statements[statement_cursor].first_pc<=pc)std::cout<<statement_cursor;else std::cout<<"null";std::cout<<'}';}
 std::cout<<"],\"references\":[";comma=false;
 if(valid)for(std::size_t pc=0;pc<program.instructions.size();++pc){const auto&i=program.instructions[pc];if(i.opcode!=1&&i.opcode!=0x5e&&i.opcode!=0x66)continue;
  auto index=i.operands.at(1);bool dynamic=false;for(const auto&v:compiler.symbols().variables())if(v.kind==4&&v.index==static_cast<std::int32_t>(index))dynamic=true;
  if(comma){std::cout<<',';}comma=true;std::cout<<"{\"pc\":"<<pc<<",\"kind\":"<<quote(i.opcode==1?"script":i.opcode==0x5e?"scene_frame":"child_frame")<<",\"dynamic\":"<<(dynamic?"true":"false")<<",\"literal\":"<<(dynamic?"null":quote(program.strings.at(index)))<<'}';}
 std::cout<<"],\"lexical_mentions\":[";comma=false;
 for(const auto&t:tokens){const auto*c=mission_script_command(t.text);const auto*p=mission_script_predicate(t.text);if(!c&&!p)continue;
  if(comma){std::cout<<',';}comma=true;std::cout<<"{\"name\":"<<quote(t.text)<<",\"category\":"<<quote(c?"command":"predicate")<<",\"span\":";span(t.span);std::cout<<'}';}
 std::cout<<"],\"include_directive_present\":"<<(source.find("#include")!=std::string::npos?"true":"false")<<"}\n";
 return valid?0:1;
}
