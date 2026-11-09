#ifndef HD2_NETWORKING_SERVER_COMMANDS_H
#define HD2_NETWORKING_SERVER_COMMANDS_H

#include "frontend/multiplayer_maps.h"
#include "networking/command_tokens.h"
#include <array>
#include <optional>

namespace hd2::networking {

// Owned cold-start fields from the actual configuration/limit owners. Zeroed
// storage is not a reconstruction of server defaults. Bootstrap must supply them.
struct ServerStartupSettings {
    std::string session_name, password, administrator_password;
    std::int32_t connection_mode = 0, style = 0, domain = 0, stored_port = 0, host_port = 0, voice = 0;
    std::int32_t maximum_ping = 0, maximum_frequency = 0, inactivity_ms = 0;
    bool dedicated = false, password_assigned = false, voice_enabled = false;
    std::array<std::int32_t,22> rules{};
    // Recovered catalogue metadata only; not the original full game descriptor.
    std::vector<frontend::MultiplayerMap> rotation;
};

class CommandFiles {
public:
    virtual ~CommandFiles() = default;
    virtual std::optional<std::string> read_bytes(const std::string& filename) = 0;
};
class LocalCommandFiles final : public CommandFiles {
public:
    std::optional<std::string> read_bytes(const std::string& filename) override;
};

// Requests stay owned until the actual controller/network owner consumes them.
// No effect here acknowledges a running host, delivered packet or successful login.
struct ColdCommandEffect {
    enum class Kind { rules_changed, host_requested, quit_requested, administrator_login };
    Kind kind;
    std::string text;
};
struct CommandDiagnostic {
    enum class Kind { unknown, unsupported, invalid_value, missing_map, no_maps,
                      missing_file, malformed, recursion_limit };
    Kind kind;
    std::string subject;
};
struct CommandQuery { std::string keyword, value; };

//007A23E0's full launcher/settings boundary in idle mode7; no active session.
// Actual bootstrap settings, selected-profile rules and loaded catalogue are
// required inputs. In particular, constructing this object cannot create a host.
class ColdServerCommands {
public:
    using ProfileRules = std::array<std::array<std::int32_t,22>,4>;
    ColdServerCommands(ServerStartupSettings initial, ProfileRules profile_rules,
        std::vector<frontend::MultiplayerMap> catalogue, bool catalogue_loaded,
        CommandFiles& files);

    // Returns original syntactic command acceptance, not transport success.
    bool execute(std::string command);
    // Returns whether the actual file was opened and safely traversed. Missing
    // files/invalid lines are diagnosed; native unsafe line lengths are rejected.
    bool execute_file(std::string filename);
    const ServerStartupSettings& settings() const { return settings_; }
    const std::vector<ColdCommandEffect>& effects() const { return effects_; }
    const std::vector<CommandDiagnostic>& diagnostics() const { return diagnostics_; }
    const std::vector<CommandQuery>& queries() const { return queries_; }
    void clear_output() { effects_.clear(); diagnostics_.clear(); queries_.clear(); }
private:
    ServerStartupSettings settings_;
    ProfileRules profile_rules_;
    std::vector<frontend::MultiplayerMap> catalogue_;
    bool catalogue_loaded_;
    CommandFiles& files_;
    std::vector<std::string> active_files_;
    std::vector<ColdCommandEffect> effects_;
    std::vector<CommandDiagnostic> diagnostics_;
    std::vector<CommandQuery> queries_;
    bool dispatch(CommandTokens& command);
    bool append_map(const std::string& name);
    void diagnostic(CommandDiagnostic::Kind kind, std::string subject);
    void query(std::string key, std::string value);
};

} // namespace hd2::networking
#endif
