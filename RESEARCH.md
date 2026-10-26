# Technical research guide

For the plain-English overview and milestone tracker, see [the README](README.md).
This guide preserves the detailed evidence, limitations, and tool instructions.

An early-stage effort to understand Hidden & Dangerous 2 and its LS3D engine.
The reference installation is the Steam release with Sabre Squadron, running
on Linux through Proton.

**Current result:** the outer UPX packing layer has been removed from copies of
both client executables. Static analysis of the base-game client has identified
startup, the main loop, and a mission-loading pipeline.
Selected ALPS1 mission resources have also been extracted locally, and the
`scene2.bin` container structure and selected object fields have been matched
to their loader and engine consumers.

**This is not a playable source reconstruction.** We have not run the unpacked
copies, verified these findings under a debugger, or established that every
remaining startup/protection mechanism is understood.

## What is actually happening?

The game executable contains machine instructions, not the original source.
Ghidra translates those instructions into an assembly listing and approximate
C-like code. We follow calls, filenames, and engine interfaces to work out what
the code does, then test those interpretations.

- **Unpacking:** recovering code compressed inside an executable.
- **Disassembly:** displaying machine instructions as readable assembly.
- **Decompilation:** estimating higher-level code from those instructions.
  This does not recover the original names, types, or comments.
- **Static evidence:** what we can establish from files without running the game.
- **Runtime verification:** checking that an interpretation matches the running
  game, for example by stopping at a function while a mission loads.
- **Address:** a location in the loaded program. Addresses below are preferred-base
  virtual addresses for one exact binary, not file offsets.

Getting readable pseudocode is a foothold, not proof that the interpretation is
correct. There is no meaningful overall completion percentage yet.

## Progress tracker

Checked items are bounded milestones, not claims that an entire subsystem has
been reconstructed.

### Completed

- [x] Fingerprint the original client binaries and inspect their PE structure.
- [x] Reproduce stock UPX's rejection of both clients.
- [x] Restore erased UPX identifiers on separate copies using a reproducible tool.
- [x] Pass UPX integrity tests and unpack both clients.
- [x] Establish headless Ghidra access without running game code.
- [x] Explain the initial entry-point decompiler failure: inline strings were
  mistaken for instructions.
- [x] Find the startup wrapper's on-disk jump target and decompile runtime startup.
- [x] Create a separate research project so the GUI project can remain open.
- [x] Identify base-game application startup, graphics initialization, and main loop.
- [x] Identify a mission-load orchestrator and concrete scene-resource references.
- [x] Record a map with 18 functions, 17 direct-call edges, and 8 resource anchors;
  validate those references against local exports and the binary fingerprint.
- [x] Extract ALPS1 `scene.4ds`, `loader.4ds`, and `scene2.bin` from the base archive.
- [x] Validate the scene root, object records, and immediate field boundaries.
- [x] Fix the recognized stack-probe model in the separate research project.
- [x] Add tested native archive extraction and scene-structure inspection tools.
- [x] Repair three archive import signatures and three chunk helpers using
  assembly-supported argument counts and calling conventions.
- [x] Decode object names, creation-kind requests, position/rotation/scale,
  fallback position, and parent-name containers.
- [x] Verify exact sample counts and selected field-consumer call sites with
  a repeatable evidence validator.

### Next: one mission-loading path

- [ ] Correct remaining helper and virtual-interface signatures in a working
  project, not the baseline.
- [ ] Decode object-field meanings and how records become engine objects.
- [ ] Resolve frontend/controller virtual dispatch into the gameplay-load caller.
- [ ] Verify selected loading stages against the original game under Proton.

### Later

- [ ] Recover relevant engine interface/vtable layouts.
- [ ] Investigate unresolved opaque calls through `00A59000`.
- [ ] Compare base-game findings with Sabre Squadron; do not reuse addresses blindly.
- [ ] Choose and investigate a bounded gameplay system, such as squad orders.
- [ ] Build tested parsers or behavioral reconstructions where evidence supports them.

Full playable reconstruction remains a long-term possibility, not a promised
deliverable.

## The map so far

Names here describe inferred roles; they are not original developer symbols.

| Address | Interpretation | Confidence |
| --- | --- | --- |
| `007E8EFD` | C/C++ runtime startup | High |
| `006DD550` | WinMain-like application entry | High |
| `006DCAE0` | Application lifecycle and main loop | High |
| `006DBFC0` | Platform and engine setup orchestration | High |
| `006DB0F0` | Graphics interface initialization | High |
| `006DC950` | Frame interval calculation and limiting | High |
| `006C20F0` | Controller update and state transitions | High |
| `006989B0` | Gameplay activation / mission-load caller | Medium |
| `006CA780` | Mission-loading orchestration | High |

The mission-load orchestrator calls preparation code, a stage that loads
`missions\%s\Scene` through an engine interface, and another stage referencing
`missions\%s\scene2.bin`. It also references `tree.klz` and invokes effects loading.
The exact virtual-dispatch connection from the frontend to this path is still
unresolved.

See [the machine-readable map](analysis/initialization-map.json) for call sites,
evidence, confidence, and limitations. Its addresses apply to the unpacked
base-game client with SHA-256:

```text
8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307
```

High confidence means strong static support for a role, not runtime confirmation.
Some loader pseudocode still has poor stack recovery. Ghidra's recognized function
count is not a count of understood functions.

## First mission-format milestone

See [the mission findings](analysis/mission-format.json) for sample fingerprints,
static anchors, counts, and limitations.

The ALPS1 sample comes specifically from the **base `missions.dta` archive**,
not an established runtime/patch overlay. The native reader extracted three
members without using game DLLs or executing game code:

| Resource | Bytes | Observation |
| --- | ---: | --- |
| `scene2.bin` | 6,727,229 | Root size exactly matches the file |
| `scene.4ds` | 1,380,788 | `4DS\0` header, version `0x29` |
| `loader.4ds` | 3,901 | Same header/version |

`scene2.bin` uses six-byte little-endian headers: a two-byte ID and four-byte
**total size including the header**. Its root ID is `0x4C53`, with 15 direct
children. The `0x4000` container at file offset `0xFA` contains exactly 4,008
`0x4010` records. Every record's immediate children also consume its payload
exactly; all have one `0x0010` field containing a NUL-terminated name-like string.
This is not an enemy count: records include scene objects whose roles are
mostly still unknown.

Static code corroborates this framing: `006CBF30` checks the root, `006CC760`
dispatches top-level chunks, and its `0x4000` handler calls `006CD940` for
`0x4010` records. `00405940` implements a read bounded by the current chunk.
The inspector descends only through these understood container levels; it does
not guess that arbitrary payloads are nested chunks.

### Improving the decompilation

Assembly at `007E5F60` implements an x86 stack-probe/allocation helper. The object
reader requests `0x23F8` stack bytes through it, but Ghidra had no call fixup
attached. Applying its built-in `alloca_probe` model removed the impossible
stack array and reduced `in_stack_` occurrences in `006CD940` from 122 to zero.
This improves analysis, not the executable: no game bytes were modified.
Type propagation warnings and incomplete helper/interface signatures remain;
the resulting pseudocode is **not** a recovered, trustworthy C implementation.

The fingerprint-guarded [ConfigureStackProbe.java](tools/ConfigureStackProbe.java)
script reproduces this metadata change. Use it **only in the independent
research project**, without `-readOnly` if saving:

```sh
"$GHIDRA_HOME/support/analyzeHeadless" "$PROJECT_DIR" HD2Research \
  -process hd2.unpacked.exe -noanalysis -scriptPath "$PWD/tools" \
  -preScript ConfigureStackProbe.java \
  -postScript ExportFunctions.java "$PWD/analysis/work/probe-export" 006cd940 \
  -max-cpu 2
```

### First readable object fields

The next obstacle was missing argument lists: Ghidra's archive imports were
marked `__stdcall` but had no parameters. The DLL export decorations and caller
instructions establish three stack arguments for `dtaRead`, `dtaWrite`, and
`dtaSeek`. Three chunk helpers also needed explicit `__thiscall` signatures.
[ConfigureChunkReaders.java](tools/ConfigureChunkReaders.java) applies these
fingerprint-guarded corrections in the research project. Reopening that project
and applying the script again produced unchanged signatures.

The chunk-entry helper now correctly recovers the six-byte header and output
ID. In `006CD940`, bare `dtaSeek()` calls dropped from 96 to zero, and the
pseudocode became shorter without deleting any instructions. That allowed
selected fields to be followed into their consumers:

| ID | Decoded interpretation | ALPS1 occurrences |
| --- | --- | ---: |
| `0x0010` | Object name | 4,008 |
| `0x4011` | Creation-kind request, unsigned 32-bit | 1,275 |
| `0x0020` | Position, three float32 components | 1,292 |
| `0x0022` | Quaternion rotation, file order `w,x,y,z` | 1,332 |
| `0x002C` | Fallback position for failed parent-name lookup | 1,275 |
| `0x002D` | Scale, three float32 components | 1,277 |
| `0x4020` | Parent reference with a nested name field | 1,200 |

The position consumer writes frame offset `+0xB0`. Rotation is reordered to
engine `x,y,z,w`, copied to `+0xC0`, and normalized through the named
`S_quat::Normalize` import. Scale reaches `+0xD0`; the named engine
`I3D_frame::UpdateWMatrixProc` uses its three components as basis multipliers.
All 1,332 sample rotations have squared length within `1e-6` of one. The tool
reports serialized components without normalizing them or calculating resolved
world transforms.

**Not every record creates an object.** The creation kind starts at zero for
each record. When a name field is encountered, zero selects lookup of an
existing name-table entry; nonzero selects a create-if-missing branch. In this
sample there are 2,733 lookup requests and 1,275 create-if-missing requests.
The latter contain kinds `2` (13), `3` (2), `6` (103), and `9` (1,157). These
numbers are not yet presented as authoritative engine class names.

The loader populates a name table through callback `006CC580` before parsing
the records. The callback reads a frame's name and stores its pointer in the
table. All 4,008 record names are unique, but only 200 of the 1,200 parent-name
references also name a record in this file. This does **not** establish 1,000
broken links: the name table has sources outside these records. We have not
resolved each reference or reconstructed the full scene hierarchy.

To reproduce both metadata corrections, use the separate research project
and a new export directory:

```sh
"$GHIDRA_HOME/support/analyzeHeadless" "$PROJECT_DIR" HD2Research \
  -process hd2.unpacked.exe -noanalysis -scriptPath "$PWD/tools" \
  -preScript ConfigureStackProbe.java \
  -preScript ConfigureChunkReaders.java \
  -postScript ExportFunctions.java "$PWD/analysis/work/typed-loader-export" \
  006cd940 00415c90 00405940 00405890 \
  -max-cpu 2
```

For optional field-by-field output, including local asset names:

```sh
python3 -B tools/inspect_scene2.py extracted/alps1-base/scene2.bin \
  --objects --output analysis/work/alps1-object-fields.json
python3 -B tools/validate_mission_sample.py --archive
```

Without `--objects`, the inspector's existing structural output is unchanged.
The optional decoder preserves field order, repeated fields, and unknown-field
IDs/lengths; it does not invent missing values. It currently supports ASCII
names, exact known payload widths, and finite float values. Parent containers
are inspected, but other unknown payloads remain opaque. Detailed output stays
under ignored `analysis/work/`; do not publish it as repository research.

The evidence validator checks fingerprints, exact counts, quaternion tolerance,
name-link observations, six repaired signatures and four selected direct calls
against local Ghidra exports. It does not prove every semantic interpretation.
It needs the ignored local inputs/exports and therefore cannot run from a fresh
clone alone. `--archive` additionally hashes the full base mission archive.

### Readable reconstruction versus decompiler output

Small, well-understood helpers are candidates for readable reconstructions with
English names, proper data types, and behavioral tests. Large loader exports
are not ready: unknown engine calls and poor type propagation must not be hidden
behind confident-looking names. Generated pseudocode stays local, separate from
our original tools.

The current Python parsers are **format-analysis tools**, not exact replacements
for game functions. In particular, they deliberately reject malformed inputs
more strictly than the observed loader. A future reconstruction should state its
supported behavior and deviations explicitly rather than claim source recovery
or binary equivalence.

### Archive format and prior art

The [native extractor](tools/extract_dta_member.py) supports **ISD0 only**:
XOR-decoded index/member metadata, raw blocks, and LZSS/RLE blocks. It reads the
two 16-bit block-size fields separately and checks the previous-block chain.
It rejects unsupported codecs, invalid lengths/backreferences, truncation, and
incorrect output sizes rather than manufacturing fallback bytes. It accepts
an explicit destination, never uses archive paths as output paths, and refuses
to overwrite existing files.

Format facts and the public base-archive keys were investigated using
[jovan-s/DTAUnpacker at faecc881](https://github.com/jovan-s/DTAUnpacker/tree/faecc88143e74adbf116bbf4324c7702f14d31d7)
and [Richard01CZ/Mafia_DTAExtractor](https://github.com/Richard01CZ/Mafia_DTAExtractor)
(`DTA Extractor/DTA_Extractor.cpp`, blob
`e32e4da3ec8984431fe17b0ad015b517c13ea501`). Their source was not copied into our
tool. The Windows DTAUnpacker attempt failed in this environment; the native
implementation was checked against local block sizes, file headers, and scene
boundaries, **not** against an independent successful extraction.

ISD1, sound-specific compression, checksum verification, and patch precedence
are not implemented. This is a deliberately narrow research tool, not a
replacement for a general archive manager.

To reproduce the sample extraction and inspection (choose new output files):

```sh
mkdir -p extracted/alps1-base analysis/work
python3 -B tools/extract_dta_member.py "Hidden and Dangerous 2/missions.dta" \
  'missions\alps1\scene2.bin' extracted/alps1-base/scene2.bin \
  --key1 0x22BCDA98 --key2 0x7654A3F0
python3 -B tools/inspect_scene2.py extracted/alps1-base/scene2.bin \
  --output analysis/work/alps1-scene2-structure.json
```

Select `missions\alps1\scene.4ds` or `missions\alps1\loader.4ds` and a separate
destination to obtain the other two samples. Names are case-insensitive and
accept either slash style. Actual output files remain ignored.

Run the synthetic checks without any game inputs or extra packages:

```sh
python3 -B -m unittest discover -s tools -p 'test_mission_tools.py' -v
```

## Repository layout and tools

| Location | Purpose | In Git? |
| --- | --- | --- |
| `tools/` | Our analysis and validation scripts | Yes |
| `analysis/initialization-map.json` | Curated findings and evidence pointers | Yes |
| `analysis/mission-format.json` | First mission-structure observations and fingerprints | Yes |
| `analysis/work/` | Local repaired/unpacked binaries and generated exports | No |
| `Hidden and Dangerous 2/` | Locally supplied game installation | No |
| `.tools/`, `ghidra/` | Downloaded tool installations | No |
| `extracted/`, `assets/`, `captures/` | Local assets and runtime captures | No |
| Ghidra projects/databases | Local analysis state | No |

Existing tools:

- [restore_upx_metadata.py](tools/restore_upx_metadata.py): restores the two
  section names and four-byte UPX marker on a new copy. It is specific to the
  observed layout, not a general-purpose unpacker.
- [InspectStartup.java](tools/InspectStartup.java): exports a saved listing and
  attempts decompilation at a selected address.
- [TraceStartup.java](tools/TraceStartup.java): decodes the startup prefix while
  recognizing its inline-string calls; it is not an emulator.
- [ExportFunctions.java](tools/ExportFunctions.java): exports selected functions'
  pseudocode, calls, and references.
- [FindResourceReferences.java](tools/FindResourceReferences.java): searches defined
  strings and exports their saved cross-references.
- [validate_initialization_map.py](tools/validate_initialization_map.py): checks
  the curated map against the local binary and generated exports.
- [extract_dta_member.py](tools/extract_dta_member.py): extracts a selected
  ISD0 member using the narrow supported codecs.
- [inspect_scene2.py](tools/inspect_scene2.py): validates known scene container
  levels and emits counts/offsets without asset contents by default; `--objects`
  additionally decodes supported fields and local names.
- [validate_mission_sample.py](tools/validate_mission_sample.py): verifies the
  curated mission observations against local samples and selected code evidence.
- [ConfigureStackProbe.java](tools/ConfigureStackProbe.java): applies the verified
  stack-probe model to the base-client research database.
- [ConfigureChunkReaders.java](tools/ConfigureChunkReaders.java): applies the
  assembly-supported archive/chunk helper signatures in that research database.
- [test_mission_tools.py](tools/test_mission_tools.py): synthetic archive and
  scene-framing regression tests.

New tools are welcome when they answer a concrete question or make a check
repeatable. Prefer small, read-only tools; use separate copies for modifications.
Generated pseudocode is analysis output, not reconstructed source.

## Running the existing tools

Use Python 3.11 or newer for the Python scripts. The Java scripts run inside Ghidra, not with
`java ScriptName.java`. This workspace has used Ghidra 12.2 with Java 25 and UPX
5.2.1. Download tools from their official distributions and verify checksums.

To reproduce outer-layer unpacking for the base game from the repository root:

```sh
mkdir -p analysis/work
python3 tools/restore_upx_metadata.py \
  "Hidden and Dangerous 2/hd2.exe" analysis/work/hd2.restored.exe
.tools/upx-5.2.1-amd64_linux/upx -t analysis/work/hd2.restored.exe
.tools/upx-5.2.1-amd64_linux/upx -d \
  -o analysis/work/hd2.unpacked.exe analysis/work/hd2.restored.exe
```

These output files must not already exist. Do not overwrite a working game
executable with an analysis copy.

For headless exports, set `GHIDRA_HOME` to the installation directory and
`PROJECT_DIR` to the directory containing your separate `HD2Research.gpr`:

```sh
"$GHIDRA_HOME/support/analyzeHeadless" "$PROJECT_DIR" HD2Research \
  -process hd2.unpacked.exe -readOnly -noanalysis \
  -scriptPath "$PWD/tools" \
  -postScript ExportFunctions.java "$PWD/analysis/work/example-export" 006dd550 \
  -max-cpu 2
```

This assumes the program has already been imported and analyzed. The output
directory must be new. A GUI-open project may be locked even for read-only
headless access: use a separate project, never delete its lock. Ghidra 12.2 also
rejected a project location containing `&`; keep the project at a compatible
path outside this workspace if necessary.

To validate the existing map:

```sh
python3 tools/validate_initialization_map.py
```

Validation requires the local binary and all exports referenced by the map.
Those inputs are deliberately not distributed in Git, so a fresh clone alone
cannot run this evidence check. Current local project paths in the map are
machine-specific.

## Evidence and tracker maintenance

After each research milestone:

1. Update the checklist, leaving incomplete work unchecked.
2. Add or refine map entries with the exact binary fingerprint and evidence.
3. Separate observed calls/strings from inferred roles and runtime-verified claims.
4. Run the map validator when changing mapped evidence.
5. Check `git status` before staging; keep binaries, assets, exports, and databases
   local. Extend [.gitignore](.gitignore) when introducing new output locations.

Keep original files unchanged. Do not publish commercial binaries, extracted
assets, memory dumps, or generated game-code exports as part of this repository.
Use a legitimately obtained installation and consider applicable law and license
terms before distributing reverse-engineering results.

**Existing-history warning:** the initial local commit included four generated
game executables. They have been removed from the current Git index while
remaining on disk, but are still present in that commit's history. Ignore rules
do not remove historical content. Resolve this before publishing the repository;
history has not been rewritten automatically.
