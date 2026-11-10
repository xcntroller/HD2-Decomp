# Modding-tools handoff — 2026-11-10

Latest work: **3D Mission Explorer fidelity and comparable coverage**, following
user-tested interactive spatial viewing. Read [FIDELITY.md](FIDELITY.md) for exact
original addresses, structural/material contracts, three-mission verification,
budgets and remaining blockers. Only `tools/modding` changed; original assets and
client/server/shared handoffs/plan/function ledger are untouched. No push/merge.

User desktop acceptance: after reporting a flipped world, the user rechecked the
positive-Z display correction and confirmed **“Looks good; commit.”** This accepts
the fidelity checkpoint and corrected orientation and authorizes its local commit.
No broader case-level testing or full renderer compatibility is inferred. No
permanent tests, extracted assets/reports or raw code/captures belong in commits.

Start `make -C tools/modding gui` → Mission explorer → Open installation → choose
profile/mission → Inspect mission → Explore 3D / selected object. Existing input,
picking/tree, record/model/script navigation remain integrated. Material properties
now distinguish source decoding, supported RGBA previews, unsupported-alpha RGB
fallbacks and gray materials. Coverage distinguishes known world matrices from
unknown sector parent identities; mesh omissions and original unsupported records
remain visible. A temporary `/tmp/hd2-fidelity-preview.png` shows corrected Tutorial
geometry outside Git.

Original10009B30/10009CC0 establishes positive-Z clip W. The previous preview
used negative-Z OpenGL camera handedness directly, causing mirroring. Both
viewports now use the same positive-Z camera basis and explicit OpenGL view
conversion in spatial_camera.py. Movement, culling, bounds/origins and picking
share it; serialized vertices/world matrices and export are untouched. Focused
math/native asymmetric display-and-pick checks, actual Tutorial/ALPS1 rendering,
standalone shared-model viewing and lifecycle passed. Corrected Tutorial capture
was visually inspected; ALPS1 close/obstructed views do not establish a matching
gameplay viewpoint. Original runtime was not executed. See FIDELITY.md.

`spatial_scene.py` registers imported instance.member names in original load order
and supports unique ASCII-insensitive scene fallback after exact scratch lookup.
A guarded translation-only sector invariant establishes world TRS independently
of unknown sector identity; it never guesses which sector won. Complete model
frame tables/no failed assets/no changed or rotated sectors are required. Parent
remains unresolved with candidate IDs. Arbitrary dotted path splitting, full
registration/traversal scope and exact containment/list order are not implemented.

`resource_lookup.py` separates explicit ordered directory profiles from exact
ResourceStore archive selection. Model roots search forward; native texture
probes use direct / last successful directory / reverse registered order. Client
and engine suffix stripping are separate. Patch/profile/CRC/codec barriers remain;
selected invalid data never becomes another root or shadowed archive. Descendant
registration enumeration and warm/global caches remain outside scope.

`model4ds.py` now traverses verified visual3/4/5 tails, visual6 effect records,
target7 and common8 bodies. Geometry remains undeformed; animation/target/effect
consumers remain partial. `material_preview.py` retains established opacity and
alpha-source interpretation independently of `gl_material.py` preview state.
Texture caches budget source RGB/indices/alpha and derived RGBA; geometry and GPU
resources remain shared between instances. No original lighting/lightmaps, skin,
animation, runtime actor/script execution or full visibility/LOD reproduction.
Some vegetation rectangles remain; RGBA8 keying is not every native device branch.

Current observations (not universal format claims): Tutorial/ALPS1/Arctic1 have
7473/9462/15017 nodes and106/73/141 assets; placed visible mesh instances
3223/4235/8388. World matrices withheld0/0/281; automatic sector identities still
unknown16/75/46. Actor bindings all unique in these examples. Fallback checks
1168/1168,1438/1438,1992/2039; Arctic1's47 disagreements remain explicit, with no
guessed correction. ALPS1 Scene2 is the selected CRC-verified Patch ISD1 resource.
Full spatial export schema2 includes deterministic coverage schema1; use
`spatial_inspect.py ... --coverage-only --output ...` for comparable reports.

Verified: independent temporary native reader matched306 unique selected model
frame tables/end offsets; ASan/UBSan/LeakSanitizer passed for that audit reader.
Focused synthetic bounds/truncation/lookup/alpha/invariant/export checks passed.
Real BMP source pixel comparisons327/288/531 passed. Actual native X11/GLX scenes
and images were exercised/visually inspected for all three missions, including
alpha-aware picking and shared model viewing. Repeated Arctic1 reloads and final
close deleted4042 buffers each with immediate handle checks; peak RSS about493MiB
in that desktop audit, not a universal leak-free guarantee. Details in FIDELITY.md.
No original executable/DLL/Proton/Ghidra or heavy game build was run. Temporary
audits are removed after verification; only the optional preview remains in/tmp.

The selected plan checkbox is implemented with concrete partial boundaries.
**Safe mission binary editing is not ready.** Further read-only work must address
exact sector containment/registration scope, Arctic1 transform discrepancies,
material/device-alpha/lightmap/visibility consumers, unknown records/streams and
round-trip preservation. Editing, binary serialization and runtime simulation
remain excluded. No additional unchecked development item is authorized here.
Prior contracts: [spatial](SPATIAL.md), [4DS](MODEL4DS.md),
[scripts/includes](SCRIPT-ANALYSIS.md), [ISD1](ISD1.md),
[formats](FORMATS.md), [resolution](RESOLUTION.md).
