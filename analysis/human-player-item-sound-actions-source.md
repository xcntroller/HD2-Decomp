# H6 item sound and actor actions — 2026-11-12

Source-only checkpoint after `cb0ebfa`; H6 remains open for real item firing and
projectile ownership. No active H4 changes imported or application loading enabled.
[Receipt](checkpoints/p2.2-human-player-item-sound-actions-20261112.json).

`007D5C70` is the actual RuntimeItem two-slot SOUND-frame cache. It looks up the
same process catalogue category2 and loads samples through the existing resource
cache with flags10. A cached requested slot returns directly; a population pass
sets88=min(variant count,2), fills all those slots and returns80 regardless of the
requested index. Missing catalogue returns null without resetting88; an empty
entry sets88=0. Exact-zero load configures type1, cone pi/2, volume variant14,
range variant08/0C/.4/.3, links the actual item model and stores XYZ0/0/1.1. A
failed load releases its frame and publishes null. These SOUND owners release
with their matching frame reference, separately from recursive item model trees.
Native refill overwrites/leaks an earlier still-playing frame when another slot
misses. Source retains overwritten references through item retirement, then
reclaims them; that safety difference is explicit, without an equivalence claim.

`007D5A80` returns immediately when88=0. Presentation draws CRT rand modulo88,
links the real frame, sets94=0/type7C and protected local XYZ before update/enable.
The player+view-offset branch uses type3 and Z0; otherwise type1/Z.7. Backend
honors E0 bit80 instead of unconditionally writing position. Authority hearing
is a separate request: mode1, not silent and unsigned age strictly>1000;8C is
published before SOUND. Shared process0098AFE0 gets group2, actual action sound,
source Human, XYZ0, real parent and flags22. Definition identifier7/22/260 sets
request+38 far-scale .3, others1; it is not request volume.

`007D6330` observes mode2, amount==1 and nonnegative payload virtual24 for the
last-round group2, then virtual28 for group3. Both use the same reusable request,
null source, third-person XYZ0/0/.7/model/flags1 or first-person XYZ0/shdum/flags41.
Other shared fields persist. Bounded original constructor/vtable and getter
reads establish serialized type4 virtual24/28 at payload44/48 (native48/4C);
types1–3 use genuine -1 getters. Getter addresses are absent from the frozen
inventory and earn no credit. The ordinary firing caller is still required.

`00542B80` now runs its real AC/flags/item14/signed58 gates. Offline7 queries
camera direction before FPV/head origin, excludes the same Human during the
actual dynamic-only006F78F0 query and restores before inspecting results. Only
Human kind3/posture0 with facing dot>.8 selects stealth: capture identity, align
-10, then produce subtype24. Otherwise send Look action8. Both paths reread the
cached item around model/sound calls and set58 from the actual payload interval.
Repair's005433B0 kind11 uses actual head/current B0 direction and loaded-World
ray, no added self-exclusion; descriptor flagsC0/frameEC actor kind4/nonzero
004DDCF0 health select subtype14. Kind10 queues subtype17 on the same Player.

The real PlayerD8/`0053D340` (absent frozen) copies subtype/identity/argument into
kind3 on the SAME first queue. It does not execute DC or move the command to the
second queue. Existing command consumers remain in their own H4/H8 boundary.
No transport/history success is inferred from command production.

Native warnings-as-errors ASan/UBSan/LSan passes100 lifetimes over actual parsed
RuntimeItem, catalogue variants/cache/refill/failed loading and retirement, shared
request retained fields/throttle/failure prefixes/protected pose, following sound
getters, and real Player/Base first-queue publication/melee cooldowns. Human,
model/engine/query and Look endpoints are explicit doubles; no actual loaded-World
collision execution claim. Strict actual nested-host i386 instantiates real
World queries/Car/frame-data types and sound backend/cache. Its remaining Runtime
item-fire/attachment declarations throw, compile-only. Launcher/probe rebuild and
link pass; no Windows/original/desktop execution. Temporary new audit sources,
native executable and object are fingerprinted then removed; frozen old audits
remain. Four new frozen partials, no complete promotions:34 complete/3254 partial
(3249 client+5 engine).

Next: shared007D4FD0/007D5210 and actual ShotMgr projectile producer/retirement.
Continue offline7; full non7 replay/DirectPlay research remains deferred. Full
World frame/application/render/user desktop acceptance still belongs to H8/P2.2.
