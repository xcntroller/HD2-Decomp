# Modding-tools handoff — 2026-11-10

Latest milestone: **integrated read-only 3D Mission Explorer**, following completed
4DS, Mission Explorer and scripting milestones. Only `tools/modding` changes.
Original assets, client/server reconstruction and shared handoffs/plan/function
ledger remain untouched. Local checkpoint only; no push/merge without renewed
approval. Earlier branch delegation permits verified local review checkpoints;
no fresh human desktop acceptance is claimed.

Launch `make -C tools/modding gui` → **Mission explorer…** → **Open installation…**
→ choose profile/mission → **Inspect mission** → **Explore 3D / selected object**.
The existing app now renders actual base Scene.4ds and supported original placed
models. Orbit/focus, WASD/QE fly + right-drag look, Shift/speed/range, click picking,
hierarchy/search, wire/solid, visibility, textures, bounds and annotation controls
work. Select a record in Explorer to navigate into 3D; inspect original record,
cached model or associated script and return to the selected object. Large
backdrop geometry can make Fit distant; zoom or focus a hierarchy entry.

This is a **partial cold serialized spatial snapshot**, not a complete original
mission renderer or runtime scene. Unknown parent/sector selection withholds
world transforms; fallback positions never become guessed mesh placements.
Original actor initialization/movement, scene paths/cache scopes, lightmaps,
lighting/alpha, animation/skin and other unknown format consumers remain excluded.
Gray means material fallback. See [SPATIAL.md](SPATIAL.md) for exact original
addresses, confirmed/inferred/unknown boundaries, resource certainty, budgets,
verification counts and visual limitations. [Static fingerprints](spatial-evidence.json)
retain evidence without raw code/captures or normal-operation binary dependencies.

`spatial_scene.py` is GUI-independent: ordered original stage composition, stable
IDs/offsets, explicit local/parent/world status, actor binding and existing graph
links, shared assets and streamed deterministic JSON. Scene2 raw WXYZ reorders to
normalized engine XYZW per original006CDE38..006CDEC8/0041D180;4DS remains direct
XYZW. Row matrices compose local*parent. The portable reconstruction reader's
quaternion pass-through discrepancy is documented, not changed. Exact default
Models-root selection is explicit; full engine cache search is not claimed.

`spatial_camera.py` separates viewer motion from original transforms;
`mission_view.py` extends existing native GLX with shared VBO/texture ownership,
culling/limits and selection; `spatial_gui.py` integrates the existing windows.
Sector5/portal and occluder12 extent contracts now traverse opaque volume tails,
unlocking genuine later scene geometry. They remain partial and non-rendered.
Headless: `python3 tools/modding/spatial_inspect.py --help`; JSON schema1 is
metadata, not a binary/asset serializer. Existing mission/script schemas stay2.

Verified Tutorial/ALPS1: base frames1923/1923 and790/790; spatial nodes6345/7211;
assets106/73; visible supported mesh instances3043/3709. ALPS1 uses checksum-
verified Patch Scene2. World/fallback position comparisons1148/1148 and1361/1361
match. Unresolved transforms101/398 including descendants; actor links78 resolved
+2 unresolved /134 resolved. All root-profile assets available in these examples.
This does not establish full format/world compatibility.

An independent temporary C++ reader matches every base frame offset/end/type/
parent/world matrix, vertex/triangle count and exact file end. Original base hashes
match three previous captures each. ASan/UBSan/LeakSanitizer passed for that reader;
focused Python malformed/transform/ordering/ambiguity/cycle/camera/export checks
passed. Actual native desktop renders and closer/ground-level captures were
visually inspected. GUI tests cover native input, picking/tree, record/model/script
navigation, filters/search, repeated reloads and GL resource deletion. Details and
memory/performance observations are in SPATIAL.md. This is actual toolkit desktop
verification, not execution of original code or a fresh user acceptance report.

No original executable/DLL/Proton/Ghidra execution, heavy game build, original
asset/capture/report commits or permanent tests. Temporary audits are removed
after verification; a `/tmp/hd2-mission-explorer-preview.png` viewport preview may
remain outside Git for user review. Original installation is always read-only.

The independent plan is complete for this requested milestone. Next work needs a
new scoped objective: exact automatic-sector/scene-path/cache relationships,
additional original geometry consumers or material/rendering semantics. Editing,
binary writing/serialization, simulation and debugging remain excluded.
Prior contracts: [4DS](MODEL4DS.md), [scripts/includes](SCRIPT-ANALYSIS.md),
[ISD1](ISD1.md), [formats](FORMATS.md), [resolution](RESOLUTION.md).
