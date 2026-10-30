#ifndef HD2_FRONTEND_SOUND_RUNTIME_H
#define HD2_FRONTEND_SOUND_RUNTIME_H

#include "frontend/sound_movie_channels.h"
#include <algorithm>
#include <array>
#include <limits>

namespace hd2::frontend {

using SoundPosition = std::array<float, 4>;
// Borrowed live application/scene view. Its owner supplies the actual camera's
// current sector; creation of that owner is separate from these sound bodies.
// A null application view is the original missing-application branch.
struct SoundWorldView { void* scene; void* listener_sector; };
// 006B66B0: view of the actual application+24 owner's +04/+224 fields.
struct SoundApplicationMode { void* activity; std::uint32_t mode; };
inline bool sound_application_fallback(const SoundApplicationMode& state) {
    return state.activity && state.mode == 3;
}
struct SoundSectorData { void* sector; float area = 0; float scale = 0.1f; };
struct TrackedSound {
    void* sound;
    std::uint8_t enabled;
    void* sector;
    float base_volume;
    float scale = 1;
};
// Fields consumed here from the original zero-filled 0x40-byte voice record.
// Remaining playback fields belong to the still separate voice producer.
struct SoundVoice {
    std::uint8_t active = 0; // +10
    void* sound = nullptr;  // +14, owned reference
    float target_volume = 0; // +24
    std::uint32_t flags = 0; // +34
    CatalogSoundVariant* cache = nullptr; // +38, borrowed catalog owner
};
struct SoundVoiceGroup {
    std::uint32_t limit = 3; // +a4
    std::int32_t cursor = 0; // +b4
    std::vector<std::int32_t> slots = std::vector<std::int32_t>(3, -1); // +c4
};
struct SoundRuntimeState {
    std::vector<std::array<void*, 3>> registrations; // +0c, borrowed records
    std::vector<std::unique_ptr<SoundVoice>> voices; // +1c
    std::vector<std::int32_t> free_voices; // +2c
    std::uint8_t volume_settled = 0; // +40
    std::vector<std::unique_ptr<SoundSectorData>> sectors; // +80
    std::vector<std::unique_ptr<TrackedSound>> tracked; // +90
    std::optional<void*> listener_sector; // +9c, initialized by startup/static owner
    std::optional<std::int32_t> driver_delay; // +a0
    std::array<SoundVoiceGroup, 4> groups;
};

inline std::int32_t sound_signed_word(std::uint32_t bits) {
    std::int32_t result;
    std::memcpy(&result, &bits, 4);
    return result;
}
// 004324C0: preserve prefix; appended slots are -1. Cursor comparison is SIGNED.
inline void set_sound_voice_limit(SoundRuntimeState& state, std::uint32_t group,
                                 std::uint32_t limit) {
    if (group >= state.groups.size() || limit > 0x3fffffffu)
        throw std::length_error("Invalid original sound voice group size");
    auto& value = state.groups[group];
    value.limit = limit;
    if (value.cursor >= sound_signed_word(limit)) value.cursor = 0;
    value.slots.resize(limit, -1);
}

// 004361B0: unconditional owned-voice teardown, not the pooled stop operation.
template<class Backend>
void destroy_sound_voice(SoundVoice& voice, Backend& backend) {
    if (voice.sound) {
        backend.sound_volume(voice.sound, 0);
        backend.sound_mode(voice.sound, 0, 1);
        backend.update_sound(voice.sound);
        backend.release_catalog_sound(voice.sound);
    }
    voice.sound = nullptr;
}

// 00437340: -1 is already invalid; all other input handles become -1, including
// bad/inactive handles. Driver unavailable leaves the reference/cache untouched.
// A cached voice transfers its reference into extra-sample storage WITHOUT addref.
template<class Backend>
void stop_sound_voice(SoundRuntimeState& state, std::int32_t& handle, Backend& backend) {
    if (handle == -1) return;
    if (handle >= 0 && static_cast<std::uint32_t>(handle) < state.voices.size() &&
            state.voices[handle] && state.voices[handle]->active) {
        auto voice = [&]() -> SoundVoice& { return *state.voices.at(static_cast<std::uint32_t>(handle)); };
        if (backend.sound_driver_initialized() && voice().sound) {
            backend.enable_sound(voice().sound, 0);
            backend.sound_volume(voice().sound, 0);
            if (voice().cache) {
                voice().cache->sounds.push_back(voice().sound);
                voice().cache = nullptr;
            } else backend.release_catalog_sound(voice().sound);
            voice().sound = nullptr;
        }
        voice().active = 0;
        state.free_voices.push_back(handle);
    }
    handle = -1;
}

// 004374A0: bounded handle lookup. Original dereferences null before its late
// null test; native rejects that undefined in-range slot rather than crashing.
inline void* sound_voice_frame(const SoundRuntimeState& state, std::int32_t handle) {
    if (handle < 0 || static_cast<std::uint32_t>(handle) >= state.voices.size()) return nullptr;
    if (!state.voices[handle]) throw std::runtime_error("Null original sound voice slot");
    return state.voices[handle]->active ? state.voices[handle]->sound : nullptr;
}

// 004368EA..00436A41. Gate snapshot belongs to the outer update, not each voice.
template<class Backend>
void advance_sound_voices(SoundRuntimeState& state, bool enabled,
                          std::int32_t milliseconds, Backend& backend) {
    if (!enabled) return;
    for (auto i = state.voices.size(); i-- != 0;) {
        auto voice = [&]() -> SoundVoice& { return *state.voices.at(i); };
        if (!state.voices[i] || !voice().active) continue;
        if (!voice().sound) {
            voice().active = 0;
            state.free_voices.push_back(static_cast<std::int32_t>(i));
        } else if (!backend.sound_active(voice().sound, 0)) {
            if (!(voice().flags & 4)) {
                auto handle = static_cast<std::int32_t>(i);
                stop_sound_voice(state, handle, backend);
            }
        } else if (voice().flags & 0x10) {
            const auto next = static_cast<long double>(backend.sound_volume(voice().sound)) +
                static_cast<long double>(milliseconds) * 0.001f;
            float volume = static_cast<float>(next);
            // Strict greater: equality and unordered do not clear bit10.
            if (next > voice().target_volume) {
                volume = voice().target_volume;
                voice().flags -= 0x10;
            }
            backend.sound_volume(voice().sound, volume);
            backend.update_sound(voice().sound);
        }
    }
}

// 00432490: retain base volume and caller scale independently of the live word.
template<class Backend>
void scale_tracked_sound(TrackedSound& sound, float scale, Backend& backend) {
    sound.scale = scale;
    backend.sound_volume(sound.sound,
        static_cast<float>(static_cast<long double>(scale) * sound.base_volume));
    backend.update_sound(sound.sound);
}

// 00432420/00432710: reverse lookup; constructor queries sector even disabled.
// Existing entries retain scale, reread live base volume, then apply it again.
template<class Backend>
TrackedSound& track_sound(SoundRuntimeState& state, void* frame,
                          std::uint8_t enabled, Backend& backend) {
    for (auto i = state.tracked.size(); i-- != 0;) {
        auto& sound = *state.tracked[i];
        if (sound.sound != frame) continue;
        sound.enabled = enabled;
        if (enabled) sound.sector = backend.sound_sector(backend.sound_world()->scene,
                                                       backend.sound_position(frame));
        sound.base_volume = backend.sound_volume(frame);
        scale_tracked_sound(sound, sound.scale, backend);
        return sound;
    }
    const auto position = backend.sound_position(frame);
    auto sound = std::make_unique<TrackedSound>(TrackedSound{frame, enabled,
        backend.sound_sector(backend.sound_world()->scene, position), backend.sound_volume(frame), 1});
    backend.retain_tracked_sound(frame); // Original increments the frame's WORD+e4.
    auto& result = *sound;
    state.tracked.push_back(std::move(sound));
    return result;
}

// 00433330's original distance shortcut, based on the STORED squared float and
// its signed bits. Preserve sum order z²+y²+x² and the near-unit shortcut.
inline long double sound_distance(const SoundPosition& first, const SoundPosition& second) {
    const auto x = static_cast<long double>(first[0]) - second[0];
    const auto y = static_cast<long double>(first[1]) - second[1];
    const auto z = static_cast<long double>(first[2]) - second[2];
    const auto squared = static_cast<float>((z * z + y * y) + x * x);
    std::uint32_t bits;
    std::memcpy(&bits, &squared, 4);
    if (sound_signed_word(bits) < 0x322bcc77) return 0;
    if (std::fabs(static_cast<long double>(squared) - 1) < 1.0e-8f) return 1;
    return std::sqrt(static_cast<long double>(squared));
}

// 00433330: sector lookup uses copied positions; distance uses original inputs.
// Only a root/non-root crossing uses metadata+08; other sector pairs scale1.
template<class Backend>
long double sound_sector_distance(float& scale, const SoundPosition& first,
                                   const SoundPosition& second, void* first_sector,
                                   void* second_sector, Backend& backend) {
    const auto first_copy = first, second_copy = second;
    if (!first_sector) first_sector = backend.sound_sector(backend.sound_world()->scene, first_copy);
    if (!second_sector) second_sector = backend.sound_sector(backend.sound_world()->scene, second_copy);
    scale = 1;
    if (first_sector != second_sector) {
        auto* primary = backend.primary_sound_sector(backend.sound_world()->scene);
        if (first_sector == primary) scale = backend.sector_sound_data(second_sector)->scale;
        else if (second_sector == primary) scale = backend.sector_sound_data(first_sector)->scale;
    }
    return sound_distance(first, second);
}

// 00432900: both directed portal arrays contribute only links to the root.
// Each contribution is stored as float32. Callback returns zero to continue.
template<class Backend>
std::int32_t add_sound_sector(SoundRuntimeState& state, void* sector, Backend& backend) {
    auto data = std::make_unique<SoundSectorData>();
    data->sector = sector;
    backend.sector_sound_data(sector, data.get());
    auto* root = backend.primary_sound_sector(backend.sound_world()->scene);
    for (unsigned side = 0; side != 2; ++side) {
        for (auto i = backend.sound_portal_count(sector, side); i-- != 0;) {
            auto* portal = backend.sound_portal(sector, side, i);
            if (backend.sound_portal_sector(portal, side) != root) continue;
            auto radius = side ? 0.0f : backend.sound_portal_radius(portal);
            const auto weight = backend.sound_portal_opened(portal) ? 1.0f : 0.1f;
            if (side) radius = backend.sound_portal_radius(portal);
            data->area = static_cast<float>(static_cast<long double>(radius) * radius *
                3.1415927410125732421875f * weight + data->area);
        }
    }
    if (data->area > 40) data->area = 40;
    data->scale = static_cast<float>((1 - (40 - static_cast<long double>(data->area)) *
        0.025f) * 0.9f + data->scale);
    state.sectors.push_back(std::move(data));
    return 0;
}
// 004328D0 and00432860. Metadata attaches to actual sector userdata; cleanup
// clears that binding before freeing each object, retaining vector capacity.
template<class Backend>
void build_sound_sectors(SoundRuntimeState& state, Backend& backend) {
    backend.enumerate_sound_sectors(backend.primary_sound_sector(backend.sound_world()->scene),
        [&](void* sector) { return add_sound_sector(state, sector, backend); });
}
template<class Backend>
void clear_sound_sectors(SoundRuntimeState& state, Backend& backend) {
    for (auto i = state.sectors.size(); i-- != 0;)
        if (state.sectors[i]) backend.sector_sound_data(state.sectors[i]->sector, nullptr);
    state.sectors.clear();
}

// Ungated00436BC6..00436E8F, including the original integer driver fade.
// Clock00899830 is wrapping state shared across manager instances, not per call.
template<class Backend>
void advance_sound_environment(SoundRuntimeState& state, SettingsSoundState& settings,
                                std::uint32_t& clock, std::int32_t milliseconds,
                                Backend& backend) {
    if (!backend.sound_world()) return;
    clock += static_cast<std::uint32_t>(milliseconds);
    backend.sound_position(backend.scene_sound_listener(backend.sound_world()->scene));
    if (sound_signed_word(clock) > 1000) {
        clock = (clock - 1) % 1000 + 1; // 1000 stays1000; 1001 becomes1.
        if (defined_sound_movie_value(state.listener_sector)) {
            for (auto i = state.tracked.size(); i-- != 0;) {
                auto tracked = [&]() -> TrackedSound& { return *state.tracked.at(i); };
                if (!tracked().enabled) continue;
                auto* sector = backend.sound_sector(backend.sound_world()->scene,
                                                    backend.sound_position(tracked().sound));
                if (sector == tracked().sector) continue;
                tracked().sector = sector;
                float scale = 1;
                const SoundPosition zero{};
                sound_sector_distance(scale, zero, zero, sector,
                    defined_sound_movie_value(state.listener_sector), backend);
                scale_tracked_sound(tracked(), scale, backend);
            }
        }
    }
    auto* listener = backend.sound_world()->listener_sector;
    if (defined_sound_movie_value(state.listener_sector) != listener) {
        for (auto i = state.tracked.size(); i-- != 0;) {
            float scale = 1;
            const SoundPosition zero{};
            sound_sector_distance(scale, zero, zero, state.tracked[i]->sector, listener, backend);
            scale_tracked_sound(*state.tracked[i], scale, backend); // Includes disabled entries.
        }
        state.listener_sector = listener;
    }
    auto delay = defined_sound_movie_value(state.driver_delay);
    if (delay) {
        delay = sound_signed_word(static_cast<std::uint32_t>(delay) - static_cast<std::uint32_t>(milliseconds));
        if (delay <= 0) delay = 0;
        state.driver_delay = delay;
    }
    if (state.volume_settled) return;
    if (delay) {
        const auto factor = delay >= 2000 ? 0 : (2000 - delay) / 2000; // INTEGER division.
        backend.sound_driver_volume(static_cast<float>(static_cast<long double>(factor) * settings.driver_volume));
    } else {
        backend.sound_driver_volume(settings.driver_volume);
        state.volume_settled = 1;
    }
}

// Entire004368D0: one initial gate, voices -> movies -> ungated environment.
template<class Resources, class Files, class Backend>
void advance_sound_runtime(SoundRuntimeState& state, SoundCatalog<Resources>& catalog,
                           Files& files, SettingsSoundState& settings, SoundMovieFade& fade,
                           SecondarySoundMovieState& secondary, SoundMovieSelection& selection,
                           const std::uint32_t& gate, std::uint32_t& clock,
                           std::int32_t milliseconds, Backend& backend) {
    const bool enabled = (gate & 2) != 0;
    advance_sound_voices(state, enabled, milliseconds, backend);
    advance_sound_movie_channels(catalog, files, settings, fade, secondary, selection,
                                 gate, enabled, milliseconds, backend);
    advance_sound_environment(state, settings, clock, milliseconds, backend);
}

// Cached sample cleanup portion of004361F0: groups/entries/variants reverse,
// primary then extras reverse; clear extras but retain their capacity and names.
template<class Resources, class Backend>
void clear_sound_samples(SoundCatalog<Resources>& catalog, Backend& backend) {
    for (auto g = catalog.groups.size(); g-- != 0;)
        for (auto e = catalog.groups.at(g)->entries.size(); e-- != 0;)
            for (auto v = catalog.groups[g]->entries[e]->variants.size(); v-- != 0;) {
                auto& sample = *catalog.groups[g]->entries[e]->variants[v];
                if (sample.sound) {
                    backend.release_catalog_sound(sample.sound);
                    sample.sound = nullptr;
                }
                for (auto i = sample.sounds.size(); i-- != 0;)
                    if (sample.sounds[i]) backend.release_catalog_sound(sample.sounds[i]);
                sample.sounds.clear();
            }
}

// 004361F0: actual006B66B0 fields are consulted only when gate2 is clear.
template<class Resources, class Files, class Backend>
void reset_sound_runtime(SoundRuntimeState& state, SoundCatalog<Resources>& catalog,
                         Files& files, SettingsSoundState& settings, SoundMovieFade& fade,
                         SecondarySoundMovieState& secondary, SoundMovieSelection& selection,
                         const std::uint32_t& gate, const SoundApplicationMode& application,
                         std::uint8_t clear_samples, std::uint8_t retain_primary, Backend& backend) {
    if (!(gate & 2) && !sound_application_fallback(application)) return;
    for (auto& group : state.groups) {
        std::fill(group.slots.begin(), group.slots.end(), -1);
        group.cursor = 0;
    }
    for (auto i = state.voices.size(); i-- != 0;)
        if (state.voices[i]) destroy_sound_voice(*state.voices[i], backend);
    decltype(state.voices){}.swap(state.voices);
    for (auto i = state.tracked.size(); i-- != 0;)
        if (state.tracked[i]) backend.release_catalog_sound(state.tracked[i]->sound);
    state.tracked.clear(); // Retain capacity, unlike voices/free/registrations.
    decltype(state.free_voices){}.swap(state.free_voices);
    decltype(state.registrations){}.swap(state.registrations);
    if (settings.movie && !retain_primary) {
        select_primary_sound_movie(catalog, files, settings, fade, selection, gate, -1, 0, backend);
        backend.release_sound_movie(settings.movie);
        settings.movie = nullptr;
    }
    if (secondary.movie) {
        backend.release_sound_movie(secondary.movie); // No explicit stop/close here.
        secondary.movie = nullptr;
    }
    if (!retain_primary) {
        settings.movie_scale = 1;
        fade.target = 1;
        selection.pending = 0;
    }
    secondary.scale = 1;
    secondary.target = 1; // Both rates, request and metadata deliberately survive.
    if (clear_samples) clear_sound_samples(catalog, backend);
    state.volume_settled = 0;
}

// 00433840/004338A0: gate BIT1 (value1), independently of frame-update bit2.
// Reset may legitimately be skipped by its own gate, while startup continues.
template<class Resources, class Files, class Backend>
void start_sound_runtime(SoundRuntimeState& state, SoundCatalog<Resources>& catalog,
                         Files& files, SettingsSoundState& settings, SoundMovieFade& fade,
                         SecondarySoundMovieState& secondary, SoundMovieSelection& selection,
                         const std::uint32_t& gate, const SoundApplicationMode& application,
                         std::uint8_t force, Backend& backend) {
    if (!force && (gate & 1)) return;
    reset_sound_runtime(state, catalog, files, settings, fade, secondary, selection,
                         gate, application, 1, 1, backend);
    catalog.load(files);
    build_sound_sectors(state, backend);
    state.listener_sector = nullptr;
    state.driver_delay = 2000;
    backend.sound_driver_volume(0);
}
template<class Resources, class Files, class Backend>
void stop_sound_runtime(SoundRuntimeState& state, SoundCatalog<Resources>& catalog,
                        Files& files, SettingsSoundState& settings, SoundMovieFade& fade,
                        SecondarySoundMovieState& secondary, SoundMovieSelection& selection,
                        const std::uint32_t& gate, const SoundApplicationMode& application,
                        std::uint8_t force, Backend& backend) {
    if (!force && (gate & 1)) return;
    reset_sound_runtime(state, catalog, files, settings, fade, secondary, selection,
                         gate, application, 1, 0, backend);
    clear_sound_sectors(state, backend);
}

}
#endif
