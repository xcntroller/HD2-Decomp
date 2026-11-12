# P22-HUM handoff —2026-11-12

FULL P2.2 / **second Human checkbox: reactions/damage/ragdoll remains OPEN**.
Read `AGENTS.md`, Phase2 section2 of `plan.md`, [Human plan](analysis/p22-hum-plan.md),
[Human contracts](analysis/human-ragdoll-source.md) and relevant linked contracts.
Continue dependencies/checkpoints until the FULL selected checkbox completes or
is genuinely blocked. Source progress does not close application/desktop work.

## Checkout and boundaries

**Work in `/home/control/Documents/Projects/H&D2 Decomp 4` (master).** User
requested transferring back here; transfer evidence is ignored WORK
`human-return-main-20261111`. Preceding checkpoint **8881fdd**; check current
HEAD/status. Preserve LIFE/UI/PACK/research worktrees, `/tmp/hd2-p22-hum-continue`
and unrelated changes. No agents were spawned. No push. No original EXE/DLL/
Proton or Windows fixture execution; native own audits only, Windows compile/link.
Heavy compilation and Ghidra MUST remain sequential. Mission loading remains
blocked; no mission desktop acceptance. Source-only checkpoint commits authorized.

## Latest verified source

[Rigid motion receipt](analysis/checkpoints/p2.2-human-car-rigid-motion-20261112.json)
adds11 client partials/extends3: **34 complete/2751 partial(2746client+5engine)**.
[Full contract](analysis/human-car-rigid-motion-source.md). Actual004C9000→004C8EE0
→wheel004C94B0/part004CB2B0 now runs through SAME Item12/RigidBody/World/tree/
buckets. Pose004CC520/impulse004CCCC0/004CE7A0/filter004CE990/sleep004CF440/
refresh004CEAA0 and borrowed actor contact cache are in source. Body20 elapsed
ABI corrected in `human-car-rigid-motion-final-abi`; initial motion-abi omits it.
004C8EE0/004C9480 absent frozen ZERO credit.007E4F02 is plane IntersectionLine,
LS3DF RVA306C0;004CD4F0 componentwise multiply,004CD520 add,004066C0 scale.
Native100 families PASS ASan/UBSan/LSan, Item12 regression PASS, strict actual
nested World/Human/Car/Item12/Physics i386 and launcher/probe PASS. Heavy jobs
sequential; explicit engine/frame/math/ray/actor/particle doubles, no original
trajectory/convergence/application/desktop claim. Temporary audits remain ignored.

**Confirmed new unproduced byte:** part004CB2CD reads/toggles5F9 before motion.
Actual ctor writes5F8/5FB/5FC/5FD; reset clears5EA..5F7 and sets5FA; neither launcher
writes5F9. Source fails BEFORE gravity/movement for a fresh part; warm verification
explicitly seeds known prior5F9. Do not invent a default or repeatedly re-audit
without new evidence. Wheel695 remains independently unproduced at final500ms.
Actual contact cache borrows ACTORS/common30, not primitive/frame retention.
Close removes descriptor BEFORE freeing cache. Query exclusions clear on failure;
water particle/requery stay inside SAME window. Matrix-only substep tails precede
containing004CEAA0 basis refresh. Preserve original ordered partial writes.

**8881fdd** [Item12 receipt](analysis/checkpoints/p2.2-human-car-item12-20261112.json),
[contract](analysis/human-car-item12-source.md): actual00479450/005FDA20/005FDAB0
clone/type12 owner, existing field74_ counter,005FDC00/005FDC50 launch and004CD550
wheel setup.005FDD10 body frame precedes lifetime; fade final500ms, expiry60 is
00486E90 camera amount0/frame54(1), NOT fade cancellation.64 is shared fade-out.

Preceding checkpoints remain verified; don't reopen unchanged kernels:
- **9ebacae** [glass fragments](analysis/human-car-fragments-source.md): actual
  69C→00485BA0 (absent frozen0credit)→00484F60 FASTCALL/RET40, SAME Physics/type4/
  fade borrower, six ENGINE draws, LOD→mesh→visual factory reference retirement.
- **a29033d** [physical reset/mesh/glass](analysis/human-car-physical-reset-source.md):
  004DC780→004E9DB0/004DCD10, immediate mesh enum, actual retained texture exchange/
  restore, seated Human DD0 opacity. Max6 saved wheel bits; mass*3 impulse at
  MODEL POSITION actor350/core2C0.004066C0 is SCALE, not decompiler subtraction.
- **5f7aa6d** [authority destruction](analysis/human-car-destruction-source.md):
  preserve254A/wire/Script/source/wheel/experience/area ordering, water current
  driver, victim hostility observer, radius5 negative amount, static-only ray ABI.
- **e6f08b7/cfd8aa3** [Car DM](analysis/human-car-dm-presentation-source.md)/
  [blast](analysis/human-car-blast-damage-source.md): SAME part/wheel/embedded24E0
  DM/packets/particle/SOUND. Confirmed unwritten EXPL record8 and unparsed smoke14
  remain honest failures, not repeated-audit targets.
- Earlier [Mine](analysis/human-mine-contact-source.md),
  [Versions](analysis/version-damage-transitions.md), clip/rest/reaction selection/
  damage/session checkpoints remain linked from the Human contract.

## Next work

1. Continue actual Car event2/wheel request serialization, timed-DM and deformed/
   global backup/occupied restore. `GameCarCallbacks::apply_car_wheel_damage`,
   `submit_car_wheel_damage`, `submit_car_human_damage` still require actual
   004DB2B0/004DB2E0. Existing `car_damage_packets.h` supplies kinds0/1/3/4/5,
   kind2 writer missing; read kind2 already exists. Original part/wheel Item12
   construction and body motion now have source. `car_ancillary_model.h` still
   requires004E1FE0/004DBF00 for positive countdown. `car_deformable_parts.h`
   reverse restore still requires actual mesh backup on flag4. Use research index
   BEFORE new exports; don't reopen confirmed clip/solver/reset/fragment kernels.
2. Full status9/received0 entry remains unbound. Read
   [shared world](analysis/multiplayer-world/shared-world.md).00730A40 needs actual
   peer6C/client334 wrappers/worker lifecycle→0072E890→00731610(true). Reuse WORK
   `human-entry-connection-originals`. Worker registration underB4 writes connection08
   =workerD4 before append; swap-last unregister does not delete. Client00723D20
   publishes334 thenport10/register; host007260A0 peer6C register AFTER418 unlock.
3. Preserve publication/failure order: status9 restores retained game BEFORE menu
   shutdown using SAME00698510/006989B0. Received0 manager replacement precedes
   old-game retirement; failed new-game deletion differs from status9's explicit
   shutdown/delete. Voice final ABI-v3 has EIGHT initializer args ending config/
   optional interface; interface18, publish0C before init, client-before-server close.
   Non7 zone decisions/frame and full Human containing frame/failure/re-entry/
   ordered teardown remain required. Actual loading/render/user acceptance remain
   separate. No fake transport/service wrappers.


## Commands

WORK=`analysis/work/mission-world-20261103`; original
`analysis/work/hd2.unpacked.exe` SHA256
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Frozen=`analysis/work/function-progress/client.json`.
`python3 -B tools/research_index.py find ADDRESS...` before exports; candidates
are not equivalence proof. `neighbors ADDRESS` has no`--count`.
`python3 -B tools/function_progress.py --render`/`--check`; `git diff --check`.

Compiler `.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`;
native runtime `.tools/hd2-sanitizer-runtime/usr/lib64`.
`make -C recovered -j1 application-launch startup-probe WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
Ghidra `ghidra/support/analyzeHeadless`, JAVA_HOME
`/home/control/.sdkman/candidates/java/25.0.0.0-oracle`, project
`/home/control/Documents/Projects/HD2ResearchProject` /HD2Research,
`-process hd2.unpacked.exe -readOnly -noanalysis`, rollback scripts, ONE scriptPath.
ExportFunctions refuses existing output dirs; inspect ERROR even on exit0.
Never overlap heavy compilation. `objdump -s` ignores bounds: bounded PE reads only.
