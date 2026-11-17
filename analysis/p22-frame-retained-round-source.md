# FRAME retained-round reset — source checkpoint, 2026-11-17

Active branch feature/p22-frame; source base d64ce5d. This extends the verified
[pending spawn/respawn](p22-frame-pending-spawn-source.md) source. **FRAME remains
partially_resolved.** The containing0069C390 body and17 of18 registered module2C
receivers now have actual World/process bindings. Module13/004762F0 still requires
the actual derived Actor44 callbacks, especially Human0059D100 and its retained
inline/model reset. The dispatcher fails at that required receiver; it never
claims a completed round reset after running only the other modules.

## Original boundary and implementation

The complete0069C390 C and raw ABI are retained in
analysis/work/p22-frame-closure-20261117 (WORK), together with the original final
module vtables and all18 module2C exports. Actor final vtables and all17 supported
Scene2 Actor44 targets are in retained-actor-reset-targets.json; the Zone final
table00812B20 comes from actual005EBED0, Photo00813400 from0060B6D0 and
Document008135E0 from0060DB70. Missing Ghidra starts were recovered only inside
always-rolled-back read-only research transactions. No frozen inventory changes
or absent-function credit follow from that research.

`session_round_reset.h` supplies0069C390, called by actual FRAME006997B0 rather
than its former external reset loan. Its adapter uses the SAME World/module
registrations, locked process peer records, Scene2 identities, mutable application
squad, HUD/FPV/Text, dubbing/navigation, blast records, entry packets, Music and
speech cache. The status04/retained1C gate precedes the Focus query; no reset
writes precede that query. The native status/timer/flag/life stores, LIFO pending
owner releases and subsequent lifecycle calls retain their distinct order.

False offline restart with retained1C!=1 deactivates reserved Humans presentation
then authority before immediate removal, resets Spawn10 in reverse identity
order, then runs modules. Afterwards it visits the live squad with fresh
count/entry/active44 and fresh00475940(0), invoking actual00611C90. Other reset
paths retain the actual actors. Neither path constructs another World.

The module pass0048DBC0 reads registrations afresh in ascending order, including
slot18, and forwards the restart byte (native CL). The concrete dispatch covers:

- Tracks00401140: finish(false), activate(false).
- GEffects004863F0: reset/fades, decoration clear/read/initialize, bird clear/load,
  clock224, scene-water/24C, detached camera disable and actual special-model
  restore. Full unload is not used.
- Physics00410A90: presentation finish, authority finish, static queue retirement,
  authority activate, presentation activate, each with fresh mode flags.
- Items007DD750: disable equipment callbacks; actual007D8A90 ascending inventory
  reset, base7C, placement/index retirement; enable, identity-1, pending reuse
  clear; authority/restart reads actual mission items. It retains hand models.
  Missing required data throws with original data preserved; native007D7BE0
  deletion is intentionally excluded under the user preservation boundary.
- Script00424900: actual process override008692D0 true, presentation/authority
  finish, previous424→current24 restore00428CA0, authority/presentation activation,
  normal-return override false. Existing binding failures remain failures.
- Sight00470090: actual observers/targets clear then delay4000, preserving cursors.
- Sound00433910: actual virtual48(0,0), listener9C=0, delayA0=2000, driver14(0).
- Versions0043EB50: authority/presentation finish then authority/presentation
  activate with fresh flags. Lightmaps0048C790 reuse the existing all-restore
  before any-release path; Vertices00474C90 reuse the existing restart body.
- TShift/HeavyFire00411620 are genuine RET4. Players00460C20 uses the already
  implemented reset with its original control/auxiliary/subgroup/process order.
- Runtime00477B00 deletes actual owned actors backwards, resets counters70/74,
  preserves descriptors/retained-item queues, then recreates static actors for
  false restart or runs authority/presentation activation for true restart.
- ShotMgr00452280 recycles shells, clears80/90/A0/B0 backwards, then retires114
  backwards by actual kind1/3/4. Textures, hit-zone/WeaponOdds models,124 queue,
 140 clock and cold pools survive. Source cleanup tolerates consumed/null rows
  after an injected mid-reset failure rather than dereferencing a freed owner.
- Camera00464E10 recreates actual presentation camera, then refreshes the current
  Human target unless attached Actor34. Existing required attached camera leaves
  stay required. Particles00447730 retire active1014 and placed1034 backwards;
  false retains definitions/pools/id/latch; true clears definitions, identity-1,
  reverse256 cached pools, then initializes the latch.

Module13 remains explicit. Its actual registry traversal is supplied as
`SceneActorRegistry::reset_round`: counts4C/50 zero, shared006E07C0 before count
snapshot, ascending identity44, fresh-slot00475EC0(true), summary only on restart.
Focused verification covers traversal/callback ordering; no actual derived
Human reset is inferred from those doubles. Common00610710, Door00602400,
Ladder005EA4D0, Zone005ED8B0, Spawn006117F0 and StaticWeapon006050B0 source
helpers are present and actual i386-instantiated, but remain unbound to module13.
Car004DD0D0 now accepts false/true restart; its existing offline reset body is
reused, gating new proximity insertion by that actual byte. Non7 Car leaves
remain guarded. No new partial inventory credit is claimed for these unbound
actor helpers at this checkpoint.

The containing tail resets actual FPV/Text selection before Iconic, shared blast
records and mission objective statuses. Non7 client entry generation/packets,
true restart500/status5, conditional application entry request, actual Music
references and selection, shared speech category clocks and offline face cache
use their original owners. Speech clears only category.field14; phrase order,
cursor, enable bits and usage survive. It does not reset HumanStore/profile stats.
Focus/Iconic use original GetIGraph/Win32/Steam observation wrappers, not cached
focus or successful protection validation. Both are compiled, not agent-executed.

## Verification and remaining work

See [receipt](checkpoints/p22-frame-retained-round-20261117.json) for exact source,
original evidence, temporary audit/output hashes and build logs. Native focused
ASan/UBSan/leak checks execute the actual containing policy, real peer/life and
pending payload owners, reset algorithms, actual registry, saved script variables
and speech cache. Engine/Human/actor/UI/application endpoints and transient row
resources in that audit are explicit doubles. Actual World/Runtime/FRAME/UI/i386
composition is compile/link only. Launcher/probe are built sequentially after
Ghidra; neither fixture nor application is staged or executed.

Continue directly with actual module13/Actor44, beginning Human0059D100,
0059C4D0/CA00/CEC0 and inline005AB4C0→00784950. WORK/actor-round-research,
reset-human-leaves, reset-inline-leaves and reset-control-leaves already contain
those originals; use the saved indexes before new exports. Required process
console/balance, authority request admission/decoder, general network pump/
teardown and application/camera services also remain. Preserve OWN's pending
uncommitted desktop work and acceptance artifact. No whole FRAME closure,
application installation, live mission population or desktop acceptance claim.
