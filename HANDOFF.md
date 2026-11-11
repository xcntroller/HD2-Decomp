# P22-HUM handoff — 2026-11-11

FULL P2.2 continues with P22-HUM, **second checkbox: reactions/damage/ragdoll**.
It remains OPEN. Read `AGENTS.md`, section2 of `plan.md`,
[Human checklist](analysis/p22-hum-plan.md) and
[contracts / exact next work](analysis/human-ragdoll-source.md).
Continue dependencies until the full selected checkbox completes or genuinely blocks.

## Checkout and delivery

Active main checkout: `/home/control/Documents/Projects/H&D2 Decomp 4`.
Checkpoint base HEAD was `494cbee40566248084aaf9c8f01e7f75724d13c5`; inspect current
HEAD/status before acting. The initial cwd `/home/control/Documents/Projects/HD2-Decomp-p22-life`
is a DIFFERENT LIFE checkout and must not be edited for this Human task.
Preserve all source and other workstreams. No original EXE/DLL/Proton/Windows
fixture execution; compile/link only. No push. Commit completed source checkpoints.
Mission loading still declines; actual application and user desktop acceptance
remain FULL P2.2 requirements. Both P2.2 boxes stay open.

## Latest source checkpoint

The [particle/surface receipt](analysis/checkpoints/p2.2-human-particle-surface-20261111.json)
records28 added frozen client partials, two extensions, no complete promotions.
Totals:34 complete/2528 partial of18783. Reached breath/blood particle source
checkbox is now checked; reactions/damage/ragdoll is still unchecked.
Accumulated reaction/damage source is preserved but has no new ledger credit yet.

Joint/body impulses, constraints, actual contacts/Car external force, sleep and
substeps remain in the same owners.007B0910 reaction entry/007B7410 sampler,
007B3C80 clip collision/transitions,007A7470 containing presentation, rest surfaces,
0074D510 selection, actual damage controller and reached inventory/death/Player
feedback now have substantial source and focused verification. Exact ownership,
addresses, precision corrections and remaining gaps are in the contracts above.

NEW: actual positive particle module17/process owns EMTR instances, placed/active
records,256 cached visual buckets, shared1024-point refill pool and publication
scratch. Actual update/pause/finish/partial-load cleanup and frame/texture/vertex
ABI endpoints are present. Damage00451C00 and breath007507A0 use this SAME owner;
bleeding0044F4F0 draws texture50/54/58 before shared ShotMgr90 quality gates.
Rest-position queue0078AC30/0078AD00 now feeds actual0075D210 and0078AA10 through
track events4/5/6, existing13 texture pairs and SAME projection pool/list.

Correction:00761690 restores saved DD0 state through00784310→005AB7A0; it is not
an evidenced normal authored loader. Focused references found no calls to
0075D1A0. Normal184=5/188=NULL comes from reset. Complete nested Human persistence
remains P2.6; do not invent a loader or claim that whole parser from two branches.

## Verification

NEW native ASan/UBSan/LSan PASS logs in ignored WORK:
`human-particle-point-native.log`, `human-particle-instance-native.log`,
`human-particle-module-native.log`, `human-footstep-native.log`, rebuilt
`human-blood-native.log`. Engine/query boundaries are explicit doubles.
Earlier focused joint/retained/reaction/surface/damage/state/inventory/death/Player
logs remain PASS as detailed in the contracts; no broad unchanged audit repeated.
Strict actual World/control/particle/footstep i386 instantiation passes in
`human-ragdoll-host-i386.log`. Latest launcher compile/link PASS:
`human-particle-footstep-application-i386.log`. Windows outputs were NOT run.
Function-progress render/check and git diff --check PASS. Temporary native audits
remain ignored for this ongoing full-checkbox continuation; checkpoint receipt
records their hashes. Remove temporary audits at completion, no permanent tests.

## Immediate next work

1. Finish005A2FE0 nonoffline inventory drop and00588EF0 actual kind10 SECOND90
   command/serializer/receiver. New repaired exports:
   `human-damage-network-inventory-abi`. It also contains0072EE60/005A0DA0.
2. Correct earlier stub labels:0072EE60 is session player/team scoring plus update
   publication, and005A0DA0 is localized hit/death message selection. Finish their
   actual session/HUD owners, nonoffline damage policy, auxiliary Players death/
   selection and Player objective tail. Keep required failures honest.
3. Continue through containing frame/damage failure/re-entry/teardown until the
   full selected checkbox is complete or genuinely blocked. Consolidate next
   ledger/receipt/tally/doc updates at the next completed source checkpoint.

## Research and commands

Use research index BEFORE new exports:
`python3 -B tools/research_index.py find ADDRESS...`.
`neighbors ADDRESS` has no `--count`. Indexed mentions are candidates, not proof.
WORK=`analysis/work/mission-world-20261103`.
Client=`analysis/work/hd2.unpacked.exe`, SHA256
`8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307`.
Frozen inventory=`analysis/work/function-progress/client.json`.
Heavy native compilation, i386/application compilation and Ghidra MUST run
sequentially due memory limits. No agents requested. No heavy process is pending
at this checkpoint. Compiler:
`.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++`.
Native runtime=`.tools/hd2-sanitizer-runtime/usr/lib64`.

```sh
make -C recovered -j1 application-launch WINDOWS_CXX=../.tools/llvm-mingw-20261020-ucrt-ubuntu-22.04-x86_64/bin/i686-w64-mingw32-g++
```

Ghidra JAVA_HOME=`/home/control/.sdkman/candidates/java/25.0.0.0-oracle`;
project `/home/control/Documents/Projects/HD2ResearchProject` /`HD2Research`,
`-process hd2.unpacked.exe -readOnly -noanalysis`, rollback ABI scripts and ONE
script path. Inspect ERROR in logs despite exit0. Latest particle ABI is v3
(`human-particle-core-abi-v3`); earlier variants contain phantom registers and
incorrect443880 prototype. Latest damage-network export completed successfully;
logs `/tmp/hd2-human-damage-network*.log`. Original research outputs are ignored.

## Other workstreams

Preserve P22-LIFE/P22-UI. LIFE source closure and external boundaries are in
[its plan](analysis/p22-life-plan.md), [implementation](analysis/p22-life-implementation.md)
and [receipt](analysis/checkpoints/p22-life-20261111.json). Actual application
loading, FRAME/RENDER, integrity producers, failure/repeat handling and desktop
acceptance remain distinct from source reconstruction.
