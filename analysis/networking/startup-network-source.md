# Startup and DirectPlay ownership — working contract

The full N1 startup/network checkbox remains open. This research covers its
bootstrap, ownership, native DirectPlay host acquisition and cleanup dependencies;
transport acquisition alone cannot replace the required application/profile,
archive/module, world-readiness or callback receivers.

Original HD2DS fingerprint is fixed in [manifest](binary-manifest.json). New local
read-only/noanalysis Ghidra exports: `server-bootstrap-20261109`,
`server-bootstrap-deps-20261109`, `server-transport-inputs-20261109`.
All used entries have successful fingerprinted JSON/C pairs. Static assembly of
00779450 and the installed dplay8.h/dpaddr.h definitions resolves indirect calls.
Original executables, DLLs and Windows fixtures have not been executed.

## Bootstrap completion boundary

00725850 owns the manual-reset Win32 event,00706120 application owner, message/
engine/network/controller loop, archive reader and ordered teardown.00725260:
COM MTA →007251D0 executable-directory selection →TEV(name,22Fh) →00787EA0
network owner →00724450 IGraph →007245C0 title →00724630 I3D →00724660 settings
→007247A0 archives →00725120 eight resource directories →00725240 language/module
owners →007F1240 session/AI-related consumers →C runtime random seed.

The required headless engine is LS3DRF.dll, not the main client's LS3DF.dll.
Static PE export mapping locates TEV at10030A40.10030A4A compares the second
argument with22Fh,10030A51 sets AL=1 and10030A59 returns for that exact input.
It is therefore not an unresolved protection computation on this reached path.
This engine entry is delegated and receives no server/client ledger credit.

00725340 closes network00788210 and settings006F8DD0, module registry004A3AE0,
remaining consumers and caches, then I3D/IGraph,16 archives and COM.00706180
retires the application critical section and polymorphic state. Bootstrap failure
and later normal shutdown must retain original ownership order without a default
successful backend. Initial settings/profile defaults and module callbacks still
need their actual containing owners before integration can be credited.

## Native transport contract

00779450 first closes prior work007882B0, then performs additional logging/config/
autorestart/UI preparation. The native transport acquisition portion is:

1. CoInitialize(NULL); create DirectPlay8Server with CLSCTX_INPROC_SERVER.
2. Initialize flags0 with original callback thunk0040C6A8 →0077DCB0 →0077DCF0.
3. EnumServiceProviders twice, filtering the supplied provider GUID; first call
   supplies buffer size, second fills it. Original reads the first provider GUID
   without validating count/size; native storage must validate this boundary.
4. Create DirectPlay8Address with CLSCTX_ALL; SetSP with the **requested** provider,
   not the enumerated first GUID. GetSP writes the saved GUID slot; result ignored.
5. If requested provider is TCP/IP and selected port is nonzero, AddComponent
   `port`, DWORD/4 bytes. LAN uses31567; Internet uses configured+220. Original
   ignores AddComponent failure and lets Host determine its resulting HRESULT.
6. Application description:72 bytes on i386, flags1 (CLIENT_SERVER), zero instance
   GUID, original application GUID, max/current players0, supplied wide session
   name, no password/reserved data. Admission limits/password are separate callbacks.
7. SetCaps flags0: size20, connect timeout2000, retriesFFFFFFFF, keepalive4000.
   The original ignores SetCaps HRESULT. Set host mode1 dedicated/3 listen before
   synchronous Host; one device address, null security/credentials/player context,
   Host flags0. Preserve actual Host HRESULT; failure resets mode7.
8. Release temporary address; retain the real server endpoint. Optional voice,
   GameSpy, serialization/session initialization and player state follow in original
   code and remain required containing services, not successful no-ops.

007882B0 cancels all async operations (handle0/flags8000h), closes server with
flags0 and releases it before retiring callback-owned state.00788210 also closes
auxiliaries, serialization owner and both critical sections. Callback context must
remain stable through Cancel/Close/Release; no UI/world pointers may expire early.

The [source transport component](../../recovered/networking/directplay_server.cpp)
requires a genuine caller-supplied handler, passes
borrowed API messages only during that callback, and cannot signal game readiness.
It must not expose an acquired COM host as a completed session. A missing handler,
API acquisition failure or malformed provider metadata must fail explicitly.

## Session-name discrepancy

0077BC50/0077BD50 (ACP/UTF8) compute length+1 for sizeFFFFFFFF, then call
MultiByteToWideChar with source length `-1` and **destination capacity length** while
requiring the API to include its NUL. The original ignores the return and writes
its own NUL at the final index. For an ASCII name the supplied output capacity is
one wchar short. The caller does not establish what prefix the API leaves on
failure; static evidence alone cannot certify partial output or its runtime name.
Assembly0077BC6B..94 and0077BD6B..97 confirms this; it is not a decompiler guess.
The transport boundary therefore takes an already-owned wide name explicitly;
no guessed conversion or uninitialized stack contents are introduced. This does
not reconstruct the original name helper or solve its integration discrepancy.
Actual configuration-to-wide-name integration remains open and must record its
safe behavior and original-fidelity implications.

## Verification boundary

The production component compiled and linked against the Windows i386 SDK;
static assertions verify the72-byte application description and20-byte caps.
The temporary native shim compiled the same production .cpp under ASan/UBSan/LSan:
21 acquisition/metadata/HRESULT cases and4000 lifecycle cycles passed. Cases cover
COM changed-mode/reference balance, each acquisition stage, malformed provider
size/count, ignored GetSP/port/caps errors, actual Host HRESULT, owned names,
callback exceptions, joined worker callbacks and callbacks during cleanup.
Temporary sources and binaries are hashed in the
[verification receipt](startup-network-verification.json), then removed.

These injected COM objects establish call/ownership behavior only. Live COM/runtime
compatibility, discovery and a genuine client/server session remain unverified.
Only00779450's native TCP/IP acquisition and007882B0's server COM cleanup subsets
receive partial ledger credit. The actual game callback0077DCF0, voice, auxiliary
sockets, player queues, profiles, engine/module owners and full bootstrap remain
dependencies. This does not deliver an independently runnable server.

The transport requires lifecycle calls on its constructing thread and a stable
caller-supplied handler/context through stop. It balances only successfully
acquired COM references; RPC_E_CHANGED_MODE acquires none. Provider bounds checks,
exception-to-HRESULT conversion and defined owned-name input are deliberate native
safety boundaries, not claims that the original performed those checks. No
delegated Windows/engine API implementation receives executable-function credit.

## Fresh command settings and profile dependency

[initial_server_command_settings](../../recovered/networking/server_initial_settings.h)
now supplies the fresh command-facing fields from configuration reset006EDCF0,
nested descriptor reset0077C630 and network initialization00787EA0. This does not
construct their containing owners. Server-specific confirmed values:

| Field | Initial value / evidence |
| --- | --- |
| Dedicated |1 at006EDD35; client reset differs |
| Host port |11001 at configuration+220; LAN transport still selects31567 |
| Admin password |Three control bytes01/02/03 at00883374, assigned to+21C; no user credential was read |
| Active rules |22 words copied to+228; warmup30, respawn10, spawn protection1 |
| Maximum ping / frequency / inactivity |Network+5064=0,+5068=50,+506C=300000ms |
| Stored query port |Configuration+1E8 is not written by reset; native zero is an explicit guard |

The four profile rule blocks differ from the active reset block. Their constants
at the tail of00644400 agree with the existing client rule helper, independently
checked against this EXE's exports. There is no new implementation/ledger credit
for that delegated helper.00644400 first tries `mpmaplist.txt`, then
`GameData\\mpmaplist.txt`; if both opens fail it returns before writing those rule
blocks. An unavailable catalogue cannot silently supply initialized profile rules.

Actual ownership is lazy:007A23E0 first creates007A21A0, which calls006ED090 to
allocate configuration+294 and load the catalogue00644400. Raw constructor
006F71B0 owns GameData/config/profile/HumanStore and additional data consumers.
006F9550 calls GameData006FF900 and HumanStore006E33E0 even when the first loader
fails, returning their low-bit AND. These containing owners and the full catalogue
descriptor tails remain required integration work. Existing client implementations
are reuse candidates only after server-address contract comparison.

Fresh settings pass a focused native ASan/UBSan/LSan audit through the actual cold
command implementation with installed Default.cfg and30 catalogue entries. The
oracle extracts active/profile constants from fingerprinted exports and the admin
marker directly from the original PE. Tests distinguish active/preset rules,
verify all fresh fields and owned instances, and confirm Default.cfg's `maxfreq 0`
retains50 while `maxinactivity 0` writes zero. Strict Windows i386 fixture link
passes without execution. [Receipt](initial-settings-verification.json).

Exploratory exports stopped at absent entries006BB860 and007A4460. Those helpers
and007A44A0 were inspected as static PE instructions only, with no new function
credit. Earlier successfully exported C/JSON pairs from those jobs remain valid.

## Cold network resources and player-slot reset

[DirectPlayServerOwner](../../recovered/networking/directplay_server_owner.cpp)
now contains the real transport, both Win32 critical sections from00787EA0 and
the initial32 slot records. Lock construction precedes slot allocation. Each
slot follows0077D4B0's known stores in its0xC4-byte record:95 bytes are written,
including the first name byte, nine counters, zero pointer/flag words, state
words2/2, reservationFFFFFFFF and value50. The remaining101 bytes survive reset;
fresh native storage defines those otherwise unwritten bytes as zero.

The slot is an owned data image, not a wire packet or an ABI object containing
usable native pointers. This representation does not implement a live player,
serializer, worker or admission reservation. No such object is fabricated.

00779450 first calls007882B0, which clears the original slot vector after closing
the old endpoint. Consequently the initial32 records become an empty retained
allocation before first Host, rather than32 admitted players. Owner::start now
performs that ordered close/clear before invoking the actual transport. Final
owner cleanup keeps both locks and records alive through Cancel/Close/Release,
then retires slot storage and locks. The queue lock exists for its real later
consumer; no successful empty queue processor is installed.

The new owner/slot source passes native ASan/UBSan/LSan:10000 independently seeded
slot images match the store mask extracted from the original body, including
preserved bytes; initialization/allocation failures and snapshot exceptions
unwind;200 owner lifetimes/400 endpoint starts retain locks through joined close
callbacks. This audit injects the previously verified transport and lock services,
so it tests the new ownership code without repeating the transport's unchanged
COM audit. The Windows fixture links both real production components against the
i386 SDK and asserts critical-section size24 and slot-image size196. It is not
executed. Temporary sources/binaries are hashed and removed in the
[receipt](network-owner-verification.json).

00787EA0/007882B0 partial mappings expand;0077D4B0 and00788210 gain partial
mappings. No complete-function promotions. Actual callback dispatch0077DCF0,
auxiliary/voice/sockets, serialization and worker owners, and their containing
application startup remain required to complete the selected N1 checkbox.

## Lazy command catalogue and profile-rule acquisition

The new three-argument ColdServerCommands constructor takes initial configuration,
the command-file reader and a separate catalogue reader. It performs no I/O.
First dispatch follows007A23E0 →007A21A0 →006ED090 →00644400 before keyword parsing:
try `mpmaplist.txt`, then `GameData\\mpmaplist.txt` only after open failure; decode
the actual base catalogue metadata; initialize the four verified22-word preset
blocks; retain both for the owner's lifetime. Later commands never reload it.
An empty/missing command file, initial-LF file or unprocessed no-newline tail does
not dispatch and therefore does not trigger that acquisition. An `exec` command
itself acquires the dependency before opening its target, as in the original.

Command fopen and catalogue DTA resolution remain separate adapters. The existing
LocalCommandFiles reads actual filesystem bytes; an integrated archive resolver
is still required for a catalogue present only in DTA archives. The existing
explicit-dependency constructor retains its established supplied-data contract.

Original missing-catalogue construction leaves rule blocks unwritten. Native lazy
loading instead enters an explicit failed state; it cannot run commands with
invented presets or silently retry on a later command. Malformed markup and
allocation failure also leave that state failed. A valid empty MAP_LIST supplies
actual preset defaults, but cannot produce a host request without a selected map.
The metadata decoder's existing bounded markup policy is retained. Full objective,
equipment, allowed-preset and zone descriptor ownership remain outside this
partial00644400 mapping; no successful gameplay descriptor is fabricated.

Focused native ASan/UBSan/LSan verifies lazy timing, separate reader order, both
fallback paths, malformed/missing/allocation failure, valid empty catalogues,
nested exec, explicit-constructor compatibility and200 fresh installed-config
lifetimes. Strict Windows i386 fixture link passes without execution. Temporary
audit source/binaries are hashed and removed in the
[receipt](lazy-catalogue-verification.json).007A23E0's existing partial expands;
00644400 gains partial credit. The containing virtual owner/ABI helpers do not
receive new credit merely because their logical dependency is acquired here.

The [headless HumanStore follow-up](headless-startup-data-source.md) now provides
the required HumanStore data loader with the server's verified portrait-free path.
It also now provides verified GameData reuse and006F9550 preparation order. The
containing application/engine/module owners remain dependencies.
