#include "networking/server_commands.h"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace hd2::networking {
namespace {
bool space(char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f'; }
bool equal(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        const auto lower = [](char c) { return c >= 'A' && c <= 'Z' ? c + ('a'-'A') : c; };
        if (lower(a[i]) != lower(b[i])) return false;
    }
    return true;
}
std::int32_t milliseconds(std::int32_t value) {
    // Preserve x86 storage wrap without signed multiplication UB.
    const auto bits = static_cast<std::uint32_t>(value) * std::uint32_t{1000};
    std::int32_t result;
    std::memcpy(&result,&bits,sizeof(result)); return result;
}
std::string windows_text(std::string_view bytes) {
    std::string result;
    for (std::size_t i = 0; i < bytes.size() && bytes[i] != '\x1a'; ++i) {
        if (bytes[i] == '\r' && i + 1 < bytes.size() && bytes[i+1] == '\n') continue;
        result += bytes[i];
    }
    return result;
}
constexpr const char* style_names[]{"deathmatch","occupation","objectives","cooperative"};
constexpr const char* codec_names[]{"none","vr12","sc03","sc06","truespeech","gsm","adpcm","pcm"};
}

std::optional<std::string> LocalCommandFiles::read_bytes(const std::string& filename) {
    if (filename.find('\0') != std::string::npos) return std::nullopt;
    std::ifstream input(filename,std::ios::binary);
    if (!input) return std::nullopt;
    std::string result((std::istreambuf_iterator<char>(input)),{});
    if (input.bad()) return std::nullopt;
    return result;
}

ColdServerCommands::ColdServerCommands(ServerStartupSettings initial, ProfileRules profile_rules,
        std::vector<frontend::MultiplayerMap> catalogue, bool loaded, CommandFiles& files)
    : settings_(std::move(initial)),profile_rules_(std::move(profile_rules)),
      catalogue_(std::move(catalogue)),catalogue_loaded_(loaded),files_(files) {
    if (settings_.style < 0 || settings_.style > 3 || settings_.domain < 0 || settings_.domain > 1 ||
        settings_.voice < 0 || settings_.voice > 7)
        throw std::invalid_argument("Invalid supplied server bootstrap settings");
}

void ColdServerCommands::diagnostic(CommandDiagnostic::Kind kind, std::string subject) {
    diagnostics_.push_back({kind,std::move(subject)});
}
void ColdServerCommands::query(std::string key, std::string value) {
    queries_.push_back({std::move(key),std::move(value)});
}
bool ColdServerCommands::append_map(const std::string& name) {
    if (!catalogue_loaded_) return false;
    //006ED2E0 reverse search and copy: repeated names are legal rotation entries.
    for (auto it = catalogue_.rbegin(); it != catalogue_.rend(); ++it)
        if (it->style == static_cast<unsigned>(settings_.style) && equal(it->name,name)) {
            settings_.rotation.push_back(*it); return true;
        }
    return false;
}

bool ColdServerCommands::execute(std::string command) {
    try {
        CommandTokens tokens(std::move(command));
        return dispatch(tokens);
    } catch (const std::logic_error&) {
        diagnostic(CommandDiagnostic::Kind::malformed,"command");
    } catch (const std::runtime_error&) {
        diagnostic(CommandDiagnostic::Kind::malformed,"command");
    }
    return false;
}

bool ColdServerCommands::dispatch(CommandTokens& command) {
    using Kind = CommandToken::Kind;
    using Error = CommandDiagnostic::Kind;
    const auto& words = server_keywords();
    while (true) {
        const auto key = command.next(words);
        if (key.kind != Kind::keyword) { diagnostic(Error::unknown,key.text); return false; }
        const auto id = key.value;
        if ((id >= 0 && id <= 13) || (id >= 15 && id <= 21)) {
            const auto value = command.next();
            if (value.kind != Kind::integer) query(std::string(words[id].text),std::to_string(settings_.rules[id]));
            else {
                auto number = value.value;
                switch (id) {
                case 0: number = std::min(number,settings_.style == 3 ? 6 : 32); break;
                case 2: number = std::clamp(number,1,99); break;
                case 4: number = std::clamp(number,1,5940); break;
                case 5: number = std::clamp(number,3,5940); break;
                case 10: number = std::min(number,15); break;
                case 17: if (settings_.style == 3) number = std::clamp(number,1,4); break;
                case 18: number = std::min(number,99); break;
                default: break;
                }
                settings_.rules[id] = number;
                effects_.push_back({ColdCommandEffect::Kind::rules_changed,{}});
            }
        } else switch (id) {
        case 22: {
            const auto map = command.next();
            if (map.kind == Kind::text) {
                if (!append_map(map.text)) diagnostic(Error::missing_map,map.text);
            } else for (const auto& row : settings_.rotation) query("mapname",row.name);
            break;
        }
        case 24: {
            const auto style = command.next(words);
            if (style.kind != Kind::keyword) query("style",style_names[settings_.style]);
            else if (style.value < 25 || style.value > 27) diagnostic(Error::invalid_value,"style");
            else {
                settings_.style = style.value - 25;
                settings_.rules = profile_rules_[settings_.style]; //006ED040/006434A0.
            }
            break;
        }
        case 29: {
            const auto domain = command.next(words);
            if (domain.kind != Kind::keyword) query("domain",settings_.domain ? "internet" : "local");
            else if (domain.value != 30 && domain.value != 31) diagnostic(Error::invalid_value,"domain");
            else settings_.domain = domain.value - 30;
            break;
        }
        case 32: case 45: case 49: case 50: {
            const auto text = command.next();
            if (id == 50) {
                if (text.kind == Kind::text) {
                    // Original allocates128 bytes and strncpy truncates without
                    // guaranteeing NUL. Own a terminated bounded payload instead.
                    effects_.push_back({ColdCommandEffect::Kind::administrator_login,text.text.substr(0,128)});
                }
            } else {
                auto& target = id == 32 ? settings_.session_name :
                    id == 45 ? settings_.password : settings_.administrator_password;
                if (text.kind == Kind::text) {
                    target = text.text;
                    if (id == 45) settings_.password_assigned = true;
                } else if (id != 49) query(std::string(words[id].text),target);
            }
            break;
        }
        case 33:
            for (const auto& word : words) query("help",std::string(word.text));
            break;
        case 34:
            command.skip_separators();
            (void)execute_file(std::string(command.remaining()));
            return true; // Entire tail was filename; no subsequent inline command.
        case 35:
            if (settings_.rotation.empty()) { diagnostic(Error::no_maps,"server"); return true; }
            settings_.connection_mode = 2;
            effects_.push_back({ColdCommandEffect::Kind::host_requested,{}});
            break;
        case 36:
            (void)command.next(); settings_.dedicated = true; break;
        case 44: {
            const auto port = command.next();
            if (port.kind != Kind::integer || port.value > 65535)
                query("port",std::to_string(settings_.stored_port));
            else {
                settings_.stored_port = settings_.host_port = port.value;
                settings_.domain = 1;
            }
            break;
        }
        case 46:
            effects_.push_back({ColdCommandEffect::Kind::quit_requested,{}}); break;
        case 47: {
            const auto codec = command.next(words);
            if (codec.kind != Kind::keyword) query("voicechat",codec_names[settings_.voice]);
            else if (codec.value < 57 || codec.value > 64) diagnostic(Error::invalid_value,"voicechat");
            else {
                settings_.voice = codec.value == 64 ? 0 : codec.value - 56;
                settings_.voice_enabled = settings_.voice != 0;
            }
            break;
        }
        case 48:
            for (unsigned style = 0; style < 3; ++style)
                for (auto it = catalogue_.rbegin(); it != catalogue_.rend(); ++it)
                    if (it->style == style) query(style_names[style],it->name);
            break;
        case 52: case 53: case 55: {
            const auto value = command.next();
            auto& target = id == 52 ? settings_.maximum_ping :
                id == 53 ? settings_.maximum_frequency : settings_.inactivity_ms;
            if (value.kind != Kind::integer)
                query(std::string(words[id].text),std::to_string(id == 55 ? target / 1000 : target));
            else if (id == 55) target = milliseconds(value.value);
            else if (id != 53 || value.value != 0) target = value.value;
            break;
        }
        default:
            diagnostic(Error::unsupported,key.text); return false;
        }
        command.skip_separators();
        if (command.remaining().empty()) return true;
    }
}

bool ColdServerCommands::execute_file(std::string filename) {
    using Error = CommandDiagnostic::Kind;
    if (filename.find('\0') != std::string::npos) { diagnostic(Error::malformed,filename); return false; }
    if (active_files_.size() == 32 || std::any_of(active_files_.begin(),active_files_.end(),
            [&](const auto& active) { return equal(active,filename); })) {
        diagnostic(Error::recursion_limit,filename); return false;
    }
    const auto bytes = files_.read_bytes(filename);
    if (!bytes) { diagnostic(Error::missing_file,filename); return false; }
    active_files_.push_back(filename);
    struct Pop { std::vector<std::string>& stack; ~Pop() { stack.pop_back(); } } pop{active_files_};
    const auto text = windows_text(*bytes);
    std::size_t position = 0;
    //007A4080: scanset conversion followed by fscanf("\n"). The EOF test
    //precedes dispatch, so no-newline tails and an initial LF are not commands.
    while (position < text.size()) {
        const auto end = text.find('\n',position);
        if (end == position) break;
        const auto count = (end == std::string::npos ? text.size() : end) - position;
        if (count > 99) { diagnostic(Error::malformed,filename); return false; }
        if (end == std::string::npos) break;
        const auto line = text.substr(position,end - position);
        position = end;
        while (position < text.size() && space(text[position])) ++position;
        (void)execute(line); // Original ignores a per-line false dispatch result.
    }
    return true;
}
} // namespace hd2::networking
