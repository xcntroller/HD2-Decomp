# Configuration and command foundation — 2026-11-09

Source-only checkpoint following N0. This supplies reusable launcher text
configuration and process specification plus the original server's bounded token
reader. It neither invokes CreateProcess nor initializes a server/session, alters
the game installation, nor changes the accepted client menus.

## Implemented boundary

`recovered/networking/launcher_config.{h,cpp}` reconstructs:

* 00412720/0040CCF0 load/reset defaults: occupation, Internet, port11001,
  maxclients32, roundlimit/respawntime30, roundcount3, warmup10,
  spawnprotection5, inverse damage100, co-op difficulty3/lives-1 and original flags.
  The stored Default.cfg explicitly changes the domain to local and adds Africa2;
  it is not the same thing as the loader's initial default state.
* 00412720 line iteration and semicolon comments; 00411F50 named setters and
  repeated map append. Numeric `%i` prefixes are preserved (hex/octal/sign and
  trailing suffix acceptance), unlike the server's decimal `atoi`. Invalid/unknown
  lines leave initialized fields intact and produce owned diagnostics; the native
  library has no MessageBox side effect. Embedded NUL, overlong keyword and
  unterminated string handling are deliberate memory-safe boundaries.
* 00410EF0 string extraction: leading whitespace, optional quotes; unquoted
  string delimiter is a literal space. Quote escaping is not invented.
* 00411600 full logical text order, all 33 recognized configuration keywords,
  repeated maps, Internet-only unsigned port, optional password/admin fields,
  and fixed `dedicated 1` / `server` tail. The function returns LF logical text;
  Windows text-file I/O produces CRLF. File opening/flush/error ownership remains
  an explicit launcher-integration dependency. Serialization does not validate
  map existence or claim to have started a server.
* 004075C0/00407C90/00407D50 build the exact application, command string and
  current-directory specification, including the leading command-line space,
  unquoted full configuration filename and original/expansion executable choice.
  No executable is loaded or spawned. 00408130 value checks reject missing
  session/map rotation and Internet port outside2000..65531. Actual catalogue,
  four-port probe, named-mutex checks and UI warning/reporting remain required.

Launcher enum orders are kept explicit: objectives/occupation/deathmatch/cooperative;
Internet/local; none/vr12/sc03/sc06/truespeech/gsm/adpcm/pcm. The first two differ
from original server/client ordinals; sharing numeric values would be incorrect.

`recovered/networking/command_tokens.{h,cpp}` reconstructs:

* Server007A47B0 returns text0/integer1/keyword2/end3. It skips space/tab/`>` and
  a leading hyphen followed by a letter, supports quoted tokens, performs keyword
  lookup before numeric classification and preserves decimal-prefix semantics
  (`123suffix`→123, `0x10`→0, `-`→0). Quotes do not force a numeric token to text.
* The exact65-record keyword table at00886F78, including the reserved empty entry
  whose value is0. Server007A4990's separate separator advance skips every hyphen.
* Server007065A0's ` -cmd ` substring selection and pending owner tail. Alternate
  `-exec`/interactive-console startup is not claimed. The command owner at+0C
  has0x204 bytes before+210; overlong tails are rejected rather than overwritten.

The original tokenizer has512-byte output buffers and unsafe missing-quote/NUL
behavior. The reconstruction rejects payloads over511 bytes, integer overflow
and missing quote termination, retaining valid-prefix behavior. Text case folding
and classification cover evidenced ASCII keywords; original locale-dependent CRT
behavior for non-ASCII command names is not claimed. No protocol packet decoder
or arbitrary full console command dispatcher is implied by this tokenizer.

## Verification

Strict C++17 native and Windows i386 object builds pass with warnings as errors.
The temporary complete audit also links for Windows i386; it was **not run**.
Native ASan/UBSan with leak detection passes the actual production sources against
the read-only original453-byte Default.cfg oracle. After CRLF normalization, every
field and emitted order matches exactly. The initial audit compared raw CRLF to
logical LF and was corrected; it did not reveal a production-field mismatch.

Focused cases cover loader defaults, all textual enum families, duplicate maps,
signed/hex/octal/prefix values, invalid/overflowing fields, optional credentials,
quote errors, empty strings, filename spaces, command selection, port bounds,
original token lengths and positive/negative integer bounds.30,000 deterministic
malformed/truncated/mutated configurations and token streams run under sanitizers.
All65 source keyword records independently match section-mapped original bytes.
These are format/ownership checks, not networking or desktop acceptance.

Evidence: fingerprinted local Ghidra directories in [receipt](evidence.json), plus
`launcher-defaults`0040CCF0/00406030; direct caller assembly for ostream calling
conventions, original `%i`/`atoi` classifications, and server keyword records.
Temporary native audit source/binaries are hashed and removed at the checkpoint.
[Verification receipt](configuration-verification.json) records the final artifacts.

All credited functions remain **partial**: portable subset/interface recovery,
not original object ABI, full process/UI integration or alternate server startup.
The separate networking ledger is validated independently; no main client credit.

Next full boundary is cold server settings dispatch and command-file execution.
Do not route this through the launcher reader: numeric interpretation, style
defaults, domain/port side effects and semicolon/blank-line behavior differ.
