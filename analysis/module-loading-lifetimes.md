# Remaining module loading lifetimes — source checkpoint, 2026-11-10

The full first P2.2 checkbox remains open. Application loading is still declined;
there is no loaded world, first frame or new desktop acceptance.
[Receipt](checkpoints/p2.2-module-loading-lifetimes-20261110.json).

## Process Script bank and actual SOUND module

Native module4 survives world finish.00427B80 does not clear its256 current or
previous DWORDs.004270D0 creates/begins contexts before variable activation:
configuration+04 kind1 clears current; other kinds copy current to previous.
The bank now belongs to Startup and is a required borrowed constructor argument
through MissionScriptWorldLoading, composition and actual module. Destroying a
world facade cannot reset campaign values. Configuration+04 is the standalone
versus campaign selection, not configuration+224 or request.mode:00638D30
selects006B9680 standalone on the SAME +04==1 predicate. The existing owned
request.preparation.standalone supplies that distinction when dispatch is wired.
No dispatch/activation integration is claimed here.

Startup now supplies actual SOUND module6 tail004338D0, activation00433840 and
finish004338A0 using its SAME runtime, catalogue, files, engine backend, process
mode and actual configuration+04/+224. Tail invokes reset(1,1) if authority OR
!(mode&1). Activation/finish pass authority as the force argument to the existing
start/stop functions; reset arguments mean clear_samples and retain_primary.
The opaque configuration04 is tested for nonzero only, never dereferenced.
Previously verified SOUND internals are reused without a broad repeat audit.

## Cold Particles17 and HeavyFire16

Particle100C scene is distinct from the0089A8F4 mission-effects scene.00444110
assigns game18 on BOTH passes.00443F30, independent of authority/mode, sets1040
=-1, ensures definitions via00444140, drains256 buckets in reverse and sets1044.
Current source has no producer for those buckets or1014/1034 instances:
00445AF0 rejects valid placed effects before allocation. Cold retirement is
therefore genuinely empty; no positive particle destructor/update/rendering
receiver is supplied. Shared private state prevents a second world from adopting
an initialized module. Closing invalidates typed scene borrowers before scene
retirement; that extra safety write is not credited as original behavior.

00443FE0 only acts while1044 is set:004458F0 retires1014 records, reverse bucket
retirement follows,00445980 destroys/clears definitions,1040=-1,1034 instances
retire, then1044=0. **This corrects the prior cache lifetime description:** the
process cache skips reload only while nonempty; an initialized module finish
clears it. Closing only the effects-file facade, or finishing a never-activated
particle module, retains the definitions as in the native branch. Definitions
read failures retain the original optional diagnostic/return semantics; a source
parse exception does not set the initialization latch.

HeavyFire16 activation00472250 unconditionally clears four bytes1AC..1AF on
both passes.004115E0 tail is a verified RET.00472270 cold finish has two empty
owned lists190/1A0; no positive fire-support/spawn/simulation producer is exposed.
Startup owns the process scalar state, and a world facade borrows registration16.

## Evidence and verification

All original evidence is read-only; no original game/DLL/Proton or Windows
fixture ran. WORK=analysis/work/mission-world-20261103. Vtable pointers+20:
Particles00443FE0, SOUND004338A0, HeavyFire00472270. Original constructor00443D30
sets100C=null and1040=-1; process1044 starts BSS zero. Captures:
particle-constructor.asm, particle-retirement.asm,
heavy-fire-activation-retirement.asm, particle-retirement-required/ and log.
First plain export stopped at absent00443FE0; subsequent transient read-only
definitions produced exports without modifying project or frozen inventory.
00443F30/00443FE0/00472250/00472270/004338D0 are absent frozen: zero credit.
Only00444110 and00445980 gain partial credit; no complete promotions.

Focused native ASan/UBSan/LSan checks actual two-world Script module constructors
with the SAME bank/registration, current/previous persistence and standalone/
campaign transitions. It does not run contexts or claim a VM-loaded world.
Particle/HeavyFire checks cover cache reload after initialized finish, retention
before activation, both passes/latch, missing/malformed/throwing definition
reads, real BASE definition fixture, foreign/null scenes, duplicate borrower,
retirement and repeat. Files/frames are explicit doubles; runtime files still
come from the actual resolver. First native fixture run used a wrong fixture
path and failed its file assertion; corrected run passes. No original-data
failure is inferred from that fixture mistake. Strict i386 actual full Human/VM/
SOUND/particle/HeavyFire graph and application compile/link pass. Containing
WorldActors/Car runtime/session are still borrowed construction boundaries.
Temporary audits/binaries hashed/removed; logs/research kept.27 tracker tests pass.

Continue actual Versions/GEffects receivers, containing session/registered
module dispatcher/ordered loader, meaningful guard observations and first
camera/update/render/failure/repeat. Private guard tables remain uninitialized;
no raw API result substitution or forced equality is permitted.
