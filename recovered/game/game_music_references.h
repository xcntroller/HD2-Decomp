#ifndef HD2_GAME_GAME_MUSIC_REFERENCES_H
#define HD2_GAME_GAME_MUSIC_REFERENCES_H
#include <cstdint>
namespace hd2::game {
//Game+C8/CC/D0/D4. Loader supplies ACTUAL track IDs.0069E920/0069E970
//is a music reference, not a pause/input/game94 flag substitute.
class GameMusicReferences {
public:
    GameMusicReferences(std::int32_t normal,std::int32_t special):normal_track(normal),special_track(special){}
    template<class Sound>void reference(bool enabled,std::uint32_t mode,Sound& sound){
        if(enabled){++references;select(true,mode,sound);}
        else if(--references==0)select(false,mode,sound); //Native DWORD wrap.
    }
    template<class Sound>void select(bool special,std::uint32_t mode,Sound& sound){
        if(!(mode&2u))return;
        const auto track=(mode&7u)==7u?(special?special_track:normal_track):-1;
        if(sound.game_music_request()!=track)sound.queue_game_music(track,4000,0);
        countdown=5000;
    }
    std::int32_t normal_track,special_track;
    std::uint32_t countdown=0,references=0; //006989B0 resets D0/D4 after activation.
};
} //namespace hd2::game
#endif
