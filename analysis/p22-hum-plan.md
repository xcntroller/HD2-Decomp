# P22-HUM implementation plan

Updated 2026-11-11 for the Human model-presentation source checkpoint. P22-HUM remains OPEN.
Both P2.2 checkboxes in `plan.md` remain OPEN. Mission loading still declines,
and no mission desktop acceptance has occurred.

Continue the following work in order, resolving dependencies within P22-HUM as
they are reached. The user now authorizes subsystem commits for the accumulated
source and a commit for each completed checkbox below. Preserve Agent #2's UI
work. Detailed contracts and verification evidence are in
[Human frame source](human-frame-source.md).

## Verified source already present

- [x] Mounted/remote equipment, animation arbitration, pooled presentation sounds
  and Door/model-attachment action branches.
- [x] Step preparation, ground/water/air motion, terrain alignment and collision
  coordination using the existing physical state and World queries.
- [x] Face control, light/opacity observation, positive Sight material lookup,
  face-hit textures and Look/model hit commands.
- [x] Secondary clips, weighted bone correction, general bone bases, two-bone
  IK, hand/weapon posing, equipment overlays and stowed-weapon presentation.
- [x] Aimed-weapon wrapper and all four posture workers, including prone World
  rays, target interpolation, retained camera adjustment and parent restoration.
  Focused native sanitizers, actual World i386 and launcher compile/link pass
  through this boundary in WORK `human-aimed-weapon-application-i386.log`.

These checks establish source progress. The motion/pose checkpoint adds100
frozen client partials, with no complete-function promotions.

- [x] Mount007BA0C0 and head007ADA10 direction workers, with focused native
  sanitizers, strict actual World i386 and launcher compile/link verification.

## Remaining completion sequence

- [x] Complete model presentation and its retained producers/pose-chain helpers
  through actual007A7470 dispatch. Source checks are recorded in the
  [model checkpoint](checkpoints/p2.2-human-model-presentation-20261111.json).
- [ ] Complete reactions, damage and ragdoll behavior. Finish0074D510,
  remaining007B0910 reason/source branches, positive obstruction/contact/fall
  damage, joint impulse consumption and actual ragdoll pose/contact retirement.
- [ ] Complete positive interaction actions. Finish00751480/00755500 occupied
  actions and exits, including actual Car seat entry/exit and mounted receivers.
  Reuse the existing Car seats, occupants, physical core and model ownership.
  Preserve required integrity observations and native failure ordering.
- [ ] Finish common AI action/output receivers. Close reached00584E30
  completions, remaining B0 action/navigation/formation work, track events and
  DD0 transition/attachment consumers through their existing owners.
- [ ] Finish Guard/Area decisions. Close required cover/door0057CC50,
  floor-list00581E30, kind9 release and remaining decision receivers. Verify
  their invocation through actual actor dispatch and shared Scene2 identity.
- [ ] Finish Player Human item behavior. Complete held-item00542E20,
  shot00545140/005454D0, reached command/FOV/dry-fire/melee/repair/transport
  receivers and their actual item, animation and physical-state mutations.
- [ ] Finish reached breath/blood particles. Supply actual particle instances
  through00445150→004451B0 and their existing process/resource lifetime.
- [ ] Verify the full required Human frame path. Exercise actual containing
  World/control dispatch, positive branches, partial failure, repeated entry and
  ordered teardown. Run a consolidated targeted native sanitizer, strict i386
  and launcher build checkpoint. Record remaining external integration gaps
  precisely, update contracts/function credit and remove temporary audit code.

The full P2.2 checkbox additionally requires the other blocker IDs, actual
application loading/rendering, failure/repeat handling and user desktop
acceptance. Keep their ownership and implementation work with their respective
workstreams.

## Current next work

Continue reactions, damage and ragdoll behavior at0074D510 and remaining007B0910
branches. The model-presentation checkbox has source verification, with actual
World i386 and launcher compile/link checks. Its original00755500 interaction
callers continue in the later positive-action checkbox. Shared mounted-control
movement sounds, occupied updates and firing remain required in their containing
Car/StaticWeapon/action paths. P22-HUM and both P2.2 checkboxes remain open.
