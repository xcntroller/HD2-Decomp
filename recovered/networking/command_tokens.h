#ifndef HD2_NETWORKING_COMMAND_TOKENS_H
#define HD2_NETWORKING_COMMAND_TOKENS_H

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace hd2::networking {

struct Keyword { std::string_view text; std::int32_t value; };
struct CommandToken {
    enum class Kind { text = 0, integer = 1, keyword = 2, end = 3 };
    Kind kind = Kind::end;
    std::string text;
    std::int32_t value = 0;
};

// HD2DS 007A47B0. Own the command; no pointer into caller/packet storage survives.
// Original buffers are 512 bytes. Reject overflow and unterminated quoted input.
class CommandTokens {
public:
    explicit CommandTokens(std::string command);
    CommandToken next(const std::vector<Keyword>& dictionary = {});
    void skip_separators(); // 007A4990: unlike next(), this also skips numeric '-'.
    std::string_view remaining() const { return std::string_view(command_).substr(position_); }
private:
    std::string command_;
    std::size_t position_ = 0;
};

const std::vector<Keyword>& server_keywords(); // 00886F78, including reserved slot14.

// 007065A0's -cmd branch. Full raw Windows command line, not CRT argv.
// nullopt means this branch was not selected; alternate startup remains separate.
std::optional<std::string> pending_command(std::string_view raw_command_line);

} // namespace hd2::networking
#endif
