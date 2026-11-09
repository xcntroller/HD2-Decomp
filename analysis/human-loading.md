# Human activation dependencies — in progress, 2026-11-05

Continue full P2.2. No Human activation or containing world has been published.
The [derived activation contract](derived-actor-activation.md) records verified
Null/Spawn/Switch/Ladder and retirement source owners; other families still reject.

## Speech definitions and lifetime

`speech_definitions.h` reconstructs005AFE00's lazy definition load/cache and
005B0340/005AF880 defaults,005B0570 phrase insertion and005B0390 release.
Actual path is **Tables\\speech.set** (string0083DD7C), not the decompiler's
underscored symbol name. `GameSpeechDefinitions` opens through the SAME startup
ChunkFiles and borrows the shared client CRT random stream. No private seed,
archive resolution or successful audio receiver is installed.

Only first root1000 is consumed. Set selection is positional; entry IDs and
phrase IDs are ignored. Set members0/1/2 carry name byte count, ignored name,
and categories. Each category below138 owns actual uint32 sound-definition
words; unknown categories skip. Flag18 becomes true only when an actual phrase
is read. Native F24 record: category array starts08, stride1C, usageF20.
The name is read but never published by the original loader. Native guards
reject incomplete chunks/words and name counts beyond its128-byte stack buffer.

005B0570 consumes one actual CRT rand per phrase, even the first. Insert index
is cursor + rand%((size−cursor)+2); an index beyond the last element appends,
so two random outcomes append. The current cursor index survives vector changes.
No independent random generator is created by the production owner. Replacing
multiple data members preserves visible last-record behavior and frees the
original leaked intermediate allocation as a native guard.

A cache hit increments usage without file reads or random draws.005B0390
releases one usage, retaining positive counts; last release swap/pops the cache
membership and destroys the matching definition. Unknown native handles are
ignored rather than dereferenced/freed. Actors must release definitions before
the containing cache dies. This owner supplies definitions, not speech playback.
0059EF80 only lazily ensures the global speech owner; it does NOT release sound.
005AFD50 is a genuine RET. Human's getter fallback to index0 remains part of
its containing activation, not a success default in this cache.

Read-only base others.DTA extraction yields16 sets,1697 phrases; default set0
has130 empty categories. This is a base fixture, not a claim of runtime overlay
equality. Fingerprints: speech.set01df8aeb08aac5b3d7f51391ec5dd6c1cab7f2d7ed162d242022e7e80b72f360;
speech.txta7ed655732b13be18b2e40cf0c93a2e9ed3496c090445631c1b4c315d0e15de8.
The source opens the actual selected table each cache miss.

Focused ASan/UBSan passes all16 base sets/1697 shared-random draws, cache reuse/
reference teardown, insertion/cursor boundaries and eight truncated inputs.
Strict i386 instantiates the actual host resolver with a mandatory random source.
Application compile/link passes; no original or Windows executable was run.
Original reader assembly and exports: human-speech-reader.asm,
human-speech-definitions/, human-speech-record-defaults/ and human-speech-release/.
The failed005AFDB0 export is unused; actual matching record destructor called by
005B0390 is005AFDC0. No behavior relies on an invented function boundary.

## Model/control boundary next

0059F5A2…0059F5BA proves005AB330 takes ECX=Human98 inline owner, with Human
pointer and property1A0 as two stack arguments. Its A0/A4/A8 pointers are real
DD0/E0/230 control owners; the AC byte is separate.005ABA90 first calls0078DE30
and00786030, frees E0, destroys/frees DD0 through00785950, destroys/frees230
through0078CA30, nulls all three and Human98. Recover complete dependencies
before replacing the cold source's neutral words.

007855C0's DD0 control owns6950 geometry0079B910, shared20-group SOUND pool
00784B70 with global use count0098A694, a native queue, and its actual state
records. Constructor helper00785940 is a genuine identity/RET;007858E0 clears
specific flags/state words.00780990 initializes its own separate queue owner.
Shared sound resources, live skeleton/bone binding, scene13C registration,
collision/query setup and initialization00785B20/00785BE0 are required, not
successful optional callbacks. Shared pool selection/release exports are under human-model-shared-pool/;
the directory name reflects the earlier, disproved model-pool interpretation.

0077DD80/E0 initializes0077DDC0 and0077FD40 state, then0077DDA0 borrows the SAME
DD0 control and its258 subobject.0077FC30 dispatches actual007AC780 behavior;
it cannot be replaced by clearing a flag alone.230 equipment owner0078CA00/
0078CA60 has real item/model and callback receivers. Original sources and
fingerprinted exports are under human-model-control-owners/ and
human-control-cold-lifetime/. These bodies are research, not new implementation
or function credit. Model simulation/controller branches remain outstanding.


## Shared Human sound pool now implemented

00784B70/00784C40/00784F60/00784ED0 and DD0 constructor/destructor use-count
ownership now have source owners in human_sound_pool.h. Original0084E938 has
20 groups×six16-byte records,40 nonnull variants; each group stops at first null.
The exact name/range/volume words are in human_sound_definitions.h and preserved
human-sound-pool-definitions.json. This is a SOUND pool, not an animation/model
pool. Its pointer-table type was resolved through actual kind4 factory calls and
wav resource names; no model behavior relies on the earlier interpretation.

00784C40 initializes ALL groups once, consuming40 shared CRT random draws with
native cursor/window insertion. Its global initialization latch/windows outlive
pool deletion. StartupSession now owns that process state alongside existing
animation owners. Current selection/advance producers remain separate; no fake
picker or private random seed is supplied.

The pool constructor prewarms one sound per variant by actual take, then disables
and caches it. Each variant has at most two native cache slots; this loading path
fills one. take first checks actual ISndGetDriver+0C; unavailable returns null.
Cached take is LIFO, resets94=0, enables30=1 and applies volume8C. Fresh take
creates kind4, loads through the SAME existing sound cache with original0 flags
(cache adds10), sets7C=1, volume8C, ranges80(near,far,.5,.5),90=0/94=0, enables
it, and tags frameEC=(group<<8)|variant. Missing optional sounds release the actual
frame and return null, preserving80000009 diagnostic suppression. The native
host adds a stderr diagnostic for other load failures.

PoolManager leases implement shared-use acquisition/release: first use constructs
the pool, each control retains a lease, last release destroys cached sounds.
A later first use creates new frames without reshuffling persistent variant
windows. Source guards prevent reference overflow/underflow and clean partial
construction; the original destructor's create-then-delete on an impossible
missing-pool/unmatched-release branch is not manufactured. Live Human controls
must release their own sound frames before the shared lease and manager.

Focused ASan/UBSan passes all40 original variants, cached/fresh parameters and
ownership, shared lease/repeat lifetime, unavailable/missing sound cases and13
endpoint failures. Strict i386 instantiates the actual GameActorServices adapters.
Affected application compile/link passes; no sound device, DLL or desktop run.
Selection, return-to-pool and playback consumers remain to recover before adding
those producers. The actual DD0 model/control owner still requires integration;
no world readiness or function-ledger credit follows from this pool.

## Ordinary skeleton/model geometry source

The23-bone graph, loading joint/body/point trees, ordinary geometry/camera work
and look-command state now have source owners and focused native verification.
[Contracts, evidence, limits and remaining live controls](human-model-geometry.md).
Human98/DD0/230 and the containing world remain unconstructed; no new function
credit or desktop claim. Continue movement collision queries, script/state
receivers, physics/animation setup and equipment holster loading together.

Latest source continuation: DD0/AA0/C00/shared presentation, inline98/230, actual inventory/equipment services and dubbing now have source/native/i386 verification. [Current composition contracts](human-control-loading.md). Earlier remaining-work statements above record the previous boundary. Human cold fields are still neutral until actual B0/Sight/VM/world consumers are reconstructed.
