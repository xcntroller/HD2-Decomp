# H6 Player items — implementation and dependency plan

Active worktree `/home/control/Documents/Projects/HD2-Decomp-p22-hum-h6`, branch
`feature/p22-hum-h6`, created from completed H2 `85593e8` at the user's request.
H2 and completed aggregate201adbf are present. Active H4 remains separate; H8
is unassigned. H6 is OPEN. No application activation or desktop claim.

## Current priority and completion boundary

The user prioritizes finishing P22-HUM and getting in game. Follow the offline7
path first; do not expand conditional DirectPlay/non7 history/replay work merely
from call-graph proximity. Keep its required guards and integration ownership.

Complete the positive receivers selected by existing005433B0 and the reached
Player00542920 notifications on the SAME Player/B0, Human, inline/E0/DD0/230,
inventory/item, FPV/camera, process random/SOUND and input-record owners. Preserve
native return/mutation/failure order, actual item firing and command publication;
no success-only Runtime or duplicate item/history/command owners. Full cross-branch
World scheduling is H8; actual application/render/user acceptance is P2.2.

Read original behavior and dependencies before implementation. Initial indexed
sources are `human-player-item-action-workers`, `human-player-live-frame-workers`,
`frame-human-feedback-dependencies` and `frame-human-shot-and-control-workers` in
WORK. New bounded static exports recover00544D20 and007D4E90. Raw H6 shot/held/
item-frame/command disassembly and exact original FLOAT constants are retained.
C exports with unknown signatures do not establish their ABI.

- [x] H6.1 — Held actions00542E20, title/timer publication, cancellation and
  action6/7/13 release through real inline actions/Player08/item-ready timers.
- [x] H6.2 — Shot basis00544D20, spread00545140 and recoil005454D0 with exact
  random/gate/store order and existing integrity/difficulty/FPV observations.
  [Controller checkpoint and limits](human-player-item-controller-source.md).
- [ ] H6.3 — Positive005433B0 firing/dry-fire/reload, command00542920 effects,
  FPV animation/FOV duration and post-shot inventory refresh/cooldowns. Source
  coordinator/dry-fire/reload notification are verified; actual item24 remains required.
- [ ] H6.4 — Melee00542B80, actor actions/repair query and actual command routing;
  reached item virtual24/28, sound/flash/projectile borrowers must be real or
  retain an explicit blocker. Shared projectile/ShotMgr dependencies need an
  ownership decision from the actual call chain, not a fabricated successful fire.
  Melee/repair/first-queue and shared shot sound are now [source-verified](human-player-item-sound-actions-source.md); projectile producer remains open.
- [ ] H6.5 — Compose reached item commands with actual input/queue owners.
  Player08 SAME-Record pending production is verified in the controller
  checkpoint. Full non7 history/replay/admission is conditional external
  D/OWN/H8/Phase3 integration, not an offline H6 detour.
- [ ] H6.6 — Actual-owner positive/failure/re-entry/teardown verification,
  focused native sanitizers, strict host i386, launcher/probe rebuild, temporary
  audit removal and consolidated contracts/ledger/handoff. Continue through
  dependencies until H6 is complete or genuinely blocked.

## Remaining D ownership

D entries are still required when their caller is selected. H4/H6 supply specific
Human producers/consumers; completion of those branches alone does not close a
whole Car/entry/World integration checkbox.

| Open dependency | Work supplied by Human milestones | Remaining integration owner |
| --- | --- | --- |
| D4 Car authority/presentation frame | H4 supplies reached AI/item admission; H6 supplies selected Player commands | Car/OWN/H8 must compose the continuing Car frame and preserve earlier guards |
| D5 occupied/non7 Car frame | H3 supplies seats/reset; H4/H6 supply reached AI/Player inputs/actions | H8/Car composes actual driver/input/mode/physical lifetimes; H3 alone did not close it |
| D6–D9 admission, status9/received0, shared entry/reset | Existing H2 source supplies worker/messages/queues | OWN/network/World integration is still required; H6's packet producer is not admission or a live session |
| D11 fresh part5F9/wheel695/EXPL8/smoke14 | No Human milestone supplies a demonstrated native producer | Still unresolved Car blockers when those paths are reached; H8 must account for them, not check them off automatically |

H8 is an integration milestone, not a declaration that these failures are
unimportant. The required subset must be completed, proven outside the selected
native path, or reported with its exact blocker before claiming the containing
World/P22-VIS path works. Reuse confirmed evidence; do not repeat unchanged
producer searches without new evidence. H6 proceeds with its independent work.
