# Next delivery: finish ordinary opening playback

Implementation follow-up: [ordinary playback completion](playback-completion.md)
is now compiled/native-audited and user desktop-confirmed on2026-10-31.
The contracts below preserve the preceding research checkpoint.

Research checkpoint, 2026-10-30, after desktop-confirmed controller delivery
commit `c0e7346`. This prepares milestone0 item3; it changes no interactive code,
checks no new delivery box and adds no function credit. Use the existing
[controller executable](controller-integration.md), preserving its single device
lifetime. The [evidence map](playback-completion-map.json) fingerprints both
original binaries and the relevant instructions, tables and installed assets.

## Delivery boundary and implementation order

Finish Logo1/Logo2/Logo3/Intro, their actual subtitles/audio, Escape/ordinary
Space behavior, music coordination and movie-to-menu resource release. Keep
English800x600/Main/Exit explicit. Credits, Outro2, D_Outro, retained/game/console
states and full failure/shutdown matrix belong to their existing later scopes.
Do not expand the stable five-item launch checklist with helper prerequisites.

The controller already supplies ordinary playback, fades, actual preferences,
menu music and Intro profile persistence. The missing work is inside the movie
owner, not another playback loop or the general sound-cache/spatial producers.

1. Add an owning ordinary subtitle model/parser using the existing localized
   file/cache code. Expose the actual startup language, executable directory,
   text-file adapter and profile settings to that owner. Keep subtitle input in
   a separate cache; never clear or merge the loaded startup/menu localization.
2. Add checked engine voice/sample wrappers and IShow elapsed-time query. Keep
   sample storage alive until the voice has closed and released its reference.
3. Supply an actual standalone text renderer using the existing atlas/metrics
   and primitive boundary, with recovered style8/rectangle/layout. Reuse native
   TextObject clear/set and actual resources; do not call original client code
   or return a successful render without submitting glyphs.
4. Extend the single-movie backend/update/render/close operations, retaining
   original sound-first/status/time/text/audio order. Keep state wrappers and
   controller replacement ownership intact. No second sound update, independent
   presentation loop or host Escape consumer.
5. Run temporary native sanitizer and strict i386 audits, existing checks and
   the desktop matrix below. Only actual desktop acceptance closes item3.

## Loader and profile contracts

006890F0 copies the filename, strips its final extension and formats
`<executable_dir>\text\<language>\subtitles\<stem>.txt`.006E8ED0 is the already
recovered executable-directory resolver;006ED860 is a real CRT existence probe.
This loose-file check precedes DTA loading; an archived-only subtitle file does
not bypass it. Startup staging already copies the loose Text tree.

At00689296/006892A8 the loader sets ECX to movie+28 and calls006E4CF0 with the
path. This is a separate numbered-text container. Reuse existing
`load_localized_text_file`/`LocalizedTextCache`: CR/LF records, first/last quote,
number parsing, signed ID order, first duplicate wins, -1 rejected, UTF8 BOM
skipped without decoding and UTF16LE rejected. Preserve the recovered unusual
handle-zero close rule. Do not substitute `load_legacy_text_lines` or feed the
raw numbered/quoted source directly to the cue parser.

00689970 consumes stored text values in signed ID order, then clears its local
input cache through006E4C80. It snapshots profile owner+72 before iterating.
This maps to **options.settings.bytes[0x3a]**, since profile settings begin+38
(006B6740). It is not payload.words[0x70/4], a global caption toggle or the input
flag at settings byte39. Default initialization sets byte3A=1.

- If a stored value begins with uppercase `S`, it is an audio record regardless
  of the subtitle setting. Allocate a sample, find first `<` and `>`, parse the
  integer after `<`, and use bytes following `>` as the sample name.
- Load `Sounds\<name>` with flags0; on any nonzero load result retry
  `sounds\DATADISK\<name>`, flags0. Append the cue even if both loads fail;
  playback calls retain the original ignored-result behavior. Do not report the
  failed load as successful or replace the sample with an empty success service.
- Otherwise, when byte3A is nonzero, find first `<`, `,`, `>`, split and parse
  start/end, and retain all bytes after `>` as text. Original checks delimiters
  exist and differ; it does not validate chronological order or sort by time.
- The integer parser007E6190 is the CRT atoi path. Preserve whitespace/sign and
  prefix parsing for defined ranges; native overflow/invalid delimiter guards
  should explicitly reject undefined original memory behavior. Malformed-record
  allocations leak in original branches; native RAII need not reproduce leaks.

## Engine audio and time ABI

Original DLL bodies remain delegated. Use actual engine self pointers on the
stack for these stdcall methods; several Ghidra C signatures omit parameters.
The stack loads and RET immediates in the map are authoritative.

| Owner/slot | Original target | Required call |
| --- | --- | --- |
| ISnd+20 | 1007AA80 | sample factory(self), allocates0x4C, constructor1007CCF0 |
| ISnd+28 | 1007AB20 | voice factory(self), allocates0x1C, constructor1007BEE0 |
| Sample+00 | 1007CEF0 | release(self) |
| Sample+04 | 1007CF30 | load(self, filename, flags0) |
| Voice+00 | 1007BFB0 | release(self), RET4 |
| Voice+04 | 1007BFE0 | bind(self, sample, flags0), RET12 |
| Voice+08 | 1007C070 | close(self), RET4 |
| Voice+0C | 1007C0E0 | play(self, flags0, offset0) |
| Voice+20 | 1007C9C0 | volume(self, float1) |
| IShow+38 | 10072400 | elapsed_time(self, uint32 output*), RET8 |

Voice table1009DE70 and sample table1009DF00 are distinct. Voice binding first
stops/releases its previous buffer and sample, duplicates the new sample buffer
and retains that sample; close releases both. Do not use I3D scene click-sound
objects or manager byte-position+34 as replacements.

IShow+38 returns **milliseconds**, including the Bink integer frame/rate path.
On unopened playback it returns80000009 without writing the output. The client
ignores this result and preserves movie+6C, initialized0 by the cold constructor.
The wrapper must retain its last value on error rather than throw or fabricate
elapsed wall time. QPC calls around text changes do not supply cue time.

00688040 creates the voice before IShow, even when no subtitle file exists or
text is disabled. Ordinary failed open skips loading/layout/play but still sets
status1 and advances when status-1 is observed. Allocation/ABI mismatches should
fail explicitly; original null dereferences are not successful fallback paths.

## Update and render order

00688C10 first updates the shared sound manager. Ordinary stopped/unopened video
is stopped/cleared/presented and gets base status2; the function still queries
IShow+38 and continues its text/audio tail before wrapper0066ED80 promotes2→13.
Do not return early and lose those calls. Cold fields from006C0CA0 are current
text index+68=-1, next audio+64=0, movie time+6C=0 and null voice/text/movie.
The four-movie path has no earlier subtitle records before Intro.

Text timing uses unsigned comparisons after wrapping32-bit `time_value * 100`:

- If active, clear only when `end*100 < movie_time`, then set active=-1.
- If inactive, scan every record and select when
  `start*100 < movie_time && movie_time < end*100`.
- Selection uses006443C0→006440F0 with style byte78 and index0. The loop does
  not break: the last matching record in stored order wins an overlap.
- At exactly the end, a previously active cue remains; a new inactive cue is
  not selected. Expiry and selection are deliberately different comparisons.

Audio tests only the next stored cue. If due (`start*100 < movie_time`), set the
real global ISnd volume to profile float08 * float32(.01), bind its sample,
set voice volume1, play(0,0), then advance the cursor once. There is no while-loop
catch-up, and results do not gate increment. Subtitle-disabled text must not
suppress audio. The movie volume's existing multiplier1.1 does not apply here.

00688F50 saves IGraph state0, clears, draws the movie within begin/end, then if
text exists sets state0=1, begins, submits text, ends, and restores the saved
state. The outer controller loop presents. Extend EngineVideo's pass composition
or equivalent rather than restoring state and presenting between movie/text.

## Concrete text layout and installed assets

00688630 creates the0x9C native-equivalent text owner and sets its seven-float
rectangle through00644B80. Constants are binary64 .125/.98/.82, with x87 then
float32 storage: X=.125W..875W, Y=.82H..98H, width=.75W, height=.16H and depth
bits3F43ACC5. At800x600 this is X100..700/Y492..588. Inset byte90 is10, interpreted
in1/1024-screen-width units with the existing truncation policy.

Ordinary ANSI style byte78=8 (wide mode uses26).00641E30 style8 is font3/SMALL,
height bits3D0F5C29 (.035f), ARGBFFFFFFFF, horizontal2(center), vertical1(top),
flags+2C/+2D=1/1 and flags DWORD+30=0. Style26 uses font2/.04f; wide support is
not delivered by the current English host. Do not reuse menu styles11/13,
black text, button bounds or the existing left-aligned label helper unchanged.

Existing renderer research covers00645270 layout,00646820 alignment,
006468D0/006469C0 vertical offset/baselines,00646A10 centering,00644CF0 style/font
lookup,00647E90 color,00648860 glyphs and006404C0 submission. Raw rectangle
coordinates already use pixel space; menu model-bound projection does not apply.
All four actual English Intro strings fit a single line at21px glyph height;
using the existing SMALL.tbl metrics gives widths342/219/259/61, comfortably
inside the inset600px box. This permits a complete bounded launch consumer;
unsupported wrapping/locales must not become successful empty layout services.

Installed English Intro.txt is194 bytes and contains four numbered text cues,
no `S` audio cues. Their100ms-unit intervals are1610–1630,1810–1825,1835–1855,
1865–1875 (first starts strictly after161,000ms). Six installed language versions
have four text/no audio cues with those same times. No Logo1/2/3 subtitle files
exist in these installed subtitle directories. Hashes/metadata are in the map;
no game text/assets are copied into tracked files. Audio still needs native
fixture coverage, since ordinary English desktop playback has no separate cue.

## Cleanup and remaining boundaries

Keep00688460's order: three clear/presents; recognized movie profile write;
restore primary fade1/2000ms (saved music indices both-1 on ordinary cold path);
IShow close/release; voice close/release; reverse subtitle-string destruction;
reverse sample release; fourteen text-style removals/reset-1; text destruction.
Then0066ED30 deletes the descriptor;006C0DE0 frees backing vector/string storage.
The controller's frontend replacement must not run movie shutdown twice.
Intro persistence occurs on skip or failed open too; do not add a success-only
condition absent from the original. Broader exception handling remains item4.

Correction to older preparation:006438D0→00641CC0 removes **style IDs** from the
global style catalog008AE2E8; it does not release font references. Credits setup
006887C0 allocates the14 handles008AE534..568 from52-byte style definitions.
The array is PE zero-initialized; ordinary cleanup still calls the remove wrapper
for each current value and writes-1. Do not infer an initial all--1 array or a
font-refcount protocol. Native bounded text/style ownership must state its ABI
and global-catalog limits; full original style cache is not recovered merely by
freeing a glyph texture. Existing global resource-cache shutdown remains item4.

## Validation required to close the checkbox

Temporary native ASan/UBSan audits should exercise real parser/owners with fake
boundary recording, not successful production stubs: no file/empty/BOM/duplicate
IDs, subtitle byte3A on/off, `S` independence, both load paths/failures, strict
start/end boundaries, overlap-last-wins, wrapping multiplication, large jumps,
one audio per update, preserved time on query error, failed open, Escape, Space,
render ordering, voice-before-sample release and partial-construction cleanup.
Compare the emitted cues/geometry independently against actual Intro.txt/SMALL.tbl.
Compile/link the real engine adapters as strict i386; never run Windows fixtures.

User desktop acceptance should cover natural four movies and visible Intro
subtitles, subtitles disabled, Escape skips/ordinary Space does not, focus return,
missing movie(s), menu music/input and clean movie→Main/Exit transition. A native
synthetic audio cue checks sequencing; do not claim a real desktop audio-cue run
from the installed English Intro. Early close/full shutdown/failure matrix remains
item4/5, though cleanup consumers must already precede devices.

For missing-movie checks, stage once with `--stage-only`, then move only the
chosen **staged copy** aside and run the printed rebuilt-launch command on the
user desktop. Rerunning the normal staging launcher restores absent movies and
invalidates that test. Do not relax immutable-file protections or touch original
movies. Record exact omitted files and log progression; restore staged copies
before the next normal staged run.

Research validation:20 new successful read-only/noanalysis exports; a transient
verified vtable definition was used for00647EE0 and discarded. Two export requests
stopped at absent definitions (10072400,00647EE0); existing IShow export and the
subsequent guarded render export supply those boundaries. Saved research program
state was not changed. Static map and mutations, independent asset checks and a
temporary native sanitizer audit pass. No new runtime source, function ledger
credit or desktop verification is claimed by this research.
