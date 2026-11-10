#ifndef HD2_GAME_EFFECTS_WATER_SETTINGS_H
#define HD2_GAME_EFFECTS_WATER_SETTINGS_H

#include "game/serialized_bytes.h"
#include "frontend/pre_mission_data.h"
#include <array>
#include <cstring>

namespace hd2::game {

//00489C60: zero41 DWORDs at10C, but load EXACT40 DWORDs from the second
//chunk after version1 in the first. Chunk IDs are not tested. These are scalar
//settings, not frame pointers; loading later overwrites camera11C..124 fields.
class EffectsWaterSettings {
public:
    void reset()noexcept{bytes_.fill(0);}
    bool read(SerializedBytes bytes){
        reset();auto version=next_mission_chunk(bytes);if(!version)return false;
        if(version->bytes.remaining()<4||version->bytes.word()!=1)return false;
        auto data=next_mission_chunk(bytes);if(!data||data->bytes.remaining()<160)return false;
        const auto* raw=data->bytes.take(160);std::memcpy(bytes_.data(),raw,160);return true;
    }
    template<class Files>bool load(Files& files,const std::string& resource){
        reset();if(resource.empty())return false;
        auto bytes=frontend::pre_mission_detail::read_file(files,("missions\\"+resource+"\\watercam.set").c_str());
        return bytes?read(SerializedBytes(*bytes)):false;
    }
    bool enabled()const{return bytes_[0]!=0;} //Native10C.
    bool use_scene_water()const{return bytes_[2]!=0;} //Native10E.
    bool restore_child_visibility()const{return bytes_[157]!=0;} //Native1A9,00486520.
    void initial_camera(const std::array<float,3>& values){std::memcpy(bytes_.data()+16,values.data(),12);bytes_[156]=0;} //004885D0 overwrites11C/120/124 and clears1A8.
    std::string above_water_name()const{return name(28);} //Native128,00486580.
    std::string below_water_name()const{return name(92);} //Native168,00486750.
    const auto& serialized()const{return bytes_;}
private:
    std::string name(std::size_t offset)const{
        const auto* first=bytes_.data()+offset;const auto* end=static_cast<const unsigned char*>(std::memchr(first,0,64));
        if(!end)throw std::runtime_error("Water visibility frame name exceeds native settings buffer");
        return {reinterpret_cast<const char*>(first),reinterpret_cast<const char*>(end)};
    }
    std::array<unsigned char,164> bytes_{};
};
} //namespace hd2::game
#endif
