# P2.4–P2.7 dependency overview

Source snapshot: `52c568964063207b0d15bea8ef98af09f89d84df` (2026-11-10).
P2.2/P2.3 are external prerequisites owned by the implementation agent. Reuse
[world ownership](../world-scene-ownership.md), [Human/VM composition](../human-vm-world-composition.md)
and [Phase 2 plan](../phase2-delivery-plan.md); this research does not expand them.

| Milestone | Executable path / owners | Hard prerequisites | Conditional / later |
| --- | --- | --- | --- |
| P2.4 | `006C0EF0` kind6 setup, kind5 briefing, kind7 `00698510`; mission from `006B9190` | Existing GameData cursor, live squad at application+14, two store owners +18/+1C, actual request consumed by the same world | Later campaign selection may load preparation save through `006C3340(...,false)`; loadout desktop access depends on legitimate progress. Standalone route synthesis `006B9680` is a separate route, not a fixed wizard. |
| P2.5 | `006C20F0` status14 → `0069E620`; menu+5B8 borrows retained game; status15/16 unwrap before menu shutdown | P2.3 live owner; actual pause/input/audio/Options services; panels use same squad/actors | Save/reload/restart actions depend on P2.6. Multiplayer actions remain Phase3 and are not offline prerequisites. |
| P2.6 | `006C39F0` writer, `006C3340` reader; live `0069C8E0/0069CFC0`; preparation `006B71C0/006B72B0` | Original chunk I/O, actual application preparation owners, registered module identities, actor lookup, Script contexts and live loaded mission for snapshot replay | Metadata/profile validation can be recovered without a world; it does not prove loadability. Resume from Main needs receiver/state routing beyond same-world snapshot application. |
| P2.7 | kind6 subtype5 `0068D2F0`; dispatcher `0068CE00`; campaign subtype6 `0068BDE0` | Real results and squad deltas; `0068C300` commit; progress `006B12D0`; preparation-save writer | Diary only when `0068C2E0` (mode owner+4==0); campaign model variants have actual null/empty sentinels. Retry uses preceding preparation save, not necessarily a live quick snapshot. |

P2.4 should reuse the accepted preparation source and its actual descriptor order;
[single-player contract](../singleplayer-pipeline.md) explicitly leaves loadout
screens completely untested until accessible. P2.5 is not blocked by save action
completion for ordinary resume. Retained-menu lifetime is a hard constraint:
restore the game as application current owner BEFORE shutdown/delete of menu.
`0069E620` allocation failure returns the original game after its final status/
audio work; its decompiler fastcall prototype is not a callable ABI contract.

P2.6 separates preparation persistence, live snapshots, profile metadata and
front-end receiving transitions. P2.7 already consumes the preparation path;
a full live snapshot is not established as a hard dependency of every debrief.
Do not make all calls in either milestone into mandatory initialization work.
Detailed unresolved boundaries appear in the graph and guide; whole-world
save/load and P2.7 integration batches remain research-blocked.
