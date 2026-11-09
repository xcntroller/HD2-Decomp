#ifndef HD2_GAME_MISSION_SCRIPT_SOURCE_H
#define HD2_GAME_MISSION_SCRIPT_SOURCE_H

#include "game/mission_script_lexer.h"
#include <vector>

namespace hd2::game {

//0042C850 read prefix and0042C580/0042C4B0 include expansion. Files is the
//actual resolver (loose/Patch/base precedence); no alternate archive view is
//accepted here. Both native reads request0xFFFE, without seeking the full file.
//Source offsets replace aliased global buffers, preserving cursor positions
//when insertion shifts subsequent source bytes.
template<class Files>class MissionScriptSource {
public:
    MissionScriptSource(Files& files,std::string resource):files_(files),resource_(std::move(resource)){}
    std::optional<std::string> load(const std::string& path) {
        auto source=read(path);if(!source)return std::nullopt;
        expand(*source);return source;
    }
    void expand(std::string& source) {
        const auto nul=source.find('\0');if(nul!=std::string::npos)source.resize(nul);
        if(source.size()>65535)throw std::length_error("Script exceeds original source buffer");
        MissionScriptLexer::State state{std::size_t{0},0,0};std::size_t scan=0,expansions=0;
        for(;;) {
            const auto where=source.find("#include",scan);if(where==std::string::npos)return;
            scan=where;const auto after=where+8;
            if(after==source.size())return; //Native empty suffix leaves this final directive unchanged.
            if(++expansions>65536)throw std::runtime_error("Script include expansion exceeds native buffer progress");
            source.replace(where,8,8,' ');source[after]=' '; //Original writes eight spaces then restores its overwritten terminator to space.
            MissionScriptLexer lexer(source);lexer.state(state);lexer.next();state=lexer.state();
            const auto token=lexer.token();if(token.size()<2||token.front()!='"'||token.back()!='"')continue;
            //Original blanks from the char AFTER #include, through the complete
            //quoted filename. It does not jump its lexical cursor to `where`.
            if(after+token.size()>=source.size())throw std::runtime_error("Script include directive exceeds bounded source");
            for(std::size_t n=0;n<=token.size();++n)source[after+n]=' ';
            const auto name=token.substr(1,token.size()-2);
            auto included=read("Scripts\\"+resource_+"\\"+name);
            if(!included)included=read("Scripts\\include\\"+name);
            if(!included)continue; //Native missing include is ignored AFTER blanking its directive.
            if(included->find('\0')!=std::string::npos)throw std::runtime_error("Embedded include terminator requires original aliased-buffer behavior");
            if(included->size()>65535u-source.size())throw std::length_error("Expanded script exceeds original64KiB source buffer");
            source.insert(where,*included); //Actual include first, then the saved directive/suffix; cursor offset is unchanged.
        }
    }
private:
    std::optional<std::string> read(const std::string& path) {
        const auto handle=files_.open(path.c_str(),0);if(handle==0xffffffffu)return std::nullopt;
        struct Close {Files& files;std::uint32_t handle;~Close(){files.close(handle);}}close{files_,handle};
        std::vector<char> buffer(0xfffe);const auto count=files_.read(handle,buffer.data(),static_cast<std::uint32_t>(buffer.size()));
        if(count>buffer.size())throw std::runtime_error("Invalid script read count");
        return std::string(buffer.data(),count);
    }
    Files& files_;std::string resource_;
};
} //namespace hd2::game
#endif
