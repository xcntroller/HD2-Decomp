# Technical research guide

For the plain-English overview and milestone tracker, see [the README](README.md).
This guide preserves the detailed evidence, limitations, and tool instructions.

An early-stage effort to understand Hidden & Dangerous 2 and its LS3D engine.
The reference installation is the Steam release with Sabre Squadron, running
on Linux through Proton.

**Current result:** the outer UPX packing layer has been removed from copies of
both client executables. Static analysis of the base-game client has identified
startup, the main loop, and a mission-loading pipeline.

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

### Next: one mission-loading path

- [ ] Extract one known mission's `Scene` and `scene2.bin` resources locally.
- [ ] Inspect the loading helpers and correct inferred signatures in a working
  project, not the baseline.
- [ ] Match loader behavior to that mission's actual file structures.
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

## Repository layout and tools

| Location | Purpose | In Git? |
| --- | --- | --- |
| `tools/` | Our analysis and validation scripts | Yes |
| `analysis/initialization-map.json` | Curated findings and evidence pointers | Yes |
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

New tools are welcome when they answer a concrete question or make a check
repeatable. Prefer small, read-only tools; use separate copies for modifications.
Generated pseudocode is analysis output, not reconstructed source.

## Running the existing tools

Use Python 3 for the Python scripts. The Java scripts run inside Ghidra, not with
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
