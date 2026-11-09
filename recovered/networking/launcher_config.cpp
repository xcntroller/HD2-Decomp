#include "networking/launcher_config.h"
#include "networking/command_tokens.h"
#include <array>
#include <cerrno>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <utility>

namespace hd2::networking {
namespace {
constexpr std::array<std::string_view,4> styles{"objectives","occupation","deathmatch","cooperative"};
constexpr std::array<std::string_view,2> domains{"internet","local"};
constexpr std::array<std::string_view,8> codecs{"none","vr12","sc03","sc06","truespeech","gsm","adpcm","pcm"};
bool space(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\v' || c == '\f'; }
std::string_view trim_left(std::string_view s) {
    while (!s.empty() && space(s.front())) s.remove_prefix(1);
    return s;
}
std::string fold(std::string_view s) {
    std::string result(s);
    for (auto& c : result) if (c >= 'A' && c <= 'Z') c = static_cast<char>(c + ('a' - 'A'));
    return result;
}
bool integer(std::string_view s, std::int32_t& result) {
    const std::string copy(s);
    char* end = nullptr;
    errno = 0;
    const auto value = std::strtoll(copy.c_str(), &end, 0); //00411F50 sscanf %i, including numeric prefixes.
    if (end == copy.c_str() || errno == ERANGE || value < std::numeric_limits<std::int32_t>::min() ||
        value > std::numeric_limits<std::int32_t>::max()) return false;
    result = static_cast<std::int32_t>(value); return true;
}
std::string text_value(std::string_view s) {
    s = trim_left(s);
    if (!s.empty() && s.front() == '"') {
        s.remove_prefix(1);
        const auto end = s.find('"');
        if (end == std::string_view::npos) throw std::runtime_error("Unterminated launcher string");
        return std::string(s.substr(0, end));
    }
    return std::string(s.substr(0, s.find(' '))); //00410EF0: unquoted delimiter is space, not all whitespace.
}
template<class Enum, std::size_t N>
bool enumeration(std::string_view s, Enum& out, const std::array<std::string_view,N>& names) {
    s = trim_left(s);
    std::size_t end = 0;
    while (end < s.size() && !space(s[end])) ++end;
    const auto word = fold(s.substr(0,end));
    for (std::size_t i = 0; i < N; ++i) if (names[i] == word) {
        out = static_cast<Enum>(i); return true;
    }
    return false;
}
bool apply(LauncherConfig& out, const std::string& key, std::string_view value) {
    const std::pair<std::string_view,std::int32_t LauncherConfig::*> integers[]{
        {"maxclients",&LauncherConfig::maxclients},{"pointlimit",&LauncherConfig::pointlimit},
        {"roundlimit",&LauncherConfig::roundlimit},{"roundcount",&LauncherConfig::roundcount},
        {"warmup",&LauncherConfig::warmup},{"respawntime",&LauncherConfig::respawntime},
        {"spawnprotection",&LauncherConfig::spawnprotection},{"inversedamage",&LauncherConfig::inversedamage},
        {"port",&LauncherConfig::port},{"maxping",&LauncherConfig::maxping},
        {"maxfreq",&LauncherConfig::maxfreq},{"maxinactivity",&LauncherConfig::maxinactivity},
        {"coopdifficulty",&LauncherConfig::coopdifficulty},{"cooplives",&LauncherConfig::cooplives}
    };
    for (auto [name, member] : integers) if (key == name) return integer(value, out.*member);
    const std::pair<std::string_view,bool LauncherConfig::*> booleans[]{
        {"friendlyfire",&LauncherConfig::friendlyfire},{"autoteambalance",&LauncherConfig::autoteambalance},
        {"3rdpersonview",&LauncherConfig::thirdpersonview},{"fallingdmg",&LauncherConfig::fallingdmg},
        {"allowvehicles",&LauncherConfig::allowvehicles},{"allowrespawn",&LauncherConfig::allowrespawn},
        {"autorestart",&LauncherConfig::autorestart},{"allowcrosshair",&LauncherConfig::allowcrosshair},
        {"teamlives",&LauncherConfig::teamlives}
    };
    for (auto [name, member] : booleans) if (key == name) {
        std::int32_t number = 0;
        if (!integer(value, number)) return false;
        out.*member = number != 0; return true;
    }
    if (key == "style") return enumeration(value, out.style, styles);
    if (key == "domain") return enumeration(value, out.domain, domains);
    if (key == "voicechat") return enumeration(value, out.voicechat, codecs);
    const std::pair<std::string_view,std::string LauncherConfig::*> strings[]{
        {"sessionname",&LauncherConfig::sessionname},{"password",&LauncherConfig::password},
        {"adminlogin",&LauncherConfig::adminlogin},{"adminpass",&LauncherConfig::adminpass}
    };
    for (auto [name, member] : strings) if (key == name) { out.*member = text_value(value); return true; }
    if (key == "mapname") {
        auto name = text_value(value);
        if (name.empty()) return false;
        out.maps.push_back(std::move(name)); return true;
    }
    // Launcher does not execute either console command while reading a file.
    return key == "dedicated" || key == "server";
}
void safe_text(std::string_view value, bool map = false) {
    if (value.size() > 511 || value.find_first_of("\"\r\n") != std::string_view::npos ||
        value.find('\0') != std::string_view::npos)
        throw std::invalid_argument("Unrepresentable launcher command string");
    if (map) for (auto c : value) if (space(c))
        throw std::invalid_argument("Map name cannot contain console separators");
}
template<class Enum, std::size_t N>
std::string_view name(Enum value, const std::array<std::string_view,N>& names) {
    const auto index = static_cast<std::size_t>(value);
    if (index >= N) throw std::invalid_argument("Invalid launcher enumeration");
    return names[index];
}
}

LauncherConfigRead read_launcher_config(std::string_view input) {
    if (input.find('\0') != std::string_view::npos) throw std::invalid_argument("Embedded configuration NUL");
    LauncherConfigRead out;
    std::size_t number = 0;
    while (!input.empty()) {
        ++number;
        const auto newline = input.find('\n');
        auto line = input.substr(0, newline);
        if (newline == std::string_view::npos) input = {};
        else input.remove_prefix(newline + 1);
        if (!line.empty() && line.back() == '\r') line.remove_suffix(1);
        line = trim_left(line);
        if (line.empty() || line.front() == ';') continue;
        std::size_t end = 0;
        while (end < line.size() && !space(line[end])) ++end;
        const auto key = fold(line.substr(0,end));
        bool accepted = false;
        if (end <= 255) {
            try { accepted = apply(out.settings,key,trim_left(line.substr(end))); }
            catch (const std::runtime_error&) { accepted = false; }
        }
        if (!accepted) out.issues.push_back({number,key});
    }
    return out;
}

std::string write_launcher_config(const LauncherConfig& s) {
    //00411600 logical text order. The file adapter must use Windows text mode
    //for CRLF conversion, or explicitly translate LF on another host.
    for (const auto* value : {&s.sessionname,&s.password,&s.adminlogin,&s.adminpass}) safe_text(*value);
    for (const auto& map : s.maps) safe_text(map,true);
    std::string output;
    const auto line = [&](std::string_view key, std::string_view value) {
        output.append(key); output += ' '; output.append(value); output += '\n';
    };
    const auto quoted = [&](std::string_view key, const std::string& value) { line(key,'"'+value+'"'); };
    const auto numeric = [&](std::string_view key, auto value) { line(key,std::to_string(value)); };
    quoted("sessionname",s.sessionname); line("style",name(s.style,styles));
    for (const auto& map : s.maps) line("mapname",map);
    numeric("maxclients",s.maxclients); numeric("pointlimit",s.pointlimit);
    numeric("roundlimit",s.roundlimit); numeric("roundcount",s.roundcount);
    numeric("allowrespawn",s.allowrespawn); numeric("respawntime",s.respawntime);
    numeric("spawnprotection",s.spawnprotection); numeric("friendlyfire",s.friendlyfire);
    numeric("inversedamage",s.inversedamage); numeric("warmup",s.warmup);
    numeric("autoteambalance",s.autoteambalance); numeric("3rdpersonview",s.thirdpersonview);
    numeric("fallingdmg",s.fallingdmg); line("domain",name(s.domain,domains));
    if (s.domain == LauncherDomain::internet) numeric("port",static_cast<std::uint32_t>(s.port));
    numeric("allowvehicles",s.allowvehicles); numeric("maxping",s.maxping);
    numeric("maxfreq",s.maxfreq); numeric("maxinactivity",s.maxinactivity);
    line("voicechat",name(s.voicechat,codecs));
    if (!s.password.empty()) quoted("password",s.password);
    if (!s.adminlogin.empty()) quoted("adminlogin",s.adminlogin);
    if (!s.adminpass.empty()) quoted("adminpass",s.adminpass);
    numeric("autorestart",s.autorestart); numeric("coopdifficulty",s.coopdifficulty);
    numeric("cooplives",s.cooplives); numeric("allowcrosshair",s.allowcrosshair);
    numeric("teamlives",s.teamlives); numeric("dedicated",1); output += "server\n";
    return output;
}

void validate_launcher_values(const LauncherConfig& s) {
    if (s.sessionname.empty()) throw std::invalid_argument("A session name is required");
    if (s.maps.empty()) throw std::invalid_argument("A map rotation is required");
    if (s.domain == LauncherDomain::internet && (s.port < 2000 || s.port > 65531))
        throw std::invalid_argument("Internet port must be in 2000..65531");
    name(s.style,styles); name(s.domain,domains); name(s.voicechat,codecs);
}

LauncherProcessSpec launcher_process_spec(std::string directory, std::string_view filename, ServerEdition edition) {
    safe_text(directory); safe_text(filename);
    if (edition != ServerEdition::original && edition != ServerEdition::sabre_squadron)
        throw std::invalid_argument("Invalid server edition");
    LauncherProcessSpec spec;
    spec.application = directory + (edition == ServerEdition::original ? "\\HD2DS.exe" : "\\HD2DS_SabreSquadron.exe");
    spec.command_line = " -cmd -exec " + directory + "\\ServerConfigs\\" + std::string(filename);
    spec.current_directory = std::move(directory);
    // Both launcher's fixed command buffer and server's pending-command owner
    //must fit; never silently truncate or split a filename containing spaces.
    if (spec.command_line.size() >= 1024) throw std::length_error("Launcher command line exceeds original buffer");
    (void)pending_command(spec.command_line);
    return spec;
}
} // namespace hd2::networking
