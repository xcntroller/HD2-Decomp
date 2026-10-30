# Startup integration executable

2026-10-30. The first launch checklist item now has a concrete i386 integration
target: recovered/build/startup_probe.exe. It composes recovered startup bodies
with real LS3DF/rw_data/IJoy/Win32 services. It has been compiled/linked and
native-audited. The user desktop-confirmed this bounded startup checkpoint on
2026-10-30; the first delivery checkbox is complete within this scope.

This target stops after application data/profile/settings/input preparation. It
does not replace the existing preview or claim the original controller/menu
launch. That transition belongs to the next delivery item.

## Build and desktop check

Build from the repository root:

```sh
make -C recovered startup-probe WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++
```

Run from an actual desktop terminal:

```sh
python3 -B tools/run_startup_probe.py --frames 180
```

The launcher creates analysis/work/startup-runtime/run and a separate Proton
prefix. It copies base archives and relevant loose text/data into that directory;
first staging needs enough free space for those copies. game.cfg and profiles
are copied once and subsequent staged edits survive. Original files are never
opened for writing by this staging tool. The executable is copied beside the
data so the recovered executable-directory resolver points inside the workspace.
Linked destinations, overlapping game/work directories, altered immutable
resources and unexpected dependency hashes are rejected. `--stage-only` prepares
and prints the command without executing Windows code.

Expected successful log sequence in
analysis/work/startup-runtime/run/host-runtime.log:

1. Real graphics/sound/I3D initialization.
2. `Startup: gate=7 ... prepare=1` with loaded text/game/vertex counts.
3. `Startup: profile/options applied; bindings=...`.
4. `Startup integration completed` and a window titled
   `HD2 startup integration completed - close to finish`.
5. `Startup probe teardown completed; result=0` after the bounded pump or close.

The display is a startup diagnostic window, not the menu. Escape closes the
bounded check. Report a missing marker or the first failure line; do not interpret
a successful build or a window alone as successful data/settings integration.
No original executable, DLL or Proton was run by the agent.

## Concrete composition and order

[StartupSession](../recovered/host/startup_session.h) constructs the existing
ApplicationStartupData before graphics initialization, preserving early game.cfg
loading. [Control initialization](control-startup.md) supplies the real shared
gate7 and lock setup. Windowed device bootstrap is the same bounded diagnostic
policy used by the confirmed host; it does not reconstruct every LS3D_setup
registry/device option or the original setup.exe fallback.

Once real devices exist, it consumes the three archive random words, resolves
installed language, performs the actual GetFocus call, registers/configures
archives, installs eight directory trees and performs the actual IsZoomed call
on the real graphics window. Keyboard/manager attachment and localized-text
preload use their existing recovered bodies and real adapters. The joystick list
starts in its proven normal empty state; no unreachable enumeration is invented.

Original006DBFB0 then calls0048DA60: ascending module startup. The reconstructed
normal registration set has18 entries, including the real SoundManagerData.
Sixteen final startup slot04 targets are verified empty RET bodies. Items3 has
an explicitly zero-initialized deque in its constructor; its cold startup sets
current=-1. Nonempty restart queues are rejected, not replaced with success.
Vertex anim9 actually loads Tables\\Vertanim.def with gate2 set. Unknown or
replacement module registrations also reject instead of guessing a handler.

Application preparation calls actual GameData and HumanStore loaders even when
the first fails. Only a true combined result permits original profile selection,
Options.bin/default binding installation and settings application. SettingsSound
observes the same initialized gate; SettingsControl observes the real cleared
wrapper0C. This absent-wrapper path is not a successful interface replacement.
The probe performs one input-production check after preparation.

Consumer cleanup detaches input, closes owned joystick references, explicitly
closes the four application data owners in original relative order, and frees
vertex data while devices still exist. Device owners then close3D/sound/graphics;
archives close afterward. Load exceptions take the same scoped cleanup path.
Native lock RAII is separate from the still-unrecovered full control shutdown.

## Resolving the opaque import boundary

The original pointer00A59000 contains009967E0. Its continuation resolver0099690A
searches the return-address table at00A59008 and returns an IAT-slot address;
009977CF..009977DC restores the call stack and tail-jumps through that slot.
Fingerprint-checked mappings include:

| Return address | IAT slot | Actual imported API |
| --- | --- | --- |
| 00688087 | 0080F3E4 | IsWindow |
| 006880DD | 0080F3E8 | GetFocus |
| 006DB8CD | 0080F3E8 | GetFocus |
| 006DBF31 | 0080F3D0 | IsZoomed |

Other table entries resolve GetTickCount, IsWindowVisible and IsIconic. The
native startup composition calls the required underlying APIs directly. It does
not install a function called protection() that returns success. Original binary
self-integrity machinery, per-thread VM state and associated private tables are
NOT ported or claimed equivalent. Those original-address checks cannot validate
native C++ object layouts. The full protection-bearing006DBFC0 wrapper and video
state remain distinct from this bounded integration target; preserve that scope
when assessing the first checkbox and when wiring the next controller milestone.

## Required vertex-animation resource

[vertex_animations.h](../recovered/frontend/vertex_animations.h) reconstructs
00473EC0 plus record ownership00473700/00473720. Missing open preserves old
entries and still closes invalid handleFFFFFFFF. Valid open clears old entries
in reverse, retaining capacity; it reads/ignores a header DWORD. Each entry has
20 raw name bytes, last-frame/group counts, group sizes and inclusive0..last-frame
rows. Each group contains size*32 bytes in16-byte-aligned storage. Only complete
entries append; incomplete current data is freed and the file closes. The signed
last-frame-1 case has no frames; zero-byte block reads terminate the current entry.
Native guards reject short positive reads and unsafe dimensions instead of
consuming original undefined bytes. Native allocation/vector/SEH ABI differs.

Independent read-only extraction/decoding of base others.DTA gives five records,
1,591,588 file bytes and1,591,424 vertex bytes. Asset SHA256:
`15f2a3ba1a3c19452c88a47f7ddbd9d795102570e7f8e044a86475be5c405bfe`.
No proprietary asset bytes are committed. Runtime archive overlays remain real
rw_data behavior and may select a newer member than this base-asset audit.

## Validation and scope

Temporary native ASan/UBSan audit matches all five real tables, dimensions and
aligned block totals. Missing/truncated/zero-block/signed-last-frame/bad-dimension,
gated initialization, original empty startup methods, Items cold/restart and
replacement-registration cases pass. Temporary staging audit verifies copied
resources, untouched originals, retained staged config/profile edits and
hash/link/overlap/resource mismatch rejection. Fixtures are removed; no permanent
tests added. Strict i386 real-adapter executable build, existing native checks
and all158 Python tests pass.

The actual isolated runtime resources/executable have also been staged, and the
prepared launch arguments/environment validated without executing Windows code.
The staged executable hash matches the current compiled target.

[Evidence map](startup-integration-map.json): both hashes,47 instruction/data
anchors,36 direct calls,18 client startup slots and10 engine slots. Four altered
maps reject. Function credit is limited to four census-backed partial client
bodies00473EC0/00473700/00473720/0048DA60:284 partial (279 client/five engine),
zero complete, frozen18,783. Integration glue, confirmed empty methods,
absent-census00474A00 and delegated implementations get no extra credit.

The user confirmed both completion markers on 2026-10-30. The actual log records
gate7, 2,590 text entries, five vertex tables, prepare=1, profile/options applied,
and teardown result=0. This run had zero bindings/joysticks/input words; it verifies
startup composition, not live controller input. The first checkbox is complete;
full application-state/controller wiring, original registry bootstrap, general
module lifecycle/cache consumers and video/subtitle/music integration remain
explicit follow-up work. This is not a complete original launch executable.
