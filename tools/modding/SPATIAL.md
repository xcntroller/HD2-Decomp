# Read-only 3D Mission Explorer — initial milestone, 2026-11-10

Current [spatial script intelligence](SPATIAL-SCRIPTS.md) adds the docked script
panel, static overlays and source/context navigation.

Current fidelity improvements and three-mission verification are documented in
[FIDELITY.md](FIDELITY.md). The initial verification receipt below is historical;
its counts are superseded by that checkpoint.

The existing toolkit now combines original base Scene.4ds geometry, supported
Scene2 placements, actors and original model resources in one interactive view.
This is a **partial serialized spatial snapshot**, not a running mission or a
complete reproduction of the original world renderer. Unsupported relationships
stay unresolved. No original executable, DLL, Proton or Ghidra was run.

## Open and navigate

Build/start with `make -C tools/modding gui`. In **Mission explorer…**, open your
installation, choose the explicit language/archive profile, select a mission and
**Inspect mission**. Choose **Explore 3D / selected object**. Existing record
selection follows into the spatial hierarchy when a unique association exists.
The main **3D missions…** entry also opens this same integrated window.

- Orbit: left drag; wheel zoom; middle drag pan. **Fit scene** restores the
  overview. Large backdrop geometry can make this overview distant; zoom or
  double-click a hierarchy entry to focus its supported bounds/position.
- Fly: select `fly`, click the viewport, hold WASD; Q/E move vertically, Shift
  accelerates, right drag looks. Wheel adjusts speed. Speed/range also accept
  numeric settings. Position and actual per-frame loading/draw statistics appear
  below the view. F focuses selection and switches to orbit.
- Click an actual mesh to select it, or use/search the hierarchy. Search is flat,
  debounced and paged; an empty query restores parent-child browsing. Selection
  highlights the mesh/model children and can show bounds and origin crosses.
- Category filters, wire/solid, textures, hidden objects, grid/axes and additional
  origins are controls for this preview. Actor/spawn/light origin markers and
  grids are explicitly viewport annotations, never decoded meshes or terrain.
- **Inspect record** returns to the original Mission Explorer file/record.
  **Inspect model** reuses its decoded asset in the existing Model Inspector;
  its return button returns to the selected 3D object. **Open associated script**
  opens the existing source/diagnostic environment with its real installation
  include context and **Back to 3D object**. Script references can also follow
  existing Mission Explorer records and then the 3D association. No script runs.
- Properties retain identities, offsets, local/world matrices, parent status,
  actor fields, model provenance, texture coverage and existing classified links.
  **Coverage / diagnostics** lists omissions and fallback-material conditions.

Loading uses one background worker, a captured inspection snapshot and bounded
progress messages. Tree insertion is batched. Failed loads retain the previous
scene and its corresponding session. Closing cancels pending work; workers never
call Tk. Export creates a new JSON report outside the installation.

Headless inspection, without Tk/OpenGL or an original executable:

```
python3 tools/modding/spatial_inspect.py /path/to/game tutorial \
  --output /path/outside-game/tutorial.spatial.json
```

Optional `--inspector /tmp/hd2-inspect` enriches script analysis; otherwise existing
bindings/dependencies are indexed. `--archives-first` / `--language` retain the
established resolver profiles. Exit0 means inspection/export completed, not full
world compatibility. Current spatial JSON schema2 is deterministic, streamed, and includes
ordered nodes, stable resource/offset IDs, assets/provenance, parent resolution,
actor/script relationships, diagnostics, statistics and limitations. Geometry is
referenced by identity; original vertex/texture payloads are not exported here.
Existing mission/script schemas and dependency certainty are unchanged.

## Reused evidence and original composition

Relevant contracts were read before implementation:
[mission scene reader](../../recovered/game/mission_scene_reader.h),
[scene documents](../../recovered/game/mission_scene_documents.h),
[scene objects](../../recovered/host/game_scene_objects.h),
[mission scene](../../recovered/host/game_mission_scene.h),
[world loading](../../analysis/mission-world-loading.md),
[scene ownership](../../analysis/world-scene-ownership.md),
[resource directories](../../recovered/io/resource_directories.h),
[cache](../../recovered/io/loaded_resource_cache.h), and the toolkit's existing
[4DS](MODEL4DS.md), [formats](FORMATS.md), [resolution](RESOLUTION.md) and
[script/include](SCRIPT-ANALYSIS.md) contracts. No reconstruction code, shared
handoff or function ledger changed. Toolkit implementation earns no runtime
reconstruction credit.

[spatial-evidence.json](spatial-evidence.json) identifies the same original
LS3DF/client images, bounded static code fingerprints and dispatch pointers.
Original addresses are evidence, never dependencies during normal operation.

| Original evidence | Spatial interpretation / boundary |
| --- | --- |
| Client006D4180 → scene+F4 / LS3DF10047D20 →1005D260/1005D970 | Load `missions/<namespace>/Scene.4ds` before the Scene2 document; genuinely visible environment geometry comes from supported original visual records here |
| Client006CC760 /006CD940, recovered scene documents | Process Scene2, actors, sounds in original order, retaining object/actor chunk order and offsets |
| LS3DF100462B0 and1001C0E0/1002C600; GameScene.register_existing_frames | Primary, Backdrop and Omni sectors have constructor identity transforms and position-lock bit80; they are identified as constructor evidence, not serialized objects. Near sector is not guessed into the scratch map |
| Client006CD940 / object0x10,0x4011 | Kind0 finds the existing exact name; supported nonzero kinds create only absent names; duplicate explicit creation is declined |
| Client006D1873..006D1987 | Position first (unless E0 bit80), new-frame automatic sector handling, explicit parent, quaternion/scale, hidden state. Known explicit-parent links use flag0 |
| LS3DF1001D680 | LinkTo flags0 retains local TRS; flags1 preserves world through inverse/decomposition; these are different operations |
| Client006CDE38..006CDEC8 and0041D180 → Normalize10030540 | Scene2 chunk0x22 is serialized **WXYZ**, reordered to engine XYZW, then normalized |
| LS3DF1001EC40/1002D450/1001E390/1002D830 | 4DS common stores **XYZW** directly; SetRot3 signs, per-row scale and row-vector `world = local * parent` are established; serialized coordinates are unchanged. Display camera conversion is documented in FIDELITY.md |
| LS3DF10047C70, scene+E0 | Automatic sector choice and LinkTo flag1 depend on sector/spatial behavior; this selection is not reconstructed |
| Client006CC760 /AE23→AE22→AE24 | Actor definition binds to the frame available when its type chunk is read; later frame chunks do not retroactively bind earlier actor definitions |
| Client006DBE60 / model_resource_key / cache1 /1005D260 | The default Models root is established; toolkit selects the exact `Models/<serialized-key>.4ds` under its explicitly restricted default-root profile |

**Quaternion source-contract discrepancy:** the portable recovered reader passes
its four serialized reals through its generic quaternion representation. The
original object-handler assembly explicitly reorders the Scene2 WXYZ stack
fields before finalization; the setter copies XYZW and normalizes. Toolkit
conversion follows that original evidence. The reconstruction is untouched. Raw
Scene2 components remain available in the original inspection record. 4DS and
Scene2 quaternion encodings must not be interchanged.

Positions/scales are retained as serialized. Scene2 quaternion normalization uses
finite double-precision arithmetic. A zero/near-zero quaternion is rejected as a
**tool safety restriction**, not asserted to match the original zero case.
Derived matrices are not claimed bit-identical to original single precision.
Explicit nesting composes supported local matrices; model children retain their
actual one-based 4DS parents beneath their actual model instance root. Identical
model instances share decoded Geometry/Image objects and GPU allocations.

The scratch name map contains original base frames, supported explicitly
created objects and prefixed imported members (`instance.originalMember`). Actor
lookup uses a unique exact scratch name, then a unique ASCII-insensitive supported
scene name. Global registration scope and duplicate traversal remain excluded;
see [current name/sector evidence](FIDELITY.md#evidence-and-scene-relationships).
Ambiguous names never select an arbitrary object.

Chunk0x2C fallback/world position is preserved as metadata and compared with
computed placements. It is **never** used to invent a mesh placement when sector
parenting or a parent name remains unresolved. The first nested0x10 in0x4020 is
looked up at its original encounter time; later names do not repair forward
references. Missing/ambiguous/unsafe parents withhold world matrices; children
inherit that unresolved state. For existing frames, an unsupported parent case
is conservatively withheld even where the engine might retain an old parent.
Repeated model replacement is explicitly unresolved rather than overlaying old
and new geometry. Hidden state is conservatively inherited for display; this is
not the complete original visibility system.

## Actual environment format subset

The previous decoder stopped at sector5 / occluder12 records. Static reads of the
actual H&D2 reader dispatch establish their lengths, allowing traversal to later
visible meshes without guessing resynchronization:

| Kind / native reader | Serialized tail after common frame |
| --- | --- |
| Sector5, vtable1009BC20+70 →1004E250 | Header48; DWORD vertex/triangle counts at+8/+12;16 bytes per volume vertex;6 bytes per triangle; byte portal count |
| Portal100431A0 | Byte count, then `(count+2)*16` bytes. Count<=23 is a defensive tool limit derived from the native fixed object buffer, not a new format promise |
| Occluder12, vtable1009B8E8+70 →1003A960 | DWORD vertex/triangle counts,16 bytes per vertex,6 bytes per triangle; no extra trailing serialized read |

These tails remain **opaque, partial, non-rendered volumes** with offsets, size
and SHA256. Index/count/bounds checks reject malformed supported records. Volume
surfaces are not presented as visible terrain. All genuine environment drawing
comes from supported visual geometry; no ground plane or placeholder building
is inserted. Unknown frame types/additional vertex streams still stop safely.

Original lightmaps/aliases, area/environment tables, occlusion/portal traversal,
original materials/alpha/lighting, animation and actor initialization can change
appearance and runtime placement. They are not simulated. Joint/skin data remain
undeformed, with partial status. Actor-attached resources not connected by the
supported serialized relationships are not invented. Supported alpha-source
previews now exist, with device/color-key and blending limitations; some tree
rectangles remain. See [material coverage](FIDELITY.md#materials-recovered-sources-versus-preview-rendering). Some ground, sky and blended
surfaces use explicit gray fallback materials. Spatial previews therefore differ
visibly from the game, even when geometry/transform decoding is established.

## Resolution, certainty and safety

Base scene/model/BMP reads use ResourceStore and retain explicit loose/Patch/base
precedence, alternate archive-first profiles and checksum results. An invalid or
unsupported selected member never falls through to a shadowed resource.

A Scene2 model link is a serialized relationship. Selection now follows explicit
ordered model/texture directory profiles, including native texture reverse/sticky
lookup rules. Windows descendant registration and warm/global cache state remain
unresolved. The existing Mission Explorer graph's model candidates remain
candidates. See [resource evidence and defensive barriers](FIDELITY.md#resource-selection).
Neutral gray is an explicit fallback, not successful material decoding. Per-asset
status/provenance and texture failures are visible in properties/model inspection
and export.

Budgets:50,000 spatial nodes,192 unique model assets,96MiB retained packed geometry,
48MiB shared decoded texture storage including auxiliary/RGBA planes,512 texture dependencies per spatial asset.
Existing64MiB/member, bounded archive decoding and per-image safeguards remain.
The standalone Model Inspector keeps its earlier64-dependency/32MiB defaults.
These are payload budgets, **not** a total-process RSS cap. Model/source metadata,
Tk, original script analysis and graphics-driver allocations add overhead.

Only LOD0 is uploaded in the mission view; buffers are shared by geometry identity,
textures by decoded Image identity. At most8,000 GPU meshes and6,000 visible draw
candidates are admitted. Conservative sphere/frustum/range culling and category
filters reduce work. Omission counts are displayed; enabling all origin markers
can consume the draw budget. No distant/unsupported object gets substituted
geometry. GPU buffers/textures are deleted with their owning current context on
replacement/close. The independent Model Inspector has a separate GL context.

## Initial verification receipt (historical)

Actual local read-only installation, default loose-first English profile:

| Measurement | Tutorial | ALPS1 |
| --- | ---: | ---: |
| Base Scene.4ds bytes | 2,858,981 | 1,380,788 |
| Declared / decoded base frames | 1,923 /1,923 | 790 /790 |
| Base visual / dummy / sector / occluder frames | 1,608 /281 /9 /25 | 724 /41 /2 /23 |
| Base unique geometry / stored vertices / stored triangles (all LODs) | 831 /74,629 /48,089 | 712 /34,746 /31,492 |
| Scene2 object / actor records | 1,890 /27 | 4,007 /1 |
| actors.bin object / actor records | 53 /53 | 133 /133 |
| sounds.bin object records | 2 | 30 |
| Total spatial nodes / unique assets | 6,345 /106 | 7,211 /73 |
| Placed, visible supported mesh instances | 3,043 | 3,709 |
| LOD0 rendered triangles, full default overview | 106,351 | 116,592 |
| Unique uploaded meshes | 1,100 | 936 |
| Unresolved transforms including descendants | 101 | 398 |
| Resolved / unresolved actor definitions | 78 /2 | 134 /0 |
| World/fallback checks, tolerance0.02, matches / comparisons | 1,148 /1,148 | 1,361 /1,361 |
| Retained packed geometry bytes | 3,712,396 | 2,415,606 |
| Shared decoded RGB bytes | 25,334,187 | 29,677,056 |

Tutorial Scene2 is selected from Missions.dta/ISD0; ALPS1 Scene2 is selected from
Patch.dta/ISD1 raw-LZSS with integrity verification, not the shadowed Missions
member. All106/73 selected assets were available under the restricted default-root
profile. This does not prove other missions/cache policies.

Base scene SHA256 fingerprints:
Tutorial `2daa12ed9d83c9a7d327f4786db99691314a7d7ed86d7427e16439e5aa814375`;
ALPS1 `c051ff74dee4653a0d6590bd161784d913a2bed191a20eb98eedaf6dff19c5f4`.
Both match three independently retained prior original resource captures each.
A temporary independent C++ stream reader, reusing only the established first-table
contract, matched **every frame start/end/type/parent and world matrix**, vertex/
triangle totals and exact file end in both scenes. ASan/UBSan/LeakSanitizer passed
for that reader. This sanitizes the native audit reader, not Python or original
code. No original code was executed. New sector/portal/occluder truncations,
count guards and the transactional malformed-input behavior passed Python checks.

Focused synthetic checks passed nested scale/rotation/translation, Scene2 WXYZ
reordering/normalization, duplicates, missing/forward/ambiguous parents, zero
quaternion withholding, automatic-sector uncertainty, cycles, ordered actor
binding, malformed Scene2 and camera math/culling/switching. Deterministic streamed
JSON round trips and original-folder/existing-file export barriers passed.

Native X11/GLX desktop checks passed for both missions: real environment/model
rendering; hierarchy/click picking synchronization; native right-drag look and
W key movement; focus/orbit; visibility/wire/textures; search; original-record
return; cached model inspector/return; associated original script/return; reload
and close. Actual overview and closer rendered images were visually inspected;
additional ground-level views near serialized actor locations were inspected.
No claim of fresh human/user desktop acceptance is made.

Mesa26.2.6 compatibility OpenGL4.6, AMD RX9070XT: full overview drawing about
50–67ms in this audit (CPU submission + swap, not a GPU benchmark). Default
filters admitted all3,043/3,709 mesh instances without draw-budget omissions.
Headless sequential load observed roughly6 seconds per mission, peak196,856KiB
before JSON checks; streamed export/round-trip audit peaked256,792KiB. Integrated
GUI/script/GL checks peaked647,640KiB; repeated ALPS1 reloads settled at161,394
resident4KiB pages (645,576KiB) with no further growth over two reloads. Each
replacement deleted2,331 buffer objects; immediate `glIsBuffer` / `glIsTexture`
checks confirmed deletion before names could be reused. These are observations,
not universal performance or leak-freedom guarantees.

Texture dependency statuses across all selected assets (repeated material uses
count separately): Tutorial514 RGB decoded,32 resolved-uninterpreted,13 ambiguous,
3 unsupported,2 invalid; ALPS1375 decoded,30 resolved-uninterpreted,2 invalid.
These include BMP validation failures, unsupported TGA, secondary/sequence maps
and ambiguous names. No invalid texture was rendered as successfully decoded.

Temporary readers, synthetic tests, raw inputs/reports and rendering captures
were kept outside Git and removed after verification. One temporary toolkit
preview may remain in `/tmp` for user review. No permanent tests, original assets,
raw captures, commercial scripts, game build or shared reconstruction edits are
part of this checkpoint.

## Remaining boundary

The initial partial interactive mission-viewing milestone was delivered; the
current [fidelity checkpoint](FIDELITY.md) extends it. Full
world compatibility needs actual automatic-sector selection, scene-path/name
scope/cache policy, lightmap/area/landscape/other unsupported record consumers,
material/alpha/lighting, skins/animation and runtime actor initialization. Those
are separate evidence/reconstruction tasks. Editing, binary writing/serialization,
mission simulation and a debugger remain excluded.
