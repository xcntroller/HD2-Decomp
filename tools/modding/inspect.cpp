// Standalone source inspector. No runtime, archive, executable or Ghidra dependency.
#include "game/mission_script_compiler.h"
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
using namespace hd2::game;
static std::string quote(const std::string& s) {
    std::ostringstream o;o<<'"';
    for(unsigned char c:s) {if(c=='"'||c=='\\')o<<'\\'<<c;else if(c<32||c>=127) {
        const char* h="0123456789abcdef";o<<"\\u00"<<h[c>>4]<<h[c&15];
    }else o<<c;}o<<'"';return o.str();
}
static const char* name(unsigned opcode) {
    for(auto i=mission_script_commands.rbegin();i!=mission_script_commands.rend();++i)
        if(i->opcode==opcode)return i->name;
    return "<internal>";
}
static void kinds(const std::array<std::uint32_t,4>& values) {
    std::cout<<'[';bool comma=false;for(auto k:values){if(!k)break;if(comma)std::cout<<',';
        comma=true;std::cout<<k;}std::cout<<']';
}
int main(int argc,char** argv) {
    bool json=false;std::string path;
    for(int i=1;i<argc;++i) {
        std::string a=argv[i];if(a=="--json")json=true;
        else if(a=="--catalog") {
            std::cout<<"{\"schema_version\":1,\"commands\":[";bool comma=false;
            for(const auto& c:mission_script_commands){if(comma)std::cout<<',';
        comma=true;
                std::cout<<"{\"name\":"<<quote(c.name)<<",\"opcode\":"<<c.opcode<<",\"raw_mask\":"<<c.mask<<",\"argument_kinds\":";
                if(auto* s=mission_script_signature(c.opcode))kinds(s->kinds);else std::cout<<"null";
                std::cout<<'}';}
            std::cout<<"],\"predicates\":[";
    comma=false;
            for(const auto& p:mission_script_predicates){if(comma)std::cout<<',';
        comma=true;std::cout<<"{\"name\":"<<quote(p.name)<<",\"opcode\":"<<p.opcode<<",\"argument_kinds\":";kinds(p.kinds);std::cout<<'}';}
            std::cout<<"]}\n";return 0;
        }else if(a=="--help") {std::cout<<"hd2-inspect [--json] FILE\nhd2-inspect --catalog\n";return 0;}
        else if(!a.empty()&&a[0]=='-'){std::cerr<<"Unknown option\n";return 2;}
        else if(path.empty())path=a;else {std::cerr<<"Expected one file\n";return 2;}
    }
    if(path.empty()){std::cerr<<"Expected a source file; use --help\n";return 2;}
    std::ifstream f(path,std::ios::binary);if(!f){std::cerr<<"Cannot open source\n";return 2;}
    std::string source;char c;
    while(f.get(c)){source+=c;if(source.size()>65534)break;}
    MissionScriptProgram program;MissionScriptCompiler compiler(program);
    std::string diagnostic;bool valid=false;
    try {
        if(source.size()>65534)diagnostic="Source exceeds original 0xFFFE read boundary; inspection refuses truncation";
        else if(source.find('\0')!=std::string::npos)diagnostic="Embedded NUL: inspection refuses silent truncation";
        else if(source.find("#include")!=std::string::npos)diagnostic="Includes require original resolver/expansion; validation unavailable for unexpanded source";
        else {valid=compiler.compile(source);if(!valid)diagnostic=compiler.error();}
    }catch(const std::exception& e){diagnostic=e.what();}
    if(!valid&&diagnostic.empty())diagnostic="Unsupported or malformed source";
    const std::string limits="Prepared-source syntax only; no execution, archive precedence, resource existence or runtime compatibility proof. References may be dynamic; no data-flow evaluation. Failed parse products are omitted.";
    if(json)std::cout<<"{\"schema_version\":1,\"file\":"<<quote(path)<<",\"syntax_accepted\":"<<(valid?"true":"false")<<",\"diagnostic\":"<<quote(diagnostic)<<",\"limitations\":["<<quote(limits)<<"],\"variables\":[";
    else std::cout<<path<<": "<<(valid?"supported syntax accepted":"validation unavailable/rejected")<<'\n'<<diagnostic<<'\n'<<limits<<'\n';
    bool comma=false;
    if(valid)for(const auto& v:compiler.symbols().variables()) {
        if(json){if(comma)std::cout<<',';std::cout<<"{\"name\":"<<quote(v.name)<<",\"kind\":"<<v.kind<<",\"pool_index\":"<<v.index<<'}';}
        else std::cout<<"symbol "<<v.name<<" kind="<<v.kind<<" index="<<v.index<<'\n';
        comma=true;
    }
    if(json) {
        std::cout<<"],\"start_pc\":"<<(valid?program.start:-1)<<",\"event_slots\":[";
        if(valid)for(std::size_t n=0;n<program.events.size();++n){if(n)std::cout<<',';std::cout<<program.events[n];}
        std::cout<<"],\"whenever_count\":"<<(valid?program.whens.size():0)<<",\"instructions\":[";
    }
    comma=false;
    if(valid)for(std::size_t pc=0;pc<program.instructions.size();++pc){const auto& i=program.instructions[pc];
        if(json){if(comma)std::cout<<',';std::cout<<"{\"pc\":"<<pc<<",\"name\":"<<quote(name(i.opcode))<<",\"opcode\":"<<i.opcode<<",\"operands\":[";
            for(std::size_t n=0;n<i.operands.size();++n){if(n)std::cout<<',';std::cout<<i.operands[n];}std::cout<<"]}";}
        else std::cout<<pc<<": "<<name(i.opcode)<<" ("<<i.opcode<<")\n";
        comma=true;
    }
    if(json)std::cout<<"],\"references\":[";
    comma=false;
    if(valid)for(std::size_t pc=0;pc<program.instructions.size();++pc){const auto& i=program.instructions[pc];
        if(i.opcode!=1&&i.opcode!=0x5e&&i.opcode!=0x66)continue;
        auto index=i.operands.at(1);bool dynamic=false;
        for(const auto& v:compiler.symbols().variables())if(v.kind==4&&v.index==static_cast<std::int32_t>(index))dynamic=true;
        const auto& value=program.strings.at(index);const char* kind=i.opcode==1?"script":i.opcode==0x5e?"scene_frame":"child_frame";
        if(json){if(comma)std::cout<<',';std::cout<<"{\"pc\":"<<pc<<",\"kind\":"<<quote(kind)<<",\"dynamic\":"<<(dynamic?"true":"false")<<",\"literal\":"<<(dynamic?"null":quote(value))<<'}';}
        else std::cout<<"reference "<<kind<<": "<<(dynamic?"<dynamic string variable>":value)<<'\n';
        comma=true;
    }
    if(json)std::cout<<"],\"lexical_mentions\":[";
    comma=false;
    // Mentions are lexical, not calls: declarations can shadow table names.
    try {
        MissionScriptLexer lexer(source);
        for(;;){lexer.next();const auto& t=lexer.token();if(t.empty())break;
            const auto* command=mission_script_command(t);const auto* predicate=mission_script_predicate(t);
            if(command||predicate) {
                if(json){if(comma)std::cout<<',';std::cout<<"{\"name\":"<<quote(t)<<",\"category\":"<<quote(command?"command":"predicate")<<'}';}
                else std::cout<<"lexical "<<(command?"command ":"predicate ")<<t<<'\n';
                comma=true;
            }
            if(!lexer.state().cursor)break;
        }
    }catch(const std::exception&) { /* Full validation diagnostic already records bounded lexer failures. */ }
    if(json)std::cout<<"],\"include_directive_present\":"<<(source.find("#include")!=std::string::npos?"true":"false")<<"}\n";
    return valid?0:1;
}
