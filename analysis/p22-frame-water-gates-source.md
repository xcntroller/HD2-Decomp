# First update: original water-camera gates — 2026-11-17

Base74a6dd7, feature/p22-frame. The user's b5ed6752 run passes the prior input
allocation guard: actual loaded1/status5 activation, Scene78 render, fixed-update
AND input complete. Full update then handles `GEffects water camera update
requires actual00487240 retained-camera transitions` and shuts down result1.
No access-violation trace. Exact executable/logs:WORK/first-frame-water-return.
There is still no stable scene/input/update/gameplay desktop acceptance.
[Receipt](checkpoints/p22-frame-water-gates-20261117.json).

The prior source rejected every eligible status1/5/6 frame with enabled water and
a selected Human. Original00487240 has a narrower boundary. After the224 clock
increment it checks status, water10C and actual selected Player. Optional existing
underwater1A8 plus wobble10D precedes a FRESH selected Human query. At48765E it
calls the SAME DD0 (HumanA0)007860B0 with a separate below-head output and process
0089B650 height output. It ignores AL, tests the explicit below byte at487663,
then attachment actor type20 at487675, then retained underwater1A8 at48767B.
All false branches directly to4879F4, bypassing the whole water transition body.

The actual World callback now performs that existing inline98/DD0 query and the
native conditional tests in order.007860B0's sentinel case leaves the height
unwritten while clearing below; source stores the actual process query height
across calls.0089B650 lies in the original PE's virtual zero-fill beyond its
section's raw data, establishing its initial0. There is no dummy ground/height,
water-disable switch or successful reflection substitute. Missing selected Human
still fails at its required receiver. Below-head/type20/retained-underwater
positives and optional underwater wobble still fail explicitly at their actual
remaining boundaries, before dependent retirement/render updates can be claimed.

Installed Tutorial watercam.set has water/wobble/scene-water enabled and serialized
underwater1. Existing original004885D0 captures actual initial camera parameters
and clears1A8 after loading; source already does this. Settings availability alone
therefore does not require a water transition. The callback borrows SAME World
Players, Human factory/services, frames and process; lifetime and module order are
unchanged. No extra camera, readiness/status publication or changed configuration.

977 targeted native ASan/UBSan/leak checks execute actual EffectsWaterSettings,
EffectsLoadingProcess and update_loading_effects plus the new water policy with
explicit frame/device/Human-query endpoints. Status/water/presence gates, fresh
query order, below versus ignored AL, type20 versus other attachment, output-left-
unwritten retention, positive errors, cold1A8 clear and later-update failure prefix
are covered. Existing DD0 query implementation is reused without another audit.
Full actual World/Human/GEffects/FRAME/application/probe i386 O2/Werror compile/link
and stage-only hashes are recorded in receipt. Temporary audit is fingerprinted
then removed; no original/DLL/Proton/Windows execution by agent. OWN dirty source
and its earlier binary stay untouched; interactive overlay remains uncommitted.

Next is actual revised user continuation, then the first real update failure.
Retained-camera transitions, wobble, positive decoration contacts/wind, and broader
FRAME/HUM/OWN remain partial; no source-complete/matrix/function-count promotion.

WORK=analysis/work/p22-frame-closure-20261117. Research index preceded original
reads. Existing00487240/007860B0/00465EE0 exports and raw assembly were reused;
00465EE0 compares the mission-camera owner with the scene camera (the old
decompiler's receiver spelling must not be read as an engine-camera method).

Actual revised user run passed the water query/gates and now exits on
`Decoration movement contact requires actual0047FAE0 bend/opacity requests`.
The same activation/render/fixed/input prefix completes; no access violation.
Exact executable/logs:WORK/first-frame-decoration-contact-return. This confirms
the negative water path in the application, not underwater transition support or
a stable full update. Continue the actually reached positive decoration contact.
