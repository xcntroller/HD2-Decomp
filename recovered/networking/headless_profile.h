#ifndef HD2_NETWORKING_HEADLESS_PROFILE_H
#define HD2_NETWORKING_HEADLESS_PROFILE_H

#include "frontend/profile_load.h"
#include "frontend/profile_settings.h"
#include "frontend/startup_configuration.h"

namespace hd2::networking {

// Owned values, not the original 0x110-byte object ABI. The headless binding
// defaults006EC550 only clear the vector; application006EBF40 is an actual RET.
// Do not install the client's 92 defaults or require an invented input device.
struct HeadlessProfileOptions {
    frontend::ProfileSettings settings;
    std::vector<input::BindingSetting> bindings;
    HeadlessProfileOptions() { frontend::initialize_profile_settings(settings); }
};

inline void reset_headless_profile_options(HeadlessProfileOptions& options) {
    frontend::ProfileSettings defaults{}; // Original +14 scratch is unwritten.
    frontend::initialize_profile_settings(defaults); // Server006F5660.
    options.settings = defaults;
    options.bindings.clear(); //006F53F0 ->006EC550 ->006EBF70; retain capacity.
}

//006EC1B0. Clear before checking the version, append raw records in file order,
// force dirty=1, and leave it set: the server's apply helper does nothing.
// Reject incomplete/no-progress records instead of the original endless loop.
template<class Files>
bool read_headless_profile_bindings(HeadlessProfileOptions& options, Files& files,
                                    io::ChunkFile<Files>& file) {
    options.bindings.clear();
    std::uint16_t tag = 0;
    if (!file.enter(tag) || tag != 0x22b8) { file.leave(); return false; }
    std::array<std::uint8_t, 4> version{};
    const auto full = file.read(version.data(), 4);
    file.leave();
    if (!full || version != std::array<std::uint8_t, 4>{{0x42,0,0,0}}) return false;
    if (!file.enter(tag) || tag != 0x22b9) { file.leave(); return false; }
    std::array<std::uint8_t, 0x1c> scratch{};
    while (frontend::settings_chunk_remaining(files, file) != 0) {
        if (frontend::settings_chunk_remaining(files, file) < 0x1c)
            throw std::runtime_error("Incomplete headless profile binding record");
        const auto before = files.seek(file.state().handle, 0, 1);
        file.read(scratch.data(), static_cast<std::uint32_t>(scratch.size()));
        if (files.seek(file.state().handle, 0, 1) == before)
            throw std::runtime_error("Headless profile binding read made no progress");
        const auto word = [&](std::size_t at) {
            return std::uint32_t{scratch[at]} | (std::uint32_t{scratch[at+1]} << 8) |
                   (std::uint32_t{scratch[at+2]} << 16) | (std::uint32_t{scratch[at+3]} << 24);
        };
        options.bindings.push_back({frontend::profile_signed_word(word(0)),
            static_cast<std::uint16_t>(scratch[4] | (std::uint16_t{scratch[5]} << 8)),
            static_cast<std::uint16_t>(scratch[6] | (std::uint16_t{scratch[7]} << 8)),
            word(8), word(12), word(16), word(20), 1, {{scratch[25],scratch[26],scratch[27]}}});
    }
    file.leave();
    return true;
}

//006F4EA0: sequential top-level version/bindings/settings chunks. A missing
// outer child resets BOTH values; a failed inner binding reader only clears
// bindings. Ignore the inner boolean and preserve partial settings transfers.
template<class Files>
bool load_headless_profile_options(HeadlessProfileOptions& options,
                                   const frontend::ProfileDirectories& profiles,
                                   Files& files, bool wide) {
    io::ChunkFile<Files> file(files);
    std::uint16_t tag = 0;
    const auto path = profiles.current_directory(wide) + "Options.bin";
    if (!file.open(path.c_str()) || !file.enter(tag) || tag != 0x25a0) {
        reset_headless_profile_options(options); file.close(); return false;
    }
    std::array<std::uint8_t, 4> version{};
    const auto full = file.read(version.data(), 4);
    file.leave();
    if (!full || version != std::array<std::uint8_t, 4>{{0x41,1,0,0}}) {
        reset_headless_profile_options(options); file.close(); return false;
    }
    if (file.enter(tag) && tag == 0x25a6) read_headless_profile_bindings(options, files, file);
    else reset_headless_profile_options(options);
    file.leave();
    if (file.enter(tag) && tag == 0x25a5) frontend::read_profile_settings(options.settings, file);
    else reset_headless_profile_options(options);
    file.leave();
    file.close();
    return true;
}

//006F4E60 always applies after loading, even on false.006F5540 gets the actual
// driver twice and writes state2B;006F5510 scales settings+10 by double0.01,
// truncates using x87 and forwards EAX to network007888D0. Backend must supply
// genuine engine/network operations.006F5460 and006F54F0 are verified RETs.
template<class Files, class Backend>
bool load_and_apply_headless_profile_options(HeadlessProfileOptions& options,
        const frontend::ProfileDirectories& profiles, Files& files, bool wide, Backend& backend) {
    static_assert(std::numeric_limits<long double>::digits >= 64, "Settings require x87 precision");
    const auto loaded = load_headless_profile_options(options, profiles, files, wide);
    if (backend.driver()) {
        auto* driver = backend.driver();
        backend.driver_state(driver, 0x2b, options.settings.quality_byte(0));
    }
    backend.set_network_voice_input_volume(frontend::settings_truncated_word(
        static_cast<long double>(options.settings.real(0x10)) * 0.01));
    return loaded;
}

struct HeadlessProfile {
    frontend::ProfileDirectories directories;
    HeadlessProfileOptions options;
    frontend::ProfilePayload payload;
};

struct HeadlessProfileSelection { std::array<std::uint32_t, 256> words{}; };

//006F0F30 and missing-file fallback006F0700. The fallback changes to the raw
// current name under PlayersProfiles (not the wide-mode physical-name map),
// ignores chdir failure, reads relative profile.pr, then restores executable
// cwd. The shared reader only parses fields; it does not choose this cwd.
// Finalizer006F1A10 matches the shared saved-game checks and promotion helpers.
template<class Files, class Platform>
bool load_headless_profile(frontend::ProfilePayload& payload,
        frontend::ProfileDirectories& profiles, Files& files, Platform& platform,
        const frontend::GameData* game, const frontend::LocalizedTextCache& text, bool wide) {
    const auto finish = [&] { frontend::finalize_profile(payload, game, profiles, files, platform, text, wide); };
    const auto result = frontend::read_profile_payload(payload, profiles, files, wide, finish);
    if (result == frontend::ProfilePayloadResult::invalid_root) return false;
    if (result == frontend::ProfilePayloadResult::payload_read) return true;
    payload.reset();
    platform.change_to_directory("PlayersProfiles\\" + std::string(profiles.current_name.c_str()));
    const auto loaded = frontend::read_legacy_profile_fields(payload, files);
    platform.restore_executable_directory();
    finish();
    return loaded;
}

//006F0380. Keep candidate before clearing current name, try the actual
// directory, restore executable cwd, clear all256 shared words backwards,
// load/apply options then load payload, ignoring the two booleans. Exceptions
// and required backend failures are not successful selection. Retain caller
// spelling across mutations as a native alias/ownership guard.
template<class Files, class Platform, class Backend>
bool select_headless_profile(HeadlessProfile& profile, const char* requested,
        HeadlessProfileSelection& selection, Files& files, Platform& platform,
        bool wide, Backend& backend, const frontend::GameData* game,
        const frontend::LocalizedTextCache& text) {
    const std::string retained(requested ? requested : "");
    const auto candidate = profile.directories.resolve(retained, wide);
    profile.directories.current_name.clear();
    if (!candidate.empty() && platform.change_to_directory("PlayersProfiles\\" + candidate))
        profile.directories.current_name = retained;
    platform.restore_executable_directory();
    for (auto count = selection.words.size(); count != 0; --count) selection.words[count - 1] = 0;
    load_and_apply_headless_profile_options(profile.options, profile.directories, files, wide, backend);
    load_headless_profile(profile.payload, profile.directories, files, platform, game, text, wide);
    return true;
}

//006F95C0: wide flag gates scanning; initial name is read afterwards from the
// distinct startup configuration owner. Startup selection does not save it.
template<class Files, class Platform, class Backend>
void prepare_headless_startup_profile(const frontend::StartupConfiguration& startup,
        HeadlessProfile& profile, HeadlessProfileSelection& selection, Files& files,
        Platform& platform, bool wide, Backend& backend, const frontend::GameData* game,
        const frontend::LocalizedTextCache& text) {
    if (wide) profile.directories.scan(platform, files, wide);
    select_headless_profile(profile, startup.current_profile().c_str(), selection,
                            files, platform, wide, backend, game, text);
}

}
#endif
