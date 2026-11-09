# Headless startup data — HumanStore and GameData

The [owned application-data composition](../../recovered/networking/headless_application_data.h)
now connects these loaders to [dedicated profile selection](headless-profile-source.md)
and the authoritative cold command settings. See the ownership boundary below.

Source-only dependency of the open N1 startup/shutdown checkbox. This does not
construct the application, start a controller or establish a session. The follow-up
below now supplies GameData loading and the original preparation order as well.

[HeadlessHumanStore](../../recovered/networking/headless_human_store.h) reuses the
[record parser](../../recovered/frontend/human_store.h) after separate HD2DS-address
comparison. The shared template defaults to the existing client portrait path.
Its headless specialization compiles graphics calls out; it supplies no graphics
service and cannot manufacture successful device operations.

## Original contracts

| HD2DS address | Verified behavior |
| --- | --- |
|006E33E0|Open required `tables\\HumanStore.def`; missing input preserves existing records; enter raw root and close after load |
|006E36E0|Reset, accept raw record IDs, append defaults before field parsing, then query the actual profile directory and attempt optional usernames |
|006E0C70|Forward record retirement with retained vector capacity; reload eight name pairs through localization |
|006E39C0|Initialize0x74-byte record strings, numeric/stats/tail defaults; derive names/eye; append before parse |
|006E15A0|Recognized fields, legacy numeric conversions, item append, unknown-child traversal, text and portrait finalizers |
|006E1210 /006E1370|Combined/display/short name derivation and face-to-eye filename derivation |
|006E2670 /006E2270 /006E22C0|Six-string wide-language finalization; old UTF8 predicate; ASCII/ED-only legacy conversion |
|0067FF70 /0067FFB0|UTF8 traversal and predicate, including surrogate acceptance and unusual F4 second-byte lower bound |
|006E27A0 /006E2220|Optional usernames root/index/name traversal; retain earlier mutations on a later invalid entry; finalize stored name |
|006E0960 /006E0A40|First name IDs27A6..27AD, second2792..2799, initially empty cached strings |
|0072FD30|Actual localization lookup or signed missing-index diagnostic |

The portrait difference is confirmed in original instructions, not inferred from
the dedicated-server name:006E2730 is `MOV AL,1; RET` with no device work.
006E0DF0 stores zero to record+20 without releasing an engine texture. The shared
headless policy therefore skips portrait loading and clears that pointer during
retirement. The default client policy retains its actual load/release calls.
The trivial portrait helper receives no extra ledger credit merely because its
call is eliminated in this specialization.

Native records retain the existing owned strings/vectors/raw IEEE words and
documented malformed-input guards; they do not claim the original object/COW ABI.
The profile-directory callback and loaded localization cache remain required
inputs. No default profile directory is guessed. Records load before the profile
directory query and optional overlay. GameData006FF900 and the containing
application constructor006F71B0/prepare006F9550 still need actual integration.

## Evidence and verification

Read-only/noanalysis exports under ignored `analysis/work/networking`:
`server-headless-data-20261109`, `server-headless-store-contract-20261109`,
`server-headless-store-helpers-20261109`, `server-headless-store-text-20261109`,
`server-headless-text-contract-20261109` and `server-headless-utf8-20261109`.
Every used function has a successful fingerprinted JSON/C pair, hashed in the
networking evidence receipt. Original executables/DLLs were not run.

The existing native Python archive decoder extracted the authorized original
`tables\\HumanStore.def` from others.DTA,9170 bytes, SHA256
`e070e9b0f8f1489ce0ff40316726916afed5bfe5cb58e2e4dcfe06ace073cb5f`.
It remains local and ignored. An independent raw-chunk parser derives canonical
hashes for all65 records; native ASan/UBSan/LSan agrees on all strings, derived
names/eyes, numeric/stats/tail fields, empty item lists and null portraits.

Focused checks also cover client default portrait calls/releases, default name
IDs, missing-input preservation, overlay ordering and partial mutations, legacy
integer stats, wide conversion, items/tail data, arbitrary root/record IDs,
300 truncations, short transfers and100 complete lifetimes. Strict Windows i386
fixture compilation/link passes without execution. Temporary audit source,
oracle header and binaries are hashed and removed in the
[receipt](headless-store-verification.json).

Only006E33E0/006E36E0/006E15A0/006E0C70 gain partial server mappings. The whitelist
for validated shared source allows this specific header for server mappings;
the frozen client/LS3DF ledger and denominator remain unchanged. No whole-function,
application-startup, desktop or interoperability completion is claimed.

## GameData and mandatory preparation

[prepare_headless_startup_data](../../recovered/networking/headless_startup_data.h)
reconstructs006F9550: call GameData006FF900; set that data owner's+E8 word to one;
attempt HumanStore regardless of the first boolean result; return the low-bit AND.
The+E8 word belongs to startup data, not the game owner's network-query readiness
field+28. This helper never sets progress100 or acknowledges a session.

The [shared GameData reader](../../recovered/frontend/game_data.h) matches the
server's format2 gate and all seven data kinds.006FF900 visits sequential
`GameData\\Gamedata%02d.gdt`, stops at the first missing file, appends loaded roots
and returns whether any real roots exist. An invalid root stays open; next open
refuses before a backend call, then normal close runs. Existing roots may therefore
make a later load return true even if its current root is invalid; that is the
original cache behavior, with a native rejected-root diagnostic retained.

| Server entries | Verified reader boundary |
| --- | --- |
|006FF330 /007033D0|Root flat words / format2 caption and child group |
|006FF090 /00702E30|Campaign flat caption/text / format2 fields3D/3E and children |
|006FED30 /007026E0|Mission flat fields / format2 fields33..37, children and related records |
|006FEC10 /00702100|Related type4 flat words / fields29/2A/2B |
|006FE4C0 /007014B0|Type5 strings and byte flag in both layouts |
|006FE880 /00701AE0|Type6 raw entries/items and byte flag ordering |
|006FD940 /006FD960|Type7 genuinely returns true without consuming payload |
|006FFEA0 /00700330 /007006A0|Parent-specific legacy child and related routing |
|00700DB0|Format2 tags10/20/30/50/60; skip unknown child tags |

Data pointers/allocators/COW objects remain native owned representations rather
than the original ABI. The existing bounded reader's short-header/string/numeric
guards remain explicit. No movie, mission, AI or gameplay consumer is executed.
The data owner's one-entry language vtable is adjacent to the mission data table;
adjacency does not make those mission slots application-controller methods.

The same native archive decoder extracted22161-byte Gamedata00.gdt from the
authorized original others.DTA, SHA256
`f92ab7e8796148af7854438ffca4df34b55cca8e83336878d7f569cdcd3319b1`.
An independent raw-chunk tree oracle agrees on all304 nodes (kinds1..7:
1/8/24/159/30/58/24), including their fields, ordered children and related lists.
Native ASan/UBSan/LSan also passes legacy and format2 synthetic records, repeated
append, sequential files, invalid-root/cache behavior, all missing-input boolean
combinations, the+E8 store/order,200 truncations and50 complete preparations.
Strict Windows i386 fixture link passes without execution. Temporary audit source,
oracle code/header and binaries are hashed/removed in the
[receipt](headless-prepare-verification.json).

New valid exports: `server-gamedata-root`, `server-gamedata-type-contracts`,
`server-gamedata-fields`, `server-gamedata-related`,
`server-gamedata-related-readers` (all20261109). The `server-gamedata-types` job
exported00700DB0 successfully, then stopped at the erroneous interior00700710
request; actual legacy mission routing is007006A0. No missing-entry credit.
Only006FF900 and006F9550 gain new partial mappings. Actual application/profile/
engine/module construction and controller/transport/session integration remain open.

## Owned startup composition

The source subset of006F71B0 now owns, in construction order, GameData(+04),
startup configuration(+08, immediately loads), dedicated profile(+20), headless
HumanStore(+10), and the recovered cold configuration/command fields(+24).
The startup loader006E8F70 matches the shared three-string reader: executable
directory plus game.cfg, independently cleared100-byte scratch for LASTPROFIL,
PARENTLOC and NETCDKEY, actual I/O even though original ignores its result.
No actual user game.cfg was opened during verification.

Fresh profile construction sets word34=0 and save words74..90=-1, initializes
settings directly and constructs movie records; it does not perform the later
payload reset which sets difficulty2 and five slot IDs=-1. HumanStore preparation
uses the preselection profile-directory value. Profile selection runs separately
after successful mandatory data preparation in the actual controller sequence.
One ColdServerCommands object owns the authoritative cold settings; command
mutations and later consumers observe that same object. Its construction neither
loads the catalogue nor starts a transport. Catalogue acquisition remains lazy.

Normal close preserves006F8DD0's relative+04,+08,+20,+10,+24 destruction order;
native close is idempotent, closed access throws and allocation/file exceptions
unwind owned members. Original allocation-null behavior, raw object/vector ABI,
unwritten scratch, full+24 descriptor/294 owner and other application managers
(+18/+1C/+14/+28/+0C) are excluded. They receive no successful substitutes.
This is an application DATA subset, not the full singleton or controller.

[Composition receipt](headless-application-verification.json): focused native
ASan/UBSan/LSan using actual GameData/HumanStore/catalogue input and synthetic
configuration, early read order, fresh profile values, preselection overlay path,
profile composition, settings identity, lazy catalogue/real command parsing,
missing/failed input, close/closed access and30 owner lifetimes. Strict Windows
i386 fixture linked and was not executed. Temporary audit artifacts hashed and
removed. Only006F71B0 and006F8DD0 gain new partial mappings; full N1 stays open.
