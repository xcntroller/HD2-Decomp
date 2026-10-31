# Original menu settings and loaded-resource caches

The full settings/resource-cache integration checkbox is implemented for the
existing bounded launch. **User desktop-confirmed2026-10-31.** Main/Exit model
and click-sound loads now use real client-side cache owners and the existing
registered search directories. The common cache contracts and concrete engine
policies cover all five families. Unused families/options adapters remain
source-only; no new screen is enabled.

The earlier queue/update/capture milestone is user desktop-confirmed and
committed as `ce2a372`. Its acceptance does not establish this changed build.
The [mixed evidence map](menu-caches-map.json) pins both existing original
fingerprints,42 anchors,83 client/engine virtual slots and350 direct calls.
Prior [directory](resource-directories.md), [profile options](startup-settings.md),
[screen](menu-screen-foundation.md) and [shutdown](shutdown-preparation.md)
findings are reused without re-investigating their confirmed behavior.

## Completion boundary

Existing StartupSession already loads the selected profile's Options.bin and
applies its real gamma, graphics, sound/music, mouse and optional control values.
This change does not invent another profile loader/default set. The missing
boundary was the menu's concrete preference adapter and loaded-resource owners:
previously MenuScene directly opened `Models\\...` and `Sounds\\button.wav`,
bypassing the client caches, while original options observer services remained
supplied contracts.

The new normal application uses:

- LoadedResources with five distinct owners sharing the already populated
  directory lists; category1 for menu models and category2 for click audio.
- Original temporary driver content-load settings across model/text/control
  construction and initial selection, followed by original normalized restores.
- OptionsSettings resolving the CURRENT selected profile and calling concrete
  per-setting application helpers, without a menu-local preference copy.
- Actual controller/final cache cleanup before state deletion/engine closure.

The separate engine-window preview retains its accepted direct-load policy.
English800x600, bitmap font rendering, windowed graphics setup, unavailable
screens, broader application reset006B6530, registry/setup/protection and widget
construction remain their established boundaries. The current checkpoint does
not replace that application-data reset with an empty cache clear or enable it
as a successful service. Font/texture objects remain real separately owned
resources: original client loaded-cache families are not a generic texture cache.

## Cache identities, factories and copy policies

| Category | Original owner | Factory | Copy / ownership policy |
| --- | --- | --- | --- |
| 1 | 008AEC90 | Driver+54 kind9 | Model+d8 clone, enumerate destination children and clear pick word+ec; frame WORD refcount+e4 |
| 2 | 008AEC54 | Driver+54 kind4 | Reopen destination from source sound's filename/flags; frame WORD refcount+e4 |
| 3 | 008AEC18 | Driver+70 | Animation DWORD refcount+04; load-into006D5600 is verified zero-return body, copy006D5610 is RET8; getter remains a real loader |
| 4 | 008AEBDC | Driver+54 kind15 | 006D5920 clears destination child pick words, without cloning source; frame WORD refcount+e4 |
| 5 | 008AEBA0 | Driver+54 kind9 | Separate owner, same model clone/child reset policy as category1 |

All constructors initialize soft capacity+34 to256 and current scratch+38 to
null. Loaded entries are vector+18/+1c and tree header+08; search directories
are separate+28/+2c state. Category1/5 sharing code never means sharing state.

Concrete backend guards original I3DGetDriver10068FB0, frame factory10019E10,
animation factory10018D20, frame Release1001C370 and animation Release10004C80.
Retain/probe uses WORD+e4 for frames and DWORD+04 for animations, as the original
client does. Engine factories, model/sound/animation file loading, model cloning
and child enumeration remain delegated; no engine body is newly credited.

006D4F90 /006D5CC0 call destination+d8 with source, then enumerate destination
children through+44 with callback,context0,maskFFFF,name0. Callbacks006D4FC0 /
006D5CF0 clear each child's pick word+ec and return0. Engine clone10034760 cleans
8 bytes, enumeration1001DE40 cleans20; callback cleans8. This prevents cached
pick identifiers from leaking into independently constructed controls.

006D52D0 reads source sound+140, resolves that file owner's +0c COW string
(data+8, empty when absent), and opens destination with source flags+178.
0051C0D0 and engine10056150 establish the same layout. The copy-open status is
ignored by original code and remains ignored; the initial search status is not.

Category4's unusual no-clone policy is preserved. Its receiver was checked
against the instruction stack offsets during final review: it enumerates the
FIRST argument (destination), not source. Native backend audit explicitly checks
that source pick words survive. This is not a fabricated clone service.

## Loading, key lookup and eviction

The portable `io/loaded_resource_cache.h` reconstructs:

| Entry | Contract |
| --- | --- |
| 004290A0 | Model load into an existing frame; category1/5 |
| 0044A1C0 | Sound load into an existing frame |
| 006D5DF0 | Kind15 load into an existing frame |
| 004C76C0 | Shared-model getter |
| 006D7EA0 | Shared-sound getter |
| 0041FC10 | Shared-animation getter, open+04 |
| 006D64D0 | Shared-kind15 getter |

Load-into removes the last matching destination pointer from the cache BEFORE
lookup. Its old key's tree value becomes null; the vector erases that entry in
order. Keys use original NUL-terminated spelling and unsigned byte/length
comparison0040AA90. There is no lowercasing, slash normalization, extension
appending or LRU movement on hits. Tree null nodes persist on misses/evictions.

A hit copies the cached prototype into destination and returns0. A miss tries
registered directories in order, concatenating each normalized directory and
unchanged key. Directory base/count are reread for every iteration, allowing
callback mutations to affect subsequent searches. Load-into passes caller flags
OR10 and OR4 when its original callback argument is nonnull; its extra original
callback/context stack arguments are not forwarded into the engine model open.

Search starts with80000001. Status80000002 or8000000A returns immediately.
80000013/14 emits bad-version/time diagnostic and continues; other negative
statuses also continue. Nonnegative status stops search and is returned intact.
No directories is a genuine80000001 failure, not successful cache population.

After successful search, if size>=capacity, scan entries from the beginning,
retain then Release each object, and evict only the first whose remaining count
is1. Pinned objects may force growth beyond capacity. Capacity is not a hard
limit, there is no repeated pruning to a target size, and hits do not reorder
entries. A fresh prototype is factory-created, populated by the family copy
policy, stored with the key and indexed by a borrowed tree pointer.

Getters first clear their output, retain hits, and return0 on hit. A miss creates
or reuses current scratch+38, loading with flags0 or4 when a failure callback is
present. Ordinary exhausted search calls that callback once; the two terminal
errors return before it. On success, soft eviction runs, then replacement scratch
is allocated. Null replacement returns80000005 with old scratch retained. The
loaded scratch gains cache/caller references, its key is published, and the new
scratch is installed last. A failed load keeps scratch for another attempt.

Native RAII/containers replace original intrusive temporary/COW/tree/vector/SEH
ABI. Caller output ownership is safely released when replaced. Null initial
scratch/load-into prototype and allocation/copy failures throw rather than
reproducing original dereferences or incomplete null vector records. Publication
is exception-safe rather than reproducing each transient original insertion
state. These differences are explicit; no full original allocator/ABI claim.

## Settings from actual owners

006B6740 resolves application+20 then settings+38. OptionsSettings asks its
service for this current record on each operation, so profile changes do not
leave a stale cached pointer. Float offsets are mouse0,gamma4,sound8,musicC,
voice10. Byte offsets are voice-enable38,invert39,subtitles3A,parental-lock3B.

006B3380 stores only the low byte of a graphics feature in quality block
1C/23/2A/31 for detail0/1/2/other. It neither forces custom detail nor applies
settings. Existing006B33E0 lookup and the four options observers now compose
with this real preference storage.

StartupSession supplies the original apply helpers separately: graphics queries
driver twice then state2B; gamma uses the already recovered ramp; sound/music
use their distinct existing manager setters and float32 scaling; voice submits
the optional control value; mouse recomputes limits and copies invert byte.
Observers retain their established ordering and clamp/truncation behavior.
These adapters are compiled with the actual profile/input/sound/control owners;
unused options widgets are still source-only and not desktop-enabled here.

0066C5B0 saves driver15/19/11 as NONZERO Booleans, then selects0/0/(mode!=11).
The normal application uses mode10. The guard includes scene/model/text/control
construction AND initial screen selection, not just file opens. Restores happen
in15/19/11 order with normalized saved values. Native failure paths attempt all
restores and preserve the first load/set error, extending original success-path
restoration without pretending a rejected engine setting succeeded.

## Cleanup and failure paths

Each cache clears loaded references in forward insertion order, then lookup tree,
then scratch+38. Capacity/search directories survive. Controller006C0920 now
shuts down the state, clears categories2→1→3→4→5, then deletes state unless
reinitializing and closes mouse when enabled. Native exception aggregation still
attempts cache cleanup, state deletion and mouse close if state shutdown throws.

Final006DC3A0 order is1→2→4→5, after window persistence and before3D/sound/graphics.
The application final-owner path uses that order. A native failure fallback also
clears any category3 left without a completed controller shutdown, so acquired
resources cannot survive their driver. Repeated empty clears operate on real
owners and preserve directory registration. Cache destructors provide the same
native acquired-owner fallback, not a successful placeholder cleanup.

Original0066AD10 ignores click-loader status. The normal application now retains
an unbuffered click frame after cache load failure rather than turning missing
audio into a fatal menu failure. No buffer/success is invented. The diagnostic
preview retains its prior strict direct-open policy. Missing menu models remain
failures; frontend model helper0066BFA0 requires status exactly0.

## Verification and accounting

Temporary ASan/UBSan audit of the actual cache/settings/controller templates
passes registered-order search; all terminal/retry/diagnostic statuses; caller
flags; exact/case-sensitive/NUL keys; copy hits; retained output references;
soft-limit growth/eviction; removal before lookup; live directory mutation;
scratch persistence/replacement failure; copy exceptions; repeated forward-order
cleanup; controller failure cleanup; all five family clear orders; normalized
content-load settings in modes10/11 and every failed set prefix; canonical
selected-profile/quality/float/byte stores and real observer composition.
An additional10000 generated request sequence checks independently tracked
cache hits, live references and clear boundaries. The fixture also links for
Windows i386 without execution.

A second native sanitizer audit compiles the ACTUAL concrete
CachedResourceBackend/LoadedResources headers against supplied native engine
services. It checks all five factory/open/reference policies; model clone and
child picking reset; sound source-name/flags and ignored copy failure;
category4 destination semantics; real model/sound cache composition and clean
controller/final release. Native engine stand-ins are audit services, never
original DLL execution or a desktop claim. Temporary fixtures are removed;
no permanent tests are added.

All three strict Windows targets, the existing native suite and158 tooling tests
pass. New/related maps, mutation checks and frozen ledger pass. Ten eligible
client partial entries are added:346 partial (341client/fiveengine), zero
complete, frozen18,783. Absent bodies and delegated engine calls get no credit.
Read-only research produced31 new valid function exports across
`menu-loaded-caches-20261031`, `menu-cache-settings-20261031` and
`menu-cache-families-20261031`. One batch rejected a guessed overlapping address;
its eight valid earlier exports were checked individually and the nine-family
follow-up completed successfully. No original/project edits were saved.

## Desktop acceptance —2026-10-31

Built/staged application SHA256 (2026-10-31):
`75fb6276a000434619acbb4a28f213af3b1f5c9e02f037948ab02a71b1ea42fb`.
Both copies match. Staging preserves existing writable profile/config files;
the accepted841-frame runtime logs and window.cfg are archived with a manifest
in ignored `analysis/work/menu-cache-acceptance-20261031`.

Stage-only preparation does not execute Windows code:

```sh
python3 -B tools/run_application_launch.py --stage-only
```

User desktop command:

```sh
python3 -B tools/run_application_launch.py
```

Check movies/skips→Main/Exit, hover/text/artwork/cursor, audible pointer clicks,
No/Escape cancellation, keyboard Enter/Space/Yes, focus return and clean shutdown.
The new log should show model keys main_menu/confirm/kurzor_menu entering the
client cache, button.wav cache status/buffer state, controller clear2/1/3/4/5
and final cache release before3D/sound/graphics close. Native audits cover the
unused family/options paths; Main/Exit cannot desktop-establish those routes.

User reports “Works - record it, commit and continue”. The retained final native
log records successful main_menu/confirm/kurzor_menu cache loads, buffered
button.wav, confirmation cancellation/reopening, status4,841 frames/state-exit
and teardown result0. Controller cache clears and final release precede engine
closure. Visuals, audible clicks and focus-return gestures rely on user
confirmation; unused family/options paths remain source-only. The checkbox is
closed for its documented boundary; interactive acceptance precedes the requested
commit. No agent Windows/original/DLL/Proton execution or push.
