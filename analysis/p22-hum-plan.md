# P22-HUM implementation plan

Updated 2026-11-12 through the authority-spawn/map source checkpoint. P22-HUM remains OPEN.
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
- [x] Finish reached breath/blood particle source through the SAME process and
  module17 lifetime, including damage effects and rest-position footprints.
  [Source checkpoint](checkpoints/p2.2-human-particle-surface-20261111.json).
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

Continue reactions/damage/ragdoll in main `H&D2 Decomp 4`. Clip/rest/selection,
actual authority/received damage, inventory drops, death, policy, localized messages,
shared session scoring/ranks and auxiliary death selection have source checkpoints.
[Latest evidence](checkpoints/p2.2-human-damage-presentation-20261111.json).
Shared PACK grammar/lifetime work is now bound to these actual effects.
Player objective selection/status/score and the actual scalar application FIFO now
have a [source checkpoint](checkpoints/p2.2-human-death-objective-session-20261111.json).
Reached peer scalar publication and console logging now have a
[source checkpoint](checkpoints/p2.2-human-peer-console-20261111.json).
Reaction-specific authority admission, native input/stream ordering and selected
reaction retirement now have a [source checkpoint](checkpoints/p2.2-human-reaction-authority-retirement-20261111.json).
Normal selected-map/rules metadata and borrowed network request admission now have a
[source checkpoint](checkpoints/p2.2-human-network-mission-metadata-20261111.json).
Cold peer creation/setup and reached disconnect/Scene2 retirement now have a
[source checkpoint](checkpoints/p2.2-human-peer-lifecycle-20261111.json).
Containing process application presentation2/10/17 now has a
[source checkpoint](checkpoints/p2.2-human-application-packets-20261111.json).
Canonical process input ownership and entry record reset now have a
[source checkpoint](checkpoints/p2.2-human-process-input-20261112.json).
Reached Car authority/impulses, actual Human contact callback and spawn-protection
HUD now have a [source checkpoint](checkpoints/p2.2-human-car-contact-20261112.json).
Presentation spawn, match/HUD, spectator camera and transmit targets now have a
[source checkpoint](checkpoints/p2.2-human-presentation-spawn-20261112.json).
Authority spawn, actual zone loading and rejection/map now have a
[source checkpoint](checkpoints/p2.2-human-authority-spawn-map-20261112.json).
Entry pending inputs/selected FIFO, optional process voice owner and received
manager prefix now have an [entry source checkpoint](checkpoints/p2.2-human-entry-input-voice-20261112.json).
Attachment80 retirement borrows the existing Car/StaticWeapon seats. Actual blast
material/damage now has a [Human containing source checkpoint](checkpoints/p2.2-human-containing-blast-20261112.json).
Runtime Mine9C ownership now has a [source checkpoint](checkpoints/p2.2-human-runtime-mine-20261112.json).
Continue its normal placement/packet producers, full control entry/transport and
the containing frame/failure/re-entry/teardown. Detailed boundaries are in
[the active contract](human-ragdoll-source.md).
The full selected checkbox, P22-HUM and P2.2 remain open; no application or desktop
acceptance is claimed.

Normal Mine creation/ground placement, Human inventory consumption and module12
packet envelopes now have a [source checkpoint](checkpoints/p2.2-human-mine-producers-20261112.json).
Continue full entry, Versions and containing frame dependencies; no Human checkbox closure.

Positive Versions Car/blast contacts now have a [source checkpoint](checkpoints/p2.2-human-versions-transitions-20261112.json),
including actual part/resource/reload/effect ownership. Continue armed Mine contact
and explosion producers, full entry and containing Human frame dependencies.

Armed Mine/Human contact and shared explosion now have a [source checkpoint](checkpoints/p2.2-human-mine-contact-20261112.json).
Continue CarD4/other reached blast damage, entry and full containing frame boundaries.

Reached CarD4/DM zone/seat damage and shared event production now have a
[source checkpoint](checkpoints/p2.2-human-car-blast-damage-20261112.json).
Continue terminal destruction/presentation/timed-DM, entry and containing frame;
full reactions checkbox remains open.
