#ifndef HD2_GAME_SESSION_CONTEXT_MARKER_H
#define HD2_GAME_SESSION_CONTEXT_MARKER_H
#include "io/chunk_file.h"
#include <cstring>
#include <stdexcept>
#include <string>
namespace hd2::game {

//006B7A00 selected profile quick-context path, ordinary kind0 and standalone1.
//No save/load/restart is performed. Unrecognized modes in these native cases
//yield the actual empty path; unknown manager kinds require their real producer.
inline std::string session_quick_context_path(const std::string& profile,std::uint32_t kind,std::uint32_t mode){
    const char* name=nullptr;
    if(kind==0){switch(mode){case 1:name="CAM_quick.sav";break;case 2:name="LOW_quick.sav";break;case 3:name="CAR_quick.sav";break;default:return {};}}
    else if(kind==1){switch(mode){case 0:name="SMCAM_quick.sav";break;case 3:name="SMCAR_quick.sav";break;default:return {};}}
    else throw std::runtime_error("Session quick path requires actual unsupported game-mode kind");
    return std::string(profile.c_str())+name; //Original concatenates; caller supplies real006B7100 directory.
}

//006C4BD0's read-only activation predicate, NOT a gameplay restore. Actual
//controller state type1 admits the open; first root4E8C/childBB6 contains the
//same selected global mission ordinal. No snapshot/preparation data is applied.
//Malformed/short DWORD input is rejected instead of reading native unwritten data.
template<class Files>
bool read_session_context_marker(Files& files,const std::string& path,std::uint32_t mission,std::uint32_t active_state_type){
    if(active_state_type!=1)return false;
    io::ChunkFile<Files> file(files);if(!file.open(path.c_str(),0))return false;
    std::uint16_t id=0;if(!file.enter(id)||id!=0x4e8c)return false;
    const auto position=files.seek(file.state().handle,0,1);if(position==0xffffffffu)return false;
    const auto bits=file.state().offsets[file.state().depth]-position;
    std::int32_t remaining;std::memcpy(&remaining,&bits,4);if(remaining<1)return false;
    if(!file.enter(id)||id!=0xbb6)return false;
    std::array<unsigned char,4> bytes{};const bool read=file.read(bytes.data(),4);file.leave();
    if(!read)return false;
    const auto stored=std::uint32_t(bytes[0])|(std::uint32_t(bytes[1])<<8)|(std::uint32_t(bytes[2])<<16)|(std::uint32_t(bytes[3])<<24);
    return stored==mission;
}
} //namespace hd2::game
#endif
