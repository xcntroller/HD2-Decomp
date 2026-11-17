#ifndef HD2_GAME_PHYSICS_OBJECT_PARAMETERS_H
#define HD2_GAME_PHYSICS_OBJECT_PARAMETERS_H

#include <cstdint>
#include <cstdlib>
#include <optional>
#include <stdexcept>
#include <string>
#include <functional>

namespace hd2::game {
struct PhysicsObjectHandle;

//00410B40's native44-byte construction record. Fields with no scene producer
//retain their original defaults; physics objects are separate from Scene2 actors.
template<class Frame>struct PhysicsObjectParameters {
    Frame frame{};
    std::uint32_t type=5;
    bool flag14=false,flag15=false,first_list=false,sight_pass=true;
    std::uint32_t field18=0;
    std::uint32_t material10=0; //Native+10→common object150; Versions fragment producer supplies part material.
    bool free_axis=false,tree=false,breakable=false,pickup=false;
    float angle=1.57079637050628662109375f;
    std::string break_model;
    float radius=-1.f;
    //Native record08 callback/C context, represented by one required typed
    //callable with captured context. ECX=event, EDX=the SAME stable handle.
    std::function<std::int32_t(unsigned,PhysicsObjectHandle&)> feedback;
};

//00410AF0 is strstr, not a token parser: even embedded substrings count. The
//original lowercases its1024-byte copy before every tag/value search. Native
//guard rejects the original buffer overflow; no game file is changed.
template<class Frame>
std::optional<PhysicsObjectParameters<Frame>> physics_scene_parameters(Frame frame,std::uint32_t kind,std::string text) {
    const auto nul=text.find('\0');if(nul!=std::string::npos)text.resize(nul);
    if(text.empty())return std::nullopt;
    if(text.size()>=1024)throw std::runtime_error("Physics frame metadata exceeds original buffer");
    for(auto& c:text)if(c>='A'&&c<='Z')c=char(c-'A'+'a');
    if(text.find("phobj")==std::string::npos||kind==9)return std::nullopt;
    PhysicsObjectParameters<Frame> value;value.frame=frame;
    const auto has=[&](const char* tag){return text.find(tag)!=std::string::npos;};
    value.sight_pass=!has("sightstop");
    if(has("can")) {value.type=2;value.first_list=true;return value;}
    if(has("cross"))value.type=6;
    value.free_axis=has("free")||has("axis");value.tree=has("tree");
    const auto broken=text.find("break");value.breakable=broken!=std::string::npos;value.pickup=value.breakable; //Native record1E→object204 is ONE flag.
    if(value.breakable) {
        const auto start=text.find_first_not_of(" \t\r\n\v\f",broken+5);
        if(start==std::string::npos)value.break_model="plotdira";
        else {
            const auto end=text.find_first_of(" \t\r\n\v\f",start);
            value.break_model=text.substr(start,end==std::string::npos?end:end-start);
            const auto dot=value.break_model.find('.');if(dot!=std::string::npos)value.break_model.resize(dot);
        }
    }
    const auto angle=text.find("angle");
    if(angle!=std::string::npos)
        value.angle=static_cast<float>(static_cast<long double>(std::strtod(text.c_str()+angle+5,nullptr))*0.008726646192371845245361328125f);
    return value;
}

} //namespace hd2::game
#endif
