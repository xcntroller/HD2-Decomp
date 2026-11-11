#ifndef HD2_GAME_MISSION_SCRIPT_BINDINGS_H
#define HD2_GAME_MISSION_SCRIPT_BINDINGS_H

#include "game/serialized_bytes.h"
#include <algorithm>

namespace hd2::game {

//00428600's binding file consists of PAIRS of string chunks: frame path and
//script resource name. It contains no VM instructions.00427730's later context
//creation/compilation remains a required separate consumer of these records.
class MissionScriptBindings {
public:
    struct Binding {std::string frame,script;};
    const std::vector<Binding>& values() const {return bindings_;}
    void clear() noexcept {bindings_.clear();} //00428A70, forward pair destruction.
    //Decode exact serialized pairs before the original frame-existence filter.
    //Inspection can report declared dependencies without manufacturing frames.
    template<class Visit>static void read_records(SerializedBytes bytes,Visit&& visit) {
        auto root=next_mission_chunk(bytes);
        if(!root)throw std::runtime_error("Mission script bindings have no root");
        //The original enters the first root without testing its ID, and
        //does not test either member's ID before reading the bounded string.
        while(auto name=next_mission_chunk(root->bytes)) {
            auto frame=text(name->bytes);
            auto script=next_mission_chunk(root->bytes);
            if(!script)throw std::runtime_error("Mission script binding has no script name");
            Binding binding{std::move(frame),text(script->bytes)};
            visit(std::move(binding));
        }
    }
    template<class Frames>void read(SerializedBytes bytes,Frames& frames) {
        clear();
        read_records(bytes,[&](Binding binding){
            //Original driver+88 only: no scene+7C fallback for this filter.
            if(frames.resolve_scene_path(binding.frame))bindings_.push_back(std::move(binding));
        });
    }
    template<class Files,class Frames>
    bool load(Files& files,Frames& frames,const std::string& resource,bool multiplayer) {
        clear();
        const auto path=std::string("missions\\")+resource+(multiplayer?"\\mpscripts.dta":"\\Scripts.dta");
        const auto handle=files.open(path.c_str(),0);
        if(handle==0xffffffffu)return false; //00428600 leaves an empty binding list.
        struct Close {Files& files;std::uint32_t handle;~Close(){files.close(handle);}} close{files,handle};
        const auto size=files.seek(handle,0,2);
        if(size==0xffffffffu||size>128u*1024u*1024u)throw std::runtime_error("Invalid mission script binding file size");
        if(files.seek(handle,0,0)!=0)throw std::runtime_error("Cannot rewind mission script bindings");
        std::vector<unsigned char> bytes(size);
        if(size&&files.read(handle,bytes.data(),size)!=size)throw std::runtime_error("Cannot read mission script bindings");
        read(SerializedBytes(bytes),frames);return true;
    }
private:
    static std::string text(SerializedBytes& bytes) {
        return bytes.section(std::min<std::size_t>(bytes.remaining(),999)).text();
    }
    std::vector<Binding> bindings_;
};

} //namespace hd2::game
#endif
