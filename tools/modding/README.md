# H&D2 Modding Toolkit

A standalone, read-only script inspector, Mission Explorer and 3D model/mission viewer on
`feature/modding-tools`. Normal operation needs no Ghidra or original executable.
Use your own local installation; no commercial assets or scripts are included.
No game, loader, server or runtime implementation is modified.

## Start

From the repository root:

```
make -C tools/modding
make -C tools/modding gui
```

The small C++17 inspector builds sequentially to `/tmp/hd2-inspect`; Python is
needed to generate checked inspection overlays during the build. The GUI needs
Python with Tk. There are no pip dependencies, network services or game builds.
After a reboot, rebuild the temporary native binary. To select another binary:

```
python3 tools/modding/gui.py --inspector /path/to/hd2-inspect
```

## Script inspection

**Open script…** (Ctrl+O) opens a read-only local source snapshot. **Inspect again**
reads a fresh snapshot. Supported syntax acceptance is not proof of original-game
compatibility, valid runtime behavior or execution.

- **Source:** lexer highlighting, diagnostic underlines, exact byte navigation
  and a root/included-source selector. Display uses Latin-1, normalized line
  breaks and visible control-byte escapes. Reopened sources must match their hash.
- **Analysis:** searchable diagnostics, events, declarations, labels, controls,
  ordinary arguments, symbol uses, references, includes and statements. Filter by
  kind, page through results, and double-click to navigate to the original source.
  **Follow mission reference** uses linked entities in the existing Explorer.
- **References / Symbols / Instructions:** compiler-backed products; double-click
  for source navigation. These tables show the first2000 rows; Analysis covers all.
- **Overview:** syntax status, compiler counts and explicit limitations.
- **Command reference:** searchable recovered command/predicate tables. A table
  entry does not promise known runtime semantics or actor applicability.
- **JSON:** bounded preview; **Export JSON…** streams the complete schema2 report
  to a new file outside the inspected sources/installation.

Analysis runs in a background worker. Errors have structured categories, severity,
source positions and evidence. Rejected compilation retains observed syntax,
while withholding compiled PCs/events/variables. Lexical mentions are separate
from instruction products and never asserted to execute.

To resolve includes for a standalone local file, choose **Include context…**,
select the original installation and enter the mission namespace, then **Inspect
again**. This uses the default loose/Patch/base profile. For explicit alternate
language/archive-first profiles, open the script through Mission Explorer.
An include without a caller-supplied context is rejected rather than guessed.
The original shared-cursor preprocessing is unusual; some nested/malformed cases
are deliberately rejected as tool restrictions. See [exact coverage](SCRIPT-ANALYSIS.md).

Headless structured analysis (no Tk):

```
python3 tools/modding/analyze_script.py /path/to/source.scr
python3 tools/modding/analyze_script.py scripts/tutorial/example.scr \
  --installation /path/to/game --resource tutorial
```

Installation mode takes a virtual resource path, retaining selected Patch/loose
precedence and include provenance. Add `--archives-first` or `--language English`
to choose an explicit profile. Output is JSON to stdout; exit0 means recovered
syntax accepted,1 means rejected/unsupported,2 means invocation/read failure.
Redirect reports outside the installation and do not commit proprietary outputs.

The native CLI also works without Python after building:

```
/tmp/hd2-inspect my-script.scr
/tmp/hd2-inspect --json my-script.scr
/tmp/hd2-inspect --catalog > command-reference.json
```

Native `--json` exposes schema2 token/statement/argument spans, diagnostics,
symbols, instructions, event tables and established receiver references. It
requires prepared input when includes occur. `--prepared` explicitly selects
already-preprocessed bytes; `--lex-one BYTE_OFFSET FILE` exposes one recovered
lexer read for the faithful Python include pipeline. The Python client adds
original-source maps, explicit events/control views and include resolution.

## Integrated Mission Explorer

**Mission explorer…** opens the existing child browser in the same application.
Choose **Open installation…**, set language and loose-first/archive-first profile,
select Tutorial/another discovered mission, then **Inspect mission**. **Refresh**
reindexes changes. The installation remains strictly read-only.

Browse recognized mission files and documented records, including supported
Scene2 object/actor fields, tables and script bindings. File/entity details retain
record IDs, offsets, unknown data and archive provenance. Separate Content and
Archive statuses distinguish partial parsing from verified decoded bytes,
unavailable compression and invalid data. Verified ISD1 raw/LZSS supports selected
Patch resources; unsupported sound codecs and invalid selections never fall back
to shadowed copies.

**Actors / objects**, **Dependency graph** and **Validation** provide paged search.
The graph distinguishes serialized/selected-resource relationships from name
candidates. Source statements link to compiler instructions, established receiver
references and include dependencies. It is a relationship diagram, not a map.

Select a script and choose **View script** to use the same source/analysis tabs,
with the actual installation profile and include context. Local folders and
optional reference folders also work through **Open mission directory…**;
without explicit installation context their script includes remain unresolved.
Unknown types/fields remain labelled opaque or unsupported.

Headless mission inspection and deterministic streaming exports:

```
python3 tools/modding/installation.py /path/to/game
python3 tools/modding/installation.py /path/to/game --mission tutorial \
  --inspector /tmp/hd2-inspect --output /path/outside-game/tutorial.json
python3 tools/modding/mission.py /path/to/Missions/Tutorial \
  --reference-root /path/to/loose-assets \
  --inspector /tmp/hd2-inspect --output /path/outside-inputs/tutorial.json
```

Without `--inspector`, scripts are indexed. Installation binding/assignment
references are expanded within explicit budgets; unbound sources stay indexed
until selected. A produced mission report can contain errors/unsupported records:
inspect per-file status, validation and coverage rather than the process exit code.
Mission exports require new files outside protected input folders.

## 4DS models and 3D viewing

For **interactive missions**, inspect an installation-aware mission in Mission
Explorer and choose **Explore 3D / selected object**. The integrated window renders
actual supported base Scene.4ds geometry and resolved Scene2/model placements.
Use orbit/focus or fly with WASD, Q/E, Shift and right-drag look. Click meshes or
search the hierarchy; inspect original records, models and associated scripts
and return to the selected 3D object. Category filters, wireframe, grid/axes/origins,
bounds, range/speed controls and loading diagnostics are available.

This is a **partial serialized spatial snapshot**: unresolved parenting is withheld,
and original lighting/lightmaps, animation and runtime actor/script movement
are excluded. Supported alpha-source previews use approximate blending/cutoffs.
Gray materials indicate texture fallbacks. Current Tutorial/ALPS1/Arctic1 coverage,
sector/resource evidence and native desktop checks: [FIDELITY.md](FIDELITY.md);
controls/composition: [SPATIAL.md](SPATIAL.md). Headless deterministic metadata export:

```
python3 tools/modding/spatial_inspect.py /path/to/game tutorial \
  --output /path/outside-game/tutorial.spatial.json
```

Choose **Model inspector…** to open the integrated model browser and viewport.
Use **Open installation…**, search an exact model resource, and double-click it.
Local .4ds files also open independently, with explicit fallback materials when
no installation context exists. No asset extraction directory is required.

The OpenGL viewport supports orbit (left drag), pan (middle/right or Shift+left),
wheel zoom, solid/wireframe, grid, axes, bounds, manual LOD selection and mesh/frame
highlighting through the hierarchy. Properties show original offsets, identifiers,
transforms, geometry samples, materials, texture dependencies and archive provenance.
Supported BMP materials show RGB/RGBA previews with explicit material/alpha
status; unsupported textures/effects
have clearly labelled fallbacks. This is a read-only viewer, not a mission editor.

In Mission Explorer, follow an existing actor/object model candidate, select the
recognized .4ds file and choose **View model**. **Back to mission record** returns
to the originating record. Candidate links stay candidates; this does not invent
engine cache selection or actor placement. Installation profiles and Patch
selection are shared with the existing resolver; invalid selected data never
falls back to a shadowed asset.

The decoder supports an evidence-backed subset of original H&D2 version0x29:
common frames/hierarchy, standard mesh LODs, shared geometry, indices, positions,
normal/UV preview fields, materials and local/world transforms. Skin bindings,
unknown payload semantics and unsupported frame/visual types remain partial or
explicitly unsupported. Real geometry is used for every displayed mesh.

```
python3 tools/modding/model_inspect.py models/ammocrate.4ds \
  --installation /path/to/game --output /path/outside-game/model.json
python3 tools/modding/model_inspect.py /path/to/local.4ds
```

Headless decoding/export does not need a desktop. The viewport uses Tk plus system
libGL/libX11 on Linux X11/XWayland, with no game engine or extra Python rendering
package. [4DS contract, confidence levels and actual rendering verification](MODEL4DS.md)
describe exact coverage and limits.

## Contracts and limitations

- [Spatial composition, interactive mission viewing and current verification](SPATIAL.md)
- [4DS models, OpenGL viewer and current verification](MODEL4DS.md)
- [Mission fidelity, coverage and editing readiness](FIDELITY.md)
- [Structured scripts, includes, schema and earlier verification](SCRIPT-ANALYSIS.md)
- [Mission format/model coverage](FORMATS.md)
- [Original lookup policy and bounds](RESOLUTION.md)
- [ISD1 evidence and archive verification](ISD1.md)
- [Current independent handoff](HANDOFF.md) and [plan](PLAN.md)
- [Earlier integrated verification](INTEGRATED-VALIDATION.md) and
  [initial validation](VALIDATION.md) describe historical checkpoints.

The verified lexer/compiler/command/context research is reused. Inspection-only
header overlays have checked hashes/anchors; runtime headers remain read-only.
Supported declarations are Integer/String/Frame; Bool/Enum/Real families remain
outside recovered support. Ordinary command signatures, original expression
quirks, labels/jumps, event registrations and Whenever structures are exposed;
expression trees/data-flow, runtime predicates, debugger/execution and editing
are excluded. Only the three evidenced string receiver references are interpreted.
String variables remain dynamic even if initialized to literals. Model/frame
candidates do not prove engine resolution.

Source/token/expansion, recursion/depth/operator and session retention limits
are explicit. Oversized reads and embedded NUL are rejected instead of silently
reproducing original prefix/truncation behavior. Partial mission format support
is never reported as complete. Sabre Squadron/expansion profiles, full asset
formats, runtime placement/simulation, binary editing and serialization are not implemented.

Original files are never modified. Temporary fixtures are removed after focused
verification; no permanent tests, extracted commercial data or proprietary
reports belong in commits. Local checkpoints only; no push/merge without approval.
