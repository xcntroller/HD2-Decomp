# Reached SetAlarmType and immediate Run dependency — 2026-11-17

Source-only correction after d8c0f1d. Actual user executable b64c5168 passes the
fade dispatch and activation/Scene78/fixed/input/full-update prefix, then exits
result1 on required script command148 SetAlarmType. Exact executable and logs:
WORK/first-frame-script148-return. No stable game/scene/control/presentation
acceptance follows those prefix markers.
[Verification receipt](checkpoints/p22-frame-script-alarm-20261117.json).

00416E10 opcode94 translates byte3B at0041CE4F to00417EA9. It resolves this
context's100 frameEC actor0/type3, without a context-family restriction. Missing
frame/data/actor or another actor type skips both integers and takes the shared
0041C8C6 guarded-advance/true tail. For a Human it reads integerA0 operand1 as
nonzero BOOL, then operand0 DWORD mask, invokes005AF150, then guarded-advances.
005AF150 uses the SAME HumanB0; kind5 tail-calls0058E650 and another kind is a
genuine RET8. Missing/malformed required owners remain explicit errors.

0058E650's1B8 field is a SUPPRESSION mask: enabledtrue clears requested bits;
false ORs them. If flags50 has no bit80000000, enabledtrue and both old/requested
mask contain20, it may replay an existing enemy. It stores the new mask BEFORE
Human268 hostile-list inspection. Nonempty list takes one actual CRT rand modulo
count and calls SAME B0 virtual98 on the selected Human. Empty list, already-
interrupting state, disabled call, previously clear20 or absent requested20 draw
nothing. The source helper uses actual state words/perception and existing
sight_enemy_acquired (00590060 emit/interrupt), with the existing Contexts random
receiver. No new perception event, duplicate state or fabricated alarm readiness.

The captured T_EE_Target1..4 initial prefixes immediately follow SetAlarmType
with Zombie, Run and Suspend. Zombie/Suspend already have receivers; Run C4 was
missing. The same batch now supplies original00421C00 C4 (translation1C at
00423524 ->00422513), Human005AF270 kind5 gate and0058F2D0's exact1B0 mutation:
(old & FFFF4FFF) |4000. It preserves other bits and only requests the mode; the
existing actual frame consumes it later. Missing named points or signal-driven
movement are not changed. Run remains a real Human-family command; base alarm
command resolves its own EC owner independently.

Native ASan/UBSan/leak checks run actual base VM command policy, actual
HumanScriptLoadingControls/script-kind gate, scalar mask policy, context/ref and
perception owners. Explicit state/actor/frame/random/virtual-notification leaves
cover all low-byte masks and untouched high bits, both boolean directions,
replay admission/modulo/empty-list/random count, mask-before-callback, positive
callback failure before PC advance, non5 RET, missing B0 failure, full-DWORD BOOL,
missing/wrong actor skip before deliberately invalid integers and guarded PC.
The fixture checks1179692 assertions. Actual production Script-state/Run wrappers,
existing sight/interrupt/factory/module/application wiring are verified by the
full i386 O2/Werror application/probe build, not claimed as a desktop run. Native
fixtures are fingerprinted then removed. No permanent tests, function-count
promotion or matrix closure. Interactive review overlay remains uncommitted;
OWN dirty source and its prior staged artifact remain unchanged.

Research index preceded original reads. Existing typed00416E10/00421C00 exports
and fresh bounded raw005AF150/0058E650/005AF270/0058F2D0 assembly/translation tables
are retained in WORK=analysis/work/p22-frame-closure-20261117. No new Ghidra
session/export was necessary; no agent execution of original game/DLLs/Proton or
Windows fixtures. PE SHA256:
8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Next: actual revised desktop continuation and its first concrete failure. Broader
commands/cutscenes remain honest required boundaries; FRAME/HUM/OWN remain partial.

Actual revised1361c82f user run passes the reached alarm/Run continuation and
completed frame prefix, then handles missing AddDiaryText137. Exact executable
and logs:WORK/first-frame-script137-return. Stable game remains unaccepted.
