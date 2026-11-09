# HD2 networking reconstruction

This workstream reconstructs the original **HD2DS.exe** dedicated server and its
ServerLauncher.exe companion on `feature/networking`. It also covers the client's
discovery and connection infrastructure. Gameplay replication, synchronized
physics and mission simulation are outside its current scope.

The implementation is currently **source only**. No independently running server,
live discovery or genuine client/server session has been verified. Successful
configuration parsing does not establish a working network session.

## Function progress

<!-- networking-progress:start -->

| Executable | Inventoried functions | Complete | Partial | No completion claim | Complete % |
| --- | ---: | ---: | ---: | ---: | ---: |
| HD2DS.exe | 13,957 | 0 | 11 | 13,946 | 0.00% |
| ServerLauncher.exe | 849 | 0 | 9 | 840 | 0.00% |

### HD2DS.exe mapped bodies

| Original address | Status | Implemented contract |
| --- | --- | --- |
| `006ED040` | partial | [Style change copies22 selected profile rules and retains rotation](../recovered/networking/server_commands.cpp) |
| `006ED2E0` | partial | [Loaded catalogue/style gate, reverse case-insensitive lookup and owned rotation append](../recovered/networking/server_commands.cpp) |
| `006EDCF0` | partial | [Fresh command-facing server configuration, dedicated flag, admin initializer and active rules](../recovered/networking/server_initial_settings.h) |
| `007065A0` | partial | [Literal -cmd branch and owned pending command](../recovered/networking/command_tokens.cpp) |
| `00779450` | partial | [Native TCP/IP DirectPlay server acquisition, caps, port and Host submission](../recovered/networking/directplay_server.cpp) |
| `00787EA0` | partial | [Fresh command-facing ping, frequency and inactivity limits](../recovered/networking/server_initial_settings.h) |
| `007882B0` | partial | [Cancel, Close and Release server COM endpoint before callback state retirement](../recovered/networking/directplay_server.cpp) |
| `007A23E0` | partial | [All launcher settings, idle-mode rules and distinct owned controller/admin requests](../recovered/networking/server_commands.cpp) |
| `007A4080` | partial | [Read-only command files with original scanset/whitespace/EOF order and nested exec](../recovered/networking/server_commands.cpp) |
| `007A47B0` | partial | [Bounded console tokenization and original keyword/numeric classifications](../recovered/networking/command_tokens.cpp) |
| `007A4990` | partial | [Separate space/tab/greater-than/hyphen separator advancement](../recovered/networking/command_tokens.cpp) |

### ServerLauncher.exe mapped bodies

| Original address | Status | Implemented contract |
| --- | --- | --- |
| `004075C0` | partial | [Selected configuration path construction](../recovered/networking/launcher_config.cpp) |
| `00407C90` | partial | [Original/expansion executable path selection](../recovered/networking/launcher_config.cpp) |
| `00407D50` | partial | [Exact CreateProcess application/command/current-directory values](../recovered/networking/launcher_config.cpp) |
| `00408130` | partial | [Session name/map presence and Internet port value checks](../recovered/networking/launcher_config.cpp) |
| `0040CCF0` | partial | [Loader rule/default values](../recovered/networking/launcher_config.cpp) |
| `00410EF0` | partial | [Quoted/unquoted launcher string extraction](../recovered/networking/launcher_config.cpp) |
| `00411600` | partial | [All configuration fields, ordering and fixed command tail](../recovered/networking/launcher_config.cpp) |
| `00411F50` | partial | [All33 configuration keyword setters](../recovered/networking/launcher_config.cpp) |
| `00412720` | partial | [Configuration read-reset and line traversal](../recovered/networking/launcher_config.cpp) |

<!-- networking-progress:end -->

The denominator is the frozen Ghidra inventory of internal non-thunk entries,
including unidentified library/runtime functions. Imported functions and thunks
are excluded. A partial body earns no complete-function credit. Entries without
a completion claim may already have research; this table measures implemented
bodies, not research coverage, playability or time remaining.

The server and launcher use separate inventories and a shared
[networking ledger](../analysis/networking/function-status.json). They do not
change the frozen client/LS3DF progress baseline. Original binary fingerprints,
versions and imports are in the [binary manifest](../analysis/networking/binary-manifest.json).

Refresh the generated progress block after ledger changes:

```sh
python3 tools/networking/update_progress.py
python3 tools/networking/update_progress.py --check
```

## Current work

Completed source checkpoints cover launcher configuration read/write and launch
specification, bounded server command tokens, cold settings dispatch and command
files, plus verified fresh server configuration and limit values. The native
DirectPlay server transport now contains COM acquisition,
Host submission and ordered callback cleanup. It requires the
actual caller's handler and an owned wide name. Real bootstrap/profile/controller
owners and session lifecycle integration are still required.

Follow the [networking plan](../analysis/networking/plan.md) and
[latest handoff](../analysis/networking/HANDOFF.md). The
[architecture map](../analysis/networking/architecture.md),
[launcher configuration contract](../analysis/networking/configuration-source.md)
and [cold command contract](../analysis/networking/server-settings-source.md), plus
the [startup/transport contract](../analysis/networking/startup-network-source.md),
retain original addresses, evidence boundaries and verification results.

Source lives in [recovered/networking](../recovered/networking). Strict native and
Windows i386 object builds use its Makefile. Temporary native sanitizer audits
and Windows compilation/link checks are distinct from desktop or interoperability
acceptance. Original commercial executables, DLLs, Ghidra databases and generated
decompilations remain local and untracked. No original executable or DLL is run
automatically; nothing is pushed or published without explicit instruction.
