#ifndef HD2_GAME_MISSION_TRACKS_H
#define HD2_GAME_MISSION_TRACKS_H

#include "game/serialized_bytes.h"
#include <algorithm>
#include <array>
#include <memory>

namespace hd2::game {

struct MissionTrackKey {
    std::array<std::uint32_t,3> position{};
    std::array<std::uint32_t,4> rotation{};
    std::uint32_t scalar=0x3f800000u;
};
struct MissionTrack {
    std::string name;
    std::uint32_t field20=0x3dcccccdu,field24=0x3f000000u;
    std::vector<MissionTrackKey> keys;
};

//00402020/00402120: stored cutscene track definitions, not a playback system.
//00401D40 creates names once;00401ED0 searches in reverse, case-sensitive.
//All returned pointers remain stable until clear/load. Actual playback consumers
//must release their borrowed tracks before replacing these definitions.
class MissionTracks {
public:
    const std::vector<std::unique_ptr<MissionTrack>>& values() const {return tracks_;}
    MissionTrack* find(const std::string& name) const {
        for(auto i=tracks_.rbegin();i!=tracks_.rend();++i)if((*i)->name==name)return i->get();
        return nullptr;
    }
    void clear() noexcept {while(!tracks_.empty())tracks_.pop_back();}
    bool read(SerializedBytes bytes) {
        clear();
        auto root=next_mission_chunk(bytes);
        if(!root||root->id!=0x2426)return false;
        while(auto chunk=next_mission_chunk(root->bytes)) {
            if(chunk->id!=0x2454)throw std::runtime_error("Unsupported mission track record");
            read_track(chunk->bytes);
        }
        //00402120 closes after the FIRST root. Captured Tutorial has177 bytes
        //outside its608-byte root; those bytes are not additional records.
        return true;
    }
    template<class Files>bool load(Files& files,const std::string& resource) {
        clear(); //00402040 runs before even attempting the optional file open.
        const auto path=std::string("missions\\")+resource+"\\tracks.dat";
        const auto handle=files.open(path.c_str(),0);
        if(handle==0xffffffffu)return false;
        struct Close {Files& files;std::uint32_t handle;~Close(){files.close(handle);}} close{files,handle};
        const auto size=files.seek(handle,0,2);
        if(size==0xffffffffu||size>128u*1024u*1024u)throw std::runtime_error("Invalid mission track file size");
        if(files.seek(handle,0,0)!=0)throw std::runtime_error("Cannot rewind mission tracks");
        std::vector<unsigned char> bytes(size);
        if(size&&files.read(handle,bytes.data(),size)!=size)throw std::runtime_error("Cannot read mission tracks");
        return read(SerializedBytes(bytes));
    }
private:
    MissionTrack* ensure(const std::string& name) {
        if(auto* track=find(name))return track;
        auto track=std::make_unique<MissionTrack>();track->name=name;
        tracks_.push_back(std::move(track));return tracks_.back().get();
    }
    static std::string text(SerializedBytes& bytes) {
        return bytes.section(std::min<std::size_t>(bytes.remaining(),999)).text();
    }
    void read_track(SerializedBytes& bytes) {
        MissionTrack* track=nullptr;
        MissionTrackKey previous;
        bool position=false,rotation=false;
        while(auto chunk=next_mission_chunk(bytes)) {
            auto& data=chunk->bytes;
            switch(chunk->id) {
            case 0x2486:track=ensure(text(data));break;
            case 0x2490: {const auto word=data.word();if(track)track->field24=word;break;}
            case 0x249a: {const auto word=data.word();if(track)track->field20=word;break;}
            case 0x24b8: {
                previous.scalar=0x3f800000u; //Other key values carry forward when omitted.
                while(auto field=next_mission_chunk(data)) {
                    auto& value=field->bytes;
                    switch(field->id) {
                    case 0x251c:for(auto& word:previous.position)word=value.word();position=true;break;
                    case 0x2580:previous.scalar=value.word();break;
                    case 0x25e4: {
                        const auto first=value.word();
                        for(unsigned i=0;i!=3;++i)previous.rotation[i]=value.word();
                        previous.rotation[3]=first;rotation=true;break;
                    }
                    default:throw std::runtime_error("Unsupported mission track key field");
                    }
                }
                if(track) {
                    if(!position||!rotation)throw std::runtime_error("Mission track key has no initialized pose");
                    //00402880 uses native48-byte aligned records and grows by2;
                    //only the three position words, quaternion and scalar are data.
                    if(track->keys.size()==track->keys.capacity())track->keys.reserve(track->keys.size()+2);
                    track->keys.push_back(previous);
                }
                break;
            }
            default:throw std::runtime_error("Unsupported mission track field");
            }
        }
    }
    std::vector<std::unique_ptr<MissionTrack>> tracks_;
};

} //namespace hd2::game
#endif
