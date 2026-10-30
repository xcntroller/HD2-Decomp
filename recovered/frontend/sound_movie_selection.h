#ifndef HD2_FRONTEND_SOUND_MOVIE_SELECTION_H
#define HD2_FRONTEND_SOUND_MOVIE_SELECTION_H

#include "frontend/settings_sound.h"
#include "frontend/sound_catalog.h"
#include "frontend/sound_movie.h"

namespace hd2::frontend {

// Only the selection fields; this is not a replacement sound-manager lifetime.
struct SoundMovieSelection {
    std::int32_t requested = -1; // +4c, constructor004336d0
    std::optional<std::uint8_t> pending; // +6c, undefined until first write
};

// 004370F0 (absent from frozen census). Catalog load is ungated and its unusual
// result ignored. Stop/close runs BEFORE request validation. Music is group12,
// indexed by request; one CRT random word chooses a variant with unsigned modulo.
// Settings and fades are the SAME state consumed by the recovered setters.
template<class Resources, class Files, class Backend>
void select_primary_sound_movie(SoundCatalog<Resources>& catalog, Files& files,
                                SettingsSoundState& state, SoundMovieFade& fade,
                                SoundMovieSelection& selection, const std::uint32_t& gate,
                                std::int32_t request, std::uint32_t position, Backend& backend) {
    catalog.load(files);
    change_sound_movie(gate, state.movie, nullptr, 0, 1, backend);
    selection.pending = 0;
    selection.requested = request;
    if (request < 0) return;
    if (catalog.groups.size() <= 12 || !catalog.groups[12])
        throw std::runtime_error("Invalid original music catalog group");
    const auto& group = *catalog.groups[12];
    if (static_cast<std::uint32_t>(request) >= group.entries.size()) return;
    const auto& entry = group.entries[static_cast<std::uint32_t>(request)];
    if (!entry) throw std::runtime_error("Null original music catalog entry");
    if (entry->variants.empty()) return;
    const auto choice = backend.sound_random_word() % entry->variants.size();
    fade.target = 1;
    state.movie_scale = 0; // Fade RATE is deliberately left alone.
    const auto& variant = entry->variants[choice];
    if (!variant) throw std::runtime_error("Null original music catalog variant");
    const auto name = variant->name; // Original takes a retained COW string copy.
    const auto volume = static_cast<float>(static_cast<long double>(state.movie_volume) * state.movie_scale);
    change_sound_movie(gate, state.movie, name.c_str(), volume, 1, backend);
    // No additional gate: an existing opened movie may seek even with bit2 clear.
    if (position && state.movie && backend.movie_opened(state.movie))
        backend.seek_sound_movie(state.movie, position);
}

}
#endif
