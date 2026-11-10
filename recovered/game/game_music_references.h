#ifndef HD2_GAME_GAME_MUSIC_REFERENCES_H
#define HD2_GAME_GAME_MUSIC_REFERENCES_H
#include <cstdint>
#include <cstring>
namespace hd2::game {
//Game+C8/CC/D0/D4. Loader supplies ACTUAL track IDs.0069E920/0069E970
//is a music reference, not a pause/input/game94 flag substitute.
class GameMusicReferences {
public:
    struct LiveTracks {const std::uint32_t& normal;const std::uint32_t& special;};
    explicit GameMusicReferences(LiveTracks tracks):normal_track(-1),special_track(-1),normal_(&tracks.normal),special_(&tracks.special){}
    GameMusicReferences(std::int32_t normal,std::int32_t special):normal_track(normal),special_track(special){}
    template<class Sound>void reference(bool enabled,std::uint32_t mode,Sound& sound){
        if(enabled){++references;select(true,mode,sound);}
        else if(--references==0)select(false,mode,sound); //Native DWORD wrap.
    }
    template<class Sound>void select(bool special,std::uint32_t mode,Sound& sound){
        if(!(mode&2u))return;
        const auto track=(mode&7u)==7u?current_track(special):-1;
        if(sound.game_music_request()!=track)sound.queue_game_music(track,4000,0);
        countdown=5000;
    }
    std::int32_t normal_track,special_track;
    std::uint32_t countdown=0,references=0; //006989B0 resets D0/D4 after activation.
private:
    std::int32_t current_track(bool special)const {
        const auto* live=special?special_:normal_;if(!live)return special?special_track:normal_track;
        std::int32_t value;std::memcpy(&value,live,4);return value;
    }
    const std::uint32_t* normal_=nullptr;const std::uint32_t* special_=nullptr;
};
} //namespace hd2::game
#endif
