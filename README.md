# Understanding Hidden & Dangerous 2

Readable reconstruction of the base game's menu and launch path, initially using
its original LS3DF.dll engine and local assets through Proton. Gameplay, Sabre
Squadron and engine replacement are outside the current menu milestone.

## Current build

The bounded launch milestone is complete: four opening movies with music and
English subtitles lead into the compiled Main/Exit menu. Pointer/keyboard input,
Escape skips, confirmation/cancellation, focus return and bounded shutdown/failure
handling are desktop-accepted. The [acceptance matrix](analysis/launch-acceptance.md)
records tested builds and distinguishes retained evidence from user confirmations.

The UI boundary remains **English,800x600, Main/Exit**. Other menu actions,
dynamic labels and broader settings/profile/save flows remain unfinished. Full
original setup/protection, object ABI, unused global caches and live network/
gameplay services are not claimed complete. Original engine calls stay delegated;
source recovery alone is not desktop delivery.

Build and run from the repository root, with the original game installed locally
under `Hidden and Dangerous 2`:

```sh
make -C recovered application-launch WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++
python3 -B tools/run_application_launch.py
```

The launcher stages isolated runtime files under `analysis/work/startup-runtime`,
preserving staged config/profiles and leaving installed assets unchanged.
Use `--stage-only` to prepare files without executing Windows code. The old
engine-window preview and startup probe remain separate diagnostics.

Next: character targets, focus entry/navigation and initial-focus policy in the
shared menu foundation. [plan.md](plan.md) is the single menu delivery checklist;
[HANDOFF.md](HANDOFF.md) records the latest result and next work.

## Verification and evidence

Original behavior and function addresses are recorded before implementation.
Reached services use real owners/bodies; unsupported branches remain explicit.
Existing native checks and tooling tests run with:

```sh
make -C recovered test
python3 -B -m unittest discover -s tools -p 'test_*.py'
python3 -B tools/function_progress.py --check
```

Temporary sanitizer audits and strict Windows i386 compile/link checks accompany
relevant changes. No permanent new tests are added under the current agreement.
Only user desktop runs establish interactive acceptance; the agent does not run
the original game, DLLs or Proton. See [AGENTS.md](AGENTS.md) for workflow rules.

## Original-function progress

The count below measures original function bodies, separately from menu delivery.
A working checkpoint can depend on partial bodies and delegated engine calls.

<!-- FUNCTION-PROGRESS:START -->
### Original-function reconstruction (base game only)

`[------------------------------] 0.00% complete`

**0 complete / 18,783 identified functions** (frozen, estimated baseline); **334 unique functions partially reconstructed**.

| Program | Counted functions | Complete | Partial |
| --- | ---: | ---: | ---: |
| Base game client | 17,294 | 0 | 329 |
| Original LS3DF.dll | 1,489 | 0 | 5 |

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

## Project map

- [Plan](plan.md): remaining menu outcomes and delivery criteria.
- [Handoff](HANDOFF.md): current state and next session.
- [Research](RESEARCH.md): detailed findings, evidence and tool instructions.
- [Launch acceptance](analysis/launch-acceptance.md): delivered boundary and provenance.
- [Menu inventory](analysis/menu-screens.md): known screens and dependencies.
- [Readable source](recovered/) and [tools](tools/): reconstruction and analysis code.

Commercial binaries/assets, generated exports and local analysis databases stay
out of Git; see [.gitignore](.gitignore). Four generated game executables remain
in the initial local commit's history despite being untracked now; that history
still needs cleanup before publishing.
