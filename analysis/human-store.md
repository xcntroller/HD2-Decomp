# HumanStore loading on the original launch path

[`frontend/human_store.h`](../recovered/frontend/human_store.h) reconstructs the
application's soldier store load before movie-state creation. It uses the
original [`ChunkFile`](../recovered/io/chunk_file.h) behavior and loaded
localization cache. [`host/human_store.h`](../recovered/host/human_store.h)
forwards portrait loads/releases to checked original LS3DF methods. These are
source-only components; the live preview and original game files are unchanged.

Application preparation 006B65D0 calls GameData 006BB540, sets the application
+04 object's +E8 flag to one, then loads `tables\HumanStore.def` through
006A18E0. Both loaders run even when GameData fails; the result is their byte
AND masked with one. Normal controller startup subsequently calls 006B6620
(profile scan/selection) before creating the opening movie state. This slice
recovers HumanStore, not that entire application sequence.

| Original entry | Recovered behavior |
| --- | --- |
| 006A18E0 / 006A1B40 | File open, raw root/record entry, reset, append/parse, optional overlay and close |
| 0069F930 / 0069FA60 | Forward destruction, portrait release, retained record-vector capacity, default-name reload |
| 006A1D80 / 006A1C80 | Default record creation/append and bounds-checked signed lookup |
| 006A00A0 | All recognized field formats, original traversal and finalization |
| 0069FDE0 / 0069FEF0 | Display-name and eye-texture derivation |
| 006A0AE0 / 0064D0B0 / 0064D0E0 | Stored-string UTF8 check with original validation rules |
| 006A0B20 / 006A0E10 | Narrow legacy conversion and conditional six-string finalization |
| 006A0EA0 | Portrait release/load with nonfatal failure |
| 006A0F30 / 006A0AA0 | Profile user-name overlay and assignment/finalization |
| 006E4FB0 | Localization lookup, including missing-index diagnostic |

## Store, records and field order

Missing main file returns false and preserves the existing store. Successful
open enters a raw root header without validating its ID, resets the store,
and loops until the current chunk ends or position lookup returns FFFFFFFF.
The root/record loops compare position for inequality, not signed remaining
greater than zero. Each raw record header is accepted regardless of ID; a
default record is appended BEFORE field parsing. After records, apply optional
profile names, ignore that boolean result, close the main file and return true.

Reset destroys records in increasing index order, releasing portraits before
record storage. It preserves vector capacity, then reloads eight default-name
pairs from the actual localization cache. Static initializer 0069F6E0 and
constructor 0069F790 establish first IDs 27A6..27AD and second IDs 2792..2799.
Reset looks up each first ID before its second. Missing IDs produce exactly
`__undefined index <ID> in all_txt __` with the signed decimal ID.

Original records occupy 74 hex bytes. Native records use named strings,
vectors, raw numeric words and a borrowed-interface portrait pointer; they do
not claim the original object ABI. Defaults are `First`, `Last`, empty nickname
and user override, `x_f_001.bmp`, derived `e_f001.bmp`, null portrait, zero +24
and +38/+3C/+40, rank FFFFFFFF, nine zero stats, +6C one and +70 flag zero.
The allocator-derived unknown +28 byte is not modeled. Native +71..73 padding
starts zero rather than reproducing uninitialized allocation bytes.

| Field chunk | Destination / behavior |
| --- | --- |
| 238C | Legacy combined name, then derive display names |
| 2396 / 23A0 | First / last name; no display-name refresh here |
| 23AA | Nickname, then derive display names |
| 23F0 | Face texture, then derive eye texture |
| 26DE | Raw word +24 |
| 26F3 / 26F4 / 26F5 | Raw words +38 / +3C / +40 |
| 2580 | Eight raw bytes at +6C, including flag/padding word +70 |
| 26FC | Append item word, preserving order |
| 26AC | Rank and nine signed integer stats converted to floats |
| 26AD | Rank and nine float words, exact raw bits preserved |
| 24B8 / 2648 | Older 72/76-byte integer layouts converted to current stats |

Both old layouts take rank from word zero and convert signed words
1, 2, 4, 5, 7, 9, 6, 13, 12 to the nine floats. This ordering was checked against
FILD/FSTP instructions, rather than trusting the decompiler's int-pointer types.
26AC initializes a four-byte scratch word once, ignores read results, and
reuses its possibly partially overwritten value on later short reads; source
preserves that behavior. Direct 26AD loads preserve NaN payloads too.

Unknown chunks are deliberately **not** left/skipped: the original loops at
their entered depth. A nested known field can therefore be read, and reaching
that unknown parent's end terminates record parsing. There is no invented
generic unknown-tag policy. Raw header entry and recognized-field leave retain
the existing ChunkFile semantics.

Full display name is `first last` or `first (nickname) last`. Short display
name is the first byte plus `. ` plus last, or last alone if first is empty.
238C captures the first space, opening parenthesis and closing parenthesis
positions before inserting terminators. This has visible quirks: `John(Nick)
Smith` assigns first `John(Nick)`, nickname `Nick`, and an empty last name,
because the first space has already been replaced by NUL. After `)`, skip only
one space when it remains present. Eye derivation lowercases a face copy in
the startup C locale and uses the suffix after the FIRST `x_f_` substring
anywhere in the string; absent match leaves the default eye texture.

## Encoding, portraits and profile overlays

Wide-language mode finalizes first, last, nickname, full, short and user strings
in that order. Empty strings pass. The old UTF8 predicate accepts ED surrogate
encodings and unusually permits F4 second bytes below 80; it rejects F4 second
bytes above 8F, overlong E0/F0 encodings and five/six-byte sequences. Preserve
these checks rather than replacing them with a modern validator.

On failed validation, 006A0B20 copies ASCII and converts only ED to C3 AD.
Any other high byte aborts conversion without changing the original contents.
Fields are converted independently; no display-name recomputation follows.

After text finalization, release any previous portrait, obtain fresh GetIGraph,
and call graphics +3C with face, `x_fc_a.bmp`, flags4 and the portrait output
slot. Negative status clears the output; nonnegative status can leave it null.
The parser ignores this result. Concrete checked RVAs are GetIGraph 68F90,
texture load 69F70 and texture release 73760, all in the original LS3DF module.
These implementations remain delegated and were not agent-executed.

The caller must provide the actual current-profile directory from 006B7100,
including its trailing separator. That resolver includes wide-profile mapping
006AE530 and is still unrecovered; this slice does not substitute a guessed
profile or claim profile selection is finished. Append `usernames.sav`.
Missing file is successful optional absence. A present file requires root029A;
each child's tag is a zero-based record index, with a nested 23B4 string.
Invalid index/tag returns false, retaining earlier overrides. For each valid
entry, leave the name chunk, assign +14, finalize text, then leave the index
chunk. No derived-name or portrait refresh. The outer store ignores overlay
failure. Overlay iteration uses signed remaining greater than zero.

## Native limits and verification

Native strings/unique ownership replace original allocation/refcount/SEH;
original files and graphics modules must outlive their borrowers. Callbacks
must not throw or structurally mutate the store. Explicit guards throw on
failed raw headers, nonpositive/unterminated string payloads, original 128/256/
1024-byte name buffers exceeded, and undefined short numeric reads outside
the initialized 26AC scratch path. Original unchecked overflow/uninitialized
reads are not emulated. Native exception cleanup closes active files, while
already appended records and applied overrides remain visible. Original FPU
setup and application owner layout are outside this slice. All entries partial.

Temporary ASan/UBSan audits compare all 65 installed HumanStore records against
an independent Python raw-offset decoder: eight strings, +24/+38/+3C/+40,
all ten stat words, tail defaults and item lists. They also cover all four
stat formats, short 26AC scratch reuse, ignored root/record IDs, unknown nested
traversal, name/texture derivation, optional/valid/invalid/partially applied
overlays, missing-file preservation, localization defaults, portrait failures,
null successful outputs, forward releases, malformed-input cleanup, every
two-byte UTF8 input and selected three/four/five-byte boundary cases.
The native audit loads real English text through the recovered cache parser.
A strict i386 fixture instantiates the actual ChunkFiles and HumanPortraits
composition and links successfully; it is never executed. Temporary fixtures
and extracted assets are removed after verification; no permanent tests added.

The fingerprinted [map](human-store-map.json) checks both binaries, 37 byte
anchors, 217 direct calls and two original texture vtable slots. This includes
18 complete original body ranges as evidence, not a claim of native ABI
equivalence. Eighteen new partial client ledger entries bring totals to 185
(180 client/five engine), zero complete, frozen denominator 18,783. No delegated
DLL or current-profile-resolver function credit.

Run `python3 -B tools/validate_engine_host.py --map analysis/human-store-map.json`.
Next on this same launch chain: actual GameData loading, current-profile path
and profile scan/selection, then remaining application/state services,
protection-state effects and controller integration. The independent original
launch milestone remains open; there are no successful placeholder services.
