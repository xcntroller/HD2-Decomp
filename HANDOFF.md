# Session handoff — 2026-11-09

Continue FULL P2.2: first unchecked implementation checkbox in plan.md. Read
AGENTS.md. Continue dependencies/checkpoints until full checkbox done or
concretely blocked. Larger verified subsystem commits; consolidate notes/tally.
No push, subagents/goals, original game/DLL/Proton or Windows fixture execution.
Preserve assets/user changes; focused verification, no repeated broad audits.

GameApplicationState still declines loading. No live world, first rendered world
frame or new mission desktop acceptance. Ledger:34 complete/1,665 partial
(1,660 client/five engine)/18,783 frozen; zero complete promotions.

## Latest verified work

After script compiler/context d49cb63 and module/visibility checkpoint01fb325:
[World sound/subtitles](analysis/sound-world-loading.md) supplies actual directed
portal hearing routing, double-closed.125 attenuation, same/root/unreachable
paths; game+60 Dialogs.bin load, same shared-font renderer, show/select/speech/
timer/render-state source. Captured English Dialogs.bin183120bytes/3250 records
parses. [Script rotations](analysis/mission-script-receivers.md#rotationframe-update-continuation--2026-11-09)
adds retained dedup/step/finish mask2 and actual matrix/viewport engine receivers.
Strict zero crossing retires rotations; equality stays active.00428B30 frame
update is separate from00428490 one-instruction script tick.

[Receipt](analysis/checkpoints/p2.2-world-presentation-20261109.json):14 new
partials/two extended; native ASan/UBSan/LSan, actual-host i386, app compile/link,
27 tracker tests pass. Twelve temporary fixture source/bin/objects hashed and
removed. Engine/Files/sector/glyph endpoints are doubles in native checks;
actual host instantiation is compile-only, not world or desktop acceptance.
No credit for absent00428B30/00427B80, delegated engine/merely exported bodies.

## Next — same entire checkbox

Continue actual containing world/session/Human/FPV/HUD/SOUND composition and
required actor/physics1/3/4/6. GamePhysicsObjects currently implements types0/2/5;
remaining factories reject. Runtime actors must precede physics, descriptors
before buckets, entries before Items. Recover actual required consumers rather
than successful empty services. GameRuntimeActors currently supports35/11;
placed item kind10 still needs its actual ground/effect receiver. MissionA8
objectives/zone37 A4 consumers and genuine first frame/failure/repeat/user
acceptance remain. Implement needed first-update VM/Human/Dog receivers alongside
those dependencies; full gameplay simulation belongs to the later P2.3 checkbox.

Deferred global animation queue008692C0 append is observed only in save restore
004257B0 (0042607B), not new mission load. It starts genuinely empty; do not treat
save-only production as an initial-load blocker. Saved reference/ASM evidence is
in WORK. Position/animation snapshot masks still reject;2/20/30 are implemented.
Source VM switch is two stage: script-base-vm-jump-map.json. FRM_Move77 and
GetPosition79 are unhandled base cases, requiring actual derived receivers.
Actual runtime Files/Patch selection, required notifications/context factories
and containing Human/FPV owners remain real dependencies. Do not credit fixture
receivers as implementations. Self ScriptAssign rejects unsafe executing-context
retirement. SetOn decoder scene-before-driver reverses compiler lookup order.

Canonical: [world](analysis/mission-world-loading.md),
[modules](analysis/loading-modules.md), [Human](analysis/human-state-loading.md),
[HUD](analysis/hud-loading.md), [activation](analysis/session-activation-loading.md),
[camera/local dispatch](analysis/camera-local-dispatch-loading.md),
[compiler](analysis/mission-script-loading.md), [script receivers](analysis/mission-script-receivers.md).

## Execution/evidence

WORK=analysis/work/mission-world-20261103 (ignored durable). Latest original
exports/ASM and fingerprints in linked contracts. All exports complete; no
pending process. Do not repeat confirmed compiler/table/portal/subtitle research.
Captured146 Tutorial scripts are BASE ISD0:145 compile; all114 base bindings
have source. This is not runtimePatch selection. Unbound weather safely rejects
raw-minus/operator folding; not evidence from running the original game.

Default sandbox for ordinary shell. Ghidra config and native sanitizers need
escalation; one Ghidra process, read-only project, fresh output; check ERROR even
on exit0. Java25:/home/control/.sdkman/candidates/java/25.0.0.0-oracle.
Sanitizer libs:.tools/hd2-sanitizer-runtime/usr/lib64 (verified; do not reinstall).
i386:.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
App:make -C recovered -j2 application-launch WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++.
