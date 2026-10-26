# Understanding Hidden & Dangerous 2

A personal project to discover how Hidden & Dangerous 2 works, from loading a
mission to controlling a squad, and gradually document its LS3D engine.

We're starting with the base game's Steam release and its original LS3DF.dll
engine, on Linux through Proton. The function tracker below covers only these
two binaries; the expansion is outside its scope.

## Where we are

**We've got past the first major obstacle: accessing the game's code.**
We've unpacked copies of the game executables and started identifying the parts
responsible for startup, the main loop, and loading missions.
We can also extract selected mission files locally and read the basic structure
of one real mission.
We can now identify how some of its objects are named, positioned, rotated,
scaled, and linked to parent objects.
The first small, readable C++ reconstruction is also in place: the code for
reading nested sections of a file.
We've also identified model-resource references and reconstructed the loader's
small model-name processing step.
We can now name the engine's object types and explain when model loading tries
another file, stops, or accepts a result.
We've also reconstructed the first checks the engine makes when opening a model
file, including its supported version and an initial entry-count limit.
We can now read the records immediately after that header, including their
conditional name fields and extra sequence data, while preserving unknown bytes.

This is still early research, not a rebuilt or playable version of the game.
Our findings so far come from examining files; we haven't yet checked them
against the game while it's running.

## Progress

Completed steps mean we've reached a useful milestone, not that we understand
every detail of that area.

<!-- FUNCTION-PROGRESS:START -->
### Original-function reconstruction (base game only)

`[------------------------------] 0.00% complete`

**0 complete / 18,783 identified functions** (frozen, estimated baseline); **9 unique functions partially reconstructed**.

| Program | Counted functions | Complete | Partial |
| --- | ---: | ---: | ---: |
| Base game client | 17,294 | 0 | 5 |
| Original LS3DF.dll | 1,489 | 0 | 4 |

This covers the **base game and original LS3D engine only**, not Sabre
Squadron. The total is an estimate from Ghidra, including library code we
haven't separated yet. Imported routines and small forwarding stubs are
counted separately, outside this bar.

The bar fills only when an entire original function is reconstructed.
Our existing source recovers useful **parts** of functions, so it appears
under Partial rather than Complete. Discoveries and research milestones
are listed below; **0% complete does not mean no progress**.

Ghidra can miss functions or get their boundaries wrong, so the baseline
may be deliberately revised. This percentage does not measure understanding,
difficulty, playability, or time remaining.

[Status ledger](analysis/function-status.json) |
[Frozen counts](analysis/function-baseline.json) |
[Counting rules and refresh commands](analysis/function-progress.md)
<!-- FUNCTION-PROGRESS:END -->

### Done

- [x] Inspect the game files and establish a consistent starting version.
- [x] Unpack copies of the game executables without changing the originals.
- [x] Set up tools to examine the code.
- [x] Build an initial map of how the game starts and runs.
- [x] Locate the code involved in loading missions.
- [x] Extract sample files for one mission and match their structure to the loader.
- [x] Decode the first object-placement fields and distinguish lookup from creation.
- [x] Reconstruct and test a small file-reading component with readable source.
- [x] Identify model-resource references and reconstruct their name-processing rule.
- [x] Recover engine object-type names and the model loader's retry/stop decisions.
- [x] Reconstruct and test the first model-file header checks.
- [x] Read and cross-check the first table in both sample model files.
- [x] Save our findings and add repeatable checks for the evidence.

### Next: understand one mission from loading to play

- [x] Extract sample files for one familiar mission.
- [ ] Follow how the game reads those files and builds the mission.
- [ ] Connect the menus and loading screen to the start of gameplay.
- [ ] Check our understanding against the running game.

### Longer term

- [ ] Understand how the game and engine work together.
- [ ] Check more of the original game's behavior against our reconstructions.
- [ ] Explore individual systems, such as squad orders, AI, and animation.
- [ ] Create useful tools and reconstruct small, well-understood parts.

A complete source reconstruction is a possible long-term ambition, not a
promise. The function bar is a narrow, reproducible count, not a percentage of
the whole project or a forecast of when a reconstruction will be finished.

## How we work

We examine small parts of the game, record what the evidence tells us, and test
our explanations. Tools can turn machine instructions into approximate readable
code, but the result still needs interpretation.

We keep the original files untouched and distinguish between what we've
observed, what we think it means, and what we've confirmed in the running game.
We also build small tools when they make the work easier or repeatable.

This tracker will be updated as milestones are completed or our understanding
changes.

## Looking deeper

- [Technical research guide](RESEARCH.md): detailed findings, tool instructions,
  and remaining questions.
- [Research map](analysis/initialization-map.json): structured evidence behind
  the initial startup and mission-loading map.
- [Mission findings](analysis/mission-format.json): the first checked file
  structures, sample fingerprints, and what remains unknown.
- [Tools](tools/): scripts created for this project.
- [Readable reconstructions](recovered/): small, tested implementations of
  understood behavior, starting with the chunk reader.

## Keeping game files local

This repository is for our tools and research, not for distributing the game.
Commercial binaries, extracted assets, generated code exports, and local
analysis databases stay out of Git. See [.gitignore](.gitignore).

**Before publishing:** four generated game executables were included in the
initial local commit. They're no longer tracked in the current index, but remain
in that commit's history. That history still needs to be addressed.
