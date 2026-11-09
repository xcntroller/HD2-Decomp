#ifndef HD2_NETWORKING_SERVER_INITIAL_SETTINGS_H
#define HD2_NETWORKING_SERVER_INITIAL_SETTINGS_H

#include "networking/server_commands.h"

namespace hd2::networking {

// Fresh cold-command data from006EDCF0/0077C630 and00787EA0. This is not
// initialization of the containing application, transport or profile owner.
// Profile rules and the genuinely loaded catalogue remain required inputs to
// ColdServerCommands. Do not substitute these active rules for per-style rules.
inline ServerStartupSettings initial_server_command_settings() {
    ServerStartupSettings settings;
    settings.dedicated = true; // Server reset differs from the client reset.
    settings.host_port = 11001;
    // Original+21C contains three control bytes, not twelve ASCII escape bytes.
    settings.administrator_password.assign("\x01\x02\x03",3);
    settings.rules = {{32,20,0,3,30,10,1,1,1,0,1,100,1,1,1,1,0,3,-1,1,0,1}};
    settings.maximum_ping = 0; // Owner+5064, not the distinct+5054 field50.
    settings.maximum_frequency = 50;
    settings.inactivity_ms = 300000;
    // Original nested descriptor's+6C (configuration+1E8) is not written by
    // its reset. Native stored_port=0 is an explicit defined guard, not an
    // original port default; the port command writes both port fields.
    return settings;
}

} // namespace hd2::networking
#endif
