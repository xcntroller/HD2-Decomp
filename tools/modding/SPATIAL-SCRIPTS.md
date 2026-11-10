# Spatial script intelligence — 2026-11-10

The existing 3D Mission Explorer now has a docked **Script Relationships** tab.
Select a real mesh or hierarchy object to browse its serialized script bindings,
compiler constructs, outgoing references and incoming references. The viewport
and selected object remain available while inspecting original source. This is
static inspection, not execution, debugging, observed assignments or simulation.
The GUI integration is implemented and toolkit-desktop tested **in the current
worktree but remains uncommitted pending user desktop confirmation**. The
independent query layer is a separate source-only local checkpoint. No push/merge or broader compatibility claim.

## Audit and original evidence

The implementation reuses [structured script/include analysis](SCRIPT-ANALYSIS.md),
[dependency graph](dependency_graph.py), [serialized spatial model](SPATIAL.md),
[current fidelity/resource contracts](FIDELITY.md) and the existing ResourceStore.
No parser/compiler grammar, original runtime header or reconstruction changed.

| Existing evidence | Classification and boundary |
| --- | --- |
| [MissionScriptBindings](../../recovered/game/mission_script_bindings.h),00428600 | Frame/script string pairs with original binary IDs/offsets. Native filters through driver+88; serialized pair presence does not prove runtime binding acceptance. |
| [Module binding](../../recovered/game/mission_script_module_loading.h),00427730/00428600 | Scripts.dta versus mpscripts.dta and reverse loading/context replacement matter at runtime. Both table kinds are labelled separately; the viewer never claims to know the active context. |
| [Frame receivers](../../recovered/game/mission_script_frame_commands.h),00416E10 opcode5E | FRM_FindFrame clears its destination, queries driver+88 then scene+7C. A unique static scene-name match is a **candidate**, not execution or a reproduced global lookup. |
| 00416E10 opcode66 | FRM_FindChildFrame requires a runtime parent slot and has destination/parent aliasing behavior. A literal child name alone never supplies a spatial endpoint. |
| 00416E10 opcode1 | ScriptAssign uses a runtime frame slot; a null destination can use the bound SpawnPoint's spawned Human. Literal script-resource selection does not establish its receiver or that assignment occurs. |
| Existing compiler0042C0B0 and event readers0042E410/0042E730/0042E460/0042E550/0042E640/0042E7C0 | Statements/instructions, registered event markers, labels, Whenever records and supported variables remain compiler products. Event block containment is not dispatch, reachability or execution. |
| Existing0042C850/0042C580/0042C4B0 preprocessing | Original include lookup/shared-cursor behavior and defensive restrictions remain unchanged. Source maps identify the actual original root/included file and byte span. |

Previously, spatial objects had outgoing script links and a button opening the
first linked script. They could not browse event/reference neighborhoods in the
world, navigate incoming references or inspect an edge. The new layer indexes
these already-recovered products, without recompiling every script or inventing
semantics for other commands' strings/frame slots. Lexical mentions are not
reference edges. Ordinary command arguments and variable declarations are shown
as established compiler observations, not evaluated values or data flow.

## Independent query representation

`spatial_scripts.py` has no Tk/OpenGL import. Typed Binding, ScriptReference,
Overlay and Item records keep original graph IDs, binding binary offsets,
statement/instruction IDs, source locations, certainty and provenance. Overlay IDs
are deterministic hashes of the original instruction and binding IDs. Existing
graph edges/certainty are retained even when the spatial index can additionally
identify a name candidate absent from the older document-only graph.

Scope is the selected frame and, for an imported member, its actual serialized
model owner. The owner association is labelled; it does not give every child its
own VM. Arbitrary scene ancestors do not inherit script ownership. This can mean
selecting the bound frame in the hierarchy to inspect another base-scene child.
All original binding records remain available in coverage. Duplicate/case-sensitive
binding names and ambiguous script targets are not arbitrarily selected.

FRM_FindFrame candidates use a unique ASCII-case-insensitive static scene name.
Duplicate names **and case variants** are withheld, even where the original
case-sensitive driver tree could potentially pick one. This is an explicit
conservative restriction because global registration scope/traversal is unknown.
Ambiguous matches retain up to64 candidate IDs and their total count. Candidate
bindings/references can appear in incoming results without acquiring a unique
spatial target. Nothing substitutes a similar filename.

ScriptAssign resource links retain resolver-selected/candidate/unavailable status
from the original graph and remain **possible assignments with unknown runtime
receivers**. They have no manufactured object endpoint. Script-to-script resource
leaves can be opened in the existing Inspector or Mission Explorer; no line to
another object's current binding is asserted to be the assignment destination.
Dynamic names and child-frame queries likewise remain panel-only. Actual runtime
assignments observed is always0 because no runtime is observed.

Original archive/loose/Patch selection and explicit installation/include profiles
are unchanged. Selected invalid/unavailable resources never fall back to another
archive or alternate source. Source reads require an existing analyzed source ID,
size<=65535 and matching SHA256. Changed/foreign/oversized sources fail visibly;
no stale or shadowed body is shown as the newly selected source.

## Panel and spatial annotations

Open **Mission explorer…**, inspect an installation mission, choose **Explore 3D**,
select a soldier/object, then **Script relationships**. The tab is beneath the
viewport and can be enlarged with the existing pane divider.

- Search by script/command/label, certainty, status, PC identity or source position;
  filter All/Outgoing/Incoming and construct kind. Rows are paged250 at a time.
- The tree discloses object → script → compiler event block → statement/reference
  and recognized target/resource leaves. If multiple event intervals contain an
  instruction, the first supplies the visual branch and all intervals remain in
  exact evidence. This is not an execution tree.
- Select a row to see original source, syntax highlighting, diagnostic markers and
  the exact original byte range. A location chooser handles constructs spanning
  multiple source-map pieces. **Exact evidence / provenance** retains archive
  selection/checksum metadata, arguments, raw operands, IDs and limitations.
- **Script Inspector** opens the existing application at that same location,
  including the included source. Returning preserves spatial selection/camera.
  A retained-unanalysed script can be explicitly compiled there; this does not
  silently change the frozen neighborhood. Analyze it through Mission Explorer
  and reload the spatial snapshot to incorporate new graph products.
- **Resource / record** follows a unique recognized target through the existing
  Mission Explorer, preserving its session/profile. **Source object** and
  **Target object** navigate supported endpoints. **Back to context** restores
  the anchor object and camera. Ambiguous/nonspatial targets explain why they
  cannot be followed as a unique spatial object.
- Enable **Static links** for bounded neighborhood annotations. **Selected only**
  defaults on to reduce clutter; disabling it exposes the filtered neighborhood.
  Optional labels show the command/literal. Click a line for its exact source
  evidence while keeping the current object selection.

A line requires two distinct supported IDs with established world matrices:
source is a uniquely matched serialized binding's frame context; destination is
an established static name candidate from the compiled FindFrame instruction.
Positions are their unchanged serialized world-frame origins, not bounding-box
centres, fallback positions, runtime actor positions or guessed spawn locations.
Self references have no zero-length line. Unknown matrices produce no endpoint.
Lines are explicit X-ray annotations and may cross hidden geometry; selection
highlights supported bounds at both ends. Arrowheads indicate reference direction,
never movement, interaction, firing or execution.

Current spatial name-reference lines are dashed orange **candidates**. The renderer
also defines distinct serialized/resolver-selected/unresolved styles, but does
not promote candidate links or invent unresolved endpoints to exercise them.
Nonspatial resources/assignments stay in the tree. Alpha-aware object picking and
line-ID picking remain separate; edge selection does not replace object selection.
Native labels use a bounded fixed X11 font display-list set owned by the GL
context. No engine, additional renderer package or original DLL is needed.

## Bounds, determinism and lifecycle

A snapshot indexes existing retained mission/compiler products once, without
materializing every object/script cross-product. Queries scan at most12000 rows
or candidate pairs, return at most250 primary rows and draw at most96 edges.
Truncation sets `total_is_exact=false`; displayed ≥ totals are lower bounds,
not complete graph counts. Incoming source contexts and ambiguous candidate IDs
are capped64 with explicit totals. Renderer labels are limited96 ASCII bytes.
The earlier file/construct/node/geometry/image budgets continue to apply.

Queries and source reads use the spatial window's single background worker.
There is one running panel task and one replaceable latest request. Generations
reject stale results; selection/reload clears old rows/edges before new results.
Tk/GL calls stay on the GUI thread. Closing cancels queued work and discards pending
results; reloading clears overlay IDs. Font lists are reused within their context
and released on close. Underlying model geometry/textures remain shared.

**Export neighborhood…** writes a new deterministic schema1 JSON report outside
the installation. It contains the current bounded page, filters, exact-total flags,
source/object provenance, overlay evidence IDs and coverage, with no script bodies
or vertex/texture payloads. Full spatial schema2 adds `coverage.scripts`; the
existing mission/script schemas and original dependency graph are unchanged.

The headless interface uses the same queries:

```
python3 tools/modding/spatial_inspect.py /path/to/game tutorial \
  --inspector /tmp/hd2-inspect --script-object TUT_Instructor9 \
  --relationship-search FRM_FindFrame \
  --output /path/outside-game/instructor.neighborhood.json
```

An exact stable spatial ID also works. Names must have a unique exact match.
Without an inspector, indexed scripts honestly lack compiler neighborhoods;
serialized bindings and unsupported/limited diagnostics remain available.

## Verification receipt

Original-profile local Tutorial, ALPS1 and Arctic1 inputs, strictly read-only.
These are retained-session observations, not a whole-installation census or
runtime/full-format compatibility claim. Some programs are unanalysed/limited or
rejected by the existing bounded front end; no compiler products are fabricated.

| Observation | Tutorial | ALPS1 | Arctic1 |
| --- | ---: | ---: | ---: |
| Serialized binding records / unique static frame matches | 114 /114 | 148 /148 | 105 /105 |
| Retained analyzed scripts | 131 | 138 | 83 |
| Analyzed scripts selected from Patch | 2 | 8 | 2 |
| Compiled receiver-reference records / original spans checked | 349 /349 | 527 /527 | 461 /461 |
| Static unique scene-name candidates | 276 | 480 | 429 |
| Missing scene-name candidates | 47 | 25 | 22 |
| Possible ScriptAssign records (runtime receiver unknown) | 26 | 22 | 6 |
| Child queries withheld for runtime parent | 0 | 0 | 4 |
| References with distinct placed source/destination context | 250 | 438 | 385 |
| Nonspatial/unresolved/self-only references | 99 | 89 | 76 |

Headless focused checks compare all1337 reference locations with the original
argument byte slices, preserve compiler PCs and graph IDs/certainty, check all
inspected overlay endpoints against their original spatial nodes, and verify
incoming/outgoing union, search/event/source-position filters, paging, export
round-trip/determinism and installation write rejection. Category samples include
Human, SpawnPoint, Switch, Animation, NullActor, StaticWeapon, Car, Dog and Door
where actual binding records exist. Other actors honestly show no verified binding.

Real Tutorial Human examples include TUT_Instructor9 with its original script;
Tutorial's Switch uses an imported member frame. Arctic1 has a Door binding on
an original base-scene frame. Observed binding-category counts include23 Tutorial
Human/4 SpawnPoint/1 Switch and Arctic1 Door/4 SpawnPoint/1 Switch. These are
binding-record categories, not actor population or gameplay ownership claims.

A real AFRICA3 af3a_14 included construct navigates to its actual included source
ID/span through the original include context; no commercial source is committed.
Synthetic negative fixtures verify ambiguous names/case variants and bindings,
dynamic strings, child-parent requirements, unknown ScriptAssign receivers, scan
limits and foreign/changed source rejection. Shared CRLF/CR/LF/control-byte display
mapping is checked independently. Core sampled category queries took about3–6ms;
these are local observations, not worst-case responsiveness guarantees.

Actual native X11/GLX checks pick a real decoded Tutorial Human mesh, browse its
binding/events/references, highlight original source, open/return from the existing
Inspector, draw labelled real relationships, pick an edge, navigate endpoints and
restore camera/selection. Final desktop checks also open ALPS1 and Arctic1, inspect
real Door/Switch/SpawnPoint source bindings, follow a script resource into the
same Mission Explorer session, reload Arctic1 and close during a pending query.
No exercised GL errors or Tk callback errors. Actual panel and viewport captures
were visually inspected. This is toolkit desktop verification, not user acceptance
or original-game execution. Peak RSS648196KiB in the exercised multi-mission
process is an observation, not proof of leak freedom or a universal memory cap.

The existing native inspector remains unchanged; its build is up to date. Python
compile checks and diff whitespace checks pass. No new C++/binary parser path was
introduced, so no unchanged broad game/sanitizer audit was repeated. Earlier
compiler/native audits remain in SCRIPT-ANALYSIS.md. All temporary fixtures,
captures/reports and audit sources are outside Git and removed after verification;
an optional viewport-only preview can remain in/tmp for user review.

No client/server/shared handoff/ledger modifications, original assets/extracted
reports in commits, original EXE/DLL/Proton execution, Ghidra, heavy concurrent
builds, push or merge. Runtime assignments, data flow, predicates' hidden object
semantics, debugging, simulation, mission editing and binary serialization remain
out of scope. Existing spatial/format limitations in FIDELITY.md still apply.
