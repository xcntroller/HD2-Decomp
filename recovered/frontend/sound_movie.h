#ifndef HD2_FRONTEND_SOUND_MOVIE_H
#define HD2_FRONTEND_SOUND_MOVIE_H

#include <cstdint>
#include <cmath>
#include <string>

namespace hd2::frontend {

// 00437210 is the sound manager's shared IShow change helper, not the opening
// state's GameMovie owner. Movie slots +48 and +68 retain their factory refs;
// this operation stops/closes opened objects but NEVER releases or clears them.
// Gate00982100 bit2 clear returns true without even resolving the supplied slot.
// Backend and DLL must outlive these slots; full manager cleanup is separate.
template<class Backend>
bool change_sound_movie(const std::uint32_t& gate, void*& movie, const char* filename,
                         float volume, std::uint8_t loop, Backend& backend) {
    if (!(gate & 2)) return true;
    if (!movie && filename) movie = backend.create_sound_movie();
    if (!movie) return false;
    if (backend.movie_opened(movie)) {
        backend.stop_sound_movie(movie);
        backend.close_sound_movie(movie);
    }
    if (filename && *filename) {
        const auto path = std::string("sounds\\") + filename;
        const auto result = backend.open_sound_movie(movie, path.c_str(), loop ? 2u : 0u);
        // Original only EXACT zero starts playback, not every nonnegative code.
        if (result == 0) {
            backend.movie_volume(movie, volume);
            backend.play_sound_movie(movie);
        }
    }
    return true; // Includes failed open, empty name, and stop/close request.
}

// 00436E90: ungated primary-channel fade scheduling. Duration is SIGNED and
// clamped to at least one, rate is abs((current54-target)/duration), stored f32.
// The recovered movie phase consumes this state; the rest of the full manager
// frame update remains separate and is not replaced by a no-op.
struct SoundMovieFade {
    float target = 1; // +50
    float rate = 1;   // +58, original final constructor writes 1.
};
inline void schedule_sound_movie_fade(SoundMovieFade& fade, float current,
                                      float target, std::int32_t milliseconds) {
    fade.target = target;
    const auto duration = milliseconds < 1 ? 1 : milliseconds;
    const long double difference = static_cast<long double>(current) - target;
    const auto speed = difference / duration;
    fade.rate = static_cast<float>(std::fabs(speed));
}

}
#endif
