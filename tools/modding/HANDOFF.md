# Modding-tools handoff — 2026-11-10

Latest work: **spatial script intelligence**, extending the user-accepted fidelity
checkpoint2a1c8f3. Start with [SPATIAL-SCRIPTS.md](SPATIAL-SCRIPTS.md) for exact
relationships, original addresses, limits, schemas and verification. Only
tools/modding changed; client/server/shared handoffs/plan/ledger and original
assets are untouched. No push/merge.

Implementation and toolkit desktop verification are complete. The independent
`spatial_scripts.py` query layer is a verified source-only local checkpoint.
**The interactive panel/overlay integration remains uncommitted pending user
desktop confirmation**: repository AGENTS.md requires “Changes to interactive
behavior require user desktop confirmation before committing.” Existing fidelity
acceptance does not cover this new interaction. Next step: user recheck below,
then record acceptance and commit the remaining verified tools/modding changes
locally. No further unchecked development objective is added here.

Run `make -C tools/modding gui` → Mission explorer → Open installation → Tutorial
→ Inspect mission → Explore 3D → select an instructor/soldier mesh → **Script
relationships**. The tab beneath the viewport keeps source/evidence alongside
3D. Browse bindings/events/references, search/filter incoming/outgoing, enable
Static links/Labels, select a line, inspect its exact source, use Source/Target
object and Back to context, and open/return from the existing Script Inspector.
TUT_Instructor9 is a verified original Human example; no fabricated demo link.
A viewport-only preview may remain at/tmp/hd2-spatial-script-preview.png.

`ScriptRelations(scene)` is independent of Tk/GL. Typed binding/reference/overlay
records reuse original graph/statement/instruction identities and certainty.
Selected imported members expose their actual model owner's binding with that
scope labelled; generic ancestors do not inherit VM ownership. Exact serialized
binding-name matches remain static observations, not original runtime acceptance.
Unique ASCII-insensitive scene-name matches are candidates; duplicates/case
variants remain ambiguous. Runtime ScriptAssign receivers, dynamic names and
child-frame parents are not evaluated and get no guessed spatial endpoints.

`spatial_script_gui.py` supplies the docked panel and original source/evidence;
`spatial_overlay.py` draws bounded X-ray candidate lines with native edge-ID
picking and optional context-owned X11 font labels. Sources require retained
map identity, bounded size and matching SHA256. Original include/profile/Patch
resolution is preserved; selected invalid data never becomes an alternate source.
Worker requests are bounded/coalesced, stale generations discarded, old edges
cleared on reload and font resources released with the GL context. Original
world transforms/model rendering remain unchanged.

Queries return250 primary rows, cap scans12000 and overlays96, preserve up to64
ambiguous/source-binding contexts and mark lower-bound totals explicitly. Full
spatial schema2 adds coverage.scripts; neighborhood schema1 has filtered/paged
rows, original object/script provenance and evidence-linked edges, no asset/source
bodies. GUI exports new files outside the installation. Headless example:
`spatial_inspect.py /path/to/game tutorial --inspector /tmp/hd2-inspect
--script-object TUT_Instructor9 --relationship-search FRM_FindFrame --output
/path/outside-game/neighborhood.json` (one shell command).

Verified local original-profile observations: Tutorial/ALPS1/Arctic1 have114/148/
105 binding records and131/138/83 retained analyzed scripts, including2/8/2 Patch
selections. All1337 compiled reference source spans were checked. Distinct placed
reference contexts250/438/385; nonspatial/unresolved/self-only99/89/76. This is
retained analysis coverage, not whole-installation or runtime compatibility.
Human/Switch/SpawnPoint and actual Arctic1 Door examples are evidenced; actors
without bindings say so honestly. Existing format/fidelity limitations remain.

Focused temporary checks cover original graph IDs/certainty, source spans/includes,
world endpoints, incoming/outgoing union, filters/paging/scan limits, deterministic
JSON/write barriers, ambiguous/case-variant names, runtime-only/dynamic/child
references and changed/foreign/oversized source protection. A real AFRICA3 include
construct reaches its actual original file/span. Native X11/GLX checks pick a real
Tutorial Human mesh, browse events/references/source, open/return from Inspector,
select labelled edges, navigate endpoints/back with camera preserved, and follow
a script resource into the same Explorer session. Actual ALPS1/Arctic1 loads,
Door/Switch/SpawnPoint source inspection, Arctic1 reload and close during pending
queries pass. Captured real panel/viewport images were visually inspected; no
exercised GL/Tk errors. Peak RSS648196KiB is an observation, not a leak-free claim.

Existing native compiler build is up to date; Python compilation/diff checks
pass. No new C++ parser or compiler path, original EXE/DLL/Proton/Ghidra execution,
heavy concurrent build or permanent test was introduced. Temporary audit fixtures,
reports/captures are outside Git and removed after final verification; only the
optional viewport preview remains. No original commercial assets in commits.
Runtime execution, simulation/debugging, mission editing and binary serialization
remain excluded. More read-only reconstruction is essential before safe editing.
Prior contracts: [fidelity](FIDELITY.md), [spatial](SPATIAL.md),
[scripts/includes](SCRIPT-ANALYSIS.md), [4DS](MODEL4DS.md), [ISD1](ISD1.md).
