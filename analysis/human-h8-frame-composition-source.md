# H8 containing Human frame — composition checkpoint, 2026-11-12

H8 remains OPEN. Branch `feature/p22-hum-h8`, new worktree
`../HD2-Decomp-p22-hum-h8`, starts at completed H6fc9d452, which includes completed
H1–H7/HUMb9d67aa. Local research was copied using filesystem reflinks, retaining
original metadata/evidence without modifying another worktree. No branch push.
[H8 dependency plan](p22-hum-h8-plan.md),
[receipt](checkpoints/p2.2-human-h8-frame-composition-20261112.json).

## Same-owner frame dispatch

Scene2's existing `GameSceneActorFrames::update_human` now calls the concrete
World Human-frame overload. One `HumanFrameRuntime` borrows that SAME World for
Human0059FE30/0059FFD0→B0→inline98→DD000787270/00787D50→E0/equipment/geometry.
It owns no Human, B0, inventory, input manager, physics state, formation or clock.
The injectable frame overload remains useful for targeted verification, but the
normal Scene2 Human route no longer depends on an unnamed outer Runtime's Human
receiver implementations. Other Scene2/runtime actor families retain their own
required receiver boundary.

The bridge binds recovered H2 walk007646F0 and material0075EEE0, H3 attachment
inputs and seat8C/98, H4 Script event/formation/available completions, and H6 real
firing. The DD0-owned walk entry captures its own Human/E0/Services and command
publication; no private access override or duplicate control is used. The call
labelled a motion event is actually0075EEE0 material damage/retained hazard state,
not another event owner. Its ordinary material return and damage-before75C-store
order are preserved.

Two H4 Script attachment sites now pass their already available actual World
rather than requiring Runtime to impersonate that World. This preserves the
existing0058DB20 source and actor/mounted/camera owners. Player's expired530
callback uses cachedA8's SECONDARY payload4C+10 (serialized0C), then actual
00492130 zoom before the caller clears530. It does not reuse the distinct mode
transition's primary payload. Model debug reads SAME Startup0098AF98 presentation
flag, not a fabricated false callback. Its positive geometry5D94 visual remains
required. Walk's native MessageBeep(-1) delegates to the actual platform endpoint.

## Verification and honest frontier

Native warnings-as-errors ASan/UBSan/LSan PASS1100 over100 lifetimes exercises the
actual scene-frame policy, Runtime bridge and0075EEE0 helper. It checks authority/
presentation order, registered14 early return, countdown prefix, motion sampling,
feedback→actual supplied fire, formation, damage/store, state failure prefix,
missing B0 and required missing-family failure. Human/B0/inline/engine/World and
walk's control entry are explicit endpoint doubles. This is NOT a full live Human
owner graph or loaded World execution. Earlier actual H2/H6 ownership audits stay
valid within their existing contracts; no unchanged-kernel audit was repeated.

Strict actual nested-host i386 FULL LINK instantiates the ENTIRE Human scene-frame
World overload, both passes, Player/Script/Guard/Area/control/Look/geometry paths,
plus the Scene2 actor→World Human bridge with EMPTY outer Runtime. All H1–H7
source/ownership types are actual; remaining named positive guards still throw.
Latest launcher/probe rebuild passes; no Windows fixture, original EXE/DLL,
Proton, application or desktop execution. New temporary audit artifacts are
fingerprinted and removed; static original evidence/logs remain.

No new function entries or complete promotions. Existing frozen partials gain
composition evidence only. Recovered sources still replace native class/vtable/
allocator/SEH ABI; delegated engine bodies receive no extra credit.

Next mandatory frontier is exact Active AI008109F8 +24=00525DA0/+28=00526550;
Remote008112D8 +24=00592A60/+28=00592E30 is conditional non7. Indexed existing C
is retained for the two Active bodies. Runtime also names required00591550 Script
alarm, remaining00751480/00755500 actions, Player delayed/suppressed/remote/input
branches, non7 visibility/fall/respawn, actor58 and optional debug visual. Existing
World00451900 hazard blast/ShotMgr packet and005A97C0 water command/HUD receiver
remain explicit failures. Their original bodies/bytes are located/exported, not
implemented by this bridge. Do not mark them complete from successful linking.
Async ShotMgr, Car/OWN/D11/absent actors and full application/desktop remain the
H8 plan's continuing required work.
