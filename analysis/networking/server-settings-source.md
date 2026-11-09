# Cold server settings and command files — 2026-11-09

Verified source-only checkpoint. Actual application/transport integration and
desktop acceptance remain open; no dedicated server executable is delivered.

Completion boundary: implement every launcher-emitted setting in HD2DS007A23E0,
the idle-mode `server` request, required style/rules/map dependencies and
007A4080 file traversal, including nested exec/failure/length/repeat behavior.
Active-session administration, Join, gameplay commands and actual controller/
transport delivery remain separate receivers. Parsing `server` cannot report a
started host. Initial settings and per-style profile rules must be supplied by
the actual bootstrap/profile owner; no invented server defaults are introduced.
Follow-up: the [startup work](startup-network-source.md) now recovers fresh
command-facing configuration/limits and actual lazy catalogue/preset loading.
Full descriptor/profile ownership and controller delivery remain required.

Evidence: original server fingerprint in [manifest](binary-manifest.json),
`server-lifecycle/007a23e0.{c,json}`, `server-settings` and
`server-settings-model` under ignored `analysis/work/networking`. All used
functions have successful fingerprinted JSON/C exports. Original helpers:

| Original | Required behavior |
| --- | --- |
|007A23E0|Keyword dispatch; rules0..13/15..21, map/style/domain/name/port/password/voice/admin/limits/dedicated/server/exec|
|007A4080|fopen rt; fscanf `%[^\n]s`, whitespace scan `\n`, EOF flag before dispatch, close file|
|007A47B0 /007A4990|Existing recovered token and separator rules; do not replace with launcher parsing|
|006ED040|Select style in profile options; copy22 signed rules from its selected block|
|006454B0 /006434A0|Profile selected style +18 and rule blocks +200/+258/+2B0/+308|
|006ED2E0|Require loaded catalogue/style0..3; reverse case-insensitive name search; append cloned descriptor|
|00643470|Select corresponding catalogue vector +1C/+30/+44/+58|

The native map representation can retain recovered front-end metadata, but it
cannot stand in for the original full0x48 descriptor's gameplay data. No original
descriptor ABI or world-ready host claim follows. Catalogue content must be
provided from actual recovered data; a missing/empty catalogue cannot append maps.

At idle mode7, host bit1 allows rule mutation. Rules retain signed integers;
do not normalize all boolean-looking values. Clamp only evidenced cases:
maxclients≤32 (≤6 in style3), roundlimit1..99, warmup1..5940,
respawntime3..5940, spawnprotection≤15, cooplives≤99; co-op difficulty1..4 only
when the initial style is3. Others retain the parsed value. Each mutation emits
original event12h. Style accepts deathmatch/occupation/objectives; base server
explicitly rejects cooperative. Style reloads rules but does not clear rotation.

Port accepts signed numeric values≤65535, writes both stored ports and selects
Internet even after `domain local`. Maxfreq0 preserves the previous limit;
maxinactivity multiplies seconds by1000 with32-bit storage. Session name,
password, mapname and admin strings only consume text-class tokens: quoting
`123` does not stop the tokenizer classifying it as a number. Password mutation
also sets the original presence byte. Dedicated consumes a token and sets true,
even for argument0. `adminpass` changes stored server password; `adminlogin`
emits administration request7 and is not a stored server default credential.

Exec uses the whole remaining path after separator advancement, preserving spaces
and without interpreting surrounding quotes. Files are opened relative to process
current directory unless absolute. Windows text conversion removes CRLF and stops
at Ctrl-Z. The scanset cannot consume an initial LF, so a leading blank line stops
the file. After a captured line the whitespace scan consumes all intervening
whitespace, including blank lines and next-line indentation. An unterminated last
line is skipped because EOF is checked before dispatch. Semicolon is **not** a
server comment; unknown commands fail individually, and file traversal continues.

Original local line buffer is100 bytes and has no scanf width. Native code
rejects captured lines>99 bytes and reports malformed tokens, cycles/depth overflow
and missing files without original stack corruption or fictitious callbacks.
These explicit safety limits do not establish whole-function equivalence.

## Source and verification

`recovered/networking/server_commands.{h,cpp}` now provides the cold interpreter,
real read-only file adapter, owned settings, diagnostics, query values and effects.
Constructor inputs are the actual initial settings, four profile rule blocks,
catalogue metadata and its availability. Native zero-initialized storage does not
claim original server defaults. Rule changes produce event12h requests; `server`
assigns connection mode2 and produces a controller-state7 request. Quit and admin
login produce their distinct requests. None is an acknowledgement from a receiver.
All other active-session/game commands explicitly report unsupported here.

The source preserves valid settings and the original file-reader control flow.
Native safety differences:100+ byte captured lines (including unterminated tails)
reject; NUL/overflow/missing-quote tokens diagnose; recursive filename cycles or
depth32 reject. Malformed lines may leave earlier successfully applied settings,
as the original reader processes in order rather than transactionally. Exceptions
unwind owned buffers/file objects and the active-file stack. Catalogue copies
retain name/directory/description/style only, not the unrecovered descriptor tail.

Strict C++17 native and Windows i386 object builds pass. The full temporary audit
links for Windows i386 and is not executed there. Native ASan/UBSan with leak
detection passes production code against the actual installed Default.cfg and
mpmaplist.txt (30 base entries). It exercises rule limits and style reset, signed
values, domain/port ordering, duplicate and missing maps, numeric quoted names,
password presence, eight codecs, admin/quit/host request ownership, missing files,
nested paths with spaces, cycles/depth limits, repeated execution and100-byte lines.

12,000 file cases compare against an independent bounded C stdio reference using
the original scanset/whitespace/EOF sequence.25,000 deterministic mutations cover
malformed/truncated files under sanitizers. Explicit Windows-text cases verify
CRLF conversion and Ctrl-Z. The audit's initial reference callback failed to skip
leading whitespace; that reference was corrected before the differential sweep.
No production parser change was required for that mismatch. Native source and
Windows binaries are hashed and removed after verification; no permanent tests
are added. [Receipt](server-settings-verification.json).

Networking ledger: four new server partials007A23E0/007A4080/006ED040/006ED2E0;
zero complete promotions. Catalogue/profile accessor helpers remain dependencies,
without additional function credit. No main client/engine ledger change. These
checks do not establish network callbacks, live discovery, admission or desktop
acceptance. Next full checkbox: actual startup/shutdown and network initialization,
including the real bootstrap/profile/controller owners and DirectPlay acquisition.
