#include "networking/command_tokens.h"
#include <limits>
#include <stdexcept>
#include <utility>

namespace hd2::networking {
namespace {
bool alpha(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }
bool digit(char c) { return c >= '0' && c <= '9'; }
std::string lower(std::string value) {
    for (auto& c : value) if (c >= 'A' && c <= 'Z') c = static_cast<char>(c + ('a' - 'A'));
    return value;
}
std::int32_t decimal_prefix(std::string_view value) {
    std::size_t at = 0;
    const bool negative = value[at] == '-';
    if (negative) ++at;
    std::uint64_t number = 0;
    const std::uint64_t limit = negative ? 2147483648ULL : 2147483647ULL;
    while (at < value.size() && digit(value[at])) {
        const auto next = static_cast<unsigned>(value[at++] - '0');
        if (number > (limit - next) / 10) throw std::runtime_error("Console integer overflow");
        number = number * 10 + next;
    }
    // Original atoi accepts '-' as zero, ignores suffixes, and parses 0x10 as0.
    return static_cast<std::int32_t>(negative ? -static_cast<std::int64_t>(number) : number);
}
}

CommandTokens::CommandTokens(std::string command) : command_(std::move(command)) {
    if (command_.find('\0') != std::string::npos) throw std::invalid_argument("Embedded console NUL");
}

CommandToken CommandTokens::next(const std::vector<Keyword>& dictionary) {
    while (position_ < command_.size()) {
        const auto c = command_[position_];
        if (c != ' ' && c != '\t' && c != '>' &&
            !(c == '-' && position_ + 1 < command_.size() && alpha(command_[position_ + 1]))) break;
        ++position_;
    }
    if (position_ == command_.size()) return {};
    CommandToken token;
    const bool quoted = command_[position_] == '"';
    if (quoted) ++position_;
    const auto start = position_;
    while (position_ < command_.size() && (quoted ? command_[position_] != '"' :
            command_[position_] != ' ' && command_[position_] != '\t')) ++position_;
    if (quoted && position_ == command_.size()) throw std::runtime_error("Unterminated console quote");
    if (position_ - start > 511) throw std::runtime_error("Console token exceeds original buffer");
    token.text = command_.substr(start, position_ - start);
    if (quoted) ++position_;
    const auto folded = lower(token.text);
    for (const auto& keyword : dictionary) if (keyword.text == folded) {
        token.kind = CommandToken::Kind::keyword; token.value = keyword.value; return token;
    }
    if (!token.text.empty() && (digit(token.text.front()) || token.text.front() == '-')) {
        token.kind = CommandToken::Kind::integer; token.value = decimal_prefix(token.text);
    } else token.kind = CommandToken::Kind::text;
    return token;
}

void CommandTokens::skip_separators() {
    while (position_ < command_.size()) {
        const auto c = command_[position_];
        if (c != ' ' && c != '\t' && c != '>' && c != '-') break;
        ++position_;
    }
}

const std::vector<Keyword>& server_keywords() {
    static const std::vector<Keyword> words{
        {"maxclients",0},{"pointlimit",1},{"roundlimit",2},{"roundcount",3},
        {"warmup",4},{"respawntime",5},{"allowrespawn",6},{"friendlyfire",7},
        {"autoteambalance",8},{"3rdpersonview",9},{"spawnprotection",10},
        {"inversedamage",11},{"fallingdmg",12},{"floodprotect",13},{"",0},
        {"allowvehicles",15},{"autorestart",16},{"coopdifficulty",17},{"cooplives",18},
        {"allowcrosshair",19},{"spawnonstart",20},{"teamlives",21},{"mapname",22},
        {"removemap",23},{"style",24},{"deathmatch",25},{"occupation",26},
        {"objectives",27},{"cooperative",28},{"domain",29},{"local",30},
        {"internet",31},{"sessionname",32},{"help",33},{"exec",34},{"server",35},
        {"dedicated",36},{"join",37},{"vote",38},{"voteyes",39},{"listplayers",40},
        {"kickplayer",41},{"nextmap",42},{"restart",43},{"port",44},{"password",45},
        {"quit",46},{"voicechat",47},{"listmaps",48},{"adminpass",49},{"adminlogin",50},
        {"svcmd",51},{"maxping",52},{"maxfreq",53},{"asay",54},{"maxinactivity",55},
        {"listobjectives",56},{"vr12",57},{"sc03",58},{"sc06",59},{"truespeech",60},
        {"gsm",61},{"adpcm",62},{"pcm",63},{"none",64}
    };
    return words;
}

std::optional<std::string> pending_command(std::string_view raw) {
    if (raw.find('\0') != std::string_view::npos) throw std::invalid_argument("Embedded command-line NUL");
    constexpr std::string_view marker = " -cmd ";
    const auto found = raw.find(marker);
    if (found == std::string_view::npos) return std::nullopt;
    const auto tail = raw.substr(found + marker.size());
    // Console owner007A43A0 stores the command at+0C, ending before+210.
    if (tail.size() >= 0x204) throw std::length_error("Pending console command exceeds original owner");
    return std::string(tail);
}
} // namespace hd2::networking
