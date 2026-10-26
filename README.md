# Understanding Hidden & Dangerous 2

A personal project to discover how Hidden & Dangerous 2 works, from loading a
mission to controlling a squad, and gradually document its LS3D engine.

We're starting with the Steam release, including Sabre Squadron, running on
Linux through Proton.

## Where we are

**We've got past the first major obstacle: accessing the game's code.**
We've unpacked copies of both game executables and started identifying the parts
responsible for startup, the main loop, and loading missions.
We can also extract selected mission files locally and read the basic structure
of one real mission.

This is still early research, not a rebuilt or playable version of the game.
Our findings so far come from examining files; we haven't yet checked them
against the game while it's running.

## Progress

Completed steps mean we've reached a useful milestone, not that we understand
every detail of that area.

### Done

- [x] Inspect the game files and establish a consistent starting version.
- [x] Unpack copies of the game executables without changing the originals.
- [x] Set up tools to examine the code.
- [x] Build an initial map of how the game starts and runs.
- [x] Locate the code involved in loading missions.
- [x] Extract sample files for one mission and match their structure to the loader.
- [x] Save our findings and add repeatable checks for the evidence.

### Next: understand one mission from loading to play

- [x] Extract sample files for one familiar mission.
- [ ] Follow how the game reads those files and builds the mission.
- [ ] Connect the menus and loading screen to the start of gameplay.
- [ ] Check our understanding against the running game.

### Longer term

- [ ] Understand how the game and engine work together.
- [ ] Compare the original game with Sabre Squadron.
- [ ] Explore individual systems, such as squad orders, AI, and animation.
- [ ] Create useful tools and reconstruct small, well-understood parts.

A complete source reconstruction is a possible long-term ambition, not a
promise. We won't put a percentage on progress while the full scope is unknown.

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

## Keeping game files local

This repository is for our tools and research, not for distributing the game.
Commercial binaries, extracted assets, generated code exports, and local
analysis databases stay out of Git. See [.gitignore](.gitignore).

**Before publishing:** four generated game executables were included in the
initial local commit. They're no longer tracked in the current index, but remain
in that commit's history. That history still needs to be addressed.
