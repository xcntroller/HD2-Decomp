# Original startup archive registration and key exchange

[`io/startup_archives.h`](../recovered/io/startup_archives.h) recovers the archive
region of client 006DB6E0, the final archive cleanup loop in 006DC3A0, and the
arithmetic of CRT random 007E5F33. The [concrete DLL boundary](../recovered/host/startup_archives.h)
uses the original rw_data module shared with engine imports and DtaTextFiles.
This is source-only. No live host resources, rendering or startup calls change.

## Original startup position and explicit exclusions

006DBFC0 calls 006DB6E0 after graphics/sound/3D setup. Then 006DBE60 registers
resource directories and 006DBFB0 calls graphics input/localization setup.
Inside 006DB6E0, three client rand calls precede installed-language resolution
and `Lang%s.dta` formatting. An opaque protection-state operation at 006DB8C7
calls through 00A59000 and updates original state tables before archive creation.
That operation is NOT recovered, skipped by a fake success service, or supplied
by this component. Full-wrapper integration remains open.

The native API deliberately exposes the recovered regions separately:
`begin_archive_key_mask(random)`, `language_archive_name(language)`, then
`StartupArchives::create(name)` and `configure(mask, random)`. A future full
caller must preserve the original ordering and resolve the intervening state
operation. `next_client_random(state)` implements the exact unsigned recurrence
`state = state * 214013 + 2531011`, returning bits 16..30. Its state is borrowed
from the caller; TLS lookup, initial seeding and stream ownership remain open.
The DLL has its own internal seed generation, still delegated to that DLL.

## Archive slots, registration and fallback

| Slot | Creation name | Key, high:low DWORDs |
| --- | --- | --- |
| 0..4 | Reserved, explicitly null | A0A0A0A0:A0A0A0A4/A3/A2/A1/F1 |
| 5 | Lang + installed language + .dta | A0A0A0A0:A0A0A0A0 |
| 6 | LangEnglish.dta, only when slot 5 creation returns null | A0A0A0A0:A0A0A0A0 |
| 7 | Patch.dta | 10ACB252:5D805270 |
| 8 | Models.dta | 10ACB252:5D805259 |
| 9 | Missions.dta | 22BCDA98:7654A3F0 |
| 10 | Maps.dta | F26527FA:B438D0A5 |
| 11 | Sounds.dta | 8D2965CA:4FE85106 |
| 12 | Scripts.dta | CF761298:0AB4E72D |
| 13 | Others.dta | 65F7AB23:EA85902A |
| 14 | Maps_C.dta | E26520FA:B038D0A0 |
| 15 | Maps_U.dta | F26520FA:B038D1A1 |

Creation calls `_dtaCreate@4` in this exact order. It does not call dtaAddDir,
manually mount archives or reorder overlays. The DLL constructor opens the
archive and appends its records to the shared registry. A valid returned object
owns one reference; no additional retain is performed by the client. Missing
archives do not abort creation of later ones. Language spelling/case is retained
and `%s` stops at NUL. No expansion/PatchX01 archives are added to the base path.

Configuration traverses all 16 slots in ascending order. Each null slot emits
`# ERROR DATAFILE : datafile '<name>' missing`, including five EMPTY reserved
names and the intentionally unused English fallback when slot 5 succeeded.
This surprising original behavior is preserved, without treating it as fatal.

For each present slot, shift the 64-bit mask left 16 and append low 16 bits of
one rand result. Call object +0C with `compiled_key XOR mask`; read the complete
64-bit +10 reply in EDX:EAX; then call +0C with `reply XOR compiled_key`.
Both setter results are ignored, even when false. The first DLL setter stores
`internal_seed XOR incoming_key`; the second combines its incoming value with
that stored value to recover the actual compiled archive key and read/decrypt
the index. This is a two-call protocol, not an ordinary key setter. The native
source still performs both calls and the intervening query, including degenerate
zero-reply behavior, rather than replacing them with one direct key assignment.

## Actual lookup precedence and lifetime

The DLL dtaOpen body 10004B8C tests the LOW BYTE of flags and global override
100478DC. If both are zero, try loose files first, then archives only on -1.
Otherwise try archives first, then loose files only on -1. Archive search
1000C110 walks registration order from index zero. The ordering vector appends
at its end (1000FF10/10011230), so language archives precede Patch, which precedes
the base assets. These search/index/codec implementations remain delegated; no
native approximation of overlay selection is inserted. Calls share one loaded
rw_data module, not a separate private registry for text or each consumer.

Original cleanup closes the 3D driver, sound and graphics before releasing
archive slots in ASCENDING order. Each slot stays non-null through its release
callback and is nulled afterward. Releasing the last DLL reference unregisters
the archive, adjusts registry indices and closes its underlying handle. The
native owner performs only the archive phase; its caller must preserve the
surrounding shutdown order and close consumers first.

## ABI, native differences and remaining work

The borrowed module must outlive all archive objects. `_dtaCreate@4` is stdcall,
export RVA 1424. Object methods are thiscall: release +08 -> thunk 1181 -> 470D;
key +0C -> thunk 1073 -> 3763; reply +10 -> thunk 1384 -> 3D84. Runtime checks
compare the exact export and slot targets. Stable original vtables are required;
the original caches its getter's vtable for the second key call. The adapter
rechecks the same immutable slots. It does not own/unload the module.

Native arrays/strings/RAII replace original globals/refcounted string allocation.
Repeat create requires close rather than overwriting/leaking old objects;
partial successes remain owned on exceptions. Close is automatic on destruction
as an additional native cleanup guarantee. Backend callbacks must not mutate
ownership storage; releases must not throw. Diagnostics use OutputDebugStringA;
the original dbgPrintf transport is not reconstructed. No game DLL is executed
by the audit. The original protection prelude, resource-directory cache bodies,
complete graphics input services and other application/state startup services
remain before live controller integration.

## Validation and accounting

Temporary ASan/UBSan audit passes all 2,048 archive availability masks: creation
order, selected-language/English fallback, missing/unused-slot diagnostics,
set/get/set ordering, false setter results, degenerate zero reply, independent two-DWORD reference
mask arithmetic, exact random consumption, forward release with callback-visible
slots, repeated empty close and partial factory exceptions at every creation
position. Known CRT random sequence also matches. Strict Windows i386 fixture
instantiates the complete owner, concrete adapter and actual localized-text
loader against the same module and links successfully; it was NOT executed.

Independent objdump decoding matches all 32 original immediate key words to
the source array. Original keys validate 53,612 complete ISD0 index/member
records across seven installed base archives. LangEnglish and Patch are ISD1:
their decoded header bounds/counts and original 10009E97 checksums match (4,560
and 1,059 records); their member loading remains delegated, not newly rebuilt.
Maps_C is absent. Expansion archives are outside this check.

Existing native suite, 158 Python tooling tests and unchanged host build pass.
Temporary fixture sources/executables are removed afterward. The
[evidence map](startup-archives-map.json) and
`python3 -B tools/validate_startup_archives.py` verify three hashes, 24 instruction
anchors, six vtable slots, 47 calls and 22 resources. A temporary negative audit
rejects eight altered hashes/anchors/slots.

Read-only client exports live under ignored `menu-resource-setup` and
`resource-search-directories`. rw_data was analyzed in a separate temporary
project under `/tmp/hd2-rw-data-startup-20261030`, leaving original binaries and
saved projects untouched; ignored exports are `rw-data-startup-exports`,
`rw-data-search-exports`, `rw-data-lookup-exports`. Three new partial CLIENT
entries bring totals to 149 partial (144 client/five engine), zero complete,
frozen denominator 18,783. rw_data is delegated and excluded from that census.
