#ifndef HD2_NETWORKING_LAUNCHER_CONFIG_H
#define HD2_NETWORKING_LAUNCHER_CONFIG_H

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace hd2::networking {

// ServerLauncher ordinals differ from the server and existing client settings.
enum class LauncherStyle { objectives, occupation, deathmatch, cooperative };
enum class LauncherDomain { internet, local };
enum class LauncherCodec { none, vr12, sc03, sc06, truespeech, gsm, adpcm, pcm };

struct LauncherConfig {
    //00412720 read-reset +0040CCF0 defaults. These are not server-owned defaults.
    std::int32_t maxclients = 32, pointlimit = 0, roundlimit = 30, roundcount = 3;
    std::int32_t warmup = 10, respawntime = 30, spawnprotection = 5, inversedamage = 100;
    LauncherStyle style = LauncherStyle::occupation;
    LauncherDomain domain = LauncherDomain::internet;
    LauncherCodec voicechat = LauncherCodec::none;
    std::int32_t port = 11001, maxping = 0, maxfreq = 0, maxinactivity = 0;
    bool fallingdmg = true, allowvehicles = true, autorestart = false, allowrespawn = true;
    bool friendlyfire = true, autoteambalance = true, thirdpersonview = false;
    bool allowcrosshair = true, teamlives = true;
    std::int32_t cooplives = -1, coopdifficulty = 3;
    std::string sessionname = "H&D 2 SERVER", password, adminlogin, adminpass;
    std::vector<std::string> maps;
};

struct LauncherConfigRead {
    LauncherConfig settings;
    // Original reader ignores individual invalid/unknown command results.
    // Preserve valid fields and report failures without a UI or uninitialized data.
    struct Issue { std::size_t line; std::string keyword; };
    std::vector<Issue> issues;
};

LauncherConfigRead read_launcher_config(std::string_view text);
std::string write_launcher_config(const LauncherConfig& settings);

struct LauncherProcessSpec {
    std::string application, command_line, current_directory;
};
enum class ServerEdition { original, sabre_squadron };
//004075C0/00407C90/00407D50. A process specification is not a started server.
LauncherProcessSpec launcher_process_spec(std::string game_directory,
    std::string_view selected_filename, ServerEdition edition = ServerEdition::original);

//00408130's value checks; actual map validation/mutex/UDP probe are separate
// required operations in the original launch path and receive no fake success.
void validate_launcher_values(const LauncherConfig& settings);

} // namespace hd2::networking
#endif
