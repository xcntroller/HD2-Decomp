//Callable native selected-byte analysis. No engine/DLL imports or execution.
#include "game/selected_resource_analysis.h"
#include "game/mission_table_reader.h"
#include "game/mission_script_bindings.h"
#include "frontend/game_data.h"
#include "game/mission_item_records.h"
#include "game/mission_tracks.h"
#include "io/data_manifest.h"
#include <fstream>
#include <iostream>
#include <iterator>
using namespace hd2;
static void emit(const char* family,const std::string& name,std::uint64_t value=0){
    std::cout<<io::data_manifest_text(family)<<'\t'<<io::data_manifest_text(name)<<'\t'<<value<<'\n';
}
struct TableReferences {
    void* table_frame(const std::string& name){emit("navigation-frame-reference",name);return nullptr;}
    void* table_actor(const std::string& name){emit("navigation-actor-reference",name);return nullptr;}
};
//Finite byte endpoint for the existing GameData parser, not an installation
//resolver. Only this verified numbered input exists in this analysis context.
struct SelectedGameDataFile {
    const std::vector<unsigned char>& bytes;std::uint32_t position=0;bool opened=false;
    std::uint32_t open(const char* name,std::uint32_t){if(opened||std::string(name)!="GameData\\Gamedata00.gdt")return 0xffffffffu;opened=true;position=0;return 1;}
    std::uint32_t open_write(const char*,std::uint32_t){throw std::runtime_error("Read-only static analysis");}
    std::uint32_t write(std::uint32_t,const void*,std::uint32_t){throw std::runtime_error("Read-only static analysis");}
    std::uint32_t seek(std::uint32_t,std::uint32_t offset,std::uint32_t origin){
        const auto next=std::uint64_t(origin==2?bytes.size():origin==1?position:0)+offset;
        if(next>bytes.size())throw std::runtime_error("GameData seek exceeds exact captured bytes");
        return position=static_cast<std::uint32_t>(next);
    }
    std::uint32_t read(std::uint32_t,void* out,std::uint32_t count){
        const auto n=static_cast<std::uint32_t>(std::min<std::size_t>(count,bytes.size()-position));
        if(n)std::memcpy(out,bytes.data()+position,n);
        position+=n;return n;
    }
    void close(std::uint32_t){opened=false;}
};
static void descriptors(const frontend::GameDataNode& node){
    if(node.kind==3){emit("mission-descriptor-resource",node.text24,node.word8);emit("mission-descriptor-flag2c",node.text24,node.flag2c);}
    for(const auto& child:node.children)descriptors(*child);
    for(const auto& child:node.related)descriptors(*child);
}
int main(int argc,char** argv){
    if(argc!=4){std::cerr<<"Usage: validate-selected-resource scene|tables|animation|bindings|gamedata|items|tracks FILE EXPECTED_SHA256\n";return 2;}
    try{
        std::ifstream file(argv[2],std::ios::binary);if(!file)throw std::runtime_error("Cannot open exact selected bytes");
        std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(file)),{});if(file.bad())throw std::runtime_error("Read failure");
        const auto digest=io::resource_sha256(bytes.data(),bytes.size());if(digest!=argv[3])throw std::runtime_error("Selected bytes fingerprint mismatch");
        std::cout<<"family\tname\tvalue\n";emit("sha256",digest,bytes.size());
        emit("evidence-level","static requirements only; no loaded population or executed branch");
        const std::string kind=argv[1];
        if(kind=="scene"){
            game::SelectedSceneRequirements analysis;analysis.read(bytes);
            bool unsupported=false;
            for(const auto& r:analysis.records()){emit(r.family.c_str(),r.name,r.value);unsupported|=r.family=="unsupported-required";}
            if(unsupported)throw std::runtime_error("Unsupported required static record; see partial report");
            emit("scope","chunk framing and typed actor properties; engine object acceptance unvalidated");
        }else if(kind=="tables"){
            game::TableStorage backend;TableReferences references;game::MissionTables<game::TableStorage> tables(backend);game::SerializedBytes cursor(bytes);
            unsigned index=0;for(auto& set:tables.sets()){game::read_mission_table(cursor,set,backend,references);emit("navigation-records",std::to_string(index++),set.count0);}
            emit("scope","production navigation parser; references reported without resolving engine objects");
        }else if(kind=="animation"){
            //Production decoder, with an explicitly unavailable resource endpoint.
            //No callback claims a successful engine animation load.
            struct References{using Reference=std::nullptr_t;int resolve_animation(const std::string& name,Reference&){emit("animation-reference",name);return -1;}}refs;
            game::AnimationTables<References> tables;tables.load(bytes,refs);
            emit("animation-records","00783BC0",tables.row_count);
            emit("scope","production fixed table parser; all animation resources unresolved");
        }else if(kind=="tracks"){
            game::MissionTracks tracks;if(!tracks.read(game::SerializedBytes(bytes)))throw std::runtime_error("Track root rejected");
            for(const auto& track:tracks.values())emit("track-definition",track->name,track->keys.size());
            emit("scope","production track definitions; playback unobserved");
        }else if(kind=="items"){
            game::SerializedBytes cursor(bytes);const auto records=game::read_mission_item_records(cursor);
            for(const auto& r:records){emit("equipment-definition",r.frame_name,static_cast<std::uint32_t>(r.definition));emit("equipment-placed",r.frame_name,r.placed);}
            emit("scope","production items.dat parser; catalogue/model/transfer owners unobserved");
        }else if(kind=="bindings"){
            game::MissionScriptBindings::read_records(game::SerializedBytes(bytes),[](const auto& binding){emit("script-frame-declaration",binding.frame);emit("script-resource-reference",binding.script);});
            emit("scope","production binding parser; frame filter, compilation and VM execution unobserved");
        }else if(kind=="gamedata"){
            SelectedGameDataFile input{bytes};frontend::GameData data;
            if(!data.load(input)||data.rejected_root())throw std::runtime_error("Selected GameData rejected");
            for(const auto& root:data.roots())descriptors(*root);
            emit("scope","only GameData00 decoded; profile/mode/selection eligibility unobserved");
        }else throw std::runtime_error("Unknown selected resource format");
        return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
