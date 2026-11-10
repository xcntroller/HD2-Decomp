# Player presentation and HUD Human loading — 2026-11-10

Source subsystem within the full first P2.2 checkbox. Application loading remains
disabled; no world frame or desktop claim. Joins [player/first-person owners](first-person-loading-composition.md)
with actual [HUD panels](hud-loading.md), [Human lifetimes](human-world-composition.md)
and Players control. [Verification receipt](checkpoints/p2.2-hud-player-presentation-20261110.json).

## Human getters and inventory identity

005A8380 offline reads actual profile.field24, returning0 without B8; this is
separate from profile.stats/rank44.005A81C0 uses profile user14, else LAST name04;
no profile uses the actual modelE8 name. It differs from005A82C0's short10 fallback
and from Actor name override. Nonoffline name/rank networking remains excluded.
005A78F0 searches DEFINITION identifier, not runtime instance identity: holders
0/2/3/4/5 first, then ascending holster/optional pack stacks with reverse entries.
No match leaves optional slot output unchanged; no cooldown/amount filter.

005A88B0/D0 return-1 for missing/non3 B0. Actual Active AI table008109F8 slots
13C/140 are00538590/B0: pending1CC/1D4 unless-1, else current1C8/1D0. These bodies
are now source overrides on the same state, not a presentation cache. Both
00538590/B0 are absent from the frozen inventory and earn zero new credit.

## Soldier publication

004B3C60 stores actual Human74, clears both actual TextStates, then004B3890 stores
rank8C and sets primary record0/style3A. Offline kinds4/5 use name only; others
localize actual rank table first ID27A6+rank, space, name. Eight-row bounds guard
native invalid access. Existing HumanStore research confirms first IDs27A6..AD
and second IDs2792..99; raw BSS zeros before initialization are not rank values.

Dead signed health selects actual death texture for photo. A living profile
updates photo only if portrait exists, then record1/style3B uses actual hand
weapon caption or empty string. Living no-profile does neither. Set material
RGBwhite, update photo visual, then cache fields78/80/84=1,7C=1,B0true,88/89false,
hide selection, initialize command/mind caches-1. Actual definition16hex/7/104
presence controls silence, signed raw stat108>=41 controls medic. Both
004B47D0/004B4860 run last: dead selects universal texture; live first/second
mode caches store BEFORE texture callbacks, with equality suppressing retry.
Current player/-1 selects universal; other first modes swap indices2/3, second
modes index directly. Artwork tables/texture borrowers remain the same HUD ones.
Later health/bar/simulation updates are separate, not claimed by initial binding.

## HUD and compass lifetime

0049B7E0 gates actual nonzero HUDflags and signed index0..5, binds soldier,
shows root, ORs4, adds actual compass marker, then refreshes0049F1C0 only when
Human equals SAME current control player. Failure preserves that native prefix.
0049B870 hides root then rereads six roots reverse. Any visible root returns
without clearing4 or marker notification. If none remain, clear4 then mark the
newest matching marker. Human74 remains borrowed; no invented slot compaction.
0049B900 now also handles populated Human borrowers: marker notification BEFORE
each reverse root hide, then flags&~6 only after the whole sequence.

00493D10 only adds for real Human, offline7, difficulty!=4 and two real ring
artwork borrowers.004936C0 chooses enemy art for property19C2, else friendly;
creates actual OBJ_, duplicates, shows, links original parent with1, copies local
position respecting native80 guard, stores native radius, copies/normalizes
local rotation, updates, then sets actual visualA8 alpha0. Marker amount starts1.
No deduplication.00493F40 reverses to the first matching Human;00493AF0 changes
nonzero amount to0 with NO frame call, erase or release. Original duplicate
behavior is preserved. Existing ring close releases marker owners reverse.
Later marker fade/simulation remains separate. Constructor/engine failures retire
the unappended local owner; world must close HUD before Human/scene retirement.

HudPanelsLoading now retains actual Items for these consumers; HudSceneLoading
exposes them. Its Actor deduction now decays a real array/vector element reference
BEFORE removing the pointer, fixing a concrete strict-composition failure with
actual borrowed pointer arrays. No alternate identity storage is introduced.

## Same presentation bridge

HumanLoadingPresentation borrows completed dependencies without requiring eager
cyclic template construction. SAME shared HumanPresentationShared supplies DD0
acquire/release/settings; actual player-mode receiver, FPV composition, camera,
first-person player lookup, Players module and HUD scene handle real callers.
Source Players0045E5D0 now composes first-player FPV bind/visibility BEFORE HUD
publication and collected-frame creation, including genuine duplicate callbacks.
Removal preserves actual roster/HUD/index/collected-frame order. Camera refresh
uses real current control, sector and target-retirement callback. Offline close
uses actual PlayersProcess004593E0.005A63F0 returns false offline7/missing B0/
nonplayer property; supported nonoffline player1 reads real70/virtual64.

Occupied Car004D9280, other nonoffline virtual64 and actual0049FBF0 inventory
refresh remain required: the former reject when reached, the latter is an explicit
containing dependency, without a successful default. Actual world/session creation,
remaining Human/FPV/SOUND ownership, guard publication and ordered loader/first
camera/render/failure/repeat are still required. This bridge alone enables none.

## Evidence and verification

Original client SHA2568c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Ignored WORK=analysis/work/mission-world-20261103 retains original exports
player-mode-and-soldier-receivers/, soldier-bind-first-frame-required/,
soldier-human-name-modes/; soldier-active-ai-modes.asm, hud-human-insertion-removal.asm,
hud-human-hide.asm and soldier-rank-modes.log. HumanStore rank initialization
was already confirmed in [HumanStore research](human-store.md). Original/DLL/
Windows code was never executed.

Focused ASan/UBSan/LSan passes actual Human/storage/Active AI getters/TextState,
soldier/artwork/compass and HUD add/remove/hide, profile/no-profile/dead/selected
conditions, caption/name/rank/texture caches, pending modes, marker gates/radius/
identity/reverse removal, source Players→presentation→HUD duplicate-add and
collected-frame lifetime, texture failure cache and partial marker cleanup.
FPV/engine/frame/item/localization/clock boundaries are explicit doubles; AI
getter fixtures seed actual bind-produced words rather than claiming full AI bind.
Strict i386 joins actual HUD panels/scene, real SessionOwner overlay/FPV/model/
Items/cache/Effects/camera, source player lookups/WorldActors map, Players module/
process, shared presentation and player close through the concrete bridge.
Remaining DD0/equipment/containing composition/SOUND/cursor/mission-world/
inventory-refresh endpoints are declarations. Application link and27 tracker
checks pass. Four temporary artifacts hashed/removed; source-only, no complete
promotions. Continue the full first checkbox after this subsystem checkpoint.
