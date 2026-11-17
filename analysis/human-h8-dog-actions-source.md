# H8 Dog Body action and animation requests — 2026-11-17

Actual `DogSceneActor` owns `DogBodyActions`, borrowing its SAME current/five
`DogBodyOrders` records. The clip forced-motion value has its own actual producer.
This completes the action policy source, not Dog activation, a live controller
or clip mixer. H8.2/H8.4/H8.5/H8.6 and the HUM matrix remain open. No desktop or
application execution is claimed.

## Native contract

Image SHA256 `8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Research index lookup preceded the work; existing Dog exports were reused.
Aligned raw in WORK=`analysis/work/mission-world-20261103` disambiguates signed
priority/return ordering, Body versus clip bases and the sound destination slots.

| Original body | Source boundary |
| --- | --- |
|005C5030|Full stance/posture/action command composition, at most four commands, fresh queue metadata reads, priorities and native bark early returns.|
|005C8490|Signed priority admission; death uses INT_MAX, clears actual queue, ground setup, removes/free/nulls each of two descriptors, then publishes action5.|
|005C8540/005C8570|Posture/stance priority admission including equality.|
|005C4CB0|Current/first-record consumption, sentinel and no-controller-target skip, time/duration gate, required clip binding and actual action effects.|
|005C8880|Signed action transition-blocking predicate.|
|005C5BF0|Separate37C/380/384 voices, native stop/reset/emit ordering, actual animation2 reset and speed1.|
|005C4B50|Direct Body248 speed store.|
|005C88B0|Direction snapshot, required404 then3EC marker updates, actual swept-sphere ground query and death state/timer publication.|
|005C75C0|Clip100 vector,110 zero,114 duration stores; clip base is Body+10.|

005C5030 reads LAST queued metadata again after an enqueue; it does not read a
snapshot of the previous current record. Existing record provenance therefore
still rejects an unwritten new record at its corresponding native read. Stance
transition sequences, posture transition cases, four-command truncation and the
native bad tail-wrap behavior are preserved. Death variants use signed CRT
remainders; their metadata action remains13 even when the actual queued clip
is14..17. A bark request outside current clips7..12 discards the last queued
record BEFORE checking timer370; both early returns omit priority reset and
metadata publication. SOUND stop receives the actual mutable voice slot.

005C4CB0 copies all nine current words before removing first. If a sentinel23
or no-target5/6 exhausts current, the next first record is copied but deferred to
the next invocation. Only one such skip is attempted, as in the original.
Action6 substitutes5 at ordered controller-speed>=FLOAT1.8. Native signed time
addition wraps; current5 adds1200 EVEN to time-1 and forces blend400. It keeps
an unfinished blocking clip unless force36C. A NULL selected table clip still
consumes the command and clears force; a present clip gets native word8=1/2,
then the mandatory005C7430 receiver, then current/previous action publication,
fresh stride flag read and actual005C5BF0 effects. The mixer is not replaced by
a successful model-only animation callback. Native metadata is signed for the
FLOAT blend conversion. Invalid table/queue states remain required failures.

005C5BF0 always stops37C, sets it-1 and stores speed1 before dispatch. Action4
uses flags13/entry6. Category2 stops384 then380, resets380/168/384, clears actual
anim2 frame XYZ under its native static80 gate, resets Body170 XYZ and emits
flags5/entry5 INTO384. Category4's timer gate controls flags5/entry4 INTO380.
Categories5/6 use entries8/7 with flags1 and their distinct preceding stops;
those final return values are ignored. Raw5C5EA4 and5C60BA pin384/380 destinations
lost by the C export's local aliases. Undefined vector W is not invented.

005C88B0 snapshots Actor40 before frame callbacks, updates Body404 even though
its matrix is unused, then updates Body3EC. Query starts at that bone XYZ with
FLOAT Y+.5, displacement(0,-1,0), radiusFLOAT.2, excluded layersFFFFFFFF and
flags10000, with both native outputs requested. No hit copies the saved four
words through005C75C0 at Body+10 and duration800 BEFORE death374=1/timer378=5000.
A hit produces374=2/378=5000 and leaves forced motion untouched. Frame/query
failure preserves earlier effects without later state publication.

## Production provenance and remaining dependencies

`initialize_scalars` represents only known005C7D30 stores. It deliberately does
not invent278/27C/280 priorities or26C/270/274 admission results. The priorities
remain optional until a real producer writes them;005C5030's common tail
produces zero. Cold queue/Body/motion ownership does not publish readiness.
The three optional priority words and queued metadata are still a targeted
activation risk; no broad absence-of-writers proof or new D11 default is claimed.
Read-only007E4F62 raw confirms aligned allocation has no local zeroing; its malloc
callee was not reclassified into a zero-producing service.

Live Body geometry/clip initialization, controller24/28, native005C7430 mixer,
frame/SOUND owners and activation/retirement still must compose. The new
algorithms require those actual callbacks; no success defaults are supplied.
Dog D4/melee, frame and activation guards stay in place. Type36 read-only owner
research is conditional and supplies no new implementation or credit here.

## Verification

The temporary strict native ASan/UBSan/leak fixture passes4575872 checks and
compares100000 generated
states against the existing005C5030 C export, adapted only for native host pointer
width, standalone call signatures and explicit SOUND endpoints. Native queue
records/indices, command words, priorities, timers, voice handles and RNG/audio
ordering match. All five retained record metadata values are explicit fixture
inputs; this is not a claim that native activation produces them. Separate
checks assert the unproduced-priority and fresh-enqueue-metadata failures.

11040 playback cases use independent transition/consumption expectations,
including time-1, signed wrapping, current5/forced400 blend, force, missing clips,
controller-speed threshold/NaN, native next-record deferral and clip-binding
failure prefixes. All24 action-effect policies and death callback failures are
checked, plus marker/sweep arguments, direction snapshot and frame/query failure
prefixes. These are actual action/queue algorithms with explicit controller,
clip, audio, frame and sweep endpoints, not a loaded engine run.

Strict actual World/module authority+presentation i386 compilation/link includes
actual Dog storage and the new ground-death policy with SAME World frames and
collision sweep. It does not install a fabricated controller/mixer. Forced O2
launcher/startup-probe rebuild passes; no Windows artifacts run. Frozen bodies
receive partial source credit only, no delegated engine/allocator/absent-body
credit or complete promotions. Successful temporary audit source/binary pairs
are fingerprinted and removed after verification.
