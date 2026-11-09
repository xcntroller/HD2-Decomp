# Headless startup data — HumanStore

Source-only dependency of the open N1 startup/shutdown checkbox. This does not
construct the application, load GameData, start a controller or establish a session.

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
