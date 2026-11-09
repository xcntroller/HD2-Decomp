"""Generate inspection-only tracing overlays; recovered sources remain untouched.

Every edit has a unique anchor and original content hash in trace-contract.json.
A changed compiler contract requires review rather than silently changing hooks.
"""
import argparse
import hashlib
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]
NAMES=['mission_script_lexer.h','mission_script_program.h','mission_script_compiler.h']


def replace(text,old,new):
    if text.count(old)!=1:raise ValueError(f'Trace anchor changed: {old[:90]}')
    return text.replace(old,new,1)


def generate(directory):
    contract=json.loads((Path(__file__).parent/'trace-contract.json').read_text())
    directory.mkdir(parents=True,exist_ok=True)
    for name in NAMES:
        source=(ROOT/'recovered/game'/name).read_bytes()
        if hashlib.sha256(source).hexdigest()!=contract['headers'][name]:
            raise ValueError(f'Review tracing hooks for changed recovered header: {name}')
        text=source.decode()
        if name=='mission_script_lexer.h':
            text=replace(text,'#include <cstdint>','#include "trace_support.h"\n#include <cstdint>')
            text=replace(text,'    const std::string& source()const', '    std::size_t span_begin()const{return token_begin_;}\n    std::size_t span_end()const{return token_end_;}\n    const std::string& source()const')
            text=replace(text,'        token_.clear();\n        if(!state_.cursor', '        token_begin_=token_end_=state_.cursor.value_or(source_.size());\n        auto trace_read=hd2::inspection::lex(*this);\n        token_.clear();\n        if(!state_.cursor')
            text=replace(text,'void append(char c){if(token_.size()>=1023)throw std::length_error("Script token exceeds original buffer");token_.push_back(c);}',
                         'void append(char c){if(token_.size()>=1023){hd2::inspection::error("token_limit","Token exceeds recovered buffer","0042CC00",{state_.cursor.value_or(source_.size()),state_.cursor.value_or(source_.size())+1});throw std::length_error("Script token exceeds original buffer");}if(token_.empty())token_begin_=state_.cursor.value_or(source_.size());token_end_=state_.cursor.value_or(source_.size())+1;token_.push_back(c);}')
            text=replace(text,'bool expect(std::string_view expected) {next();return equal(token_,expected);}',
                         'bool expect(std::string_view expected) {return hd2::inspection::expect(*this,expected);}')
            text=replace(text,'    std::string source_,token_;State state_;','    std::size_t token_begin_=0,token_end_=0;\n    std::string source_,token_;State state_;')
        elif name=='mission_script_program.h':
            text=replace(text,'if(variable(name))return false;','if(variable(name)){if(hd2::inspection::active)hd2::inspection::error("duplicate_symbol","Duplicate symbol: "+name,"0042ECD0",hd2::inspection::active->last);return false;}')
            text=replace(text,'else return false; //Bool/Enum/Real', 'else {if(hd2::inspection::active)hd2::inspection::error("unsupported_declaration","Declaration type outside recovered supported branch","0042ECD0",hd2::inspection::active->last);return false;} //Bool/Enum/Real')
        else:
            text=replace(text,'            if(!statement(lexer)) {','            const auto* trace_command=mission_script_command(lexer.token());\n            hd2::inspection::StatementGuard trace_statement(lexer,program_,trace_command?trace_command->opcode:0u);\n            if(!statement(lexer)) {')
            text=replace(text,'            if(!lexer.state().cursor)break;', '            trace_statement.accepted=true;\n            if(!lexer.state().cursor)break;')
            text=replace(text,'error_="Unknown script command: "+token;return false;', 'hd2::inspection::error("unknown_command","Unknown command or symbol: "+token,"0042C0B0",{lexer.span_begin(),lexer.span_end()});error_="Unknown script command: "+token;return false;')
            text=replace(text,'if(!root)return -1;','if(!root){hd2::inspection::error("expression_rejected","Expression rejected by recovered parser","0042D170/00412760",{lexer.span_begin(),lexer.span_end()});return -1;}')
            text=replace(text,'lexer.next();const auto index=symbols_.operand(mission_script_argument_kind(opcode,n),lexer.token(),lexer);',
                         'lexer.next();const auto trace_start=lexer.span_begin();const auto index=symbols_.operand(mission_script_argument_kind(opcode,n),lexer.token(),lexer);')
            text=replace(text,'            if(index==-1)return false;','            hd2::inspection::argument(n,mission_script_argument_kind(opcode,n),index,{trace_start,lexer.span_end()});\n            if(index==-1){hd2::inspection::error("operand_rejected","Argument does not satisfy recovered operand kind","0042D890/0042D920",{trace_start,lexer.span_end()});return false;}')
            text=replace(text,'if(label.pc!=-1)return false;', 'if(label.pc!=-1){hd2::inspection::error("duplicate_label","Label already defined","0042F0A0",{lexer.span_begin(),lexer.span_end()});return false;}')
            text=replace(text,'if(program_.start!=-1)return false;', 'if(program_.start!=-1){hd2::inspection::error("duplicate_start","ScriptStartPoint already defined","0042E410",{lexer.span_begin(),lexer.span_end()});return false;}')
            text=replace(text,'if(old->kind==8&&old->index>=0)return false;', 'if(old->kind==8&&old->index>=0){hd2::inspection::error("duplicate_whenever","Whenever already defined","0042E7C0",{lexer.span_begin(),lexer.span_end()});return false;}')
            text=replace(text,'if(label.pc<0){error_=', 'if(label.pc<0){hd2::inspection::validation("unresolved_label",label.name);error_=')
            text=replace(text,'if(!blocks_.empty()){error_=', 'if(!blocks_.empty()){hd2::inspection::validation("unclosed_block",{},blocks_.back());error_=')
            for message,category in [('Unresolved Whenever reference','unresolved_whenever'),('Script Else has no preceding block','invalid_else'),('Script event/control has no following block','missing_event_block')]:
                text=replace(text,'{error_="'+message+'";', '{hd2::inspection::validation("'+category+'",{},static_cast<std::int64_t>(n));error_="'+message+'";')
        (directory/name).write_text(text)


if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('output',type=Path)
    args=p.parse_args();generate(args.output)
