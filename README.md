# Hidden & Dangerous 2 reconstruction

Rebuilding the base game as readable C++, initially using its original LS3D
engine. Original game assets and `LS3DF.dll` are required.

## Current state

The opening sequence and core front-end menus work, including profiles, settings,
credits and cinematics. **Mission loading is under development; gameplay is not
yet playable.**

Current work connects the recovered mission systems into the running game.
Multiplayer, Sabre Squadron and engine replacement remain separate work.

<!-- FUNCTION-PROGRESS:START -->
## Function count

`[------------------------------] 0.18% complete`

**34 complete / 18,783 identified functions** · **3,479 unique functions partially reconstructed**.

| Program | Counted functions | Complete | Partial |
| --- | ---: | ---: | ---: |
| Base game client | 17,294 | 34 | 3,474 |
| Original LS3DF.dll | 1,489 | 0 | 5 |

The estimated total covers the base game client and original engine, including
library code. Partial functions earn no completion credit; these counts do not
measure playability. [Counting rules and ledger](analysis/function-progress.md).
<!-- FUNCTION-PROGRESS:END -->

## Useful links

- [Progress dashboard](https://xcntroller.github.io/HD2-Decomp/)
- [Roadmap](plan.md) and [current blockers](analysis/p2.2-critical-path/blocker-matrix.md)
- [Build and packaging guide](docs/STANDALONE.md)
- [Source code](recovered/) and [research notes](RESEARCH.md)
