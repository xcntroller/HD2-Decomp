# Loading spatial sound: actual scene and late camera query — 2026-11-17

Base6f37c21, feature/p22-frame. The user's next actual run passed SupplyBox clip
acquisition and returned to Main on `Spatial sound has no actual world scene`.
The 5f044176 candidate/logs are archived under WORK/loading-spatial-return.
No loaded mission/frame or full desktop acceptance follows from this run.
[Receipt](checkpoints/p22-frame-loading-spatial-20261117.json).

## Actual original query boundaries

The earlier source's never-published WorldViews.sound conflated a constructed
current game, its engine scene/listener, and its mission camera's sector. This
was not an engine playback error or an absent scene. Original00433FA0 obtains
00698500->scene18->listener15C before any relative-frame transform update; it
then computes listener position and later queries the fresh current scene/root
when parenting a newly acquired sound. Loading-time SupplyBox group1/entry10/
flags1 can therefore emit before a mission camera-sector query is needed.

Original004368D0 genuinely returns only for absent CURRENT GAME. Otherwise it
advances the shared clock, updates scene15C listener transform, refreshes enabled
tracked sounds/attenuation, then queries current game's camera virtual8C. Actual
004650C0 returns its existing24C sector, including genuine NULL. That late value
is compared against manager9C before updating tracked sounds and driver fade.
There is no loaded-listener-view gate.00432420/00432710 tracked registration,
00433330 root attenuation and00432BE0 portal routing likewise use actual fresh
current-game scene queries. Research index preceded the static reads; saved
C/raw evidence preserves these distinctions and addresses.

## Concrete owners and retirement

SoundServices now borrows the SAME process WorldViews owner/scene. Scene queries
validate that actual game construction has published the scene; absence remains
a required error in native undefined dereference paths. The environment's absent
game RET remains separate. No nullable sound snapshot, new listener, copied scene,
empty world, skipped emission or successful default is supplied.

The actual GameSessionLifecycle binds a WorldSoundCameraLoan to its existing
MissionCamera only AFTER the complete World/activation/UI owners exist. Binding
requires the SAME published engine scene, one unique borrower and a real query.
The getter reads that camera's current sector on EVERY call, without requiring
activation success or manufacturing a sector. Explicit close preserves the loan
through mandatory unload/finish, including a failed cleanup that needs retry,
then retracts it before destroying camera/World. Member unwinding also retracts
on constructor failure before camera storage dies. Parent unbind clears the loan;
an older loan cannot withdraw a later owner's binding. No readiness/state5 flag
is changed by this source adapter.

Emission, tracking, attenuation and portal routing now use scene-only queries.
The environment uses a separate late camera-sector query. Listener capture stays
BEFORE relative-frame update; tracked frame transforms precede their fresh scene
lookup, so argument evaluation order cannot change these native observations.

## Focused verification and limits

Native ASan/UBSan/leaks checks exercise actual WorldViews/loan/voice/catalogue/
emission/environment/attenuation/portal policies with explicit engine endpoints.
They cover pre-camera loading emission (including SupplyBox's real request
shape), actual NULL sector versus missing query, duplicate/foreign/rebind guards,
constructor unwind, failed-close retention, first missing-camera failure prefix,
late changed-sector reads, listener capture and fresh sound parenting, voice
pooling and load exceptions. The receipt records exact final check count and
fingerprinted removed temporary audit; this is not a repeat of old sound suites.

Full strict actual World/Startup/GameSessionLifecycle/HumanFrameRuntime/FRAME/
application/probe i386 O2/Werror compile/link and stage-only results are recorded
in the receipt. Windows/original/DLL/Proton execution is performed only by the
user. Exact revised candidate still requires user mission testing; no first
frame, audible engine output, present or game acceptance is claimed by these
checks. Earlier sound receipts retain their historical source/view boundaries;
this closes their missing actual scene/camera query adapter at source only.
General FRAME/HUM/OWN remain partial, with external positive receivers unchanged.
