#ifndef HD2_GAME_FIRST_PERSON_PLAYBACK_STATE_H
#define HD2_GAME_FIRST_PERSON_PLAYBACK_STATE_H
#include <cstdint>
#include <vector>
namespace hd2::game {
template<class Frame>struct FirstPersonPlaybackState {
    struct Track {Frame clip{};std::uint32_t slot=0;float speed=0,weight=0;};
    Track current;
    std::vector<Track> fading; //Native84/88/8C, entries70..7F, borrowed cached clips.
    std::int32_t animation=0,variant=0;
};
}
#endif
