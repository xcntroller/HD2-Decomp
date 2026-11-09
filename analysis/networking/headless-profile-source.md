# Dedicated profile startup — source checkpoint

HD2DS.exe fingerprint and inventory remain those in [binary-manifest.json](binary-manifest.json).
This is owned source composition, not integrated application startup or desktop acceptance.
[Implementation](../../recovered/networking/headless_profile.h) requires actual file,
directory, engine and network backends. No default successful services were added.
The full N1 startup/shutdown checkbox remains open.

## Original path and server differences

`006F95C0` optionally scans profiles when application+30 is true, then queries
the separate startup-name owner through `006E9580`, then selects through the
application+20 profile owner. It does not write game.cfg. `006F0380` resolves the
candidate before clearing the current name; only a nonempty candidate and a
successful actual directory change retain the requested spelling. It restores
the executable directory even on failure, clears all256 DWORDs at
00899304..00899700 in reverse order, calls options `006F4E60`, then payload
`006F0F30`, ignores both booleans and returns true. Native exceptions propagate.
The native retained spelling also guards a caller string aliased to current name.

Assembly `006F03A6` saves ECX as owner and `006F05C9` returns with RET4.
[ConfigureNetworkingEvidence.java](../../tools/networking/ConfigureNetworkingEvidence.java)
now transiently models both this function and its00409D1D thunk as thiscall with
one name argument. Read-only Ghidra models change no original bytes; the former
model invented an uninitialized owner local.

Directory scan `006F0060`, physical resolution `006EF380`, metadata `006F0A60`
and directory formatting `006FA160` match the reused ProfileDirectories behavior:
only directories other than dot entries, unsorted enumeration, wide-mode metadata
and map only, first exact duplicate wins, no missing-map fallback. Metadata reads
the first1EDD inside1EDC. `007330F0` constructs executable-directory-relative
components and performs real chdir; empty components and a failed chdir matter.

`006F4EA0` reads three sequential top-level Options.bin chunks:25A0/version141,
25A6 bindings,25A5 settings. Missing outer children reset both settings and
bindings, but the outer loader still returns true after a valid version. Inner
binding failure only clears bindings. The shared 60-byte settings reader matches
`006F5A70`: child1201, seven DWORDs, four seven-byte quality blocks, four tail
bytes; read results are ignored and partial transfers retain other initialized
bytes. `006F5660` matches the shared default constants. Its unwritten+14 scratch
is explicitly zero in native reset, not claimed as an original value.

The server is materially different from client input setup: `006EC550` sends
either default-layout selector to `006EBF70`, which only assigns vector end=begin.
It does not append the client's92 default records. `006EC1B0` reads22B8/version42,
then22B9 containing raw28-byte records, overwrites dirty=1 and appends in order.
Its final apply helper `006EBF40` is a real RET, so dirty bytes remain set.
No input manager, keyboard snapshot or joystick service belongs in this path.

`006F4E60` applies even after failed loading: `006F5540` gets I3D driver twice
when present and writes state2B from quality-byte0; `006F5460` is RET;
`006F5510` loads owner+48 (=settings+10), multiplies by the double0.01 at008509B8,
uses __ftol and forwards EAX to `007888D0`; `006F54F0` is RET. The required backend
preserves those two real operations. `007888D0` gates on network+0C and constructs
an eight-DWORD voice-client configuration before `007A2090` delegates virtual+20.
Actual voice ownership remains an integration dependency; this checkpoint gives
those delegated functions no implementation credit.

## Payload and legacy fallback

Server `006F0F30` matches shared payload traversal: open profile.sav first, reset
`006F0670` only after successful open, require root1EDC, process the recognized
name/numeric/string/movie tags and skip unknown chunks. Wide mode alone permits
1EDD to change the displayed name. Old1EE8 fields select first/second strings;
new fields overwrite their corresponding slots. `006EA120` reads the ten movie
flags backwards. Existing native bounds/string/depth guards remain explicit.

Missing profile.sav invokes `006F0700`: reset, change into PlayersProfiles plus
the **raw current displayed name**, ignore chdir failure, read relative profile.pr,
restore executable cwd, finalize. This directory change is implemented explicitly
in the server wrapper. The shared client's outer load wrapper was not reused or
changed. Its field-only legacy reader matches the server GAME_DIFFICULTY,
LASTGAME, LASTCAMPAIGN, LASTLONEWOLF, LASTCARNAGE, LASTSAVE and LASTSAVETEXT lookups.
Absent later numeric/string values retain the prior scratch. Undefined first
numeric scratch is rejected natively. Difficulty greater than4 becomes3 using
the original signed comparison. Invalid binary root returns false without this
fallback; a parsed binary payload returns true irrespective of finalizer result.

`006F1A10`, quick-save validator `0070AA50` and promotion `006F1BE0` match the
shared helpers: five saved-game slots, case-sensitive quick substring, root>4E8B
and marker0BB6, mission output even for invalid marker, signed mission upper
bound without a nonnegative check, prior-mission CAM/LOW/CAR promotion and actual
filesystem probing/localized captions. A missing GameData owner makes the
finalizer return false without mutations; it is not ready-world substitution.
GameData mission lookups were already verified separately.

## Verification and remaining boundary

[Focused receipt](headless-profile-verification.json): native ASan/UBSan/LSan,
independently assembled chunks and literal expected fields, reset/inner-versus-
outer fallback, raw records/dirty/padding, partial settings, malformed/no-progress
records, all options truncations, sampled payload truncations, directory ordering,
wide duplicate names, aliased selection, failed/missing inputs, backend exceptions,
signed/nonfinite conversion, saved-game validation/promotion with actual GameData,
and100 repeated profile lifetimes. Strict Windows i386 fixture linked and was
not executed. Temporary audits were hashed and removed. No actual game.cfg or
user credential was read. Frozen client/LS3DF ledger remains unchanged.

Continue actual application construction/configuration, backend acquisition,
module registry/controller/command ownership and post-Host auxiliaries. A generic
source backend requirement is not an implemented engine/network backend or a
standalone dedicated server. Session query still requires real owner progress100.
