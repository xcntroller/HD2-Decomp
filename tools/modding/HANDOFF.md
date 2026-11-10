# Modding-tools handoff — 2026-11-10

Latest milestone: integrated read-only 4DS Model Inspector / OpenGL viewer,
following the completed script and Mission Explorer milestones. Only
`tools/modding` changed. Main/client/server, shared handoffs/plan/function ledger
and original assets remain untouched. Verified local checkpoint only; no push or
merge without renewed approval. Earlier branch delegation permits local review
checkpoints; no new human desktop acceptance is claimed.

Launch `make -C tools/modding gui` → **Model inspector…** → **Open installation…**,
search a model and open it. Props/buildings/character-related resources render
actual decoded geometry with RGB BMP previews. Orbit, pan, zoom, wire/solid,
manual LOD, grid/axes/bounds, hierarchy highlight, frame-origin selection and
properties are integrated in the same app. Linux X11/XWayland uses system
libGL/libX11 through ctypes, no new engine/pip/build dependency. A local .4ds also
opens independently, with an explicit fallback material when context is absent.

Mission Explorer recognizes model records and texture mentions. Follow an existing
actor/object candidate → select .4ds → **View model**, then **Back to mission
record**. Existing certainty/profile/Patch/invalid-selection barriers are retained;
no invented actor links, runtime cache selection or mission placement. Headless:
`python3 tools/modding/model_inspect.py --help`. Model metadata JSON is schema1;
mission/script schemas remain2. Reports stream and refuse original folders or
existing files.

`model4ds.py` / `model_math.py` are GUI-independent; `model_resources.py` wraps
ResourceStore and bounded BMP decoding; `gl_view.py` owns GLX resources;
`model_gui.py` integrates browsing/properties. Exact structural formats, native
addresses, confidence distinctions, capacities, statuses and tests:
[MODEL4DS](MODEL4DS.md), [static evidence](model4ds-evidence.json).
Supported: version0x29, established material table, common frame hierarchy/TRS,
visual0/1 meshes, LODs, aliases, triangle/material references and positions;
normal/UV preview interpretation is corroborated, not a reconstruction of graphics
declarations. Fixed dummy/joint and visual2 binding tails retain opaque fields.
Skin meshes are undeformed and explicitly partial. Unknown types/layouts stop
without guessed resynchronization; invalid supported data renders no geometry.

Verified nine real models against an independent C++ stream reader and original
byte slices, with focused ASan/UBSan/LeakSanitizer on that audit reader. Actual BMP
pixels match Pillow (audit only). A3444-member selected model corpus checked table
endpoints/parents/indices:1496 decoded structural subsets,1916 partial,20
unsupported,12 numeric guards. Sequential decoder audit peak80244KiB. Synthetic
truncation/unknown/alias/index/matrix/BMP and200 malformed-input checks passed.

Actual native toolkit viewport rendered textured crate/barrel/building/helmet,
complex prop, undeformed hands and checksum-verified Patch `b_drevo2`. Rendered
images were visually inspected beyond native assertions. GUI checks cover mouse
orbit/pan/wheel, LOD, shared GL textures, hierarchy highlight, read-only export,
invalid clearing and repeated context replacement/reopen. Tutorial actor→en09 and
ALPS1 actor→e_elbx1 candidate/view/return paths passed. See the contract for
memory/lifecycle observations and remaining rendering limitations.

No original executable/DLL/Proton or Ghidra execution, heavy game build, permanent
tests, commercial assets or raw captures in commits. Temporary audits remain
outside Git and are removed after verification. All normal asset access is
read-only and bounded; whole archives are not held in RAM.

The independent plan is complete for the requested milestone. Further formats,
full texture/blend/effect/skin/animation semantics, original cache search,
mission placement, editing, serialization and runtime/debugging are excluded.
Prior completed contracts: [scripts/includes](SCRIPT-ANALYSIS.md),
[ISD1](ISD1.md), [formats](FORMATS.md), [resolution](RESOLUTION.md).
