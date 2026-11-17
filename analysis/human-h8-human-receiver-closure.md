# H8.2 Human-owned offline receiver source closure — 2026-11-17

H8.2 is source-complete for the Human-owned offline decision/action/callback
boundary. H8.4 conditional actor owners, H8.5 containing verification, H8.6
application/desktop and overall H8/HUM remain OPEN. This is not a claim that an
arbitrary scene/actor population runs, nor that any remaining guard is absent.
The existing milestone split assigns conditional nonHuman owners to H8.4;
those requirements are listed below rather than silently treated as success.

## Containing boundary checked

Actual Scene2 dispatch calls SAME World→Human0059FE30/0059FFD0→B0→inline98→
DD0/E0/equipment/geometry. World binds Player1,Active3,Guard4,Script5,Passive6,
Area8; Passive006-frame entries005A4330/4340 genuinely RET4 before the later
kind filter.0059F2C0/005A3E60 selection (`human_state_loading_owner.h`) proves
ordinary offline MODE7 initial/script/player selection does not choose Remote7;
non7 selection and transport stay required. No replacement state/input/query/
roster/inventory owner is supplied.

Human-owned source now includes the containing Active prefix/decisions through
00526550 return; current/pending orders, commands and task progression; placed,
retreat, cover/group/signal, grenade/vector/vehicle aim; Player delayed commands,
cover41 and firing callback; Script/Guard/Area; actual common/timed action
start/end00751480/00755500; actual Human inventory/damage/hazard/water/animation
callbacks and same-owner retirement. The authoritative detailed contracts are
linked from p22-hum-h8-plan.md: authority-body, task-execution/completion/runtime,
player-dispatch/cover, command-fire-line, timed-interaction, animation-frame,
pack-effects/common and blast-sight. H1–H7 closures remain applicable; their
unchanged kernels were not broadly re-audited. ShotMgr remains separately H8.3.

The focused source review checked current World/factory/Runtime dispatch and
positive required receivers, not every exception string as a missing function.
Real missing-state/frame/retired-borrower/malformed-data guards remain necessary.
Non7 respawn, visibility, fall publication, Remote input/history and objective
metadata are behind original mode gates (`human_scene_frame.h`,
`human_control_loading.h`, `human_ground_motion.h`, Player/factory source).
Optional model-debug/render paths retain their own explicit requirements.

## Last concrete Human integration correction

AI weapon selection (`HumanLoadingInventoryCommands`→00538770) still called the
older restricted transfer/swap helpers despite completed general PACK receivers.
It rejected cross-Human and general slots before reaching actual005A2D50/
005A2E90, and could stop at old pack/head fallback guards. It now uses SAME
World00587BF0/00587E00 common receivers and actual general transfer/swap owners.
No transfer/equipment/HUD/FPV logic is duplicated or replaced with a success.

005A2B40 request preserves item identity and self-source−1/actual other id,
source/destination/index. Canonical source lookup belongs to received authority.
Common00587BF0 performs the transfer FIRST, then requires non7 publication only
on success. The previous early non7 failure was wrong for that prefix.

005A2BB0 clothing eligibility now runs BEFORE B0/item identity reads: slot4
checks1A0, target005A6380 then source only on false, followed by target/source
1A5. Rejection requires no B0/entry. Player1/Remote7 still queue; accepted AI
3/4/5/6/8 uses actual005A2E90, canonical current/selected entries, both Humans'
178 publication and fresh actual Player-mode reset after transfer.00587E00
non7 publication remains an honest post-effect requirement. NULL explicit source
is outside this reference-taking API; explicit self/other source semantics match
native. Prior full transfer/swap kernels are unchanged and retain their earlier
verification. Legacy narrow helpers remain source, but no longer receive this
AI selection path; no extra function credit is earned by delegation.

00534BF0's complete Human-owned handoff producer now rejects Human3 before
its actual005A6380 nonHuman-layout predicate, rejects false, and on true uses
existing00534620 task24/false/FLOAT1.5/notify1. That external predicate remains
required at its native call site until H8.4 supplies a valid nonHuman EC/B0/19C
owner. The former generic specialized-producer failure no longer hides which
owner is missing. A mock predicate passing in a native audit does NOT implement
or prove that owner.

## Continuing H8.4/external requirements

- Populated type36 sound registry005E8B70 and type34 singleton005F0E60/target
  state require actual actor production/lifetime. Empty/cold cases use SAME live
  registry proofs; no private empty collection is substituted.
- Dog actual activation/controller authority/perception/containing frame and
  retirement remain open. Verified Body/controller lifecycle/follow source is
  not enabled activation. Native unproduced priorities/normal remain honest.
- Aircraft20 seat/attachment/formation consumers, nonHuman EC-dependent target
  predicates and contacted Actor58 types14/5 require actual family owners.
  Existing Human/Door/Ladder/Car/mounted paths are source-bound. Physics frameEC
  contact dispatch precedes the actor-only14/5 branch.
- Application currently declines before World activation in this checkout.
  OWN/FRAME integration and actual population/execution/user acceptance remain.

These are continuing requirements, not newly added plan checkboxes, an inference
that the selected mission omits them, or a resolved_source promotion of HUM.
FRAME's91a7d33 review used HUM51082a2: its missing Active/00751480/00755500 and
rigid-part5F9 claims are obsolete. Current H8 provides those source bodies and
explicit authorized D11 phase0 correction. Conditional actors and live application
forwarding still require current integration. Reassess exact revisions before
imports; do not rely on the old54-missing-operation count.

## Verification

Native73 strict ASan/UBSan/leak cases PASS for new request routing, AI general/
cross-source slot arguments, Player/Remote queue distinction, false/throwing
receiver prefixes, clothing eligibility before missing B0/entries and actual
handoff/task-word policy including model-before-direction publication. Items,
queue/World/engine endpoints are explicit doubles; no full loaded World claim.
Strict actual World/module i386 both passes and concrete inventory.move/swap/
select_weapon plus handoff producer compile/link PASS. Forced O2 application/
startup-probe compile/link PASS; Windows/originals not run. Existing major native
Human/Actor/Items ownership audits stay within documented boundaries; no repeated
broad audit or permanent test. Temporary audits fingerprinted/removed. Existing
frozen partials gain binding evidence only; no new/complete/absent/engine credit.

[Receipt](checkpoints/p2.2-human-h8-human-receivers-20261117.json).
