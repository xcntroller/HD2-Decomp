# Mission Explorer fidelity checkpoint — 2026-11-10

This extends the initial [spatial milestone](SPATIAL.md). The viewer displays
more original serialized content, with supported alpha sources and comparable
coverage reports. It remains a **partial cold serialized snapshot**, not the
original renderer or the runtime world. No original executable, DLL, Proton or
Ghidra was executed. Only `tools/modding` changed.

## Use and export

Open the existing **Mission explorer…**, inspect an installation mission, then
**Explore 3D / selected object**. Existing fly/orbit, picking, hierarchy, model,
record and script navigation remain. **Coverage / diagnostics** separates known
world matrices from unknown parent identities, omitted geometry, material
fallbacks and partially interpreted assets. Selected properties retain original
and prefixed names, stable IDs, source offsets, transforms and provenance.

```
python3 tools/modding/spatial_inspect.py /path/to/game tutorial \
  --coverage-only --output /path/outside-game/tutorial.coverage.json
```

Full spatial metadata is now schema2; its nested/standalone coverage report is
schema1. Existing mission/script schemas remain2 and model metadata remains1
with additive fields. Exports are deterministic for the same inputs/profile,
exclude timing/camera state, stream JSON, create a new file and reject paths
inside the installation. Counts measure serialized coverage, not visual fidelity.
Full node/asset reports preserve omission reasons, unknown extents and diagnostic
offsets; compact GUI previews can truncate long lists. Original pixels/meshes
are not serialized by spatial export.

## Evidence and scene relationships

[fidelity-evidence.json](fidelity-evidence.json) records original image hashes,
bounded code fingerprints and dispatch pointers, supplementing
[spatial-evidence.json](spatial-evidence.json) and the existing reconstructed
[scene reader](../../recovered/game/mission_scene_reader.h),
[resource directories](../../recovered/io/resource_directories.h) and
[cache](../../recovered/io/loaded_resource_cache.h). Addresses below refer to
that LS3DF image unless prefixed client. These are static evidence, not required
binaries during normal toolkit operation or new runtime reconstruction credit.

| Evidence | Established behavior / toolkit boundary |
| --- | --- |
| 100346A0 →10034630 →1001D470; format100A6784; model copy10034760 | Imported members acquire `instance.originalMember` names; the original member suffix is retained separately. This is full-name registration, not a dotted path grammar. |
| Client006CE71D..006CE734, callback006CC580 | Successful model loading enumerates imported members into the client scratch map. The toolkit now registers these names at that stage. |
| 10046920 →1001DF00 /10098740 | Default scene lookup uses ASCII/C-locale case-insensitive names and mask-filtered sector traversal. Toolkit tries a unique exact scratch name, then a unique supported case-insensitive scene name. Duplicate traversal/order/scope is withheld, not guessed. |
| Driver+88 →1001A0F0 | Exact registered full-name tree lookup, filtered by owning scene through sector ancestry; it does not split names into paths. Unrecovered global registrations/scopes remain unresolved. |
| 10047C70 →10047A90;1004CD40 | Automatic parenting examines existing sector ancestry, special flags and containment; first containing sector in a circular scene list wins, otherwise Primary. Reparenting uses LinkTo flag1. |
| 1004D6F0 /1004D270; epsilon1009D260 | Containment needs updated world bounds and normalized/filtered derived planes. Serialized triangle bounds alone are insufficient. Sector list order, portal/deferred updates and derived-plane handling are not reconstructed. |
| 1001D680; client006D1873..006D1987 | Flag1 preserves world before later local rotation/scale setters; flag0 preserves local TRS. The distinction is retained. |
| 1001CFE0 | Serialized flags bit0 seeds visibility; hidden/ancestor suppression is conservative preview filtering, not the complete runtime visibility policy. |

New imported-name registration resolves actual Tutorial nested furniture/tank
parents and barrier/alarm actor frames; complete character tables also expose
ALPS1 attached frames. Original encounter order, exact offsets and identities are
retained. Duplicate names remain ambiguous rather than assuming the original
map replacement or scene traversal order.

Automatic **sector identity is still unresolved**. A narrower matrix invariant
can nevertheless establish world TRS: if all loaded frame tables are complete,
no referenced model failed, every possible sector has identity world linear
transform (translation only), and no sector transform/link changed during
document composition, world-preserving parenting cannot affect the final TRS
after the supported local overrides. The unknown translation cancels. Such roots
keep `parent=None`, candidate sector IDs, parent status
`automatic_sector_identity_unresolved`, and world status
`verified_translation_only_sector_invariant`. They are not claimed as serialized
root frames. Descendants compose only through established world matrices.

Rotated/scaled/changed/unknown sectors, incomplete tables and failed model loads
disable this inference. Arctic1 contains a sector with a rotated ancestor, so
its affected matrices remain withheld. This is a derived invariant of established
link semantics, not successful original sector selection. Fallback chunk0x2C is
comparison metadata; it never supplies a guessed mesh placement. Scene2 WXYZ
normalization, direct4DS XYZW and row-vector `local * parent` remain unchanged.

## Display handedness correction after user review

The user reported that the world appeared flipped relative to gameplay and
otherwise accepted the fidelity changes. The cause was a display-camera defect,
separate from serialized placement: the toolkit combined untouched original
coordinates with a conventional right-handed negative-Z OpenGL camera.

Original camera builder10009B30 zeroes the projection then, in its perspective
branch, writes1.0 to camera+1BC at10009CC0 (`m23` of the matrix at+190), leaving
`m33=0`. Thus clip W is **positive camera Z**, with positive X/Y scales. This is
the original positive-Z camera convention, unlike glFrustum's negative-Z depth.
The additional static code fingerprint is in fidelity-evidence.json.

`spatial_camera.Camera` now derives right/up/forward in that positive-Z convention
and emits an OpenGL view matrix with negative camera depth. Both mission and
standalone model viewports use this display conversion. Source vertices,
quaternions, matrices, offsets and exported world coordinates are unchanged.
The default mission eye/direction is preserved while screen handedness is
corrected. Movement, panning, frustum tests, bounds/origins and ID picking use
the same basis/matrix. Face culling stays disabled; the reflection would need
explicit winding handling if culling is implemented later. This does not claim
to reproduce original FOV, clipping range or the gameplay camera controller.

Focused temporary checks passed: orthonormal bases and matrix direction across
35 yaw/pitch combinations in both camera modes, canonical +Z forward/+X right/
+Y up, mode transitions, rightward movement and frustum consistency. A native
asymmetric fixture rendered/picked +X on the right and -X on the left; GL's actual
modelview matched the computed matrix. Actual Tutorial/ALPS1 geometry, picking,
standalone shared-model viewing and GL cleanup passed with no exercised GL
errors. Corrected Tutorial geometry was visually inspected; ALPS1 test cameras
were obstructed by foliage/walls, so those captures do not establish a comparable
gameplay viewpoint. No original renderer was executed. The user subsequently
rechecked the corrected orientation and confirmed “Looks good; commit,” accepting
the interactive checkpoint and authorizing its local commit. This does not imply
additional case-level verification or original-renderer equivalence.

## Resource selection

`resource_lookup.py` is GUI-independent and uses ResourceStore for every probe.
Archive/loose/Patch precedence and CRC/codec barriers are unchanged. Selections
identify an **explicit ordered directory profile**, separately from exact archive
provenance and existing Mission Explorer `name_candidate` graph relationships.

- Model keys: recovered `model_resource_key`, client006D4EC0/006DBE60 and engine
  1005D260 remove the client last-dot suffix, probe registered model directories
  forward, independently remove a final filename suffix in the engine worker,
  then append `.4ds`. Both removals matter for unusual multi-dot inputs. Default
  toolkit profile is `Models`; explicit supplied roots are bounded to256.
- Textures: graphics table1009D840+3C →10069F70 calls1006FBD0. It probes an explicit
  path first, then the last successful directory, then registered directories
  in reverse order. Default registration `Maps,Maps_C,Maps_U` therefore searches
  `Maps_U` first from a cold state. Direct path hits do not update the sticky
  directory. The selected attempts and relationship certainty are exported.
- Engine warm caches, process-global scopes, and Windows FindFirst/FindNext
  descendant registration order are not reproduced. Directory profile selection
  is not a claim of full original cache equivalence. Each mission load starts a
  new cold lookup state; shared decoded model instances are reused.
- Only missing probes allow another candidate. A selected invalid, unsupported,
  limited or unavailable resource stops inspection. This is an explicit defensive
  barrier, even where an original status loop might continue; it never substitutes
  a shadowed archive or guesses a basename/extension.

## Additional structural 4DS coverage

The decoder still stops at unknown layouts without resynchronizing. Native
factory/vtable reader dispatch establishes these additional lengths after the
common frame. Offsets, opaque hashes, target IDs and material IDs are retained.
Only actual serialized mesh data is drawn.

| Record / original reader | Supported boundary |
| --- | --- |
| Visual3,10054240 | Skin mesh10053900 followed by animation tail10035490; drawn undeformed, partial. |
| Visual4,10007DB0 | Common mesh10038410 then5 opaque bytes; runtime behavior unapplied, partial. |
| Visual5,10035F00 | Common mesh then10035490; no animation evaluation, partial. |
| Animation10035490 | Byte channels; when nonzero byte groups, byte sets; each group×set has WORD count, then channels×count×24 bytes when count>0, byte word-presence and optional count×2 bytes; trailing48 bytes. These are extents only. |
| Visual6,10009610 | Common frame, byte count, each8 opaque bytes plus WORD material reference; no serialized triangle mesh, non-rendered partial. |
| Target7,10056C70 /1005D970 postpass | Common frame, opaque WORD, byte count, WORD frame IDs. IDs checked against declared table (zero retained); target behavior unapplied, partial. |
| User8, vtable1009BB30+70 →1001EC40 | Common serialized frame only; no invented user semantics. |

Counts, allocation sizes, references and truncations are checked before reads.
Malformed supported data removes drawable products from that model. Safe partial
opaque records do not imply that their runtime consumers are implemented.

## Materials: recovered sources versus preview rendering

`material_preview.py` interprets established fields independently of OpenGL.
Four RGB blocks and two scalar setters1002B140..1002B1D0 are exposed with raw
payload; fourth components of the blocks remain unnamed. Native material+58
(payload float17) participates in opacity classification1000181C/1000EA7C.
Only finite values in0..1 are used. The first RGB block is an explicitly labelled
untextured color preview; complete diffuse/ambient/specular consumers are unknown.
Textured preview modulation stays white, avoiding arbitrary material tint.

1002BBE0 maps material flags20000000→texture mode8 (color key),40000000→4
(external opacity),01000000→2 (intrinsic alpha in the relevant alpha family).
10074FC0/1007501D initialize transparent key0. The established converters are:

- Indexed BMP →RGBA8 target:10075140..10075271 compares packed palette RGB with
  black; palette padding bytes are not alpha. This reproduces that target's
  source rule, not device555/565/DXT quantization branches.
- 24/32-bit nonintrinsic key:1007543A/10075503 compares the first source **blue
  byte** with zero. It is deliberately not replaced by a luminance heuristic.
- External opacity:10075184/10075435 use raw8-bit source indices, not palette
  colors. Only equal-size supported BMP inputs are combined; resampling and
  other opacity formats remain unsupported with an explicit RGB fallback.
- Intrinsic32-bit mode uses actual source fourth bytes when requested. It does
  not reinterpret palette reserved bytes as alpha.

Supported textures are still only BITMAPINFOHEADER40 BI_RGB8/24/32 BMPs. TGA,
sequence/extra maps and other layouts are resolved/probed but not rendered as
successful original effects. Source status (`decoded`, `invalid`, `unsupported`,
`unavailable`, `resolved_uninterpreted`) is separate from `RGBA_preview`,
`RGB_preview`, `RGB_fallback_alpha_unsupported`, `color_only_preview` or
`fallback_material`. Missing sources use explicit neutral gray.

`gl_material.py` supplies **viewer approximations**: directional lighting,
SRC_ALPHA/ONE_MINUS_SRC_ALPHA, depth-write suppression during blending, cutoff0.5
for keys /0.001 for continuous alpha, and deterministic far-to-near instance
sorting in missions. These are not recovered original render ordering or alpha
thresholds. Standalone model groups retain their iteration order; neither viewer
sorts individual triangles. Picking keeps exact RGB IDs while texture alpha
rejects holes. Selection tint retains cutouts. Alpha state resets after geometry.
Some tree/vegetation rectangles remain visibly wrong; this is not complete
vegetation/transparency fidelity. No lightmaps, original light consumers,
skin/animation, environment effects or automatic original LOD are simulated.
Scene registration B820 is reported separately from lightmap AE00/B810/40A0.

## Verification receipt

Authorized local installation, original profile with loose/Patch/base selection;
ALPS1 Scene2 remains the checksum-verified selected Patch ISD1 member. Aggregates
below are observations of these inputs, not invariants for other versions.

| Observation | Tutorial | ALPS1 | Arctic1 |
| --- | ---: | ---: | ---: |
| Base frames decoded / declared | 1923 /1923 | 790 /790 | 1742 /1742 |
| Scene2 object / actor records | 1890 /27 | 4007 /1 | 3651 /9 |
| Actors document object / actor records | 53 /53 | 133 /133 | 66 /66 |
| Spatial nodes / distinct loaded assets | 7473 /106 | 9462 /73 | 15017 /141 |
| Established / withheld world matrices | 7473 /0 | 9462 /0 | 14736 /281 |
| Unknown automatic sector parent identities | 16 | 75 | 46 |
| Placed visible supported mesh instances | 3223 | 4235 | 8388 |
| Actor definitions with unresolved frame binding | 0 /80 | 0 /134 | 0 /75 |
| Fallback-position comparisons within0.02 | 1168 /1168 | 1438 /1438 | 1992 /2039 |
| Materials with supported RGBA preview | 44 | 34 | 100 |
| Materials retaining RGB because alpha unsupported | 11 | 8 | 10 |
| Retained packed geometry bytes | 4445180 | 3104008 | 3825828 |
| Retained texture/auxiliary/RGBA bytes | 36156139 | 40996800 | 43213056 |

Previous checkpoint withheld101 Tutorial and398 ALPS1 world transforms. The new
result does **not** establish exact sector identity or runtime-world fidelity.
Arctic1's46 unknown automatic parents withhold281 matrices including descendants.
Its47 fallback disagreements range about0.05859..0.10208 units; they remain
diagnostics, without asserting editor staleness, animation or another unproven
cause. Exact matrices/fallbacks and identities are available in local exports.

Focused temporary audits, removed after use:

- Independent bounded C++ stream reader checked **306 unique selected models**
  across these missions (106+69+131 new resources), every frame start/end, kind,
  subtype, parent and exact file end against the Python decoder. All selected
  model tables completed structurally. New real records included17 visual3,
  10 visual4,13 visual6 and34 target7. Visual5/user8 have static dispatch evidence
  and synthetic extent tests, not real-corpus verification here. ASan, UBSan and
  LeakSanitizer passed for that native audit reader; no original code was run.
- BMP RGB bytes compared independently with Pillow for327/288/531 selected
  sources respectively; auxiliary8-bit indices/32-bit fourth bytes compared
  with raw row access. Real RGBA previews containing both zero/full alpha occur
  in19/10/26 materials. Pillow is audit-only, not a runtime dependency.
- Synthetic checks covered missing/invalid lookup barriers, reverse/sticky
  directory order, double suffix removal, unsafe names, exact alpha sources,
  dimension mismatch, malformed BMP, positive/negative/repeated sector-invariant
  composition, target/material bounds, and visual4/5 tail truncations. Coverage
  encoding/round-trip and protected installation export barriers passed.
- Actual native X11/GLX desktop rendering on Mesa/AMD was exercised and rendered
  Tutorial, ALPS1 and Arctic1 images visually inspected. Real original meshes,
  corrected textured modulation, wire/textures, picking and cached standalone
  model viewing work. A controlled alpha fixture verified transparent pixels
  match background, holes do not pick and visible fragments retain exact IDs.
  No GL errors in the exercised path. This is toolkit desktop verification,
  **not new user acceptance** or an original-renderer comparison.
- Repeated Arctic1 reloads explicitly deleted4042 GL buffers per replacement
  and final close; immediate GL handle checks passed before reuse. Resident
  pages119170→124729→122481 (4KiB pages), peak504948KiB in that desktop run;
  three observations do not prove leak freedom or a universal memory bound.

Existing limits remain50k nodes,192 assets,96MiB packed geometry,48MiB shared
texture storage including source auxiliary and derived RGBA planes (standalone
32MiB),512 texture dependencies per spatial asset (standalone64),64MiB/member,
4096/image dimension and8million/image pixels. Archives are never held whole.
GPU geometry/images are shared between instances. At most8000 unique GPU meshes
and6000 draw instances; Arctic1 can exceed the draw budget in an overview, which
is reported rather than counted as fully rendered. Ground-level native checks
drew1562/372/121 instances at about37/21/31ms; these are camera-specific
observations, not full-scene performance guarantees.

## Editing readiness and remaining blockers

**Safe mission binary editing is not ready.** Additional read-only reconstruction
is essential: exact sector containment/order and scene registration scopes,
the Arctic1 transform discrepancy, complete material/device-alpha/lightmap and
visibility consumers, skin/animation and runtime actor initialization, unknown
records/streams and round-trip preservation contracts. No serializer, unknown
field rewrite or guessed successful fallback was added. The supported fidelity
work is implemented; those concrete remaining boundaries prevent claims of full
world/format compatibility. The user accepted the fidelity changes and then
confirmed the display correction, satisfying the repository AGENTS.md desktop
confirmation requirement for the local interactive commit. No push/merge approved.
