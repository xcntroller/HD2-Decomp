# Sound catalog and primary movie selection

Source-only recovery, 2026-10-30, following [startup owners](startup-owners.md).
[Fingerprint and instruction evidence](sound-catalog-map.json) covers both
original binaries, 45 anchors, 124 direct calls and eight engine slots.
The live preview is unchanged; original launch/controller delivery remains open.

## Implemented source

[sound_catalog.h](../recovered/frontend/sound_catalog.h) owns the catalog at
client00899838: loaded byte00, group vector04..10 and signed-key map14.
Global0089983C is its group-array pointer, not another catalog object.
Native strings, unique pointers, vectors and map replace the original container,
COW/refcount, allocation and exception ABI.

| Original body | Recovered behavior |
| --- | --- |
| 00439080 | Cached load, file precedence, root validation, nested loading, mapping and padding |
| 004398B0 | Reverse group destruction, map clear and loaded reset |
| 00432040 | Group name/entry parser; skip unknown group children |
| 00431F80 / 00431FD0 | Reverse entry cleanup and entry destruction |
| 0043A6F0 | Entry name/flag/variant parser; unknown children fail |
| 0043A5E0 / 0043A630 | Reverse variant cleanup; primary sample then reverse extra references |
| 0043AA50 | Variant fields; unknown children fail |

Constructor004394D0's fourteen category names/order are recovered from00838A8C,
including Music at index12. This constructor context, generic container helpers
and empty-group constructor receive no added function credit. Sample loading and
cache population are separate, unfinished consumers; the parser acquires no
sample references. Cleanup performs real virtual releases through the supplied
resource backend, which must outlive the catalog and all references.

## File and failure contracts

Open `Tables\IngameSounds.def` first; try `.dat` only if that open fails.
A successful open with a bad root never tries the fallback. Require root1000;
missing files or bad roots preserve previous groups/map/loaded flag. A valid
root clears old data before parsing. Root children1100 are groups,1050 are raw
DWORD key/value pairs; unknown root children skip. Map ordering is signed and
the last duplicate value wins. Root/pair loops use signed remaining **>0**;
the three nested readers use **!=0**, with seek-failure/closed-file termination.

Every ordinary noncached exit pads the list to at least fourteen groups, using
empty group names, including missing files, bad roots and parser failures. This
does not restore constructor category names after a valid root cleared them.
Completed groups survive later parser failure; the failed group is not appended.
Normal completion sets loaded; parse failure leaves it false. **Only the cached
early exit returns true. A successful fresh or forced load returns false.**

| Container | Accepted child tags (decimal) |
| --- | --- |
| Group1100 | 1110 name;1200 entry; unknown tags skip |
| Entry1200 | 1210 name;1300 variant;1310 raw flag byte; unknown tags fail |
| Variant1300 | 1400 name;1410 word04;1420 word08;1430 float0C;1440 float10;1450 float14;1470 float18 |

Variant1460 is rejected. Numeric defaults are100,0,20.0,0.5,1.0,0.5 in the
table's field order. Name defaults empty; entry name defaults `#name error`.
Entry/group reload clears children but retains name and entry flag. Failed
entry/variant tags remain active until caller/file cleanup, as in the original.

Original code ignores several read results and may consume uninitialized stack
words or scan an unterminated 999-byte string. Native guards throw on short
headers/scalars/names, missing NULs and backward chunks that can loop forever.
These exceptional paths close through RAII and do not claim original padding or
return behavior. Failed temporary objects are freed instead of reproducing
original leaks. Variant byte20 has undefined constructor provenance and uses
an unset optional value; no original default or consumer is invented.

## Primary movie selection

[sound_movie_selection.h](../recovered/frontend/sound_movie_selection.h)
implements004370F0 using the existing settings state, fade state and
[movie helper](../recovered/frontend/sound_movie.h). It loads the catalog without
a gate and ignores the result, stops/closes the retained primary movie, clears
pending6C and stores request4C before validating the request.

Negative/out-of-range requests and empty variants return after those effects.
Otherwise select an entry in group12 and a variant by unsigned CRT `rand()%count`.
Set fade target50=1 and current scale54=0, preserving rate58. Retain the selected
filename, call the looped movie helper with volume44 times scale54, then seek via
IShow+30 if the position is nonzero and the retained movie is opened. The seek
has no additional shared-gate check. Gate clear can therefore still select,
change fade state and seek an already-opened movie.

[SettingsServices](../recovered/host/settings_services.h) supplies real CRT
randomness, debugger diagnostics, sample virtual release and checked IShow seek
at100722A0. Sample release uses the supplied object's slot0 with self on stack;
type-specific factory/vtable validation remains open, and no target is guessed.
This is distinct from opening-state GameMovie ownership. Movie/codec/device
implementations remain delegated. Full manager registration/update/destruction
and runtime sequence are not represented by this bounded selection state.

## Validation and next work

Temporary ASan/UBSan audit passes for constructor data, both file choices,
bad-root preservation, cached/forced loads, duplicate signed keys, all variant
fields/defaults, unknown tags at each level, retained successful prefixes,
fourteen-group padding, reverse sample cleanup, short/backward/unterminated input,
and composed selection/settings/fade/movie order including disabled gate and
nonzero open-result paths. Real i386 DTA/LS3DF adapters strictly compile/link;
Windows fixtures are never executed. Native checks and all 158 Python tests pass.
New and three adjacent evidence maps validate; six altered maps are rejected.
Temporary audits removed; no permanent tests or original files changed.

Nine census-backed client bodies add partial credit: 261 total (256 client/five
engine), zero complete, frozen denominator 18,783. Selection004370F0 is absent
from the frozen census and receives no credit; delegated engine bodies receive
none. Next: full sound-manager registration/state/cleanup and update004368D0,
normal secondary-movie coordination and sample-cache consumers, then remaining
gate/control/application/state/protection/cache owners before live launch wiring.
