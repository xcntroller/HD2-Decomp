# Original application data and profile preparation

This slice recovers the data loaded on the way to the opening movies, extending
`975e1c9` HumanStore work. Source is in
[GameData](../recovered/frontend/game_data.h),
[application preparation](../recovered/frontend/application_data.h),
[profile directories](../recovered/frontend/profile_directories.h),
[profile payload/movie state](../recovered/frontend/profile_payload.h), and
[profile loading/finalization](../recovered/frontend/profile_load.h).
The [Win32/CRT boundary](../recovered/host/profile_directories.h) uses the actual
executable-directory owner and original file/graphics adapters. No live preview
behavior changes, game execution or original-file writes were made.

## Application preparation

006B65D0 loads GameData using application +04, writes that SAME owner's +E8 to
one, then loads HumanStore using application +10. Both run even if GameData
returns false; return their byte AND masked with one. The implementation now
composes the actual recovered loaders, rather than successful callback stubs.
HumanStore resolves the current profile only after its records/textures finish,
matching the original overlay query order. Its old explicit-directory API remains
available for bounded use. Shared 006E4FB0 localization lookup now lives in
`localized_text.h`; this move receives no additional function credit.

GameData manager construction at 006B4AE4 initializes +EC to two, +E8 to zero.
Loading appends roots without clearing previous state. Try
`GameData\Gamedata%02d.gdt`, starting at zero, with one reused ChunkFile object.
First absent file returns whether the root list is nonempty, including roots
from earlier calls. A first chunk3 stores a format word; equal to +EC chooses
the chunked reader. Absent/mismatched metadata chooses the older flat reader.
The subsequent root must be one. Invalid root remains entered/open; next open
is rejected by ChunkFile before backend access, ending with the same nonempty
result. Native ownership frees the original otherwise-leaked unused allocation.
Recognized root is appended, left, and file closed before the next numbered file.

## GameData records

| Original reader | Native kind / contents |
| --- | --- |
| 006BE3D0 / 006BB0A0 | Root1, stored word +08 and children; old header discards three words first |
| 006BDF60 / 006BAE90 | Campaign2, word +08, string +24 and children |
| 006BD990 / 006BABE0 | Mission3, word +08, flag +2C, strings +28/+24, four words +40 and related records |
| 006BD4F0 / 006BAB00 | Related4, words +08/+10 and newer flag +14 |
| 006BCB30 / 006BA530 | Text5, strings +10/+14 and flag +18 |
| 006BD010 / 006BA830 | List6, word +10, flags +14/+15, raw 16-byte entries and item words |
| 006B9C90 / 006B9C80 | Kind7 readers really return true without consuming their payload |

006BC5A0 creates kinds5/6/7/3/2 from chunk tags10/20/30/50/60 respectively.
Old hierarchy helpers 006BB9B0/006BBD40/006BC000 instead recognize direct kind
IDs appropriate to each parent. Both retain mixed child order. Mission-related
kind4 records have a separate vector. The chunked mission reader leaves each
related record before appending it; ordinary children append before leave.
GameData readers skip unknown chunks by leaving them, unlike HumanStore.

Old mission headers discard four extra words and have two optional tag1000
strings. Old list headers read flag +15 BEFORE +14; newer tags22/23 target +14/
+15 respectively. Related +10 defaults to four; list +10 to FFFFFFFF; container
iteration word +20 defaults to FFFFFFFF. Stored strings/vectors otherwise start
empty and defined numeric fields zero. Native records unify derived layouts
without claiming their original ABI. Unknown allocator-derived bytes/padding
are represented by deterministic native initialization, not guessed meanings.
Native destruction releases owned child/related lists from the back.

Native GameData guards reject failed headers, short fixed-size fields,
unterminated strings and file/count overflow. Older original readers often
ignored short transfers or continued from stale/uninitialized scratch instead;
these malformed-input differences are explicit. Native allocation/SEH and
unused object bytes also differ; the original ABI is not reconstructed here.

006B9030 counts only kind3 missions under kind2 campaigns. Read-only 006B8D20
flattens the same order and returns the mission's localization ID, or FFFFFFFF
on failed lookup. Neither helper selects a runtime iterator or changes +20.
Their mutation-capable underlying virtual methods are outside this scope.

## Profile directories and names

006AEF50 first clears the global display-name -> physical-directory map, then
enumerates `PlayersProfiles\*` in API order. Require directory bit10; skip exact
`.` and `..`, while retaining dot-prefixed and other directory names. ANSI mode
passes physical names directly to the optional callback and leaves the map empty.
Wide mode calls 006AF730, inserts the first exact duplicate key only, then invokes
the callback for every accepted directory, including duplicates. Callback uses
name/user context (original ECX/EDX). Normal exit closes the search and attempts
to restore the executable cwd, even if the initial search failed.

006AF730 initializes output to the physical directory, opens its `profile.sav`,
requires root1EDC, and returns the first child1EDD string, including empty text.
Missing file, wrong root or no matching field retains the physical name. Extra
fields are skipped; early success closes without leaving the root. Native
exception cleanup closes active files/searches.

006AE530 resolves the current display name exactly in wide mode; missing lookup
returns empty, with no fallback to the displayed name. ANSI mode returns the
stored name. 006B7100 formats `PlayersProfiles\<resolved>\`; empty resolution
therefore retains two backslashes. No case folding or guessed default profile.
The Win32 adapter consumes the same name/directory bit as the original CRT
_findfirst wrappers; unused timestamps/errno/normal-attribute conversion remain
outside this bounded contract. Cwd changes use CRT _chdir with the cached actual
executable directory, not the repository or original installation path.

## Profile payload, fallback and save checks

006AFB00 opens the resolved `profile.sav`; on success reset006AF420 precedes the
root1EDC check. Reset clears strings +04/+08, sets difficulty +34 to two, eight
words +74..90 and five slot IDs +0C/+14/+1C/+24/+2C to FFFFFFFF, then resets
movies. Other words and slot strings persist. All known binary fields are
recovered: display name1EDD (wide only), old/new difficulty and progress fields,
repeated old1EE8 names (first +04, later +08), individual save names/captions,
five saved-slot IDs and strings, and movie flags. Direct numeric reads preserve
initialized/partially overwritten words when the original ignores read failure.
Payload loop uses signed remaining >0 and skips unknown fields.

Movie reset006A6B10 installs ten fixed records A0..A9, Intro through Outro2,
reserves at least 20 records and enables Intro. 006A6D10 replaces the flag of the
last case-insensitive filename match. 006A6E80 loads ten flag bytes in reverse
record order from child1EC8, preserving initialized flags on short bounded reads.
These are profile movie availability flags, distinct from the four opening clips.

Missing binary profile invokes actual legacy fallback006AF490: reset, restore
executable cwd, read `profile.pr`, restore cwd again, finalize saves. The old line
reader006E9230 appends nonempty raw CR/LF-separated lines and closes only signed
nonnegative handles. This differs deliberately from ChunkFile's handle rule.
006E9840 finds the first case-sensitive key SUBSTRING anywhere on a line, then
returns everything after the first `=`. No comment/key-prefix/quote processing.
006E9920 reads a whitespace-delimited token and requires complete decimal strtol
consumption, preserving 32-bit saturation. String values retain whitespace.

Fallback keys are GAME_DIFFICULTY, LASTGAME, LASTCAMPAIGN, LASTLONEWOLF,
LASTCARNAGE, LASTSAVE and LASTSAVETEXT. Numeric scratch persists across failed
later lookups; text scratch persists too. Signed difficulty >4 becomes three;
negative values remain. Native guard rejects a failed FIRST LASTGAME lookup
because its original scratch was uninitialized, plus invalid value pointers,
unterminated strings, old buffer overflow and failed raw headers.

Finalizer006B03B0 runs while a valid binary profile's root/file is STILL open.
It validates five saved slots, clearing invalid IDs/names, then checks the last
quick save. Null original application context returns false without changes.
Validation006C47C0 requires case-sensitive `quick` anywhere in the filename,
opens through the same DTA context, checks root >4E8B and first child0BB6, reads
mission index and compares it SIGNED against actual GameData mission count.
Negative indices are not rejected by the original. Output index is assigned
even for invalid root/marker, enabling the later recovery path. Failed numeric
reads preserve the original initialized header/scratch word.

Promotion006B0520 requires a positive index below mission count and a valid
previous-mission caption. A CAM_/LOW_/CAR_ prefix selects the prior completed-save
filename and localized category. Filename mutation precedes physical existence
check through CRT fopen("r"). Caption uses localized2051 with category, mission
name and empty third string. Native formatting guards unsupported substitutions;
known translations use %s/%%. Failed promotion clears last-save name/caption.
Finalize results are ignored by the binary/fallback loaders as in the original.
The fallback returns its file-read status; invalid binary root returns false.

## Verification and remaining boundary

An independent Python raw-offset decoder and sanitized native loader agree on
all stored fields of the installed 22,161-byte Gamedata00.gdt: 304 records
(one root, eight campaigns, 24 missions, 159 related records, 30 text records,
58 lists, 24 kind7 records), including 502 raw 16-byte entries. Synthetic old/new
files containing all seven kinds produce identical native state. Audits cover
numbered files/gaps/reloads, invalid roots, format matching/mismatch, unknown
fields, mixed order and malformed-input cleanup.

The larger temporary ASan/UBSan audit exercises directory duplicates/case/empty
names, metadata fallbacks, cwd/search lifetime, actual GameData +65 HumanStore
record composition, +E8/failure ordering and late profile path lookup. It also
covers every profile payload field, reverse/short movie flags, reset preservation,
legacy text/token/handle quirks, quick-save signed bounds/output, completed-save
promotion/captions and nested profile/save handle lifetime. Strict i386 links the
actual ChunkFiles/HumanPortraits/Win32 profile-platform/data/profile composition;
no Windows executable or original DLL operation is agent-executed. Existing
native checks, all 158 Python tests and unchanged host target pass. Temporary
fixtures/extracted data are removed; no permanent tests added.

[Evidence](startup-data-map.json) checks both original hashes, 74 byte anchors,
578 direct calls, 22 client slots and two engine texture slots. Six negative
mutations are rejected. `DefineGameDataFunctions.java` verifies three original
slots and transiently defines missing reader functions in the read-only project;
changes are discarded. 006BD4F0 is absent from the frozen inventory: source and
evidence are present, but it receives no function credit. No constructor, vector
helper, duplicate localization, original no-op or delegated DLL extra credit.
36 new partial client entries bring totals to 221 (216 client/five engine), zero
complete, denominator 18,783. Native layouts, guards and narrowed interfaces keep
all entries partial.

Run `python3 -B tools/validate_engine_host.py --map analysis/startup-data-map.json`.
Next: original Options.bin/default binding/settings load and actual graphics/
sound/input application, then outer profile selection006AF1D0 and startup profile
reset006B6620. Those services are not replaced by successful placeholders.
Protection-state effects and full application/controller/state ownership still
precede live original-launch integration. The independent launch milestone stays
open; the current preview does not yet exercise these new source components.
