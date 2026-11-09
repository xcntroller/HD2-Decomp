# Mission script compiler and context source — 2026-11-09

P2.2 remains open. This is a source subsystem checkpoint, not a running script
module or a loaded world. `GameApplicationState` still declines loading. No new
mission desktop test has been performed.

## Recovered source contract

`mission_script_source.h` reconstructs0042C850's0xFFFE-byte read prefix and
0042C580/0042C4B0 include expansion against the supplied live Files resolver.
It tries `Scripts\<resource>\<name>` before `Scripts\include\<name>`, closes
every opened handle and retains the native shared-cursor insertion behavior.
Directives are case sensitive; missing includes are ignored after blanking.
Native unsafe aliasing/embedded include NUL, buffer overrun and nonprogressing
expansion cases explicitly reject. No archive precedence is substituted.

`mission_script_lexer.h` retains0042CC00 cursor-presence versus token-presence
returns,0042CDD0 CR-only line counting, ASCII identifiers, dot-first numeric
tokens, quoted literal/comment behavior, consuming expect0042CF60 and bounded
1023-character tokens. It supplies no new credit for delegated CRT functions.

`mission_script_commands.h` preserves the actual ordered tables:275 commands
at00835B68,246 signatures at008338A8 and33 predicates at00832D60. Command
lookup0042DE80 is reverse case insensitive; predicate lookup0042E0E0 is forward.
Signature/predicate arity00414680/004146D0/0042E080 stops at the first zero kind.
Frozen table data alone does not earn a separate constructor-body credit.

`mission_script_program.h`, `mission_script_compiler.h` and
`mission_script_expression_parser.h` reconstruct the supported0042C850 front
end: Integer/String/Frame declarations, literal/symbol pools, assignments,
normal signature-driven commands, labels/forward GoTo/Gosub, braces/backpatches,
If/Else, Block/Return, start point, event registration and Whenever records.
Duplicate numeric events keep the first mapping and both instruction markers.
Whenever defaults enabled/500ms/zero elapsed; only SetWhenever8B forward
references are patched, not SetWheneverTime8E. Assignment expressions need no
extra semicolon check after0042D170 returns a delimiter (0042E160/0042ECD0).
Native string/instruction/payload ownership is represented by typed containers;
no host pointer is packed into a DWORD operand. Bool/Enum/Real declaration
families are not supplied as successful alternative types.

00411D00 numeric expression literals spill through FLOAT before integer
conversion; ordinary0042D920 arguments convert from double. Expressions retain
00412760's groups,00412540's supported operator list,00412D70's homogeneity
test and00412560's ordered binary folds. This includes native NOT/group and
mixed-operator quirks.00411650 uses wrapping integer arithmetic, signed
comparison/division and AND/OR short circuit;00411860 concatenates only the
actual string/integer fields and plus groups. Missing operator accesses and
native division traps explicitly reject. World predicates require a receiver.

`mission_script_context.h` reconstructs0041DCB0/0041DF20 frame lifetime using
the SAME actor EC data word1 and retained frame API; it does not register in the
unrelated004CE8F0 AI-context owner. Frame pool90 holds independent references.
0041E140 releases populated slots before world destruction.0041D8D0 resets
execution state, preserving existing slot references on a repeat begin; it
executes no initialization instruction.

004167B0 executes **one instruction per containing module tick**, not a loop
until delay. Periodic checks use strict signed accumulator>500 and
when.elapsed>when.period; elapsed is added after checking. Delays, GoTo, Gosub
(100-entry cap), Return, event-body skips and synchronous Block0041DC00 are
separate from00416E10 command dispatch. Synchronous blocks/exit have a bounded
execution guard.00416CF0 retains the authority and actual008692D0/game28
advance gate. Return end00416DB0 requires the real Human cleanup receiver.

Implemented VM branches include assignments, If/Else/braces, GetMyFrame,
SetWhenever/Time, DisableSignals/Alarms/Whenevers and EnableSignal. The original
00416E10 switch is two stage: its decompiled cases are **not original opcodes**.
`script-base-vm-jump-map.json` preserves the original opcode→case→target map.
0041D920/960/9B0/DA20/DA80/DAE0/DB50/DB80/DBB0 retain death/use/cutscene/
alarm/alarm-done/cutscene-done/load/exit/hit distinctions.00416610 preserves
signal masks, event exclusion and required Human B0-kind5 admission.
0041E3F0 case9 consumes pending signal bits; other predicates require actual
world receivers. Pending asynchronous stream operations explicitly reject.

`mission_script_contexts.h` reconstructs00427730 ownership and binding:
construct actual base/Human/Dog through a required factory, load/expand/compile,
append, begin, notify. Required005A3DF0/005907C0/Dog534+40 remain external
receivers, not credited implementations. Only exact `.scr` or `.SCR`
substrings suppress appending the extension. Null/empty removes; an existing
context with a nonempty script runs exit, destroys and swap-pops, then returns
false instead of replacing it. Failure destroys the context/EC binding before
notification. Reverse context traversal supports the native cutscene filter.
The containing module's deferred frame queue, timer, loaded gate and actual
activation/global variable stores remain to compose.00428490 is absent from the
frozen inventory and receives no new credit.

## Evidence and verification

WORK=`analysis/work/mission-world-20261103`. Original binary SHA256 remains
8c52ce664091d0fcf090b1af5c1f085e055a645ae4cb497227184ed36f10b307. Relevant
original C/JSON/assembly exports are in script-program-owner/,
human-state-script-dependencies/, script-compiler-required/,
script-lexer-context-required/, script-parser-operand-required/,
script-expression-required/, script-compile-owner-required/,
script-full-parser-required/, script-event-registration-required/,
script-expression-tree-required/, script-expression-evaluation-required/,
script-core-vm-required/, script-execution-controller-required/,
script-flow-pc-required/, script-module-tick-required/, script-events-required/
and script-signal-flags-required/. Failed overlapping probes0042D850,
0042DF60 and0042D320 are not body evidence. Valid preceding exports remain.

Focused native ASan/UBSan/LSan fixtures cover cursor/lexer quirks, pools,
captured script images, expressions/short circuit, labels/blocks/Whenever,
context retain/release, duplicate binding, one-step scheduling, delay/call/
branch/exit/event behavior, include fallback/precedence/close/read bounds and
binding failure/removal notifications. World commands, predicates, notification
factory and performance clock are boundary doubles. Strict i386 compilation
instantiates actual GameActorServices/ChunkFiles and required world receiver
declarations; it does not prove those world receivers exist. The Windows app
compiles/links; no original executable, DLL, Proton or Windows fixture is run.

Captured **base ISD0** census:145/146 source scripts compile,4481 instructions.
All114 captured Tutorial B810 binding names have source among the145. The
unbound `T_EE_Weather.scr` rejects at byte1443 (`If (X < -60)`): retained raw
minus yields an unsafe extra operator/child fold. This is a bounded source
rejection, not a claim that the original executable was tested or rejects with
the same diagnostic. Archive format ISD1 is not VM bytecode. None of these
captures establishes the runtime loose/Patch selection.

See [checkpoint](checkpoints/p2.2-script-compiler-context-20261109.json) for body
selections, verification logs and temporary artifact hashes/removal.

## Continue the full checkbox

Recover/implement actual base/Human00421C00/Dog00421970 command receivers and
0041E3F0 world predicates, including deferred/asynchronous operations; compose
script activation and actual runtime Files selection. Then finish containing
world/session/Human/FPV/HUD, SOUND portals/subtitles/effects, required actor/
physics families and missionA8 objectives/zone37 A4 consumers. Preserve actors
before physics and ordered teardown. First world frame, failure/repeat and user
desktop acceptance remain required. No successful empty context or VM fallback.
