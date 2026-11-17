# Actual Players selection producers — 2026-11-17

Source-only checkpoint on feature/p22-own, following e2c8487. The application
desktop review is separate and remains uncommitted. These producers use the
existing borrowed roster, actual HumanStateLoadingOwner and existing World/FPV
owners; they have not yet been connected to GameUiPlayerControls in the app.

## Original contract

The research index was queried before static reads. Existing C exports were
compared with original i386 assembly in
`analysis/work/p22-own-20261112/own-player-controls-abi.asm` and
`own-player-selection-dependencies-abi.asm`; the latter's model dispatch table
is decoded separately in `own-player-model-type-table.json`. Frozen original
client SHA256 is 8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.

- 0045DA40 (22 bytes): resume roster[current2C] through actual005A3E60(true),
  unless current2C is -1. The distinct override58 is not consulted.
- 0045DC20 (158 bytes): validate signed index, positive signed healthEC and
  role19C==1; restore OLD current model through0045DB90, reload old current2C
  and demote its B0 through005A3E60(false). Then publish current2C/previous30,
  clear selected28, bind actual004922A0 FPV, refresh0045DAC0 visibility, and
  evaluate005A8320 name even though retail0069BFE0 is RET. There is no same-index
  shortcut or target Player promotion here. Resume performs the latter.
- 0045DCC0 (116 bytes): same previous30 returns true before validation; -1
  clears previous30 only. Other targets require positive signed health and
  Actor type3, not role19C. Publish previous30 first; if selected28 equals the
  target, **reload current2C and demote that soldier**, then clear selected28.
  Original0045DD12 makes the current-vs-command-target distinction explicit.
- 0045D660/0045D9D0 (99/100 bytes): forward/reverse cycling rechecks roster
  count, wraps, skips current2C and accepts positive signed health/role1. Empty
  or return-to-current yields0. With current=-1 and no eligible soldier, native
  final results can be count/-1; the later selector rejects those results.
- 0045DB90 (94 bytes): null is a no-op. Actual field160 Actor families
  4/20/21/34 consult Actor virtual8C and hide the old model iff bit10 is set;
  all others show it. This is distinct from has_control. The only frame effect
  is actual virtual30 visibility, with no guessed scene-update call.

Callback failures preserve the original already-published prefix. New source
guards reject missing native roster pointers or unrepresentable counts rather
than dereferencing invalid memory. No new roster, B0 owner or selection indices
are introduced. World forwarding reuses composition.states().select_player,
actual scene.frames().object_visible and existing presentation bindings.

## Verification and remaining delivery

Native ASan/UBSan/leaks: **1,032 checks PASS**, using actual
HumanPlayerControlLoading and HumanStateLoadingOwner with explicitly substituted
Factory/Base/Inline/model endpoints. This includes override-vs-current, ordered
model/B0/FPV effects, failure prefixes, same-index behavior, a command target
different from current, cycling edge cases and repeated owner retirement.
Strict actual full-World i386 module compile/link PASS; Windows output was not
executed. [Receipt](checkpoints/p22-player-selection-20261117.json) fingerprints
logs, source, evidence and removed temporary audit fixtures/binaries.

The staged application review artifact was preserved without rebuilding or
restaging it. No actual mission/desktop result has been observed. UI binding,
positive detached focus00462780, and selected conditional input/actor/cutscene
requirements remain; this source checkpoint does not close HUM/FRAME/OWN or
change their matrix statuses. Only the six frozen internal function entries
above receive partial source credit; no engine/delegated callees receive credit.
