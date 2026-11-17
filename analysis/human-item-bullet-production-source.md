# H6 weapon and bullet production source — 2026-11-12

H6 remains OPEN. This source checkpoint follows `1c11dd1`; it supplies the shared
weapon dispatcher, ordinary offline bullet owner/producer and muzzle-light
lifetime. Selected shell, capsule, grenade and immediate melee endpoints remain
required. Full asynchronous World/ShotMgr combat-frame composition is H8; it is
not necessary to research unrelated network replay to complete these producers.
[Receipt](checkpoints/p2.2-human-item-bullet-production-20261112.json).

## Source and original ordering

`007D4FD0` increments6C before payload/kind9 dispatch. Ordinary primary payload
kind3 selects a tracer for equipment command1/4 and signed6C modulo3==0: bounded
assembly confirms actual signed IDIV. It produces the real bullet before looking
up shdum/local sound, following/last-round requests, ammunition subtraction and
68=game28-old64 then64=game28. A late required failure preserves the prefix and
does not consume ammunition/cooldowns early. Type2 retains blsdum particle before
required00451670, then marker hiding/sound/clocks/ammunition. Item virtual38/
`007D46B0` returns original marker74 after lazy007D5250, not auxiliary clone78.
Native9C/A0 are typed borrowed cardum/blsdum frames. Secondary007D5210 only
produces a kind9 grenade; its other return0 is genuine. Nonweapon instance-family
dispatch still fails explicitly. Player005433B0 now invokes this actual World
receiver instead of an external Runtime successful fire callback. Shared AI/
containing-Human Runtime integration remains H4/H8 where selected.

`00451100` uses real FPV model40 marker getters004928D0/00492900 when the shooter
is the Player and view offset is enabled, otherwise SAME item marker caches.
Mode2 gates these observations. `00450D30` offline7 acquires from SAME process
ShotMgr pool0C, initializes the record and only then publishes on active114.
The flare predicate is definition identifier265 (native item0C, proven by
007D3A40), not the module's runtime identity. Human160 selects an initial100ms
0044B910 call BEFORE publication; its result is ignored. Unmounted offline7
skips that call. Non7 conditional record production remains a required branch.

The existing16-record cold mark pool is the bullet pool. It now owns actual
aligned typed `ShotBulletRecord` instances, refills in batches16 and recycles
completed records in LIFO order; there is no second projectile registry. The
actual ShotMgr active114 vector owns each acquired record. Source pointers to
actual SceneActor/payload and matching visual owners are separate from the
native DWORD layout; no pointer truncation or serialized fabricated identity.
`004482A0` records shooter, full position/direction and payload damage10.
`0044B030` scales velocity XYZ through the actual payload virtual14, preserves W,
sets last position30, motion/age/flags/B4, and creates the selected real driver
trace or flare visual. Trace uses the original cloned mesh material/hidden
sector/position/visibility calls. Flare uses `m_w_flare`, half scale and actual
flare-child fields. Cold/unwritten native lanes receive no initialization credit.

Retirement0044B350/380 releases visual68 before module17 particleB4, then stores
-1. Actual process MissionEffectsProcess::stop is used. Module14's authority pass
uses native return1, retires kind1, returns the record to the SAME pool and stable
erases; presentation reloads vector/count after callbacks and ignores virtual0C
return. Unimplemented selected0044B910/0044B400 bodies throw with exact addresses.
These guards are not idle/successful projectile simulation. Close drains actual
owned callbacks while World/Frames are still alive, including failed/partial
retirement; original cold pool lifetime is retained when no runtime began.

Mode2 shell0044E5D0 precedes muzzle work; its current required failure occurs
AFTER actual bullet publication. Actual loaded Scene11C equality gates the
muzzle module17 emission through the existing producer. B0 light allocation and
retained item flash follow. `00453D80` configures actual WeaponLight/sector links,
light properties, flash opacity/visibility and retained reference. Bounded
original constants/assembly establish neighboring sector threshold16 and the
unordered-inclusive comparison, with Z²+Y²+X² extended subtraction order.
`00454060` hides/releases flash only when OLD signed time>50; signed time>=duration
hides light/returns2; otherwise intensity=1-time/duration, range=4-time/duration,
update, then elapsed addition. `00454120` removes light sectors in reverse,
unlinks/releases light before hiding/releasing retained flash. SAME ShotMgrB0
owner ticks/stable-erases these records before A0/90 and retires them before
resource/Scene teardown. Data observations now count the actual80/B0 owners.

## Verification boundary

Focused native ASan/UBSan/LSan warnings-as-errors checks100 lifetimes of actual
ShotMgr pool/active vectors, SceneActor source, authored payload fields and typed
bullet record/retirement, initial100 failure before publication, shell failure
after publication, signed tracer rollover, weapon ammunition/clock ordering,
muzzle-light time boundaries/failed creation/retained references and final
resource balance. Engine operations, item fixture fields, actual bullet frame/
damage, particle stop and selected effect calls are explicit doubles. This is
producer/lifetime verification, not actual combat simulation or a loaded World.

Strict nested-host i386 instantiates actual RuntimeItem/Human/Player/World/ShotMgr,
item24/28, FPV markers, driver trace/flare/light, actual module17 and full005433B0.
Only attachment remains an external throwing declaration in that wrapper; actual
required source guards above are not successful substitutes. Launcher/probe
rebuild/link and progress/evidence checks are recorded in the receipt. No Windows,
original EXE/DLL, Proton or desktop execution. New temporary audits are
fingerprinted then removed; original evidence/frozen earlier audits remain.

Source/typed STL/RAII ownership replaces original vtable, aligned-node allocator,
container and exception ABI. Neither delegated LS3DF bodies nor absent
00451EC0/00451FB0/FPV getter addresses earn new inventory credit. No complete
function promotions or application/readiness/desktop claim.

## Continue H6

Finish the remaining **producer and retirement** boundaries selected offline:
shell0044E5D0/00454190/004546C0 on same pool64/transient134; capsule00451670/
00448CE0; grenade00450400/0044CFF0 and immediate melee00451230 with existing H2
receivers. Keep native async bullet0044B910/visual0044B400 and shell00454360
World-frame assembly explicit under H8, including attached initial100 where
reached. New exact-frontier exports in `human-h6-shot-record-dependencies` provide
shell/capsule/falloff C; unknown signatures still require caller/raw confirmation.
No new broad export or unchanged-kernel audit is needed.
