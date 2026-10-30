#ifndef HD2_FRONTEND_PROFILE_SETTINGS_H
#define HD2_FRONTEND_PROFILE_SETTINGS_H

#include "frontend/profile_directories.h"
#include "input/binding_defaults.h"
#include "input/input_manager.h"
#include <cmath>

namespace hd2::frontend {

// On-disk 60-byte value at profile owner+38, independent of native object ABI.
// Byte storage preserves partial transfers, float bits and unaligned presets.
struct ProfileSettings {
    std::array<std::uint8_t, 0x3c> bytes{};
    std::uint32_t word(std::size_t offset) const {
        return std::uint32_t{bytes.at(offset)} | (std::uint32_t{bytes.at(offset + 1)} << 8) |
               (std::uint32_t{bytes.at(offset + 2)} << 16) | (std::uint32_t{bytes.at(offset + 3)} << 24);
    }
    void word(std::size_t offset, std::uint32_t value) {
        for (unsigned i = 0; i < 4; ++i) bytes.at(offset + i) = static_cast<std::uint8_t>(value >> (i * 8));
    }
    float real(std::size_t offset) const {
        const auto bits = word(offset);
        float value;
        std::memcpy(&value, &bits, 4);
        return value;
    }
    // 006B33E0: all qualities other than 0/1/2 use the fourth block.
    std::uint8_t quality_byte(std::size_t index) const {
        if (index >= 7) throw std::out_of_range("Invalid settings quality byte");
        const auto quality = word(0x18);
        return bytes[(quality < 3 ? 0x1c + quality * 7 : 0x31) + index];
    }
};

// 006B3420 does NOT write +14. Direct helper use preserves it. 006B31C0
// copies an undefined stack word there; native reset instead defines it as zero.
// No recovered application helper consumes that word.
inline void initialize_profile_settings(ProfileSettings& value) {
    value.word(0, 0x42853333);
    for (std::size_t at = 4; at <= 0x10; at += 4) value.word(at, 0x42480000);
    value.word(0x18, 2);
    constexpr std::array<std::uint8_t, 32> tail{{
        0,0,0,0,0,0,0, 0,1,0,1,0,1,0, 1,1,1,1,1,1,1,
        0,1,0,1,0,1,1, 0,0,1,1
    }};
    std::copy(tail.begin(), tail.end(), value.bytes.begin() + 0x1c);
}

struct ProfileOptions {
    ProfileSettings settings;
    input::BindingConfiguration bindings;
};

template<class Services>
void reset_profile_options(ProfileOptions& options, input::BindingInputState& input, Services& services) {
    ProfileSettings defaults{};
    initialize_profile_settings(defaults);
    options.settings = defaults;
    input::reset_binding_defaults(options.bindings, input, services, 0);
}

template<class Files>
std::int32_t settings_chunk_remaining(Files& files, const io::ChunkFile<Files>& file) {
    const auto& state = file.state();
    if (!state.opened || !state.depth) return 0;
    const auto position = files.seek(state.handle, 0, 1);
    if (position == 0xffffffffu) return 0;
    const auto bits = state.offsets[state.depth] - position;
    std::int32_t remaining;
    std::memcpy(&remaining, &bits, 4);
    return remaining;
}

// 006A8900: only clear configuration records, preserving mode and installed
// manager state. Header/version short reads have undefined/stale provenance in
// original; native treats them as the corresponding invalid-header branch.
template<class Files, class Services>
bool read_profile_bindings(input::BindingConfiguration& configuration,
                           input::BindingInputState& input, Services& services,
                           Files& files, io::ChunkFile<Files>& file) {
    input::clear_binding_settings(configuration);
    const auto fallback = [&] { input::reset_binding_defaults(configuration, input, services, 0); };
    std::uint16_t tag = 0;
    if (!file.enter(tag) || tag != 0x22b8) {
        fallback(); file.leave(); return false;
    }
    std::array<std::uint8_t, 4> version{};
    const auto full_version = file.read(version.data(), 4);
    file.leave();
    if (!full_version || version != std::array<std::uint8_t, 4>{{0x42,0,0,0}}) {
        fallback(); return false; // Already left version, not the containing chunk.
    }
    if (!file.enter(tag) || tag != 0x22b9) {
        fallback(); file.leave(); return false;
    }
    std::array<std::uint8_t, 0x1c> scratch{}; // Reused across partial reads.
    while (settings_chunk_remaining(files, file) != 0) {
        // Original can append forever for 1..27 trailing bytes or zero progress.
        // Native rejects these states, retaining earlier appended records.
        if (settings_chunk_remaining(files, file) < 0x1c)
            throw std::runtime_error("Incomplete profile binding record");
        const auto before = files.seek(file.state().handle, 0, 1);
        file.read(scratch.data(), static_cast<std::uint32_t>(scratch.size()));
        if (files.seek(file.state().handle, 0, 1) == before)
            throw std::runtime_error("Profile binding read made no progress");
        scratch[0x18] = 1;
        const auto word = [&](std::size_t at) {
            return std::uint32_t{scratch[at]} | (std::uint32_t{scratch[at+1]} << 8) |
                   (std::uint32_t{scratch[at+2]} << 16) | (std::uint32_t{scratch[at+3]} << 24);
        };
        const auto index_bits = word(0);
        std::int32_t index;
        std::memcpy(&index, &index_bits, 4);
        configuration.settings.push_back({index,
            static_cast<std::uint16_t>(scratch[4] | (std::uint16_t{scratch[5]} << 8)),
            static_cast<std::uint16_t>(scratch[6] | (std::uint16_t{scratch[7]} << 8)),
            word(8), word(12), word(16), word(20), scratch[24], {{scratch[25],scratch[26],scratch[27]}}});
    }
    file.leave();
    input::apply_dirty_bindings(configuration, input, services);
    return true;
}

// 006B3750: deliberately ignore field read results, including partial backend
// overwrites. A wrong child still gets left; caller owns the outer 25A5 leave.
template<class Files>
void read_profile_settings(ProfileSettings& settings, io::ChunkFile<Files>& file) {
    std::uint16_t tag = 0;
    if (file.enter(tag) && tag == 0x1201) {
        for (std::size_t at = 0; at <= 0x18; at += 4) file.read(settings.bytes.data() + at, 4);
        for (std::size_t at = 0x1c; at <= 0x31; at += 7) file.read(settings.bytes.data() + at, 7);
        for (std::size_t at = 0x38; at <= 0x3b; ++at) file.read(settings.bytes.data() + at, 1);
    }
    file.leave();
}

// 006B2D80: three sequential top-level chunks, not a root with two children.
// Inner reader booleans are ignored. Both outer fallbacks reset BOTH values.
template<class Files, class Services>
bool load_profile_options(ProfileOptions& options, const ProfileDirectories& profiles,
                          Files& files, bool wide, input::BindingInputState& input, Services& services) {
    io::ChunkFile<Files> file(files);
    const auto fallback = [&] { reset_profile_options(options, input, services); };
    const auto path = profiles.current_directory(wide) + "Options.bin";
    std::uint16_t tag = 0;
    if (!file.open(path.c_str()) || !file.enter(tag) || tag != 0x25a0) {
        fallback(); file.close(); return false;
    }
    std::array<std::uint8_t, 4> version{};
    const auto full_version = file.read(version.data(), 4);
    file.leave();
    if (!full_version || version != std::array<std::uint8_t,4>{{0x41,1,0,0}}) {
        fallback(); file.close(); return false;
    }
    if (file.enter(tag) && tag == 0x25a6)
        read_profile_bindings(options.bindings, input, services, files, file);
    else fallback();
    file.leave();
    // Native rejects a failed header rather than interpreting the stale tag.
    if (file.enter(tag) && tag == 0x25a5) read_profile_settings(options.settings, file);
    else fallback();
    file.leave();
    file.close();
    return true;
}

// __ftol uses a signed 64-bit scratch, then callers consume EAX/AX. Preserve
// low bits for representable values. Reject nonfinite/out-of-int64 natively;
// original invalid FISTP/control-word behavior is not a C++ conversion policy.
inline std::uint32_t settings_truncated_word(long double value) {
    value = std::trunc(value);
    if (!std::isfinite(value) || value < -0x1p63L || value >= 0x1p63L)
        throw std::domain_error("Invalid original settings integer conversion");
    return static_cast<std::uint32_t>(static_cast<std::int64_t>(value));
}

using SettingsGammaRamp = std::array<std::uint16_t, 768>;
inline SettingsGammaRamp profile_gamma_ramp(const ProfileSettings& settings) {
    const auto increment = static_cast<std::uint16_t>(settings_truncated_word(
        (static_cast<long double>(settings.real(4)) - 50.0L) * 2.0L + 255.0L));
    SettingsGammaRamp ramp{};
    std::uint32_t value = 0;
    for (std::size_t i = 0; i < 256; ++i) {
        ramp[i] = ramp[256 + i] = ramp[512 + i] = static_cast<std::uint16_t>(value);
        value = std::clamp<std::uint32_t>(value + increment, 255, 65535);
    }
    return ramp;
}

// 006B3340/3210/32A0/32D0/3320/32F0 in original order. Backend performs real
// graphics/sound/interface calls; it must not substitute successful empty bodies.
// Long double models x87 intermediates; volume arguments explicitly round to f32.
template<class Backend, class Sound, class Control>
void apply_profile_settings(const ProfileSettings& settings, Backend& backend, Sound& sound,
                             Control& control, input::BindingInputState& input, std::uint8_t& flag) {
    static_assert(std::numeric_limits<long double>::digits >= 64, "Settings require x87 precision");
    if (backend.driver()) {
        auto* driver = backend.driver(); // Deliberate second getter.
        backend.driver_state(driver, 0x2b, settings.quality_byte(0));
    }
    backend.gamma(profile_gamma_ramp(settings), 0);
    sound.driver_volume(static_cast<float>(static_cast<long double>(settings.real(8)) * 0.01f));
    sound.movie_volume(static_cast<float>(static_cast<long double>(settings.real(12)) * 0.01f));
    control.set_value(settings_truncated_word(static_cast<long double>(settings.real(16)) * 0.01));
    const auto bits = settings_truncated_word(static_cast<long double>(settings.real(0)) * -9.0f + 1000.0f);
    std::int32_t limit;
    std::memcpy(&limit, &bits, 4);
    input::set_binding_axis_limits(input, limit, limit, 0x78);
    flag = settings.bytes[0x39];
}

template<class Files, class Services, class Backend, class Sound, class Control>
bool load_and_apply_profile_options(ProfileOptions& options, const ProfileDirectories& profiles,
                                   Files& files, bool wide, input::BindingInputState& input,
                                   Services& services, Backend& backend, Sound& sound,
                                   Control& control, std::uint8_t& flag) {
    const auto loaded = load_profile_options(options, profiles, files, wide, input, services);
    apply_profile_settings(options.settings, backend, sound, control, input, flag);
    return loaded;
}

}
#endif
