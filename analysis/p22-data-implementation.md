# P22-DATA implementation — COMPLETE

Source-only DATA boundary on `feature/p22-data`, base and fetched origin/master
`30b681c1f3cb9135d0916335920aa4d0b39569cb`. No merge, push, application loading
activation, original code execution or desktop acceptance. Other checkouts were
read-only. [Plan](p22-data-plan.md), [receipt](checkpoints/p22-data-20261111.json).

## Preserved implementation and evidence

The initialized-resolver capture lives read-only in the main checkout at
`analysis/work/startup-runtime/run/loading-inputs-90f74f210e9f4150a123e200e31f523a`.
All 29 entries in its `fingerprints.json` were verified, including its completion
manifest and 28 payloads. This is the existing user-run capture, stopped before
controller/game construction. Historical resolver bytes establish neither the
current installation nor loaded actors, runtime vectors, engine acceptance or
executed branches. The receipt pins the fingerprint inventory and every member.
Historical mode/language/configuration and individual selected archive origins
are unknown where the capture did not record them.

Research/p22 was read through Git at `19cdfde`; PACK at `31b8786` and UI at
`66889df` were inspected through Git only. Their unmerged implementation is not
used. Current base already implements the material Sight factor, Human contact,
particle/surface and damage/scoring work: old research claims that these are
universally missing are stale. DATA adds no alternate Human, packet or actor
owner and no general parser rewrite. No demonstrated captured-input parser
defect required a gameplay fix. No frozen function credit is added.

Original contracts remain the existing 006CA780 stage adapter, 006CBF30 scene
wrapper, 006CC760/006CD940 scene/object reader, 006D4280 check-table fallback,
006EE570 collision loader, 00783350/00783BC0 animation gate/decoder and 00428600
script binding reader. Research index lookup preceded the address research;
existing fingerprint-qualified candidates were locators, not equivalence proof.
[Loading](mission-world-loading.md), [scene ownership](world-scene-ownership.md),
[lifetime](p22-life-implementation.md) retain the behavioral evidence.

## Concrete capture composition

`ChunkFiles` observes its actual open/seek/read/close results after delegating
exactly once to rw_data. Opt-in `ResourceObservations` stores owning values,
never file, actor, state, frame or callback borrowers. It adds no reads, seek,
resolution, registry or cache. Resource names, flags, open/close outcomes,
reported size, short reads and failed I/O are distinct. SHA-256 is published only
when returned bytes cover a contiguous complete `[0,size)` payload; selective
or skipped reads leave the digest unknown. Zero-length opened files differ from
absence. Short reads can be natural EOF; only the containing parser/loader can
classify them as malformed or required failure. No archive origin is guessed:
the exposed rw_data API provides no selected-origin query.

The existing five `LoadedResources` caches record their actual ordered search
attempts/results and cache hits, without modifying cache/directory/terminal-error
policy. Their delegated engine opens expose no bytes: these events establish
neither a payload hash nor an archive origin. Cache hits are reported separately
from new resolver reads. No cache was duplicated or cleared for diagnostics.

`GameMissionLoading::load_observed()` calls the SAME loader over its owned
request/descriptor, cold World, process files/caches and selected configuration.
The ordinary `load()` entry also records selection and failures when process
observation scopes are installed, so the existing GameSessionActivation and
GameSessionLifecycle path can be captured without bypassing either owner.
Each `load_observed()` call creates a new attempt store; prior owning stores remain independently
readable. It records resource, descriptor kind, application mode, language and
configuration flag170. Existing per-stage adapters report entry/result/completion
and verbatim source exceptions. False and nonzero Scene2 results remain false/
nonzero. Native engine status bits are retained; source exception guards are
explicitly labeled as source exceptions, not native engine outcomes.

The `MissionSceneDocuments` branches report optional actors/sounds absence,
Scene2's original status, check2 parsing, and the exact check.bin fallback.
A present legacy check.bin still only closes; there is no fabricated legacy
decoder. An ignored different Scene2 root retains its original status0. Parser
exceptions still propagate, and the scratch/module failure tail keeps its order.
Required tree/actor/resource failures remain required failures.

`observe_runtime()` binds the same store to process files/caches for the caller's
actual activation/frame/finish scope; it does not execute those operations.
`snapshot_data_owners()` samples the real World allocations. `data_observations()`
returns owning storage and `write_data_observations(output)` writes escaped TSV
resource/event manifests, then a diagnostic completion marker last. Output
failure throws independently; callers must retain/rethrow any primary loading
exception when separately attempting diagnostic output. No diagnostic disk
write is inserted into the loader's failure path.

Observer scopes restore the previous store unless explicitly replaced; open
handles finish recording in the store selected at their own open. No observer
callback can read retired World data. Stores are bounded to 65,536 resources and
65,536 events, with dropped-record counts and diagnostic-loss flags. Capture
allocation failure is distinct from actual I/O failure and cannot replace a
required source exception. Stack observation scopes must retire before their
process owner; stored reports may survive it. Use on the owning game thread.

The existing user-operated `--capture-loading-inputs` flow now writes SHA-256,
outcome and unavailable-origin provenance, plus mode/language/configuration/
wide-character context. Tutorial and ALPS1 remain diagnostic candidates, not a
new selected request or a replacement for GameData eligibility. Original assets
are copied only by that explicitly user-operated capture flow; completion
manifest remains last. No new engine-backed capture was performed here.

## Static validation callable over exact verified bytes

Build and run, without an engine or installation resolver:

```sh
c++ -std=c++17 -O1 -Wall -Wextra -Wpedantic -Werror -Irecovered tools/validate_selected_resource.cpp -o /tmp/hd2-data-validator
python3 -B tools/validate_loading_capture.py '/path/to/loading-inputs-CAPTURE' --validator /tmp/hd2-data-validator > /tmp/hd2-data-report.json
```

`validate-selected-resource FORMAT FILE EXPECTED_SHA256` verifies the exact
payload before parsing. Formats: `scene`, `tables`, `animation`, `bindings`,
`gamedata`, `items`, `tracks`. The Python composition verifies the entire stored
fingerprint inventory and completion manifest before invoking the native tool;
duplicated logical resources, unsafe member names, symlinks and mismatches fail.
Historical absences remain historical absences with loader-defined requiredness.
Current installation, loaded populations and executed branches are explicit nulls.

The native tool uses the production bounded chunk reader, typed actor property
readers, Car COMP decoder, navigation parser/storage, animation table decoder,
GameData decoder, items.dat parser, track definitions and script-binding parser.
Script binding pair extraction is shared with production `read()` before its
unchanged actual driver-frame filter; no successful fake frame is supplied.
Animation references are explicitly unresolved, and navigation frame/actor names
are recorded with null resolution. These are static endpoints, not engine doubles
that establish loading success.

Static scene analysis reports authored actor/property families, Human state
fields, equipment IDs, model/sound/animation names, geometry/navigation dependency
records and known unsupported actor/AE00 records. Door/Switch callbacks report
STATIC required portal/animation effects; they do not execute successful effects.
Conditional occluder/projector/sector records remain conditional on real frame
and visual types. Unknown chunks retain original skip behavior. Scope is chunk
framing and these property decoders, not full engine Scene2 acceptance, collision
mesh fixups or all object-field semantics. Unsupported required static records
exit nonzero; malformed/truncated inputs and fingerprint mismatches retain their
exact diagnostics. Retained trailing bytes intentionally ignored by original
readers are not universally called malformed.

GameData00 reports mission `text24` resources, captions and flag2c; text28 is the
briefing image, not the mission resource. Numbered later GameData files, profile
and selected mode eligibility are not inferred. Script bindings declare names,
not compiled VM instructions. Item and SupplyBox definitions do not prove
catalogue/model/spawn acceptance. Tree/effects/model payloads outside the declared
static parser scope are fingerprinted and explicitly unvalidated.

## Population and execution bindings

Source bindings address SAME existing owners and actual execution points:

| Observation | Concrete binding | Evidence when invoked |
| --- | --- | --- |
| Actor families | GameSceneActors initialization/property read and identity snapshot | Completed initialized/decoded owners; current identity population |
| Runtime family | GameRuntimeActors::tick | Actual active actor dispatch attempted, authority/presentation distinct |
| Human B0/DD0 | HumanWorldLoadingServices::advance_human_state | Actual B0 kind and DD0 field184 at frame entry |
| Material/contact | sight_surface_factor | Actual DD0+754 material at GEffects94 evaluation |
| Commands | publish_human_frame_commands | Actual family publication attempted to request/broadcast stream; no claim of remote execution |
| Source failure | loader/document/module frame wrappers | First chronological exact exception plus containing stage/module; original exception rethrown |
| Particles | actual module17 emit/place publication | Positive allocation/reuse published in actual owner |
| Shot marks | actual surface/blood owner publication | Positive published owner record; allocation/reuse distinction not inferred |
| Geometry/render | area geometry construction and successful volumetric primitive draw | Actual successful engine endpoint, if invoked |
| World snapshot | existing scene/runtime, ShotMgr vectors, particle process and GEffects volumesC4/clock224 | Populations at that instant; volume field2C float bits and particle counts |

Module frame tracing preserves original dispatch order; snapshots do not execute
missing receivers. An entered frame or command attempt does not imply completion.
A constructor/property census does not imply active B0 state, DD0 action, contact,
shot, script, geometry or rendering reachability. Cold vectors are not evidence
of future inactivity. Source bindings were compiled, not engine-executed here.

## Remaining external boundary

P22-DATA is complete for its implementation scope, per the user's scope
clarification. Remaining integration and live evidence belong to the external
components below and do not keep DATA open. OWN must install the existing lifecycle/loading/activation graph;
FRAME must supply its actual Runtime and containing session scheduling; RENDER
must supply positive visibility/procedural rendering and camera/history owners.
The current GameApplicationState still deliberately rejects loaded activation.
A future authorized caller can wrap the existing lifecycle operation directly:

```cpp
auto observations = std::make_shared<hd2::io::ResourceObservations>();
{
    auto files = startup.pre_mission_files().observe_scope(observations);
    auto caches = startup.loaded_resources().observe_scope(observations);
    // Existing lifecycle.activate()/actual frame/finish calls remain unchanged.
    // This scope itself executes none of them.
}
hd2::io::write_data_observations(*observations, output);
```

UI/PACK remain their own workstreams; this branch does not assume their unmerged
receivers. Reached external failures must identify the selected resource, actual
producer/owner and branch from these records, then be fixed by the owning stream.

Tracks playback, HeavyFire positive producers, procedural resources and relevant
camera/history have no claimed successful runtime observation here. The existing
cold source alone cannot establish that they remain inactive. Fresh active
resolver inputs and engine-backed actor/B0/DD0/contact/command/allocation/render
observations remain unknown. User desktop failure/return/repeat and eligible
ALPS1 coverage belong to P22-VIS/P22-REPEAT/P22-SECOND acceptance. Native endpoint cycles test observation and
source wrappers; they are not actual Windows World unbind/rebind execution.

## Verified historical static results

Shared origin/master/FETCH_HEAD later advanced independently to
`0ce2694aba97efd0f4c99efd13f4d5ebf8d9825e` during this work. The initial fetch
and actual implementation base remain `30b681c`; no later source was imported.

These counts are decoded afresh from the fingerprint-verified historical capture,
not from baseline archives and not from a running mission:

| Payload | Tutorial | ALPS1 |
| --- | ---: | ---: |
| actors.bin Human declarations | 23 | 49 |
| actors.bin Car declarations | 4 | 5 |
| actors.bin Dog / Null declarations | 0 / 0 | 1 / 55 |
| scene2.bin Door declarations | 25 | 1 |
| check2.bin navigation sets | 863 / 56 | 5113 / 100 |
| items.dat records | 75 | 374 |
| Scripts.dta binding pairs | 114 | 148 |

Captured GameData00 includes `Tutorial` and `Alps1` mission-resource text24 values;
this does not establish current route/profile eligibility. The animation payload
decodes 3,720 rows and declares 1,316 gated resolution attempts; no animation
resource was successfully resolved by this static endpoint. Tutorial has one
stored track definition; ALPS1 tracks absence remains only historical evidence.

Focused reused native endpoint fixtures also execute the actual particle and
ShotMgr surface publication bodies and assert that the new records arrive from
those producers; no manually invoked observation callback stands in for them.
Their engine endpoints remain doubles.

Final native checks cover 100 endpoint cycles plus 16 negative parser/fingerprint/
unsupported-record cases, ASan/UBSan/LSan enabled. LeakSanitizer required execution
outside the ptrace sandbox; only temporary native programs ran. Actual i386
loading/World/diagnostic construction and calls compile/link; Windows outputs
were not executed. Existing application link verification passes. Temporary
audit sources/binaries are fingerprinted in the receipt and removed. No permanent
tests were added. The function tracker check cannot complete in this isolated
worktree because existing ignored evidence `analysis/work/multiplayer-rule-caret-20261101/rule-refresh.asm`
is absent; no ledger or denominator was changed to bypass that check.
