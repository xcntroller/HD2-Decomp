#ifndef HD2_FRONTEND_SOUND_MOVIE_CHANNELS_H
#define HD2_FRONTEND_SOUND_MOVIE_CHANNELS_H

#include "frontend/sound_movie_selection.h"

namespace hd2::frontend {

// Additional fields of manager00899728. Constructor004336D0 initializes movie68
// to null, but does not initialize the three secondary fade words or pending6c.
// Undefined original values stay unset, rather than silently choosing defaults.
// This borrowed state is not an automatic owner or the full manager destructor.
struct SecondarySoundMovieState {
    std::optional<float> target; // +5c
    std::optional<float> scale;  // +60
    std::optional<float> rate;   // +64
    void* movie = nullptr;      // +68, owned by the actual manager lifetime
};

template<class T>
const T& defined_sound_movie_value(const std::optional<T>& value) {
    if (!value) throw std::runtime_error("Undefined original sound movie state");
    return *value;
}

// 00432400: query secondary IShow opened, independent of the shared gate.
template<class Backend>
bool secondary_sound_movie_opened(const SecondarySoundMovieState& state, Backend& backend) {
    return state.movie && backend.movie_opened(state.movie);
}

// 00436ED0 differs from primary00436E90: rate is SIGNED, with no fabs.
inline void schedule_secondary_sound_movie_fade(SecondarySoundMovieState& state,
                                               float target, std::int32_t milliseconds) {
    const auto current = defined_sound_movie_value(state.scale);
    state.target = target;
    const auto duration = milliseconds < 1 ? 1 : milliseconds;
    state.rate = static_cast<float>((static_cast<long double>(current) - target) / duration);
}

// 00436F10: fade both channels to original float0.01, then queue the request.
// Signed division truncates toward zero, matching __ftol(int32 * float0.5).
// No gate or movie existence checks; metadata survives subsequent consumption.
inline void queue_primary_sound_movie(SettingsSoundState& primary, SoundMovieFade& fade,
                                     SecondarySoundMovieState& secondary,
                                     SoundMovieSelection& selection, std::int32_t request,
                                     std::int32_t milliseconds, std::uint32_t position) {
    const auto duration = milliseconds / 2;
    schedule_secondary_sound_movie_fade(secondary, 0.01f, duration);
    schedule_sound_movie_fade(fade, primary.movie_scale, 0.01f, duration);
    selection.pending = 1;
    selection.transition = SoundMovieRequest{request, duration, position};
}

// 00436F60 and 00436B60: re-resolve the slot after each engine call. IShow
// release is +04 (not the sample/frame release at +00) and runs even unopened.
template<class Backend>
void release_secondary_sound_movie(SecondarySoundMovieState& state, Backend& backend) {
    if (backend.movie_opened(state.movie)) {
        backend.stop_sound_movie(state.movie);
        backend.close_sound_movie(state.movie);
    }
    backend.release_sound_movie(state.movie);
    state.movie = nullptr;
}

// 00436F60, absent from the frozen census. This does NOT load the catalog.
// An opened secondary blocks ordinary selection; pending blocks cancellation too.
// Creation precedes validation, is ungated, and may retain an unopened object.
template<class Resources, class Backend>
void select_secondary_sound_movie(const SoundCatalog<Resources>& catalog,
                                 SettingsSoundState& primary, SoundMovieFade& fade,
                                 SecondarySoundMovieState& secondary,
                                 const SoundMovieSelection& selection,
                                 const std::uint32_t& gate, std::int32_t request,
                                 Backend& backend) {
    if (secondary_sound_movie_opened(secondary, backend) && request != -1) return;
    if (defined_sound_movie_value(selection.pending)) return;
    if (request == -1) {
        if (secondary.movie) release_secondary_sound_movie(secondary, backend);
        return; // Explicit cancellation does not restore the primary fade.
    }
    if (!secondary.movie) secondary.movie = backend.create_sound_movie();
    if (request < 0) return;
    if (catalog.groups.size() <= 12 || !catalog.groups[12])
        throw std::runtime_error("Invalid original music catalog group");
    const auto& group = *catalog.groups[12];
    if (static_cast<std::uint32_t>(request) >= group.entries.size()) return;
    const auto& entry = group.entries[static_cast<std::uint32_t>(request)];
    if (!entry) throw std::runtime_error("Null original music catalog entry");
    if (entry->variants.empty()) return;
    const auto choice = backend.sound_random_word() % entry->variants.size();
    const auto& variant = entry->variants[choice];
    if (!variant) throw std::runtime_error("Null original music catalog variant");
    const auto name = variant->name;
    const auto changed = change_sound_movie(gate, secondary.movie, name.c_str(), 0, 0, backend);
    secondary.scale = 0; // Even on a false helper result.
    if (changed) {
        schedule_secondary_sound_movie_fade(secondary, 1, 2000);
        schedule_sound_movie_fade(fade, primary.movie_scale, 0.15f, 2000);
    }
}

// Complete MOVIE PHASE00436A42..00436BC5 of update004368D0. In the original
// this follows active-voice processing inside a single bit2 gate, and precedes
// ungated listener/driver work. Caller must pass the gate snapshot taken BEFORE
// processing voices. This function does not stand in for the full manager update.
// Actual final-class virtual targets+60/+64 are primary selection/fade scheduling.
template<class Resources, class Files, class Backend>
void advance_sound_movie_channels(SoundCatalog<Resources>& catalog, Files& files,
                                  SettingsSoundState& primary, SoundMovieFade& fade,
                                  SecondarySoundMovieState& secondary,
                                  SoundMovieSelection& selection, const std::uint32_t& gate,
                                  bool gated_phase_enabled, std::int32_t milliseconds,
                                  Backend& backend) {
    if (!gated_phase_enabled) return;
    if (fade.target != primary.movie_scale) { // Unordered comparison also enters.
        const auto step = static_cast<long double>(milliseconds) * fade.rate;
        if (fade.target < primary.movie_scale) {
            const auto next = static_cast<long double>(primary.movie_scale) - step;
            primary.movie_scale = static_cast<float>(next);
            // Compare the x87 intermediate, not its already stored float32.
            if (next <= fade.target) {
                primary.movie_scale = fade.target;
                if (defined_sound_movie_value(selection.pending)) {
                    const auto request = defined_sound_movie_value(selection.transition);
                    selection.pending = 0; // Before invoking primary selection.
                    select_primary_sound_movie(catalog, files, primary, fade, selection,
                                               gate, request.request, request.position, backend);
                    schedule_sound_movie_fade(fade, primary.movie_scale, 1,
                        defined_sound_movie_value(selection.transition).duration);
                }
            }
        } else {
            const auto next = static_cast<long double>(primary.movie_scale) + step;
            primary.movie_scale = static_cast<float>(next);
            if (next >= fade.target) primary.movie_scale = fade.target;
        }
        if (primary.movie && backend.movie_opened(primary.movie))
            backend.movie_volume(primary.movie,
                static_cast<float>(static_cast<long double>(primary.movie_volume) * primary.movie_scale));
    }
    const auto secondary_target = defined_sound_movie_value(secondary.target);
    const auto secondary_current = defined_sound_movie_value(secondary.scale);
    if (secondary_target != secondary_current) {
        const auto next = static_cast<long double>(secondary_current) -
            static_cast<long double>(milliseconds) * defined_sound_movie_value(secondary.rate);
        secondary.scale = static_cast<float>(next);
        if (secondary_target < secondary_current) {
            if (next <= secondary_target) secondary.scale = secondary_target;
        } else {
            if (next >= secondary_target) secondary.scale = secondary_target;
        }
        if (secondary_sound_movie_opened(secondary, backend))
            backend.movie_volume(secondary.movie,
                static_cast<float>(static_cast<long double>(primary.movie_volume) * *secondary.scale));
    }
    if (secondary.movie) {
        const auto status = backend.sound_movie_status(secondary.movie);
        if (status == 1 || !backend.movie_opened(secondary.movie)) {
            release_secondary_sound_movie(secondary, backend);
            schedule_sound_movie_fade(fade, primary.movie_scale, 1, 10000);
        }
    }
    if (primary.movie && backend.sound_movie_status(primary.movie) == 1) {
        backend.seek_sound_movie(primary.movie, 0);
        backend.play_sound_movie(primary.movie);
    }
}

}
#endif
