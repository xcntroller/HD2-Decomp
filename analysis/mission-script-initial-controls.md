# Initial mission script controls — 2026-11-10

Source-only P2.2 subsystem checkpoint. Application loading remains disabled;
there is no loaded world, first world frame or new desktop acceptance. These
receivers still need the actual containing VM/session/FPV/world composition.
[Verification receipt](checkpoints/p2.2-script-initial-controls-20261110.json).

## Required loading boundary

The actual source compiler expands/compiles145/146 captured Tutorial programs.
All114 names referenced by the captured initial binding list compile. A first
VM tick executes one instruction, which can be a whole synchronous `Block`.
Intersecting those114 bindings gives these additional initial receivers:
Human Suspend/SetAlarm/SETAIMODE_Zombie/SETEVENTS/SETMODE_Guard; face texture;
Get/SetActiveCamera, SetCameraFrame; SetupTacticalMode; SetObjectiveStatus.
Existing frame lookup, assignment, Block and Whenever receivers are reused.
Static command union is dependency research, not execution evidence. The live
resolver and overlays remain authoritative. NPC1..4 first CreateItem commands
are assigned later, rather than among the114 initial bindings; do not expand
all P2.3 commands into this loading checkpoint on that basis.

## Human and same-owner receivers

`human_script_context_services.h` resolves the SAME Human model EC context,
including a pending bind-local context. Context nativeEC is **events[4]**, the
alarm-done event PC (0041DA80/0041D8D0), distinct from executingPC+D8 and pending
stream+24. Interrupts call actual0041DA20 and random draws use the shared owner.
005518F0 replays actual2AC records reverse through B4 into the command FIRST
queue. Previous Script5 state position140 has inclusive Y bounds[-100000,100000].
Its event1B4 mapping1/2/4/8/10/20/40/100/200 becomes respectively40000010,
3000000C,3000000B,3000000D,3000000E,30000001,40000015,20000013,20000004 in
the distinct processed SECOND queue, lifetime10000. Unknown masks emit nothing.

`human_script_loading_controls.h` requires the same per-Human factory/inline,
Scene2 active registry, Sight, process SOUND, Players and FPV.00421C00's reached
original opcodes1D/2F/C7/CF/F8 dispatch through the actual Human actor10C/B0.
Containing runtime must try base00416E10 before derived dispatch.

- 005AF190→0058E710: SetAlarm(true) genuinely does nothing. False clears50 sign
  bit and, only if hostile list268 is populated, draws shared rand, chooses a
  real Human and invokes actual state98 acquisition. Cold empty list draws none.
- 005AF2D0→0058F330 records `(1B0 & FFFF8FFF) | 8000`; it does **not** switch
  to Guard immediately. Later AI-update consumption is still required.
- 005AF3F0→0058F610 replaces only50 bits700 with Zombie mask400.
- 005AF3B0/3D0→0058F5D0/F5F0 set/clear50 bit40000000 **before** actual
  disable/enable.005A7F50 disables observer258, removes listener2A4, then stores
  FFFFFFFF.005A7FA0 enables observer258 and unconditionally adds the actual
  model/005A5430 listener, storing2A4. No invented deduplication.
- 0046F8C0 toggles SAME observer24. Disable reverses relations, resolves model
  EC on every notification and invokes Human lost notification before clearing
  relation words0/1; enable only clears flag24. Invalid/dead indices do nothing.
- First Suspend(true) sets delay18=2000, decrementsD8 and sets104/108. Retry
  invokes005AF570 then ordinary00416CF0 advance and clears108.0058FAA0 offline
  same-value guard stores1D2, suspends Human, then controls event flags/listeners.
  Non7 queued command16/+FC remains explicit rejection, before partial mutation.
- 005A5D50 changes actual Scene2 active membership, model visibility/update,
  inline suspension, current stateF0, target25C mode3/4, then current-player
  visibility refresh on resume. Attachment hiding still requires real receiver.
  F0 targets verified for ALL seven kinds:1/7→00583250 genuine RET4;
  3/4/5/6/8→005786C0, which updates actual input38 to0 suspended/7 resumed
  only outside offline7.005786C0 is absent inventory and earns no credit.
- 005AC170 uses SAME DD0 physical22C flag8 and existing collision transition.
  0058FCC0 calls real inline working-mode before storing1D4.005AC110 handles
  0/1/2 by clearing6 and adding2/4 for modes1/2; other modes genuinely do nothing.

## Base VM output and view receivers

0069BFE0 is a genuine **single C3 RET**, verified directly in the original PE.
`mission_script_output_commands.h` handles original00416E10 opcodes8C/B7:
actual string/integer pool operands, signed `%s %d` formatting for PrintfNumber,
retail empty output body and guarded00416CF0 advancement. No console/HUD sink
or guessed native stack-buffer capacity. Invalid operands reject safely.

`mission_script_view_commands.h` borrows real MissionCamera/FPV/Effects/session:

- 3A SwitchFaceTexture stores HumanBC BEFORE actual GEffects eye texture call,
  even if Effects declines; other/null frames pass to actual Effects receiver.
- 7B SetActiveCamera first saves/disables FPV58 and59 in context byte7/8 when
  frame differs from MissionCamera240. A noncamera still changes those flags.
  Kind3 own camera restores the saved flags before switching. For a real scene
  with an old active camera, copy near/far124/128, parameter138 and FOV120 via
  new-camera7C/8C/74, update24, then scene90. No old camera uses direct scene90.
- 7C GetActiveCamera releases old slot reference BEFORE querying scene15C and
  retaining the new frame. Missing scene/camera leaves a null retained slot.
- 7D SetCameraFrame calls actual MissionCamera virtual74=00465B20, setting
  borrowed288 and angle28C with int32*0080F6E8 (.01745329238474369f). This command
  passes angle-1. It is separate from retained248 Human-follow target and from
  0046C370 target selection.00465B20 is absent inventory: zero credit.
- FD SetupTacticalMode writes SAME SessionActivationState.words36 only.
  It does not toggle HUD or FPV.00465EE0 virtual4C compares actual camera240,
  including null equality.

All commands preserve authority/ordinary advance gates. Application does not
construct these view/derived runtime consumers yet. Objective88, ReturnEnd/
EndScript, subsequent needed tick commands/predicates, script look-target camera
update and the actual containing session/VM remain unfinished. No unsupported
required command may silently succeed.

## Evidence and verification

Ignored durable `analysis/work/mission-world-20261103` retains original exports:
`script-actor-command-receivers/`, `script-actor-command-typed-required/`,
`script-first-human-controls-required/`, `script-human-state-controls-required/`,
`script-initial-controls-dependencies/`,
`script-initial-suspend-objective-dependencies/`,
`script-initial-camera-objective-required/`,
`script-initial-camera-target-objective-state/`,
`camera-height-offset-producers/00465ee0.c`,
`human-containing-activation-dependencies/005a87c0.c` and
`human-suspend-vtable-targets.json`. Human derived switch labels decode float
bits against COMMANDS only; predicate opcode numbers overlap. Corrected
`script-human-command-labels.json` and base jump map/ASM are authoritative.
Client SHA2568c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307.
Read-only exports complete; no original executable/DLL/Windows fixture was run.

Focused native ASan/UBSan/LSan checks cover actual Human/B0/AI/Guard/Script,
compiler/VM Block, event queues/pool, Sight and process SOUND registration source,
2000ms suspension/retry/resume, actual MissionCamera/retained slots, camera
parameter order/FPV save-restore, persistent face texture, tactical flag and output
operand/authority/PC faults. Frame/inline/identity/presentation endpoints use
explicit doubles. Strict i386 compiles actual GameActorServices, Inputs/factory,
Scene2 active registry, Sight, Human SOUND and StartupSession registration,
MissionCamera and Effects; required inline/FPV/Players/VM consumers are explicit
declarations. Application compile/link and27 tracker tests pass. Temporary
fixtures/binaries are hashed and removed at this checkpoint; logs/captures stay.

Ledger adds22 partial client bodies, extends005518F0/00416E10, promotes none:
34 complete/1,833 partial (1,828 client/five engine), frozen18,783. No delegated
engine credit and no credit for absent005786C0/00465B20.
