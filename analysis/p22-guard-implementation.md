# P22-GUARD implementation in progress

[Work plan](p22-guard-plan.md). Branch feature/p22-guard, base/fetched master
494cbee40566248084aaf9c8f01e7f75724d13c5. No other checkout modified, no merge/push.
Research/p22 was read through Git at40aea3ef4135a8443c986ec552942a239396686f.
The denominator remains18,783. Seven implemented scalar/predicate rows are added
as partial and006E9EA0 is extended; totals34 complete/2,507 partial. No whole-body
completion promotion. GUARD is **not source-closed**.

## Implemented initialization

The client SHA256 is8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Original006DB0F0 calls006E9EA0 at006DB104 before graphics. The seed call at
006E9EBC returns to006E9EC2;00A59120 stores pair006E9EC2/0080F1B4;
0080F1B4 contains import name RVA004303A6, whose hint/name is GetTickCount.
Read-only PE bytes were used; no original execution. Existing fingerprinted
006E9EA0 C/JSON retain the two independent permutations and seven draws per
iteration. The original scalar owner is preserved without a new table or RNG.

StartupSession::begin_device_attempt validates control_ready, writes both identity permutations, then reads the actual
Win32 tick DWORD once via initialize_from_seed, seeds the SAME std::rand CRT via seed_random and initializes
its existing MissionIntegrityTables. launch calls it after before_devices and
immediately before graphics acquisition. It is callable again for device retry
while control_ready; no mission calls/reseeds were added. Successful ordinary
startup now consumes the896 draws before subsequent archive/sound draws.
The existing table initializer invalidates access before mutation, and marks
initialized only after the final draw. Source exception guards retain this
failure gate; native CRT calls themselves do not throw.

## Other implemented bodies

`game/integrity_descriptors.h`:00994810's first-match search across32 descriptor
records, active bit, optional second/third comparisons, selectors0..3 and10..14,
including12/16 overlapping caller-frame windows. Inputs are original DWORDs,
not truncated native pointers. Results retain all three matched words and slot;
no-match is32.009FB050 uses nullable two-DWORD state, unsigned shift and low-five
slot bits. Neither body asserts validation or publishes observations.

`game/integrity_configuration.h`:00A0DB60's null branch, magic DWORD44646441,
low-byte masking of DWORD4, unaligned DWORD56 XORACDDDDCA and byte offsets8..49
XOR index<<(index%3). It transforms an actual supplied84hex block; it does not
supply a default configuration or bypass009F0A60 digest/flag validation.
These leaf bodies are not yet bound into the process producer graph.

## Evidence and remaining investigation

New read-only/noanalysis Ghidra exports: ignored WORK/export3 contains009F0A60,
009F02A0,009905B0,009995F0,009F8C60,009F8D40 and0099AEB0. helpers2 contains
009FFBB0,00A0DB60,009EA5B0 and009ED0D0. Inspect metadata hashes, instruction
arguments and warnings, rather than treating decompiler signatures as ABI.
0099AEB0's export has numerous unreachable-block warnings and incomplete call
arguments. Retained objdump windows are in WORK. Early export attempts failed
and a helper attempt encountered a transient project lock; subsequent unique
output exports completed. No database edits or original code execution.

009905B0 writes initial record scores and changes them according to actual probe
outcomes;009995F0 selects indices under record-score/status gates. Its allocation
is not an initialized/valid observation.009F02A0 uses ordinal-style callbacks
with actual device/session output pointers; its length and accepted-value checks
must be retained.009F0A60 reads the executable, supports embedded/resource
configuration, decodes84hex bytes and checks a digest/flags relation.

Important distinction: dormant wrapper code00997648 calls00994DF0, tests EAX
at00997650 and branches to fallback on zero.0099766A checks009FB050 and
009976F3 calls00996330. However, the actual matched-descriptor path at00997602
contains `E9 51 00 00 00` (JMP00997658), skipping validation entirely. The earlier
claim that the current wrapper validates every action was incorrect; retained
instructions contradict it.00996330 itself is only the action dispatcher.

Private registry string records00ABD9FC,00ABDA3C,00ABDA48,00ABDA78 and the related
second-query records, private import slots00B5A798/00B5A788, default block00A96270
and library-name owner00B4FA60 contain zero bytes in the pinned file. Need their
actual upstream population, not an empty-string registry query. No external
blocker is established by these unresolved GUARD-owned initialization semantics.
PE entry00996A05 sets selected wrapper imports and tail-jumps through00A59004,
which stores007E8EFD. Private CRT entry00A4D782 calls0099AEB0 at00A4D84B; its
relationship to actual startup still requires investigation (including indirect
entries). Neither direct entry alone establishes successful private startup.

## Verification limits

Native ASan/UBSan/LSan passed for four seeds, all arrays/permutations and896
ordered draws, subsequent shared-stream draw, raw API/observed distinction,
interrupted draws1/2/7/450/896 and retry. RNG is a Microsoft CRT arithmetic double,
not execution of original CRT. Matcher checks cover every selector, last scan
window, optional fields, first-match ordering, inactive entries and blocked bits.
Configuration checks compare all256 fill bytes with an independent DWORD model.
Temporary i386 headers compile/link and actual application-launch compile/link
passed with -Wall -Wextra -Wpedantic -Werror. Windows artifacts were not run.
Seed-read ordering/throw/retry was rechecked after adding the provider overload.
No full concrete positive GUARD/World composition, validation/action/publication,
private producer, thread override or actual engine/desktop verification is claimed.
Application loading remains disabled. No permanent tests added.

Current consumers rechecked: GameLoadingQueries already owns GetParent,
IsWindowVisible, IsIconic, activation GetTickCount/GetCapture and finish GetFocus.
Its seed comment already identified006E9EC2 as GetTickCount; the new binary lookup
independently rechecks this existing finding. These owners must be extended,
not duplicated. Human guards read stages39 (conditional obstruction item) and41
(offline mode zoom), and objective failure compares stage13 with a mismatch-only
CRT draw. World constructors already pass StartupSession's SAME tables.

Further upstream exports009ED500/009ED7B0 resolve loaded-library and procedure
owners through encrypted strings at00AB0520 onward, also zero in the pinned file.
Private CRT's initialization tables00A5925C..00A5929C are zero in that image.
The initial CRT007E8EFD reaches006DD550. This narrows the upstream relationship
but does not prove the absence of indirect/dynamic population. Continue original
startup investigation; no missing external subsystem is established yet.

## Confirmed reference-data blocker

The supplied installed base-game input has SHA256
`e015f149b860d99e866a1ae71a7688c2691de00fe81a0f1ec9e796eb44dde2b3`.
To rule out stale/modified exports, restore_upx_metadata.py was run on that input
into a NEW /tmp copy, then UPX5.2.1 decompressed the COPY. Output SHA256 is exactly
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
No original EXE/DLL code was executed. Original source/input stayed untouched.
Thus the bypass is present in the supplied original reference, not introduced
by the current worktree, decompiler or stale ignored export.

Required real-validation/private-environment equivalence cannot be established
from a bypassed path plus zero-valued private string/configuration records.
The exact missing evidence is intact pre-bypass protection data/startup or an
independently fingerprinted retained image/exports establishing its population:
registry records00ABD9FC..00ABDAFC and imports00B5A798/00B5A788 for009F8C60/
009F8D40; embedded block00A96270/flags00A62BF8 for009F0A60; library/procedure
records00AB0520 onward and ordinal producer endpoints for009F02A0/009905B0.
These are needed to bind real platform keys/arguments and original producer
results. They cannot be replaced with empty strings, successful doubles, zero
records or a port of00997602's bypass under the user's required contract.

This is a confirmed reference-data conflict, not a claim that any arbitrary
unresolved GUARD function is an external subsystem. Dormant code recovery has
continued independently; matching, blocked bits, decoding and initializer are
implemented. Full source closure and positive concrete producer/publication
verification remain blocked on that intact reference evidence. No broader
source closure or application success is claimed. No source checkpoint commit
has been made; the larger GUARD subsystem checkpoint remains unfinished.

## Resumed scalar checkpoint

The prior stop treated zero file bytes too strongly as proof that runtime
population cannot be recovered. That inference has been rechecked rather than
used to skip GUARD work. New read-only references show reads/address loans for
00A96270,00A62BF8,00AB0520,00ABD9FC and private import slots, with no direct
static writer among the returned references. This does not exclude indirect
population. The sibling Sabre image has the same private section layout and
only542 nonzero .dogui raw bytes / zero nonzero .idata raw bytes, like the base
image. Its fingerprint is recorded separately and establishes no base-client
semantic equivalence. No additional intact data source has been found.

Implemented independent work:

- `integrity_validation.h`: scalar11/20/21 predicates borrowing actual private
  inputs. Unsigned counter storage preserves wrapping; the selector preserves
  signed negative remainder and explicit forced selectors.11 checks all four
  actual selected score indices or the length-minus96hex relation.21 performs
  its required query calls on branches0/2, exact flag==1/WORD checks on branch1
  and actual unconditional branch3. Missing configuration/indices and invalid
  indices raise explicit source guards. These are predicates, not the full
  00994DF0 body: clock/debug side effects, validation2 and remaining cases,
  real registry keys/imports and containing process binding remain open.
- Validation20 keeps its normal always-zero selector, forced override and actual
  configuration gates; it invokes the required checksum receiver otherwise,
  retaining cached WORD00B4C8D0 and the comparison with00B4FFEA. No checksum
  callback is installed in production and no successful default exists.
- `integrity_checksum.h`: actual00A0D960 C0C1/4003 table recurrence and00A0DAF0
  WORD accumulation over supplied bytes. Byte-table entries independently match
  the reflectedA001 bit oracle; zero-seed `123456789` yieldsBB3D. The checksum
  file-open/seek/read body00A0DA00 is not replaced by this byte accumulator.
- `integrity_observation_actions.h`: validated/unblocked scalar actions0/1/2/3/
  60/61/62/63.62 writes the matched word to the actual supplied destination;
  02 records override and03 records XOR against actual later API EAX. Rejection
  and blocking leave destination/return state unchanged. Non-scalar actions and
  fallback32/33/41 explicitly reject their absent concrete receiver; active
  descriptor02/62 fallback has its native no-op behavior. Actual thread-slot/
  caller replacement, native continuation return and exception-loan ownership
  remain required; call-local scalar state is not a production TLS substitute.

Focused native sanitizers cover negative/positive scalar predicates, forced
selectors, signed wrap, absent private allocations, exact==1 flags, bounds,
query/checksum exceptions, unchanged rejected destinations, override versus raw
EAX, XOR and checksum chunk composition. Query/checksum endpoints and private
arrays are native doubles/fixtures. Two local thread continuation values were
checked only as local values, not as a production per-thread owner. Strict i386
World-header object and scalar template compile/link pass; Windows fixtures
were not run. Actual application-launch link and27 existing tracker tests pass.
Temporary audits were fingerprinted and removed. Original data queries/exports
and concise logs remain ignored under WORK, with receipt hashes.

### Exact wrapped-call inventory findings

Read-only query enumerated all38 pairs at00A59008..00A59130 and the instructions
around their callers, with import names resolved through each original IAT slot.
WORK/all-wrapper-call-details.txt and wrapper-inventory.txt retain the complete
bounded table. This is static reachability/evidence, not selected-mission data.

| Call return | Underlying API | Stage | Validation/action | Existing owner |
| --- | --- | --- | --- | --- |
|006CA913|GetParent|117|11/62|GameLoadingQueries / mission loader|
|006CAAC0|IsWindowVisible|34|02/62|GameLoadingQueries / mission loader|
|006CAB29|IsIconic|39|21/62|GameLoadingQueries / mission loader|
|00698AAE|GetTickCount|77|21/02|GameLoadingQueries / activation|
|00698C09|IsWindowVisible|13|11/62|GameLoadingQueries / activation|
|00698C5D|GetCapture|5|21/02|GameLoadingQueries / activation|
|00699112|GetFocus|100|21/02|GameLoadingQueries / finish|
|006DC360|IsZoomed|41|20/02|Startup setup; Human zoom later consumes41|
|0067379F|IsIconic|41|11/02|Conditional preparation stock replacement006736D0|
|006DB8CD|GetFocus|53|11/62|Startup archive phase|
|006DBF31|IsZoomed|69|20/62|Startup directory phase|
|006E9EC2|GetTickCount|none|no deliberate descriptor tag|Initializer seed|

Other table callers cover graphics/device attempts, opening/video layout,
conditional menu/preparation and game/script/frame contexts. No populated
conditional branch is asserted to be required merely because it is in the
lookup table. The ordinary loader already requires validation2 as well as11/21;
startup feeding the existing Human consumer requires20. Thus11/21 alone cannot
close the requested boundary. Existing process consumers remain stage13
objectives,39 obstruction and41 Human zoom, over SAME StartupSession tables.

Instruction00995B1E PUSHes DWORD00B4FFF2,00995B24 calls009F7020 without consuming
that word, then PUSHes its result and file-name address00B51940 before the
three-argument checksum wrapper00A0D930. The decompiler's apparent two-argument
signature is wrong.009F7020 calls private slot00B4ED78(0) and reads returned
pointer+3C; interpreting this as main-module PE e_lfanew is an inference pending
exact import/data recovery.00A0DA00 opens via private CRT mode00B228CC, seeks
arg2 and accumulates up to signed(arg3-arg2) bytes in4000hex chunks. That mode
record is zero in the current image; no guessed fopen mode/API was installed.

Further artifact conflict:006DCAE0 contains direct expected→observed writes
for stages83/60/100 at006DCBB0/006DCBD7/006DCBF3 before the next wrapped stage90
call at006DCC2B. These are recorded in WORK/caller-observation-writes.asm; they
are not ported as a shortcut around the user's required validated publication.
The supplied image is therefore insufficient evidence for the asserted intact
private validation path, even though it still contains many recoverable bodies.
The exact remaining reference need is still intact original private constants/
imports/startup population. GUARD unknowns remain GUARD work; no missing external
simulation, UI or lifecycle owner is blamed for them.
